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

#endif // GAME_TYPES_H
