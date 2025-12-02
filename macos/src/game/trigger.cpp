/**
 * Red Alert macOS Port - Trigger System Implementation
 *
 * Based on original TRIGGER.CPP, TRIGTYPE.CPP, TEVENT.CPP, TACTION.CPP
 */

#include "trigger.h"
#include "scenario.h"
#include "house.h"
#include "team.h"
#include "object.h"
#include <cstring>
#include <cstdio>

//===========================================================================
// Global Trigger Arrays
//===========================================================================

TriggerTypeClass TriggerTypes[TRIGGERTYPE_MAX];
TriggerClass Triggers[TRIGGER_MAX];
int TriggerTypeCount = 0;
int TriggerCount = 0;

//===========================================================================
// TEventClass Implementation
//===========================================================================

TEventClass::TEventClass() {
    Reset();
}

void TEventClass::Reset() {
    event = TEventType::NONE;
    teamIndex = -1;
    data.value = 0;
}

EventNeedType TEventClass::Event_Needs(TEventType type) {
    switch (type) {
        case TEventType::CREDITS:
        case TEventType::TIME:
        case TEventType::NBUILDINGS_DESTROYED:
        case TEventType::NUNITS_DESTROYED:
        case TEventType::GLOBAL_SET:
        case TEventType::GLOBAL_CLEAR:
            return EventNeedType::NEED_NUMBER;

        case TEventType::HOUSE_DISCOVERED:
        case TEventType::LOW_POWER:
        case TEventType::UNITS_DESTROYED:
        case TEventType::BUILDINGS_DESTROYED:
        case TEventType::ALL_DESTROYED:
        case TEventType::NOFACTORIES:
            return EventNeedType::NEED_HOUSE;

        case TEventType::BUILD:
        case TEventType::BUILDING_EXISTS:
            return EventNeedType::NEED_STRUCTURE;

        case TEventType::BUILD_UNIT:
            return EventNeedType::NEED_UNIT;

        case TEventType::BUILD_INFANTRY:
            return EventNeedType::NEED_INFANTRY;

        case TEventType::BUILD_AIRCRAFT:
            return EventNeedType::NEED_AIRCRAFT;

        case TEventType::LEAVES_MAP:
            return EventNeedType::NEED_TEAM;

        case TEventType::ENTERS_ZONE:
        case TEventType::CROSS_HORIZONTAL:
        case TEventType::CROSS_VERTICAL:
            return EventNeedType::NEED_WAYPOINT;

        default:
            return EventNeedType::NEED_NONE;
    }
}

AttachType TEventClass::Attaches_To(TEventType type) {
    switch (type) {
        case TEventType::PLAYER_ENTERED:
        case TEventType::ENTERS_ZONE:
        case TEventType::CROSS_HORIZONTAL:
        case TEventType::CROSS_VERTICAL:
            return AttachType::CELL;

        case TEventType::SPIED:
        case TEventType::THIEVED:
        case TEventType::DISCOVERED:
        case TEventType::ATTACKED:
        case TEventType::DESTROYED:
        case TEventType::ANY:
            return AttachType::OBJECT;

        case TEventType::HOUSE_DISCOVERED:
        case TEventType::UNITS_DESTROYED:
        case TEventType::BUILDINGS_DESTROYED:
        case TEventType::ALL_DESTROYED:
        case TEventType::CREDITS:
        case TEventType::NBUILDINGS_DESTROYED:
        case TEventType::NUNITS_DESTROYED:
        case TEventType::NOFACTORIES:
        case TEventType::LOW_POWER:
            return AttachType::HOUSE;

        case TEventType::LEAVES_MAP:
            return AttachType::TEAM;

        case TEventType::TIME:
        case TEventType::MISSION_TIMER_EXPIRED:
        case TEventType::GLOBAL_SET:
        case TEventType::GLOBAL_CLEAR:
        case TEventType::BUILD:
        case TEventType::BUILD_UNIT:
        case TEventType::BUILD_INFANTRY:
        case TEventType::BUILD_AIRCRAFT:
        case TEventType::EVAC_CIVILIAN:
        case TEventType::FAKES_DESTROYED:
        case TEventType::ALL_BRIDGES_DESTROYED:
        case TEventType::BUILDING_EXISTS:
            return AttachType::GENERAL;

        default:
            return AttachType::NONE;
    }
}

//===========================================================================
// TActionClass Implementation
//===========================================================================

TActionClass::TActionClass() {
    Reset();
}

void TActionClass::Reset() {
    action = TActionType::NONE;
    teamIndex = -1;
    triggerIndex = -1;
    data.value = 0;
}

bool TActionClass::Execute(HousesType house, ObjectClass* object,
                           int triggerId, int16_t cell) {
    (void)object;
    (void)triggerId;
    (void)cell;

    switch (action) {
        case TActionType::WIN:
            // Player wins
            if (house == Scen.playerHouse_ || house == HousesType::NONE) {
                HouseClass* player = HouseClass::As_Pointer(Scen.playerHouse_);
                if (player) {
                    player->isToWin_ = true;
                }
            }
            return true;

        case TActionType::LOSE:
            // Player loses
            if (house == Scen.playerHouse_ || house == HousesType::NONE) {
                HouseClass* player = HouseClass::As_Pointer(Scen.playerHouse_);
                if (player) {
                    player->isToLose_ = true;
                }
            }
            return true;

        case TActionType::BEGIN_PRODUCTION:
            // Computer starts building
            {
                HouseClass* housePtr = HouseClass::As_Pointer(house);
                if (housePtr && !housePtr->isHuman_) {
                    housePtr->Begin_Production();
                    fprintf(stderr, "TRIGGER: BEGIN_PRODUCTION for house %s\n",
                            housePtr->Name());
                }
            }
            return true;

        case TActionType::CREATE_TEAM:
            // Create a new team
            if (teamIndex >= 0 && teamIndex < TEAMTYPE_MAX) {
                TriggerTypes[teamIndex].isActive_ = true;
                TeamTypeClass* teamType = &TeamTypes[teamIndex];
                if (teamType->isActive_) {
                    Create_Team(teamType);
                }
            }
            return true;

        case TActionType::DESTROY_TEAM:
            // Destroy all instances of a team type
            if (teamIndex >= 0) {
                for (int i = 0; i < TEAM_MAX; i++) {
                    bool active = Teams[i].isActive_;
                    bool match = Teams[i].typeClass_ == &TeamTypes[teamIndex];
                    if (active && match) {
                        Teams[i].Disband();
                    }
                }
            }
            return true;

        case TActionType::ALL_HUNT:
            // All enemy units hunt
            {
                HouseClass* housePtr = HouseClass::As_Pointer(house);
                if (housePtr) {
                    // Would set all units to hunt mode
                }
            }
            return true;

        case TActionType::REVEAL_ALL:
            // Reveal entire map
            // Would clear shroud from all cells
            return true;

        case TActionType::REVEAL_SOME:
            // Reveal area around cell
            // Would clear shroud in radius
            return true;

        case TActionType::START_TIMER:
            Scen.Start_Mission_Timer(data.value);
            return true;

        case TActionType::STOP_TIMER:
            Scen.Stop_Mission_Timer();
            return true;

        case TActionType::ADD_TIMER:
            Scen.Add_Mission_Timer(data.value);
            return true;

        case TActionType::SUB_TIMER:
            Scen.Sub_Mission_Timer(data.value);
            return true;

        case TActionType::SET_TIMER:
            Scen.Start_Mission_Timer(data.value);
            return true;

        case TActionType::SET_GLOBAL:
            if (data.value >= 0 && data.value < GLOBAL_FLAG_COUNT) {
                Scen.Set_Global(data.value, true);
            }
            return true;

        case TActionType::CLEAR_GLOBAL:
            if (data.value >= 0 && data.value < GLOBAL_FLAG_COUNT) {
                Scen.Set_Global(data.value, false);
            }
            return true;

        case TActionType::FORCE_TRIGGER:
            // Force another trigger
            if (triggerIndex >= 0 && triggerIndex < TRIGGER_MAX) {
                TriggerClass* trig = &Triggers[triggerIndex];
                if (trig->isActive_) {
                    trig->Spring(TEventType::ANY, nullptr, -1, true);
                }
            }
            return true;

        case TActionType::DESTROY_TRIGGER:
            // Destroy a trigger
            if (triggerIndex >= 0 && triggerIndex < TRIGGER_MAX) {
                TriggerClass* trig = &Triggers[triggerIndex];
                if (trig->isActive_) {
                    Destroy_Trigger(trig);
                }
            }
            return true;

        case TActionType::AUTOCREATE:
            // Enable auto-create for AI teams
            {
                HouseClass* housePtr = HouseClass::As_Pointer(house);
                if (housePtr && !housePtr->isHuman_) {
                    // Would enable AI autocreate
                }
            }
            return true;

        case TActionType::FIRE_SALE:
            // Sell all buildings and attack
            {
                HouseClass* housePtr = HouseClass::As_Pointer(house);
                if (housePtr && !housePtr->isHuman_) {
                    // Would trigger fire sale behavior
                }
            }
            return true;

        case TActionType::ALLOWWIN:
            // Allow win condition
            return true;

        case TActionType::TEXT_TRIGGER:
            // Display text message
            // Would show mission text
            return true;

        case TActionType::PLAY_MOVIE:
        case TActionType::PLAY_SOUND:
        case TActionType::PLAY_MUSIC:
        case TActionType::PLAY_SPEECH:
            // Media playback
            // Would play respective media
            return true;

        default:
            return true;
    }
}

//===========================================================================
// TriggerTypeClass Implementation
//===========================================================================

TriggerTypeClass::TriggerTypeClass() {
    Init();
}

void TriggerTypeClass::Init() {
    name_[0] = '\0';
    id_ = -1;
    isActive_ = false;

    persistence_ = PersistantType::VOLATILE;
    house_ = HousesType::NONE;

    event1_.Reset();
    event2_.Reset();
    eventControl_ = MultiStyleType::ONLY;

    action1_.Reset();
    action2_.Reset();
    actionControl_ = MultiStyleType::ONLY;
}

AttachType TriggerTypeClass::Attaches_To() const {
    AttachType result = TEventClass::Attaches_To(event1_.event);

    if (eventControl_ != MultiStyleType::ONLY) {
        // Combine with event2's attachment type
        AttachType attach2 = TEventClass::Attaches_To(event2_.event);
        result = static_cast<AttachType>(
            static_cast<uint8_t>(result) | static_cast<uint8_t>(attach2));
    }

    return result;
}

TriggerClass* TriggerTypeClass::Create_Instance() {
    // Find free slot
    for (int i = 0; i < TRIGGER_MAX; i++) {
        if (!Triggers[i].isActive_) {
            Triggers[i].Init(this);
            TriggerCount++;
            return &Triggers[i];
        }
    }
    return nullptr;
}

TriggerTypeClass* TriggerTypeClass::From_Name(const char* name) {
    if (!name || !name[0]) return nullptr;

    for (int i = 0; i < TRIGGERTYPE_MAX; i++) {
        if (TriggerTypes[i].isActive_ &&
            strcmp(TriggerTypes[i].name_, name) == 0) {
            return &TriggerTypes[i];
        }
    }
    return nullptr;
}

TriggerTypeClass* TriggerTypeClass::From_ID(int id) {
    if (id < 0 || id >= TRIGGERTYPE_MAX) return nullptr;
    if (!TriggerTypes[id].isActive_) return nullptr;
    return &TriggerTypes[id];
}

//===========================================================================
// TriggerClass Implementation
//===========================================================================

TriggerClass::TriggerClass() {
    typeClass_ = nullptr;
    id_ = -1;
    isActive_ = false;
    attachCount_ = 0;
    cell_ = -1;
}

TriggerClass::TriggerClass(TriggerTypeClass* type) {
    Init(type);
}

TriggerClass::~TriggerClass() {
    // Would detach from all objects
}

void TriggerClass::Init(TriggerTypeClass* type) {
    typeClass_ = type;
    isActive_ = true;
    attachCount_ = 0;
    cell_ = -1;

    event1State_.Reset();
    event2State_.Reset();

    // Assign ID
    for (int i = 0; i < TRIGGER_MAX; i++) {
        if (&Triggers[i] == this) {
            id_ = i;
            break;
        }
    }
}

const char* TriggerClass::Name() const {
    return typeClass_ ? typeClass_->Name() : "Unknown";
}

HousesType TriggerClass::House() const {
    return typeClass_ ? typeClass_->house_ : HousesType::NONE;
}

bool TriggerClass::Spring(TEventType event, ObjectClass* object,
                          int16_t cell, bool forced) {
    if (!isActive_ || !typeClass_) return false;

    bool event1Matched = false;
    bool event2Matched = false;

    // Check if event1 matches
    if (forced || typeClass_->event1_.event == event ||
        typeClass_->event1_.event == TEventType::ANY) {
        event1Matched = true;
        event1State_.isTripped = true;
    }

    // Check if event2 matches (if applicable)
    if (typeClass_->eventControl_ != MultiStyleType::ONLY) {
        if (forced || typeClass_->event2_.event == event ||
            typeClass_->event2_.event == TEventType::ANY) {
            event2Matched = true;
            event2State_.isTripped = true;
        }
    }

    // Determine if we should fire based on event control
    bool shouldFire = false;

    switch (typeClass_->eventControl_) {
        case MultiStyleType::ONLY:
            shouldFire = event1Matched;
            break;

        case MultiStyleType::AND:
            shouldFire = event1State_.isTripped && event2State_.isTripped;
            break;

        case MultiStyleType::OR:
            shouldFire = event1State_.isTripped || event2State_.isTripped;
            break;

        case MultiStyleType::LINKED:
            // Fire immediately for matching event
            shouldFire = event1Matched || event2Matched;
            break;
    }

    // Handle semi-persistent triggers
    if (typeClass_->persistence_ == PersistantType::SEMIPERSISTANT) {
        if (attachCount_ > 1) {
            attachCount_--;
            return false;  // Not all attachments have triggered yet
        }
    }

    if (!shouldFire) return false;

    // Execute actions
    HousesType house = typeClass_->house_;
    bool result = false;

    if (typeClass_->eventControl_ == MultiStyleType::LINKED) {
        // Linked mode: execute matching action
        if (event1Matched) {
            result = typeClass_->action1_.Execute(house, object, id_, cell);
        }
        bool notOnly = typeClass_->actionControl_ != MultiStyleType::ONLY;
        if (event2Matched && notOnly) {
            TActionClass& a2 = typeClass_->action2_;
            result = a2.Execute(house, object, id_, cell) || result;
        }
    } else {
        // Normal mode: execute action(s)
        TActionClass& a1 = typeClass_->action1_;
        result = a1.Execute(house, object, id_, cell);

        if (typeClass_->actionControl_ == MultiStyleType::AND) {
            TActionClass& a2 = typeClass_->action2_;
            result = a2.Execute(house, object, id_, cell) && result;
        }
    }

    // Handle persistence
    switch (typeClass_->persistence_) {
        case PersistantType::VOLATILE:
            // Destroy after execution
            Destroy_Trigger(this);
            break;

        case PersistantType::SEMIPERSISTANT:
            // Destroy if no more attachments
            if (attachCount_ <= 1) {
                Destroy_Trigger(this);
            }
            break;

        case PersistantType::PERSISTANT:
            // Reset for next trigger
            event1State_.Reset();
            event2State_.Reset();
            break;
    }

    return result;
}

void TriggerClass::Attach(ObjectClass* object) {
    if (object) {
        attachCount_++;
        // Would set object's trigger pointer
    }
}

void TriggerClass::Detach(ObjectClass* object) {
    if (object && attachCount_ > 0) {
        attachCount_--;
        // Would clear object's trigger pointer
    }
}

void TriggerClass::Attach_To_Cell(int16_t newCell) {
    cell_ = newCell;
    attachCount_++;
    // Would set cell's trigger pointer
}

void TriggerClass::Spring_All(TEventType event, HousesType house) {
    for (int i = 0; i < TRIGGER_MAX; i++) {
        if (Triggers[i].isActive_) {
            TriggerClass* trig = &Triggers[i];
            if (trig->typeClass_ &&
                (trig->typeClass_->house_ == house ||
                 trig->typeClass_->house_ == HousesType::NONE ||
                 house == HousesType::NONE)) {
                trig->Spring(event, nullptr, -1);
            }
        }
    }
}

void TriggerClass::Spring_All(TEventType event, ObjectClass* object) {
    for (int i = 0; i < TRIGGER_MAX; i++) {
        if (Triggers[i].isActive_) {
            Triggers[i].Spring(event, object, -1);
        }
    }
}

//===========================================================================
// Helper Functions
//===========================================================================

void Init_TriggerTypes() {
    for (int i = 0; i < TRIGGERTYPE_MAX; i++) {
        TriggerTypes[i].Init();
    }
    TriggerTypeCount = 0;
}

void Init_Triggers() {
    for (int i = 0; i < TRIGGER_MAX; i++) {
        Triggers[i] = TriggerClass();
    }
    TriggerCount = 0;
}

TriggerClass* Create_Trigger(TriggerTypeClass* type) {
    if (!type) return nullptr;
    return type->Create_Instance();
}

void Destroy_Trigger(TriggerClass* trigger) {
    if (trigger && trigger->isActive_) {
        trigger->isActive_ = false;
        trigger->typeClass_ = nullptr;
        TriggerCount--;
    }
}

void Process_Triggers(TEventType event, HousesType house,
                      ObjectClass* object, int16_t cell) {
    // Process all active triggers for this event
    for (int i = 0; i < TRIGGER_MAX; i++) {
        if (Triggers[i].isActive_) {
            TriggerClass* trig = &Triggers[i];

            // Check house match
            if (house != HousesType::NONE && trig->typeClass_ &&
                trig->typeClass_->house_ != HousesType::NONE &&
                trig->typeClass_->house_ != house) {
                continue;
            }

            // Check cell match
            if (cell >= 0 && trig->cell_ >= 0 && trig->cell_ != cell) {
                continue;
            }

            trig->Spring(event, object, cell);
        }
    }
}
