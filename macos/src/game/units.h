/**
 * Red Alert macOS Port - Unit/Entity System
 *
 * Handles all game units (infantry, vehicles, buildings).
 */

#ifndef GAME_UNITS_H
#define GAME_UNITS_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum units
#define MAX_UNITS       256
#define MAX_BUILDINGS   128

// Unit types
typedef enum {
    UNIT_NONE = 0,
    // Infantry
    UNIT_RIFLE,         // Rifle infantry
    UNIT_GRENADIER,     // Grenade infantry
    UNIT_ROCKET,        // Rocket soldier
    UNIT_ENGINEER,      // Engineer
    // Vehicles
    UNIT_HARVESTER,     // Ore harvester
    UNIT_TANK_LIGHT,    // Light tank
    UNIT_TANK_MEDIUM,   // Medium tank
    UNIT_TANK_HEAVY,    // Heavy/Mammoth tank
    UNIT_APC,           // Armored personnel carrier
    UNIT_ARTILLERY,     // Artillery
    // Naval
    UNIT_GUNBOAT,       // Gunboat
    UNIT_DESTROYER,     // Destroyer
    UNIT_TYPE_COUNT
} UnitType;

// Building types
typedef enum {
    BUILDING_NONE = 0,
    BUILDING_CONSTRUCTION,  // Construction yard
    BUILDING_POWER,         // Power plant
    BUILDING_REFINERY,      // Ore refinery
    BUILDING_BARRACKS,      // Infantry production
    BUILDING_FACTORY,       // Vehicle production
    BUILDING_RADAR,         // Radar dome
    BUILDING_TURRET,        // Gun turret
    BUILDING_SAM,           // SAM site
    BUILDING_TYPE_COUNT
} BuildingType;

// Team/player
typedef enum {
    TEAM_NEUTRAL = 0,
    TEAM_PLAYER,        // Human player (Allies)
    TEAM_ENEMY,         // AI enemy (Soviet)
    TEAM_COUNT
} Team;

// Unit state
typedef enum {
    STATE_IDLE = 0,
    STATE_MOVING,
    STATE_ATTACKING,
    STATE_HARVESTING,
    STATE_RETURNING,
    STATE_DYING,
    STATE_ATTACK_MOVE,      // Moving but will attack enemies in range
    STATE_GUARDING          // Stationary but attacks nearby enemies
} UnitState;

// Maximum path waypoints per unit
#define MAX_PATH_WAYPOINTS 32

// Harvester constants
#define HARVESTER_MAX_CARGO     1000    // Max ore a harvester can carry
#define HARVESTER_LOAD_RATE     50      // Ore harvested per tick while harvesting
#define ORE_VALUE               7       // Credits per unit of ore

// Unit structure
typedef struct {
    uint8_t type;           // UnitType
    uint8_t team;           // Team
    uint8_t state;          // UnitState
    uint8_t facing;         // Direction (0-7, N/NE/E/SE/S/SW/W/NW)
    int16_t health;         // Current health
    int16_t maxHealth;      // Maximum health
    int32_t worldX;         // Position X (sub-pixel)
    int32_t worldY;         // Position Y (sub-pixel)
    int32_t targetX;        // Final destination X
    int32_t targetY;        // Final destination Y
    int16_t targetUnit;     // Attack target unit ID (-1 if none)
    int16_t speed;          // Movement speed
    int16_t attackRange;    // Attack range in pixels
    int16_t attackDamage;   // Damage per attack
    int16_t attackCooldown; // Current cooldown
    int16_t attackRate;     // Ticks between attacks
    int16_t sightRange;     // Sight range in cells (fog of war)
    uint8_t selected;       // Is unit selected?
    uint8_t active;         // Is unit slot active?
    // Path following
    int16_t pathCells[MAX_PATH_WAYPOINTS]; // Cell indices for path
    int8_t pathLength;      // Total waypoints in path
    int8_t pathIndex;       // Current waypoint index
    int32_t nextWaypointX;  // Current waypoint world X
    int32_t nextWaypointY;  // Current waypoint world Y
    // Harvester-specific fields
    int16_t cargo;          // Ore currently carried (0-1000)
    int16_t homeRefinery;   // Building ID of assigned refinery (-1 if none)
    int16_t harvestTimer;   // Ticks remaining in current harvest action
    // Combat behavior fields
    int16_t lastAttacker;   // Unit ID that last attacked this unit (-1 if none)
    int16_t scatterTimer;   // Ticks remaining before scatter movement allowed again
} Unit;

// Building structure
typedef struct {
    uint8_t type;           // BuildingType
    uint8_t team;           // Team
    int16_t health;         // Current health
    int16_t maxHealth;      // Maximum health
    int16_t cellX;          // Position in cells
    int16_t cellY;
    uint8_t width;          // Size in cells
    uint8_t height;
    uint8_t selected;       // Is building selected?
    uint8_t active;         // Is building slot active?
    int16_t attackCooldown; // For turrets
    int16_t sightRange;     // Sight range in cells (fog of war)
} Building;

/**
 * Initialize the unit system
 */
void Units_Init(void);

/**
 * Shutdown the unit system
 */
void Units_Shutdown(void);

/**
 * Clear all units and buildings
 */
void Units_Clear(void);

/**
 * Spawn a unit
 * @return Unit ID, or -1 on failure
 */
int Units_Spawn(UnitType type, Team team, int worldX, int worldY);

/**
 * Remove a unit
 */
void Units_Remove(int unitId);

/**
 * Get unit by ID
 */
Unit* Units_Get(int unitId);

/**
 * Get unit count for a team
 */
int Units_CountByTeam(Team team);

/**
 * Spawn a building
 * @return Building ID, or -1 on failure
 */
int Buildings_Spawn(BuildingType type, Team team, int cellX, int cellY);

/**
 * Remove a building
 */
void Buildings_Remove(int buildingId);

/**
 * Get building by ID
 */
Building* Buildings_Get(int buildingId);

/**
 * Command unit to move to position
 */
void Units_CommandMove(int unitId, int worldX, int worldY);

/**
 * Command unit to attack target
 */
void Units_CommandAttack(int unitId, int targetUnitId);

/**
 * Command unit to stop
 */
void Units_CommandStop(int unitId);

/**
 * Command unit to attack-move to position (attack enemies encountered)
 */
void Units_CommandAttackMove(int unitId, int worldX, int worldY);

/**
 * Command unit to guard position (stay but attack nearby enemies)
 */
void Units_CommandGuard(int unitId);

/**
 * Command unit to force-attack target (even friendlies or ground)
 */
void Units_CommandForceAttack(int unitId, int worldX, int worldY);

/**
 * Notify that a unit was attacked (for return fire behavior)
 * @param victimId Unit that was attacked
 * @param attackerId Unit that attacked
 */
void Units_NotifyAttacked(int victimId, int attackerId);

/**
 * Cause infantry near a position to scatter (from explosions)
 * @param worldX, worldY Center of explosion
 * @param radius Scatter radius in pixels
 */
void Units_ScatterInfantryNear(int worldX, int worldY, int radius);

/**
 * Select unit
 */
void Units_Select(int unitId, BOOL addToSelection);

/**
 * Deselect all units
 */
void Units_DeselectAll(void);

/**
 * Get first selected unit ID
 * @return Unit ID, or -1 if none selected
 */
int Units_GetFirstSelected(void);

/**
 * Get selected unit count
 */
int Units_GetSelectedCount(void);

/**
 * Select units in rectangle (screen coordinates)
 */
void Units_SelectInRect(int x1, int y1, int x2, int y2, Team team);

/**
 * Get unit at screen position
 * @return Unit ID, or -1 if none
 */
int Units_GetAtScreen(int screenX, int screenY);

/**
 * Update all units
 */
void Units_Update(void);

/**
 * Render all units and buildings
 */
void Units_Render(void);

/**
 * Set pointer to player credits for harvester income
 * (Called from game_ui.cpp)
 */
void Units_SetCreditsPtr(int* creditsPtr);

#ifdef __cplusplus
}
#endif

#endif // GAME_UNITS_H
