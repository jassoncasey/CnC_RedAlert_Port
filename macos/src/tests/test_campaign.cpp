/**
 * Red Alert macOS Port - Campaign System Tests
 *
 * Tests for CampaignClass, MissionData, ScoreClass
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "../game/campaign.h"

//===========================================================================
// Test Framework
//===========================================================================

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name()
#define RUN_TEST(name) do { \
    printf("  Testing %s...", #name); \
    test_##name(); \
    printf(" OK\n"); \
    tests_passed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf(" FAILED at line %d: %s\n", __LINE__, #cond); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf(" FAILED at line %d: %s != %s\n", __LINE__, #a, #b); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_STR_EQ(a, b) do { \
    if (strcmp((a), (b)) != 0) { \
        printf(" FAILED at line %d: '%s' != '%s'\n", __LINE__, (a), (b)); \
        tests_failed++; \
        return; \
    } \
} while(0)

//===========================================================================
// Campaign Type Tests
//===========================================================================

TEST(campaign_name) {
    ASSERT_STR_EQ(CampaignName(CampaignType::ALLIED), "Allied");
    ASSERT_STR_EQ(CampaignName(CampaignType::SOVIET), "Soviet");
    ASSERT_STR_EQ(CampaignName(CampaignType::AFTERMATH), "Aftermath");
    ASSERT_STR_EQ(CampaignName(CampaignType::COUNTERSTRIKE), "Counter-Strike");
    ASSERT_STR_EQ(CampaignName(CampaignType::NONE), "Unknown");
}

TEST(campaign_mission_count) {
    ASSERT_EQ(CampaignClass::Get_Mission_Count(CampaignType::ALLIED), 14);
    ASSERT_EQ(CampaignClass::Get_Mission_Count(CampaignType::SOVIET), 14);
    ASSERT_EQ(CampaignClass::Get_Mission_Count(CampaignType::AFTERMATH), 8);
    ASSERT_EQ(CampaignClass::Get_Mission_Count(CampaignType::COUNTERSTRIKE), 8);
    ASSERT_EQ(CampaignClass::Get_Mission_Count(CampaignType::NONE), 0);
}

//===========================================================================
// Campaign Initialization Tests
//===========================================================================

TEST(campaign_init) {
    CampaignClass campaign;
    campaign.Init();

    ASSERT_EQ(campaign.Get_Campaign(), CampaignType::NONE);
    ASSERT_EQ(campaign.Get_Current_Mission(), 0);
    ASSERT(!campaign.Is_Campaign_Active());
    ASSERT_EQ(campaign.Get_Total_Score(), 0);
}

TEST(campaign_start_allied) {
    CampaignClass campaign;
    campaign.Init();

    bool started = campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);
    ASSERT(started);
    ASSERT_EQ(campaign.Get_Campaign(), CampaignType::ALLIED);
    ASSERT_EQ(campaign.Get_Current_Mission(), 1);
    ASSERT(campaign.Is_Campaign_Active());
}

TEST(campaign_start_soviet) {
    CampaignClass campaign;
    campaign.Init();

    bool started = campaign.Start_Campaign(CampaignType::SOVIET, DifficultyType::HARD);
    ASSERT(started);
    ASSERT_EQ(campaign.Get_Campaign(), CampaignType::SOVIET);
    ASSERT_EQ(campaign.Get_Current_Mission(), 1);
    ASSERT(campaign.Is_Campaign_Active());
}

TEST(campaign_start_invalid) {
    CampaignClass campaign;
    campaign.Init();

    bool started = campaign.Start_Campaign(CampaignType::NONE, DifficultyType::NORMAL);
    ASSERT(!started);
    ASSERT(!campaign.Is_Campaign_Active());
}

//===========================================================================
// Mission Data Tests
//===========================================================================

TEST(mission_data_allied) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    const MissionData* data = campaign.Get_Mission_Data(1);
    ASSERT(data != nullptr);
    ASSERT_EQ(data->missionNumber, 1);
    ASSERT(data->name != nullptr);
    ASSERT(data->briefing != nullptr);
    ASSERT(strlen(data->briefing) > 0);
}

TEST(mission_data_soviet) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::SOVIET, DifficultyType::NORMAL);

    const MissionData* data = campaign.Get_Mission_Data(1);
    ASSERT(data != nullptr);
    ASSERT_EQ(data->missionNumber, 1);
    ASSERT(data->name != nullptr);
    ASSERT(data->briefing != nullptr);
    ASSERT(strlen(data->briefing) > 0);
}

TEST(mission_data_all_allied) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    for (int i = 1; i <= ALLIED_MISSION_COUNT; i++) {
        const MissionData* data = campaign.Get_Mission_Data(i);
        ASSERT(data != nullptr);
        ASSERT_EQ(data->missionNumber, i);
        ASSERT(data->name != nullptr);
        ASSERT(data->briefing != nullptr);
    }
}

TEST(mission_data_all_soviet) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::SOVIET, DifficultyType::NORMAL);

    for (int i = 1; i <= SOVIET_MISSION_COUNT; i++) {
        const MissionData* data = campaign.Get_Mission_Data(i);
        ASSERT(data != nullptr);
        ASSERT_EQ(data->missionNumber, i);
        ASSERT(data->name != nullptr);
        ASSERT(data->briefing != nullptr);
    }
}

TEST(mission_data_invalid) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    const MissionData* data = campaign.Get_Mission_Data(0);
    ASSERT(data == nullptr);

    data = campaign.Get_Mission_Data(100);
    ASSERT(data == nullptr);
}

//===========================================================================
// Briefing Tests
//===========================================================================

TEST(briefing_allied) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);
    campaign.Start_Mission(1);

    const char* briefing = campaign.Get_Briefing();
    ASSERT(briefing != nullptr);
    ASSERT(strlen(briefing) > 0);
    // Allied mission 1 mentions "Soviet forces"
    ASSERT(strstr(briefing, "Soviet") != nullptr);
}

TEST(briefing_soviet) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::SOVIET, DifficultyType::NORMAL);
    campaign.Start_Mission(1);

    const char* briefing = campaign.Get_Briefing();
    ASSERT(briefing != nullptr);
    ASSERT(strlen(briefing) > 0);
    // Soviet mission 1 mentions "Allied" or "capitalist"
    ASSERT(strstr(briefing, "Allied") != nullptr || strstr(briefing, "capitalist") != nullptr);
}

//===========================================================================
// Mission State Tests
//===========================================================================

TEST(mission_state_initial) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    // Mission 1 should be in progress after start
    ASSERT_EQ(campaign.Get_Mission_State(1), MissionState::IN_PROGRESS);

    // Other missions should be not played
    ASSERT_EQ(campaign.Get_Mission_State(2), MissionState::NOT_PLAYED);
    ASSERT_EQ(campaign.Get_Mission_State(14), MissionState::NOT_PLAYED);
}

TEST(mission_state_won) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    campaign.Mission_Won();
    ASSERT_EQ(campaign.Get_Mission_State(1), MissionState::COMPLETED);
}

TEST(mission_state_lost) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    campaign.Mission_Lost();
    ASSERT_EQ(campaign.Get_Mission_State(1), MissionState::FAILED);
}

//===========================================================================
// Mission Availability Tests
//===========================================================================

TEST(mission_available) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    // Mission 1 is always available
    ASSERT(campaign.Is_Mission_Available(1));

    // Mission 2 is not available until mission 1 is complete
    ASSERT(!campaign.Is_Mission_Available(2));

    // Complete mission 1
    campaign.Mission_Won();

    // Now mission 2 should be available
    ASSERT(campaign.Is_Mission_Available(2));
}

TEST(mission_available_bounds) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    ASSERT(!campaign.Is_Mission_Available(0));
    ASSERT(!campaign.Is_Mission_Available(100));
}

//===========================================================================
// Campaign Progression Tests
//===========================================================================

TEST(campaign_progression) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    ASSERT_EQ(campaign.Get_Current_Mission(), 1);

    campaign.Mission_Won();
    ASSERT_EQ(campaign.Get_Current_Mission(), 2);

    // Start next mission
    campaign.Start_Mission(2);
    ASSERT_EQ(campaign.Get_Current_Mission(), 2);
}

TEST(campaign_complete_check) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::NORMAL);

    // Not complete at start
    ASSERT(!campaign.Is_Campaign_Complete());

    // Progress through missions (simulate wins)
    for (int i = 1; i < ALLIED_MISSION_COUNT; i++) {
        campaign.Mission_Won();
        campaign.Start_Mission(campaign.Get_Current_Mission());
        ASSERT(!campaign.Is_Campaign_Complete());
    }

    // Win final mission
    campaign.Mission_Won();
    ASSERT(campaign.Is_Campaign_Complete());
}

//===========================================================================
// Scenario Filename Tests
//===========================================================================

TEST(scenario_name_basic) {
    char buffer[64];

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 1,
                                       ScenarioPlayerType::GREECE,
                                       ScenarioDirType::EAST,
                                       ScenarioVarType::A);
    ASSERT_STR_EQ(buffer, "SCG01EA.INI");

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 1,
                                       ScenarioPlayerType::USSR,
                                       ScenarioDirType::EAST,
                                       ScenarioVarType::A);
    ASSERT_STR_EQ(buffer, "SCU01EA.INI");
}

TEST(scenario_name_variations) {
    char buffer[64];

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 5,
                                       ScenarioPlayerType::GREECE,
                                       ScenarioDirType::WEST,
                                       ScenarioVarType::B);
    ASSERT_STR_EQ(buffer, "SCG05WB.INI");

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 14,
                                       ScenarioPlayerType::USSR,
                                       ScenarioDirType::EAST,
                                       ScenarioVarType::C);
    ASSERT_STR_EQ(buffer, "SCU14EC.INI");
}

TEST(scenario_name_players) {
    char buffer[64];

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 1,
                                       ScenarioPlayerType::SPAIN,
                                       ScenarioDirType::EAST,
                                       ScenarioVarType::A);
    ASSERT_STR_EQ(buffer, "SCS01EA.INI");

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 1,
                                       ScenarioPlayerType::JAPAN,
                                       ScenarioDirType::EAST,
                                       ScenarioVarType::A);
    ASSERT_STR_EQ(buffer, "SCJ01EA.INI");

    CampaignClass::Make_Scenario_Name(buffer, sizeof(buffer), 1,
                                       ScenarioPlayerType::MULTI,
                                       ScenarioDirType::EAST,
                                       ScenarioVarType::A);
    ASSERT_STR_EQ(buffer, "SCM01EA.INI");
}

TEST(scenario_name_parse) {
    int scenario;
    ScenarioPlayerType player;
    ScenarioDirType dir;
    ScenarioVarType var;

    bool success = CampaignClass::Parse_Scenario_Name("SCG01EA.INI",
                                                       scenario, player, dir, var);
    ASSERT(success);
    ASSERT_EQ(scenario, 1);
    ASSERT_EQ(player, ScenarioPlayerType::GREECE);
    ASSERT_EQ(dir, ScenarioDirType::EAST);
    ASSERT_EQ(var, ScenarioVarType::A);

    success = CampaignClass::Parse_Scenario_Name("SCU14WB.INI",
                                                  scenario, player, dir, var);
    ASSERT(success);
    ASSERT_EQ(scenario, 14);
    ASSERT_EQ(player, ScenarioPlayerType::USSR);
    ASSERT_EQ(dir, ScenarioDirType::WEST);
    ASSERT_EQ(var, ScenarioVarType::B);
}

TEST(scenario_name_parse_invalid) {
    int scenario;
    ScenarioPlayerType player;
    ScenarioDirType dir;
    ScenarioVarType var;

    bool success = CampaignClass::Parse_Scenario_Name("INVALID",
                                                       scenario, player, dir, var);
    ASSERT(!success);

    success = CampaignClass::Parse_Scenario_Name(nullptr,
                                                  scenario, player, dir, var);
    ASSERT(!success);
}

//===========================================================================
// Score Tests
//===========================================================================

TEST(score_init) {
    ScoreClass score;

    ASSERT_EQ(score.Units_Killed(), 0);
    ASSERT_EQ(score.Enemy_Units_Killed(), 0);
    ASSERT_EQ(score.Buildings_Destroyed(), 0);
    ASSERT_EQ(score.Enemy_Buildings_Destroyed(), 0);
    ASSERT_EQ(score.Civilians_Killed(), 0);
    ASSERT_EQ(score.Ore_Harvested(), 0);
    ASSERT_EQ(score.Elapsed_Time(), 0);
}

TEST(score_unit_kills) {
    ScoreClass score;

    // Kill enemy units
    score.Add_Unit_Kill(HousesType::USSR);
    score.Add_Unit_Kill(HousesType::USSR);
    score.Add_Unit_Kill(HousesType::BAD);
    ASSERT_EQ(score.Enemy_Units_Killed(), 3);

    // Lose player units
    score.Add_Unit_Kill(HousesType::GREECE);
    score.Add_Unit_Kill(HousesType::GOOD);
    ASSERT_EQ(score.Units_Killed(), 2);
}

TEST(score_building_kills) {
    ScoreClass score;

    score.Add_Building_Kill(HousesType::USSR);
    score.Add_Building_Kill(HousesType::USSR);
    ASSERT_EQ(score.Enemy_Buildings_Destroyed(), 2);

    score.Add_Building_Kill(HousesType::GREECE);
    ASSERT_EQ(score.Buildings_Destroyed(), 1);
}

TEST(score_civilians) {
    ScoreClass score;

    score.Add_Civilian_Kill();
    score.Add_Civilian_Kill();
    score.Add_Civilian_Kill();
    ASSERT_EQ(score.Civilians_Killed(), 3);
}

TEST(score_resources) {
    ScoreClass score;

    score.Add_Ore_Harvested(1000);
    score.Add_Ore_Harvested(500);
    ASSERT_EQ(score.Ore_Harvested(), 1500);

    score.Add_Enemy_Ore_Lost(300);
    ASSERT_EQ(score.Enemy_Ore_Lost(), 300);
}

TEST(score_calculation) {
    ScoreClass score;

    // Setup a typical mission score
    score.Add_Unit_Kill(HousesType::USSR);  // +50
    score.Add_Unit_Kill(HousesType::USSR);  // +50
    score.Add_Building_Kill(HousesType::USSR);  // +100
    score.Add_Ore_Harvested(5000);  // +50
    score.Set_Elapsed_Time(10000);  // Under 30 min = +1000

    int totalScore = score.Calculate_Score();
    // 50+50+100+50+1000 = 1250
    ASSERT_EQ(totalScore, 1250);
}

TEST(score_calculation_with_penalties) {
    ScoreClass score;

    // Setup score with penalties
    score.Add_Unit_Kill(HousesType::USSR);  // +50
    score.Add_Building_Kill(HousesType::USSR);  // +100
    score.Add_Unit_Kill(HousesType::GREECE);  // -25
    score.Add_Building_Kill(HousesType::GREECE);  // -50
    score.Add_Civilian_Kill();  // -100
    score.Set_Elapsed_Time(30000);  // Over 30 min = no bonus

    int totalScore = score.Calculate_Score();
    // 50+100-25-50-100 = -25 (clamped to 0)
    ASSERT_EQ(totalScore, 0);
}

TEST(score_reset) {
    ScoreClass score;

    score.Add_Unit_Kill(HousesType::USSR);
    score.Add_Ore_Harvested(1000);
    score.Set_Elapsed_Time(5000);

    score.Reset();

    ASSERT_EQ(score.Enemy_Units_Killed(), 0);
    ASSERT_EQ(score.Ore_Harvested(), 0);
    ASSERT_EQ(score.Elapsed_Time(), 0);
}

//===========================================================================
// Campaign Progress Persistence Tests
//===========================================================================

TEST(campaign_save_load) {
    CampaignClass campaign;
    campaign.Start_Campaign(CampaignType::ALLIED, DifficultyType::HARD);
    campaign.Mission_Won();
    campaign.Start_Mission(2);
    campaign.Mission_Won();

    // Save progress
    const char* testFile = "/tmp/test_campaign.sav";
    bool saved = campaign.Save_Progress(testFile);
    ASSERT(saved);

    // Create new campaign and load
    CampaignClass campaign2;
    bool loaded = campaign2.Load_Progress(testFile);
    ASSERT(loaded);

    ASSERT_EQ(campaign2.Get_Campaign(), CampaignType::ALLIED);
    ASSERT_EQ(campaign2.Get_Mission_State(1), MissionState::COMPLETED);
    ASSERT_EQ(campaign2.Get_Mission_State(2), MissionState::COMPLETED);

    // Clean up
    remove(testFile);
}

TEST(campaign_load_invalid) {
    CampaignClass campaign;

    bool loaded = campaign.Load_Progress("/nonexistent/file.sav");
    ASSERT(!loaded);

    loaded = campaign.Load_Progress(nullptr);
    ASSERT(!loaded);
}

//===========================================================================
// Main Test Runner
//===========================================================================

int main() {
    printf("Campaign System Tests\n");
    printf("=====================\n\n");

    printf("Campaign Type Tests:\n");
    RUN_TEST(campaign_name);
    RUN_TEST(campaign_mission_count);

    printf("\nCampaign Initialization Tests:\n");
    RUN_TEST(campaign_init);
    RUN_TEST(campaign_start_allied);
    RUN_TEST(campaign_start_soviet);
    RUN_TEST(campaign_start_invalid);

    printf("\nMission Data Tests:\n");
    RUN_TEST(mission_data_allied);
    RUN_TEST(mission_data_soviet);
    RUN_TEST(mission_data_all_allied);
    RUN_TEST(mission_data_all_soviet);
    RUN_TEST(mission_data_invalid);

    printf("\nBriefing Tests:\n");
    RUN_TEST(briefing_allied);
    RUN_TEST(briefing_soviet);

    printf("\nMission State Tests:\n");
    RUN_TEST(mission_state_initial);
    RUN_TEST(mission_state_won);
    RUN_TEST(mission_state_lost);

    printf("\nMission Availability Tests:\n");
    RUN_TEST(mission_available);
    RUN_TEST(mission_available_bounds);

    printf("\nCampaign Progression Tests:\n");
    RUN_TEST(campaign_progression);
    RUN_TEST(campaign_complete_check);

    printf("\nScenario Filename Tests:\n");
    RUN_TEST(scenario_name_basic);
    RUN_TEST(scenario_name_variations);
    RUN_TEST(scenario_name_players);
    RUN_TEST(scenario_name_parse);
    RUN_TEST(scenario_name_parse_invalid);

    printf("\nScore Tests:\n");
    RUN_TEST(score_init);
    RUN_TEST(score_unit_kills);
    RUN_TEST(score_building_kills);
    RUN_TEST(score_civilians);
    RUN_TEST(score_resources);
    RUN_TEST(score_calculation);
    RUN_TEST(score_calculation_with_penalties);
    RUN_TEST(score_reset);

    printf("\nPersistence Tests:\n");
    RUN_TEST(campaign_save_load);
    RUN_TEST(campaign_load_invalid);

    printf("\n=====================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("=====================\n");

    return tests_failed > 0 ? 1 : 0;
}
