/**
 * Red Alert macOS Port - Unit Type Definitions
 *
 * Static data tables for vehicle/unit types.
 * Ported from original UDATA.CPP
 */

#ifndef GAME_UNIT_TYPES_H
#define GAME_UNIT_TYPES_H

#include "types.h"

//===========================================================================
// Remap Types - How unit graphics are colored
//===========================================================================
enum class RemapType : int8_t {
    NORMAL = 0,     // Standard house remap
    ALTERNATE,      // Alternate color scheme (harvesters, MCVs)
    NONE            // No remapping
};

//===========================================================================
// Animation Types - Explosion animations
//===========================================================================
enum class AnimType : int8_t {
    NONE = -1,
    FBALL1 = 0,     // Large fireball
    FRAG1,          // Fragment explosion
    VEH_HIT1,       // Vehicle hit 1
    VEH_HIT2,       // Vehicle hit 2
    VEH_HIT3,       // Vehicle hit 3
    ART_EXP1,       // Artillery explosion
    NAPALM1,        // Napalm explosion
    NAPALM2,        // Napalm 2
    NAPALM3,        // Napalm 3
    SMOKE_M,        // Medium smoke
    PIFF,           // Small impact
    PIFFPIFF,       // Double impact
    FIRE_SMALL,     // Small fire
    FIRE_MED,       // Medium fire
    FIRE_MED2,      // Medium fire 2
    FIRE_TINY,      // Tiny fire
    MUZZLE_FLASH,   // Gun muzzle flash
    SMOKE_PUFF,     // Smoke puff
    PIFF_EXP,       // Impact explosion
    ANT_DEATH,      // Ant death

    COUNT
};

//===========================================================================
// Unit Type Data - Static data for each vehicle type
//===========================================================================
struct UnitTypeData {
    UnitType type;              // Unit type enum
    int16_t nameId;             // Text ID for name
    const char* iniName;        // INI file identifier
    AnimType explosion;         // Death explosion animation
    RemapType remap;            // Remap type for sidebar

    // Weapon offsets (fixed point, pixels * 256)
    int16_t verticalOffset;     // Vertical render offset
    int16_t primaryOffset;      // Primary weapon offset along turret centerline
    int16_t primaryLateral;     // Primary weapon lateral offset
    int16_t secondaryOffset;    // Secondary weapon offset
    int16_t secondaryLateral;   // Secondary weapon lateral offset

    // Boolean flags
    bool isCrateGoodie;         // Can appear in crates
    bool isNominal;             // Always shows name
    bool isCrusher;             // Can crush infantry
    bool isHarvester;           // Harvests ore
    bool isStealthy;            // Invisible to radar
    bool isInsignificant;       // Not announced
    bool hasTurret;             // Has rotating turret
    bool hasRadarDish;          // Has rotating radar dish
    bool hasFireAnim;           // Has firing animation
    bool isLockTurret;          // Turret locked while moving
    bool isGigundo;             // Large unit (multiple cells)
    bool isAnimating;           // Has constant animation
    bool isJammer;              // Jams radar
    bool isGapper;              // Mobile gap generator

    // Other properties
    int8_t rotationStages;      // Body rotation stages (typically 32)
    int8_t turretOffset;        // Turret center offset along body centerline
    MissionType defaultMission; // Default order for new units

    // Combat stats (loaded from RULES.INI)
    int16_t strength;           // Hit points
    int16_t cost;               // Build cost
    int8_t speed;               // Max speed
    int8_t sightRange;          // Sight range in cells
    ArmorType armor;            // Armor type
    SpeedType speedType;        // Movement type (wheel/track)
    WeaponType primaryWeapon;   // Primary weapon
    WeaponType secondaryWeapon; // Secondary weapon
    int8_t passengers;          // Max passengers (if transport)

    // Tech and ownership (loaded from RULES.INI)
    int8_t techLevel;           // Tech level required (-1 = can't build)
    uint32_t owners;            // House ownership flags (OwnerFlag)
    int16_t points;             // Score points when destroyed
    int8_t ammo;                // Ammo count (-1 = unlimited)
    int8_t rot;                 // Rate of Turn
    int8_t guardRange;          // Guard area scan range
    uint32_t prereqs;           // Prerequisite building flags

    // Additional flags (loaded from RULES.INI)
    bool isTracked;             // Is tracked vehicle (vs wheeled)
    bool isCrewed;              // Has crew that can escape
    bool noMovingFire;          // Must stop to fire
    bool selfHealing;           // Heals over time
    bool isCloakable;           // Has cloaking device
    bool hasSensors;            // Can detect cloaked units
    bool explodes;              // Explodes when destroyed
};

//===========================================================================
// Harvester Animation Tables
//===========================================================================
extern const int HarvesterDumpList[];
extern const int HarvesterLoadList[];
extern const int HarvesterLoadCount;

//===========================================================================
// Unit Type Table (const defaults - do not modify directly)
//===========================================================================
extern const UnitTypeData UnitTypeDefaults[];
extern const int UnitTypeCount;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Initialize mutable unit type data from defaults
 * Call once at startup before loading RULES.INI
 */
void InitUnitTypes();

/**
 * Get mutable unit type data by type enum
 */
UnitTypeData* GetUnitType(UnitType type);

/**
 * Get const unit type data by type enum (for read-only access)
 */
const UnitTypeData* GetUnitTypeConst(UnitType type);

/**
 * Get unit type by INI name
 */
UnitType UnitTypeFromName(const char* name);

#endif // GAME_UNIT_TYPES_H
