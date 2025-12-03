/**
 * Red Alert macOS Port - Mission Loader Implementation
 */

#include "mission.h"
#include "ini.h"
#include "map.h"
#include "units.h"
#include "ai.h"
#include "terrain.h"
#include "../assets/lcw.h"
#include "../assets/assetloader.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

// Forward declaration for house production control
// (Can't include house.h due to type conflicts with units.h/mission.h)
// Implemented in house_bridge.cpp
extern void EnableAIProduction(int houseIndex);

// Forward declaration for AI autocreate (sets isAlerted_ on house)
// Implemented in house_bridge.cpp
extern void EnableAIAutocreate(int houseIndex);

// Forward declarations for destroying units/buildings by trigger
// Implemented in units.cpp
extern void Units_DestroyByTrigger(const char* triggerName);
extern void Buildings_DestroyByTrigger(const char* triggerName);

// Forward declarations for trigger event helper functions
// Implemented in units.cpp (C linkage) - declared in units.h
// Just use casts where needed for HouseType param

// Parsed trigger storage (simplified - stores raw INI data)
// These will be used to create proper trigger instances when type systems merge
struct ParsedTrigger {
    char name[24];
    int persist;        // 0=volatile, 1=semi, 2=persistent
    int house;          // Country number
    int eventControl;   // 0=only, 1=and, 2=or, 3=linked
    int actionControl;  // 0=only, 1=and
    int event1, e1p1, e1p2;
    int event2, e2p1, e2p2;
    int action1, a1p1, a1p2, a1p3;
    int action2, a2p1, a2p2, a2p3;
    bool active;

    // Event state flags (set by objects when events occur)
    bool wasAttacked;   // Object with this trigger was attacked
    bool wasDestroyed;  // Object with this trigger was destroyed
    bool wasEvacuated;  // Civilian was evacuated
};

#define MAX_PARSED_TRIGGERS 80
static ParsedTrigger g_parsedTriggers[MAX_PARSED_TRIGGERS];
static int g_parsedTriggerCount = 0;

// Global flags for trigger system (up to 32 flags)
#define MAX_GLOBAL_FLAGS 32
static bool g_globalFlags[MAX_GLOBAL_FLAGS] = {false};

// Mission timer state
static bool g_missionTimerActive = false;
static int g_missionTimerValue = 0;      // Current timer value in frames
static int g_missionTimerInitial = 0;    // Initial value (for display)

// Team unit tracking - maps team type index to list of spawned unit IDs
#define MAX_TEAM_TRACK 32
#define MAX_UNITS_PER_TEAM 32
struct TeamTrack {
    int teamTypeIndex;          // Index into mission->teamTypes[]
    int unitIds[MAX_UNITS_PER_TEAM];
    int unitCount;
    bool active;
};
static TeamTrack g_teamTracks[MAX_TEAM_TRACK];
static int g_teamTrackCount = 0;

// Mission text display state
#define MAX_MISSION_TEXT 256
static char g_missionText[MAX_MISSION_TEXT];
static int g_missionTextTimer = 0;  // Frames remaining to display

// Drop zone flare state
struct DropZoneFlare {
    int worldX, worldY;
    int timer;  // Frames remaining
    bool active;
};
#define MAX_DZ_FLARES 8
static DropZoneFlare g_dzFlares[MAX_DZ_FLARES];

// Forward declarations for VQA playback and AI sell
extern bool VQA_Play(const char* filename);
extern void AI_SellAllBuildings(int houseIndex);

// ============================================================================
// Trigger Event Notification Functions
// Called by objects when trigger-related events occur
// ============================================================================

/**
 * Find a trigger by name and return its index, or -1 if not found
 */
static int FindTriggerByName(const char* name) {
    if (!name || name[0] == '\0') return -1;
    for (int i = 0; i < g_parsedTriggerCount; i++) {
        if (strcasecmp(g_parsedTriggers[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/**
 * Notify that an object with the given trigger was attacked
 * Called from TechnoClass::TakeDamage when object takes damage
 */
void Mission_TriggerAttacked(const char* triggerName) {
    int idx = FindTriggerByName(triggerName);
    if (idx >= 0) {
        g_parsedTriggers[idx].wasAttacked = true;
    }
}

/**
 * Notify that an object with the given trigger was destroyed
 * Called from Record_The_Kill when object dies
 */
void Mission_TriggerDestroyed(const char* triggerName) {
    int idx = FindTriggerByName(triggerName);
    if (idx >= 0) {
        g_parsedTriggers[idx].wasDestroyed = true;
    }
}

/**
 * Notify that a civilian with the given trigger was evacuated
 * Called when civilian reaches map edge or extraction point
 */
void Mission_TriggerEvacuated(const char* triggerName) {
    int idx = FindTriggerByName(triggerName);
    if (idx >= 0) {
        g_parsedTriggers[idx].wasEvacuated = true;
    }
}

// ============================================================================
// Mission Timer Functions
// ============================================================================

bool Mission_IsTimerActive(void) {
    return g_missionTimerActive;
}

int Mission_GetTimerValue(void) {
    return g_missionTimerValue;
}

void Mission_UpdateTimer(void) {
    if (g_missionTimerActive && g_missionTimerValue > 0) {
        g_missionTimerValue--;
    }
}

void Mission_ResetTimer(void) {
    g_missionTimerActive = false;
    g_missionTimerValue = 0;
    g_missionTimerInitial = 0;
}

// ============================================================================
// Team Tracking Functions
// ============================================================================

static void ResetTeamTracking(void) {
    for (int i = 0; i < MAX_TEAM_TRACK; i++) {
        g_teamTracks[i].active = false;
        g_teamTracks[i].unitCount = 0;
    }
    g_teamTrackCount = 0;
}

static TeamTrack* FindOrCreateTeamTrack(int teamTypeIndex) {
    // Find existing
    for (int i = 0; i < g_teamTrackCount; i++) {
        if (g_teamTracks[i].active &&
            g_teamTracks[i].teamTypeIndex == teamTypeIndex) {
            return &g_teamTracks[i];
        }
    }
    // Create new
    if (g_teamTrackCount < MAX_TEAM_TRACK) {
        TeamTrack* track = &g_teamTracks[g_teamTrackCount++];
        track->teamTypeIndex = teamTypeIndex;
        track->unitCount = 0;
        track->active = true;
        return track;
    }
    return nullptr;
}

static void TrackTeamUnit(int teamTypeIndex, int unitId) {
    TeamTrack* track = FindOrCreateTeamTrack(teamTypeIndex);
    if (track && track->unitCount < MAX_UNITS_PER_TEAM) {
        track->unitIds[track->unitCount++] = unitId;
    }
}

static void DestroyTeamUnits(int teamTypeIndex) {
    for (int i = 0; i < g_teamTrackCount; i++) {
        if (g_teamTracks[i].active &&
            g_teamTracks[i].teamTypeIndex == teamTypeIndex) {
            // Remove all units in this team
            for (int j = 0; j < g_teamTracks[i].unitCount; j++) {
                Units_Remove(g_teamTracks[i].unitIds[j]);
            }
            g_teamTracks[i].active = false;
            g_teamTracks[i].unitCount = 0;
            fprintf(stderr, "    Destroyed team %d units\n", teamTypeIndex);
            return;
        }
    }
}

// ============================================================================
// Drop Zone Flare Functions
// ============================================================================

static void ResetDropZoneFlares(void) {
    for (int i = 0; i < MAX_DZ_FLARES; i++) {
        g_dzFlares[i].active = false;
    }
}

static void AddDropZoneFlare(int worldX, int worldY) {
    for (int i = 0; i < MAX_DZ_FLARES; i++) {
        if (!g_dzFlares[i].active) {
            g_dzFlares[i].worldX = worldX;
            g_dzFlares[i].worldY = worldY;
            g_dzFlares[i].timer = 15 * 10;  // 10 seconds at 15 fps
            g_dzFlares[i].active = true;
            return;
        }
    }
}

int Mission_GetDropZoneFlare(int index, int* worldX, int* worldY) {
    if (index < 0 || index >= MAX_DZ_FLARES) return 0;
    if (!g_dzFlares[index].active) return 0;
    if (worldX) *worldX = g_dzFlares[index].worldX;
    if (worldY) *worldY = g_dzFlares[index].worldY;
    return 1;
}

void Mission_UpdateDropZoneFlares(void) {
    for (int i = 0; i < MAX_DZ_FLARES; i++) {
        if (g_dzFlares[i].active) {
            g_dzFlares[i].timer--;
            if (g_dzFlares[i].timer <= 0) {
                g_dzFlares[i].active = false;
            }
        }
    }
}

// ============================================================================
// Mission Text Functions
// ============================================================================

static void SetMissionText(const char* text, int duration) {
    strncpy(g_missionText, text, MAX_MISSION_TEXT - 1);
    g_missionText[MAX_MISSION_TEXT - 1] = '\0';
    g_missionTextTimer = duration;
}

const char* Mission_GetDisplayText(void) {
    return g_missionTextTimer > 0 ? g_missionText : nullptr;
}

void Mission_UpdateDisplayText(void) {
    if (g_missionTextTimer > 0) {
        g_missionTextTimer--;
    }
}

void Mission_Init(MissionData* mission) {
    if (!mission) return;

    // Reset trigger-related state
    ResetTeamTracking();
    ResetDropZoneFlares();
    g_missionText[0] = '\0';
    g_missionTextTimer = 0;

    memset(mission, 0, sizeof(MissionData));
    strncpy(mission->name, "Untitled", sizeof(mission->name) - 1);
    strncpy(mission->description, "No description",
            sizeof(mission->description) - 1);
    mission->theater = 0;  // Temperate
    mission->mapWidth = 64;
    mission->mapHeight = 64;
    mission->playerTeam = TEAM_PLAYER;
    mission->startCredits = 5000;
    mission->winCondition = 0;  // Destroy all enemies
    mission->loseCondition = 0; // Lose all units
    mission->timeLimit = 0;     // Unlimited
    mission->targetCell = -1;   // No specific target
    mission->baseHouse = -1;    // No base
    mission->baseCount = 0;
    mission->terrainType = nullptr;
    mission->terrainIcon = nullptr;
    mission->overlayType = nullptr;
    mission->overlayData = nullptr;
}

void Mission_Free(MissionData* mission) {
    if (!mission) return;

    if (mission->terrainType) {
        free(mission->terrainType);
        mission->terrainType = nullptr;
    }
    if (mission->terrainIcon) {
        free(mission->terrainIcon);
        mission->terrainIcon = nullptr;
    }
    if (mission->overlayType) {
        free(mission->overlayType);
        mission->overlayType = nullptr;
    }
    if (mission->overlayData) {
        free(mission->overlayData);
        mission->overlayData = nullptr;
    }
}

// Parse unit type from string
static UnitType ParseUnitType(const char* str) {
    if (!str) return UNIT_NONE;

    // Infantry - Military
    if (strcasecmp(str, "E1") == 0) return UNIT_RIFLE;
    if (strcasecmp(str, "E2") == 0) return UNIT_GRENADIER;
    if (strcasecmp(str, "E3") == 0) return UNIT_ROCKET;
    if (strcasecmp(str, "E4") == 0) return UNIT_FLAMETHROWER;
    if (strcasecmp(str, "E6") == 0) return UNIT_ENGINEER;
    if (strcasecmp(str, "E7") == 0) return UNIT_TANYA;
    bool isSpy = strcasecmp(str, "E5") == 0 || strcasecmp(str, "SPY") == 0;
    if (isSpy) return UNIT_SPY;
    if (strcasecmp(str, "DOG") == 0) return UNIT_DOG;
    if (strcasecmp(str, "MEDI") == 0) return UNIT_MEDIC;
    if (strcasecmp(str, "THF") == 0) return UNIT_THIEF;
    if (strcasecmp(str, "SHOK") == 0) return UNIT_SHOCK;
    if (strcasecmp(str, "GNRL") == 0) return UNIT_GENERAL;

    // Infantry - Civilians
    if (strcasecmp(str, "C1") == 0) return UNIT_CIVILIAN_1;
    if (strcasecmp(str, "C2") == 0) return UNIT_CIVILIAN_2;
    if (strcasecmp(str, "C3") == 0) return UNIT_CIVILIAN_3;
    if (strcasecmp(str, "C4") == 0) return UNIT_CIVILIAN_4;
    if (strcasecmp(str, "C5") == 0) return UNIT_CIVILIAN_5;
    if (strcasecmp(str, "C6") == 0) return UNIT_CIVILIAN_6;
    if (strcasecmp(str, "C7") == 0) return UNIT_CIVILIAN_7;
    if (strcasecmp(str, "C8") == 0) return UNIT_CIVILIAN_8;
    if (strcasecmp(str, "C9") == 0) return UNIT_CIVILIAN_9;
    if (strcasecmp(str, "C10") == 0) return UNIT_CIVILIAN_10;
    if (strcasecmp(str, "CHAN") == 0) return UNIT_CHAN;
    if (strcasecmp(str, "EINSTEIN") == 0) return UNIT_CIVILIAN_8;

    // Vehicles
    if (strcasecmp(str, "HARV") == 0) return UNIT_HARVESTER;
    if (strcasecmp(str, "1TNK") == 0) return UNIT_TANK_LIGHT;
    if (strcasecmp(str, "2TNK") == 0) return UNIT_TANK_MEDIUM;
    if (strcasecmp(str, "3TNK") == 0) return UNIT_TANK_HEAVY;
    if (strcasecmp(str, "4TNK") == 0) return UNIT_TANK_MAMMOTH;
    if (strcasecmp(str, "APC") == 0) return UNIT_APC;
    if (strcasecmp(str, "ARTY") == 0) return UNIT_ARTILLERY;
    if (strcasecmp(str, "JEEP") == 0) return UNIT_JEEP;
    if (strcasecmp(str, "MCV") == 0) return UNIT_MCV;
    if (strcasecmp(str, "V2RL") == 0) return UNIT_V2RL;
    if (strcasecmp(str, "MNLY") == 0) return UNIT_MINELAYER;
    if (strcasecmp(str, "TRUK") == 0) return UNIT_TRUCK;
    if (strcasecmp(str, "CTNK") == 0) return UNIT_CHRONO;
    if (strcasecmp(str, "MGG") == 0) return UNIT_MOBILE_GAP;
    if (strcasecmp(str, "MRJ") == 0) return UNIT_MOBILE_RADAR;

    // Naval
    if (strcasecmp(str, "GNBT") == 0) return UNIT_GUNBOAT;
    if (strcasecmp(str, "DD") == 0) return UNIT_DESTROYER;
    if (strcasecmp(str, "SS") == 0) return UNIT_SUBMARINE;
    if (strcasecmp(str, "CA") == 0) return UNIT_CRUISER;
    if (strcasecmp(str, "LST") == 0) return UNIT_TRANSPORT;
    if (strcasecmp(str, "PT") == 0) return UNIT_PT_BOAT;

    // Aircraft
    if (strcasecmp(str, "HIND") == 0) return UNIT_HIND;
    if (strcasecmp(str, "HELI") == 0) return UNIT_LONGBOW;
    if (strcasecmp(str, "TRAN") == 0) return UNIT_CHINOOK;
    if (strcasecmp(str, "YAK") == 0) return UNIT_YAK;
    if (strcasecmp(str, "MIG") == 0) return UNIT_MIG;

    // Unknown unit type
    fprintf(stderr, "WARNING: Unknown unit type '%s'\n", str);
    return UNIT_NONE;
}

// Parse building type from string
static BuildingType ParseBuildingType(const char* str) {
    if (!str) return BUILDING_NONE;

    // Core structures
    if (strcasecmp(str, "FACT") == 0) return BUILDING_CONSTRUCTION;
    if (strcasecmp(str, "POWR") == 0) return BUILDING_POWER;
    if (strcasecmp(str, "APWR") == 0) return BUILDING_ADV_POWER;
    if (strcasecmp(str, "PROC") == 0) return BUILDING_REFINERY;
    if (strcasecmp(str, "SILO") == 0) return BUILDING_SILO;

    // Production
    bool isBarr = strcasecmp(str, "TENT") == 0;
    isBarr = isBarr || strcasecmp(str, "BARR") == 0;
    if (isBarr) return BUILDING_BARRACKS;
    if (strcasecmp(str, "WEAP") == 0) return BUILDING_FACTORY;
    if (strcasecmp(str, "AFLD") == 0) return BUILDING_AIRFIELD;
    if (strcasecmp(str, "HPAD") == 0) return BUILDING_HELIPAD;
    if (strcasecmp(str, "SYRD") == 0) return BUILDING_SHIPYARD;
    if (strcasecmp(str, "SPEN") == 0) return BUILDING_SUB_PEN;

    // Tech
    if (strcasecmp(str, "DOME") == 0) return BUILDING_RADAR;
    bool isTech = strcasecmp(str, "ATEK") == 0;
    isTech = isTech || strcasecmp(str, "STEK") == 0;
    if (isTech) return BUILDING_TECH_CENTER;
    if (strcasecmp(str, "KENN") == 0) return BUILDING_KENNEL;
    if (strcasecmp(str, "BIO") == 0) return BUILDING_BIO_LAB;
    if (strcasecmp(str, "FCOM") == 0) return BUILDING_FORWARD_COM;
    if (strcasecmp(str, "MISS") == 0) return BUILDING_MISSION;

    // Defense
    if (strcasecmp(str, "GUN") == 0) return BUILDING_TURRET;
    if (strcasecmp(str, "SAM") == 0) return BUILDING_SAM;
    if (strcasecmp(str, "TSLA") == 0) return BUILDING_TESLA;
    if (strcasecmp(str, "AGUN") == 0) return BUILDING_AA_GUN;
    if (strcasecmp(str, "PBOX") == 0) return BUILDING_PILLBOX;
    if (strcasecmp(str, "HBOX") == 0) return BUILDING_CAMO_PILLBOX;
    if (strcasecmp(str, "FTUR") == 0) return BUILDING_FLAME_TOWER;
    if (strcasecmp(str, "GAP") == 0) return BUILDING_GAP;
    if (strcasecmp(str, "MINP") == 0) return BUILDING_MINE_AP;
    if (strcasecmp(str, "MINV") == 0) return BUILDING_MINE_AV;

    // Special
    if (strcasecmp(str, "FIX") == 0) return BUILDING_FIX;
    if (strcasecmp(str, "IRON") == 0) return BUILDING_IRON_CURTAIN;
    if (strcasecmp(str, "PDOX") == 0) return BUILDING_CHRONOSPHERE;
    if (strcasecmp(str, "MSLO") == 0) return BUILDING_MISSILE_SILO;

    // Fake structures
    if (strcasecmp(str, "FACF") == 0) return BUILDING_FAKE_CONST;
    if (strcasecmp(str, "WEAF") == 0) return BUILDING_FAKE_FACTORY;
    if (strcasecmp(str, "SYRF") == 0) return BUILDING_FAKE_SHIPYARD;
    if (strcasecmp(str, "DOMF") == 0) return BUILDING_FAKE_RADAR;

    // Props
    if (strcasecmp(str, "BARL") == 0) return BUILDING_BARREL;
    if (strcasecmp(str, "BRL3") == 0) return BUILDING_BARREL_3;

    // Civilian buildings (V01-V19)
    if (strcasecmp(str, "V01") == 0) return BUILDING_CIV_01;
    if (strcasecmp(str, "V02") == 0) return BUILDING_CIV_02;
    if (strcasecmp(str, "V03") == 0) return BUILDING_CIV_03;
    if (strcasecmp(str, "V04") == 0) return BUILDING_CIV_04;
    if (strcasecmp(str, "V05") == 0) return BUILDING_CIV_05;
    if (strcasecmp(str, "V06") == 0) return BUILDING_CIV_06;
    if (strcasecmp(str, "V07") == 0) return BUILDING_CIV_07;
    if (strcasecmp(str, "V08") == 0) return BUILDING_CIV_08;
    if (strcasecmp(str, "V09") == 0) return BUILDING_CIV_09;
    if (strcasecmp(str, "V10") == 0) return BUILDING_CIV_10;
    if (strcasecmp(str, "V11") == 0) return BUILDING_CIV_11;
    if (strcasecmp(str, "V13") == 0) return BUILDING_CIV_13;
    if (strcasecmp(str, "V19") == 0) return BUILDING_CIV_19;

    return BUILDING_NONE;
}

// Parse team from string
static Team ParseTeam(const char* str) {
    if (!str) return TEAM_NEUTRAL;

    // Player side names (Allies)
    if (strcasecmp(str, "Greece") == 0 ||
        strcasecmp(str, "England") == 0 ||
        strcasecmp(str, "France") == 0 ||
        strcasecmp(str, "Germany") == 0 ||
        strcasecmp(str, "Spain") == 0 ||
        strcasecmp(str, "Turkey") == 0 ||
        strcasecmp(str, "GoodGuy") == 0 ||
        strcasecmp(str, "Player") == 0 ||
        strcasecmp(str, "Allies") == 0) {
        return TEAM_PLAYER;
    }

    // Enemy side names (Soviet)
    if (strcasecmp(str, "USSR") == 0 ||
        strcasecmp(str, "Ukraine") == 0 ||
        strcasecmp(str, "BadGuy") == 0 ||
        strcasecmp(str, "Enemy") == 0 ||
        strcasecmp(str, "Soviet") == 0) {
        return TEAM_ENEMY;
    }

    return TEAM_NEUTRAL;
}

// Parse mission type from string
static MissionType ParseMissionType(const char* str) {
    if (!str) return MISSION_GUARD;

    if (strcasecmp(str, "Guard") == 0) return MISSION_GUARD;
    if (strcasecmp(str, "Hunt") == 0) return MISSION_HUNT;
    if (strcasecmp(str, "Sleep") == 0) return MISSION_SLEEP;
    if (strcasecmp(str, "Harvest") == 0) return MISSION_HARVEST;
    if (strcasecmp(str, "Attack") == 0) return MISSION_ATTACK;
    if (strcasecmp(str, "Area Guard") == 0) return MISSION_GUARD_AREA;
    if (strcasecmp(str, "Retreat") == 0) return MISSION_RETREAT;
    if (strcasecmp(str, "None") == 0) return MISSION_NONE;
    if (strcasecmp(str, "Stop") == 0) return MISSION_NONE;

    return MISSION_GUARD;  // Default
}

// Parse house name to house number (for TeamTypes, Base sections)
// Houses: 0=Spain, 1=Greece, 2=USSR, 3=England, 4=Ukraine, 5-7=DE/FR/TR
static int ParseHouseName(const char* str) {
    if (!str) return -1;

    if (strcasecmp(str, "Spain") == 0) return 0;
    if (strcasecmp(str, "Greece") == 0) return 1;
    if (strcasecmp(str, "USSR") == 0) return 2;
    if (strcasecmp(str, "England") == 0) return 3;
    if (strcasecmp(str, "Ukraine") == 0) return 4;
    if (strcasecmp(str, "Germany") == 0) return 5;
    if (strcasecmp(str, "France") == 0) return 6;
    if (strcasecmp(str, "Turkey") == 0) return 7;

    // Aliases
    if (strcasecmp(str, "GoodGuy") == 0) return 1;  // Greece
    if (strcasecmp(str, "BadGuy") == 0) return 2;   // USSR
    if (strcasecmp(str, "Neutral") == 0) return 0;  // Spain (often neutral)
    if (strcasecmp(str, "Special") == 0) return 0;

    return -1;
}

// Convert cell to X/Y (Red Alert uses 128-wide maps internally)
#define CELL_TO_X(cell) ((cell) % 128)
#define CELL_TO_Y(cell) ((cell) / 128)

// ============================================================================
// INI Section Parsers - each parses one section of mission INI
// ============================================================================

// Forward declaration for pack section parser
static uint8_t* ParsePackSection(INIClass* ini, const char* section,
                                  int* outSize);

// Parse [Basic] section - mission name, player, brief/win/lose videos
static void ParseBasicSection(MissionData* mission, INIClass* ini) {
    ini->GetString("Basic", "Name", "Mission",
                   mission->name, sizeof(mission->name));

    char playerStr[32];
    ini->GetString("Basic", "Player", "Greece", playerStr, sizeof(playerStr));
    mission->playerTeam = ParseTeam(playerStr);

    ini->GetString("Basic", "Brief", "",
                   mission->briefVideo, sizeof(mission->briefVideo));
    ini->GetString("Basic", "Win", "",
                   mission->winVideo, sizeof(mission->winVideo));
    ini->GetString("Basic", "Lose", "",
                   mission->loseVideo, sizeof(mission->loseVideo));
}

// Parse [Map] section - theater and dimensions
static void ParseMapSection(MissionData* mission, INIClass* ini) {
    char theaterStr[32];
    int sz = sizeof(theaterStr);
    ini->GetString("Map", "Theater", "TEMPERATE", theaterStr, sz);
    if (strcasecmp(theaterStr, "SNOW") == 0) mission->theater = 1;
    else if (strcasecmp(theaterStr, "INTERIOR") == 0) mission->theater = 2;
    else if (strcasecmp(theaterStr, "DESERT") == 0) mission->theater = 3;
    else mission->theater = 0;

    mission->mapX = ini->GetInt("Map", "X", 0);
    mission->mapY = ini->GetInt("Map", "Y", 0);
    mission->mapWidth = ini->GetInt("Map", "Width", 64);
    mission->mapHeight = ini->GetInt("Map", "Height", 64);
}

// Parse [Briefing] section - concatenate all lines
static void ParseBriefingSection(MissionData* mission, INIClass* ini) {
    int briefCount = ini->EntryCount("Briefing");
    mission->description[0] = '\0';
    size_t descLen = 0;

    for (int i = 0; i < briefCount && i < 10; i++) {
        char lineKey[8];
        snprintf(lineKey, sizeof(lineKey), "%d", i + 1);
        char line[256];
        ini->GetString("Briefing", lineKey, "", line, sizeof(line));

        if (strlen(line) > 0 && descLen < sizeof(mission->description) - 2) {
            size_t lineLen = strlen(line);
            size_t remaining = sizeof(mission->description) - descLen - 1;
            if (lineLen < remaining) {
                if (descLen > 0) mission->description[descLen++] = ' ';
                strncpy(mission->description + descLen, line, remaining);
                descLen += lineLen;
            }
        }
    }
}

// Parse [UNITS] section - vehicles
static void ParseUnitsSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("UNITS");
    for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
        const char* entry = ini->GetEntry("UNITS", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("UNITS", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, facing;
        missionStr[0] = trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing,
                   missionStr, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);
            unit->health = (int16_t)health;
            unit->facing = (int16_t)facing;
            unit->mission = ParseMissionType(missionStr);
            unit->subCell = 0;
            // Store trigger name (skip "None")
            if (trigger[0] != '\0' && strcasecmp(trigger, "None") != 0) {
                strncpy(unit->triggerName, trigger, sizeof(unit->triggerName) - 1);
                unit->triggerName[sizeof(unit->triggerName) - 1] = '\0';
            } else {
                unit->triggerName[0] = '\0';
            }

            if (unit->type != UNIT_NONE) mission->unitCount++;
        }
    }
}

// Parse [STRUCTURES] section - buildings
static void ParseStructuresSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("STRUCTURES");
    for (int i = 0; i < count && mission->buildingCount < MAX_MISSION_BUILDINGS;
         i++) {
        const char* entry = ini->GetEntry("STRUCTURES", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("STRUCTURES", entry, "", value, sizeof(value));

        char house[32], type[32], trigger[32];
        int health, cell, facing, sellable = 1, rebuild = 0;
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%d",
                   house, type, &health, &cell, &facing,
                   trigger, &sellable, &rebuild) >= 4) {
            MissionBuilding* bld = &mission->buildings[mission->buildingCount];
            bld->type = ParseBuildingType(type);
            bld->team = ParseTeam(house);
            bld->cellX = CELL_TO_X(cell);
            bld->cellY = CELL_TO_Y(cell);
            bld->health = (int16_t)health;
            bld->facing = (int16_t)facing;
            bld->sellable = (int8_t)sellable;
            bld->rebuild = (int8_t)rebuild;
            // Store trigger name (skip "None")
            if (trigger[0] != '\0' && strcasecmp(trigger, "None") != 0) {
                strncpy(bld->triggerName, trigger, sizeof(bld->triggerName) - 1);
                bld->triggerName[sizeof(bld->triggerName) - 1] = '\0';
            } else {
                bld->triggerName[0] = '\0';
            }

            if (bld->type != BUILDING_NONE) mission->buildingCount++;
        }
    }
}

// Parse [INFANTRY] section
static void ParseInfantrySection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("INFANTRY");
    for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
        const char* entry = ini->GetEntry("INFANTRY", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("INFANTRY", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, subCell, facing = 0;
        missionStr[0] = trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%31s",
                   house, type, &health, &cell, &subCell,
                   missionStr, &facing, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);
            unit->health = (int16_t)health;
            unit->facing = (int16_t)facing;
            unit->mission = ParseMissionType(missionStr);
            unit->subCell = (int16_t)subCell;
            // Store trigger name (skip "None")
            if (trigger[0] != '\0' && strcasecmp(trigger, "None") != 0) {
                strncpy(unit->triggerName, trigger, sizeof(unit->triggerName) - 1);
                unit->triggerName[sizeof(unit->triggerName) - 1] = '\0';
            } else {
                unit->triggerName[0] = '\0';
            }

            if (unit->type != UNIT_NONE) mission->unitCount++;
        }
    }
}

// Parse [SHIPS] section - naval units (same format as UNITS)
static void ParseShipsSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("SHIPS");
    for (int i = 0; i < count && mission->unitCount < MAX_MISSION_UNITS; i++) {
        const char* entry = ini->GetEntry("SHIPS", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("SHIPS", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, facing;
        missionStr[0] = trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing,
                   missionStr, trigger) >= 5) {
            MissionUnit* unit = &mission->units[mission->unitCount];
            unit->type = ParseUnitType(type);
            unit->team = ParseTeam(house);
            unit->cellX = CELL_TO_X(cell);
            unit->cellY = CELL_TO_Y(cell);
            unit->health = (int16_t)health;
            unit->facing = (int16_t)facing;
            unit->mission = ParseMissionType(missionStr);
            unit->subCell = 0;
            // Store trigger name (skip "None")
            if (trigger[0] != '\0' && strcasecmp(trigger, "None") != 0) {
                strncpy(unit->triggerName, trigger, sizeof(unit->triggerName) - 1);
                unit->triggerName[sizeof(unit->triggerName) - 1] = '\0';
            } else {
                unit->triggerName[0] = '\0';
            }

            if (unit->type != UNIT_NONE) mission->unitCount++;
        }
    }
}

// Parse [Trigs] section - trigger definitions
static void ParseTrigsSection(INIClass* ini) {
    int count = ini->EntryCount("Trigs");
    g_parsedTriggerCount = 0;

    for (int i = 0; i < count && g_parsedTriggerCount < MAX_PARSED_TRIGGERS;
         i++) {
        const char* trigName = ini->GetEntry("Trigs", i);
        if (!trigName) continue;

        char value[256];
        ini->GetString("Trigs", trigName, "", value, sizeof(value));

        int persist, house, eventCtrl, actionCtrl;
        int event1, e1p1, e1p2, event2, e2p1, e2p2;
        int action1, a1p1, a1p2, a1p3, action2, a2p1, a2p2, a2p3;

        int parsed = sscanf(value,
            "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
            &persist, &house, &eventCtrl, &actionCtrl,
            &event1, &e1p1, &e1p2, &event2, &e2p1, &e2p2,
            &action1, &a1p1, &a1p2, &a1p3, &action2, &a2p1, &a2p2, &a2p3);

        if (parsed >= 11) {
            ParsedTrigger* trig = &g_parsedTriggers[g_parsedTriggerCount];
            memset(trig, 0, sizeof(ParsedTrigger));

            strncpy(trig->name, trigName, 23);
            trig->name[23] = '\0';
            trig->active = true;

            trig->persist = persist;
            trig->house = house;
            trig->eventControl = eventCtrl;
            trig->actionControl = actionCtrl;
            trig->event1 = event1;
            trig->e1p1 = e1p1;
            trig->e1p2 = e1p2;
            trig->event2 = event2;
            trig->e2p1 = e2p1;
            trig->e2p2 = e2p2;
            trig->action1 = action1;
            trig->a1p1 = a1p1;
            trig->a1p2 = a1p2;
            trig->a1p3 = a1p3;

            if (parsed >= 18) {
                trig->action2 = action2;
                trig->a2p1 = a2p1;
                trig->a2p2 = a2p2;
                trig->a2p3 = a2p3;
            }

            g_parsedTriggerCount++;
        }
    }
}

// Parse [Waypoints] section
static void ParseWaypointsSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("Waypoints");
    mission->waypointCount = 0;

    for (int i = 0; i < MAX_MISSION_WAYPOINTS; i++) {
        mission->waypoints[i].cell = -1;
        mission->waypoints[i].cellX = -1;
        mission->waypoints[i].cellY = -1;
    }

    for (int i = 0; i < count; i++) {
        const char* entry = ini->GetEntry("Waypoints", i);
        if (!entry) continue;

        int wpNum = atoi(entry);
        if (wpNum < 0 || wpNum >= MAX_MISSION_WAYPOINTS) continue;

        int cell = ini->GetInt("Waypoints", entry, -1);
        if (cell < 0) continue;

        mission->waypoints[wpNum].cell = cell;
        mission->waypoints[wpNum].cellX = CELL_TO_X(cell);
        mission->waypoints[wpNum].cellY = CELL_TO_Y(cell);

        if (wpNum >= mission->waypointCount) {
            mission->waypointCount = wpNum + 1;
        }
    }
}

// Parse [CellTriggers] section - maps cells to trigger names
static void ParseCellTriggersSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("CellTriggers");
    mission->cellTriggerCount = 0;

    for (int i = 0; i < count && mission->cellTriggerCount < 256; i++) {
        const char* entry = ini->GetEntry("CellTriggers", i);
        if (!entry) continue;

        int cell = atoi(entry);
        if (cell < 0) continue;

        char trigName[24];
        ini->GetString("CellTriggers", entry, "", trigName, sizeof(trigName));
        if (trigName[0] == '\0') continue;

        int idx = mission->cellTriggerCount;
        mission->cellTriggerCells[idx] = cell;
        strncpy(mission->cellTriggerNames[idx], trigName, 23);
        mission->cellTriggerNames[idx][23] = '\0';
        mission->cellTriggerCount++;
    }

    if (mission->cellTriggerCount > 0) {
        fprintf(stderr, "Mission: Parsed %d cell triggers\n",
                mission->cellTriggerCount);
    }
}

// Helper: Add an object trigger (cell + trigger name)
static void AddObjectTrigger(MissionData* mission, int cell,
                              const char* trigName) {
    // Skip "None" or empty triggers
    if (!trigName || trigName[0] == '\0') return;
    if (strcasecmp(trigName, "None") == 0) return;

    // Don't exceed array bounds
    if (mission->objectTriggerCount >= 256) return;

    int idx = mission->objectTriggerCount;
    mission->objectTriggerCells[idx] = cell;
    strncpy(mission->objectTriggerNames[idx], trigName, 23);
    mission->objectTriggerNames[idx][23] = '\0';
    mission->objectTriggerCount++;
}

// Parse object triggers from STRUCTURES, UNITS, INFANTRY, SHIPS sections
// This extracts trigger attachments for ENTERED event support
static void ParseObjectTriggersSection(MissionData* mission, INIClass* ini) {
    mission->objectTriggerCount = 0;

    // STRUCTURES format: house,type,health,cell,facing,trigger,sellable,rebuild
    int count = ini->EntryCount("STRUCTURES");
    for (int i = 0; i < count; i++) {
        const char* entry = ini->GetEntry("STRUCTURES", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("STRUCTURES", entry, "", value, sizeof(value));

        char house[32], type[32], trigger[32];
        int health, cell, facing;
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,]",
                   house, type, &health, &cell, &facing, trigger) >= 6) {
            AddObjectTrigger(mission, cell, trigger);
        }
    }

    // UNITS format: house,type,health,cell,facing,mission,trigger
    count = ini->EntryCount("UNITS");
    for (int i = 0; i < count; i++) {
        const char* entry = ini->GetEntry("UNITS", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("UNITS", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, facing;
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing,
                   missionStr, trigger) >= 7) {
            AddObjectTrigger(mission, cell, trigger);
        }
    }

    // INFANTRY format: house,type,health,cell,subcell,mission,facing,trigger
    count = ini->EntryCount("INFANTRY");
    for (int i = 0; i < count; i++) {
        const char* entry = ini->GetEntry("INFANTRY", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("INFANTRY", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, subCell, facing;
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%d,%31s",
                   house, type, &health, &cell, &subCell,
                   missionStr, &facing, trigger) >= 8) {
            AddObjectTrigger(mission, cell, trigger);
        }
    }

    // SHIPS format: house,type,health,cell,facing,mission,trigger
    count = ini->EntryCount("SHIPS");
    for (int i = 0; i < count; i++) {
        const char* entry = ini->GetEntry("SHIPS", i);
        if (!entry) continue;

        char value[128];
        ini->GetString("SHIPS", entry, "", value, sizeof(value));

        char house[32], type[32], missionStr[32], trigger[32];
        int health, cell, facing;
        trigger[0] = '\0';

        if (sscanf(value, "%31[^,],%31[^,],%d,%d,%d,%31[^,],%31s",
                   house, type, &health, &cell, &facing,
                   missionStr, trigger) >= 7) {
            AddObjectTrigger(mission, cell, trigger);
        }
    }

    if (mission->objectTriggerCount > 0) {
        fprintf(stderr, "Mission: Parsed %d object triggers\n",
                mission->objectTriggerCount);
    }
}

// Parse single team type member (type:qty)
static bool ParseTeamMember(char** ptr, TeamMember* member) {
    char* colon = strchr(*ptr, ':');
    if (!colon) return false;

    int typeLen = (int)(colon - *ptr);
    if (typeLen > 7) typeLen = 7;
    strncpy(member->unitType, *ptr, typeLen);
    member->unitType[typeLen] = '\0';

    *ptr = colon + 1;
    char* next;
    member->quantity = strtol(*ptr, &next, 10);
    *ptr = (*next == ',') ? next + 1 : next;
    return true;
}

// Parse single team mission (mission:data)
static bool ParseTeamMission(char** ptr, TeamMission* tmission) {
    char* colon = strchr(*ptr, ':');
    if (!colon) return false;

    char* next;
    tmission->mission = strtol(*ptr, &colon, 10);
    *ptr = colon + 1;
    tmission->data = strtol(*ptr, &next, 10);
    *ptr = (*next == ',') ? next + 1 : next;
    return true;
}

// Parse [TeamTypes] section
static void ParseTeamTypesSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("TeamTypes");
    mission->teamTypeCount = 0;

    for (int i = 0; i < count && mission->teamTypeCount < MAX_TEAM_TYPES; i++) {
        const char* teamName = ini->GetEntry("TeamTypes", i);
        if (!teamName) continue;

        char value[512];
        ini->GetString("TeamTypes", teamName, "", value, sizeof(value));
        if (value[0] == '\0') continue;

        MissionTeamType* team = &mission->teamTypes[mission->teamTypeCount];
        memset(team, 0, sizeof(MissionTeamType));
        strncpy(team->name, teamName, sizeof(team->name) - 1);

        char* ptr = value;
        char* next;

        // Parse fixed fields: house,flags,recruit,init,max,origin,trigger
        team->house = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->flags = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->recruitPriority = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->initNum = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->maxAllowed = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->origin = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;
        team->trigger = strtol(ptr, &next, 10);
        if (*next != ',') continue; ptr = next + 1;

        // numMembers
        int numMembers = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1;

        // Parse members
        team->memberCount = 0;
        for (int m = 0; m < numMembers && m < MAX_TEAM_MEMBERS; m++) {
            if (!ParseTeamMember(&ptr, &team->members[m])) break;
            team->memberCount++;
        }

        // numMissions
        int numMissions = strtol(ptr, &next, 10);
        if (*next == ',') ptr = next + 1;

        // Parse missions
        team->missionCount = 0;
        for (int m = 0; m < numMissions && m < MAX_TEAM_MISSIONS; m++) {
            if (!ParseTeamMission(&ptr, &team->missions[m])) break;
            team->missionCount++;
        }

        mission->teamTypeCount++;
    }
}

// Parse [Base] section
static void ParseBaseSection(MissionData* mission, INIClass* ini) {
    char basePlayer[32];
    ini->GetString("Base", "Player", "", basePlayer, sizeof(basePlayer));
    if (basePlayer[0] != '\0') {
        mission->baseHouse = ParseHouseName(basePlayer);
    }
    mission->baseCount = ini->GetInt("Base", "Count", 0);
}

// Parse [TERRAIN] section - trees and terrain objects
static void ParseTerrainSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("TERRAIN");
    mission->terrainObjCount = 0;

    for (int i = 0; i < count && mission->terrainObjCount < MAX_MISSION_TERRAIN;
         i++) {
        const char* entry = ini->GetEntry("TERRAIN", i);
        if (!entry) continue;

        int cell = atoi(entry);
        if (cell < 0) continue;

        char terrType[16];
        ini->GetString("TERRAIN", entry, "", terrType, sizeof(terrType));
        if (terrType[0] == '\0') continue;

        int idx = mission->terrainObjCount;
        MissionTerrainObj* obj = &mission->terrainObjs[idx];
        strncpy(obj->type, terrType, sizeof(obj->type) - 1);
        obj->type[sizeof(obj->type) - 1] = '\0';
        obj->cellX = CELL_TO_X(cell);
        obj->cellY = CELL_TO_Y(cell);
        mission->terrainObjCount++;
    }
}

// Parse [SMUDGE] section - craters and scorch marks
static void ParseSmudgeSection(MissionData* mission, INIClass* ini) {
    int count = ini->EntryCount("SMUDGE");
    mission->smudgeCount = 0;

    for (int i = 0; i < count && mission->smudgeCount < MAX_MISSION_SMUDGE;
         i++) {
        const char* entry = ini->GetEntry("SMUDGE", i);
        if (!entry) continue;

        int cell = atoi(entry);
        if (cell < 0) continue;

        char value[64];
        ini->GetString("SMUDGE", entry, "", value, sizeof(value));
        if (value[0] == '\0') continue;

        char smudgeType[16];
        int smudgeCell, smudgeData = 0;
        if (sscanf(value, "%15[^,],%d,%d",
                   smudgeType, &smudgeCell, &smudgeData) >= 2) {
            MissionSmudge* smudge = &mission->smudges[mission->smudgeCount];
            strncpy(smudge->type, smudgeType, sizeof(smudge->type) - 1);
            smudge->type[sizeof(smudge->type) - 1] = '\0';
            smudge->cellX = CELL_TO_X(smudgeCell);
            smudge->cellY = CELL_TO_Y(smudgeCell);
            smudge->data = (int16_t)smudgeData;
            mission->smudgeCount++;
        }
    }
}

// Parse [MapPack] section - terrain data
static void ParseMapPackSection(MissionData* mission, INIClass* ini) {
    int mapPackSize = 0;
    uint8_t* mapPackData = ParsePackSection(ini, "MapPack", &mapPackSize);

    if (mapPackData && mapPackSize >= MAP_CELL_TOTAL * 3) {
        mission->terrainType = (uint8_t*)malloc(MAP_CELL_TOTAL);
        mission->terrainIcon = (uint8_t*)malloc(MAP_CELL_TOTAL);

        if (mission->terrainType && mission->terrainIcon) {
            for (int i = 0; i < MAP_CELL_TOTAL; i++) {
                uint16_t tileID = mapPackData[i * 2] |
                                  (mapPackData[i * 2 + 1] << 8);
                mission->terrainType[i] = (tileID == 0 || tileID == 0xFFFF)
                                          ? 0xFF : (uint8_t)(tileID & 0xFF);
            }
            memcpy(mission->terrainIcon,
                   mapPackData + MAP_CELL_TOTAL * 2,
                   MAP_CELL_TOTAL);
        }
        free(mapPackData);
    }
}

// Parse [OverlayPack] section - overlay data (ore, walls, etc.)
static void ParseOverlayPackSection(MissionData* mission, INIClass* ini) {
    int overlayPackSize = 0;
    uint8_t* overlayPackData = ParsePackSection(ini, "OverlayPack",
                                                 &overlayPackSize);

    if (overlayPackData && overlayPackSize >= MAP_CELL_TOTAL) {
        mission->overlayType = (uint8_t*)malloc(MAP_CELL_TOTAL);
        if (mission->overlayType) {
            memcpy(mission->overlayType, overlayPackData, MAP_CELL_TOTAL);
        }

        if (overlayPackSize >= MAP_CELL_TOTAL * 2) {
            mission->overlayData = (uint8_t*)malloc(MAP_CELL_TOTAL);
            if (mission->overlayData) {
                memcpy(mission->overlayData,
                       overlayPackData + MAP_CELL_TOTAL,
                       MAP_CELL_TOTAL);
            }
        }
        free(overlayPackData);
    }
}

// Parse [MapPack] or [OverlayPack] section
// Format: Base64-encoded, chunked LCW compression
// Each chunk: 4-byte length (& 0xDFFFFFFF) + LCW data -> 8192 bytes
// Returns allocated buffer on success, nullptr on failure
static uint8_t* ParsePackSection(INIClass* ini, const char* section,
                                  int* outSize) {
    if (!ini || !section || !outSize) return nullptr;

    // Count entries and estimate base64 size
    int entryCount = ini->EntryCount(section);
    if (entryCount <= 0) return nullptr;

    // Concatenate all base64 lines
    int maxB64Size = entryCount * 128;
    char* b64Data = (char*)malloc(maxB64Size);
    if (!b64Data) return nullptr;

    int b64Len = 0;
    for (int i = 0; i < entryCount; i++) {
        char key[8];
        snprintf(key, sizeof(key), "%d", i + 1);
        char line[128];
        ini->GetString(section, key, "", line, sizeof(line));
        int lineLen = (int)strlen(line);
        if (b64Len + lineLen < maxB64Size) {
            memcpy(b64Data + b64Len, line, lineLen);
            b64Len += lineLen;
        }
    }
    b64Data[b64Len] = '\0';

    if (b64Len == 0) {
        free(b64Data);
        return nullptr;
    }

    // Decode base64 -> packed chunk data
    int maxPackedSize = (b64Len * 3) / 4 + 16;
    uint8_t* packed = (uint8_t*)malloc(maxPackedSize);
    if (!packed) {
        free(b64Data);
        return nullptr;
    }

    int packedSize = Base64_Decode(b64Data, b64Len, packed, maxPackedSize);
    free(b64Data);

    if (packedSize <= 0) {
        free(packed);
        return nullptr;
    }

    // Allocate output buffer (MapPack = 128*128*3 bytes for RA format)
    // Each chunk decompresses to 8192 bytes
    int maxDecompSize = MAP_CELL_TOTAL * 3;
    uint8_t* decompressed = (uint8_t*)malloc(maxDecompSize);
    if (!decompressed) {
        free(packed);
        return nullptr;
    }

    // Process chunks: [4-byte length][LCW data] -> 8192 bytes each
    int srcIdx = 0;
    int dstIdx = 0;
    const int CHUNK_SIZE = 8192;

    while (srcIdx + 4 <= packedSize) {
        // Read chunk length (little-endian, mask off high bits)
        uint32_t chunkLen = packed[srcIdx] |
                           (packed[srcIdx + 1] << 8) |
                           (packed[srcIdx + 2] << 16) |
                           (packed[srcIdx + 3] << 24);
        chunkLen &= 0xDFFFFFFF;  // Mask out compression flag (bit 29)
        srcIdx += 4;

        if (chunkLen == 0 || srcIdx + (int)chunkLen > packedSize) {
            break;
        }

        // Decompress this chunk to 8192 bytes
        if (dstIdx + CHUNK_SIZE > maxDecompSize) {
            break;
        }

        int decompLen = LCW_Decompress(&packed[srcIdx],
                                        &decompressed[dstIdx],
                                        chunkLen, CHUNK_SIZE);
        if (decompLen <= 0) {
            // Try next chunk anyway
            srcIdx += chunkLen;
            dstIdx += CHUNK_SIZE;
            continue;
        }

        srcIdx += chunkLen;
        dstIdx += CHUNK_SIZE;
    }

    free(packed);

    if (dstIdx <= 0) {
        free(decompressed);
        return nullptr;
    }

    *outSize = dstIdx;
    return decompressed;
}

int Mission_LoadFromINI(MissionData* mission, const char* filename) {
    if (!mission || !filename) return 0;

    INIClass ini;
    if (!ini.Load(filename)) {
        return 0;
    }

    return Mission_LoadFromINIClass(mission, &ini);
}

int Mission_LoadFromINIClass(MissionData* mission, INIClass* ini) {
    if (!mission || !ini) return 0;

    Mission_Init(mission);

    // Parse each INI section via dedicated helpers
    ParseBasicSection(mission, ini);
    ParseMapSection(mission, ini);
    ParseBriefingSection(mission, ini);

    // Credits from player's house section (needs player from Basic)
    char playerStr[32];
    ini->GetString("Basic", "Player", "Greece", playerStr, sizeof(playerStr));
    mission->startCredits = ini->GetInt(playerStr, "Credits", 5000);
    if (mission->startCredits == 0) {
        mission->startCredits = ini->GetInt("Basic", "Credits", 5000);
    }

    // Entity sections
    ParseUnitsSection(mission, ini);
    ParseStructuresSection(mission, ini);
    ParseInfantrySection(mission, ini);
    ParseShipsSection(mission, ini);

    // Scripting sections
    ParseTrigsSection(ini);
    ParseWaypointsSection(mission, ini);
    ParseCellTriggersSection(mission, ini);
    ParseObjectTriggersSection(mission, ini);
    ParseTeamTypesSection(mission, ini);
    ParseBaseSection(mission, ini);

    // Map data sections
    ParseMapPackSection(mission, ini);
    ParseOverlayPackSection(mission, ini);

    // Decoration sections
    ParseTerrainSection(mission, ini);
    ParseSmudgeSection(mission, ini);

    return 1;
}

int Mission_LoadFromBuffer(MissionData* mission, const char* buffer, int size) {
    if (!mission || !buffer || size <= 0) return 0;

    INIClass ini;
    if (!ini.LoadFromBuffer(buffer, size)) {
        return 0;
    }

    // Use same parsing logic as file version
    return Mission_LoadFromINIClass(mission, &ini);
}

// Helper: Set theater from mission data
static void SetupTheater(const MissionData* mission) {
    TheaterType theater = THEATER_SNOW;
    switch (mission->theater) {
        case 0: theater = THEATER_TEMPERATE; break;
        case 1: theater = THEATER_SNOW; break;
        case 2: theater = THEATER_INTERIOR; break;
        case 3: theater = THEATER_DESERT; break;
    }
    Assets_SetTheater(theater);
    Terrain_SetTheater(mission->theater);
    fprintf(stderr, "Mission: Set theater to %d (%s)\n", mission->theater,
            theater == THEATER_TEMPERATE ? "TEMPERATE" :
            theater == THEATER_SNOW ? "SNOW" :
            theater == THEATER_INTERIOR ? "INTERIOR" : "DESERT");
}

// Helper: Load terrain data or generate demo map
static void LoadMissionMap(const MissionData* mission) {
    if (mission->terrainType && mission->terrainIcon) {
        Map_LoadFromMission(mission->terrainType, mission->terrainIcon,
                           mission->overlayType, mission->overlayData,
                           mission->mapX, mission->mapY,
                           mission->mapWidth, mission->mapHeight);
    } else {
        Map_GenerateDemo();
    }
}

// Helper: Spawn buildings, converting 128x128 coords to local
static void SpawnMissionBuildings(const MissionData* mission) {
    for (int i = 0; i < mission->buildingCount; i++) {
        const MissionBuilding* bld = &mission->buildings[i];
        int localX = bld->cellX - mission->mapX;
        int localY = bld->cellY - mission->mapY;
        if (localX >= 0 && localX < mission->mapWidth &&
            localY >= 0 && localY < mission->mapHeight) {
            int buildingId = Buildings_Spawn(bld->type, bld->team,
                                             localX, localY);
            // Attach trigger if specified
            if (buildingId >= 0 && bld->triggerName[0] != '\0') {
                Building* spawned = Buildings_Get(buildingId);
                if (spawned) {
                    strncpy(spawned->triggerName, bld->triggerName,
                            sizeof(spawned->triggerName) - 1);
                    spawned->triggerName[sizeof(spawned->triggerName) - 1] =
                        '\0';
                }
            }
        }
    }
}

// Helper: Spawn units, converting 128x128 coords to local world
// Check if unit type is a civilian (C1-C10, CHAN)
static bool IsCivilianType(UnitType type) {
    return (type >= UNIT_CIVILIAN_1 && type <= UNIT_CIVILIAN_10) ||
           type == UNIT_CHAN;
}

static void SpawnMissionUnits(const MissionData* mission) {
    for (int i = 0; i < mission->unitCount; i++) {
        const MissionUnit* unit = &mission->units[i];
        int localCellX = unit->cellX - mission->mapX;
        int localCellY = unit->cellY - mission->mapY;
        if (localCellX >= 0 && localCellX < mission->mapWidth &&
            localCellY >= 0 && localCellY < mission->mapHeight) {
            int worldX = localCellX * CELL_SIZE + CELL_SIZE / 2;
            int worldY = localCellY * CELL_SIZE + CELL_SIZE / 2;
            // Force civilians to neutral team (player can't control them)
            Team team = unit->team;
            if (IsCivilianType(unit->type)) {
                team = TEAM_NEUTRAL;
            }
            int unitId = Units_Spawn(unit->type, team, worldX, worldY);
            // Attach trigger if specified
            if (unitId >= 0 && unit->triggerName[0] != '\0') {
                Unit* spawned = Units_Get(unitId);
                if (spawned) {
                    strncpy(spawned->triggerName, unit->triggerName,
                            sizeof(spawned->triggerName) - 1);
                    spawned->triggerName[sizeof(spawned->triggerName) - 1] =
                        '\0';
                }
            }
        }
    }
}

// Helper: Center viewport on player start position
// Priority: waypoint 98 (player start) > first player unit > default (0,0)
static void CenterOnPlayerStart(const MissionData* mission) {
    // Try waypoint 98 first (standard player start waypoint)
    if (mission->waypointCount > 98 && mission->waypoints[98].cell >= 0) {
        int localCellX = mission->waypoints[98].cellX - mission->mapX;
        int localCellY = mission->waypoints[98].cellY - mission->mapY;
        if (localCellX >= 0 && localCellX < mission->mapWidth &&
            localCellY >= 0 && localCellY < mission->mapHeight) {
            int worldX = localCellX * CELL_SIZE + CELL_SIZE / 2;
            int worldY = localCellY * CELL_SIZE + CELL_SIZE / 2;
            Map_CenterViewport(worldX, worldY);
            return;
        }
    }

    // Fall back to first player unit
    for (int i = 0; i < mission->unitCount; i++) {
        const MissionUnit* unit = &mission->units[i];
        if (unit->team == TEAM_PLAYER) {
            int localCellX = unit->cellX - mission->mapX;
            int localCellY = unit->cellY - mission->mapY;
            if (localCellX >= 0 && localCellX < mission->mapWidth &&
                localCellY >= 0 && localCellY < mission->mapHeight) {
                int worldX = localCellX * CELL_SIZE + CELL_SIZE / 2;
                int worldY = localCellY * CELL_SIZE + CELL_SIZE / 2;
                Map_CenterViewport(worldX, worldY);
                return;
            }
        }
    }
}

// Helper: Log mission data for debugging
static void LogMissionData(const MissionData* mission) {
    if (g_parsedTriggerCount > 0) {
        fprintf(stderr, "  Loaded %d triggers from mission INI\n",
                g_parsedTriggerCount);
        for (int i = 0; i < g_parsedTriggerCount && i < 5; i++) {
            ParsedTrigger* trig = &g_parsedTriggers[i];
            fprintf(stderr, "    Trigger '%s': event1=%d action1=%d\n",
                    trig->name, trig->event1, trig->action1);
        }
        if (g_parsedTriggerCount > 5)
            fprintf(stderr, "    ... and %d more\n", g_parsedTriggerCount - 5);
    }
    if (mission->waypointCount > 0) {
        fprintf(stderr, "  Loaded %d waypoints\n", mission->waypointCount);
        int shown = 0;
        for (int i = 0; i < mission->waypointCount && shown < 5; i++) {
            if (mission->waypoints[i].cell >= 0) {
                fprintf(stderr, "    Waypoint %d: cell=%d (%d,%d)\n",
                        i, mission->waypoints[i].cell,
                        mission->waypoints[i].cellX,
                        mission->waypoints[i].cellY);
                shown++;
            }
        }
    }
    if (mission->teamTypeCount > 0) {
        fprintf(stderr, "  Loaded %d team types\n", mission->teamTypeCount);
        for (int i = 0; i < mission->teamTypeCount && i < 5; i++) {
            const MissionTeamType* team = &mission->teamTypes[i];
            fprintf(stderr, "    Team '%s': house=%d members=%d missions=%d\n",
                    team->name, team->house,
                    team->memberCount, team->missionCount);
        }
        if (mission->teamTypeCount > 5) {
            int more = mission->teamTypeCount - 5;
            fprintf(stderr, "    ... and %d more\n", more);
        }
    }
    if (mission->terrainObjCount > 0)
        fprintf(stderr, "  Loaded %d terrain objects (trees, etc.)\n",
                mission->terrainObjCount);
    if (mission->smudgeCount > 0)
        fprintf(stderr, "  Loaded %d smudges (craters, etc.)\n",
                mission->smudgeCount);
}

// Initialize fog of war based on player unit/building positions
// This must be called AFTER units/buildings are spawned, before first render
static void InitializeFogOfWar(void) {
    // At this point, all cells have flags=0 from Map_Create()
    // We only need to reveal around player units/buildings

    // Reveal around player units
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_PLAYER) continue;

        int cellX, cellY;
        Map_WorldToCell(unit->worldX, unit->worldY, &cellX, &cellY);
        Map_RevealAround(cellX, cellY, unit->sightRange, TEAM_PLAYER);
    }

    // Reveal around player buildings
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_PLAYER) continue;

        int centerX = bld->cellX + bld->width / 2;
        int centerY = bld->cellY + bld->height / 2;
        Map_RevealAround(centerX, centerY, bld->sightRange, TEAM_PLAYER);
    }
}

void Mission_Start(const MissionData* mission) {
    if (!mission) return;

    // Reset global flags for new mission
    for (int i = 0; i < MAX_GLOBAL_FLAGS; i++) {
        g_globalFlags[i] = false;
    }

    SetupTheater(mission);
    Map_Init();
    Units_Init();
    AI_Init();
    LoadMissionMap(mission);
    SpawnMissionBuildings(mission);
    SpawnMissionUnits(mission);
    InitializeFogOfWar();  // Initialize fog AFTER units/buildings are spawned
    CenterOnPlayerStart(mission);
    LogMissionData(mission);
}

int Mission_CheckVictory(const MissionData* mission, int frameCount) {
    if (!mission) return 0;

    // Helper: count enemy buildings
    auto countEnemyBuildings = []() -> int {
        int count = 0;
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            Building* bld = Buildings_Get(i);
            if (bld && bld->team == TEAM_ENEMY) {
                count++;
            }
        }
        return count;
    };

    // Helper: count player buildings
    auto countPlayerBuildings = []() -> int {
        int count = 0;
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            Building* bld = Buildings_Get(i);
            if (bld && bld->team == TEAM_PLAYER) {
                count++;
            }
        }
        return count;
    };

    // Check win condition
    switch (mission->winCondition) {
        case 0:  // Destroy all enemy units and buildings
            if (Units_CountByTeam(TEAM_ENEMY) == 0 &&
                countEnemyBuildings() == 0) {
                return 1;  // Win!
            }
            break;

        case 1:  // Destroy enemy buildings only
            if (countEnemyBuildings() == 0) {
                return 1;  // Win!
            }
            break;

        case 2:  // Survive for time limit
            if (mission->timeLimit > 0 && frameCount >= mission->timeLimit) {
                // Check player still has units/buildings (survived)
                if (Units_CountByTeam(TEAM_PLAYER) > 0 ||
                    countPlayerBuildings() > 0) {
                    return 1;  // Win - survived!
                }
            }
            break;

        case 3:  // Capture specific building (at targetCell)
            // Check if player now owns building at target cell
            if (mission->targetCell >= 0) {
                int targetX = mission->targetCell % 128;
                int targetY = mission->targetCell / 128;
                for (int i = 0; i < MAX_BUILDINGS; i++) {
                    Building* bld = Buildings_Get(i);
                    if (bld && bld->team == TEAM_PLAYER) {
                        // Check if building is at target location
                        if (bld->cellX == targetX && bld->cellY == targetY) {
                            return 1;  // Win - captured!
                        }
                    }
                }
            }
            break;
    }

    // Check lose condition
    switch (mission->loseCondition) {
        case 0:  // Lose all units and buildings
            if (Units_CountByTeam(TEAM_PLAYER) == 0 &&
                countPlayerBuildings() == 0) {
                return -1;  // Lose
            }
            break;

        case 1:  // Lose all buildings
            if (countPlayerBuildings() == 0) {
                return -1;  // Lose
            }
            break;

        case 2:  // Time expires (before completing objective)
            if (mission->timeLimit > 0 && frameCount >= mission->timeLimit) {
                // Only lose if win condition isn't also time-based
                if (mission->winCondition != 2) {
                    return -1;  // Lose - time's up!
                }
            }
            break;

        case 3:  // Lose specific unit/building (at targetCell)
            // Check if target is destroyed
            if (mission->targetCell >= 0) {
                int targetX = mission->targetCell % 128;
                int targetY = mission->targetCell / 128;
                bool targetExists = false;
                // Check buildings
                for (int i = 0; i < MAX_BUILDINGS && !targetExists; i++) {
                    Building* bld = Buildings_Get(i);
                    if (bld && bld->team == TEAM_PLAYER) {
                        if (bld->cellX == targetX && bld->cellY == targetY) {
                            targetExists = true;
                        }
                    }
                }
                // If target was at that cell and now isn't, we lose
                // (This is simplified - would need to track initial state)
            }
            break;
    }

    return 0;  // Game ongoing
}

void Mission_GetDemo(MissionData* mission) {
    if (!mission) return;

    Mission_Init(mission);

    strncpy(mission->name, "Demo Skirmish", sizeof(mission->name) - 1);
    strncpy(mission->description, "Destroy the enemy base.",
            sizeof(mission->description) - 1);
    mission->theater = 1;  // Snow
    mission->mapWidth = 64;
    mission->mapHeight = 64;
    mission->startCredits = 5000;
    mission->winCondition = 0;  // Destroy all
    mission->loseCondition = 0; // Lose all

    // Building/unit macros for concise initialization
    #define B(t,tm,x,y,s,r) {t, tm, x, y, 256, 0, s, r, ""}
    #define U(t,tm,x,y,f,m,s) {t, tm, x, y, 256, f, m, s, ""}
    #define P TEAM_PLAYER
    #define E TEAM_ENEMY
    #define CY BUILDING_CONSTRUCTION
    #define PW BUILDING_POWER
    #define BA BUILDING_BARRACKS
    #define RE BUILDING_REFINERY
    #define FA BUILDING_FACTORY
    #define TU BUILDING_TURRET
    #define TM UNIT_TANK_MEDIUM
    #define TL UNIT_TANK_LIGHT
    #define TH UNIT_TANK_HEAVY
    #define RI UNIT_RIFLE
    #define RK UNIT_ROCKET
    #define HA UNIT_HARVESTER
    #define G MISSION_GUARD
    #define H MISSION_HUNT

    // Player buildings
    MissionBuilding* bld = mission->buildings;
    int& bc = mission->buildingCount;
    bld[bc++] = B(CY, P, 2, 15, 1, 0);
    bld[bc++] = B(PW, P, 6, 16, 1, 0);
    bld[bc++] = B(BA, P, 2, 19, 1, 0);
    bld[bc++] = B(RE, P, 6, 19, 1, 0);

    // Enemy buildings
    bld[bc++] = B(CY, E, 55, 10, 0, 1);
    bld[bc++] = B(PW, E, 52, 10, 0, 1);
    bld[bc++] = B(BA, E, 55, 6, 0, 1);
    bld[bc++] = B(FA, E, 52, 6, 0, 1);
    bld[bc++] = B(TU, E, 50, 12, 0, 1);
    bld[bc++] = B(TU, E, 58, 12, 0, 1);
    bld[bc++] = B(RE, E, 58, 8, 0, 1);

    // Player units
    MissionUnit* unt = mission->units;
    int& uc = mission->unitCount;
    unt[uc++] = U(TM, P, 4, 16, 64, G, 0);
    unt[uc++] = U(TM, P, 5, 17, 64, G, 0);
    unt[uc++] = U(TL, P, 7, 16, 64, G, 0);
    unt[uc++] = U(TL, P, 7, 18, 64, G, 0);
    unt[uc++] = U(RI, P, 3, 18, 64, G, 0);
    unt[uc++] = U(RI, P, 4, 18, 64, G, 1);
    unt[uc++] = U(RI, P, 5, 18, 64, G, 2);
    unt[uc++] = U(RK, P, 2, 17, 64, G, 0);
    unt[uc++] = U(HA, P, 8, 20, 64, MISSION_HARVEST, 0);

    // Enemy units
    unt[uc++] = U(TH, E, 54, 12, 192, G, 0);
    unt[uc++] = U(TM, E, 52, 13, 192, G, 0);
    unt[uc++] = U(TM, E, 56, 13, 192, G, 0);
    unt[uc++] = U(RI, E, 50, 14, 192, H, 0);
    unt[uc++] = U(RI, E, 51, 14, 192, H, 1);
    unt[uc++] = U(RI, E, 52, 14, 192, H, 2);
    unt[uc++] = U(RK, E, 54, 10, 192, G, 0);

    #undef B
    #undef U
    #undef P
    #undef E
    #undef CY
    #undef PW
    #undef BA
    #undef RE
    #undef FA
    #undef TU
    #undef TM
    #undef TL
    #undef TH
    #undef RI
    #undef RK
    #undef HA
    #undef G
    #undef H
}

// ============================================================================
// Trigger Processing
// ============================================================================

// Red Alert INI event types (from original game)
// Note: These match the INI file format, not our internal enum
enum {
    RA_EVENT_NONE = 0,
    RA_EVENT_ENTERED = 1,        // Player entered cell/zone
    RA_EVENT_SPIED = 2,          // Building spied upon
    RA_EVENT_THIEVED = 3,        // Thief stole vehicle
    RA_EVENT_DISCOVERED = 4,     // Object discovered
    RA_EVENT_HOUSE_DISC = 5,     // House discovered
    RA_EVENT_ATTACKED = 6,       // Object attacked
    RA_EVENT_DESTROYED = 7,      // Object destroyed
    RA_EVENT_ANY = 8,            // Any event
    RA_EVENT_UNITS_DESTR = 9,    // All units destroyed
    RA_EVENT_BLDGS_DESTR = 10,   // All buildings destroyed
    RA_EVENT_ALL_DESTR = 11,     // All units+buildings destroyed
    RA_EVENT_CREDITS = 12,       // Credits reach amount
    RA_EVENT_TIME = 13,          // Time elapsed
    RA_EVENT_TIMER_EXP = 14,     // Mission timer expired
    RA_EVENT_NOBLDGS = 15,       // No buildings left
    RA_EVENT_CIVEVAC = 16,       // Civilian evacuated
    RA_EVENT_OBJBUILT = 17,      // Object built
    RA_EVENT_LEAVES = 18,        // Team leaves map
    RA_EVENT_ZONE_ENT = 19,      // Zone entered
    RA_EVENT_HORZ_CROSS = 20,    // Crosses horizontal line
    RA_EVENT_VERT_CROSS = 21,    // Crosses vertical line
    RA_EVENT_GLOBAL_SET = 22,    // Global variable set
    RA_EVENT_GLOBAL_CLR = 23,    // Global variable cleared
    RA_EVENT_FAKES_DESTR = 24,   // Fake buildings destroyed
    RA_EVENT_LOW_POWER = 25,     // Low power
    RA_EVENT_BRIDGE_DESTR = 26,  // All bridges destroyed
    RA_EVENT_BUILDING_EXISTS = 27, // Specific building exists
};

// Red Alert INI action types (from original game)
enum {
    RA_ACTION_NONE = 0,
    RA_ACTION_WIN = 1,           // Player wins
    RA_ACTION_LOSE = 2,          // Player loses
    RA_ACTION_BEGIN_PROD = 3,    // Begin production
    RA_ACTION_CREATE_TEAM = 4,   // Create team
    RA_ACTION_DESTROY_TEAM = 5,  // Destroy team
    RA_ACTION_ALL_HUNT = 6,      // All hunt
    RA_ACTION_REINFORCE = 7,     // Reinforcements arrive
    RA_ACTION_DZ = 8,            // Drop zone marker
    RA_ACTION_FIRE_SALE = 9,     // Fire sale
    RA_ACTION_PLAY_MOVIE = 10,   // Play movie
    RA_ACTION_TEXT = 11,         // Display text
    RA_ACTION_DESTR_TRIG = 12,   // Destroy trigger
    RA_ACTION_AUTOCREATE = 13,   // Auto create teams
    RA_ACTION_WINLOSE = 14,      // Win/lose based on object
    RA_ACTION_ALLOWWIN = 15,     // Allow win (same as win?)
    RA_ACTION_REVEAL_ALL = 16,   // Reveal entire map
    RA_ACTION_REVEAL_SOME = 17,  // Reveal around waypoint
    RA_ACTION_REVEAL_ZONE = 18,  // Reveal zone
    RA_ACTION_PLAY_SOUND = 19,   // Play sound
    RA_ACTION_PLAY_MUSIC = 20,   // Play music
    RA_ACTION_PLAY_SPEECH = 21,  // Play speech
    RA_ACTION_FORCE_TRIG = 22,   // Force trigger
    RA_ACTION_START_TIMER = 23,  // Start mission timer
    RA_ACTION_STOP_TIMER = 24,   // Stop mission timer
    RA_ACTION_ADD_TIMER = 25,    // Add time
    RA_ACTION_SUB_TIMER = 26,    // Subtract time
    RA_ACTION_SET_TIMER = 27,    // Set timer
    RA_ACTION_SET_GLOBAL = 28,   // Set global flag
    RA_ACTION_CLEAR_GLOBAL = 29, // Clear global flag
    RA_ACTION_BASE_BUILDING = 30, // Auto base building
    RA_ACTION_GROW_SHROUD = 31,  // Grow shroud one step
    RA_ACTION_DESTROY_OBJ = 32,  // Destroy attached object
    RA_ACTION_1_SPECIAL = 33,    // One-time special
    RA_ACTION_FULL_SPECIAL = 34, // Full special
    RA_ACTION_PREF_TARGET = 35,  // Preferred target
    RA_ACTION_LAUNCH_NUKES = 36, // Launch fake nukes
};

// Helper: Count units by team (enemy house detection)
static int CountUnitsByHouse(int houseNum) {
    // House numbers: 0=Spain, 1=Greece, 2=USSR, 3=England, 4=Ukraine,
    //                5=Germany, 6=France, 7=Turkey
    // USSR (2) and Ukraine (4) are Soviet
    // Others are Allied
    Team team;
    if (houseNum == 2 || houseNum == 4) {
        team = TEAM_ENEMY;  // Soviet houses
    } else {
        team = TEAM_PLAYER; // Allied houses
    }
    return Units_CountByTeam(team);
}

// Helper: Count buildings by team
static int CountBuildingsByHouse(int houseNum) {
    Team team;
    if (houseNum == 2 || houseNum == 4) {
        team = TEAM_ENEMY;
    } else {
        team = TEAM_PLAYER;
    }

    int count = 0;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (bld && bld->team == team) {
            count++;
        }
    }
    return count;
}

// Helper: Get player power production
static int Units_GetPlayerPower(void) {
    int power = 0;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (bld && bld->team == TEAM_PLAYER) {
            // Power plants produce power
            if (bld->type == BUILDING_POWER) {
                power += 100;  // Basic power plant
            } else if (bld->type == BUILDING_ADV_POWER) {
                power += 200;  // Advanced power plant
            }
        }
    }
    return power;
}

// Helper: Get player power drain
static int Units_GetPlayerDrain(void) {
    int drain = 0;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (bld && bld->team == TEAM_PLAYER) {
            // Each building drains some power
            switch (bld->type) {
                case BUILDING_POWER:
                case BUILDING_ADV_POWER:
                    drain += 0;  // Power plants don't drain
                    break;
                case BUILDING_REFINERY:
                    drain += 40;
                    break;
                case BUILDING_FACTORY:
                    drain += 30;
                    break;
                case BUILDING_RADAR:
                    drain += 40;
                    break;
                case BUILDING_HELIPAD:
                    drain += 20;
                    break;
                default:
                    drain += 10;  // Base drain for other buildings
                    break;
            }
        }
    }
    return drain;
}

// Helper: Count buildings of a specific type
static int Buildings_CountByType(int buildingType) {
    int count = 0;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (bld && bld->type == buildingType) {
            count++;
        }
    }
    return count;
}

// Convert team house number to game Team enum
static Team HouseToTeam(int houseNum) {
    // USSR (2) and Ukraine (4) are Soviet -> TEAM_ENEMY
    // All others (Spain, Greece, England, Germany, France, Turkey) -> TEAM_PLAYER
    if (houseNum == 2 || houseNum == 4) {
        return TEAM_ENEMY;
    }
    return TEAM_PLAYER;
}

// Team mission type constants (from original Red Alert)
enum {
    TMISSION_ATTACK = 0,        // Attack specified quarry type
    TMISSION_ATTACK_WP = 1,     // Attack at waypoint
    TMISSION_FORMATION = 2,     // Change formation
    TMISSION_MOVE = 3,          // Move to waypoint
    TMISSION_MOVE_CELL = 4,     // Move to specific cell
    TMISSION_GUARD = 5,         // Guard current location
    TMISSION_JUMP = 6,          // Jump to mission step
    TMISSION_ATTACK_TC = 7,     // Attack target comm
    TMISSION_UNLOAD = 8,        // Unload transported units
    TMISSION_DEPLOY = 9,        // Deploy (MCV)
    TMISSION_FOLLOW = 10,       // Follow leader
    TMISSION_ENTER = 11,        // Enter building/transport
    TMISSION_SPY = 12,          // Spy infiltration
    TMISSION_PATROL = 13,       // Patrol to waypoint
    TMISSION_SET_GLOBAL = 14,   // Set global variable
    TMISSION_INVULN = 15,       // Make team invulnerable
    TMISSION_LOAD = 16,         // Load into transport
};

// Spawn a team's units at the specified waypoint
// Returns number of units spawned
static int SpawnTeamUnits(const MissionTeamType* team,
                          const MissionData* mission,
                          int* spawnedIds, int maxSpawned) {
    if (!team || !mission) return 0;

    // Get origin waypoint coordinates
    int wpNum = team->origin;
    if (wpNum < 0 || wpNum >= MAX_MISSION_WAYPOINTS) {
        fprintf(stderr, "    Team '%s': invalid origin waypoint %d\n",
                team->name, wpNum);
        return 0;
    }

    const MissionWaypoint* wp = &mission->waypoints[wpNum];
    if (wp->cell < 0) {
        fprintf(stderr, "    Team '%s': waypoint %d not defined\n",
                team->name, wpNum);
        return 0;
    }

    // Convert waypoint cell to world coordinates
    // Account for map offset
    int baseCellX = wp->cellX - mission->mapX;
    int baseCellY = wp->cellY - mission->mapY;
    int baseWorldX = baseCellX * CELL_SIZE + CELL_SIZE / 2;
    int baseWorldY = baseCellY * CELL_SIZE + CELL_SIZE / 2;

    // Determine team (enemy or player based on house)
    Team gameTeam = HouseToTeam(team->house);

    fprintf(stderr, "    Spawning team '%s' at wp%d (%d,%d) -> world(%d,%d)\n",
            team->name, wpNum, baseCellX, baseCellY, baseWorldX, baseWorldY);

    int spawned = 0;

    // Spawn each member type
    for (int m = 0; m < team->memberCount && spawned < maxSpawned; m++) {
        const TeamMember* member = &team->members[m];
        UnitType unitType = ParseUnitType(member->unitType);

        if (unitType == UNIT_NONE) {
            fprintf(stderr, "      Unknown unit type '%s'\n", member->unitType);
            continue;
        }

        // Spawn the quantity requested
        for (int q = 0; q < member->quantity && spawned < maxSpawned; q++) {
            // Offset each unit slightly to avoid stacking
            // Use a spiral pattern: (0,0), (1,0), (1,1), (0,1), (-1,1)...
            int offsetX = 0, offsetY = 0;
            if (spawned > 0) {
                // Simple grid offset: row = spawned / 4, col = spawned % 4
                int col = spawned % 4;
                int row = spawned / 4;
                offsetX = (col - 1) * CELL_SIZE / 2;  // -1, 0, 1, 2 cells
                offsetY = row * CELL_SIZE / 2;
            }

            int spawnX = baseWorldX + offsetX;
            int spawnY = baseWorldY + offsetY;

            int unitId = Units_Spawn(unitType, gameTeam, spawnX, spawnY);
            if (unitId >= 0) {
                if (spawnedIds) {
                    spawnedIds[spawned] = unitId;
                }
                spawned++;
                fprintf(stderr, "      Spawned %s (#%d) at (%d,%d)\n",
                        member->unitType, unitId, spawnX, spawnY);
            } else {
                fprintf(stderr, "      Failed to spawn %s\n", member->unitType);
            }
        }
    }

    return spawned;
}

// Execute a team's first mission for its spawned units
static void ExecuteTeamMission(const MissionTeamType* team,
                               const MissionData* mission,
                               const int* unitIds, int unitCount) {
    if (!team || !mission || unitCount == 0) return;
    if (team->missionCount == 0) {
        // No missions defined - default to guard
        for (int i = 0; i < unitCount; i++) {
            Units_CommandGuard(unitIds[i]);
        }
        return;
    }

    // Get first mission
    const TeamMission* tmission = &team->missions[0];
    int missionType = tmission->mission;
    int missionData = tmission->data;

    fprintf(stderr, "    Team '%s' mission: type=%d data=%d\n",
            team->name, missionType, missionData);

    switch (missionType) {
        case TMISSION_ATTACK:
        case TMISSION_ATTACK_WP:
        case TMISSION_ATTACK_TC:
            // Attack - set units to attack-move toward a waypoint
            if (missionData >= 0 && missionData < MAX_MISSION_WAYPOINTS) {
                const MissionWaypoint* targetWp = &mission->waypoints[missionData];
                if (targetWp->cell >= 0) {
                    int targetX = (targetWp->cellX - mission->mapX) * CELL_SIZE;
                    int targetY = (targetWp->cellY - mission->mapY) * CELL_SIZE;
                    for (int i = 0; i < unitCount; i++) {
                        Units_CommandAttackMove(unitIds[i], targetX, targetY);
                    }
                    fprintf(stderr, "      -> Attack-move to wp%d (%d,%d)\n",
                            missionData, targetX, targetY);
                    return;
                }
            }
            // Fallback: set to attack-move mode at current position
            for (int i = 0; i < unitCount; i++) {
                Unit* unit = Units_Get(unitIds[i]);
                if (unit) {
                    unit->state = STATE_ATTACK_MOVE;
                }
            }
            break;

        case TMISSION_MOVE:
        case TMISSION_MOVE_CELL:
        case TMISSION_PATROL:
            // Move to waypoint
            if (missionData >= 0 && missionData < MAX_MISSION_WAYPOINTS) {
                const MissionWaypoint* targetWp = &mission->waypoints[missionData];
                if (targetWp->cell >= 0) {
                    int targetX = (targetWp->cellX - mission->mapX) * CELL_SIZE;
                    int targetY = (targetWp->cellY - mission->mapY) * CELL_SIZE;
                    for (int i = 0; i < unitCount; i++) {
                        Units_CommandMove(unitIds[i], targetX, targetY);
                    }
                    fprintf(stderr, "      -> Move to wp%d (%d,%d)\n",
                            missionData, targetX, targetY);
                    return;
                }
            }
            break;

        case TMISSION_GUARD:
            // Guard current position
            for (int i = 0; i < unitCount; i++) {
                Units_CommandGuard(unitIds[i]);
            }
            fprintf(stderr, "      -> Guard\n");
            break;

        case TMISSION_UNLOAD:
            // For transports - unload cargo
            // For now, treat as guard
            for (int i = 0; i < unitCount; i++) {
                Units_CommandGuard(unitIds[i]);
            }
            break;

        case TMISSION_DEPLOY:
            // For MCV - deploy into construction yard
            // For now, treat as guard
            for (int i = 0; i < unitCount; i++) {
                Units_CommandGuard(unitIds[i]);
            }
            break;

        case TMISSION_SET_GLOBAL:
            // Set a global flag
            if (missionData >= 0 && missionData < MAX_GLOBAL_FLAGS) {
                g_globalFlags[missionData] = true;
                fprintf(stderr, "      -> Set global %d\n", missionData);
            }
            // Units default to guard
            for (int i = 0; i < unitCount; i++) {
                Units_CommandGuard(unitIds[i]);
            }
            break;

        default:
            // Unknown mission - default to guard
            for (int i = 0; i < unitCount; i++) {
                Units_CommandGuard(unitIds[i]);
            }
            break;
    }
}

// Check if any player unit is within radius of a cell position
// Returns true if at least one player unit is within range
static bool IsPlayerUnitNearCell(int cellX, int cellY, int radiusCells) {
    int centerX = cellX * CELL_SIZE + CELL_SIZE / 2;
    int centerY = cellY * CELL_SIZE + CELL_SIZE / 2;
    int radiusPixels = radiusCells * CELL_SIZE;
    int radiusSq = radiusPixels * radiusPixels;

    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        if (unit->team != TEAM_PLAYER) continue;
        if (unit->state == STATE_DYING) continue;

        int dx = unit->worldX - centerX;
        int dy = unit->worldY - centerY;
        int distSq = dx * dx + dy * dy;

        if (distSq <= radiusSq) {
            return true;
        }
    }
    return false;
}

// Check if a trigger event is satisfied
static bool CheckTriggerEvent(ParsedTrigger* trig, int eventNum, int param1,
                               int param2, int frameCount,
                               const MissionData* mission) {
    (void)param2;

    switch (eventNum) {
        case RA_EVENT_NONE:
            return false;

        case RA_EVENT_ENTERED: {
            // Three modes:
            // 1. param1 >= 0: waypoint-based, check if player unit near waypoint
            // 2. param1 < 0: cell-based via [CellTriggers] section
            // 3. param1 < 0: object-attached via STRUCTURES/UNITS trigger field
            if (!mission) break;

            int wp = param1;
            if (wp >= 0 && wp < MAX_MISSION_WAYPOINTS &&
                mission->waypoints[wp].cell >= 0) {
                // Waypoint-based: check if player within 2 cells of waypoint
                int cellX = mission->waypoints[wp].cellX;
                int cellY = mission->waypoints[wp].cellY;
                if (IsPlayerUnitNearCell(cellX, cellY, 2)) {
                    return true;
                }
            } else {
                // Cell-based: check [CellTriggers] entries for this trigger
                for (int i = 0; i < mission->cellTriggerCount; i++) {
                    if (strcasecmp(mission->cellTriggerNames[i],
                                   trig->name) == 0) {
                        int cell = mission->cellTriggerCells[i];
                        int cellX = CELL_TO_X(cell);
                        int cellY = CELL_TO_Y(cell);
                        if (IsPlayerUnitNearCell(cellX, cellY, 0)) {
                            return true;
                        }
                    }
                }
                // Object-attached: check object trigger cells
                for (int i = 0; i < mission->objectTriggerCount; i++) {
                    if (strcasecmp(mission->objectTriggerNames[i],
                                   trig->name) == 0) {
                        int cell = mission->objectTriggerCells[i];
                        int cellX = CELL_TO_X(cell);
                        int cellY = CELL_TO_Y(cell);
                        if (IsPlayerUnitNearCell(cellX, cellY, 0)) {
                            return true;
                        }
                    }
                }
            }
            break;
        }

        case RA_EVENT_ATTACKED:  // Object attacked
            // Triggered when attached object is attacked
            // The wasAttacked flag is set by Mission_TriggerAttacked()
            // when an object with this trigger takes damage
            if (trig->wasAttacked) {
                return true;
            }
            break;

        case RA_EVENT_DESTROYED:  // Object destroyed
            // Triggered when attached object is destroyed
            // The wasDestroyed flag is set by Mission_TriggerDestroyed()
            // when an object with this trigger is killed
            if (trig->wasDestroyed) {
                return true;
            }
            break;

        case RA_EVENT_ALL_DESTR:  // All units+buildings destroyed
            // param2 is house number
            if (CountUnitsByHouse(param2) == 0 &&
                CountBuildingsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_UNITS_DESTR:  // All units destroyed
            if (CountUnitsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_BLDGS_DESTR:  // All buildings destroyed
            if (CountBuildingsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_NOBLDGS:  // No buildings (same as BLDGS_DESTR?)
            if (CountBuildingsByHouse(param2) == 0) {
                return true;
            }
            break;

        case RA_EVENT_TIME:  // Time elapsed
            // param2 is time in 1/10 seconds, frameCount is in frames at 60fps
            // Time in frames = param2 * 6 (for 60fps)
            if (frameCount >= param2 * 6) {
                return true;
            }
            break;

        case RA_EVENT_CREDITS:  // Credits reach amount
            // param2 = amount needed
            if (Units_GetPlayerCredits() >= param2) {
                return true;
            }
            break;

        case RA_EVENT_ANY:  // Any event (always triggers)
            return true;

        case RA_EVENT_DISCOVERED:  // Object discovered (revealed)
            // Triggered when unit/building is first seen by player
            // param1 = unit ID to check (set when trigger is attached to object)
            if (param1 >= 0 && Units_WasDiscovered(param1)) {
                Units_ClearDiscovered(param1);  // One-shot event
                return true;
            }
            break;

        case RA_EVENT_HOUSE_DISC:  // House discovered
            // Triggered when any unit of a house is discovered for first time
            // param2 = house index to check
            if (param2 >= 0 && param2 < HOUSE_COUNT &&
                Units_WasHouseDiscovered(static_cast<HouseType>(param2))) {
                return true;
            }
            break;

        case RA_EVENT_GLOBAL_SET:  // Global variable set
            // param2 = global flag number
            if (param2 >= 0 && param2 < MAX_GLOBAL_FLAGS) {
                return g_globalFlags[param2];
            }
            break;

        case RA_EVENT_GLOBAL_CLR:  // Global variable cleared
            // param2 = global flag number
            if (param2 >= 0 && param2 < MAX_GLOBAL_FLAGS) {
                return !g_globalFlags[param2];
            }
            break;

        case RA_EVENT_TIMER_EXP:  // Mission timer expired
            // Trigger when countdown timer reaches zero
            if (g_missionTimerActive && g_missionTimerValue <= 0) {
                return true;
            }
            break;

        case RA_EVENT_LOW_POWER:  // Low power
            // Trigger when player power is below 100%
            // param2 = house to check
            {
                int power = Units_GetPlayerPower();
                int drain = Units_GetPlayerDrain();
                if (drain > 0 && power < drain) {
                    return true;
                }
            }
            break;

        case RA_EVENT_BUILDING_EXISTS:  // Specific building exists
            // param1 = building type to check for
            // Triggers when a building of the specified type exists
            if (Buildings_CountByType(param1) > 0) {
                return true;
            }
            break;

        case RA_EVENT_CIVEVAC:  // Civilian evacuated
            // Triggered via wasEvacuated flag
            if (trig->wasEvacuated) {
                return true;
            }
            break;

        case RA_EVENT_ZONE_ENT:  // Zone entered (same as ENTERED with wp)
            // param1 = waypoint number
            if (mission && param1 >= 0 && param1 < MAX_MISSION_WAYPOINTS &&
                mission->waypoints[param1].cell >= 0) {
                int cellX = mission->waypoints[param1].cellX;
                int cellY = mission->waypoints[param1].cellY;
                if (IsPlayerUnitNearCell(cellX, cellY, 2)) {
                    return true;
                }
            }
            break;

        case RA_EVENT_SPIED:  // Building spied upon
            // Requires spy unit implementation - stub for now
            // Would check if trig->wasSpied flag set
            break;

        case RA_EVENT_THIEVED:  // Thief stole vehicle
            // Requires thief unit implementation - stub for now
            break;

        case RA_EVENT_OBJBUILT:  // Object built
            // param1 = object type to check
            // Requires build tracking - stub for now
            break;

        case RA_EVENT_LEAVES:  // Team leaves map
            // Requires team tracking at map edges - stub for now
            break;

        case RA_EVENT_HORZ_CROSS:  // Crosses horizontal line
            // param1 = Y coordinate of line
            // Requires unit position tracking - stub for now
            break;

        case RA_EVENT_VERT_CROSS:  // Crosses vertical line
            // param1 = X coordinate of line
            // Requires unit position tracking - stub for now
            break;

        case RA_EVENT_FAKES_DESTR:  // Fake buildings destroyed
            // Requires fake building tracking - stub for now
            break;

        case RA_EVENT_BRIDGE_DESTR:  // All bridges destroyed
            // Requires bridge object tracking - stub for now
            break;

        default:
            // Unknown event type - don't trigger
            break;
    }

    return false;
}

// Execute a trigger action
// Returns: 1=win, -1=lose, 0=continue
static int ExecuteTriggerAction(ParsedTrigger* trig, int actionNum,
                                 int param1, int param2, int param3,
                                 const MissionData* mission) {
    (void)param1;
    (void)param2;
    (void)param3;
    (void)mission;

    switch (actionNum) {
        case RA_ACTION_NONE:
            break;

        case RA_ACTION_WIN:
        case RA_ACTION_ALLOWWIN:  // Treat allow-win as win
            fprintf(stderr, "  TRIGGER: Win action executed!\n");
            return 1;

        case RA_ACTION_LOSE:
            fprintf(stderr, "  TRIGGER: Lose action executed!\n");
            return -1;

        case RA_ACTION_BEGIN_PROD: {
            // Use trigger's house field (like original Data.House)
            fprintf(stderr, "  TRIGGER: Begin production (house %d)\n",
                    trig->house);
            EnableAIProduction(trig->house);
            break;
        }

        case RA_ACTION_CREATE_TEAM: {
            // param1 = team type index
            fprintf(stderr, "  TRIGGER: Create team %d\n", param1);
            if (mission && param1 >= 0 && param1 < mission->teamTypeCount) {
                const MissionTeamType* team = &mission->teamTypes[param1];
                fprintf(stderr, "    Team '%s': %d members at waypoint %d\n",
                        team->name, team->memberCount, team->origin);

                // Spawn all team members
                int spawnedIds[32];
                int spawnCount = SpawnTeamUnits(team, mission, spawnedIds, 32);

                // Track spawned units for DESTROY_TEAM
                for (int i = 0; i < spawnCount; i++) {
                    TrackTeamUnit(param1, spawnedIds[i]);
                }

                // Execute the team's first mission
                if (spawnCount > 0) {
                    ExecuteTeamMission(team, mission, spawnedIds, spawnCount);
                    fprintf(stderr, "    Created %d units for team '%s'\n",
                            spawnCount, team->name);
                }
            }
            break;
        }

        case RA_ACTION_DESTROY_TEAM:
            // param1 = team type index
            fprintf(stderr, "  TRIGGER: Destroy team %d\n", param1);
            DestroyTeamUnits(param1);
            break;

        case RA_ACTION_ALL_HUNT: {
            // Get the team for this trigger's house
            Team huntTeam = HouseToTeam(trig->house);
            int huntCount = Units_CommandAllHunt(huntTeam);
            fprintf(stderr, "  TRIGGER: All hunt (house %d -> team %d)\n",
                    trig->house, huntTeam);
            fprintf(stderr, "    Set %d units to hunt mode\n", huntCount);
            break;
        }

        case RA_ACTION_REINFORCE: {
            // param1 = team type index
            // Reinforcement spawns at team's origin waypoint (same as CREATE_TEAM)
            // Difference: REINFORCE typically implies arrival from map edge or
            // transport, but we spawn at waypoint for simplicity
            fprintf(stderr, "  TRIGGER: Reinforcement action (team %d)\n",
                    param1);
            if (mission && param1 >= 0 && param1 < mission->teamTypeCount) {
                const MissionTeamType* team = &mission->teamTypes[param1];
                fprintf(stderr, "    Team '%s': %d members at waypoint %d\n",
                        team->name, team->memberCount, team->origin);

                // Spawn all team members
                int spawnedIds[32];
                int spawnCount = SpawnTeamUnits(team, mission, spawnedIds, 32);

                // Track spawned units for DESTROY_TEAM
                for (int i = 0; i < spawnCount; i++) {
                    TrackTeamUnit(param1, spawnedIds[i]);
                }

                // Execute the team's first mission
                if (spawnCount > 0) {
                    ExecuteTeamMission(team, mission, spawnedIds, spawnCount);
                    fprintf(stderr, "    Reinforced %d units for team '%s'\n",
                            spawnCount, team->name);
                }
            }
            break;
        }

        case RA_ACTION_DZ: {
            // param3 = waypoint for drop zone flare
            fprintf(stderr, "  TRIGGER: Drop zone at waypoint %d\n", param3);
            int dzX, dzY;
            if (Mission_GetWaypoint(mission, param3, &dzX, &dzY)) {
                AddDropZoneFlare(dzX, dzY);
                fprintf(stderr, "    Flare at world %d,%d\n", dzX, dzY);
            }
            break;
        }

        case RA_ACTION_FIRE_SALE: {
            // Sell all buildings for the trigger's house
            fprintf(stderr, "  TRIGGER: Fire sale for house %d\n", trig->house);
            // Iterate buildings and sell those owned by this house
            for (int i = 0; i < MAX_BUILDINGS; i++) {
                Building* bld = Buildings_Get(i);
                if (bld && bld->active) {
                    Team bldTeam = HouseToTeam(trig->house);
                    if (bld->team == (uint8_t)bldTeam) {
                        Buildings_Remove(i);
                    }
                }
            }
            break;
        }

        case RA_ACTION_PLAY_MOVIE: {
            // param3 = movie index (would need text table lookup)
            // For now, use common briefing videos
            fprintf(stderr, "  TRIGGER: Play movie (id=%d)\n", param3);
            // Simple mapping of common movie IDs
            const char* movieName = nullptr;
            switch (param3) {
                case 0: movieName = "ALLY1.VQA"; break;
                case 1: movieName = "ALLY2.VQA"; break;
                case 2: movieName = "SOVT1.VQA"; break;
                case 3: movieName = "SOVT2.VQA"; break;
                default: movieName = nullptr; break;
            }
            if (movieName) {
                VQA_Play(movieName);
            }
            break;
        }

        case RA_ACTION_TEXT: {
            // param3 = text ID from mission strings
            fprintf(stderr, "  TRIGGER: Display text ID %d\n", param3);
            // Simple canned messages for common text IDs
            const char* text = nullptr;
            switch (param3) {
                case 1: text = "Mission objective updated."; break;
                case 2: text = "Reinforcements have arrived!"; break;
                case 3: text = "Warning: Enemy forces detected."; break;
                case 4: text = "Base is under attack!"; break;
                case 5: text = "Objective complete."; break;
                default:
                    // Generic message with ID
                    {
                        static char buf[64];
                        snprintf(buf, sizeof(buf), "Message #%d", param3);
                        text = buf;
                    }
                    break;
            }
            if (text) {
                SetMissionText(text, 15 * 5);  // Display for 5 seconds
            }
            break;
        }

        case RA_ACTION_DESTR_TRIG:
            // param3 = trigger ID to destroy
            fprintf(stderr, "  TRIGGER: Destroy trigger %d\n", param3);
            // Deactivate the specified trigger
            if (param3 >= 0 && param3 < g_parsedTriggerCount) {
                g_parsedTriggers[param3].active = false;
            }
            break;

        case RA_ACTION_AUTOCREATE:
            // Enable automatic team creation for this trigger's house
            fprintf(stderr, "  TRIGGER: Auto-create teams ON for house %d\n",
                    trig->house);
            EnableAIAutocreate(trig->house);
            break;

        case RA_ACTION_REVEAL_ALL:
            fprintf(stderr, "  TRIGGER: Reveal entire map\n");
            Map_RevealAll();
            break;

        case RA_ACTION_REVEAL_SOME: {
            // param3 = waypoint number
            fprintf(stderr, "  TRIGGER: Reveal around wp %d\n", param3);
            bool validWp = mission && param3 >= 0;
            validWp = validWp && param3 < MAX_MISSION_WAYPOINTS;
            validWp = validWp && mission->waypoints[param3].cell >= 0;
            if (validWp) {
                // Convert waypoint cell to world coordinates
                const MissionWaypoint* wp = &mission->waypoints[param3];
                int wpX = (wp->cellX - mission->mapX) * CELL_SIZE;
                int wpY = (wp->cellY - mission->mapY) * CELL_SIZE;
                // Reveal 5-cell radius around waypoint
                Map_RevealArea(wpX, wpY, 5 * CELL_SIZE);
            }
            break;
        }

        case RA_ACTION_FORCE_TRIG:
            // param3 = trigger ID to force (by index in our array)
            // Note: In the real game, this uses trigger name lookup
            fprintf(stderr, "  TRIGGER: Force trigger %d\n", param3);
            if (param3 >= 0 && param3 < g_parsedTriggerCount) {
                // Mark this trigger as ready to fire on next process
                // We can't directly execute here due to recursion concerns
                // So we'll mark it to fire on next frame via a flag
                g_parsedTriggers[param3].active = true;
            }
            break;

        case RA_ACTION_START_TIMER:
            // param3 = timer value (in 1/10th minutes, so 10 = 1 minute)
            // Convert to frames: value * 6 seconds * 15 fps = value * 90 frames
            g_missionTimerValue = param3 * 90;
            g_missionTimerInitial = g_missionTimerValue;
            g_missionTimerActive = true;
            fprintf(stderr, "  TRIGGER: Start timer %d (%d frames)\n",
                    param3, g_missionTimerValue);
            break;

        case RA_ACTION_STOP_TIMER:
            fprintf(stderr, "  TRIGGER: Stop mission timer\n");
            g_missionTimerActive = false;
            break;

        case RA_ACTION_SET_GLOBAL:
            // param3 = global flag number
            fprintf(stderr, "  TRIGGER: Set global flag %d\n", param3);
            if (param3 >= 0 && param3 < MAX_GLOBAL_FLAGS) {
                g_globalFlags[param3] = true;
            }
            break;

        case RA_ACTION_CLEAR_GLOBAL:
            // param3 = global flag number
            fprintf(stderr, "  TRIGGER: Clear global flag %d\n", param3);
            if (param3 >= 0 && param3 < MAX_GLOBAL_FLAGS) {
                g_globalFlags[param3] = false;
            }
            break;

        case RA_ACTION_DESTROY_OBJ:
            // Destroy all objects attached to this trigger
            fprintf(stderr, "  TRIGGER: Destroy objects with trigger '%s'\n",
                    trig->name);
            Units_DestroyByTrigger(trig->name);
            Buildings_DestroyByTrigger(trig->name);
            break;

        case RA_ACTION_WINLOSE:
            // Win if object captured, lose if destroyed
            // For now, treat as win (capture logic not implemented)
            fprintf(stderr, "  TRIGGER: Win/Lose action (treating as win)\n");
            return 1;

        case RA_ACTION_REVEAL_ZONE:
            // Reveal area around waypoint zone
            // param1 = waypoint number
            fprintf(stderr, "  TRIGGER: Reveal zone %d\n", param1);
            if (mission && param1 >= 0 && param1 < MAX_MISSION_WAYPOINTS) {
                int cellX = mission->waypoints[param1].cellX;
                int cellY = mission->waypoints[param1].cellY;
                Map_RevealArea(cellX * 24, cellY * 24, 5);
            }
            break;

        case RA_ACTION_PLAY_SOUND:
            // Play sound effect
            // param1 = sound ID
            fprintf(stderr, "  TRIGGER: Play sound %d\n", param1);
            // Audio_PlaySound(param1); - would need audio integration
            break;

        case RA_ACTION_PLAY_MUSIC:
            // Play music track
            // param1 = music track ID
            fprintf(stderr, "  TRIGGER: Play music %d\n", param1);
            // Audio_PlayMusic(param1); - would need audio integration
            break;

        case RA_ACTION_PLAY_SPEECH:
            // Play EVA speech
            // param1 = speech ID
            fprintf(stderr, "  TRIGGER: Play speech %d\n", param1);
            // Audio_PlaySpeech(param1); - would need audio integration
            break;

        case RA_ACTION_ADD_TIMER:
            // Add time to mission timer
            // param1 = frames to add (or param2 * 60 for minutes)
            fprintf(stderr, "  TRIGGER: Add time to timer: %d frames\n", param1);
            if (g_missionTimerActive) {
                g_missionTimerValue += param1;
            }
            break;

        case RA_ACTION_SUB_TIMER:
            // Subtract time from mission timer
            // param1 = frames to subtract
            fprintf(stderr, "  TRIGGER: Subtract time from timer: %d frames\n",
                    param1);
            if (g_missionTimerActive) {
                g_missionTimerValue -= param1;
                if (g_missionTimerValue < 0) g_missionTimerValue = 0;
            }
            break;

        case RA_ACTION_SET_TIMER:
            // Set and start timer
            // param1 = frames (or param1 * 60 for minutes)
            fprintf(stderr, "  TRIGGER: Set timer to %d frames\n", param1);
            g_missionTimerActive = true;
            g_missionTimerValue = param1;
            break;

        case RA_ACTION_BASE_BUILDING:
            // Enable auto base building for AI
            fprintf(stderr, "  TRIGGER: Enable AI base building\n");
            // AI_EnableBaseBuilding(); - would need AI integration
            break;

        case RA_ACTION_GROW_SHROUD:
            // Grow shroud one step (creep shadow)
            fprintf(stderr, "  TRIGGER: Grow shroud one step\n");
            // Map_GrowShroud(); - would need fog of war integration
            break;

        case RA_ACTION_1_SPECIAL:
            // Grant one-time special weapon
            // param1 = special weapon type
            fprintf(stderr, "  TRIGGER: Grant one-time special %d\n", param1);
            // Player_GrantSpecial(param1, false); - would need special weapon
            break;

        case RA_ACTION_FULL_SPECIAL:
            // Grant repeating special weapon
            // param1 = special weapon type
            fprintf(stderr, "  TRIGGER: Grant full special %d\n", param1);
            // Player_GrantSpecial(param1, true); - would need special weapon
            break;

        case RA_ACTION_PREF_TARGET:
            // Set preferred attack target for AI
            // param1 = target type (0=anything, 1=buildings, etc.)
            fprintf(stderr, "  TRIGGER: Set preferred target %d\n", param1);
            // AI_SetPreferredTarget(param1); - would need AI integration
            break;

        case RA_ACTION_LAUNCH_NUKES:
            // Launch fake nuclear missiles
            fprintf(stderr, "  TRIGGER: Launch fake nukes\n");
            // FX_LaunchNukes(); - would need FX system
            break;

        default:
            // Log unsupported actions for debugging
            fprintf(stderr, "  TRIGGER: Unknown action %d\n", actionNum);
            break;
    }

    return 0;
}

int Mission_ProcessTriggers(const MissionData* mission, int frameCount) {
    if (!mission) return 0;

    int result = 0;

    for (int i = 0; i < g_parsedTriggerCount; i++) {
        ParsedTrigger* trig = &g_parsedTriggers[i];
        if (!trig->active) continue;

        // Check event1
        bool event1Fired = CheckTriggerEvent(trig, trig->event1,
                                              trig->e1p1, trig->e1p2,
                                              frameCount, mission);

        // Check event2 if using AND/OR control
        bool event2Fired = false;
        if (trig->eventControl != 0) {  // Not "ONLY"
            event2Fired = CheckTriggerEvent(trig, trig->event2,
                                             trig->e2p1, trig->e2p2,
                                             frameCount, mission);
        }

        // Determine if trigger should fire
        bool shouldFire = false;
        switch (trig->eventControl) {
            case 0:  // ONLY - just event1
                shouldFire = event1Fired;
                break;
            case 1:  // AND - both events
                shouldFire = event1Fired && event2Fired;
                break;
            case 2:  // OR - either event
                shouldFire = event1Fired || event2Fired;
                break;
            case 3:  // LINKED - events linked to actions
                shouldFire = event1Fired || event2Fired;
                break;
        }

        if (!shouldFire) continue;

        fprintf(stderr, "  TRIGGER '%s' fired!\n", trig->name);

        // Execute action1
        int actionResult = ExecuteTriggerAction(trig, trig->action1,
                                                 trig->a1p1, trig->a1p2,
                                                 trig->a1p3, mission);
        if (actionResult != 0) {
            result = actionResult;
        }

        // Execute action2 if using AND control
        if (trig->actionControl != 0) {  // Not "ONLY"
            actionResult = ExecuteTriggerAction(trig, trig->action2,
                                                 trig->a2p1, trig->a2p2,
                                                 trig->a2p3, mission);
            if (actionResult != 0) {
                result = actionResult;
            }
        }

        // Handle persistence
        if (trig->persist == 0) {  // Volatile - fire once
            trig->active = false;
        }
        // Semi-persistent (1) and persistent (2) can fire again
    }

    return result;
}

int Mission_GetWaypoint(const MissionData* mission, int waypointNum,
                         int* outX, int* outY) {
    if (!mission || waypointNum < 0 ||
        waypointNum >= MAX_MISSION_WAYPOINTS) {
        return 0;
    }

    const MissionWaypoint* wp = &mission->waypoints[waypointNum];
    if (wp->cell < 0) {
        return 0;
    }

    // Convert from map cell coords to world coords
    int localCellX = wp->cellX - mission->mapX;
    int localCellY = wp->cellY - mission->mapY;

    if (localCellX < 0 || localCellX >= mission->mapWidth ||
        localCellY < 0 || localCellY >= mission->mapHeight) {
        return 0;
    }

    *outX = localCellX * CELL_SIZE + CELL_SIZE / 2;
    *outY = localCellY * CELL_SIZE + CELL_SIZE / 2;

    return 1;
}
