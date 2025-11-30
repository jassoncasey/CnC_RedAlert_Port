/**
 * Red Alert macOS Port - Infantry Class Implementation
 *
 * Based on original INFANTRY.CPP (~8K lines)
 */

#include "infantry.h"
#include "mapclass.h"
#include "cell.h"
#include <cmath>
#include <cstring>

//===========================================================================
// Global Infantry Pool
//===========================================================================
ObjectPool<InfantryClass, INFANTRY_MAX> Infantry;

//===========================================================================
// Sub-cell position offsets (in leptons from cell center)
// Cell is 256 leptons, center is 128,128
//===========================================================================
static const int SpotOffsetX[5] = { 0, -64, 64, -64, 64 };
static const int SpotOffsetY[5] = { 0, -64, -64, 64, 64 };

//===========================================================================
// Construction
//===========================================================================

InfantryClass::InfantryClass()
    : FootClass(RTTIType::INFANTRY, 0)
    , type_(InfantryType::NONE)
    , doing_(DoType::STAND_READY)
    , fear_(FEAR_NONE)
    , spot_(SpotType::CENTER)
    , spotTarget_(SpotType::CENTER)
    , isProne_(false)
    , isTechnician_(false)
    , isStoked_(false)
    , isStopping_(false)
    , frame_(0)
    , stageCount_(0)
    , idleTimer_(0)
{
}

InfantryClass::InfantryClass(InfantryType type, HousesType house)
    : FootClass(RTTIType::INFANTRY, 0)
    , type_(InfantryType::NONE)
    , doing_(DoType::STAND_READY)
    , fear_(FEAR_NONE)
    , spot_(SpotType::CENTER)
    , spotTarget_(SpotType::CENTER)
    , isProne_(false)
    , isTechnician_(false)
    , isStoked_(false)
    , isStopping_(false)
    , frame_(0)
    , stageCount_(0)
    , idleTimer_(0)
{
    Init(type, house);
}

void InfantryClass::Init(InfantryType type, HousesType house) {
    type_ = type;
    SetHouse(house);

    const InfantryTypeData* typeData = TypeClass();
    if (typeData) {
        strength_ = typeData->strength;
        isTechnician_ = typeData->canCapture;
    }

    // Set initial animation
    SetDoType(DoType::STAND_READY);

    // Random idle timer
    idleTimer_ = 300 + (rand() % 300);  // 5-10 seconds at 60fps
}

//===========================================================================
// Type Queries
//===========================================================================

const InfantryTypeData* InfantryClass::TypeClass() const {
    return GetInfantryType(type_);
}

const char* InfantryClass::Name() const {
    const InfantryTypeData* typeData = TypeClass();
    if (typeData) {
        return typeData->iniName;
    }
    return "INFANTRY";
}

bool InfantryClass::IsDog() const {
    const InfantryTypeData* typeData = TypeClass();
    return typeData ? typeData->isDog : false;
}

bool InfantryClass::CanCapture() const {
    const InfantryTypeData* typeData = TypeClass();
    return typeData ? typeData->canCapture : false;
}

bool InfantryClass::IsCivilian() const {
    const InfantryTypeData* typeData = TypeClass();
    return typeData ? typeData->isCivilian : false;
}

//===========================================================================
// Position and Movement
//===========================================================================

int32_t InfantryClass::SpotCoord(CELL cell, SpotType spot) {
    // Get cell center coordinate
    int32_t baseCoord = Cell_Coord(cell);
    int baseX = Coord_X(baseCoord);
    int baseY = Coord_Y(baseCoord);

    // Apply sub-cell offset
    int idx = static_cast<int>(spot);
    if (idx >= 0 && idx < 5) {
        baseX += SpotOffsetX[idx];
        baseY += SpotOffsetY[idx];
    }

    return XY_Coord(baseX, baseY);
}

bool InfantryClass::AssignSpot(CELL cell, SpotType spot) {
    // Check if spot is free
    if (!Map.IsValidCell(cell)) return false;

    CellClass& cellObj = Map[cell];
    if (!cellObj.IsSpotFree(spot)) return false;

    // Reserve the spot
    spotTarget_ = spot;
    navCom_ = static_cast<uint32_t>(SpotCoord(cell, spot));
    isNewNavCom_ = true;

    return true;
}

bool InfantryClass::StartDrive(int32_t destination) {
    if (!FootClass::StartDrive(destination)) return false;

    // Set walking animation
    if (isProne_) {
        SetDoType(DoType::CRAWL);
    } else {
        SetDoType(DoType::WALK);
    }

    return true;
}

bool InfantryClass::StopDrive() {
    if (!FootClass::StopDrive()) return false;

    isStopping_ = true;

    // Return to standing animation
    if (isProne_) {
        SetDoType(DoType::PRONE);
    } else {
        SetDoType(DoType::STAND_READY);
    }

    return true;
}

MoveType InfantryClass::CanEnterCell(int16_t cell, FacingType facing) const {
    if (!Map.IsValidCell(cell)) return MoveType::NO;

    const CellClass& cellObj = Map[cell];

    // Check basic passability for infantry
    if (!cellObj.IsPassable(SpeedType::FOOT)) {
        return MoveType::NO;
    }

    // Check for free sub-cell spot
    for (int i = 0; i < 5; i++) {
        if (cellObj.IsSpotFree(static_cast<SpotType>(i))) {
            return MoveType::OK;
        }
    }

    // No free spots
    return MoveType::NO;
}

int InfantryClass::TopSpeed() const {
    const InfantryTypeData* typeData = TypeClass();
    int baseSpeed = typeData ? typeData->speed : 4;

    // Prone movement is slower
    if (isProne_) {
        baseSpeed = baseSpeed * 2 / 3;
    }

    // Stoked gives temporary speed boost
    if (isStoked_) {
        baseSpeed = baseSpeed * 3 / 2;
    }

    // Scared civilians move faster
    if (IsCivilian() && IsScared()) {
        baseSpeed = baseSpeed * 4 / 3;
    }

    return baseSpeed * 4;  // Scale to match game speed
}

//===========================================================================
// Combat
//===========================================================================

void InfantryClass::GoProne() {
    if (isProne_) return;
    if (IsDog()) return;  // Dogs can't go prone

    isProne_ = true;
    SetDoType(DoType::LIE_DOWN);
}

void InfantryClass::StandUp() {
    if (!isProne_) return;

    isProne_ = false;
    SetDoType(DoType::GET_UP);
}

ResultType InfantryClass::TakeDamage(int& damage, int distance, WarheadType warhead,
                                      TechnoClass* source, bool forced) {
    // Prone infantry take less damage from HE
    if (isProne_ && warhead == WarheadType::HE) {
        damage = damage * 2 / 3;
    }

    // Increase fear when taking damage
    fear_ = static_cast<uint8_t>(std::min(255, fear_ + damage));

    // Check if should panic and run
    const InfantryTypeData* typeData = TypeClass();
    if (typeData && typeData->isFraidyCat && fear_ >= FEAR_SCARED) {
        // Fraidy cats run when scared
        AssignMission(MissionType::RETREAT);
    }

    return FootClass::TakeDamage(damage, distance, warhead, source, forced);
}

bool InfantryClass::CanFire() const {
    if (!TechnoClass::CanFire()) return false;

    // Can't fire while moving (except dogs)
    if (isDriving_ && !IsDog()) return false;

    // Can't fire while doing certain animations
    if (doing_ == DoType::LIE_DOWN || doing_ == DoType::GET_UP) {
        return false;
    }

    return true;
}

int InfantryClass::WeaponRange(int weapon) const {
    const InfantryTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Would look up weapon range from weapon type data
    // For now, return a default range based on unit type
    if (IsDog()) return 1 * LEPTONS_PER_CELL;  // Melee
    return 4 * LEPTONS_PER_CELL;  // Default rifle range
}

int InfantryClass::RearmTime(int weapon) const {
    const InfantryTypeData* typeData = TypeClass();
    if (!typeData) return 60;

    // Base rearm time would come from weapon type
    // Dogs have fast attack rate
    if (IsDog()) return 30;

    return 45;  // Default
}

//===========================================================================
// Animation
//===========================================================================

void InfantryClass::SetDoType(DoType doing) {
    if (doing_ == doing) return;

    doing_ = doing;
    frame_ = 0;
    stageCount_ = 0;

    StartAnimation();
}

const DoInfoStruct* InfantryClass::DoControls() const {
    return GetInfantryDoControls(type_);
}

int InfantryClass::ShapeNumber() const {
    const DoInfoStruct* controls = DoControls();
    if (!controls) return 0;

    int doIdx = static_cast<int>(doing_);
    if (doIdx < 0 || doIdx >= static_cast<int>(DoType::COUNT)) {
        doIdx = 0;
    }

    const DoInfoStruct& doInfo = controls[doIdx];

    // Calculate frame based on facing and animation
    int facing = static_cast<int>(bodyFacing_) / 32;  // 8 facings
    if (facing < 0) facing = 0;
    if (facing > 7) facing = 7;

    int baseFrame = doInfo.frame;
    int frameCount = doInfo.count;
    int frameJump = doInfo.jump;

    // Calculate animation frame within sequence
    int animFrame = frame_ % frameCount;

    // Add facing offset
    return baseFrame + (facing * frameJump) + animFrame;
}

void InfantryClass::StartAnimation() {
    const DoInfoStruct* controls = DoControls();
    if (!controls) return;

    int doIdx = static_cast<int>(doing_);
    if (doIdx < 0 || doIdx >= static_cast<int>(DoType::COUNT)) return;

    frame_ = 0;
    stageCount_ = 0;
}

void InfantryClass::AnimateFrame() {
    const DoInfoStruct* controls = DoControls();
    if (!controls) return;

    int doIdx = static_cast<int>(doing_);
    if (doIdx < 0 || doIdx >= static_cast<int>(DoType::COUNT)) return;

    const DoInfoStruct& doInfo = controls[doIdx];

    // Advance frame counter
    stageCount_++;

    // Animation speed (frames per game tick)
    int animSpeed = 4;
    if (isDriving_) animSpeed = 2;  // Faster when moving

    if (stageCount_ >= animSpeed) {
        stageCount_ = 0;
        frame_++;

        // Check for animation completion
        if (frame_ >= doInfo.count) {
            // Handle animation completion
            switch (doing_) {
                case DoType::LIE_DOWN:
                    // Transition to prone
                    doing_ = DoType::PRONE;
                    frame_ = 0;
                    break;

                case DoType::GET_UP:
                    // Transition to standing
                    doing_ = DoType::STAND_READY;
                    frame_ = 0;
                    break;

                case DoType::FIRE_WEAPON:
                case DoType::FIRE_PRONE:
                    // Return to ready position
                    if (isProne_) {
                        doing_ = DoType::PRONE;
                    } else {
                        doing_ = DoType::STAND_READY;
                    }
                    frame_ = 0;
                    break;

                case DoType::GUN_DEATH:
                case DoType::EXPLOSION_DEATH:
                case DoType::EXPLOSION2_DEATH:
                case DoType::GRENADE_DEATH:
                case DoType::FIRE_DEATH:
                    // Stay on last frame
                    frame_ = doInfo.count - 1;
                    break;

                case DoType::IDLE1:
                case DoType::IDLE2:
                    // Return to standing
                    doing_ = DoType::STAND_READY;
                    frame_ = 0;
                    idleTimer_ = 300 + (rand() % 300);
                    break;

                case DoType::WALK:
                case DoType::CRAWL:
                    // Loop walking animation
                    frame_ = 0;
                    break;

                default:
                    // Loop or hold
                    if (doInfo.count > 1) {
                        frame_ = 0;
                    }
                    break;
            }
        }
    }
}

//===========================================================================
// Mission Handlers
//===========================================================================

int InfantryClass::MissionAttack() {
    // Face target
    if (tarCom_ != 0) {
        // Would calculate direction to target
        // For now, simplified
    }

    // Set firing animation
    if (isProne_) {
        SetDoType(DoType::FIRE_PRONE);
    } else {
        SetDoType(DoType::FIRE_WEAPON);
    }

    return 15;  // Delay before next check
}

int InfantryClass::MissionGuard() {
    // Decrease fear over time
    DecayFear();

    // Check for idle animation
    if (idleTimer_ > 0) {
        idleTimer_--;
    } else if (doing_ == DoType::STAND_READY || doing_ == DoType::STAND_GUARD) {
        // Play random idle animation
        if (rand() % 2 == 0) {
            SetDoType(DoType::IDLE1);
        } else {
            SetDoType(DoType::IDLE2);
        }
        idleTimer_ = 600 + (rand() % 600);  // 10-20 seconds
    }

    return 60;  // 1 second delay
}

int InfantryClass::MissionMove() {
    // Check if reached destination
    if (!isDriving_ && navCom_ == 0) {
        // Reached destination, update spot
        spot_ = spotTarget_;
        SetMission(MissionType::GUARD);
        return 15;
    }

    // Continue moving
    return 15;
}

int InfantryClass::MissionHunt() {
    // Look for enemies
    // Would scan for nearby enemy targets

    // Move towards enemy base
    return 60;
}

int InfantryClass::MissionCapture() {
    // Check if adjacent to target building
    if (!CanCapture()) {
        SetMission(MissionType::GUARD);
        return 15;
    }

    // Would enter building to capture it
    return 30;
}

int InfantryClass::MissionEnter() {
    // Move towards transport/building
    return 15;
}

int InfantryClass::MissionRetreat() {
    // Run away from threats
    // Would find safe direction and move

    // Decrease fear while retreating
    if (fear_ > 0) {
        fear_--;
    }

    // Stop retreating when calm
    if (fear_ < FEAR_ANXIOUS) {
        SetMission(MissionType::GUARD);
    }

    return 15;
}

//===========================================================================
// AI Processing
//===========================================================================

void InfantryClass::AI() {
    FootClass::AI();

    // Animate
    AnimateFrame();

    // Decay fear over time
    DecayFear();

    // Check for scatter
    CheckScatter();

    // Update sub-cell position if moving
    if (isDriving_) {
        // Movement handled by FootClass
        // Just ensure animation is correct
        if (doing_ != DoType::WALK && doing_ != DoType::CRAWL) {
            if (isProne_) {
                SetDoType(DoType::CRAWL);
            } else {
                SetDoType(DoType::WALK);
            }
        }
    }
}

void InfantryClass::PerCellProcess(PCPType pcp) {
    TechnoClass::PerCellProcess(pcp);

    switch (pcp) {
        case PCPType::CELL:
            // Entered new cell - update spot
            spot_ = SpotType::CENTER;  // Default to center until assigned
            break;

        case PCPType::DESTINATION:
            // Reached destination
            spot_ = spotTarget_;
            break;

        default:
            break;
    }
}

void InfantryClass::DecayFear() {
    // Fear decays slowly over time
    if (fear_ > 0) {
        // Slower decay for civilians
        int decayRate = IsCivilian() ? 1 : 2;
        if (fear_ > decayRate) {
            fear_ -= decayRate;
        } else {
            fear_ = 0;
        }
    }
}

void InfantryClass::CheckScatter() {
    // Would check for incoming threats and scatter if needed
    // For now, simplified
}

//===========================================================================
// Rendering
//===========================================================================

void InfantryClass::DrawIt(int x, int y, int window) const {
    // Would render infantry sprite at given position
    // Actual rendering handled by graphics system
    (void)x;
    (void)y;
    (void)window;
}

//===========================================================================
// Limbo/Unlimbo
//===========================================================================

bool InfantryClass::Limbo() {
    if (!FootClass::Limbo()) return false;

    // Clear spot in cell
    CELL cell = Coord_Cell(coord_);
    if (Map.IsValidCell(cell)) {
        Map[cell].OccupyUp(this);
    }

    return true;
}

bool InfantryClass::Unlimbo(int32_t coord, DirType facing) {
    if (!FootClass::Unlimbo(coord, facing)) return false;

    // Find free spot in destination cell
    CELL cell = Coord_Cell(coord);
    if (Map.IsValidCell(cell)) {
        SpotType freeSpot = FindFreeSpot(cell);
        spot_ = freeSpot;
        spotTarget_ = freeSpot;

        // Adjust coordinate to spot position
        coord_ = SpotCoord(cell, freeSpot);

        // Occupy the spot
        Map[cell].OccupyDown(this);
    }

    // Set initial animation
    SetDoType(DoType::STAND_READY);

    return true;
}

//===========================================================================
// Helper Functions
//===========================================================================

InfantryClass* CreateInfantry(InfantryType type, HousesType house, CELL cell) {
    InfantryClass* infantry = Infantry.Allocate();
    if (infantry) {
        infantry->Init(type, house);
        infantry->Unlimbo(Cell_Coord(cell), DirType::S);
    }
    return infantry;
}

SpotType FindFreeSpot(CELL cell) {
    if (!Map.IsValidCell(cell)) return SpotType::CENTER;

    const CellClass& cellObj = Map[cell];

    // Check spots in order: center, then corners
    if (cellObj.IsSpotFree(SpotType::CENTER)) return SpotType::CENTER;
    if (cellObj.IsSpotFree(SpotType::UPPER_LEFT)) return SpotType::UPPER_LEFT;
    if (cellObj.IsSpotFree(SpotType::UPPER_RIGHT)) return SpotType::UPPER_RIGHT;
    if (cellObj.IsSpotFree(SpotType::LOWER_LEFT)) return SpotType::LOWER_LEFT;
    if (cellObj.IsSpotFree(SpotType::LOWER_RIGHT)) return SpotType::LOWER_RIGHT;

    // No free spot, default to center
    return SpotType::CENTER;
}
