/**
 * Red Alert macOS Port - Scenario System
 *
 * ScenarioClass - Mission container with settings, timers, waypoints
 *
 * Based on original SCENARIO.H (~200 lines)
 */

#ifndef GAME_SCENARIO_H
#define GAME_SCENARIO_H

#include "types.h"
#include "house.h"
#include <cstdint>

// Forward declarations
class TriggerTypeClass;
class TriggerClass;
class TeamTypeClass;

//===========================================================================
// Constants
//===========================================================================

// Waypoint count (A-Z plus special waypoints)
constexpr int WAYPT_COUNT = 30;
constexpr int WAYPT_HOME = 26;      // Base/home waypoint
constexpr int WAYPT_REINF = 27;     // Reinforcement waypoint
constexpr int WAYPT_SPECIAL = 28;   // Special waypoint
constexpr int WAYPT_FLARE = 29;     // Flare waypoint

// Global flags for persistent trigger conditions
constexpr int GLOBAL_FLAG_COUNT = 30;

// Maximum description length
constexpr int DESCRIPTION_MAX = 128;

// Maximum scenario name length
constexpr int SCENARIO_NAME_MAX = 64;

//===========================================================================
// Theater Types (map terrain)
//===========================================================================

enum class TheaterType : int8_t {
    NONE = -1,
    TEMPERATE = 0,  // European
    SNOW,           // Winter
    INTERIOR,       // Indoor missions

    COUNT
};

const char* TheaterName(TheaterType theater);
TheaterType TheaterFromName(const char* name);

//===========================================================================
// Video/Movie Types
//===========================================================================

enum class VQType : int8_t {
    NONE = -1,

    // Allied campaign movies
    ALLY01 = 0,
    ALLY02,
    ALLY04,
    ALLY05,
    ALLY06,
    ALLY08,
    ALLY09,
    ALLY10,
    ALLY11,
    ALLY12,
    ALLY14,

    // Soviet campaign movies
    SOVIET01,
    SOVIET02,
    SOVIET03,
    SOVIET04,
    SOVIET05,
    SOVIET06,
    SOVIET07,
    SOVIET08,
    SOVIET09,
    SOVIET10,
    SOVIET11,
    SOVIET12,
    SOVIET13,

    // Misc movies
    INTRO,
    TOOFAR,
    PROGRES,
    MASTEFIN,

    COUNT
};

// Get VQA filename for a VQType (without .VQA extension)
const char* VQTypeName(VQType type);

//===========================================================================
// Music Theme Types
//===========================================================================

enum class ThemeType : int8_t {
    NONE = -1,
    BIGFOOT = 0,
    CRUSH,
    FACE_THE_ENEMY_1,
    FACE_THE_ENEMY_2,
    HELL_MARCH,
    RUN_FOR_YOUR_LIFE,
    SMASH,
    TRENCHES,
    WORKMEN,
    AWAIT,
    DENSE,
    FOGGER,
    MUDHAND,
    RADIO,
    TWIN_GUNS,
    VECTOR,

    COUNT
};

//===========================================================================
// Source Edge (reinforcement arrival direction)
//===========================================================================

enum class SourceType : int8_t {
    NONE = -1,
    NORTH = 0,
    EAST,
    SOUTH,
    WEST,
    AIR,    // Airborne

    COUNT
};

//===========================================================================
// ScenarioClass - Mission Container
//===========================================================================

class ScenarioClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    int scenario_;                          // Mission number
    TheaterType theater_;                   // Map terrain type
    char name_[SCENARIO_NAME_MAX];          // Scenario filename
    char description_[DESCRIPTION_MAX];     // Mission briefing text

    //-----------------------------------------------------------------------
    // Movies
    //-----------------------------------------------------------------------
    VQType introMovie_;                     // Intro video
    VQType briefMovie_;                     // Briefing video
    VQType winMovie_;                       // Victory video
    VQType loseMovie_;                      // Defeat video
    VQType actionMovie_;                    // Action scene video

    //-----------------------------------------------------------------------
    // Music
    //-----------------------------------------------------------------------
    ThemeType theme_;                       // Background music

    //-----------------------------------------------------------------------
    // Timers (in game frames, 15 FPS)
    //-----------------------------------------------------------------------
    int32_t elapsedTime_;                   // Mission elapsed time
    int32_t missionTimer_;                  // Countdown timer (-1 = disabled)
    int32_t shroudTimer_;                   // Fog of war regrowth timer

    //-----------------------------------------------------------------------
    // Player & Difficulty
    //-----------------------------------------------------------------------
    HousesType playerHouse_;                // Player's faction
    DifficultyType difficulty_;             // Player difficulty
    DifficultyType computerDifficulty_;     // Computer difficulty

    //-----------------------------------------------------------------------
    // Financial
    //-----------------------------------------------------------------------
    int32_t carryOverMoney_;                // Money from previous mission
    int32_t carryOverCap_;                  // Maximum carryover
    int carryOverPercent_;                  // Percentage carried (0-100)
    int buildPercent_;                      // Computer base build percentage

    //-----------------------------------------------------------------------
    // Waypoints (A-Z plus special)
    //-----------------------------------------------------------------------
    int16_t waypoints_[WAYPT_COUNT];        // Cell coordinates for waypoints

    //-----------------------------------------------------------------------
    // Global Flags (persistent trigger conditions)
    //-----------------------------------------------------------------------
    bool globalFlags_[GLOBAL_FLAG_COUNT];

    //-----------------------------------------------------------------------
    // Flags
    //-----------------------------------------------------------------------
    bool isToCarryOver_ : 1;                // Preserve units to next mission
    bool isToInherit_ : 1;                  // Inherit units from prev mission
    bool isInheritTimer_ : 1;               // Carry over mission timer
    bool isEndOfGame_ : 1;                  // Final mission flag
    bool isOneTimeOnly_ : 1;                // Don't repeat mission
    bool isNoMapSel_ : 1;                   // Skip map selection
    bool isTanyaEvac_ : 1;                  // Auto-evacuate Tanya
    bool isSkipScore_ : 1;                  // Skip score screen
    bool isNoSpyPlane_ : 1;                 // Disable spy plane
    bool isTruckCrate_ : 1;                 // Trucks drop crates
    bool isMoneyTiberium_ : 1;              // Money stored in silos
    bool isBridgeDestroyed_ : 1;            // Bridge has been destroyed
    bool isVariant_ : 1;                    // Alternate scenario version

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    ScenarioClass();
    ~ScenarioClass() = default;

    void Init();
    void Clear();

    //-----------------------------------------------------------------------
    // Scenario Loading
    //-----------------------------------------------------------------------
    bool Read_INI(const char* filename);
    bool Start(bool showBriefing = true);

    //-----------------------------------------------------------------------
    // Waypoint Management
    //-----------------------------------------------------------------------
    int16_t Get_Waypoint(int index) const;
    void Set_Waypoint(int index, int16_t cell);
    int16_t Get_Waypoint_Cell(char letter) const;  // A-Z lookup

    //-----------------------------------------------------------------------
    // Global Flags
    //-----------------------------------------------------------------------
    bool Get_Global(int index) const;
    void Set_Global(int index, bool value);

    //-----------------------------------------------------------------------
    // Timer Management
    //-----------------------------------------------------------------------
    void Start_Mission_Timer(int frames);
    void Stop_Mission_Timer();
    void Add_Mission_Timer(int frames);
    void Sub_Mission_Timer(int frames);
    bool Is_Mission_Timer_Active() const { return missionTimer_ >= 0; }
    int Get_Mission_Timer() const { return missionTimer_; }

    //-----------------------------------------------------------------------
    // Game Loop
    //-----------------------------------------------------------------------
    void AI();  // Per-frame processing

    //-----------------------------------------------------------------------
    // Queries
    //-----------------------------------------------------------------------
    const char* Name() const { return name_; }
    const char* Description() const { return description_; }
    TheaterType Theater() const { return theater_; }
    bool IsEndOfGame() const { return isEndOfGame_; }
};

//===========================================================================
// Global Scenario Instance
//===========================================================================

extern ScenarioClass Scen;

//===========================================================================
// Helper Functions
//===========================================================================

// Set scenario by number and variant
void Set_Scenario_Name(int scenario,
                       TheaterType theater = TheaterType::TEMPERATE,
                       SideType side = SideType::ALLIED,
                       bool isAftermathSC = false);

// Get scenario INI filename
const char* Scenario_Filename(int scenario, TheaterType theater,
                              SideType side, bool isAftermathSC);

#endif // GAME_SCENARIO_H
