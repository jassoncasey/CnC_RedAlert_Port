/**
 * Red Alert macOS Port - Campaign System
 *
 * Manages campaign progression, mission order, briefings, and score tracking.
 * Based on original SCENARIO.CPP and SCORE.CPP campaign logic.
 *
 * Architecture:
 *   - CampaignClass: Manages overall campaign state and progression
 *   - MissionData: Static data for each mission (briefing, movies, etc.)
 *   - ScoreClass: Tracks player statistics for score screen
 */

#ifndef GAME_CAMPAIGN_H
#define GAME_CAMPAIGN_H

#include "types.h"
#include "scenario.h"
#include <cstdint>

//===========================================================================
// Constants
//===========================================================================

// Campaign counts
constexpr int ALLIED_MISSION_COUNT = 14;
constexpr int SOVIET_MISSION_COUNT = 14;
constexpr int AFTERMATH_MISSION_COUNT = 8;  // Expansion missions
constexpr int COUNTERSTRIKE_MISSION_COUNT = 8;  // Expansion missions

// Maximum briefing text length
constexpr int BRIEFING_MAX = 1024;

// Maximum mission name length
constexpr int MISSION_NAME_MAX = 64;

//===========================================================================
// Campaign Type
//===========================================================================

enum class CampaignType : int8_t {
    NONE = -1,
    ALLIED = 0,         // Allied campaign (Greece perspective)
    SOVIET,             // Soviet campaign (USSR perspective)
    AFTERMATH,          // Aftermath expansion
    COUNTERSTRIKE,      // Counter-Strike expansion

    COUNT
};

const char* CampaignName(CampaignType campaign);

//===========================================================================
// Mission State
//===========================================================================

enum class MissionState : int8_t {
    NOT_PLAYED = 0,     // Mission not attempted
    IN_PROGRESS,        // Currently playing
    COMPLETED,          // Completed successfully
    FAILED              // Failed (can retry)
};

//===========================================================================
// Scenario Player Type (for filename generation)
//===========================================================================

enum class ScenarioPlayerType : int8_t {
    GREECE = 0,         // 'G' - Allied (Greece)
    USSR,               // 'U' - Soviet
    SPAIN,              // 'S' - Allied (Spain)
    JAPAN,              // 'J' - Allied (Japan)
    MULTI,              // 'M' - Multiplayer

    COUNT
};

//===========================================================================
// Scenario Direction Type
//===========================================================================

enum class ScenarioDirType : int8_t {
    NONE = -1,          // Random selection
    EAST = 0,           // 'E'
    WEST,               // 'W'

    COUNT
};

//===========================================================================
// Scenario Variation Type
//===========================================================================

enum class ScenarioVarType : int8_t {
    NONE = -1,          // Random selection
    A = 0,              // 'A'
    B,                  // 'B'
    C,                  // 'C'
    D,                  // 'D'

    COUNT
};

//===========================================================================
// Mission Data - Static mission information
//===========================================================================

struct MissionData {
    int missionNumber;              // Mission number (1-14, etc.)
    const char* name;               // Display name
    const char* briefing;           // Briefing text
    VQType introMovie;              // Intro movie
    VQType briefMovie;              // Briefing movie
    VQType winMovie;                // Victory movie
    VQType loseMovie;               // Defeat movie
    ThemeType theme;                // Background music
    TheaterType theater;            // Map terrain
    bool hasMapChoice;              // Player can choose next mission variant
    int nextMissionA;               // Next mission (variant A)
    int nextMissionB;               // Next mission (variant B), -1 if none
};

//===========================================================================
// Score Class - Mission Statistics
//===========================================================================

class ScoreClass {
public:
    ScoreClass();

    void Reset();

    // Track kills
    void Add_Unit_Kill(HousesType killedHouse);
    void Add_Building_Kill(HousesType killedHouse);
    void Add_Civilian_Kill();

    // Track harvesting
    void Add_Ore_Harvested(int amount);
    void Add_Enemy_Ore_Lost(int amount);

    // Track time
    void Set_Elapsed_Time(int frames);

    // Calculate final score
    int Calculate_Score() const;

    // Accessors
    int Units_Killed() const { return unitsKilled_; }
    int Enemy_Units_Killed() const { return enemyUnitsKilled_; }
    int Buildings_Destroyed() const { return buildingsDestroyed_; }
    int Enemy_Buildings_Destroyed() const { return enemyBuildingsDestroyed_; }
    int Civilians_Killed() const { return civiliansKilled_; }
    int Ore_Harvested() const { return oreHarvested_; }
    int Enemy_Ore_Lost() const { return enemyOreLost_; }
    int Elapsed_Time() const { return elapsedTime_; }

    // Presentation
    void Presentation();  // Display score screen

private:
    int unitsKilled_;               // Player units lost
    int enemyUnitsKilled_;          // Enemy units destroyed
    int buildingsDestroyed_;        // Player buildings lost
    int enemyBuildingsDestroyed_;   // Enemy buildings destroyed
    int civiliansKilled_;           // Civilian casualties
    int oreHarvested_;              // Ore collected
    int enemyOreLost_;              // Enemy ore we denied/destroyed
    int elapsedTime_;               // Mission time in frames
};

//===========================================================================
// Campaign Class - Campaign Manager
//===========================================================================

class CampaignClass {
public:
    CampaignClass();
    ~CampaignClass() = default;

    //-----------------------------------------------------------------------
    // Campaign Management
    //-----------------------------------------------------------------------

    // Initialize campaign state
    void Init();

    // Start a new campaign
    bool Start_Campaign(CampaignType campaign, DifficultyType difficulty);

    // Get current campaign info
    CampaignType Get_Campaign() const { return currentCampaign_; }
    int Get_Current_Mission() const { return currentMission_; }
    bool Is_Campaign_Active() const {
        return currentCampaign_ != CampaignType::NONE;
    }

    // Get mission count for campaign type
    static int Get_Mission_Count(CampaignType campaign);

    //-----------------------------------------------------------------------
    // Mission Management
    //-----------------------------------------------------------------------

    // Load and start a mission
    bool Start_Mission(int missionNum);

    // Called when mission ends
    void Mission_Won();
    void Mission_Lost();

    // Get mission state
    MissionState Get_Mission_State(int missionNum) const;

    // Check if mission is available
    bool Is_Mission_Available(int missionNum) const;

    //-----------------------------------------------------------------------
    // Briefing System
    //-----------------------------------------------------------------------

    // Get briefing text for current/specified mission
    const char* Get_Briefing() const;
    const char* Get_Briefing(int missionNum) const;

    // Get mission data
    const MissionData* Get_Mission_Data(int missionNum) const;

    // Play briefing sequence (movies + text)
    void Play_Briefing();

    //-----------------------------------------------------------------------
    // Progression
    //-----------------------------------------------------------------------

    // Get next mission (after win)
    int Get_Next_Mission() const;

    // Choose mission variant (for branching)
    void Choose_Variant(ScenarioVarType variant);

    // Advance to next mission
    void Advance_Mission();

    // Check if campaign is complete
    bool Is_Campaign_Complete() const;

    //-----------------------------------------------------------------------
    // Score & Statistics
    //-----------------------------------------------------------------------

    // Get score tracker
    ScoreClass& Score() { return score_; }
    const ScoreClass& Score() const { return score_; }

    // Display score screen
    void Show_Score_Screen();

    // Get total campaign score
    int Get_Total_Score() const { return totalScore_; }

    //-----------------------------------------------------------------------
    // Carry-Over System
    //-----------------------------------------------------------------------

    // Save/load carry-over state
    void Save_Carryover();
    void Load_Carryover();

    // Get carry-over money
    int Get_Carryover_Money() const { return carryoverMoney_; }

    //-----------------------------------------------------------------------
    // Scenario Filename Generation
    //-----------------------------------------------------------------------

    // Generate scenario filename
    static void Make_Scenario_Name(
        char* buffer, int bufSize,
        int scenario,
        ScenarioPlayerType player,
        ScenarioDirType dir = ScenarioDirType::EAST,
        ScenarioVarType var = ScenarioVarType::A
    );

    // Parse scenario filename
    static bool Parse_Scenario_Name(
        const char* name,
        int& scenario,
        ScenarioPlayerType& player,
        ScenarioDirType& dir,
        ScenarioVarType& var
    );

    //-----------------------------------------------------------------------
    // Persistence
    //-----------------------------------------------------------------------

    // Save/load campaign progress
    bool Save_Progress(const char* filename) const;
    bool Load_Progress(const char* filename);

private:
    // Current state
    CampaignType currentCampaign_;
    int currentMission_;
    DifficultyType difficulty_;

    // Mission states
    MissionState missionStates_[20];  // Max missions in any campaign

    // Score tracking
    ScoreClass score_;
    int totalScore_;

    // Carry-over state
    int carryoverMoney_;
    int carryoverUnits_[32];     // Unit type counts
    int carryoverBuildings_[32]; // Building type counts

    // Mission variant choice
    ScenarioVarType chosenVariant_;

    // Helper to get mission data table
    const MissionData* Get_Mission_Table(CampaignType campaign) const;
};

//===========================================================================
// Global Campaign Instance
//===========================================================================

extern CampaignClass Campaign;

//===========================================================================
// Campaign Data Tables (defined in campaign.cpp)
//===========================================================================

extern const MissionData AlliedMissions[];
extern const MissionData SovietMissions[];

#endif // GAME_CAMPAIGN_H
