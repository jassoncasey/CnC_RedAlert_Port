/**
 * Red Alert macOS Port - Scenario Implementation
 *
 * Based on original SCENARIO.CPP (~5000 lines)
 */

#include "scenario.h"
#include "trigger.h"
#include "team.h"
#include "ini.h"
#include <cstring>
#include <cstdio>

//===========================================================================
// Global Instance
//===========================================================================

ScenarioClass Scen;

//===========================================================================
// Theater Name Table
//===========================================================================

static const char* TheaterNames[] = {
    "TEMPERATE",
    "SNOW",
    "INTERIOR"
};

const char* TheaterName(TheaterType theater) {
    if (theater < TheaterType::TEMPERATE || theater >= TheaterType::COUNT) {
        return "TEMPERATE";
    }
    return TheaterNames[static_cast<int>(theater)];
}

TheaterType TheaterFromName(const char* name) {
    if (!name || !name[0]) return TheaterType::TEMPERATE;

    for (int i = 0; i < static_cast<int>(TheaterType::COUNT); i++) {
        if (strcasecmp(name, TheaterNames[i]) == 0) {
            return static_cast<TheaterType>(i);
        }
    }

    // Check short names
    if (strcasecmp(name, "TEMP") == 0) return TheaterType::TEMPERATE;
    if (strcasecmp(name, "SNO") == 0) return TheaterType::SNOW;
    if (strcasecmp(name, "INT") == 0) return TheaterType::INTERIOR;

    return TheaterType::TEMPERATE;
}

//===========================================================================
// VQType Name Table
//===========================================================================

static const char* VQTypeNames[] = {
    // Allied campaign movies (match VQType enum order)
    "ALLY1",        // ALLY01
    "ALLY2",        // ALLY02
    "ALLY4",        // ALLY04
    "ALLY5",        // ALLY05
    "ALLY6",        // ALLY06
    "ALLY8",        // ALLY08
    "ALLY9",        // ALLY09
    "ALLY10",       // ALLY10
    "ALLY11",       // ALLY11
    "ALLY12",       // ALLY12
    "ALLY14",       // ALLY14

    // Soviet campaign movies
    "SOVIET1",      // SOVIET01
    "SOVIET2",      // SOVIET02
    "SOVIET3",      // SOVIET03
    "SOVIET4",      // SOVIET04
    "SOVIET5",      // SOVIET05
    "SOVIET6",      // SOVIET06
    "SOVIET7",      // SOVIET07
    "SOVIET8",      // SOVIET08
    "SOVIET9",      // SOVIET09
    "SOVIET10",     // SOVIET10
    "SOVIET11",     // SOVIET11
    "SOVIET12",     // SOVIET12
    "SOVIET13",     // SOVIET13

    // Misc movies
    "PROLOG",       // INTRO
    "TOOFAR",       // TOOFAR
    "PROGRES",      // PROGRES
    "MASASSLT"      // MASTEFIN
};

const char* VQTypeName(VQType type) {
    if (type == VQType::NONE || type >= VQType::COUNT) {
        return nullptr;
    }
    int index = static_cast<int>(type);
    if (index >= 0 && index < static_cast<int>(VQType::COUNT)) {
        return VQTypeNames[index];
    }
    return nullptr;
}

//===========================================================================
// ScenarioClass Implementation
//===========================================================================

ScenarioClass::ScenarioClass() {
    Init();
}

void ScenarioClass::Init() {
    scenario_ = 1;
    theater_ = TheaterType::TEMPERATE;
    name_[0] = '\0';
    description_[0] = '\0';

    introMovie_ = VQType::NONE;
    briefMovie_ = VQType::NONE;
    winMovie_ = VQType::NONE;
    loseMovie_ = VQType::NONE;
    actionMovie_ = VQType::NONE;

    theme_ = ThemeType::NONE;

    elapsedTime_ = 0;
    missionTimer_ = -1;  // Disabled
    shroudTimer_ = 0;

    playerHouse_ = HousesType::GREECE;
    difficulty_ = DifficultyType::NORMAL;
    computerDifficulty_ = DifficultyType::NORMAL;

    carryOverMoney_ = 0;
    carryOverCap_ = 0;
    carryOverPercent_ = 0;
    buildPercent_ = 100;

    for (int i = 0; i < WAYPT_COUNT; i++) {
        waypoints_[i] = -1;  // Invalid
    }

    for (int i = 0; i < GLOBAL_FLAG_COUNT; i++) {
        globalFlags_[i] = false;
    }

    isToCarryOver_ = false;
    isToInherit_ = false;
    isInheritTimer_ = false;
    isEndOfGame_ = false;
    isOneTimeOnly_ = false;
    isNoMapSel_ = false;
    isTanyaEvac_ = false;
    isSkipScore_ = false;
    isNoSpyPlane_ = false;
    isTruckCrate_ = false;
    isMoneyTiberium_ = false;
    isBridgeDestroyed_ = false;
    isVariant_ = false;
}

void ScenarioClass::Clear() {
    Init();

    // Clear all game objects
    Init_TriggerTypes();
    Init_Triggers();
    Init_TeamTypes();
    Init_Teams();
    Init_Houses();
}

bool ScenarioClass::Read_INI(const char* filename) {
    if (!filename || !filename[0]) return false;

    INIClass ini;
    if (!ini.Load(filename)) {
        return false;
    }

    // [Basic] section
    int sz = sizeof(description_);
    ini.GetString("Basic", "Name", "Mission", description_, sz);

    // Theater
    char theaterStr[32];
    ini.GetString("Basic", "Theater", "TEMPERATE", theaterStr, 32);
    theater_ = TheaterFromName(theaterStr);

    // Player house
    char playerStr[32];
    ini.GetString("Basic", "Player", "Greece", playerStr, sizeof(playerStr));
    playerHouse_ = HouseTypeFromName(playerStr);

    // Movies (would parse VQ type from string)
    // For now, leave as NONE - would need VQ name lookup

    // Theme (would parse theme type from string)
    // For now, leave as NONE

    // Financial
    carryOverMoney_ = ini.GetInt("Basic", "CarryOverMoney", 0);
    carryOverCap_ = ini.GetInt("Basic", "CarryOverCap", 0);
    carryOverPercent_ = ini.GetInt("Basic", "Percent", 100);
    buildPercent_ = ini.GetInt("Basic", "BuildLevel", 100);

    // Flags
    isToCarryOver_ = ini.GetBool("Basic", "ToCarryOver", false);
    isToInherit_ = ini.GetBool("Basic", "ToInherit", false);
    isInheritTimer_ = ini.GetBool("Basic", "TimerInherit", false);
    isEndOfGame_ = ini.GetBool("Basic", "EndOfGame", false);
    isOneTimeOnly_ = ini.GetBool("Basic", "OneTimeOnly", false);
    isNoMapSel_ = ini.GetBool("Basic", "SkipMapSelect", false);
    isTanyaEvac_ = ini.GetBool("Basic", "CivEvac", false);
    isSkipScore_ = ini.GetBool("Basic", "SkipScore", false);
    isNoSpyPlane_ = ini.GetBool("Basic", "NoSpyPlane", false);
    isTruckCrate_ = ini.GetBool("Basic", "TruckCrate", false);
    isMoneyTiberium_ = ini.GetBool("Basic", "FillSilos", false);

    // [Waypoints] section
    for (int i = 0; i < 26; i++) {  // A-Z
        char key[16];
        snprintf(key, sizeof(key), "%d", i);
        int cell = ini.GetInt("Waypoints", key, -1);
        if (cell >= 0) {
            waypoints_[i] = static_cast<int16_t>(cell);
        }
    }

    // Special waypoints
    int home = ini.GetInt("Waypoints", "Home", -1);
    int reinf = ini.GetInt("Waypoints", "Reinf", -1);
    int special = ini.GetInt("Waypoints", "Special", -1);
    int flare = ini.GetInt("Waypoints", "Flare", -1);
    waypoints_[WAYPT_HOME] = static_cast<int16_t>(home);
    waypoints_[WAYPT_REINF] = static_cast<int16_t>(reinf);
    waypoints_[WAYPT_SPECIAL] = static_cast<int16_t>(special);
    waypoints_[WAYPT_FLARE] = static_cast<int16_t>(flare);

    // Store filename
    strncpy(name_, filename, sizeof(name_) - 1);
    name_[sizeof(name_) - 1] = '\0';

    return true;
}

bool ScenarioClass::Start(bool showBriefing) {
    (void)showBriefing;  // Would play briefing movie

    // Reset elapsed time
    elapsedTime_ = 0;

    return true;
}

int16_t ScenarioClass::Get_Waypoint(int index) const {
    if (index < 0 || index >= WAYPT_COUNT) return -1;
    return waypoints_[index];
}

void ScenarioClass::Set_Waypoint(int index, int16_t cell) {
    if (index >= 0 && index < WAYPT_COUNT) {
        waypoints_[index] = cell;
    }
}

int16_t ScenarioClass::Get_Waypoint_Cell(char letter) const {
    if (letter >= 'A' && letter <= 'Z') {
        return waypoints_[letter - 'A'];
    }
    if (letter >= 'a' && letter <= 'z') {
        return waypoints_[letter - 'a'];
    }
    return -1;
}

bool ScenarioClass::Get_Global(int index) const {
    if (index < 0 || index >= GLOBAL_FLAG_COUNT) return false;
    return globalFlags_[index];
}

void ScenarioClass::Set_Global(int index, bool value) {
    if (index < 0 || index >= GLOBAL_FLAG_COUNT) return;

    bool oldValue = globalFlags_[index];
    globalFlags_[index] = value;

    // If value changed, trigger global flag events
    if (oldValue != value) {
        int16_t idx = static_cast<int16_t>(index);
        TEventType evt = value ? TEventType::GLOBAL_SET
                               : TEventType::GLOBAL_CLEAR;
        Process_Triggers(evt, HousesType::NONE, nullptr, idx);
    }
}

void ScenarioClass::Start_Mission_Timer(int frames) {
    missionTimer_ = frames;
}

void ScenarioClass::Stop_Mission_Timer() {
    missionTimer_ = -1;
}

void ScenarioClass::Add_Mission_Timer(int frames) {
    if (missionTimer_ >= 0) {
        missionTimer_ += frames;
    }
}

void ScenarioClass::Sub_Mission_Timer(int frames) {
    if (missionTimer_ >= 0) {
        missionTimer_ -= frames;
        if (missionTimer_ < 0) {
            missionTimer_ = 0;
        }
    }
}

void ScenarioClass::AI() {
    // Increment elapsed time
    elapsedTime_++;

    // Process mission timer
    if (missionTimer_ > 0) {
        missionTimer_--;
        if (missionTimer_ == 0) {
            // Timer expired - trigger event
            Process_Triggers(TEventType::MISSION_TIMER_EXPIRED);
        }
    }

    // Process shroud regrowth timer
    if (shroudTimer_ > 0) {
        shroudTimer_--;
        // Would regrow shroud when timer expires
    }

    // Process time-based triggers
    Process_Triggers(TEventType::TIME);
}

//===========================================================================
// Helper Functions
//===========================================================================

void Set_Scenario_Name(int scenario, TheaterType theater,
                       SideType side, bool isAftermathSC) {
    const char* fn = Scenario_Filename(scenario, theater, side,
                                       isAftermathSC);
    strncpy(Scen.name_, fn, sizeof(Scen.name_) - 1);
    Scen.name_[sizeof(Scen.name_) - 1] = '\0';
    Scen.scenario_ = scenario;
    Scen.theater_ = theater;
}

const char* Scenario_Filename(int scenario, TheaterType theater,
                              SideType side, bool isAftermathSC) {
    static char filename[32];

    // Format: SCG01EA.INI or SCU01EA.INI
    // SC = Scenario
    // G/U = GoodGuy(Allied)/Ukraine(Soviet)
    // 01 = Mission number
    // E = East (could be W, N, S for variations)
    // A = Aftermath expansion (blank for original)

    char sideChar = (side == SideType::ALLIED) ? 'G' : 'U';
    char theaterChar;

    switch (theater) {
        case TheaterType::SNOW: theaterChar = 'W'; break;
        case TheaterType::INTERIOR: theaterChar = 'I'; break;
        default: theaterChar = 'E'; break;
    }

    if (isAftermathSC) {
        snprintf(filename, sizeof(filename), "SC%c%02d%cA.INI",
                 sideChar, scenario, theaterChar);
    } else {
        snprintf(filename, sizeof(filename), "SC%c%02d%c.INI",
                 sideChar, scenario, theaterChar);
    }

    return filename;
}
