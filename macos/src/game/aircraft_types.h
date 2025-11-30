/**
 * Red Alert macOS Port - Aircraft Type Definitions
 *
 * Static data tables for aircraft types.
 * Ported from original ADATA.CPP
 */

#ifndef GAME_AIRCRAFT_TYPES_H
#define GAME_AIRCRAFT_TYPES_H

#include "types.h"
#include "unit_types.h"  // For AnimType, RemapType

//===========================================================================
// Landing Type - How aircraft lands
//===========================================================================
enum class LandingType : int8_t {
    NONE = 0,       // Doesn't land (spy plane)
    HELIPAD,        // Lands on helipad
    AIRSTRIP        // Lands on airstrip
};

//===========================================================================
// Aircraft Type Data - Static data for each aircraft type
//===========================================================================
struct AircraftTypeData {
    AircraftType type;          // Aircraft type enum
    int16_t nameId;             // Text ID for name
    const char* iniName;        // INI file identifier

    // Visual properties
    AnimType explosion;         // Death explosion animation
    RemapType remap;            // Sidebar remap type
    int8_t rotationStages;      // Number of rotation frames (typically 32)

    // Flight properties
    LandingType landingType;    // How it lands
    bool isFixedWing;           // Fixed wing (plane) vs rotary (helicopter)
    bool isSelectable;          // Can player select?
    bool canHover;              // Can hover in place
    bool hasRotor;              // Has visible rotor
    bool isLandable;            // Can land

    // Weapon offsets
    int16_t verticalOffset;     // Vertical render offset
    int16_t primaryOffset;      // Primary weapon offset

    // Combat stats (loaded from RULES.INI)
    int16_t strength;           // Hit points
    int16_t cost;               // Build cost
    int8_t speed;               // Flight speed
    int8_t sightRange;          // Sight range in cells
    ArmorType armor;            // Armor type
    WeaponType primaryWeapon;   // Primary weapon
    WeaponType secondaryWeapon; // Secondary weapon
    int8_t passengers;          // Max passengers (transport only)
    int8_t ammo;                // Ammunition capacity
};

//===========================================================================
// Aircraft Type Table
//===========================================================================
extern const AircraftTypeData AircraftTypes[];
extern const int AircraftTypeCount;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Get aircraft type data by type enum
 */
const AircraftTypeData* GetAircraftType(AircraftType type);

/**
 * Get aircraft type by INI name
 */
AircraftType AircraftTypeFromName(const char* name);

#endif // GAME_AIRCRAFT_TYPES_H
