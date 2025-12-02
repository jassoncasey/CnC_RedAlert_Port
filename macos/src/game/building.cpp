/**
 * Red Alert macOS Port - Building Class Implementation
 *
 * Based on original BUILDING.CPP (~12K lines)
 */

#include "building.h"
#include "house.h"
#include "mapclass.h"
#include "cell.h"
#include "unit.h"
#include "infantry.h"
#include "unit_types.h"
#include "infantry_types.h"
#include <cmath>
#include <cstring>
#include <cstdio>

//===========================================================================
// Global Building Pool
//===========================================================================
ObjectPool<BuildingClass, BUILDING_MAX> Buildings;

//===========================================================================
// Occupy Lists - Cell offsets for each building size
// List is terminated with 0x8000
//===========================================================================

// 1x1 building
static const int16_t Occupy11[] = { 0, static_cast<int16_t>(0x8000) };

// 2x1 building
static const int16_t Occupy21[] = { 0, 1, static_cast<int16_t>(0x8000) };

// 1x2 building
static const int16_t Occupy12[] = {
    0, MAP_CELL_W, static_cast<int16_t>(0x8000)
};

// 2x2 building
static const int16_t Occupy22[] = {
    0, 1, MAP_CELL_W, MAP_CELL_W + 1,
    static_cast<int16_t>(0x8000)
};

// 2x3 building
static const int16_t Occupy23[] = {
    0, 1,
    MAP_CELL_W, MAP_CELL_W + 1,
    MAP_CELL_W * 2, MAP_CELL_W * 2 + 1,
    static_cast<int16_t>(0x8000)
};

// 3x2 building
static const int16_t Occupy32[] = {
    0, 1, 2,
    MAP_CELL_W, MAP_CELL_W + 1, MAP_CELL_W + 2,
    static_cast<int16_t>(0x8000)
};

// 3x3 building
static const int16_t Occupy33[] = {
    0, 1, 2,
    MAP_CELL_W, MAP_CELL_W + 1, MAP_CELL_W + 2,
    MAP_CELL_W * 2, MAP_CELL_W * 2 + 1, MAP_CELL_W * 2 + 2,
    static_cast<int16_t>(0x8000)
};

// 4x2 building
static const int16_t Occupy42[] = {
    0, 1, 2, 3,
    MAP_CELL_W, MAP_CELL_W + 1, MAP_CELL_W + 2, MAP_CELL_W + 3,
    static_cast<int16_t>(0x8000)
};

static const int16_t* OccupyLists[] = {
    Occupy11,  // BSIZE_11
    Occupy21,  // BSIZE_21
    Occupy12,  // BSIZE_12
    Occupy22,  // BSIZE_22
    Occupy23,  // BSIZE_23
    Occupy32,  // BSIZE_32
    Occupy33,  // BSIZE_33
    Occupy42,  // BSIZE_42
    Occupy33   // BSIZE_55 (placeholder)
};

//===========================================================================
// Construction
//===========================================================================

BuildingClass::BuildingClass()
    : TechnoClass(RTTIType::BUILDING, 0)
    , type_(BuildingType::NONE)
    , bstate_(BStateType::IDLE)
    , bstateTarget_(BStateType::IDLE)
    , frame_(0)
    , stageCount_(0)
    , animStage_(0)
    , factoryState_(FactoryState::IDLE)
    , productionProgress_(0)
    , producingType_(RTTIType::NONE)
    , producingIndex_(-1)
    , isPowered_(true)
    , isRepairing_(false)
    , hasCharged_(false)
    , isCapturable_(true)
    , isGoingToBlow_(false)
    , isSurvivorless_(false)
    , countdownTimer_(0)
    , chargeTimer_(0)
    , lastTargetCoord_(0)
{
}

BuildingClass::BuildingClass(BuildingType type, HousesType house)
    : TechnoClass(RTTIType::BUILDING, 0)
    , type_(BuildingType::NONE)
    , bstate_(BStateType::IDLE)
    , bstateTarget_(BStateType::IDLE)
    , frame_(0)
    , stageCount_(0)
    , animStage_(0)
    , factoryState_(FactoryState::IDLE)
    , productionProgress_(0)
    , producingType_(RTTIType::NONE)
    , producingIndex_(-1)
    , isPowered_(true)
    , isRepairing_(false)
    , hasCharged_(false)
    , isCapturable_(true)
    , isGoingToBlow_(false)
    , isSurvivorless_(false)
    , countdownTimer_(0)
    , chargeTimer_(0)
    , lastTargetCoord_(0)
{
    Init(type, house);
}

void BuildingClass::Init(BuildingType type, HousesType house) {
    type_ = type;
    SetHouse(house);

    const BuildingTypeData* typeData = TypeClass();
    if (typeData) {
        strength_ = typeData->strength;

        // Walls are not capturable
        isCapturable_ = !typeData->isWall;
    }

    // Buildings start in construction state if not a wall
    if (!IsWall()) {
        SetBState(BStateType::CONSTRUCTION);
        AssignMission(MissionType::CONSTRUCTION);
    } else {
        SetBState(BStateType::IDLE);
        AssignMission(MissionType::GUARD);
    }
}

//===========================================================================
// Type Queries
//===========================================================================

const BuildingTypeData* BuildingClass::TypeClass() const {
    return GetBuildingType(type_);
}

const char* BuildingClass::Name() const {
    const BuildingTypeData* typeData = TypeClass();
    if (typeData) {
        return typeData->iniName;
    }
    return "BUILDING";
}

bool BuildingClass::IsFactory() const {
    const BuildingTypeData* typeData = TypeClass();
    return typeData ? (typeData->factoryType != RTTIType::NONE) : false;
}

RTTIType BuildingClass::FactoryType() const {
    const BuildingTypeData* typeData = TypeClass();
    return typeData ? typeData->factoryType : RTTIType::NONE;
}

bool BuildingClass::IsPowerPlant() const {
    bool isPwr = type_ == BuildingType::POWER;
    bool isAdv = type_ == BuildingType::ADVANCED_POWER;
    return isPwr || isAdv;
}

int BuildingClass::PowerOutput() const {
    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    int basePower = typeData->power;

    // Damaged buildings produce less power
    if (basePower > 0 && strength_ < typeData->strength) {
        int ratio = (strength_ * 100) / typeData->strength;
        basePower = (basePower * ratio) / 100;
    }

    return basePower;
}

bool BuildingClass::IsRefinery() const {
    return type_ == BuildingType::REFINERY;
}

bool BuildingClass::IsWall() const {
    return IsBuildingWall(type_);
}

bool BuildingClass::HasTurret() const {
    const BuildingTypeData* typeData = TypeClass();
    return typeData ? typeData->hasTurret : false;
}

void BuildingClass::GetSize(int& width, int& height) const {
    const BuildingTypeData* typeData = TypeClass();
    if (typeData) {
        GetBuildingSize(typeData->size, width, height);
    } else {
        width = 1;
        height = 1;
    }
}

//===========================================================================
// Cell Occupation
//===========================================================================

const int16_t* BuildingClass::OccupyList(bool placement) const {
    (void)placement;

    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) {
        return Occupy11;
    }

    int idx = static_cast<int>(typeData->size);
    if (idx < 0 || idx >= static_cast<int>(BSizeType::COUNT)) {
        return Occupy11;
    }

    return OccupyLists[idx];
}

int32_t BuildingClass::CenterCoord() const {
    // Calculate center based on building size
    int width, height;
    GetSize(width, height);

    int baseX = Coord_X(coord_);
    int baseY = Coord_Y(coord_);

    int centerX = baseX + (width * LEPTONS_PER_CELL) / 2;
    int centerY = baseY + (height * LEPTONS_PER_CELL) / 2;

    return XY_Coord(centerX, centerY);
}

int32_t BuildingClass::ExitCoord() const {
    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) return coord_;

    int baseX = Coord_X(coord_);
    int baseY = Coord_Y(coord_);

    return XY_Coord(baseX + typeData->exitX, baseY + typeData->exitY);
}

bool BuildingClass::CanPlaceAt(CELL cell) const {
    return CanPlaceBuildingAt(type_, cell, house_);
}

//===========================================================================
// Factory Operations
//===========================================================================

bool BuildingClass::StartProduction(RTTIType type, int index) {
    if (!IsFactory()) return false;
    if (factoryState_ != FactoryState::IDLE) return false;

    // Check if this factory produces the requested type
    if (FactoryType() != type) return false;

    producingType_ = type;
    producingIndex_ = static_cast<int16_t>(index);
    productionProgress_ = 0;
    factoryState_ = FactoryState::BUILDING;

    // Set active animation state
    SetBState(BStateType::ACTIVE);

    return true;
}

bool BuildingClass::CancelProduction() {
    if (factoryState_ == FactoryState::IDLE) return false;

    factoryState_ = FactoryState::IDLE;
    producingType_ = RTTIType::NONE;
    producingIndex_ = -1;
    productionProgress_ = 0;

    SetBState(BStateType::IDLE);

    return true;
}

bool BuildingClass::PauseProduction() {
    if (factoryState_ != FactoryState::BUILDING) return false;

    factoryState_ = FactoryState::HOLDING;
    return true;
}

bool BuildingClass::ResumeProduction() {
    if (factoryState_ != FactoryState::HOLDING) return false;

    factoryState_ = FactoryState::BUILDING;
    return true;
}

bool BuildingClass::CompleteProduction() {
    if (factoryState_ != FactoryState::READY) return false;

    // Get exit coordinate for spawned unit
    int32_t exitCoord = ExitCoord();
    CELL exitCell = Coord_Cell(exitCoord);

    // Find a free cell near the exit
    CELL spawnCell = exitCell;
    if (!Map.IsValidCell(spawnCell) ||
        Map[spawnCell].CellOccupier() != nullptr) {
        // Try adjacent cells
        static const int16_t offsets[] = {0, 1, -1, 128, -128, 129, -129};
        bool found = false;
        for (int i = 0; i < 7; i++) {
            CELL test = exitCell + offsets[i];
            if (Map.IsValidCell(test) &&
                Map[test].CellOccupier() == nullptr) {
                spawnCell = test;
                found = true;
                break;
            }
        }
        if (!found) {
            // No space to spawn - keep waiting
            fprintf(stderr, "CompleteProduction: No space to spawn!\n");
            return false;
        }
    }

    // Get owner house
    HousesType ownerHouse = Owner();

    // Spawn the produced object
    bool success = false;
    if (producingType_ == RTTIType::INFANTRY) {
        InfantryType itype = static_cast<InfantryType>(producingIndex_);
        InfantryClass* inf = CreateInfantry(itype, ownerHouse, spawnCell);
        if (inf) {
            fprintf(stderr, "CompleteProduction: Spawned infantry %d at %d\n",
                    producingIndex_, spawnCell);
            success = true;
        }
    } else if (producingType_ == RTTIType::UNIT) {
        UnitType utype = static_cast<UnitType>(producingIndex_);
        UnitClass* unit = CreateUnit(utype, ownerHouse, spawnCell);
        if (unit) {
            fprintf(stderr, "CompleteProduction: Spawned unit %d at %d\n",
                    producingIndex_, spawnCell);
            success = true;
        }
    }

    if (!success) {
        fprintf(stderr, "CompleteProduction: Failed to spawn!\n");
        return false;
    }

    // Reset state
    factoryState_ = FactoryState::IDLE;
    producingType_ = RTTIType::NONE;
    producingIndex_ = -1;
    productionProgress_ = 0;

    SetBState(BStateType::IDLE);

    return true;
}

//===========================================================================
// Power System
//===========================================================================

void BuildingClass::UpdatePower() {
    // Would check house's total power vs demand
    // For now, assume always powered
    isPowered_ = true;
}

int BuildingClass::GetPower() const {
    return PowerOutput();
}

//===========================================================================
// Combat
//===========================================================================

bool BuildingClass::CanFire() const {
    if (!TechnoClass::CanFire()) return false;

    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) return false;

    // Must have a weapon
    if (typeData->primaryWeapon == WeaponType::NONE) return false;

    // Must be powered
    if (!isPowered_) return false;

    // Can't fire while under construction
    if (bstate_ == BStateType::CONSTRUCTION) return false;

    return true;
}

int BuildingClass::WeaponRange(int weapon) const {
    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Turret buildings have different ranges
    if (type_ == BuildingType::TESLA) {
        return 6 * LEPTONS_PER_CELL;
    }
    if (type_ == BuildingType::TURRET || type_ == BuildingType::AAGUN ||
        type_ == BuildingType::SAM) {
        return 7 * LEPTONS_PER_CELL;
    }

    return 5 * LEPTONS_PER_CELL;
}

int BuildingClass::RearmTime(int weapon) const {
    // Tesla coil has slow recharge
    if (type_ == BuildingType::TESLA) return 120;

    // SAM sites reload slowly
    if (type_ == BuildingType::SAM) return 90;

    return 60;
}

ResultType BuildingClass::TakeDamage(int& damage, int distance,
                                      WarheadType warhead,
                                      TechnoClass* source, bool forced) {
    // Walls take extra damage from AP
    if (IsWall() && warhead == WarheadType::AP) {
        damage = damage * 3 / 2;
    }

    return TechnoClass::TakeDamage(damage, distance, warhead, source, forced);
}

bool BuildingClass::Sell() {
    // Can't sell walls
    if (IsWall()) return false;

    // Can't sell while under construction
    if (bstate_ == BStateType::CONSTRUCTION) return false;

    // Start deconstruction
    AssignMission(MissionType::DECONSTRUCTION);

    return true;
}

bool BuildingClass::StartRepair() {
    if (isRepairing_) return false;

    // Can't repair walls
    if (IsWall()) return false;

    // Already at full health?
    const BuildingTypeData* typeData = TypeClass();
    if (typeData && strength_ >= typeData->strength) return false;

    isRepairing_ = true;
    AssignMission(MissionType::REPAIR);

    return true;
}

bool BuildingClass::StopRepair() {
    if (!isRepairing_) return false;

    isRepairing_ = false;
    AssignMission(MissionType::GUARD);

    return true;
}

bool BuildingClass::Capture(HousesType newOwner) {
    if (!isCapturable_) return false;

    // Change ownership
    SetHouse(newOwner);

    // Stop any production
    CancelProduction();

    return true;
}

//===========================================================================
// Animation
//===========================================================================

int BuildingClass::ShapeNumber() const {
    // Simple frame calculation
    // Would be more complex based on building type and animation state
    int baseFrame = 0;

    // Add damage state offset
    const BuildingTypeData* typeData = TypeClass();
    if (typeData) {
        int healthRatio = HealthRatio();
        if (healthRatio < 128) {
            baseFrame += 1;  // Damaged frame
        }
    }

    // Add animation frame
    baseFrame += animStage_;

    return baseFrame;
}

void BuildingClass::SetBState(BStateType state) {
    if (bstate_ == state && bstateTarget_ == state) return;

    bstateTarget_ = state;
    if (bstate_ != state) {
        bstate_ = state;
        frame_ = 0;
        stageCount_ = 0;
    }
}

void BuildingClass::UpdateAnimation() {
    stageCount_++;

    // Animation speed depends on state
    int animSpeed = 4;
    if (bstate_ == BStateType::CONSTRUCTION) {
        animSpeed = 8;  // Slower construction animation
    }

    if (stageCount_ >= animSpeed) {
        stageCount_ = 0;
        frame_++;

        // Would check for animation completion
        // For now, just loop
        if (frame_ >= 4) {
            frame_ = 0;
        }
    }
}

//===========================================================================
// Mission Handlers
//===========================================================================

int BuildingClass::MissionAttack() {
    // Turret buildings aim at target
    if (HasTurret() && tarCom_ != 0) {
        TurretAI();
    }

    return 15;
}

int BuildingClass::MissionGuard() {
    // Scan for enemies
    if (HasTurret()) {
        // Would scan for enemies in range
    }

    return 30;
}

int BuildingClass::MissionConstruction() {
    // Advance construction animation
    productionProgress_++;
    if (productionProgress_ >= 100) {
        // Construction complete
        SetBState(BStateType::IDLE);
        SetMission(MissionType::GUARD);
    }

    return 3;  // Faster updates during construction
}

int BuildingClass::MissionDeconstruction() {
    // Play sell animation backwards
    productionProgress_--;
    if (productionProgress_ <= 0) {
        // Building sold, remove from game
        Limbo();
    }

    return 3;
}

int BuildingClass::MissionRepair() {
    RepairAI();
    return 30;
}

//===========================================================================
// AI Processing
//===========================================================================

void BuildingClass::AI() {
    TechnoClass::AI();

    // Update animation
    UpdateAnimation();

    // Update power status
    UpdatePower();

    // Factory production
    if (IsFactory()) {
        FactoryAI();
    }

    // Turret targeting
    if (HasTurret()) {
        TurretAI();
    }

    // Countdown timers
    if (countdownTimer_ > 0) countdownTimer_--;
    if (chargeTimer_ > 0) chargeTimer_--;

    // Superweapon charging
    bool isChrono = type_ == BuildingType::CHRONOSPHERE;
    bool isIronC = type_ == BuildingType::IRON_CURTAIN;
    if (isChrono || isIronC) {
        if (chargeTimer_ == 0 && isPowered_) {
            hasCharged_ = true;
        }
    }
}

void BuildingClass::FactoryAI() {
    // Get owner house
    HouseClass* house = HouseClass::As_Pointer(Owner());
    if (!house) return;

    // Get what type this factory produces
    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) return;
    RTTIType produces = typeData->factoryType;
    if (produces == RTTIType::NONE) return;

    if (!isPowered_) {
        // Suspend production when unpowered
        if (factoryState_ == FactoryState::BUILDING) {
            factoryState_ = FactoryState::SUSPENDED;
        }
        return;
    }

    // Resume suspended production
    if (factoryState_ == FactoryState::SUSPENDED) {
        factoryState_ = FactoryState::BUILDING;
    }

    // If idle and AI production enabled, start building something
    if (factoryState_ == FactoryState::IDLE) {
        // Only AI houses with isStarted_ auto-produce
        if (!house->isHuman_ && house->isStarted_) {
            // Get what the AI wants to build
            const TechnoTypeClass* toBuild =
                house->Suggest_New_Object(produces);
            if (toBuild) {
                // Start production
                // Determine what we're building based on factory type
                if (produces == RTTIType::INFANTRY) {
                    const InfantryTypeData* idata =
                        reinterpret_cast<const InfantryTypeData*>(toBuild);
                    producingType_ = RTTIType::INFANTRY;
                    producingIndex_ = static_cast<int16_t>(idata->type);
                    fprintf(stderr, "FactoryAI: %s starting infantry %d\n",
                            Name(), producingIndex_);
                } else if (produces == RTTIType::UNIT) {
                    const UnitTypeData* udata =
                        reinterpret_cast<const UnitTypeData*>(toBuild);
                    producingType_ = RTTIType::UNIT;
                    producingIndex_ = static_cast<int16_t>(udata->type);
                    fprintf(stderr, "FactoryAI: %s starting unit %d\n",
                            Name(), producingIndex_);
                }

                factoryState_ = FactoryState::BUILDING;
                productionProgress_ = 0;
            }
        }
    }

    if (factoryState_ == FactoryState::BUILDING) {
        productionProgress_++;
        if (productionProgress_ >= 100) {
            factoryState_ = FactoryState::READY;
            fprintf(stderr, "FactoryAI: %s production complete!\n", Name());
        }
    }

    // When production is ready, spawn the unit (for AI)
    if (factoryState_ == FactoryState::READY) {
        if (!house->isHuman_) {
            // Auto-spawn for AI houses
            if (CompleteProduction()) {
                factoryState_ = FactoryState::IDLE;
                productionProgress_ = 0;
                producingType_ = RTTIType::NONE;
                producingIndex_ = -1;

                // Clear house's build queue so AI can pick next item
                if (produces == RTTIType::INFANTRY) {
                    house->buildInfantry_ = -1;
                } else if (produces == RTTIType::UNIT) {
                    house->buildUnit_ = -1;
                }
            }
        }
    }
}

void BuildingClass::TurretAI() {
    if (tarCom_ != 0) {
        // Aim at target
        uint8_t dir = DirectionTo(tarCom_);
        turretFacingTarget_ = static_cast<DirType>(dir);
    }
}

void BuildingClass::RepairAI() {
    if (!isRepairing_) return;

    const BuildingTypeData* typeData = TypeClass();
    if (!typeData) return;

    // Repair one point of damage
    if (strength_ < typeData->strength) {
        strength_++;
        // Would deduct credits from owner
    } else {
        // Repair complete
        StopRepair();
    }
}

void BuildingClass::DoFireAnimation() {
    SetBState(BStateType::ACTIVE);
    // Animation will return to idle after a few frames
}

//===========================================================================
// Rendering
//===========================================================================

void BuildingClass::DrawIt(int x, int y, int window) const {
    // Would render building sprite at given position
    (void)x;
    (void)y;
    (void)window;
}

//===========================================================================
// Limbo/Unlimbo
//===========================================================================

bool BuildingClass::Limbo() {
    if (!TechnoClass::Limbo()) return false;

    // Clear cell occupation
    CELL baseCell = Coord_Cell(coord_);
    const int16_t* occupyList = OccupyList();

    while (*occupyList != static_cast<int16_t>(0x8000)) {
        CELL cell = baseCell + *occupyList;
        if (Map.IsValidCell(cell)) {
            Map[cell].OccupyUp(this);
        }
        occupyList++;
    }

    return true;
}

bool BuildingClass::Unlimbo(int32_t coord, DirType facing) {
    if (!TechnoClass::Unlimbo(coord, facing)) return false;

    // Set turret facing
    turretFacing_ = facing;
    turretFacingTarget_ = facing;

    // Occupy cells
    CELL baseCell = Coord_Cell(coord);
    const int16_t* occupyList = OccupyList();

    while (*occupyList != static_cast<int16_t>(0x8000)) {
        CELL cell = baseCell + *occupyList;
        if (Map.IsValidCell(cell)) {
            Map[cell].OccupyDown(this);
            // Mark cell as having building
            // Would set cell flags
        }
        occupyList++;
    }

    return true;
}

//===========================================================================
// Helper Functions
//===========================================================================

BuildingClass* CreateBuilding(BuildingType type, HousesType house, CELL cell) {
    if (!CanPlaceBuildingAt(type, cell, house)) {
        return nullptr;
    }

    BuildingClass* building = Buildings.Allocate();
    if (building) {
        building->Init(type, house);
        building->Unlimbo(Cell_Coord(cell), DirType::S);
    }
    return building;
}

bool CanPlaceBuildingAt(BuildingType type, CELL cell, HousesType house) {
    (void)house;  // Would check ownership

    const BuildingTypeData* typeData = GetBuildingType(type);
    if (!typeData) return false;

    // Get occupy list
    int idx = static_cast<int>(typeData->size);
    if (idx < 0 || idx >= static_cast<int>(BSizeType::COUNT)) {
        return false;
    }
    const int16_t* occupyList = OccupyLists[idx];

    // Check each cell
    while (*occupyList != static_cast<int16_t>(0x8000)) {
        CELL checkCell = cell + *occupyList;
        if (!Map.IsValidCell(checkCell)) {
            return false;
        }
        if (!Map[checkCell].IsClearToBuild()) {
            return false;
        }
        occupyList++;
    }

    return true;
}
