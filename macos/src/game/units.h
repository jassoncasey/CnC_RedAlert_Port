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
    // Infantry - Military
    UNIT_RIFLE,         // E1 - Rifle infantry
    UNIT_GRENADIER,     // E2 - Grenade infantry
    UNIT_ROCKET,        // E3 - Rocket soldier
    UNIT_FLAMETHROWER,  // E4 - Flamethrower infantry
    UNIT_ENGINEER,      // E6 - Engineer
    UNIT_TANYA,         // E7 - Tanya
    UNIT_DOG,           // DOG - Attack dog
    UNIT_SPY,           // SPY/E5 - Spy
    UNIT_MEDIC,         // MEDI - Medic
    UNIT_THIEF,         // THF - Thief
    UNIT_SHOCK,         // SHOK - Shock trooper
    UNIT_GENERAL,       // GNRL - General (special mission unit)
    // Infantry - Civilians
    UNIT_CIVILIAN_1,    // C1 - Civilian
    UNIT_CIVILIAN_2,    // C2 - Civilian
    UNIT_CIVILIAN_3,    // C3 - Civilian
    UNIT_CIVILIAN_4,    // C4 - Civilian
    UNIT_CIVILIAN_5,    // C5 - Civilian
    UNIT_CIVILIAN_6,    // C6 - Civilian
    UNIT_CIVILIAN_7,    // C7 - Civilian (technician)
    UNIT_CIVILIAN_8,    // C8 - Civilian (scientist - Einstein)
    UNIT_CIVILIAN_9,    // C9 - Civilian
    UNIT_CIVILIAN_10,   // C10 - Civilian
    UNIT_CHAN,          // CHAN - Special civilian (crate?)
    // Vehicles
    UNIT_HARVESTER,     // HARV - Ore harvester
    UNIT_TANK_LIGHT,    // 1TNK - Light tank
    UNIT_TANK_MEDIUM,   // 2TNK - Medium tank
    UNIT_TANK_HEAVY,    // 3TNK - Heavy/Mammoth tank
    UNIT_TANK_MAMMOTH,  // 4TNK - Mammoth tank (Soviet)
    UNIT_APC,           // APC - Armored personnel carrier
    UNIT_ARTILLERY,     // ARTY - Artillery
    UNIT_JEEP,          // JEEP - Ranger/Jeep
    UNIT_MCV,           // MCV - Mobile construction vehicle
    UNIT_V2RL,          // V2RL - V2 Rocket Launcher
    UNIT_MINELAYER,     // MNLY - Minelayer
    UNIT_TRUCK,         // TRUK - Supply truck
    UNIT_CHRONO,        // CTNK - Chrono tank
    UNIT_MOBILE_GAP,    // MGG - Mobile Gap Generator
    UNIT_MOBILE_RADAR,  // MRJ - Mobile Radar Jammer
    // Naval
    UNIT_GUNBOAT,       // GNBT - Gunboat
    UNIT_DESTROYER,     // DD - Destroyer
    UNIT_SUBMARINE,     // SS - Submarine
    UNIT_CRUISER,       // CA - Cruiser
    UNIT_TRANSPORT,     // LST - Transport
    UNIT_PT_BOAT,       // PT - PT Boat
    // Aircraft
    UNIT_HIND,          // HIND - Hind helicopter
    UNIT_LONGBOW,       // HELI - Longbow helicopter
    UNIT_CHINOOK,       // TRAN - Transport helicopter
    UNIT_YAK,           // YAK - Yak attack plane
    UNIT_MIG,           // MIG - MiG attack plane
    UNIT_TYPE_COUNT
} UnitType;

// Building types
typedef enum {
    BUILDING_NONE = 0,
    // Core structures
    BUILDING_CONSTRUCTION,  // FACT - Construction yard
    BUILDING_POWER,         // POWR - Power plant
    BUILDING_ADV_POWER,     // APWR - Advanced power plant
    BUILDING_REFINERY,      // PROC - Ore refinery
    BUILDING_SILO,          // SILO - Ore silo
    // Production
    BUILDING_BARRACKS,      // TENT/BARR - Infantry production
    BUILDING_FACTORY,       // WEAP - Vehicle production
    BUILDING_AIRFIELD,      // AFLD - Airfield
    BUILDING_HELIPAD,       // HPAD - Helipad
    BUILDING_SHIPYARD,      // SYRD - Naval yard
    BUILDING_SUB_PEN,       // SPEN - Submarine pen
    // Tech
    BUILDING_RADAR,         // DOME - Radar dome
    BUILDING_TECH_CENTER,   // ATEK/STEK - Tech center
    BUILDING_KENNEL,        // KENN - Kennel (dog training)
    BUILDING_BIO_LAB,       // BIO - Bio-research lab
    BUILDING_FORWARD_COM,   // FCOM - Forward command post
    BUILDING_MISSION,       // MISS - Mission control
    // Defense
    BUILDING_TURRET,        // GUN - Gun turret
    BUILDING_SAM,           // SAM - SAM site
    BUILDING_TESLA,         // TSLA - Tesla coil
    BUILDING_AA_GUN,        // AGUN - Anti-aircraft gun
    BUILDING_PILLBOX,       // PBOX - Pillbox
    BUILDING_CAMO_PILLBOX,  // HBOX - Camo pillbox
    BUILDING_FLAME_TOWER,   // FTUR - Flame tower
    BUILDING_GAP,           // GAP - Gap generator
    BUILDING_MINE_AP,       // MINP - Anti-personnel mine
    BUILDING_MINE_AV,       // MINV - Anti-vehicle mine
    // Special
    BUILDING_FIX,           // FIX - Service depot
    BUILDING_IRON_CURTAIN,  // IRON - Iron curtain
    BUILDING_CHRONOSPHERE,  // PDOX - Chronosphere
    BUILDING_MISSILE_SILO,  // MSLO - Missile silo
    // Fake structures
    BUILDING_FAKE_CONST,    // FACF - Fake construction yard
    BUILDING_FAKE_FACTORY,  // WEAF - Fake weapons factory
    BUILDING_FAKE_SHIPYARD, // SYRF - Fake shipyard
    BUILDING_FAKE_RADAR,    // DOMF - Fake radar
    // Props
    BUILDING_BARREL,        // BARL - Explosive barrel
    BUILDING_BARREL_3,      // BRL3 - Barrel variant
    // Civilian buildings (V01-V19)
    BUILDING_CIV_01,        // V01 - Church
    BUILDING_CIV_02,        // V02 - Han's house
    BUILDING_CIV_03,        // V03 - Hewitt house
    BUILDING_CIV_04,        // V04 - Ricktor house
    BUILDING_CIV_05,        // V05 - Gretchin house
    BUILDING_CIV_06,        // V06 - Barn
    BUILDING_CIV_07,        // V07 - Windmill
    BUILDING_CIV_08,        // V08 - Fenced house
    BUILDING_CIV_09,        // V09 - Church 2
    BUILDING_CIV_10,        // V10 - Hospital
    BUILDING_CIV_11,        // V11 - Grain silo
    BUILDING_CIV_13,        // V13 - Water tower (V12 unused)
    BUILDING_CIV_19,        // V19 - Oil derrick
    BUILDING_TYPE_COUNT
} BuildingType;

// Team/player (simplified for combat)
typedef enum {
    TEAM_NEUTRAL = 0,
    TEAM_PLAYER,        // Human player (Allies)
    TEAM_ENEMY,         // AI enemy (Soviet)
    TEAM_COUNT
} Team;

// House enum (matches original Red Alert house numbers)
typedef enum {
    HOUSE_SPAIN = 0,    // Allied
    HOUSE_GREECE,       // Allied
    HOUSE_USSR,         // Soviet
    HOUSE_ENGLAND,      // Allied
    HOUSE_UKRAINE,      // Soviet
    HOUSE_GERMANY,      // Allied
    HOUSE_FRANCE,       // Allied
    HOUSE_TURKEY,       // Allied
    HOUSE_COUNT,
    HOUSE_NONE = -1
} HouseType;

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
#define HARVESTER_LOAD_RATE     50      // Ore/tick while harvesting
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
    int16_t scatterTimer;   // Ticks until scatter allowed again
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

/**
 * Convert house number to team (for combat)
 * Soviet houses (USSR, Ukraine) -> TEAM_ENEMY
 * Allied houses -> TEAM_PLAYER
 * @param house House number (0-7)
 * @return Team for combat purposes
 */
Team House_ToTeam(HouseType house);

/**
 * Check if two houses are allies
 * @return true if same side (both Allied or both Soviet)
 */
int House_IsAlly(HouseType h1, HouseType h2);

/**
 * Get house name string
 * @return House name (e.g., "Greece", "USSR")
 */
const char* House_GetName(HouseType house);

#ifdef __cplusplus
}
#endif

#endif // GAME_UNITS_H
