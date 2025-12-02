/**
 * Red Alert macOS Port - Building Type Definitions
 *
 * Static data tables for building/structure types.
 * Ported from original BDATA.CPP
 */

#ifndef GAME_BUILDING_TYPES_H
#define GAME_BUILDING_TYPES_H

#include "types.h"
#include "unit_types.h"  // For AnimType, RemapType

//===========================================================================
// Building Size - Dimensions in cells
//===========================================================================
enum class BSizeType : int8_t {
    BSIZE_11 = 0,   // 1x1 cells
    BSIZE_21,       // 2x1 cells
    BSIZE_12,       // 1x2 cells
    BSIZE_22,       // 2x2 cells
    BSIZE_23,       // 2x3 cells
    BSIZE_32,       // 3x2 cells
    BSIZE_33,       // 3x3 cells
    BSIZE_42,       // 4x2 cells
    BSIZE_55,       // 5x5 cells (unused)

    COUNT
};

// FacingType and RTTIType are defined in types.h

//===========================================================================
// Building Type Data - Static data for each building type
//===========================================================================
struct BuildingTypeData {
    BuildingType type;          // Building type enum
    int16_t nameId;             // Text ID for name
    const char* iniName;        // INI file identifier (4 chars)

    // Layout
    FacingType foundation;      // Direction from center
    int16_t exitX;              // Exit point X (leptons)
    int16_t exitY;              // Exit point Y (leptons)
    RemapType remap;            // Sidebar remap type
    BSizeType size;             // Building size

    // Weapon offsets (fixed point, pixels * 256)
    int16_t verticalOffset;     // Vertical render offset
    int16_t primaryOffset;      // Primary weapon offset along turret centerline
    int16_t primaryLateral;     // Primary weapon lateral offset

    // Boolean flags
    bool isFake;                // Is this a decoy building?
    bool isRegulated;           // Animation rate regulated for constant speed?
    bool isNominal;             // Always shows name?
    bool isWall;                // Wall type structure?
    bool isSimpleDamage;        // Simple (one frame) damage imagery?
    bool isInvisible;           // Invisible to radar?
    bool isSelectable;          // Can player select?
    bool isLegalTarget;         // Legal target for attack?
    bool isInsignificant;       // Not announced when destroyed?
    bool isTheater;             // Theater-specific graphics?
    bool hasTurret;             // Has rotating turret?
    bool canRemap;              // Can be color remapped?

    // Factory properties
    RTTIType factoryType;       // Type of objects this factory produces
    DirType startDirection;     // Starting idle frame direction

    // Combat stats (loaded from RULES.INI)
    int16_t strength;           // Hit points
    int16_t cost;               // Build cost
    int8_t sightRange;          // Sight range in cells
    int16_t power;              // Power +/- (positive=produce)
    ArmorType armor;            // Armor type
    WeaponType primaryWeapon;   // Primary weapon
    WeaponType secondaryWeapon; // Secondary weapon

    // Prerequisites
    uint32_t prereqs;           // Prerequisite building flags
    uint32_t owners;            // House ownership flags

    // Tech and points (loaded from RULES.INI)
    int8_t techLevel;           // Tech level required (-1 = can't build)
    int16_t points;             // Score points when destroyed

    // Additional flags (loaded from RULES.INI)
    bool isCapturable;          // Can be captured by engineers
    bool isCrewed;              // Has crew that can escape when destroyed
    bool hasBib;                // Has concrete bib foundation
};

//===========================================================================
// Building Type Table (const defaults - do not modify directly)
//===========================================================================
extern const BuildingTypeData BuildingTypeDefaults[];
extern const int BuildingTypeCount;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Initialize mutable building type data from defaults
 * Call once at startup before loading RULES.INI
 */
void InitBuildingTypes();

/**
 * Get mutable building type data by type enum
 */
BuildingTypeData* GetBuildingType(BuildingType type);

/**
 * Get const building type data by type enum (for read-only access)
 */
const BuildingTypeData* GetBuildingTypeConst(BuildingType type);

/**
 * Get building type by INI name
 */
BuildingType BuildingTypeFromName(const char* name);

/**
 * Get building size dimensions
 */
void GetBuildingSize(BSizeType size, int& width, int& height);

/**
 * Check if building is a wall type
 */
bool IsBuildingWall(BuildingType type);

/**
 * Check if building is a civilian structure
 */
bool IsBuildingCivilian(BuildingType type);

/**
 * Check if building produces units/buildings
 */
bool IsBuildingFactory(BuildingType type);

#endif // GAME_BUILDING_TYPES_H
