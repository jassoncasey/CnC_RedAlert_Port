/**
 * Red Alert macOS Port - AI Team Management Implementation
 *
 * TeamTypeClass - Team templates (blueprints for unit groups)
 * TeamClass - Active team instances (groups of units executing orders)
 *
 * Based on original TEAM.CPP and TEAMTYPE.CPP
 */

#include "team.h"
#include "house.h"
#include "object.h"
#include <cstring>
#include <cstdlib>

//===========================================================================
// Global Team Arrays
//===========================================================================

TeamTypeClass TeamTypes[TEAMTYPE_MAX];
TeamClass Teams[TEAM_MAX];
int TeamTypeCount = 0;
int TeamCount = 0;

//===========================================================================
// TeamTypeClass Implementation
//===========================================================================

TeamTypeClass::TeamTypeClass() {
    Init();
}

void TeamTypeClass::Init() {
    name_[0] = '\0';
    id_ = -1;
    isActive_ = false;
    house_ = HousesType::NONE;

    // Flags
    isRoundabout_ = false;
    isSuicide_ = false;
    isAutocreate_ = false;
    isPrebuilt_ = false;
    isReinforcable_ = false;
    isTransient_ = false;
    isAlert_ = false;
    isWhiner_ = false;
    isLooseRecruit_ = false;
    isAggressive_ = false;
    isAnnoyance_ = false;

    // Composition
    priority_ = 0;
    maxAllowed_ = 1;
    initNum_ = 0;
    fear_ = 0;

    memberCount_ = 0;
    for (int i = 0; i < 8; i++) {
        members_[i].type = RTTIType::NONE;
        members_[i].typeIndex = -1;
        members_[i].count = 0;
    }

    // Mission script
    missionCount_ = 0;
    for (int i = 0; i < TEAM_MISSION_MAX; i++) {
        missions_[i].mission = TeamMissionType::NONE;
        missions_[i].argument = 0;
    }

    waypoint_ = -1;
}

int TeamTypeClass::TotalCount() const {
    int total = 0;
    for (int i = 0; i < memberCount_; i++) {
        total += members_[i].count;
    }
    return total;
}

bool TeamTypeClass::IsAvailable() const {
    if (!isActive_) return false;

    // Check if we've hit max instances
    int count = 0;
    for (int i = 0; i < TEAM_MAX; i++) {
        if (Teams[i].isActive_ && Teams[i].typeClass_ == this) {
            count++;
        }
    }

    return count < maxAllowed_;
}

TeamClass* TeamTypeClass::Create_Instance() {
    if (!IsAvailable()) return nullptr;

    // Find free slot
    for (int i = 0; i < TEAM_MAX; i++) {
        if (!Teams[i].isActive_) {
            Teams[i].Init(this);
            TeamCount++;
            return &Teams[i];
        }
    }

    return nullptr;
}

TeamTypeClass* TeamTypeClass::From_Name(const char* name) {
    if (!name || !name[0]) return nullptr;

    for (int i = 0; i < TEAMTYPE_MAX; i++) {
        if (TeamTypes[i].isActive_ &&
            strcmp(TeamTypes[i].name_, name) == 0) {
            return &TeamTypes[i];
        }
    }

    return nullptr;
}

TeamTypeClass* TeamTypeClass::Suggested_New_Team(HouseClass* house, bool alert) {
    if (!house) return nullptr;

    TeamTypeClass* best = nullptr;
    int bestPriority = 0;

    for (int i = 0; i < TEAMTYPE_MAX; i++) {
        TeamTypeClass* type = &TeamTypes[i];

        if (!type->isActive_) continue;
        if (type->house_ != house->type_) continue;
        if (!type->IsAvailable()) continue;

        // Alert teams only when alerted
        if (type->isAlert_ && !alert) continue;
        if (!type->isAlert_ && alert) continue;

        // Autocreate teams only
        if (!type->isAutocreate_) continue;

        // Check priority
        if (type->priority_ > bestPriority) {
            bestPriority = type->priority_;
            best = type;
        }
    }

    return best;
}

//===========================================================================
// TeamClass Implementation
//===========================================================================

TeamClass::TeamClass() {
    typeClass_ = nullptr;
    id_ = -1;
    isActive_ = false;
    house_ = HousesType::NONE;

    isForcedActive_ = false;
    isHasBeen_ = false;
    isUnderStrength_ = false;
    isReforming_ = false;
    isLagging_ = false;
    isMoving_ = false;
    isFull_ = false;

    memberCount_ = 0;
    for (int i = 0; i < TEAM_MEMBER_MAX; i++) {
        members_[i] = nullptr;
    }

    currentMission_ = 0;
    suspendedMission_ = -1;
    missionTimer_ = 0;

    target_ = 0;
    destination_ = 0;

    formation_ = FormationType::NONE;
    formationCenter_ = 0;

    zone_ = -1;
}

TeamClass::TeamClass(TeamTypeClass* type) {
    Init(type);
}

TeamClass::~TeamClass() {
    if (isActive_) {
        Disband();
    }
}

void TeamClass::Init(TeamTypeClass* type) {
    typeClass_ = type;
    id_ = -1;
    isActive_ = true;
    house_ = type ? type->house_ : HousesType::NONE;

    isForcedActive_ = false;
    isHasBeen_ = false;
    isUnderStrength_ = false;
    isReforming_ = false;
    isLagging_ = false;
    isMoving_ = false;
    isFull_ = false;

    memberCount_ = 0;
    for (int i = 0; i < TEAM_MEMBER_MAX; i++) {
        members_[i] = nullptr;
    }

    currentMission_ = 0;
    suspendedMission_ = -1;
    missionTimer_ = 0;

    target_ = 0;
    destination_ = 0;

    formation_ = FormationType::NONE;
    formationCenter_ = 0;

    zone_ = -1;

    // Assign ID
    for (int i = 0; i < TEAM_MAX; i++) {
        if (&Teams[i] == this) {
            id_ = i;
            break;
        }
    }
}

const char* TeamClass::Name() const {
    return typeClass_ ? typeClass_->Name() : "Unknown";
}

bool TeamClass::IsFull() const {
    if (!typeClass_) return false;

    int required = typeClass_->TotalCount();
    return memberCount_ >= required;
}

bool TeamClass::IsUnderStrength() const {
    if (!typeClass_) return true;

    // Under strength if less than 50% of required
    int required = typeClass_->TotalCount();
    return memberCount_ < (required / 2);
}

int TeamClass::Strength() const {
    if (!typeClass_) return 0;

    int required = typeClass_->TotalCount();
    if (required == 0) return 256;

    // Return 0-256 scale (256 = 100%)
    return (memberCount_ * 256) / required;
}

bool TeamClass::Add(FootClass* unit) {
    if (!unit) return false;
    if (memberCount_ >= TEAM_MEMBER_MAX) return false;

    // Check if already a member
    for (int i = 0; i < memberCount_; i++) {
        if (members_[i] == unit) return false;
    }

    members_[memberCount_++] = unit;

    // Update full status
    isFull_ = IsFull();
    isUnderStrength_ = IsUnderStrength();

    if (isFull_ && !isHasBeen_) {
        isHasBeen_ = true;
    }

    return true;
}

bool TeamClass::Remove(FootClass* unit) {
    if (!unit) return false;

    for (int i = 0; i < memberCount_; i++) {
        if (members_[i] == unit) {
            // Shift remaining members
            for (int j = i; j < memberCount_ - 1; j++) {
                members_[j] = members_[j + 1];
            }
            members_[--memberCount_] = nullptr;

            // Update status
            isFull_ = false;
            isUnderStrength_ = IsUnderStrength();

            return true;
        }
    }

    return false;
}

void TeamClass::Recruit() {
    // Stub - would scan for available units matching member specs
    // In full implementation, this queries house's unit pool
}

FootClass* TeamClass::Leader() const {
    // First member is the leader
    return (memberCount_ > 0) ? members_[0] : nullptr;
}

void TeamClass::AI() {
    if (!isActive_ || !typeClass_) return;

    // Check if team should disband
    if (memberCount_ == 0 && isHasBeen_) {
        Disband();
        return;
    }

    // Decrement mission timer
    if (missionTimer_ > 0) {
        missionTimer_--;
        return;
    }

    // Execute current mission
    if (!Execute_Mission()) {
        // Mission complete or failed, try next
        if (!Next_Mission()) {
            // No more missions, disband if transient
            if (typeClass_->isTransient_) {
                Disband();
            }
        }
    }
}

bool TeamClass::Next_Mission() {
    if (!typeClass_) return false;

    currentMission_++;

    if (currentMission_ >= typeClass_->missionCount_) {
        // Wrap to beginning or stop
        if (typeClass_->missionCount_ > 0) {
            // Check for loop mission
            const TeamMissionData& lastMission =
                typeClass_->missions_[typeClass_->missionCount_ - 1];

            if (lastMission.mission == TeamMissionType::JUMP) {
                currentMission_ = lastMission.argument;
                return true;
            }
        }

        currentMission_ = typeClass_->missionCount_ - 1;
        return false;
    }

    return true;
}

bool TeamClass::Execute_Mission() {
    if (!typeClass_ || currentMission_ < 0 ||
        currentMission_ >= typeClass_->missionCount_) {
        return false;
    }

    const TeamMissionData& mission = typeClass_->missions_[currentMission_];

    switch (mission.mission) {
        case TeamMissionType::ATTACK:
            // Attack specified quarry type
            {
                QuarryType quarry = static_cast<QuarryType>(mission.argument);
                uint32_t tgt = Find_Target(quarry);
                if (tgt) {
                    Attack(tgt);
                    return true;
                }
            }
            return false;

        case TeamMissionType::ATTACK_WAYPOINT:
            // Move to waypoint and attack anything there
            // Stub - needs waypoint system
            return false;

        case TeamMissionType::MOVE:
            // Move to waypoint
            // Stub - needs waypoint system
            return false;

        case TeamMissionType::MOVE_TO_CELL:
            // Move to specific cell
            return Move_To(mission.argument);

        case TeamMissionType::GUARD:
            // Guard current location
            missionTimer_ = 100;  // Guard for a while
            return true;

        case TeamMissionType::JUMP:
            // Jump to mission step
            currentMission_ = mission.argument - 1;  // -1 because Next_Mission increments
            return true;

        case TeamMissionType::PATROL:
            // Patrol to waypoint and back
            // Stub - needs waypoint system
            missionTimer_ = 50;
            return true;

        case TeamMissionType::UNLOAD:
            // Unload transported units
            // Stub - needs transport logic
            return false;

        case TeamMissionType::DEPLOY:
            // Deploy (MCV)
            // Stub - needs deploy logic
            return false;

        case TeamMissionType::CHANGE_FORMATION:
            formation_ = static_cast<FormationType>(mission.argument);
            return false;  // Immediate, proceed to next

        case TeamMissionType::SET_GLOBAL:
            // Set global trigger variable
            // Stub - needs trigger system
            return false;

        case TeamMissionType::INVULNERABLE:
            // Make team invulnerable
            // Stub - would set invulnerability flag on all members
            return false;

        default:
            return false;
    }
}

void TeamClass::Assign_Mission_Target(uint32_t newTarget) {
    target_ = newTarget;
}

bool TeamClass::Move_To(int32_t coord) {
    destination_ = coord;
    isMoving_ = true;

    // Command all members to move
    // Stub - would issue move orders to all members

    return true;
}

bool TeamClass::Is_At_Destination() const {
    if (!isMoving_) return true;
    if (memberCount_ == 0) return true;

    // Check if leader is close to destination
    FootClass* leader = Leader();
    if (!leader) return true;

    // Stub - would check actual position
    return false;
}

void TeamClass::Calc_Center() {
    if (memberCount_ == 0) {
        formationCenter_ = 0;
        return;
    }

    // Calculate average position of all members
    // Stub - would sum and average all member positions
    formationCenter_ = destination_;
}

void TeamClass::Attack(uint32_t attackTarget) {
    target_ = attackTarget;

    // Command all members to attack
    // Stub - would issue attack orders to all members
}

uint32_t TeamClass::Find_Target(QuarryType quarry) {
    // Find suitable target based on quarry type
    // Stub - would scan enemy units/buildings

    HouseClass* house = HouseClass::As_Pointer(house_);
    if (!house) return 0;

    HouseClass* enemy = HouseClass::As_Pointer(house->enemy_);
    if (!enemy) return 0;

    // In full implementation, would scan enemy's units based on quarry type
    // and return the target ID of the best match

    return 0;
}

void TeamClass::Take_Damage(TechnoClass* source) {
    // React to damage
    if (!source) return;

    // Aggressive teams attack back
    if (typeClass_ && typeClass_->isAggressive_) {
        // Find the attacker and retaliate
        // Stub - would set target to source
    }

    // Notify house of attack
    HouseClass* house = HouseClass::As_Pointer(house_);
    if (house) {
        house->Attacked(source);
    }
}

void TeamClass::Disband() {
    if (!isActive_) return;

    // Release all members
    for (int i = 0; i < memberCount_; i++) {
        // Members return to individual AI control
        members_[i] = nullptr;
    }
    memberCount_ = 0;

    isActive_ = false;
    TeamCount--;
}

void TeamClass::Suspend() {
    suspendedMission_ = currentMission_;
    // Stop current activity
}

void TeamClass::Resume() {
    if (suspendedMission_ >= 0) {
        currentMission_ = suspendedMission_;
        suspendedMission_ = -1;
    }
}

//===========================================================================
// Helper Functions
//===========================================================================

void Init_TeamTypes() {
    for (int i = 0; i < TEAMTYPE_MAX; i++) {
        TeamTypes[i].Init();
    }
    TeamTypeCount = 0;
}

void Init_Teams() {
    for (int i = 0; i < TEAM_MAX; i++) {
        Teams[i] = TeamClass();
    }
    TeamCount = 0;
}

TeamClass* Create_Team(TeamTypeClass* type) {
    if (!type) return nullptr;
    return type->Create_Instance();
}

void Destroy_Team(TeamClass* team) {
    if (team) {
        team->Disband();
    }
}
