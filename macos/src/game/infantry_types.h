/**
 * Red Alert macOS Port - Infantry Type Definitions
 *
 * Static data tables for infantry types.
 * Ported from original IDATA.CPP
 */

#ifndef GAME_INFANTRY_TYPES_H
#define GAME_INFANTRY_TYPES_H

#include "types.h"

//===========================================================================
// Infantry Animation Control - Do states for infantry
//===========================================================================
enum class DoType : int8_t {
    STAND_READY = 0,    // Standing ready
    STAND_GUARD,        // Standing guard
    PRONE,              // Prone position
    WALK,               // Walking
    FIRE_WEAPON,        // Firing weapon
    LIE_DOWN,           // Lying down
    CRAWL,              // Crawling
    GET_UP,             // Getting up
    FIRE_PRONE,         // Firing while prone
    IDLE1,              // Idle animation 1
    IDLE2,              // Idle animation 2
    GUN_DEATH,          // Death by gun
    EXPLOSION_DEATH,    // Death by explosion
    EXPLOSION2_DEATH,   // Alternate explosion death
    GRENADE_DEATH,      // Death by grenade
    FIRE_DEATH,         // Death by fire
    GESTURE1,           // Gesture animation 1
    SALUTE1,            // Salute animation 1
    GESTURE2,           // Gesture animation 2
    SALUTE2,            // Salute animation 2
    DOG_MAUL,           // Dog mauling

    COUNT
};

//===========================================================================
// Do Info Structure - Animation frame control for each do state
//===========================================================================
struct DoInfoStruct {
    int16_t frame;      // Starting frame
    int8_t count;       // Number of frames
    int8_t jump;        // Frames to jump between facings
};

//===========================================================================
// Pip Types - Transport pip indicators
//===========================================================================
enum class PipType : int8_t {
    EMPTY = 0,
    FULL,
    ENGINEER,
    CIVILIAN,
    COMMANDO
};

//===========================================================================
// Infantry Type Class - Static data for each infantry type
//===========================================================================
struct InfantryTypeData {
    InfantryType type;          // Infantry type enum
    int16_t nameId;             // Text ID for name
    const char* iniName;        // INI file identifier
    int16_t verticalOffset;     // Vertical render offset
    int16_t primaryOffset;      // Primary weapon offset
    bool isFemale;              // Is female
    bool hasCrawling;           // Has crawling animation
    bool isCivilian;            // Is civilian
    bool isRemapOverride;       // Uses override remap
    bool isNominal;             // Always shows name
    bool isTheater;             // Theater-specific graphics
    PipType pip;                // Transport pip type
    int8_t fireLaunch;          // Frame of projectile launch
    int8_t proneLaunch;         // Frame of prone projectile launch

    // Combat stats (loaded from RULES.INI)
    int16_t strength;           // Hit points
    int16_t cost;               // Build cost
    int8_t speed;               // Movement speed
    int8_t sightRange;          // Sight range in cells
    ArmorType armor;            // Armor type
    WeaponType primaryWeapon;   // Primary weapon
    WeaponType secondaryWeapon; // Secondary weapon

    // Flags
    bool isFraidyCat;           // Runs when hurt
    bool canCapture;            // Can capture buildings
    bool isBomber;              // Has C4 charges
    bool isDog;                 // Is attack dog
};

//===========================================================================
// Infantry Type Table - All infantry types
//===========================================================================
extern const InfantryTypeData InfantryTypes[];
extern const int InfantryTypeCount;

//===========================================================================
// Animation Control Tables - Per-infantry-type animation data
//===========================================================================
extern const DoInfoStruct DogDoControls[];
extern const DoInfoStruct E1DoControls[];
extern const DoInfoStruct E2DoControls[];
extern const DoInfoStruct E3DoControls[];
extern const DoInfoStruct E4DoControls[];
extern const DoInfoStruct E6DoControls[];      // Engineer
extern const DoInfoStruct E7DoControls[];      // Tanya
extern const DoInfoStruct E9DoControls[];      // Thief
extern const DoInfoStruct SpyDoControls[];
extern const DoInfoStruct MedicDoControls[];
extern const DoInfoStruct GeneralDoControls[];
extern const DoInfoStruct CivilianDoControls[];
extern const DoInfoStruct EinsteinDoControls[];

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Get infantry type data by type enum
 */
const InfantryTypeData* GetInfantryType(InfantryType type);

/**
 * Get infantry type by INI name
 */
InfantryType InfantryTypeFromName(const char* name);

/**
 * Get animation control data for an infantry type
 */
const DoInfoStruct* GetInfantryDoControls(InfantryType type);

#endif // GAME_INFANTRY_TYPES_H
