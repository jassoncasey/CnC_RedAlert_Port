/**
 * Red Alert macOS Port - Factory Implementation
 *
 * Based on original FACTORY.CPP (~600 lines)
 */

#include "factory.h"
#include "house.h"
#include "infantry_types.h"
#include "unit_types.h"
#include "building_types.h"
#include "aircraft_types.h"
#include <algorithm>
#include <cstring>

//===========================================================================
// Global Factory Array
//===========================================================================

FactoryClass Factories[FACTORY_MAX];
int FactoryCount = 0;

//===========================================================================
// FactoryClass Implementation
//===========================================================================

FactoryClass::FactoryClass() {
    Init();
}

FactoryClass::~FactoryClass() {
    // Don't delete object_ - it's either completed or abandoned
}

void FactoryClass::Init() {
    id_ = -1;
    isActive_ = false;

    productionType_ = RTTIType::NONE;
    productionId_ = -1;
    object_ = nullptr;
    specialItem_ = SpecialWeaponType::SPC_NONE;

    balance_ = 0;
    originalBalance_ = 0;

    stage_ = 0;
    rate_ = 1;
    ticksRemaining_ = 0;

    house_ = nullptr;

    isSuspended_ = false;
    isDifferent_ = false;
    hasCompleted_ = false;

    // Clear queue
    for (int i = 0; i < QUEUE_MAX; i++) {
        queue_[i].clear();
    }
    queueCount_ = 0;
}

bool FactoryClass::Set(RTTIType type, int id, HouseClass* house) {
    if (!house) return false;

    Init();

    productionType_ = type;
    productionId_ = id;
    house_ = house;
    specialItem_ = SpecialWeaponType::SPC_NONE;

    // Get the cost based on type
    int cost = 0;
    int buildTime = 100;  // Base build time in ticks

    switch (type) {
        case RTTIType::INFANTRY:
            if (id >= 0 && id < static_cast<int>(InfantryType::COUNT)) {
                auto inf = GetInfantryType(static_cast<InfantryType>(id));
                if (inf) {
                    cost = inf->cost;
                    // Default build time - would come from RULES.INI
                    buildTime = 100;
                }
            }
            break;

        case RTTIType::UNIT:
            if (id >= 0 && id < static_cast<int>(UnitType::COUNT)) {
                auto unit = GetUnitType(static_cast<UnitType>(id));
                if (unit) {
                    cost = unit->cost;
                    buildTime = 100;
                }
            }
            break;

        case RTTIType::BUILDING:
            if (id >= 0 && id < static_cast<int>(BuildingType::COUNT)) {
                auto bld = GetBuildingType(static_cast<BuildingType>(id));
                if (bld) {
                    cost = bld->cost;
                    buildTime = 100;
                }
            }
            break;

        case RTTIType::AIRCRAFT:
            if (id >= 0 && id < static_cast<int>(AircraftType::COUNT)) {
                const AircraftTypeData& air = AircraftTypes[id];
                cost = air.cost;
                buildTime = 100;
            }
            break;

        default:
            return false;
    }

    if (cost <= 0) return false;

    // Apply house cost bias if any
    balance_ = cost;
    originalBalance_ = cost;

    // Calculate production rate
    rate_ = Calculate_Rate(buildTime);

    isActive_ = true;
    return true;
}

bool FactoryClass::Set_Special(SpecialWeaponType special, HouseClass* house) {
    if (!house) return false;
    if (special == SpecialWeaponType::SPC_NONE) return false;

    Init();

    productionType_ = RTTIType::SPECIAL;
    productionId_ = static_cast<int>(special);
    house_ = house;
    specialItem_ = special;

    // Special weapons have fixed recharge times
    // These would come from RULES.INI in the full implementation
    int rechargeTime = 300;  // 5 minutes at 60 ticks/sec

    balance_ = 0;  // Special weapons are free once unlocked
    originalBalance_ = 0;

    rate_ = Calculate_Rate(rechargeTime);

    isActive_ = true;
    return true;
}

int FactoryClass::Calculate_Rate(int baseTime) {
    if (baseTime <= 0) baseTime = 100;

    // Apply power fraction penalty if house has low power
    int time = baseTime;
    if (house_) {
        int powerFraction = house_->Power_Fraction();
        // powerFraction is 0-256 (256 = 100%)
        // Minimum 16 (6.25%), capped at 256 (100%)
        powerFraction = std::max(16, std::min(256, powerFraction));
        // time = baseTime * (256 / powerFraction)
        // Low power = slower production (higher time)
        time = (baseTime * 256) / powerFraction;
    }

    // Divide into STEP_COUNT stages
    int rate = time / STEP_COUNT;

    // Bound to [1, 255]
    return std::max(1, std::min(255, rate));
}

bool FactoryClass::Start() {
    if (!isActive_) return false;
    if (hasCompleted_) return false;

    isSuspended_ = false;
    ticksRemaining_ = rate_;

    // If first start, begin at stage 0
    if (stage_ == 0) {
        isDifferent_ = true;
    }

    return true;
}

bool FactoryClass::Suspend() {
    if (!isActive_) return false;
    if (hasCompleted_) return false;

    isSuspended_ = true;
    return true;
}

bool FactoryClass::Abandon() {
    if (!isActive_) return false;

    // Refund remaining balance to house
    if (house_ && balance_ > 0) {
        house_->Refund_Money(balance_);
    }

    // Clean up object if one was created
    // Note: In the full implementation, this deletes the TechnoClass
    // For now, we don't create objects during production
    object_ = nullptr;

    Init();
    return true;
}

void FactoryClass::AI() {
    if (!isActive_) return;
    if (isSuspended_) return;
    if (hasCompleted_) return;

    // Decrement tick counter
    ticksRemaining_--;
    if (ticksRemaining_ > 0) return;

    // Time for next stage
    ticksRemaining_ = rate_;

    // Calculate cost for this step
    int cost = Cost_Per_Tick();

    // Check if house can afford this step
    if (house_ && cost > 0) {
        if (!house_->Spend_Money(cost)) {
            // Can't afford - reverse one stage (stall production)
            if (stage_ > 0) {
                stage_--;
                isDifferent_ = true;
            }
            return;
        }

        // Deduct from balance
        balance_ -= cost;
        if (balance_ < 0) balance_ = 0;
    }

    // Advance stage
    stage_++;
    isDifferent_ = true;

    // Check for completion
    if (stage_ >= STEP_COUNT) {
        hasCompleted_ = true;

        // Spend any remaining balance
        if (house_ && balance_ > 0) {
            house_->Spend_Money(balance_);
            balance_ = 0;
        }
    }
}

int FactoryClass::Completion() const {
    if (!isActive_) return 0;
    if (hasCompleted_) return 100;

    // Convert stage (0-54) to percentage (0-100)
    return (stage_ * 100) / STEP_COUNT;
}

int FactoryClass::Cost_Per_Tick() const {
    if (!isActive_) return 0;
    if (balance_ <= 0) return 0;

    // Spread remaining cost across remaining stages
    int remainingStages = STEP_COUNT - stage_;
    if (remainingStages <= 0) return balance_;

    return balance_ / remainingStages;
}

bool FactoryClass::Has_Changed() {
    bool changed = isDifferent_;
    isDifferent_ = false;
    return changed;
}

TechnoClass* FactoryClass::Complete() {
    if (!hasCompleted_) return nullptr;

    TechnoClass* obj = object_;
    object_ = nullptr;

    // Advance queue to next item
    Queue_Advance();

    return obj;
}

//===========================================================================
// Queue Management
//===========================================================================

bool FactoryClass::Queue_Add(RTTIType type, int id) {
    // Check if queue is full
    if (queueCount_ >= QUEUE_MAX) return false;

    // Validate type
    if (type == RTTIType::NONE || id < 0) return false;

    // If nothing is currently being produced, start this item
    if (productionType_ == RTTIType::NONE || !isActive_) {
        if (Set(type, id, house_)) {
            Start();
            return true;
        }
        return false;
    }

    // If same type category, add to queue
    // Note: In Red Alert, each factory type has its own queue
    // (barracks for infantry, war factory for vehicles, etc.)
    // For simplicity, we allow queueing items of the same RTTI type

    // Add to queue
    queue_[queueCount_].type = type;
    queue_[queueCount_].id = id;
    queueCount_++;

    return true;
}

bool FactoryClass::Queue_Remove(int index) {
    if (index < 0 || index >= queueCount_) return false;

    // Shift remaining items down
    for (int i = index; i < queueCount_ - 1; i++) {
        queue_[i] = queue_[i + 1];
    }

    // Clear last slot
    queueCount_--;
    queue_[queueCount_].clear();

    return true;
}

const QueueEntry* FactoryClass::Queue_Get(int index) const {
    if (index < 0 || index >= queueCount_) return nullptr;
    return &queue_[index];
}

void FactoryClass::Queue_Advance() {
    // Reset current production state but keep house
    HouseClass* savedHouse = house_;

    productionType_ = RTTIType::NONE;
    productionId_ = -1;
    object_ = nullptr;
    balance_ = 0;
    originalBalance_ = 0;
    stage_ = 0;
    ticksRemaining_ = 0;
    isSuspended_ = false;
    isDifferent_ = false;
    hasCompleted_ = false;

    // Check if there's a queued item
    if (queueCount_ > 0) {
        RTTIType nextType = queue_[0].type;
        int nextId = queue_[0].id;

        // Shift queue
        Queue_Remove(0);

        // Start producing the next item
        if (Set(nextType, nextId, savedHouse)) {
            Start();
        }
    } else {
        // No more items - deactivate factory
        isActive_ = false;
        house_ = nullptr;
    }
}

//===========================================================================
// Global Helper Functions
//===========================================================================

void Init_Factories() {
    for (int i = 0; i < FACTORY_MAX; i++) {
        Factories[i].Init();
        Factories[i].id_ = static_cast<int16_t>(i);
    }
    FactoryCount = 0;
}

FactoryClass* Create_Factory() {
    for (int i = 0; i < FACTORY_MAX; i++) {
        if (!Factories[i].isActive_) {
            Factories[i].Init();
            Factories[i].id_ = static_cast<int16_t>(i);
            Factories[i].isActive_ = true;  // Mark as in use
            FactoryCount++;
            return &Factories[i];
        }
    }
    return nullptr;
}

void Destroy_Factory(FactoryClass* factory) {
    if (!factory) return;
    if (!factory->isActive_) return;

    factory->Abandon();
    factory->isActive_ = false;
    FactoryCount--;
}

FactoryClass* Find_Factory(RTTIType type, int id) {
    for (int i = 0; i < FACTORY_MAX; i++) {
        if (Factories[i].isActive_ &&
            Factories[i].productionType_ == type &&
            Factories[i].productionId_ == id) {
            return &Factories[i];
        }
    }
    return nullptr;
}
