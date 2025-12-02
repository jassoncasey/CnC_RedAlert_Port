/**
 * Red Alert macOS Port - House/Faction Implementation
 *
 * Based on original HOUSE.CPP (~7765 lines) and HDATA.CPP
 */

#include "house.h"
#include "team.h"
#include "object.h"
#include "unit_types.h"
#include "infantry_types.h"
#include "building_types.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

//===========================================================================
// Global Variables
//===========================================================================

HouseClass Houses[HOUSE_MAX];
HouseClass* PlayerPtr = nullptr;
int HouseCount = 0;

//===========================================================================
// House Type Data (from HDATA.CPP)
//===========================================================================

#define A SideType::ALLIED
#define S SideType::SOVIET
#define N SideType::NEUTRAL
#define D 256  // Default bias value
static const HouseTypeData HouseTypeDataArray[] = {
    // Name        FullName   INI  Side  Col1 Col2  Biases (7x)
    {"Spain",     "Spain",   "SPA", A,   5,  176, D, D, D, D, D, D, D},
    {"Greece",    "Greece",  "GRE", A,   1,  135, D, D, D, D, D, D, D},
    {"USSR",      "Russia",  "RED", S, 123,  127, D, D, D, D, D, D, D},
    {"England",   "England", "ENG", A, 159,  167, D, D, D, D, D, D, D},
    {"Ukraine",   "Ukraine", "UKR", S,  24,   25, D, D, D, D, D, D, D},
    {"Germany",   "Germany", "GER", A, 204,  207, D, D, D, D, D, D, D},
    {"France",    "France",  "FRA", A, 136,  143, D, D, D, D, D, D, D},
    {"Turkey",    "Turkey",  "TRK", A, 184,  191, D, D, D, D, D, D, D},
    {"GoodGuy",   "Allies",  "GDI", A,   1,  135, D, D, D, D, D, D, D},
    {"BadGuy",    "Soviet",  "NOD", S, 123,  127, D, D, D, D, D, D, D},
    {"Neutral",   "Neutral", "NEU", N, 204,  207, D, D, D, D, D, D, D},
    {"Special",   "Special", "SPC", N, 204,  207, D, D, D, D, D, D, D},
    {"Multi1",    "Multi1",  "MP1", A,   5,  176, D, D, D, D, D, D, D},
    {"Multi2",    "Multi2",  "MP2", S, 123,  127, D, D, D, D, D, D, D},
    {"Multi3",    "Multi3",  "MP3", A, 159,  167, D, D, D, D, D, D, D},
    {"Multi4",    "Multi4",  "MP4", S,  24,   25, D, D, D, D, D, D, D},
};
#undef A
#undef S
#undef N
#undef D

static constexpr int HOUSE_TYPE_COUNT =
    sizeof(HouseTypeDataArray) / sizeof(HouseTypeDataArray[0]);

const HouseTypeData* GetHouseType(HousesType type) {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < HOUSE_TYPE_COUNT) {
        return &HouseTypeDataArray[idx];
    }
    return nullptr;
}

HousesType HouseTypeFromName(const char* name) {
    if (name == nullptr) return HousesType::NONE;

    for (int i = 0; i < HOUSE_TYPE_COUNT; i++) {
        if (strcasecmp(name, HouseTypeDataArray[i].iniName) == 0) {
            return static_cast<HousesType>(i);
        }
    }
    return HousesType::NONE;
}

//===========================================================================
// HouseClass Construction
//===========================================================================

HouseClass::HouseClass()
    : type_(HousesType::NONE)
    , id_(-1)
    , isActive_(false)
    , isHuman_(false)
    , isPlayerControl_(false)
    , isDefeated_(false)
    , isToWin_(false)
    , isToLose_(false)
    , isAlerted_(false)
    , isDiscovered_(false)
    , isMaxedOut_(false)
    , isStarted_(false)
    , isBaseBuilding_(false)
    , allies_(0)
    , credits_(0)
    , tiberium_(0)
    , capacity_(0)
    , drain_(0)
    , power_(0)
    , bKilled_(0), uKilled_(0), iKilled_(0), aKilled_(0)
    , bLost_(0), uLost_(0), iLost_(0), aLost_(0)
    , bBuilt_(0), uBuilt_(0), iBuilt_(0), aBuilt_(0)
    , harvested_(0)
    , bScan_(0), uScan_(0), iScan_(0), aScan_(0), vScan_(0)
    , difficulty_(DifficultyType::NORMAL)
    , state_(HouseStateType::NONE)
    , alertTimer_(0)
    , aiTimer_(0)
    , buildBuilding_(-1)
    , buildUnit_(-1)
    , buildInfantry_(-1)
    , buildAircraft_(-1)
    , enemy_(HousesType::NONE)
    , lastAttacker_(HousesType::NONE)
    , lastAttackFrame_(0)
    , baseCenter_(0)
    , baseRadius_(0)
{
    memset(urgency_, 0, sizeof(urgency_));
}

HouseClass::HouseClass(HousesType type)
    : HouseClass()
{
    Init(type);
}

void HouseClass::Init(HousesType type) {
    type_ = type;
    isActive_ = true;
    credits_ = 0;
    tiberium_ = 0;

    // Default alliance with self
    allies_ = (1U << static_cast<int>(type));

    // Set initial state
    state_ = HouseStateType::BUILDUP;
    difficulty_ = DifficultyType::NORMAL;

    // Clear urgency
    for (int i = 0; i < static_cast<int>(StrategyType::COUNT); i++) {
        urgency_[i] = UrgencyType::NONE;
    }
}

//===========================================================================
// Type Queries
//===========================================================================

const HouseTypeData* HouseClass::TypeClass() const {
    return GetHouseType(type_);
}

const char* HouseClass::Name() const {
    const HouseTypeData* data = TypeClass();
    return data ? data->fullName : "Unknown";
}

SideType HouseClass::Side() const {
    const HouseTypeData* data = TypeClass();
    return data ? data->side : SideType::NONE;
}

bool HouseClass::IsAllied() const {
    return Side() == SideType::ALLIED;
}

bool HouseClass::IsSoviet() const {
    return Side() == SideType::SOVIET;
}

//===========================================================================
// Alliance Management
//===========================================================================

bool HouseClass::Is_Ally(HousesType house) const {
    if (house == HousesType::NONE) return false;
    return (allies_ & (1U << static_cast<int>(house))) != 0;
}

bool HouseClass::Is_Ally(const HouseClass* house) const {
    if (house == nullptr) return false;
    return Is_Ally(house->type_);
}

void HouseClass::Make_Ally(HousesType house) {
    if (house == HousesType::NONE) return;
    allies_ |= (1U << static_cast<int>(house));
}

void HouseClass::Make_Enemy(HousesType house) {
    if (house == HousesType::NONE) return;
    allies_ &= ~(1U << static_cast<int>(house));
}

//===========================================================================
// Resource Management
//===========================================================================

bool HouseClass::Spend_Money(int amount) {
    if (amount <= 0) return true;

    int available = Available_Money();
    if (available < amount) return false;

    // Spend from credits first, then tiberium
    if (credits_ >= amount) {
        credits_ -= amount;
    } else {
        amount -= credits_;
        credits_ = 0;
        tiberium_ -= amount;
    }
    return true;
}

void HouseClass::Refund_Money(int amount) {
    if (amount <= 0) return;
    credits_ += amount;
}

void HouseClass::Harvest_Tiberium(int amount, int storage) {
    if (amount <= 0) return;

    // Update capacity if storage provided
    if (storage > capacity_) {
        capacity_ = storage;
    }

    // Add to storage (capped by capacity)
    int space = capacity_ - tiberium_;
    if (amount > space) {
        amount = space;
    }
    if (amount > 0) {
        tiberium_ += amount;
        harvested_ += amount;
    }
}

int HouseClass::Power_Fraction() const {
    if (drain_ <= 0) return 256;  // No drain = full power
    if (power_ <= 0) return 0;    // No generation = no power
    if (power_ >= drain_) return 256;  // Surplus

    // Calculate percentage (0-256)
    return (power_ * 256) / drain_;
}

//===========================================================================
// Production Queries
//===========================================================================

bool HouseClass::Can_Build(int type, RTTIType rtti) const {
    // Simplified - would check tech tree, prerequisites
    (void)type;
    (void)rtti;
    return true;
}

int HouseClass::Cost_Of(int type, RTTIType rtti) const {
    // Simplified - would look up cost from type data
    (void)type;
    (void)rtti;
    return 1000;
}

//===========================================================================
// Unit Tracking
//===========================================================================

void HouseClass::Tracking_Add(TechnoClass* object) {
    if (object == nullptr) return;

    RTTIType rtti = object->WhatAmI();
    // Would update bScan_/uScan_/iScan_/aScan_ bitfields
    (void)rtti;
}

void HouseClass::Tracking_Remove(TechnoClass* object) {
    if (object == nullptr) return;
    // Would update scan bitfields
}

//===========================================================================
// AI Processing
//===========================================================================

void HouseClass::AI() {
    if (!isActive_ || isDefeated_) return;

    // Decrement timers
    if (alertTimer_ > 0) {
        alertTimer_--;
        if (alertTimer_ == 0) {
            isAlerted_ = false;
        }
    }

    // AI think timer
    if (aiTimer_ > 0) {
        aiTimer_--;
    } else {
        // Run expert AI every ~5 seconds (300 frames at 60fps)
        if (!isHuman_) {
            Expert_AI();

            // If production is enabled (BEGIN_PROD triggered), decide builds
            if (isStarted_) {
                AI_Unit();
                AI_Infantry();
                AI_Building();
                AI_Aircraft();
            }
        }
        aiTimer_ = 300;
    }

    // Check for state transitions
    if (state_ == HouseStateType::BUILDUP) {
        // Check if we should transition to different state
        int powerFrac = Power_Fraction();
        if (powerFrac < 128) {  // Below 50% power
            // Stay in buildup but prioritize power
        }
    }
}

void HouseClass::Expert_AI() {
    // Evaluate each strategy and assign urgency
    urgency_[static_cast<int>(StrategyType::BUILD_POWER)] = Check_Build_Power();
    int defIdx = static_cast<int>(StrategyType::BUILD_DEFENSE);
    int offIdx = static_cast<int>(StrategyType::BUILD_OFFENSE);
    urgency_[defIdx] = Check_Build_Defense();
    urgency_[offIdx] = Check_Build_Offense();
    urgency_[static_cast<int>(StrategyType::ATTACK)] = Check_Attack();
    urgency_[static_cast<int>(StrategyType::FIRE_SALE)] = Check_Fire_Sale();

    // Select enemy if we don't have one
    if (enemy_ == HousesType::NONE) {
        enemy_ = Find_Enemy();
    }

    // Execute highest urgency strategies first
    int critical = static_cast<int>(UrgencyType::CRITICAL);
    for (int level = critical; level > 0; level--) {
        for (int s = 0; s < static_cast<int>(StrategyType::COUNT); s++) {
            if (urgency_[s] == static_cast<UrgencyType>(level)) {
                // Would execute strategy here
                // E.g., build suggested unit, launch attack, etc.
            }
        }
    }
}

UrgencyType HouseClass::Check_Build_Power() const {
    int powerFrac = Power_Fraction();

    if (powerFrac < 64) return UrgencyType::CRITICAL;   // Below 25%
    if (powerFrac < 128) return UrgencyType::HIGH;      // Below 50%
    if (powerFrac < 192) return UrgencyType::MEDIUM;    // Below 75%
    if (powerFrac < 240) return UrgencyType::LOW;       // Below 94%
    return UrgencyType::NONE;
}

UrgencyType HouseClass::Check_Build_Defense() const {
    // Simplified - would check if base is under threat
    if (isAlerted_) return UrgencyType::HIGH;
    return UrgencyType::LOW;
}

UrgencyType HouseClass::Check_Build_Offense() const {
    // Simplified - check if we have enough offensive units
    return UrgencyType::MEDIUM;
}

UrgencyType HouseClass::Check_Attack() const {
    // Attack is critical after game has been running a while
    if (state_ == HouseStateType::ENDGAME) return UrgencyType::CRITICAL;
    if (isAlerted_) return UrgencyType::HIGH;
    return UrgencyType::MEDIUM;
}

UrgencyType HouseClass::Check_Fire_Sale() const {
    // Fire sale when we have no production capability
    if (bScan_ == 0) return UrgencyType::CRITICAL;
    return UrgencyType::NONE;
}

HousesType HouseClass::Find_Enemy() const {
    // Find closest active enemy house
    HousesType best = HousesType::NONE;
    int bestScore = 0;

    for (int i = 0; i < HOUSE_MAX; i++) {
        HousesType candidate = static_cast<HousesType>(i);
        if (candidate == type_) continue;
        if (Is_Ally(candidate)) continue;

        const HouseClass* house = &Houses[i];
        if (!house->isActive_ || house->isDefeated_) continue;

        // Score based on various factors
        int score = 100;

        // Bonus for last attacker
        if (candidate == lastAttacker_) {
            score += 50;
        }

        // Penalty for same side (less likely to attack)
        if (house->Side() == Side()) {
            score -= 30;
        }

        if (score > bestScore) {
            bestScore = score;
            best = candidate;
        }
    }

    return best;
}

int32_t HouseClass::Find_Cell_In_Zone(int zone) const {
    // Would search for cell in specified threat zone
    (void)zone;
    return baseCenter_;
}

//===========================================================================
// AI Production Functions
//
// These functions determine what the AI should build next, based on:
// - Team requirements (from mission TeamTypes)
// - Current unit counts
// - Available money
//
// Returns number of game ticks until function should be called again.
//===========================================================================

// TICKS_PER_SECOND equivalent (60 fps)
static constexpr int TICKS_PER_SECOND = 60;

int HouseClass::AI_Unit() {
    // If already building a unit, wait
    if (buildUnit_ >= 0) return TICKS_PER_SECOND;

    // Check team requirements for units we need to build
    // Scan through team types looking for units we're short on
    int counter[static_cast<int>(UnitType::COUNT)];
    memset(counter, 0, sizeof(counter));

    // Build a list of units needed for teams
    for (int i = 0; i < TEAMTYPE_MAX; i++) {
        TeamTypeClass* team = &TeamTypes[i];
        if (!team->isActive_) continue;
        if (team->house_ != type_) continue;

        // Check if this team type needs units
        bool needsUnits = team->isPrebuilt_ || team->isReinforcable_;
        if (!needsUnits) continue;

        // Only consider prebuilt teams or teams we're alerted about
        if (team->isAutocreate_ && !isAlerted_) continue;

        // Count needed units from team member specs
        for (int m = 0; m < team->memberCount_; m++) {
            if (team->members_[m].type == RTTIType::UNIT) {
                int typeIdx = team->members_[m].typeIndex;
                if (typeIdx >= 0 &&
                    typeIdx < static_cast<int>(UnitType::COUNT)) {
                    counter[typeIdx] += team->members_[m].count;
                }
            }
        }
    }

    // Pick the most needed unit type we can afford
    int bestVal = -1;
    int bestCount = 0;
    UnitType bestList[static_cast<int>(UnitType::COUNT)];

    for (int ut = 0; ut < static_cast<int>(UnitType::COUNT); ut++) {
        if (counter[ut] > 0) {
            const UnitTypeData* utype = GetUnitType(static_cast<UnitType>(ut));
            if (utype && utype->cost <= Available_Money()) {
                if (bestVal == -1 || counter[ut] > bestVal) {
                    bestVal = counter[ut];
                    bestCount = 0;
                }
                if (counter[ut] == bestVal) {
                    bestList[bestCount++] = static_cast<UnitType>(ut);
                }
            }
        }
    }

    // Randomly pick from equally-needed units
    if (bestCount > 0) {
        buildUnit_ = static_cast<int8_t>(bestList[rand() % bestCount]);
        fprintf(stderr, "AI_Unit: House %s queued unit type %d\n",
                Name(), buildUnit_);
    }

    return TICKS_PER_SECOND;
}

int HouseClass::AI_Infantry() {
    // If already building infantry, wait
    if (buildInfantry_ >= 0) return TICKS_PER_SECOND;

    // Check team requirements for infantry we need to build
    int counter[static_cast<int>(InfantryType::COUNT)];
    memset(counter, 0, sizeof(counter));

    // Build a list of infantry needed for teams
    for (int i = 0; i < TEAMTYPE_MAX; i++) {
        TeamTypeClass* team = &TeamTypes[i];
        if (!team->isActive_) continue;
        if (team->house_ != type_) continue;

        bool needsInf = team->isPrebuilt_ || team->isReinforcable_;
        if (!needsInf) continue;

        if (team->isAutocreate_ && !isAlerted_) continue;

        // Count needed infantry from team member specs
        for (int m = 0; m < team->memberCount_; m++) {
            if (team->members_[m].type == RTTIType::INFANTRY) {
                int typeIdx = team->members_[m].typeIndex;
                if (typeIdx >= 0 &&
                    typeIdx < static_cast<int>(InfantryType::COUNT)) {
                    counter[typeIdx] += team->members_[m].count;
                }
            }
        }
    }

    // Pick the most needed infantry type we can afford
    int bestVal = -1;
    int bestCount = 0;
    InfantryType bestList[static_cast<int>(InfantryType::COUNT)];

    for (int it = 0; it < static_cast<int>(InfantryType::COUNT); it++) {
        if (counter[it] > 0) {
            const InfantryTypeData* itype =
                GetInfantryType(static_cast<InfantryType>(it));
            if (itype && itype->cost <= Available_Money()) {
                if (bestVal == -1 || counter[it] > bestVal) {
                    bestVal = counter[it];
                    bestCount = 0;
                }
                if (counter[it] == bestVal) {
                    bestList[bestCount++] = static_cast<InfantryType>(it);
                }
            }
        }
    }

    // Randomly pick from equally-needed infantry
    if (bestCount > 0) {
        buildInfantry_ = static_cast<int8_t>(bestList[rand() % bestCount]);
        fprintf(stderr, "AI_Infantry: House %s queued infantry type %d\n",
                Name(), buildInfantry_);
    }

    return TICKS_PER_SECOND;
}

int HouseClass::AI_Building() {
    // If already building a building, wait
    if (buildBuilding_ >= 0) return TICKS_PER_SECOND;

    // AI building logic would go here
    // For campaign missions, AI typically doesn't build new structures
    // (they start with a base already)

    return TICKS_PER_SECOND;
}

int HouseClass::AI_Aircraft() {
    // If already building aircraft, wait
    if (buildAircraft_ >= 0) return TICKS_PER_SECOND;

    // Similar to AI_Unit but for aircraft
    // Not implemented yet as aircraft aren't fully supported

    return TICKS_PER_SECOND;
}

const TechnoTypeClass* HouseClass::Suggest_New_Object(RTTIType rtti) const {
    // Return the suggested object based on what AI decided to build
    switch (rtti) {
        case RTTIType::INFANTRY:
            if (buildInfantry_ >= 0) {
                // Return pointer to infantry type data
                // (TechnoTypeClass is a base class for type data)
                return reinterpret_cast<const TechnoTypeClass*>(
                    GetInfantryType(
                        static_cast<InfantryType>(buildInfantry_)));
            }
            break;

        case RTTIType::UNIT:
            if (buildUnit_ >= 0) {
                return reinterpret_cast<const TechnoTypeClass*>(
                    GetUnitType(static_cast<UnitType>(buildUnit_)));
            }
            break;

        case RTTIType::BUILDING:
            if (buildBuilding_ >= 0) {
                return reinterpret_cast<const TechnoTypeClass*>(
                    GetBuildingType(
                        static_cast<BuildingType>(buildBuilding_)));
            }
            break;

        case RTTIType::AIRCRAFT:
            // Not yet supported
            break;

        default:
            break;
    }

    return nullptr;
}

//===========================================================================
// Team Management
//===========================================================================

TeamTypeClass* HouseClass::Suggested_New_Team(bool alert) const {
    HouseClass* self = const_cast<HouseClass*>(this);
    return TeamTypeClass::Suggested_New_Team(self, alert);
}

int HouseClass::Team_Count() const {
    // Would count active teams owned by this house
    return 0;
}

void HouseClass::Recruit(TechnoClass* object) {
    // Would assign object to a team if appropriate
    (void)object;
}

//===========================================================================
// Combat Callbacks
//===========================================================================

void HouseClass::Attacked(TechnoClass* source) {
    if (source == nullptr) return;

    // Record attacker
    lastAttacker_ = source->Owner();
    lastAttackFrame_ = 0;  // Would use game frame

    // Enter alert state
    isAlerted_ = true;
    alertTimer_ = 900;  // 15 seconds at 60fps

    // Upgrade state if needed
    if (state_ == HouseStateType::BUILDUP) {
        state_ = HouseStateType::THREATENED;
    }
}

void HouseClass::Destroyed(TechnoClass* object) {
    if (object == nullptr) return;

    // Update lost counters based on type
    RTTIType rtti = object->WhatAmI();
    switch (rtti) {
        case RTTIType::BUILDING:
            bLost_++;
            break;
        case RTTIType::UNIT:
            uLost_++;
            break;
        case RTTIType::INFANTRY:
            iLost_++;
            break;
        case RTTIType::AIRCRAFT:
            aLost_++;
            break;
        default:
            break;
    }

    // Remove from tracking
    Tracking_Remove(object);
}

void HouseClass::Killed(TechnoClass* object) {
    if (object == nullptr) return;

    // Update killed counters based on type
    RTTIType rtti = object->WhatAmI();
    switch (rtti) {
        case RTTIType::BUILDING:
            bKilled_++;
            break;
        case RTTIType::UNIT:
            uKilled_++;
            break;
        case RTTIType::INFANTRY:
            iKilled_++;
            break;
        case RTTIType::AIRCRAFT:
            aKilled_++;
            break;
        default:
            break;
    }
}

//===========================================================================
// Static Methods
//===========================================================================

HouseClass* HouseClass::As_Pointer(HousesType type) {
    int idx = static_cast<int>(type);
    if (idx >= 0 && idx < HOUSE_MAX) {
        return &Houses[idx];
    }
    return nullptr;
}

//===========================================================================
// Global Helper Functions
//===========================================================================

HouseClass* Find_House(HousesType type) {
    return HouseClass::As_Pointer(type);
}

void Init_Houses() {
    HouseCount = 0;
    PlayerPtr = nullptr;

    for (int i = 0; i < HOUSE_MAX; i++) {
        Houses[i] = HouseClass();
    }
}

//===========================================================================
// Bridge function for mission.cpp (which has conflicting types)
//===========================================================================

void EnableAIProduction(int houseIndex) {
    if (houseIndex < 0 || houseIndex >= HOUSE_MAX) return;

    HousesType htype = static_cast<HousesType>(houseIndex);
    HouseClass* house = HouseClass::As_Pointer(htype);
    if (house && !house->isHuman_) {
        house->Begin_Production();
        fprintf(stderr, "EnableAIProduction: House %s production enabled\n",
                house->Name());
    }
}

void EnableAIAutocreate(int houseIndex) {
    if (houseIndex < 0 || houseIndex >= HOUSE_MAX) return;

    HousesType htype = static_cast<HousesType>(houseIndex);
    HouseClass* house = HouseClass::As_Pointer(htype);
    if (house && !house->isHuman_) {
        house->isAlerted_ = true;
        fprintf(stderr, "EnableAIAutocreate: House %s autocreate enabled\n",
                house->Name());
    }
}
