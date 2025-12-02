/**
 * Red Alert macOS Port - Object Class Hierarchy Implementation
 *
 * Base classes for all game objects.
 */

#include "object.h"
#include <cstring>
#include <cmath>
#include <cstdio>

// Forward declarations for trigger functions (avoid including mission.h
// which has conflicting type definitions from units.h vs types.h)
extern "C" void Mission_TriggerAttacked(const char* triggerName);
extern "C" void Mission_TriggerDestroyed(const char* triggerName);

//===========================================================================
// Helper Functions
//===========================================================================

int Distance(int32_t coord1, int32_t coord2) {
    // Coordinate format: high 16 bits = X, low 16 bits = Y
    int16_t x1 = static_cast<int16_t>(coord1 >> 16);
    int16_t x2 = static_cast<int16_t>(coord2 >> 16);
    int16_t y1 = static_cast<int16_t>(coord1 & 0xFFFF);
    int16_t y2 = static_cast<int16_t>(coord2 & 0xFFFF);
    int dx = x1 - x2;
    int dy = y1 - y2;
    // Approximate distance using max + half min
    int ax = abs(dx);
    int ay = abs(dy);
    return (ax > ay) ? (ax + ay / 2) : (ay + ax / 2);
}

uint8_t Direction256(int32_t coord1, int32_t coord2) {
    // Coordinate format: high 16 bits = X, low 16 bits = Y
    int16_t x1 = static_cast<int16_t>(coord1 >> 16);
    int16_t x2 = static_cast<int16_t>(coord2 >> 16);
    int16_t y1 = static_cast<int16_t>(coord1 & 0xFFFF);
    int16_t y2 = static_cast<int16_t>(coord2 & 0xFFFF);
    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 0 && dy == 0) return 0;

    // Use atan2 for accurate direction
    // In game coordinates: +X is East, +Y is South
    // Direction 0 = North, 64 = East, 128 = South, 192 = West
    double angle = atan2(static_cast<double>(dx), static_cast<double>(-dy));
    int dir = static_cast<int>((angle * 128.0) / M_PI) & 0xFF;
    return static_cast<uint8_t>(dir);
}

//===========================================================================
// AbstractClass Implementation
//===========================================================================

AbstractClass::AbstractClass(RTTIType rtti, int id)
    : rtti_(rtti)
    , id_(static_cast<int16_t>(id))
    , coord_(0xFFFFFFFF)  // Invalid coordinate
    , height_(0)
    , isActive_(true)
{
}

uint8_t AbstractClass::DirectionTo(const AbstractClass* object) const {
    if (object == nullptr) return 0;
    return Direction256(CenterCoord(), object->TargetCoord());
}

uint8_t AbstractClass::DirectionTo(int32_t coord) const {
    return Direction256(CenterCoord(), coord);
}

int AbstractClass::DistanceTo(int32_t coord) const {
    return Distance(CenterCoord(), coord);
}

int AbstractClass::DistanceTo(const AbstractClass* object) const {
    if (object == nullptr) return 0x7FFFFFFF;
    return Distance(CenterCoord(), object->TargetCoord());
}

MoveType AbstractClass::CanEnterCell(int16_t /*cell*/,
                                     FacingType /*facing*/) const {
    return MoveType::OK;
}

//===========================================================================
// ObjectClass Implementation
//===========================================================================

ObjectClass::ObjectClass(RTTIType rtti, int id)
    : AbstractClass(rtti, id)
    , isDown_(false)
    , isToDamage_(false)
    , isToDisplay_(false)
    , isInLimbo_(true)
    , isSelected_(false)
    , isAnimAttached_(false)
    , isFalling_(false)
    , riser_(0)
    , next_(nullptr)
    , strength_(0)
{
}

bool ObjectClass::IsFoot() const {
    return rtti_ == RTTIType::INFANTRY ||
           rtti_ == RTTIType::UNIT ||
           rtti_ == RTTIType::VESSEL ||
           rtti_ == RTTIType::AIRCRAFT;
}

bool ObjectClass::IsTechno() const {
    return rtti_ == RTTIType::BUILDING ||
           rtti_ == RTTIType::UNIT ||
           rtti_ == RTTIType::INFANTRY ||
           rtti_ == RTTIType::VESSEL ||
           rtti_ == RTTIType::AIRCRAFT;
}

bool ObjectClass::Limbo() {
    if (isInLimbo_) return false;

    Unselect();
    isDown_ = false;
    isInLimbo_ = true;
    return true;
}

bool ObjectClass::Unlimbo(int32_t coord, DirType /*facing*/) {
    if (!isInLimbo_) return false;

    coord_ = coord;
    isInLimbo_ = false;
    isDown_ = true;
    return true;
}

void ObjectClass::Detach(uint32_t /*target*/, bool /*all*/) {
    // Override in derived classes
}

void ObjectClass::DetachAll(bool all) {
    Detach(0, all);
}

const int16_t* ObjectClass::OccupyList(bool /*placement*/) const {
    static int16_t emptyList[] = {static_cast<int16_t>(0x8000)};  // END marker
    return emptyList;
}

const int16_t* ObjectClass::OverlapList(bool /*redraw*/) const {
    static int16_t emptyList[] = {static_cast<int16_t>(0x8000)};  // END marker
    return emptyList;
}

int ObjectClass::HealthRatio() const {
    // Returns 0-256 (256 = 100% health)
    // Would normally check strength_ vs max strength from type class
    if (strength_ <= 0) return 0;
    return 256;  // Placeholder - need type class for max strength
}

bool ObjectClass::Mark(MarkType /*mark*/) {
    isToDisplay_ = true;
    return true;
}

bool ObjectClass::Select() {
    if (isSelected_) return false;
    isSelected_ = true;
    return true;
}

void ObjectClass::Unselect() {
    isSelected_ = false;
}

bool ObjectClass::InRange(int32_t coord, int weapon) const {
    int range = WeaponRange(weapon);
    if (range <= 0) return false;
    return DistanceTo(coord) <= range;
}

int ObjectClass::WeaponRange(int /*weapon*/) const {
    return 0;  // Override in derived classes
}

ResultType ObjectClass::TakeDamage(int& damage, int /*distance*/,
                                   WarheadType /*warhead*/,
                                   TechnoClass* /*source*/,
                                   bool /*forced*/) {
    if (damage <= 0) return ResultType::NONE;

    // Apply damage
    if (strength_ > damage) {
        strength_ -= static_cast<int16_t>(damage);
        return ResultType::LIGHT;
    } else if (strength_ > 0) {
        strength_ = 0;
        return ResultType::DESTROYED;
    }
    return ResultType::NONE;
}

void ObjectClass::AI() {
    AbstractClass::AI();

    // Handle falling objects
    if (isFalling_) {
        height_ -= riser_;
        if (height_ <= 0) {
            height_ = 0;
            isFalling_ = false;
        }
    }
}

//===========================================================================
// MissionClass Implementation
//===========================================================================

MissionClass::MissionClass(RTTIType rtti, int id)
    : ObjectClass(rtti, id)
    , mission_(MissionType::NONE)
    , suspendedMission_(MissionType::NONE)
    , missionQueue_(MissionType::NONE)
    , status_(0)
    , timer_(0)
{
}

void MissionClass::AssignMission(MissionType mission) {
    if (mission != MissionType::NONE) {
        missionQueue_ = mission;
    }
}

bool MissionClass::Commence() {
    if (missionQueue_ != MissionType::NONE) {
        mission_ = missionQueue_;
        missionQueue_ = MissionType::NONE;
        status_ = 0;
        timer_ = 0;
        return true;
    }
    return false;
}

void MissionClass::SetMission(MissionType mission) {
    mission_ = mission;
    missionQueue_ = MissionType::NONE;
    status_ = 0;
    timer_ = 0;
}

// Default mission handlers - return delay in frames
int MissionClass::MissionSleep() { return 15 * 60; }  // 15 seconds at 60fps
int MissionClass::MissionAmbush() { return 15 * 60; }
int MissionClass::MissionAttack() { return 15; }
int MissionClass::MissionCapture() { return 15; }
int MissionClass::MissionGuard() { return 15 * 60; }
int MissionClass::MissionGuardArea() { return 15 * 60; }
int MissionClass::MissionHarvest() { return 15; }
int MissionClass::MissionHunt() { return 15 * 60; }
int MissionClass::MissionMove() { return 15; }
int MissionClass::MissionRetreat() { return 15; }
int MissionClass::MissionReturn() { return 15; }
int MissionClass::MissionStop() { return 15 * 60; }
int MissionClass::MissionUnload() { return 15; }
int MissionClass::MissionEnter() { return 15; }
int MissionClass::MissionConstruction() { return 15; }
int MissionClass::MissionDeconstruction() { return 15; }
int MissionClass::MissionRepair() { return 15; }
int MissionClass::MissionMissile() { return 15; }

void MissionClass::OverrideMission(MissionType mission,
                                   uint32_t /*target1*/,
                                   uint32_t /*target2*/) {
    suspendedMission_ = mission_;
    SetMission(mission);
}

bool MissionClass::RestoreMission() {
    if (suspendedMission_ != MissionType::NONE) {
        SetMission(suspendedMission_);
        suspendedMission_ = MissionType::NONE;
        return true;
    }
    return false;
}

const char* MissionClass::MissionName(MissionType mission) {
    static const char* names[] = {
        "Sleep", "Attack", "Move", "QMove", "Retreat", "Guard",
        "Sticky", "Enter", "Capture", "Harvest", "GuardArea",
        "Return", "Stop", "Ambush", "Hunt", "Unload", "Sabotage",
        "Construction", "Deconstruction", "Repair", "Rescue",
        "Missile", "Harmless"
    };
    int idx = static_cast<int>(mission);
    if (idx >= 0 && idx < static_cast<int>(sizeof(names) / sizeof(names[0]))) {
        return names[idx];
    }
    return "None";
}

MissionType MissionClass::MissionFromName(const char* name) {
    if (name == nullptr) return MissionType::NONE;

    static const struct { const char* name; MissionType mission; } table[] = {
        {"Sleep", MissionType::SLEEP},
        {"Attack", MissionType::ATTACK},
        {"Move", MissionType::MOVE},
        {"QMove", MissionType::QMOVE},
        {"Retreat", MissionType::RETREAT},
        {"Guard", MissionType::GUARD},
        {"Sticky", MissionType::STICKY},
        {"Enter", MissionType::ENTER},
        {"Capture", MissionType::CAPTURE},
        {"Harvest", MissionType::HARVEST},
        {"Area Guard", MissionType::GUARD_AREA},
        {"Return", MissionType::RETURN},
        {"Stop", MissionType::STOP},
        {"Ambush", MissionType::AMBUSH},
        {"Hunt", MissionType::HUNT},
        {"Unload", MissionType::UNLOAD},
        {"Sabotage", MissionType::SABOTAGE},
        {"Construction", MissionType::CONSTRUCTION},
        {"Deconstruction", MissionType::DECONSTRUCTION},
        {"Repair", MissionType::REPAIR},
        {"Rescue", MissionType::RESCUE},
        {"Missile", MissionType::MISSILE},
        {"Harmless", MissionType::HARMLESS},
    };

    for (const auto& entry : table) {
        if (strcasecmp(name, entry.name) == 0) {
            return entry.mission;
        }
    }
    return MissionType::NONE;
}

bool MissionClass::IsRecruitableMission(MissionType mission) {
    switch (mission) {
        case MissionType::GUARD:
        case MissionType::GUARD_AREA:
        case MissionType::SLEEP:
        case MissionType::HARMLESS:
            return true;
        default:
            return false;
    }
}

void MissionClass::AI() {
    ObjectClass::AI();

    // Process queued mission
    Commence();

    // Decrement timer
    if (timer_ > 0) {
        timer_--;
        return;
    }

    // Execute current mission
    int delay = 15;  // Default delay
    switch (mission_) {
        case MissionType::SLEEP: delay = MissionSleep(); break;
        case MissionType::AMBUSH: delay = MissionAmbush(); break;
        case MissionType::ATTACK: delay = MissionAttack(); break;
        case MissionType::CAPTURE: delay = MissionCapture(); break;
        case MissionType::GUARD: delay = MissionGuard(); break;
        case MissionType::GUARD_AREA: delay = MissionGuardArea(); break;
        case MissionType::HARVEST: delay = MissionHarvest(); break;
        case MissionType::HUNT: delay = MissionHunt(); break;
        case MissionType::MOVE:
        case MissionType::QMOVE: delay = MissionMove(); break;
        case MissionType::RETREAT: delay = MissionRetreat(); break;
        case MissionType::RETURN: delay = MissionReturn(); break;
        case MissionType::STOP: delay = MissionStop(); break;
        case MissionType::UNLOAD: delay = MissionUnload(); break;
        case MissionType::ENTER: delay = MissionEnter(); break;
        case MissionType::CONSTRUCTION: delay = MissionConstruction(); break;
        case MissionType::DECONSTRUCTION:
            delay = MissionDeconstruction();
            break;
        case MissionType::REPAIR: delay = MissionRepair(); break;
        case MissionType::MISSILE: delay = MissionMissile(); break;
        default: break;
    }
    timer_ = static_cast<int16_t>(delay);
}

//===========================================================================
// RadioClass Implementation
//===========================================================================

RadioClass::RadioClass(RTTIType rtti, int id)
    : MissionClass(rtti, id)
    , radio_(nullptr)
{
    oldMessages_[0] = RadioMessageType::STATIC;
    oldMessages_[1] = RadioMessageType::STATIC;
    oldMessages_[2] = RadioMessageType::STATIC;
}

RadioMessageType RadioClass::ReceiveMessage(RadioClass* from,
                                            RadioMessageType message,
                                            int32_t& /*param*/) {
    // Store message history
    oldMessages_[2] = oldMessages_[1];
    oldMessages_[1] = oldMessages_[0];
    oldMessages_[0] = message;

    switch (message) {
        case RadioMessageType::OVER_OUT:
            if (radio_ == from) {
                radio_ = nullptr;
            }
            return RadioMessageType::ROGER;

        case RadioMessageType::HELLO:
            if (radio_ == nullptr || radio_ == from) {
                radio_ = from;
                return RadioMessageType::ROGER;
            }
            return RadioMessageType::NEGATIVE;

        default:
            return RadioMessageType::STATIC;
    }
}

RadioMessageType RadioClass::TransmitMessage(RadioMessageType message,
                                             int32_t& param,
                                             RadioClass* to) {
    if (to == nullptr) {
        to = radio_;
    }
    if (to == nullptr) {
        return RadioMessageType::STATIC;
    }
    return to->ReceiveMessage(this, message, param);
}

RadioMessageType RadioClass::TransmitMessage(RadioMessageType message,
                                             RadioClass* to) {
    int32_t param = 0;
    return TransmitMessage(message, param, to);
}

bool RadioClass::Limbo() {
    if (radio_ != nullptr) {
        int32_t param = 0;
        TransmitMessage(RadioMessageType::OVER_OUT, param, radio_);
        radio_ = nullptr;
    }
    return MissionClass::Limbo();
}

const char* RadioClass::MessageName(RadioMessageType message) {
    static const char* names[] = {
        "Static", "Roger", "Hello", "OverOut", "Negative",
        "SquishMe", "ImIn", "BackingUp", "Tether", "Untether"
    };
    int idx = static_cast<int>(message);
    if (idx >= 0 && idx < static_cast<int>(sizeof(names) / sizeof(names[0]))) {
        return names[idx];
    }
    return "Unknown";
}

//===========================================================================
// TechnoClass Implementation
//===========================================================================

TechnoClass::TechnoClass(RTTIType rtti, int id)
    : RadioClass(rtti, id)
    , isUseless_(false)
    , isTickedOff_(false)
    , isCloakable_(false)
    , isLeader_(false)
    , isALoaner_(false)
    , isLocked_(false)
    , isInRecoilState_(false)
    , isTethered_(false)
    , isOwnedByPlayer_(false)
    , isDiscoveredByPlayer_(false)
    , isDiscoveredByComputer_(false)
    , isALemon_(false)
    , isSecondShot_(false)
    , armorBias_(256)
    , firepowerBias_(256)
    , idleTimer_(0)
    , ironCurtainTimer_(0)
    , spiedBy_(0)
    , archiveTarget_(0)
    , house_(HousesType::NONE)
    , cloakState_(CloakType::UNCLOAKED)
    , cloakTimer_(0)
    , cloakStage_(0)
    , tarCom_(0)
    , suspendedTarCom_(0)
    , navCom_(0)
    , suspendedNavCom_(0)
    , ammo_(-1)
    , pricePaid_(0)
    , turretFacing_(DirType::N)
    , turretFacingTarget_(DirType::N)
{
    arm_[0] = 0;
    arm_[1] = 0;
    triggerName_[0] = '\0';
}

void TechnoClass::SetHouse(HousesType house) {
    house_ = house;
    // Simplified - would check actual player house
    isOwnedByPlayer_ = (house == HousesType::GOOD);
}

void TechnoClass::AttachTrigger(const char* triggerName) {
    if (triggerName && triggerName[0] != '\0') {
        // Skip "None" triggers
        if (strcasecmp(triggerName, "None") == 0) {
            triggerName_[0] = '\0';
            return;
        }
        strncpy(triggerName_, triggerName, sizeof(triggerName_) - 1);
        triggerName_[sizeof(triggerName_) - 1] = '\0';
    } else {
        triggerName_[0] = '\0';
    }
}

bool TechnoClass::IsAllowedToRetaliate() const {
    // Check mission control settings
    return true;  // Simplified
}

bool TechnoClass::IsAllowedToScatter() const {
    // Check mission control settings
    return true;  // Simplified
}

bool TechnoClass::CanFire() const {
    if (isInLimbo_) return false;
    if (arm_[0] > 0) return false;  // Still rearming
    return true;
}

int TechnoClass::RearmTime(int weapon) const {
    (void)weapon;
    return 60;  // 1 second at 60fps - override in derived classes
}

void TechnoClass::Assign_Target(uint32_t target) {
    tarCom_ = target;
}

bool TechnoClass::Fire_At(int32_t targetCoord, int weapon) {
    // Check if we can fire
    if (!CanFire()) return false;

    // Check weapon index and arm timer
    if (weapon < 0 || weapon > 1) return false;
    if (arm_[weapon] > 0) return false;

    // Get weapon type from derived class
    int weaponType = GetWeapon(weapon);
    if (weaponType < 0) return false;

    // Check range
    if (!InRange(targetCoord, weapon)) return false;

    // Fire the weapon (creates bullet)
    // Note: Fire_Weapon is declared in combat.h
    // For now, just set the rearm timer
    arm_[weapon] = static_cast<int16_t>(RearmTime(weapon));

    // Set recoil state
    isInRecoilState_ = true;

    return true;
}

void TechnoClass::Cloak() {
    if (!isCloakable_) return;
    if (cloakState_ != CloakType::UNCLOAKED) return;

    cloakState_ = CloakType::CLOAKING;
    cloakTimer_ = 0;
}

void TechnoClass::Uncloak() {
    if (cloakState_ == CloakType::UNCLOAKED) return;

    cloakState_ = CloakType::UNCLOAKING;
    cloakTimer_ = 0;
}

void TechnoClass::AI() {
    RadioClass::AI();

    // Rearm timers
    if (arm_[0] > 0) arm_[0]--;
    if (arm_[1] > 0) arm_[1]--;

    // Idle timer
    if (idleTimer_ > 0) idleTimer_--;

    // Iron curtain countdown
    if (ironCurtainTimer_ > 0) ironCurtainTimer_--;

    // Cloak processing
    if (cloakState_ == CloakType::CLOAKING) {
        cloakStage_++;
        if (cloakStage_ >= 32) {
            cloakState_ = CloakType::CLOAKED;
            cloakStage_ = 0;
        }
    } else if (cloakState_ == CloakType::UNCLOAKING) {
        cloakStage_++;
        if (cloakStage_ >= 32) {
            cloakState_ = CloakType::UNCLOAKED;
            cloakStage_ = 0;
        }
    }

    // Turret rotation (simplified)
    if (turretFacing_ != turretFacingTarget_) {
        // Rotate towards target facing
        int current = static_cast<int>(turretFacing_);
        int target = static_cast<int>(turretFacingTarget_);
        int diff = target - current;
        if (diff > 128) diff -= 256;
        if (diff < -128) diff += 256;

        if (diff > 0) {
            current += 8;  // Rotation speed
            if (current > 255) current -= 256;
        } else if (diff < 0) {
            current -= 8;
            if (current < 0) current += 256;
        }
        turretFacing_ = static_cast<DirType>(current);
    }
}

ResultType TechnoClass::TakeDamage(int& damage, int distance,
                                   WarheadType warhead, TechnoClass* source,
                                   bool forced) {
    // Fire ATTACKED trigger if this object has one (before processing damage)
    // Only fire if there's a source (intentional attack, not environment)
    if (source != nullptr && HasTrigger()) {
        Mission_TriggerAttacked(triggerName_);
    }

    // Process the damage using base class
    ResultType result = RadioClass::TakeDamage(damage, distance, warhead,
                                                source, forced);

    // If destroyed, fire DESTROYED trigger and record kill
    if (result == ResultType::DESTROYED) {
        RecordKill(source);
    }

    return result;
}

void TechnoClass::RecordKill(TechnoClass* source) {
    // Fire DESTROYED trigger if this object has one
    if (HasTrigger()) {
        Mission_TriggerDestroyed(triggerName_);
    }

    // Record statistics (source killed this)
    (void)source;  // Would update kill counts here
}

//===========================================================================
// FootClass Implementation
//===========================================================================

FootClass::FootClass(RTTIType rtti, int id)
    : TechnoClass(rtti, id)
    , isInitiated_(false)
    , isMovingOntoBridge_(false)
    , isUnloading_(false)
    , isScattering_(false)
    , isPrimaryFacing_(true)
    , isRotating_(false)
    , isFiring_(false)
    , isDriving_(false)
    , isToLook_(false)
    , isDeploying_(false)
    , isNewNavCom_(false)
    , isPlanning_(false)
    , pathLength_(0)
    , pathIndex_(0)
    , headTo_(0)
    , member_(0)
    , speed_(0)
    , speedAccum_(0)
    , group_(-1)
    , bodyFacing_(DirType::N)
    , bodyFacingTarget_(DirType::N)
{
    for (int i = 0; i < 24; i++) {
        path_[i] = FacingType::NORTH;
    }
}

bool FootClass::StartDrive(int32_t destination) {
    if (isDriving_) return false;

    headTo_ = destination;
    isDriving_ = true;
    isNewNavCom_ = true;
    return true;
}

bool FootClass::StopDrive() {
    if (!isDriving_) return false;

    isDriving_ = false;
    speed_ = 0;
    return true;
}

void FootClass::DoTurn(DirType facing) {
    bodyFacingTarget_ = facing;
    isRotating_ = (bodyFacing_ != facing);
}

bool FootClass::BasicPath(int32_t destination) {
    // Simplified pathfinding - just set destination
    navCom_ = static_cast<uint32_t>(destination);
    pathLength_ = 0;
    pathIndex_ = 0;
    return true;
}

MoveType FootClass::CanEnterCell(int16_t cell, FacingType /*facing*/) const {
    // Simplified - would check terrain, other units, etc.
    (void)cell;
    return MoveType::OK;
}

int FootClass::TopSpeed() const {
    return 255;  // Override in derived classes based on type data
}

void FootClass::AI() {
    TechnoClass::AI();

    // Body rotation
    if (isRotating_ && bodyFacing_ != bodyFacingTarget_) {
        int current = static_cast<int>(bodyFacing_);
        int target = static_cast<int>(bodyFacingTarget_);
        int diff = target - current;
        if (diff > 128) diff -= 256;
        if (diff < -128) diff += 256;

        int rotateSpeed = 8;  // Rotation speed
        if (diff > rotateSpeed) {
            current += rotateSpeed;
        } else if (diff < -rotateSpeed) {
            current -= rotateSpeed;
        } else {
            current = target;
            isRotating_ = false;
        }
        if (current > 255) current -= 256;
        if (current < 0) current += 256;
        bodyFacing_ = static_cast<DirType>(current);
    }

    // Movement processing (simplified)
    if (isDriving_ && speed_ > 0) {
        // Calculate movement vector from facing
        int facing = static_cast<int>(bodyFacing_);
        double angle = (facing * M_PI * 2.0) / 256.0;
        int dx = static_cast<int>(sin(angle) * speed_);
        int dy = static_cast<int>(-cos(angle) * speed_);

        // Update position (simplified - would need proper cell handling)
        int x = (coord_ >> 8) & 0xFFFF;
        int y = coord_ & 0xFF;
        x += dx / 256;
        y += dy / 256;
        coord_ = (x << 8) | (y & 0xFF);
    }
}
