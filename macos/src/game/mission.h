/**
 * Red Alert macOS Port - Mission Loader
 *
 * Loads mission data from INI files and spawns game entities.
 */

#ifndef GAME_MISSION_H
#define GAME_MISSION_H

#include "units.h"
#include "map.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
class INIClass;

// Maximum entities per mission
#define MAX_MISSION_UNITS       64
#define MAX_MISSION_BUILDINGS   32
#define MAX_MISSION_TRIGGERS    32

// Mission unit placement data
typedef struct {
    UnitType type;
    Team team;
    int16_t cellX;
    int16_t cellY;
} MissionUnit;

// Mission building placement data
typedef struct {
    BuildingType type;
    Team team;
    int16_t cellX;
    int16_t cellY;
} MissionBuilding;

// Mission trigger (simplified)
typedef struct {
    int eventType;      // TEventType
    int actionType;     // TActionType
    int value;          // Event/action data
} MissionTrigger;

// Mission data
typedef struct {
    // Identity
    char name[64];
    char description[512];

    // Videos
    char briefVideo[32];    // Pre-mission briefing video (e.g., "ALLY1")
    char winVideo[32];      // Victory video
    char loseVideo[32];     // Defeat video

    // Map
    int theater;        // 0=temperate, 1=snow, 2=interior, 3=desert
    int mapX;           // Map viewport X offset
    int mapY;           // Map viewport Y offset
    int mapWidth;
    int mapHeight;

    // Player
    Team playerTeam;
    int startCredits;

    // Units
    MissionUnit units[MAX_MISSION_UNITS];
    int unitCount;

    // Buildings
    MissionBuilding buildings[MAX_MISSION_BUILDINGS];
    int buildingCount;

    // Triggers
    MissionTrigger triggers[MAX_MISSION_TRIGGERS];
    int triggerCount;

    // Win/Lose conditions (simplified)
    int winCondition;   // 0=destroy all, 1=destroy buildings, 2=survive time
    int loseCondition;  // 0=lose all units, 1=lose buildings, 2=time expires
    int timeLimit;      // In frames (0=unlimited)
} MissionData;

/**
 * Initialize mission data to defaults
 */
void Mission_Init(MissionData* mission);

/**
 * Load mission from INI file
 * @return true on success
 */
int Mission_LoadFromINI(MissionData* mission, const char* filename);

/**
 * Load mission from already-parsed INI data
 * @return true on success
 */
int Mission_LoadFromINIClass(MissionData* mission, INIClass* ini);

/**
 * Load mission from memory buffer (INI format)
 * @return true on success
 */
int Mission_LoadFromBuffer(MissionData* mission, const char* buffer, int size);

/**
 * Start a loaded mission (spawns all entities)
 */
void Mission_Start(const MissionData* mission);

/**
 * Check win/lose conditions
 * @return 1=win, -1=lose, 0=ongoing
 */
int Mission_CheckVictory(const MissionData* mission);

/**
 * Get current demo mission (hardcoded for testing)
 */
void Mission_GetDemo(MissionData* mission);

#ifdef __cplusplus
}
#endif

#endif // GAME_MISSION_H
