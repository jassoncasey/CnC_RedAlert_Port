/**
 * Red Alert macOS Port - AI System Tests
 *
 * Tests for HouseClass, TeamTypeClass, and TeamClass
 */

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

//===========================================================================
// HouseTypeData Tests
//===========================================================================

TEST(house_type_data) {
    // Test that we can get house type data for all types
    const HouseTypeData* spain = GetHouseType(HousesType::SPAIN);
    ASSERT(spain != nullptr);
    ASSERT(strcmp(spain->iniName, "Spain") == 0);
    ASSERT(spain->side == SideType::ALLIED);

    const HouseTypeData* ussr = GetHouseType(HousesType::USSR);
    ASSERT(ussr != nullptr);
    ASSERT(strcmp(ussr->iniName, "USSR") == 0);
    ASSERT(ussr->side == SideType::SOVIET);

    const HouseTypeData* greece = GetHouseType(HousesType::GREECE);
    ASSERT(greece != nullptr);
    ASSERT(greece->side == SideType::ALLIED);

    const HouseTypeData* ukraine = GetHouseType(HousesType::UKRAINE);
    ASSERT(ukraine != nullptr);
    ASSERT(ukraine->side == SideType::SOVIET);
}

TEST(house_type_from_name) {
    ASSERT_EQ(HouseTypeFromName("Spain"), HousesType::SPAIN);
    ASSERT_EQ(HouseTypeFromName("USSR"), HousesType::USSR);
    ASSERT_EQ(HouseTypeFromName("Greece"), HousesType::GREECE);
    ASSERT_EQ(HouseTypeFromName("Turkey"), HousesType::TURKEY);
    ASSERT_EQ(HouseTypeFromName("BadGuy"), HousesType::BAD);
    ASSERT_EQ(HouseTypeFromName("InvalidName"), HousesType::NONE);
}

//===========================================================================
// HouseClass Construction Tests
//===========================================================================

TEST(house_construction) {
    HouseClass house;
    house.Init(HousesType::GREECE);

    ASSERT_EQ(house.type_, HousesType::GREECE);
    ASSERT(house.isActive_);
    ASSERT(!house.isDefeated_);
    ASSERT_EQ(house.credits_, 0);
    ASSERT_EQ(house.tiberium_, 0);
}

TEST(house_type_queries) {
    HouseClass house;
    house.Init(HousesType::ENGLAND);

    const HouseTypeData* typeData = house.TypeClass();
    ASSERT(typeData != nullptr);
    ASSERT_EQ(typeData->side, SideType::ALLIED);
    ASSERT(house.IsAllied());
    ASSERT(!house.IsSoviet());

    HouseClass sovietHouse;
    sovietHouse.Init(HousesType::USSR);
    ASSERT(!sovietHouse.IsAllied());
    ASSERT(sovietHouse.IsSoviet());
}

//===========================================================================
// Alliance Tests
//===========================================================================

TEST(alliance_basic) {
    HouseClass allies;
    allies.Init(HousesType::GREECE);

    HouseClass soviets;
    soviets.Init(HousesType::USSR);

    // Initially no alliances
    ASSERT(!allies.Is_Ally(HousesType::USSR));
    ASSERT(!soviets.Is_Ally(HousesType::GREECE));

    // Make alliance
    allies.Make_Ally(HousesType::USSR);
    ASSERT(allies.Is_Ally(HousesType::USSR));

    // Alliance is one-way unless reciprocated
    ASSERT(!soviets.Is_Ally(HousesType::GREECE));

    // Make reciprocal alliance
    soviets.Make_Ally(HousesType::GREECE);
    ASSERT(soviets.Is_Ally(HousesType::GREECE));
}

TEST(alliance_enemy) {
    HouseClass house;
    house.Init(HousesType::SPAIN);

    house.Make_Ally(HousesType::ENGLAND);
    ASSERT(house.Is_Ally(HousesType::ENGLAND));

    // Break alliance
    house.Make_Enemy(HousesType::ENGLAND);
    ASSERT(!house.Is_Ally(HousesType::ENGLAND));
}

//===========================================================================
// Resource Tests
//===========================================================================

TEST(resource_money) {
    HouseClass house;
    house.Init(HousesType::GREECE);

    house.credits_ = 1000;
    house.tiberium_ = 500;

    ASSERT_EQ(house.Available_Money(), 1500);

    // Spend less than available
    ASSERT(house.Spend_Money(300));
    ASSERT_EQ(house.credits_, 700);
    ASSERT_EQ(house.tiberium_, 500);

    // Spend more than credits (dips into tiberium)
    ASSERT(house.Spend_Money(900));
    ASSERT_EQ(house.credits_, 0);
    ASSERT_EQ(house.tiberium_, 300);

    // Try to spend more than total
    ASSERT(!house.Spend_Money(500));
    // Values should be unchanged
    ASSERT_EQ(house.tiberium_, 300);
}

TEST(resource_refund) {
    HouseClass house;
    house.Init(HousesType::GREECE);

    house.credits_ = 100;
    house.Refund_Money(500);
    ASSERT_EQ(house.credits_, 600);
}

TEST(resource_harvest) {
    HouseClass house;
    house.Init(HousesType::UKRAINE);

    house.capacity_ = 1000;
    house.tiberium_ = 0;

    // Harvest some ore
    house.Harvest_Tiberium(200, 1000);
    ASSERT_EQ(house.tiberium_, 200);
    ASSERT_EQ(house.harvested_, 200);

    // Harvest more than capacity allows
    house.Harvest_Tiberium(1500, 1000);
    ASSERT_EQ(house.tiberium_, 1000);  // Capped at capacity
}

TEST(power_fraction) {
    HouseClass house;
    house.Init(HousesType::SPAIN);

    // No drain
    house.power_ = 100;
    house.drain_ = 0;
    ASSERT_EQ(house.Power_Fraction(), 256);  // 100%

    // Normal power
    house.drain_ = 50;
    ASSERT_EQ(house.Power_Fraction(), 256);  // Still 100% (power > drain)

    // Low power
    house.power_ = 50;
    house.drain_ = 100;
    ASSERT_EQ(house.Power_Fraction(), 128);  // 50%

    // No power
    house.power_ = 0;
    house.drain_ = 100;
    ASSERT_EQ(house.Power_Fraction(), 0);
}

//===========================================================================
// AI State Tests
//===========================================================================

TEST(ai_state_machine) {
    HouseClass house;
    house.Init(HousesType::USSR);
    house.isHuman_ = false;

    ASSERT_EQ(house.state_, HouseStateType::BUILDUP);

    // Set various states
    house.state_ = HouseStateType::THREATENED;
    ASSERT_EQ(house.state_, HouseStateType::THREATENED);

    house.state_ = HouseStateType::ATTACKED;
    ASSERT_EQ(house.state_, HouseStateType::ATTACKED);
}

TEST(ai_urgency) {
    HouseClass house;
    house.Init(HousesType::UKRAINE);

    // Initially all NONE
    for (int i = 0; i < static_cast<int>(StrategyType::COUNT); i++) {
        ASSERT_EQ(house.urgency_[i], UrgencyType::NONE);
    }

    // Set some urgencies
    house.urgency_[static_cast<int>(StrategyType::BUILD_POWER)] = UrgencyType::HIGH;
    house.urgency_[static_cast<int>(StrategyType::ATTACK)] = UrgencyType::CRITICAL;

    ASSERT_EQ(house.urgency_[static_cast<int>(StrategyType::BUILD_POWER)], UrgencyType::HIGH);
    ASSERT_EQ(house.urgency_[static_cast<int>(StrategyType::ATTACK)], UrgencyType::CRITICAL);
}

TEST(find_enemy) {
    HouseClass player;
    player.Init(HousesType::GREECE);
    player.isHuman_ = true;
    player.isActive_ = true;

    HouseClass ai;
    ai.Init(HousesType::USSR);
    ai.isHuman_ = false;
    ai.isActive_ = true;

    // Store in global array for testing
    Houses[static_cast<int>(HousesType::GREECE)] = player;
    Houses[static_cast<int>(HousesType::USSR)] = ai;
    HouseCount = 2;

    // Should find the non-allied house
    HousesType enemy = ai.Find_Enemy();
    ASSERT_EQ(enemy, HousesType::GREECE);
}

//===========================================================================
// TeamTypeClass Tests
//===========================================================================

TEST(teamtype_construction) {
    TeamTypeClass type;
    type.Init();

    ASSERT(!type.isActive_);
    ASSERT_EQ(type.memberCount_, 0);
    ASSERT_EQ(type.missionCount_, 0);
    ASSERT_EQ(type.priority_, 0);
}

TEST(teamtype_flags) {
    TeamTypeClass type;
    type.Init();

    type.isActive_ = true;
    type.isAutocreate_ = true;
    type.isAggressive_ = true;
    type.isSuicide_ = false;
    type.isAlert_ = true;

    ASSERT(type.isActive_);
    ASSERT(type.isAutocreate_);
    ASSERT(type.isAggressive_);
    ASSERT(!type.isSuicide_);
    ASSERT(type.isAlert_);
}

TEST(teamtype_members) {
    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;

    // Add member specs
    type.memberCount_ = 2;
    type.members_[0].type = RTTIType::INFANTRY;
    type.members_[0].typeIndex = 0;
    type.members_[0].count = 5;

    type.members_[1].type = RTTIType::UNIT;
    type.members_[1].typeIndex = 1;
    type.members_[1].count = 3;

    ASSERT_EQ(type.TotalCount(), 8);
}

TEST(teamtype_missions) {
    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;

    type.missionCount_ = 3;
    type.missions_[0].mission = TeamMissionType::MOVE;
    type.missions_[0].argument = 5;  // Waypoint 5

    type.missions_[1].mission = TeamMissionType::ATTACK;
    type.missions_[1].argument = static_cast<int8_t>(QuarryType::BUILDINGS);

    type.missions_[2].mission = TeamMissionType::GUARD;
    type.missions_[2].argument = 0;

    ASSERT_EQ(type.missionCount_, 3);
    ASSERT_EQ(type.missions_[0].mission, TeamMissionType::MOVE);
    ASSERT_EQ(type.missions_[1].mission, TeamMissionType::ATTACK);
}

TEST(teamtype_from_name) {
    Init_TeamTypes();

    // Create a named team type
    TeamTypes[0].Init();
    TeamTypes[0].isActive_ = true;
    strcpy(TeamTypes[0].name_, "AttackForce");
    TeamTypes[0].house_ = HousesType::USSR;

    TeamTypeClass* found = TeamTypeClass::From_Name("AttackForce");
    ASSERT(found != nullptr);
    ASSERT(found == &TeamTypes[0]);

    // Not found
    TeamTypeClass* notFound = TeamTypeClass::From_Name("NonExistent");
    ASSERT(notFound == nullptr);
}

TEST(teamtype_availability) {
    Init_TeamTypes();
    Init_Teams();

    TeamTypes[0].Init();
    TeamTypes[0].isActive_ = true;
    TeamTypes[0].maxAllowed_ = 2;
    strcpy(TeamTypes[0].name_, "TestTeam");

    ASSERT(TeamTypes[0].IsAvailable());

    // Create instances
    TeamClass* team1 = TeamTypes[0].Create_Instance();
    ASSERT(team1 != nullptr);
    ASSERT(TeamTypes[0].IsAvailable());

    TeamClass* team2 = TeamTypes[0].Create_Instance();
    ASSERT(team2 != nullptr);
    ASSERT(!TeamTypes[0].IsAvailable());  // Max reached
}

//===========================================================================
// TeamClass Tests
//===========================================================================

TEST(team_construction) {
    Init_Teams();

    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;
    type.house_ = HousesType::USSR;
    strcpy(type.name_, "TestTeam");

    TeamClass team;
    team.Init(&type);

    ASSERT(team.isActive_);
    ASSERT_EQ(team.house_, HousesType::USSR);
    ASSERT_EQ(team.typeClass_, &type);
    ASSERT_EQ(team.memberCount_, 0);
}

TEST(team_strength) {
    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;
    type.memberCount_ = 1;
    type.members_[0].count = 4;

    TeamClass team;
    team.Init(&type);

    // Empty team
    ASSERT_EQ(team.Strength(), 0);
    ASSERT(team.IsUnderStrength());
    ASSERT(!team.IsFull());

    // Add mock members (would be FootClass* in real use)
    // For test, we just manipulate memberCount_
    team.memberCount_ = 2;
    ASSERT_EQ(team.Strength(), 128);  // 50%
    ASSERT(!team.IsUnderStrength());  // At 50%, not under strength

    team.memberCount_ = 4;
    ASSERT_EQ(team.Strength(), 256);  // 100%
    ASSERT(team.IsFull());
}

TEST(team_missions) {
    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;
    type.missionCount_ = 3;
    type.missions_[0].mission = TeamMissionType::MOVE;
    type.missions_[1].mission = TeamMissionType::ATTACK;
    type.missions_[2].mission = TeamMissionType::GUARD;

    TeamClass team;
    team.Init(&type);

    ASSERT_EQ(team.currentMission_, 0);

    // Advance missions
    ASSERT(team.Next_Mission());
    ASSERT_EQ(team.currentMission_, 1);

    ASSERT(team.Next_Mission());
    ASSERT_EQ(team.currentMission_, 2);

    // End of missions
    ASSERT(!team.Next_Mission());
    ASSERT_EQ(team.currentMission_, 2);  // Stays at last
}

TEST(team_jump_mission) {
    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;
    type.missionCount_ = 3;
    type.missions_[0].mission = TeamMissionType::ATTACK;
    type.missions_[1].mission = TeamMissionType::GUARD;
    type.missions_[2].mission = TeamMissionType::JUMP;
    type.missions_[2].argument = 0;  // Jump back to step 0

    TeamClass team;
    team.Init(&type);

    // Go through missions
    team.Next_Mission();  // 0 -> 1
    team.Next_Mission();  // 1 -> 2 (JUMP)

    // At JUMP mission, Next_Mission should loop back
    team.Next_Mission();  // Should wrap to 0
    ASSERT_EQ(team.currentMission_, 0);
}

TEST(team_formation) {
    TeamClass team;
    team.typeClass_ = nullptr;

    team.formation_ = FormationType::LINE;
    ASSERT_EQ(team.formation_, FormationType::LINE);

    team.formation_ = FormationType::WEDGE;
    ASSERT_EQ(team.formation_, FormationType::WEDGE);
}

TEST(team_disband) {
    Init_Teams();

    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;
    strcpy(type.name_, "DisbandTest");

    TeamClass* team = type.Create_Instance();
    ASSERT(team != nullptr);
    ASSERT(team->isActive_);
    int oldCount = TeamCount;

    team->Disband();
    ASSERT(!team->isActive_);
    ASSERT_EQ(team->memberCount_, 0);
    ASSERT_EQ(TeamCount, oldCount - 1);
}

TEST(team_suspend_resume) {
    TeamTypeClass type;
    type.Init();
    type.isActive_ = true;
    type.missionCount_ = 3;

    TeamClass team;
    team.Init(&type);
    team.currentMission_ = 2;

    team.Suspend();
    ASSERT_EQ(team.suspendedMission_, 2);

    team.currentMission_ = 0;  // Simulate doing something else

    team.Resume();
    ASSERT_EQ(team.currentMission_, 2);
    ASSERT_EQ(team.suspendedMission_, -1);
}

//===========================================================================
// Integration Tests
//===========================================================================

TEST(house_team_integration) {
    Init_TeamTypes();
    Init_Teams();
    Init_Houses();

    // Setup USSR house
    Houses[static_cast<int>(HousesType::USSR)].Init(HousesType::USSR);
    Houses[static_cast<int>(HousesType::USSR)].isActive_ = true;
    Houses[static_cast<int>(HousesType::USSR)].isHuman_ = false;
    HouseCount = 1;

    // Create team type for USSR
    TeamTypes[0].Init();
    TeamTypes[0].isActive_ = true;
    TeamTypes[0].house_ = HousesType::USSR;
    TeamTypes[0].isAutocreate_ = true;
    TeamTypes[0].isAlert_ = false;
    TeamTypes[0].priority_ = 10;
    strcpy(TeamTypes[0].name_, "SovietAttack");
    TeamTypeCount = 1;

    // House should suggest this team
    HouseClass* ussr = &Houses[static_cast<int>(HousesType::USSR)];
    TeamTypeClass* suggested = ussr->Suggested_New_Team(false);
    ASSERT(suggested != nullptr);
    ASSERT(suggested == &TeamTypes[0]);
}

TEST(complete_team_lifecycle) {
    Init_TeamTypes();
    Init_Teams();

    // Create team type
    TeamTypes[0].Init();
    TeamTypes[0].isActive_ = true;
    TeamTypes[0].house_ = HousesType::USSR;
    TeamTypes[0].maxAllowed_ = 1;
    TeamTypes[0].isTransient_ = true;
    TeamTypes[0].missionCount_ = 2;
    TeamTypes[0].missions_[0].mission = TeamMissionType::GUARD;
    TeamTypes[0].missions_[1].mission = TeamMissionType::ATTACK;
    strcpy(TeamTypes[0].name_, "LifecycleTest");
    TeamTypeCount = 1;

    // Create instance
    TeamClass* team = Create_Team(&TeamTypes[0]);
    ASSERT(team != nullptr);
    ASSERT(team->isActive_);

    // Simulate reaching full strength
    team->isHasBeen_ = true;
    team->memberCount_ = 0;  // All died

    // AI should detect empty team after having been full
    team->AI();

    // Team should have disbanded (transient + empty after being full)
    ASSERT(!team->isActive_);
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Red Alert AI System Tests\n");
    printf("=========================\n\n");

    // Initialize global arrays
    Init_TeamTypes();
    Init_Teams();
    Init_Houses();

    printf("HouseTypeData Tests:\n");
    RUN_TEST(house_type_data);
    RUN_TEST(house_type_from_name);

    printf("\nHouseClass Construction Tests:\n");
    RUN_TEST(house_construction);
    RUN_TEST(house_type_queries);

    printf("\nAlliance Tests:\n");
    RUN_TEST(alliance_basic);
    RUN_TEST(alliance_enemy);

    printf("\nResource Tests:\n");
    RUN_TEST(resource_money);
    RUN_TEST(resource_refund);
    RUN_TEST(resource_harvest);
    RUN_TEST(power_fraction);

    printf("\nAI State Tests:\n");
    RUN_TEST(ai_state_machine);
    RUN_TEST(ai_urgency);
    RUN_TEST(find_enemy);

    printf("\nTeamTypeClass Tests:\n");
    RUN_TEST(teamtype_construction);
    RUN_TEST(teamtype_flags);
    RUN_TEST(teamtype_members);
    RUN_TEST(teamtype_missions);
    RUN_TEST(teamtype_from_name);
    RUN_TEST(teamtype_availability);

    printf("\nTeamClass Tests:\n");
    RUN_TEST(team_construction);
    RUN_TEST(team_strength);
    RUN_TEST(team_missions);
    RUN_TEST(team_jump_mission);
    RUN_TEST(team_formation);
    RUN_TEST(team_disband);
    RUN_TEST(team_suspend_resume);

    printf("\nIntegration Tests:\n");
    RUN_TEST(house_team_integration);
    RUN_TEST(complete_team_lifecycle);

    printf("\n=========================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("=========================\n");

    return tests_failed > 0 ? 1 : 0;
}
