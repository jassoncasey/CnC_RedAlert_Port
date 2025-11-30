/**
 * Red Alert macOS Port - Game Type Definitions
 *
 * Core enumerations and type definitions ported from original source.
 * Based on DEFINES.H from original C&C Red Alert source code.
 */

#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <cstdint>

//===========================================================================
// Infantry Types - All infantry unit types in the game
//===========================================================================
enum class InfantryType : int8_t {
    NONE = -1,
    E1 = 0,         // Rifle Infantry (Mini-gun armed)
    E2,             // Grenadier (Grenade thrower)
    E3,             // Rocket Soldier (Rocket launcher)
    E4,             // Flamethrower
    RENOVATOR,      // Engineer
    TANYA,          // Tanya (Saboteur)
    SPY,            // Spy
    THIEF,          // Thief
    MEDIC,          // Field Medic
    GENERAL,        // Field Marshal
    DOG,            // Soviet Attack Dog

    // Civilians
    C1,             // Civilian (male)
    C2,             // Civilian (male, alternate)
    C3,             // Civilian (female)
    C4,             // Civilian (female, alternate)
    C5,             // Civilian
    C6,             // Civilian
    C7,             // Civilian
    C8,             // Civilian
    C9,             // Civilian
    C10,            // Nikoomba
    EINSTEIN,       // Dr. Einstein
    DELPHI,         // Agent Delphi
    CHAN,           // Dr. Chan

    // Aftermath expansion units
    SHOCK,          // Shock Trooper
    MECHANIC,       // Mechanic

    COUNT,
    FIRST = 0,
    RA_COUNT = SHOCK  // Base game count (before Aftermath)
};

//===========================================================================
// Unit Types - All vehicle unit types in the game
//===========================================================================
enum class UnitType : int8_t {
    NONE = -1,
    HTANK = 0,      // Mammoth Tank
    MTANK,          // Heavy Tank
    MTANK2,         // Medium Tank
    LTANK,          // Light Tank
    APC,            // Armored Personnel Carrier
    MINELAYER,      // Mine Layer
    JEEP,           // Ranger
    HARVESTER,      // Ore Truck
    ARTY,           // Artillery
    MRJ,            // Mobile Radar Jammer
    MGG,            // Mobile Gap Generator
    MCV,            // Mobile Construction Vehicle
    V2_LAUNCHER,    // V2 Rocket Launcher
    TRUCK,          // Supply Truck

    // Ant mission units
    ANT1,           // Warrior Ant (small)
    ANT2,           // Warrior Ant (medium)
    ANT3,           // Warrior Ant (large)

    // Aftermath expansion units
    CHRONOTANK,     // Chrono Tank
    TESLATANK,      // Tesla Tank
    MAD,            // M.A.D. Tank
    DEMOTRUCK,      // Demolition Truck
    PHASE,          // Phase Transport

    COUNT,
    FIRST = 0,
    RA_COUNT = CHRONOTANK  // Base game count (before Aftermath)
};

//===========================================================================
// Building Types - All structure types in the game
// Order must match original DEFINES.H for compatibility with save files
//===========================================================================
enum class BuildingType : int8_t {
    NONE = -1,
    ADVANCED_TECH = 0,  // Allied Tech Center (ATEK)
    IRON_CURTAIN,       // Iron Curtain
    WEAP,               // Weapons Factory
    CHRONOSPHERE,       // Chronosphere (PDOX)
    PILLBOX,            // Pillbox
    CAMOPILLBOX,        // Camo Pillbox
    RADAR,              // Radar Dome
    GAP,                // Gap Generator
    TURRET,             // Gun Turret
    AAGUN,              // Anti-Aircraft Gun
    FLAME_TURRET,       // Flame Turret
    CONST,              // Construction Yard
    REFINERY,           // Ore Refinery
    STORAGE,            // Ore Silo
    HELIPAD,            // Helipad
    SAM,                // SAM Site
    AIRSTRIP,           // Airfield
    POWER,              // Power Plant
    ADVANCED_POWER,     // Advanced Power Plant
    SOVIET_TECH,        // Soviet Tech Center (STEK)
    HOSPITAL,           // Hospital
    BARRACKS,           // Allied Barracks
    TENT,               // Soviet Barracks
    KENNEL,             // Dog Kennel
    REPAIR,             // Service Depot
    BIO_LAB,            // Bio Research Lab
    MISSION,            // Mission control (not used?)
    SHIP_YARD,          // Shipyard
    SUB_PEN,            // Sub Pen
    MSLO,               // Missile Silo
    FORWARD_COM,        // Forward Command
    TESLA,              // Tesla Coil

    // Fake structures (decoys)
    FAKEWEAP,           // Fake Weapons Factory
    FAKECONST,          // Fake Construction Yard
    FAKE_YARD,          // Fake Shipyard
    FAKE_PEN,           // Fake Sub Pen
    FAKE_RADAR,         // Fake Radar

    // Walls
    SANDBAG_WALL,       // Sandbag Wall
    CYCLONE_WALL,       // Chain Link Fence
    BRICK_WALL,         // Concrete Wall
    BARBWIRE_WALL,      // Barbed Wire
    WOOD_WALL,          // Wood Fence
    FENCE,              // Wire Fence

    // Mines
    AVMINE,             // Anti-Vehicle Mine
    APMINE,             // Anti-Personnel Mine

    // Civilian structures
    V01, V02, V03, V04, V05, V06, V07, V08, V09, V10,
    V11, V12, V13, V14, V15, V16, V17, V18,
    PUMP,               // Water Pump (V19)
    V20, V21, V22, V23, V24, V25, V26, V27, V28, V29,
    V30, V31, V32, V33, V34, V35, V36, V37,

    // Special structures
    BARREL,             // Explosive Barrel
    BARREL3,            // 3-Barrel Group

    // Ant mission structures
    QUEEN,              // Ant Queen
    LARVA1,             // Larva 1
    LARVA2,             // Larva 2

    COUNT,
    FIRST = 0
};

//===========================================================================
// Aircraft Types - All aircraft types in the game
//===========================================================================
enum class AircraftType : int8_t {
    NONE = -1,
    TRANSPORT = 0,  // Chinook Transport
    BADGER,         // Soviet Bomber
    U2,             // Allied Spy Plane
    MIG,            // MiG Fighter
    YAK,            // Yak Attack Plane
    HELI,           // Longbow Apache
    HIND,           // Hind Gunship

    COUNT,
    FIRST = 0
};

//===========================================================================
// Vessel Types - All naval unit types in the game
//===========================================================================
enum class VesselType : int8_t {
    NONE = -1,
    SS = 0,         // Submarine
    DD,             // Destroyer
    CA,             // Cruiser
    LST,            // Transport
    PT,             // Gunboat
    MSUB,           // Missile Submarine
    CARRIER,        // Aircraft Carrier (Aftermath)

    COUNT,
    FIRST = 0
};

//===========================================================================
// House Types - Playable factions
//===========================================================================
enum class HousesType : int8_t {
    NONE = -1,
    SPAIN = 0,      // Allied
    GREECE,         // Allied
    USSR,           // Soviet
    ENGLAND,        // Allied
    UKRAINE,        // Soviet
    GERMANY,        // Allied
    FRANCE,         // Allied
    TURKEY,         // Allied

    // Special houses
    GOOD,           // General Allied
    BAD,            // General Soviet
    NEUTRAL,        // Civilian/Neutral
    SPECIAL,        // Special
    MULTI1,         // Multiplayer 1
    MULTI2,         // Multiplayer 2
    MULTI3,         // Multiplayer 3
    MULTI4,         // Multiplayer 4
    MULTI5,         // Multiplayer 5
    MULTI6,         // Multiplayer 6
    MULTI7,         // Multiplayer 7
    MULTI8,         // Multiplayer 8

    COUNT,
    FIRST = 0
};

//===========================================================================
// Weapon Types - All weapon types in the game
//===========================================================================
enum class WeaponType : int8_t {
    NONE = -1,
    COLT45 = 0,     // Pistol
    ZSU23,          // Anti-aircraft gun
    VULCAN,         // Vulcan cannon
    MAVERICK,       // Maverick missile
    CAMERA,         // Spy plane camera
    FIREBALL,       // Fireball launcher
    SNIPER,         // Sniper rifle
    CHAINGUN,       // Chain gun
    PISTOL,         // Pistol
    M1CARBINE,      // M1 Carbine
    DRAGON,         // Dragon missile
    HELLFIRE,       // Hellfire missile
    GRENADE,        // Hand grenade
    M60MG,          // M60 machine gun
    TOMAHAWK,       // Tomahawk missile
    TOW,            // TOW missile
    MAMMOTH_TUSK,   // Mammoth tusk missile
    M_CANNON,       // 105mm cannon
    TURRET_CANNON,  // Turret cannon
    MG_CARRIER,     // Aircraft carrier MG
    DEPTH_CHARGE,   // Depth charge
    TORPEDO,        // Torpedo
    AA_CANNON,      // Anti-aircraft cannon
    FLAK,           // Flak gun
    TESLA_COIL,     // Tesla coil weapon
    NIKE,           // Nike missile
    SCUD,           // Scud missile
    V2_ROCKET,      // V2 rocket
    STINGER,        // Stinger missile
    AP_BULLETS,     // Anti-personnel bullets
    HEAL_MISSILE,   // Medic heal
    DOG_JAW,        // Attack dog bite
    ENGINEER,       // Engineer capture
    FIRE,           // Flamethrower
    // ... more weapons follow in original

    COUNT,
    FIRST = 0
};

//===========================================================================
// Warhead Types - Damage types for weapons
//===========================================================================
enum class WarheadType : int8_t {
    NONE = -1,
    SA = 0,         // Small arms (bullet)
    HE,             // High explosive
    AP,             // Armor piercing
    FIRE,           // Fire damage
    HOLLOW_POINT,   // Hollow point (anti-infantry)
    SUPER,          // Super weapon damage
    ORGANIC,        // Organic damage
    NUKE,           // Nuclear damage

    COUNT,
    FIRST = 0
};

//===========================================================================
// Armor Types - Armor categories
//===========================================================================
enum class ArmorType : int8_t {
    NONE = 0,
    WOOD,           // Wood/light structures
    LIGHT,          // Light vehicles
    HEAVY,          // Heavy vehicles
    CONCRETE,       // Concrete structures

    COUNT
};

//===========================================================================
// Speed Types - Movement speed categories
//===========================================================================
enum class SpeedType : int8_t {
    NONE = -1,
    FOOT = 0,       // Infantry
    TRACK,          // Tracked vehicles
    WHEEL,          // Wheeled vehicles
    WINGED,         // Aircraft
    FLOAT,          // Naval vessels

    COUNT,
    FIRST = 0
};

//===========================================================================
// Mission Types - Orders that units can execute
//===========================================================================
enum class MissionType : int8_t {
    NONE = -1,
    SLEEP = 0,      // Do nothing
    ATTACK,         // Attack target
    MOVE,           // Move to destination
    QMOVE,          // Queued move
    RETREAT,        // Retreat from combat
    GUARD,          // Guard area
    STICKY,         // Stay in position
    ENTER,          // Enter transport
    CAPTURE,        // Capture building
    HARVEST,        // Harvest ore
    GUARD_AREA,     // Guard specific area
    RETURN,         // Return to base
    STOP,           // Stop current action
    AMBUSH,         // Ambush mode
    HUNT,           // Hunt enemies
    UNLOAD,         // Unload passengers
    SABOTAGE,       // Sabotage building
    CONSTRUCTION,   // Build structure
    DECONSTRUCTION, // Sell/deconstruct structure
    SELLING,        // Sell structure
    REPAIR,         // Repair structure
    RESCUE,         // Rescue (medic)
    MISSILE,        // Launch missile
    HARMLESS,       // Non-combat mode

    COUNT,
    FIRST = 0
};

//===========================================================================
// Direction Types - 8 compass directions + variations
//===========================================================================
enum class DirType : uint8_t {
    N = 0,          // North
    NE = 32,        // Northeast
    E = 64,         // East
    SE = 96,        // Southeast
    S = 128,        // South
    SW = 160,       // Southwest
    W = 192,        // West
    NW = 224,       // Northwest

    MAX = 255
};

//===========================================================================
// Owner Flags - Bit flags for house ownership
//===========================================================================
namespace OwnerFlag {
    constexpr uint32_t SPAIN    = 1 << static_cast<int>(HousesType::SPAIN);
    constexpr uint32_t GREECE   = 1 << static_cast<int>(HousesType::GREECE);
    constexpr uint32_t USSR     = 1 << static_cast<int>(HousesType::USSR);
    constexpr uint32_t ENGLAND  = 1 << static_cast<int>(HousesType::ENGLAND);
    constexpr uint32_t UKRAINE  = 1 << static_cast<int>(HousesType::UKRAINE);
    constexpr uint32_t GERMANY  = 1 << static_cast<int>(HousesType::GERMANY);
    constexpr uint32_t FRANCE   = 1 << static_cast<int>(HousesType::FRANCE);
    constexpr uint32_t TURKEY   = 1 << static_cast<int>(HousesType::TURKEY);

    // Alliance groups
    constexpr uint32_t ALLIES = SPAIN | GREECE | ENGLAND | GERMANY | FRANCE | TURKEY;
    constexpr uint32_t SOVIET = USSR | UKRAINE;
    constexpr uint32_t ALL = ALLIES | SOVIET;
}

//===========================================================================
// Prerequisite Flags - Building prerequisites
//===========================================================================
namespace PrereqFlag {
    constexpr uint32_t NONE       = 0;
    constexpr uint32_t POWER      = 1 << 0;
    constexpr uint32_t BARRACKS   = 1 << 1;
    constexpr uint32_t RADAR      = 1 << 2;
    constexpr uint32_t FACTORY    = 1 << 3;
    constexpr uint32_t TECH       = 1 << 4;
    constexpr uint32_t HELIPAD    = 1 << 5;
    constexpr uint32_t AIRFIELD   = 1 << 6;
    constexpr uint32_t PROC       = 1 << 7;
    constexpr uint32_t ADVANCED   = 1 << 8;
}

//===========================================================================
// Fixed Point Math - Used throughout the engine
//===========================================================================
using LEPTON = int16_t;       // Fractional cell position (256 per cell)
using COORDINATE = uint32_t;  // Full coordinate (cell + lepton)
using CELL = uint16_t;        // Map cell index
using TARGET = uint32_t;      // Target reference

// Coordinate helpers
constexpr int CELL_SIZE = 24;           // Pixels per cell
constexpr int LEPTONS_PER_CELL = 256;   // Leptons per cell
constexpr int MAP_CELL_WIDTH = 128;     // Max map width in cells
constexpr int MAP_CELL_HEIGHT = 128;    // Max map height in cells

//===========================================================================
// Additional Types for Object Hierarchy
//===========================================================================

// Direction types - also used as DirType alias
using DirType = enum class DirType : uint8_t;  // Forward declared above

// Runtime Type Information - What kind of object is this?
enum class RTTIType : int8_t {
    NONE = -1,
    INFANTRY = 0,
    UNIT,           // Vehicles
    AIRCRAFT,
    BUILDING,
    BULLET,
    ANIMATION,
    TRIGGER,
    TEAM,
    TEAMTYPE,
    TEMPLATE,
    TERRAIN,
    OVERLAY,
    SMUDGE,
    VESSEL,         // Ships/subs
    SPECIAL,        // Special weapons

    COUNT
};

// Special Weapons (superweapons)
enum class SpecialWeaponType : int8_t {
    SPC_NONE = -1,
    SPC_SONAR_PULSE = 0,    // Reveal submarines
    SPC_NUCLEAR_BOMB,       // Nuclear strike
    SPC_CHRONOSPHERE,       // Teleport units
    SPC_PARABOMB,           // Paratroop drop (with bombs)
    SPC_PARAINFANTRY,       // Paratroop drop
    SPC_SPY_PLANE,          // Reconnaissance
    SPC_IRON_CURTAIN,       // Invulnerability
    SPC_GPS,                // Reveal map

    SPC_COUNT
};

// Radio message types - Inter-object communication
enum class RadioMessageType : int8_t {
    STATIC = 0,     // No message / interference
    ROGER,          // Acknowledged
    HELLO,          // Establish contact
    OVER_OUT,       // End contact
    NEGATIVE,       // Cannot comply

    // Transport messages
    SQUISH_ME,      // About to be run over
    IM_IN,          // Passenger loaded
    BACKUP,         // Backup up
    TETHER,         // Establish physical link
    UNTETHER,       // Release physical link

    // Construction messages
    BUILDING,       // Start construction
    COMPLETE,       // Construction complete
    CANT,           // Cannot comply

    COUNT
};

// Cloak states
enum class CloakType : int8_t {
    UNCLOAKED = 0,
    CLOAKING,
    CLOAKED,
    UNCLOAKING
};

// Combat result types
enum class ResultType : int8_t {
    NONE = 0,       // No effect
    LIGHT,          // Light damage
    HALF,           // Half damage
    HEAVY,          // Heavy damage
    DESTROYED       // Object destroyed
};

// Per-cell process callback types
enum class PCPType : int8_t {
    NONE = 0,
    CELL,           // Entering new cell
    SCATTER,        // About to scatter
    DESTINATION     // Reached destination
};

// Movement result types
enum class MoveType : int8_t {
    OK = 0,         // Can enter cell
    CLOAK,          // Cell is occupied by cloaked enemy
    MOVING_BLOCK,   // Moving object blocking
    CLOSE_ENOUGH,   // Close enough to destination
    NO,             // Cannot enter
    TEMP            // Temporarily blocked
};

// Mark types for display refresh
enum class MarkType : int8_t {
    UP = 0,         // Object removed from display
    DOWN,           // Object placed on display
    CHANGE,         // Object changed, needs refresh
    OVERLAP_UP,     // Overlap area removed
    OVERLAP_DOWN    // Overlap area placed
};

// Facing types - 8 directions (used for pathfinding, movement)
enum class FacingType : int8_t {
    NORTH = 0,
    NORTH_EAST,
    EAST,
    SOUTH_EAST,
    SOUTH,
    SOUTH_WEST,
    WEST,
    NORTH_WEST,

    COUNT,
    NONE = -1
};

// Action types - What can be done when clicking on something
enum class ActionType : int8_t {
    NONE = 0,
    MOVE,
    NO_MOVE,
    ENTER,
    SELF,
    ATTACK,
    HARVEST,
    SABOTAGE,
    CAPTURE,
    TOGGLE,         // Toggle selection
    SELECT,
    REPAIR,
    SELL,
    SELL_UNIT,
    NO_SELL,
    NO_REPAIR,
    GUARD_AREA,
    ION_CANNON,
    NUKE,
    AIR_STRIKE,
    CHRONOSPHERE,
    CHRONO_TARGET
};

// Mission state machine status values (MissionClass::status_)
enum class MissionStatus : int8_t {
    NONE = 0,
    APPROACHING,
    EXECUTING,
    LEAVING,
    DONE
};

//===========================================================================
// Terrain and Map Types
//===========================================================================

// Land type - Determines passability and movement cost
enum class LandType : int8_t {
    CLEAR = 0,      // Open terrain (default)
    ROAD,           // Road - faster movement
    WATER,          // Water - only boats
    ROCK,           // Impassable rock
    WALL,           // Wall structure
    TIBERIUM,       // Ore/gems (harvestable)
    BEACH,          // Transition water/land
    ROUGH,          // Rough terrain - slow
    RIVER,          // River (like water)

    COUNT
};

// Overlay type - Walls and resources on cells
enum class OverlayType : int8_t {
    NONE = -1,
    SANDBAG_WALL = 0,
    CYCLONE_WALL,
    BRICK_WALL,
    BARBWIRE_WALL,
    WOOD_WALL,
    GOLD1,          // Ore (4 stages)
    GOLD2,
    GOLD3,
    GOLD4,
    GEMS1,          // Gems (4 stages)
    GEMS2,
    GEMS3,
    GEMS4,
    V12,            // Civilian structures
    V13,
    V14,
    V15,
    V16,
    V17,
    V18,
    FLAG_SPOT,      // Flag location
    WOOD_CRATE,     // Crate
    STEEL_CRATE,
    FENCE,
    WATER_CRATE,

    COUNT
};

// Smudge type - Craters, scorches, building footprints
enum class SmudgeType : int8_t {
    NONE = -1,
    CRATER1 = 0,
    CRATER2,
    CRATER3,
    CRATER4,
    CRATER5,
    CRATER6,
    SCORCH1,
    SCORCH2,
    SCORCH3,
    SCORCH4,
    SCORCH5,
    SCORCH6,
    BIB1,           // Building foundation marks
    BIB2,
    BIB3,

    COUNT
};

// Template type - Base terrain graphics
enum class TemplateType : int16_t {
    NONE = -1,
    CLEAR1 = 0,
    WATER,
    WATER2,
    SHORE1,
    SHORE2,
    // ... many more terrain templates
    // For now, just key ones needed

    COUNT = 256     // Reserve space for all templates
};

// Movement zone type - Different movement categories for pathfinding
enum class MZoneType : int8_t {
    NORMAL = 0,     // Regular ground movement
    CRUSHER,        // Can crush infantry/fences
    DESTROYER,      // Can destroy walls
    WATER,          // Naval movement

    COUNT
};

//===========================================================================
// Cell and Coordinate Types (extends earlier CELL definition)
//===========================================================================

// Note: CELL is already defined as uint16_t earlier in this file

// Cell index constants
constexpr int CELL_X_BITS = 7;
constexpr int CELL_Y_BITS = 7;
constexpr int CELL_X_MASK = (1 << CELL_X_BITS) - 1;  // 0x7F
constexpr int CELL_Y_MASK = (1 << CELL_Y_BITS) - 1;  // 0x7F

// Map cell dimensions (128x128 grid)
constexpr int MAP_CELL_W = 128;
constexpr int MAP_CELL_H = 128;
constexpr int MAP_CELL_TOTAL = MAP_CELL_W * MAP_CELL_H;  // 16384

// Cell/coordinate conversion helpers
inline CELL XY_Cell(int x, int y) {
    return static_cast<CELL>((y << CELL_X_BITS) | (x & CELL_X_MASK));
}

inline int Cell_X(CELL cell) {
    return cell & CELL_X_MASK;
}

inline int Cell_Y(CELL cell) {
    return (cell >> CELL_X_BITS) & CELL_Y_MASK;
}

// Coordinate to cell (high 16 bits = X, low 16 bits = Y)
// Extract cell from each axis by dividing by 256 (leptons per cell)
inline CELL Coord_Cell(int32_t coord) {
    int x = (coord >> 16) / LEPTONS_PER_CELL;
    int y = (coord & 0xFFFF) / LEPTONS_PER_CELL;
    return XY_Cell(x, y);
}

// Cell to coordinate (center of cell)
inline int32_t Cell_Coord(CELL cell) {
    int x = Cell_X(cell) * LEPTONS_PER_CELL + LEPTONS_PER_CELL / 2;
    int y = Cell_Y(cell) * LEPTONS_PER_CELL + LEPTONS_PER_CELL / 2;
    return (x << 16) | y;
}

// Make coordinate from X, Y (in leptons)
inline int32_t XY_Coord(int x, int y) {
    return (x << 16) | (y & 0xFFFF);
}

// Extract X and Y from coordinate (in leptons)
inline int Coord_X(int32_t coord) {
    return coord >> 16;
}

inline int Coord_Y(int32_t coord) {
    return coord & 0xFFFF;
}

// Adjacent cell offsets (for 8 directions)
// Index by FacingType (NORTH=0, NORTH_EAST=1, etc.)
constexpr int AdjacentCell[8] = {
    -MAP_CELL_W,            // NORTH
    -MAP_CELL_W + 1,        // NORTH_EAST
    1,                      // EAST
    MAP_CELL_W + 1,         // SOUTH_EAST
    MAP_CELL_W,             // SOUTH
    MAP_CELL_W - 1,         // SOUTH_WEST
    -1,                     // WEST
    -MAP_CELL_W - 1         // NORTH_WEST
};

#endif // GAME_TYPES_H
