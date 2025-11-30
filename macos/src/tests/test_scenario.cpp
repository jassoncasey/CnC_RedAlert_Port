/**
 * Red Alert macOS Port - Scenario & Trigger System Tests
 *
 * Tests for ScenarioClass, TriggerTypeClass, and TriggerClass
 */

#include "game/scenario.h"
#include "game/trigger.h"
#include "game/house.h"
#include "game/team.h"
#include <cassert>
#include <cstdio>
#include <cstring>

// Test counters
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name()
#define RUN_TEST(name) do { \
    printf("  Testing %s... ", #name); \
    test_##name(); \
    printf("OK\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAIL\n    Assertion failed: %s\n    at %s:%d\n", \
               #cond, __FILE__, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAIL\n    Expected %s == %s\n    at %s:%d\n", \
               #a, #b, __FILE__, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_STREQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        printf("FAIL\n    Expected \"%s\" == \"%s\"\n    at %s:%d\n", \
               (a), (b), __FILE__, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

//===========================================================================
// Theater Tests
//===========================================================================

TEST(theater_names) {
    ASSERT_STREQ(TheaterName(TheaterType::TEMPERATE), "TEMPERATE");
    ASSERT_STREQ(TheaterName(TheaterType::SNOW), "SNOW");
    ASSERT_STREQ(TheaterName(TheaterType::INTERIOR), "INTERIOR");
}

TEST(theater_from_name) {
    ASSERT_EQ(TheaterFromName("TEMPERATE"), TheaterType::TEMPERATE);
    ASSERT_EQ(TheaterFromName("SNOW"), TheaterType::SNOW);
    ASSERT_EQ(TheaterFromName("INTERIOR"), TheaterType::INTERIOR);
    ASSERT_EQ(TheaterFromName("TEMP"), TheaterType::TEMPERATE);
    ASSERT_EQ(TheaterFromName("SNO"), TheaterType::SNOW);
    ASSERT_EQ(TheaterFromName("invalid"), TheaterType::TEMPERATE);
}

//===========================================================================
// ScenarioClass Construction Tests
//===========================================================================

TEST(scenario_construction) {
    ScenarioClass scen;
    scen.Init();

    ASSERT_EQ(scen.scenario_, 1);
    ASSERT_EQ(scen.theater_, TheaterType::TEMPERATE);
    ASSERT_EQ(scen.playerHouse_, HousesType::GREECE);
    ASSERT_EQ(scen.difficulty_, DifficultyType::NORMAL);
    ASSERT_EQ(scen.missionTimer_, -1);
    ASSERT(!scen.isEndOfGame_);
}

TEST(scenario_clear) {
    ScenarioClass scen;
    scen.scenario_ = 5;
    scen.theater_ = TheaterType::SNOW;
    scen.isEndOfGame_ = true;

    scen.Clear();

    ASSERT_EQ(scen.scenario_, 1);
    ASSERT_EQ(scen.theater_, TheaterType::TEMPERATE);
    ASSERT(!scen.isEndOfGame_);
}

//===========================================================================
// Waypoint Tests
//===========================================================================

TEST(waypoint_set_get) {
    ScenarioClass scen;
    scen.Init();

    scen.Set_Waypoint(0, 100);
    scen.Set_Waypoint(1, 200);
    scen.Set_Waypoint(25, 500);

    ASSERT_EQ(scen.Get_Waypoint(0), 100);
    ASSERT_EQ(scen.Get_Waypoint(1), 200);
    ASSERT_EQ(scen.Get_Waypoint(25), 500);
    ASSERT_EQ(scen.Get_Waypoint(50), -1);  // Invalid index
}

TEST(waypoint_letter) {
    ScenarioClass scen;
    scen.Init();

    scen.Set_Waypoint(0, 111);   // A
    scen.Set_Waypoint(1, 222);   // B
    scen.Set_Waypoint(25, 999);  // Z

    ASSERT_EQ(scen.Get_Waypoint_Cell('A'), 111);
    ASSERT_EQ(scen.Get_Waypoint_Cell('B'), 222);
    ASSERT_EQ(scen.Get_Waypoint_Cell('Z'), 999);
    ASSERT_EQ(scen.Get_Waypoint_Cell('a'), 111);  // lowercase
    ASSERT_EQ(scen.Get_Waypoint_Cell('1'), -1);   // Invalid
}

//===========================================================================
// Global Flag Tests
//===========================================================================

TEST(global_flags) {
    ScenarioClass scen;
    scen.Init();

    // Initially all false
    for (int i = 0; i < GLOBAL_FLAG_COUNT; i++) {
        ASSERT(!scen.Get_Global(i));
    }

    scen.Set_Global(0, true);
    scen.Set_Global(15, true);

    ASSERT(scen.Get_Global(0));
    ASSERT(!scen.Get_Global(1));
    ASSERT(scen.Get_Global(15));

    scen.Set_Global(0, false);
    ASSERT(!scen.Get_Global(0));
}

TEST(global_flag_bounds) {
    ScenarioClass scen;
    scen.Init();

    // Out of bounds should be safe
    ASSERT(!scen.Get_Global(-1));
    ASSERT(!scen.Get_Global(100));

    scen.Set_Global(-1, true);  // Should be no-op
    scen.Set_Global(100, true); // Should be no-op
}

//===========================================================================
// Timer Tests
//===========================================================================

TEST(mission_timer) {
    ScenarioClass scen;
    scen.Init();

    ASSERT(!scen.Is_Mission_Timer_Active());

    scen.Start_Mission_Timer(1000);
    ASSERT(scen.Is_Mission_Timer_Active());
    ASSERT_EQ(scen.Get_Mission_Timer(), 1000);

    scen.Add_Mission_Timer(500);
    ASSERT_EQ(scen.Get_Mission_Timer(), 1500);

    scen.Sub_Mission_Timer(200);
    ASSERT_EQ(scen.Get_Mission_Timer(), 1300);

    scen.Stop_Mission_Timer();
    ASSERT(!scen.Is_Mission_Timer_Active());
}

TEST(timer_underflow) {
    ScenarioClass scen;
    scen.Init();

    scen.Start_Mission_Timer(100);
    scen.Sub_Mission_Timer(500);  // More than timer

    ASSERT_EQ(scen.Get_Mission_Timer(), 0);
}

//===========================================================================
// Scenario Filename Tests
//===========================================================================

TEST(scenario_filename) {
    const char* allied = Scenario_Filename(1, TheaterType::TEMPERATE, SideType::ALLIED, false);
    ASSERT_STREQ(allied, "SCG01E.INI");

    const char* soviet = Scenario_Filename(5, TheaterType::SNOW, SideType::SOVIET, false);
    ASSERT_STREQ(soviet, "SCU05W.INI");

    const char* aftermath = Scenario_Filename(3, TheaterType::INTERIOR, SideType::ALLIED, true);
    ASSERT_STREQ(aftermath, "SCG03IA.INI");
}

//===========================================================================
// TEventClass Tests
//===========================================================================

TEST(event_construction) {
    TEventClass event;
    ASSERT_EQ(event.event, TEventType::NONE);
    ASSERT_EQ(event.teamIndex, -1);
}

TEST(event_needs) {
    ASSERT_EQ(TEventClass::Event_Needs(TEventType::CREDITS), EventNeedType::NEED_NUMBER);
    ASSERT_EQ(TEventClass::Event_Needs(TEventType::TIME), EventNeedType::NEED_NUMBER);
    ASSERT_EQ(TEventClass::Event_Needs(TEventType::BUILD), EventNeedType::NEED_STRUCTURE);
    ASSERT_EQ(TEventClass::Event_Needs(TEventType::BUILD_UNIT), EventNeedType::NEED_UNIT);
    ASSERT_EQ(TEventClass::Event_Needs(TEventType::LEAVES_MAP), EventNeedType::NEED_TEAM);
    ASSERT_EQ(TEventClass::Event_Needs(TEventType::DESTROYED), EventNeedType::NEED_NONE);
}

TEST(event_attachment) {
    ASSERT_EQ(TEventClass::Attaches_To(TEventType::PLAYER_ENTERED), AttachType::CELL);
    ASSERT_EQ(TEventClass::Attaches_To(TEventType::DESTROYED), AttachType::OBJECT);
    ASSERT_EQ(TEventClass::Attaches_To(TEventType::CREDITS), AttachType::HOUSE);
    ASSERT_EQ(TEventClass::Attaches_To(TEventType::TIME), AttachType::GENERAL);
    ASSERT_EQ(TEventClass::Attaches_To(TEventType::LEAVES_MAP), AttachType::TEAM);
}

//===========================================================================
// TActionClass Tests
//===========================================================================

TEST(action_construction) {
    TActionClass action;
    ASSERT_EQ(action.action, TActionType::NONE);
    ASSERT_EQ(action.teamIndex, -1);
    ASSERT_EQ(action.triggerIndex, -1);
}

//===========================================================================
// TriggerTypeClass Tests
//===========================================================================

TEST(triggertype_construction) {
    TriggerTypeClass type;
    type.Init();

    ASSERT(!type.isActive_);
    ASSERT_EQ(type.persistence_, PersistantType::VOLATILE);
    ASSERT_EQ(type.house_, HousesType::NONE);
    ASSERT_EQ(type.eventControl_, MultiStyleType::ONLY);
    ASSERT_EQ(type.actionControl_, MultiStyleType::ONLY);
}

TEST(triggertype_name_lookup) {
    Init_TriggerTypes();

    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    strcpy(TriggerTypes[0].name_, "TestTrigger");

    TriggerTypeClass* found = TriggerTypeClass::From_Name("TestTrigger");
    ASSERT(found != nullptr);
    ASSERT(found == &TriggerTypes[0]);

    TriggerTypeClass* notFound = TriggerTypeClass::From_Name("NonExistent");
    ASSERT(notFound == nullptr);
}

TEST(triggertype_attachment) {
    TriggerTypeClass type;
    type.Init();
    type.event1_.event = TEventType::DESTROYED;
    type.eventControl_ = MultiStyleType::ONLY;

    ASSERT_EQ(type.Attaches_To(), AttachType::OBJECT);

    // With second event
    type.event2_.event = TEventType::TIME;
    type.eventControl_ = MultiStyleType::AND;

    AttachType combined = type.Attaches_To();
    // Should combine OBJECT and GENERAL
    ASSERT((static_cast<uint8_t>(combined) & static_cast<uint8_t>(AttachType::OBJECT)) != 0);
    ASSERT((static_cast<uint8_t>(combined) & static_cast<uint8_t>(AttachType::GENERAL)) != 0);
}

//===========================================================================
// TriggerClass Tests
//===========================================================================

TEST(trigger_construction) {
    Init_Triggers();

    TriggerTypeClass type;
    type.Init();
    type.isActive_ = true;
    type.house_ = HousesType::USSR;

    TriggerClass trigger;
    trigger.Init(&type);

    ASSERT(trigger.isActive_);
    ASSERT_EQ(trigger.typeClass_, &type);
    ASSERT_EQ(trigger.House(), HousesType::USSR);
}

TEST(trigger_volatile) {
    Init_TriggerTypes();
    Init_Triggers();

    // Setup volatile trigger
    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    TriggerTypes[0].persistence_ = PersistantType::VOLATILE;
    TriggerTypes[0].event1_.event = TEventType::ANY;
    TriggerTypes[0].action1_.action = TActionType::NONE;
    strcpy(TriggerTypes[0].name_, "VolatileTest");

    TriggerClass* trigger = Create_Trigger(&TriggerTypes[0]);
    ASSERT(trigger != nullptr);
    ASSERT(trigger->isActive_);

    // Spring should destroy volatile trigger
    trigger->Spring(TEventType::DESTROYED, nullptr, -1);
    ASSERT(!trigger->isActive_);
}

TEST(trigger_persistent) {
    Init_TriggerTypes();
    Init_Triggers();

    // Setup persistent trigger
    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    TriggerTypes[0].persistence_ = PersistantType::PERSISTANT;
    TriggerTypes[0].event1_.event = TEventType::ANY;
    TriggerTypes[0].action1_.action = TActionType::NONE;

    TriggerClass* trigger = Create_Trigger(&TriggerTypes[0]);
    ASSERT(trigger != nullptr);

    // Spring should NOT destroy persistent trigger
    trigger->Spring(TEventType::DESTROYED, nullptr, -1);
    ASSERT(trigger->isActive_);

    // Should be able to trigger again
    trigger->Spring(TEventType::ATTACKED, nullptr, -1);
    ASSERT(trigger->isActive_);
}

TEST(trigger_and_logic) {
    Init_TriggerTypes();
    Init_Triggers();

    // Setup AND trigger (both events must occur)
    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    TriggerTypes[0].persistence_ = PersistantType::PERSISTANT;
    TriggerTypes[0].event1_.event = TEventType::DESTROYED;
    TriggerTypes[0].event2_.event = TEventType::TIME;
    TriggerTypes[0].eventControl_ = MultiStyleType::AND;
    TriggerTypes[0].action1_.action = TActionType::NONE;

    TriggerClass* trigger = Create_Trigger(&TriggerTypes[0]);
    ASSERT(trigger != nullptr);

    // First event alone shouldn't fully trigger (returns false)
    ASSERT(!trigger->event1State_.isTripped);
    bool result1 = trigger->Spring(TEventType::DESTROYED, nullptr, -1);
    ASSERT(!result1);  // AND requires both, so first alone doesn't fire
    ASSERT(trigger->event1State_.isTripped);

    // Second event completes the trigger (returns true)
    bool result2 = trigger->Spring(TEventType::TIME, nullptr, -1);
    ASSERT(result2);  // Now both events occurred, trigger fires
    // After PERSISTANT trigger fires, states are reset
    ASSERT(!trigger->event1State_.isTripped);  // Reset after fire
}

TEST(trigger_or_logic) {
    Init_TriggerTypes();
    Init_Triggers();

    // Setup OR trigger (either event triggers)
    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    TriggerTypes[0].persistence_ = PersistantType::PERSISTANT;
    TriggerTypes[0].event1_.event = TEventType::DESTROYED;
    TriggerTypes[0].event2_.event = TEventType::ATTACKED;
    TriggerTypes[0].eventControl_ = MultiStyleType::OR;
    TriggerTypes[0].action1_.action = TActionType::NONE;

    TriggerClass* trigger = Create_Trigger(&TriggerTypes[0]);
    ASSERT(trigger != nullptr);

    // Either event should trigger - event2 match fires immediately
    bool result = trigger->Spring(TEventType::ATTACKED, nullptr, -1);
    ASSERT(result);  // OR means either event fires immediately
    // After PERSISTANT trigger fires, states are reset
    ASSERT(!trigger->event2State_.isTripped);  // Reset after fire
}

//===========================================================================
// Integration Tests
//===========================================================================

TEST(trigger_win_action) {
    Init_TriggerTypes();
    Init_Triggers();
    Init_Houses();

    // Setup player house
    Scen.Init();
    Scen.playerHouse_ = HousesType::GREECE;
    Houses[static_cast<int>(HousesType::GREECE)].Init(HousesType::GREECE);
    Houses[static_cast<int>(HousesType::GREECE)].isActive_ = true;
    HouseCount = 1;

    // Setup win trigger
    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    TriggerTypes[0].house_ = HousesType::GREECE;
    TriggerTypes[0].event1_.event = TEventType::ANY;
    TriggerTypes[0].action1_.action = TActionType::WIN;

    TriggerClass* trigger = Create_Trigger(&TriggerTypes[0]);

    // Before trigger
    ASSERT(!Houses[static_cast<int>(HousesType::GREECE)].isToWin_);

    // Trigger win
    trigger->Spring(TEventType::DESTROYED, nullptr, -1, true);

    // After trigger
    ASSERT(Houses[static_cast<int>(HousesType::GREECE)].isToWin_);
}

TEST(global_flag_trigger) {
    Init_TriggerTypes();
    Init_Triggers();
    Init_Houses();

    Scen.Init();

    // Setup trigger that listens for global flag 5
    TriggerTypes[0].Init();
    TriggerTypes[0].isActive_ = true;
    TriggerTypes[0].persistence_ = PersistantType::PERSISTANT;
    TriggerTypes[0].event1_.event = TEventType::GLOBAL_SET;
    TriggerTypes[0].event1_.data.value = 5;
    TriggerTypes[0].action1_.action = TActionType::NONE;

    TriggerClass* trigger = Create_Trigger(&TriggerTypes[0]);
    ASSERT(!trigger->event1State_.isTripped);

    // Setting global 5 should trigger the event
    Scen.Set_Global(5, true);
    // Note: In full implementation, this would call Process_Triggers
}

TEST(scenario_ai_timer) {
    Init_Triggers();
    Scen.Init();

    Scen.Start_Mission_Timer(5);
    ASSERT_EQ(Scen.Get_Mission_Timer(), 5);

    // Simulate AI ticks
    Scen.AI();
    ASSERT_EQ(Scen.Get_Mission_Timer(), 4);

    Scen.AI();
    Scen.AI();
    Scen.AI();
    ASSERT_EQ(Scen.Get_Mission_Timer(), 1);

    Scen.AI();  // Timer reaches 0, would trigger MISSION_TIMER_EXPIRED
    ASSERT_EQ(Scen.Get_Mission_Timer(), 0);
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Red Alert Scenario & Trigger System Tests\n");
    printf("=========================================\n\n");

    // Initialize global arrays
    Init_TriggerTypes();
    Init_Triggers();
    Init_TeamTypes();
    Init_Teams();
    Init_Houses();
    Scen.Init();

    printf("Theater Tests:\n");
    RUN_TEST(theater_names);
    RUN_TEST(theater_from_name);

    printf("\nScenarioClass Tests:\n");
    RUN_TEST(scenario_construction);
    RUN_TEST(scenario_clear);

    printf("\nWaypoint Tests:\n");
    RUN_TEST(waypoint_set_get);
    RUN_TEST(waypoint_letter);

    printf("\nGlobal Flag Tests:\n");
    RUN_TEST(global_flags);
    RUN_TEST(global_flag_bounds);

    printf("\nTimer Tests:\n");
    RUN_TEST(mission_timer);
    RUN_TEST(timer_underflow);

    printf("\nScenario Filename Tests:\n");
    RUN_TEST(scenario_filename);

    printf("\nTEventClass Tests:\n");
    RUN_TEST(event_construction);
    RUN_TEST(event_needs);
    RUN_TEST(event_attachment);

    printf("\nTActionClass Tests:\n");
    RUN_TEST(action_construction);

    printf("\nTriggerTypeClass Tests:\n");
    RUN_TEST(triggertype_construction);
    RUN_TEST(triggertype_name_lookup);
    RUN_TEST(triggertype_attachment);

    printf("\nTriggerClass Tests:\n");
    RUN_TEST(trigger_construction);
    RUN_TEST(trigger_volatile);
    RUN_TEST(trigger_persistent);
    RUN_TEST(trigger_and_logic);
    RUN_TEST(trigger_or_logic);

    printf("\nIntegration Tests:\n");
    RUN_TEST(trigger_win_action);
    RUN_TEST(global_flag_trigger);
    RUN_TEST(scenario_ai_timer);

    printf("\n=========================================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("=========================================\n");

    return tests_failed > 0 ? 1 : 0;
}
