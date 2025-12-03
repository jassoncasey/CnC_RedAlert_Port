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
#define MAX_MISSION_WAYPOINTS   100
#define MAX_TEAM_TYPES          32
#define MAX_TEAM_MEMBERS        5
#define MAX_TEAM_MISSIONS       20
#define MAX_MISSION_TERRAIN     64
#define MAX_MISSION_SMUDGE      32

// Map constants (Red Alert uses 128x128 cell maps)
#define MAP_CELL_W              128
#define MAP_CELL_H              128
#define MAP_CELL_TOTAL          (MAP_CELL_W * MAP_CELL_H)

// Unit/entity mission types (simplified from original)
typedef enum {
    MISSION_NONE = 0,
    MISSION_GUARD,      // Stay in place, attack if attacked
    MISSION_HUNT,       // Seek and destroy enemies
    MISSION_SLEEP,      // Inactive (civilians, triggers)
    MISSION_HARVEST,    // Collect ore/gems
    MISSION_ATTACK,     // Attack a specific target
    MISSION_GUARD_AREA, // Guard an area
    MISSION_RETREAT,    // Return to base
} MissionType;

// Mission unit placement data
typedef struct {
    UnitType type;
    Team team;
    int16_t cellX;
    int16_t cellY;
    int16_t health;     // Starting health (0-256, 256=full)
    int16_t facing;     // Direction (0-255, 0=N, 64=E, 128=S, 192=W)
    MissionType mission; // Initial mission
    int16_t subCell;    // Sub-cell position for infantry (0-4)
    char triggerName[24]; // Attached trigger name (for ATTACKED/DESTROYED events)
} MissionUnit;

// Mission building placement data
typedef struct {
    BuildingType type;
    Team team;
    int16_t cellX;
    int16_t cellY;
    int16_t health;     // Starting health (0-256)
    int16_t facing;     // Turret direction
    int8_t sellable;    // Can be sold by player
    int8_t rebuild;     // AI will rebuild if destroyed
    char triggerName[24]; // Attached trigger name (for ATTACKED/DESTROYED events)
} MissionBuilding;

// Mission trigger (simplified)
typedef struct {
    int eventType;      // TEventType
    int actionType;     // TActionType
    int value;          // Event/action data
} MissionTrigger;

// Waypoint (spawn point, movement target, etc.)
typedef struct {
    int cell;           // Cell number (128-width map)
    int16_t cellX;      // Extracted X coordinate
    int16_t cellY;      // Extracted Y coordinate
} MissionWaypoint;

// Team member (unit type and quantity)
typedef struct {
    char unitType[8];   // Unit type name (E1, 1TNK, etc.)
    int quantity;       // Number of units
} TeamMember;

// Team mission (action and data)
typedef struct {
    int mission;        // Team mission type (0=Attack, 3=Move, etc.)
    int data;           // Mission data (waypoint #, etc.)
} TeamMission;

// Team type definition (AI team composition and behavior)
typedef struct {
    char name[24];          // Team name (from INI key)
    int house;              // House/owner (0=Spain, 2=USSR, etc.)
    int flags;              // Packed team behavior flags
    int recruitPriority;    // Priority for recruiting
    int initNum;            // Initial number to create
    int maxAllowed;         // Maximum allowed
    int origin;             // Origin waypoint
    int trigger;            // Associated trigger ID
    TeamMember members[MAX_TEAM_MEMBERS];
    int memberCount;
    TeamMission missions[MAX_TEAM_MISSIONS];
    int missionCount;
} MissionTeamType;

// Terrain object (trees, etc. from [TERRAIN] section)
typedef struct {
    char type[8];           // Type name (T01, TC03, etc.)
    int16_t cellX;
    int16_t cellY;
} MissionTerrainObj;

// Smudge (craters, scorch marks from [SMUDGE] section)
typedef struct {
    char type[8];           // Type name (CR1, SC2, etc.)
    int16_t cellX;
    int16_t cellY;
    int16_t data;           // Smudge variant/frame
} MissionSmudge;

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

    // Waypoints (indexed by waypoint number)
    MissionWaypoint waypoints[MAX_MISSION_WAYPOINTS];
    int waypointCount;

    // Team types (AI team definitions)
    MissionTeamType teamTypes[MAX_TEAM_TYPES];
    int teamTypeCount;

    // Terrain objects (trees, etc. from [TERRAIN])
    MissionTerrainObj terrainObjs[MAX_MISSION_TERRAIN];
    int terrainObjCount;

    // Smudges (craters, scorch marks from [SMUDGE])
    MissionSmudge smudges[MAX_MISSION_SMUDGE];
    int smudgeCount;

    // Cell triggers (from [CellTriggers])
    // Maps cell number to trigger name index (-1 if none)
    // Stored as sparse array: cellTriggerCells[i] = cell number,
    //                        cellTriggerNames[i] = trigger name
    int cellTriggerCells[256];      // Cell numbers with triggers
    char cellTriggerNames[256][24]; // Trigger names for each cell
    int cellTriggerCount;

    // Object triggers (from STRUCTURES/UNITS/INFANTRY trigger fields)
    // Tracks cells where objects with triggers are placed
    int objectTriggerCells[256];      // Cell numbers with trigger-attached objects
    char objectTriggerNames[256][24]; // Trigger names for each object
    int objectTriggerCount;

    // Base section (AI build order info)
    int baseHouse;          // House that owns the base (-1 if not set)
    int baseCount;          // Number of base structures (for AI rebuild)

    // Win/Lose conditions (0=destroy all, 1=buildings, etc)
    int winCondition;
    int loseCondition;
    int timeLimit;      // In frames (0=unlimited)
    int targetCell;     // Target cell for capture/protect (-1=none)

    // Terrain data from [MapPack] - per cell
    uint8_t* terrainType;   // Template type index (MAP_CELL_TOTAL bytes)
    uint8_t* terrainIcon;   // Tile index within template (MAP_CELL_TOTAL bytes)

    // Overlay data from [OverlayPack] - per cell
    uint8_t* overlayType;   // Overlay type (ore, walls, etc)
    uint8_t* overlayData;   // Overlay variant/frame
} MissionData;

/**
 * Initialize mission data to defaults
 */
void Mission_Init(MissionData* mission);

/**
 * Free mission terrain/overlay data
 */
void Mission_Free(MissionData* mission);

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
 * @param mission Active mission data
 * @param frameCount Current game frame number (for time-based conditions)
 * @return 1=win, -1=lose, 0=ongoing
 */
int Mission_CheckVictory(const MissionData* mission, int frameCount);

/**
 * Get current demo mission (hardcoded for testing)
 */
void Mission_GetDemo(MissionData* mission);

/**
 * Process triggers each frame (call from game loop)
 * @param mission Active mission data
 * @param frameCount Current game frame number
 * @return 1=win triggered, -1=lose triggered, 0=continue
 */
int Mission_ProcessTriggers(const MissionData* mission, int frameCount);

/**
 * Get waypoint world coordinates
 * @param mission Active mission data
 * @param waypointNum Waypoint number (0-99)
 * @param outX Output world X coordinate
 * @param outY Output world Y coordinate
 * @return true if waypoint exists
 */
int Mission_GetWaypoint(const MissionData* mission, int waypointNum,
                        int* outX, int* outY);

/**
 * Notify trigger system that an object was attacked
 * Called by TechnoClass::TakeDamage when object takes damage
 * @param triggerName Name of trigger attached to the object
 */
void Mission_TriggerAttacked(const char* triggerName);

/**
 * Notify trigger system that an object was destroyed
 * Called when object dies (Record_The_Kill equivalent)
 * @param triggerName Name of trigger attached to the object
 */
void Mission_TriggerDestroyed(const char* triggerName);

/**
 * Notify trigger system that a civilian was evacuated
 * Called when civilian reaches map edge/extraction point
 * @param triggerName Name of trigger attached to the civilian
 */
void Mission_TriggerEvacuated(const char* triggerName);

/**
 * Check if mission timer is active
 */
bool Mission_IsTimerActive(void);

/**
 * Get current timer value in frames
 */
int Mission_GetTimerValue(void);

/**
 * Update timer (call once per frame when game is running)
 */
void Mission_UpdateTimer(void);

/**
 * Reset timer state (call when mission ends/restarts)
 */
void Mission_ResetTimer(void);

/**
 * Get current mission text to display (or NULL if none)
 */
const char* Mission_GetDisplayText(void);

/**
 * Update mission text display timer (call once per frame)
 */
void Mission_UpdateDisplayText(void);

/**
 * Get drop zone flare data for rendering
 * @param index Flare index (0 to MAX_DZ_FLARES-1)
 * @param worldX Output X coordinate (if returns true)
 * @param worldY Output Y coordinate (if returns true)
 * @return true if flare is active
 */
int Mission_GetDropZoneFlare(int index, int* worldX, int* worldY);

/**
 * Update drop zone flares (call once per frame)
 */
void Mission_UpdateDropZoneFlares(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_MISSION_H
