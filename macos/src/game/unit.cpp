/**
 * Red Alert macOS Port - Unit Class Implementation
 *
 * Based on original UNIT.CPP and DRIVE.CPP (~12K lines combined)
 */

#include "unit.h"
#include "infantry.h"
#include "mapclass.h"
#include "pathfind.h"
#include "cell.h"
#include <cmath>
#include <cstring>

//===========================================================================
// Global Unit Pool
//===========================================================================
ObjectPool<UnitClass, UNIT_MAX> Units;

//===========================================================================
// Construction
//===========================================================================

UnitClass::UnitClass()
    : FootClass(RTTIType::UNIT, 0)
    , type_(UnitType::NONE)
    , trackStage_(0)
    , trackCounter_(0)
    , isTurretRotating_(false)
    , turretDesiredFacing_(DirType::N)
    , isHarvesting_(false)
    , isDeploying_(false)
    , isReturning_(false)
    , hasParachute_(false)
    , harvestState_(HarvestState::IDLE)
    , oreLoad_(0)
    , gemsLoad_(0)
    , harvestTimer_(0)
    , tiltX_(0)
    , tiltY_(0)
    , passengerCount_(0)
    , passengers_{}
{
}

UnitClass::UnitClass(UnitType type, HousesType house)
    : FootClass(RTTIType::UNIT, 0)
    , type_(UnitType::NONE)
    , trackStage_(0)
    , trackCounter_(0)
    , isTurretRotating_(false)
    , turretDesiredFacing_(DirType::N)
    , isHarvesting_(false)
    , isDeploying_(false)
    , isReturning_(false)
    , hasParachute_(false)
    , harvestState_(HarvestState::IDLE)
    , oreLoad_(0)
    , gemsLoad_(0)
    , harvestTimer_(0)
    , tiltX_(0)
    , tiltY_(0)
    , passengerCount_(0)
    , passengers_{}
{
    Init(type, house);
}

void UnitClass::Init(UnitType type, HousesType house) {
    type_ = type;
    SetHouse(house);

    const UnitTypeData* typeData = TypeClass();
    if (typeData) {
        strength_ = typeData->strength;
        ammo_ = -1;  // Unlimited ammo for vehicles by default

        // Set default mission
        AssignMission(typeData->defaultMission);
    }

    // Initialize turret facing to match body
    turretFacing_ = bodyFacing_;
    turretFacingTarget_ = bodyFacing_;
    turretDesiredFacing_ = bodyFacing_;
}

//===========================================================================
// Type Queries
//===========================================================================

const UnitTypeData* UnitClass::TypeClass() const {
    return GetUnitType(type_);
}

const char* UnitClass::Name() const {
    const UnitTypeData* typeData = TypeClass();
    if (typeData) {
        return typeData->iniName;
    }
    return "UNIT";
}

bool UnitClass::IsHarvester() const {
    const UnitTypeData* typeData = TypeClass();
    return typeData ? typeData->isHarvester : false;
}

bool UnitClass::IsCrusher() const {
    const UnitTypeData* typeData = TypeClass();
    return typeData ? typeData->isCrusher : false;
}

bool UnitClass::HasTurret() const {
    const UnitTypeData* typeData = TypeClass();
    return typeData ? typeData->hasTurret : false;
}

bool UnitClass::IsTransport() const {
    const UnitTypeData* typeData = TypeClass();
    return typeData ? (typeData->passengers > 0) : false;
}

bool UnitClass::IsMCV() const {
    return type_ == UnitType::MCV;
}

ArmorType UnitClass::GetArmor() const {
    const UnitTypeData* typeData = TypeClass();
    return typeData ? typeData->armor : ArmorType::NONE;
}

//===========================================================================
// Movement
//===========================================================================

bool UnitClass::StartDrive(int32_t destination) {
    if (!FootClass::StartDrive(destination)) return false;

    // Start track animation
    trackCounter_ = 0;

    return true;
}

bool UnitClass::StopDrive() {
    if (!FootClass::StopDrive()) return false;

    // Stop track animation
    trackStage_ = 0;
    trackCounter_ = 0;

    return true;
}

MoveType UnitClass::CanEnterCell(int16_t cell, FacingType facing) const {
    if (!Map.IsValidCell(cell)) return MoveType::NO;

    const CellClass& cellObj = Map[cell];
    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return MoveType::NO;

    // Check basic passability for unit's speed type
    if (!cellObj.IsPassable(typeData->speedType)) {
        return MoveType::NO;
    }

    // Check for occupied cell
    ObjectClass* occupier = cellObj.CellOccupier();
    if (occupier != nullptr) {
        // Crushers can enter cells with infantry
        if (IsCrusher() && occupier->IsInfantry()) {
            return MoveType::OK;  // Will crush infantry
        }

        // Otherwise blocked
        return MoveType::MOVING_BLOCK;
    }

    return MoveType::OK;
}

int UnitClass::TopSpeed() const {
    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return 32;

    int baseSpeed = typeData->speed;

    // Apply terrain modifiers
    CELL cell = Coord_Cell(coord_);
    if (Map.IsValidCell(cell)) {
        LandType land = Map[cell].GetLandType();
        if (land == LandType::ROAD) {
            baseSpeed = baseSpeed * 12 / 10;  // 20% faster on roads
        } else if (land == LandType::ROUGH) {
            baseSpeed = baseSpeed * 8 / 10;   // 20% slower on rough terrain
        }
    }

    return baseSpeed * 4;  // Scale to match game speed
}

void UnitClass::AnimateTracks() {
    if (!isDriving_) return;

    trackCounter_++;
    if (trackCounter_ >= 2) {
        trackCounter_ = 0;
        trackStage_++;
        if (trackStage_ >= TRACK_STAGES) {
            trackStage_ = 0;
        }
    }
}

void UnitClass::CalculateTilt() {
    // Would sample terrain height at corners of vehicle
    // and calculate tilt values
    // For now, simplified to flat
    tiltX_ = 0;
    tiltY_ = 0;
}

//===========================================================================
// Turret Control
//===========================================================================

void UnitClass::SetTurretFacing(DirType facing) {
    turretDesiredFacing_ = facing;
    if (turretFacing_ != facing) {
        isTurretRotating_ = true;
    }
}

int UnitClass::TurretShapeOffset() const {
    if (!HasTurret()) return 0;

    // Turret has 32 rotation stages
    int facing = static_cast<int>(turretFacing_) / 8;
    return facing;
}

void UnitClass::UpdateTurret() {
    if (!HasTurret()) return;
    if (turretFacing_ == turretDesiredFacing_) {
        isTurretRotating_ = false;
        return;
    }

    // Rotate turret towards desired facing
    int current = static_cast<int>(turretFacing_);
    int target = static_cast<int>(turretDesiredFacing_);
    int diff = target - current;

    // Wrap around
    if (diff > 128) diff -= 256;
    if (diff < -128) diff += 256;

    // Turret rotation speed
    int rotateSpeed = 8;
    const UnitTypeData* typeData = TypeClass();
    if (typeData && typeData->isLockTurret && isDriving_) {
        // Turret locked while moving - match body facing
        turretDesiredFacing_ = bodyFacing_;
        target = static_cast<int>(bodyFacing_);
        diff = target - current;
        if (diff > 128) diff -= 256;
        if (diff < -128) diff += 256;
    }

    if (diff > rotateSpeed) {
        current += rotateSpeed;
    } else if (diff < -rotateSpeed) {
        current -= rotateSpeed;
    } else {
        current = target;
        isTurretRotating_ = false;
    }

    if (current > 255) current -= 256;
    if (current < 0) current += 256;

    turretFacing_ = static_cast<DirType>(current);
}

//===========================================================================
// Combat
//===========================================================================

bool UnitClass::CanFire() const {
    if (!TechnoClass::CanFire()) return false;

    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return false;

    // Must have a weapon
    if (typeData->primaryWeapon == WeaponType::NONE) return false;

    // Harvesters can't fire while harvesting
    if (IsHarvester() && isHarvesting_) return false;

    return true;
}

int UnitClass::WeaponRange(int weapon) const {
    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Would look up weapon range from weapon type data
    // For now, return default based on unit type
    if (type_ == UnitType::ARTY || type_ == UnitType::V2_LAUNCHER) {
        return 8 * LEPTONS_PER_CELL;  // Long range artillery
    }
    return 5 * LEPTONS_PER_CELL;  // Default tank range
}

int UnitClass::RearmTime(int weapon) const {
    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return 60;

    // Artillery is slow
    if (type_ == UnitType::V2_LAUNCHER) return 180;
    if (type_ == UnitType::ARTY) return 90;

    // Mammoth can fire dual weapons
    if (type_ == UnitType::HTANK) return 30;

    return 45;  // Default
}

ResultType UnitClass::TakeDamage(int& damage, int distance, WarheadType warhead,
                                  TechnoClass* source, bool forced) {
    // Parachuted units take no damage briefly after landing
    if (hasParachute_) {
        damage = 0;
        return ResultType::NONE;
    }

    return FootClass::TakeDamage(damage, distance, warhead, source, forced);
}

//===========================================================================
// Harvester Operations
//===========================================================================

bool UnitClass::StartHarvest() {
    if (!IsHarvester()) return false;

    CELL cell = Coord_Cell(coord_);
    if (!Map.IsValidCell(cell)) return false;

    CellClass& cellObj = Map[cell];
    if (!cellObj.HasOre() && !cellObj.HasGems()) {
        return false;  // No ore here
    }

    isHarvesting_ = true;
    harvestState_ = HarvestState::HARVESTING;
    harvestTimer_ = 30;  // Ticks between ore pickups

    return true;
}

bool UnitClass::ReturnToRefinery() {
    if (!IsHarvester()) return false;

    isHarvesting_ = false;
    isReturning_ = true;
    harvestState_ = HarvestState::RETURN;

    // Would find nearest refinery and navigate to it
    AssignMission(MissionType::RETURN);

    return true;
}

int UnitClass::DumpOre() {
    if (!IsHarvester()) return 0;

    int value = (oreLoad_ * 25) + (gemsLoad_ * 50);
    oreLoad_ = 0;
    gemsLoad_ = 0;
    isReturning_ = false;
    harvestState_ = HarvestState::IDLE;

    return value;
}

void UnitClass::HarvesterAI() {
    if (!IsHarvester()) return;

    switch (harvestState_) {
        case HarvestState::IDLE:
            // Look for ore
            if (oreLoad_ + gemsLoad_ < 100) {
                harvestState_ = HarvestState::APPROACH;
            }
            break;

        case HarvestState::APPROACH:
            // Move towards ore field
            // Would scan for nearest ore and navigate
            break;

        case HarvestState::HARVESTING:
            harvestTimer_--;
            if (harvestTimer_ <= 0) {
                harvestTimer_ = 30;

                // Pick up ore from current cell
                CELL cell = Coord_Cell(coord_);
                if (Map.IsValidCell(cell)) {
                    CellClass& cellObj = Map[cell];
                    if (cellObj.HasGems()) {
                        int taken = cellObj.ReduceOre(50);
                        gemsLoad_ += taken / 50;
                    } else if (cellObj.HasOre()) {
                        int taken = cellObj.ReduceOre(25);
                        oreLoad_ += taken / 25;
                    } else {
                        // No more ore here, find more or return
                        if (IsOreLoadFull()) {
                            ReturnToRefinery();
                        } else {
                            harvestState_ = HarvestState::APPROACH;
                        }
                    }
                }

                // Check if full
                if (IsOreLoadFull()) {
                    ReturnToRefinery();
                }
            }
            break;

        case HarvestState::RETURN:
            // Movement handled by mission system
            break;

        case HarvestState::DUMPING:
            // Animation would play, then return to harvesting
            DumpOre();
            harvestState_ = HarvestState::APPROACH;
            break;
    }
}

//===========================================================================
// Transport Operations
//===========================================================================

bool UnitClass::LoadPassenger(ObjectClass* passenger) {
    if (!IsTransport()) return false;
    if (passenger == nullptr) return false;

    // Only infantry can be passengers
    if (!passenger->IsInfantry()) return false;

    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return false;

    if (passengerCount_ >= typeData->passengers) {
        return false;  // Full
    }

    if (passengerCount_ >= MAX_PASSENGERS) {
        return false;  // Safety check
    }

    // Store passenger pointer and limbo the unit
    InfantryClass* infantry = static_cast<InfantryClass*>(passenger);
    passengers_[passengerCount_] = infantry;
    passengerCount_++;
    infantry->Limbo();

    return true;
}

bool UnitClass::UnloadPassengers() {
    if (!IsTransport()) return false;
    if (passengerCount_ == 0) return false;

    CELL baseCell = Coord_Cell(coord_);
    int unloaded = 0;

    // Try to unload each passenger to adjacent cells
    for (int i = 0; i < passengerCount_; i++) {
        InfantryClass* infantry = passengers_[i];
        if (infantry == nullptr) continue;

        // Find an adjacent cell to place infantry
        bool placed = false;
        for (int d = 0; d < 8; d++) {
            FacingType dir = static_cast<FacingType>(d);
            CELL adjacent = Adjacent_Cell(baseCell, dir);

            if (adjacent == baseCell) continue;
            if (!Map.IsValidCell(adjacent)) continue;

            // Check if cell can accept infantry
            CellClass& cell = Map[adjacent];
            if (!cell.IsClearToMove(SpeedType::FOOT)) continue;

            // Place infantry in adjacent cell
            int32_t coord = Cell_Coord(adjacent);
            if (infantry->Unlimbo(coord, static_cast<DirType>(d * 32))) {
                infantry->AssignMission(MissionType::GUARD);
                placed = true;
                unloaded++;
                break;
            }
        }

        // If no adjacent cell, try the base cell
        if (!placed) {
            CellClass& cell = Map[baseCell];
            if (cell.IsClearToMove(SpeedType::FOOT, true)) {
                int32_t coord = Cell_Coord(baseCell);
                if (infantry->Unlimbo(coord, DirType::S)) {
                    infantry->AssignMission(MissionType::GUARD);
                    unloaded++;
                }
            }
        }

        passengers_[i] = nullptr;
    }

    passengerCount_ = 0;
    return unloaded > 0;
}

//===========================================================================
// MCV Operations
//===========================================================================

bool UnitClass::Deploy() {
    if (!IsMCV()) return false;
    if (!CanDeploy()) return false;

    isDeploying_ = true;
    // Would trigger deployment animation
    // Then spawn Construction Yard building

    return true;
}

bool UnitClass::CanDeploy() const {
    if (!IsMCV()) return false;

    // Check if area is clear for building
    CELL cell = Coord_Cell(coord_);
    if (!Map.IsValidCell(cell)) return false;

    // Would check 3x3 area for clear terrain
    // Simplified for now
    return Map[cell].IsClearToBuild();
}

//===========================================================================
// Animation
//===========================================================================

int UnitClass::ShapeNumber() const {
    const UnitTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Calculate body frame from facing
    int stages = typeData->rotationStages;
    int bodyFrame = static_cast<int>(bodyFacing_) / (256 / stages);
    if (bodyFrame >= stages) bodyFrame = 0;

    // Add track animation offset if moving
    int trackOffset = 0;
    if (isDriving_ && typeData->speedType == SpeedType::TRACK) {
        trackOffset = trackStage_ * typeData->rotationStages;
    }

    return bodyFrame + trackOffset;
}

//===========================================================================
// Mission Handlers
//===========================================================================

int UnitClass::MissionAttack() {
    // Face target with turret
    if (tarCom_ != 0 && HasTurret()) {
        // Calculate direction to target
        uint8_t dir = DirectionTo(tarCom_);
        SetTurretFacing(static_cast<DirType>(dir));
    }

    return 15;
}

int UnitClass::MissionGuard() {
    // Scan for enemies
    // Would look for targets in range
    return 60;
}

int UnitClass::MissionMove() {
    // Continue movement
    if (!isDriving_ && navCom_ == 0) {
        SetMission(MissionType::GUARD);
    }
    return 15;
}

int UnitClass::MissionHunt() {
    // Actively seek enemies
    return 60;
}

int UnitClass::MissionHarvest() {
    HarvesterAI();
    return 15;
}

int UnitClass::MissionUnload() {
    if (!IsTransport()) {
        SetMission(MissionType::GUARD);
        return 15;
    }

    // Unload passengers
    UnloadPassengers();
    SetMission(MissionType::GUARD);

    return 30;
}

//===========================================================================
// AI Processing
//===========================================================================

void UnitClass::AI() {
    FootClass::AI();

    // Update turret
    UpdateTurret();

    // Animate tracks
    AnimateTracks();

    // Update tilt
    CalculateTilt();

    // Harvester logic
    if (IsHarvester()) {
        HarvesterAI();
    }

    // Check for crushing infantry
    if (isDriving_ && IsCrusher()) {
        CheckCrush();
    }

    // Parachute timeout
    if (hasParachute_) {
        // Would decrement timer
        hasParachute_ = false;  // Simplified
    }
}

void UnitClass::PerCellProcess(PCPType pcp) {
    TechnoClass::PerCellProcess(pcp);

    switch (pcp) {
        case PCPType::CELL:
            // Entered new cell
            // Check for crushing, ore pickup, etc.
            if (IsCrusher()) {
                CheckCrush();
            }
            if (IsHarvester() && harvestState_ == HarvestState::APPROACH) {
                // Check if this cell has ore
                CELL cell = Coord_Cell(coord_);
                if (Map.IsValidCell(cell)) {
                    CellClass& cellObj = Map[cell];
                    if (cellObj.HasOre() || cellObj.HasGems()) {
                        StartHarvest();
                    }
                }
            }
            break;

        default:
            break;
    }
}

void UnitClass::SmoothMovement() {
    // Would smooth position updates for visual effect
}

void UnitClass::CheckCrush() {
    if (!IsCrusher()) return;

    CELL cell = Coord_Cell(coord_);
    if (!Map.IsValidCell(cell)) return;

    // Would check for infantry in cell and crush them
}

//===========================================================================
// Rendering
//===========================================================================

void UnitClass::DrawIt(int x, int y, int window) const {
    // Would render unit sprite at given position
    (void)x;
    (void)y;
    (void)window;
}

//===========================================================================
// Limbo/Unlimbo
//===========================================================================

bool UnitClass::Limbo() {
    if (!FootClass::Limbo()) return false;

    // Clear cell occupation
    CELL cell = Coord_Cell(coord_);
    if (Map.IsValidCell(cell)) {
        Map[cell].OccupyUp(this);
    }

    return true;
}

bool UnitClass::Unlimbo(int32_t coord, DirType facing) {
    if (!FootClass::Unlimbo(coord, facing)) return false;

    // Set initial facing
    bodyFacing_ = facing;
    bodyFacingTarget_ = facing;
    turretFacing_ = facing;
    turretFacingTarget_ = facing;
    turretDesiredFacing_ = facing;

    // Occupy cell
    CELL cell = Coord_Cell(coord);
    if (Map.IsValidCell(cell)) {
        Map[cell].OccupyDown(this);
    }

    return true;
}

//===========================================================================
// Helper Functions
//===========================================================================

UnitClass* CreateUnit(UnitType type, HousesType house, CELL cell) {
    UnitClass* unit = Units.Allocate();
    if (unit) {
        unit->Init(type, house);
        unit->Unlimbo(Cell_Coord(cell), DirType::S);
    }
    return unit;
}
