/**
 * Red Alert macOS Port - Unit Type Data Tables
 *
 * Static data tables for vehicle/unit types.
 * Ported from original UDATA.CPP
 */

#include "unit_types.h"
#include <cstring>

//===========================================================================
// Harvester Animation Lists
//===========================================================================

// Frames for dumping ore at refinery
const int HarvesterDumpList[22] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 6, 5, 4, 3, 2, 1, 0
};

// Frames for loading ore from ground
const int HarvesterLoadList[9] = {0, 1, 2, 3, 4, 5, 6, 7, 0};

const int HarvesterLoadCount = 8;

//===========================================================================
// Unit Type Table - Static data for all vehicle types
//===========================================================================

const UnitTypeData UnitTypeDefaults[] = {
    // V2 Rocket Launcher
    {
        UnitType::V2_LAUNCHER, 0, "V2RL",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  // offsets
        // crate, nominal, crusher, harvester, stealthy, insignificant
        true, false, true, false, false, false,
        // turret, radar, fire_anim, lock, gigundo, anim, jammer, gapper
        false, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        // Combat stats
        150, 700, 7, 5, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::V2_ROCKET, WeaponType::NONE, 0,
        // Tech/owner
        4, OwnerFlag::SOVIET, 40, 1, 5, 0, PrereqFlag::FACTORY,
        // Flags
        true, true, true, false, false, false, false
    },

    // Light Tank (1TNK)
    {
        UnitType::LTANK, 0, "1TNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0020, 0x00C0, 0x0000, 0x0000, 0x0000,
        true, false, true, false, false, false,
        true, false, false, false, false, false, false, false,
        32, 0, MissionType::HUNT,
        300, 700, 9, 4, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::TURRET_CANNON, WeaponType::NONE, 0,
        4, OwnerFlag::ALLIES, 30, -1, 5, 0, PrereqFlag::FACTORY,
        true, true, false, false, false, false, false
    },

    // Heavy Tank (3TNK)
    {
        UnitType::MTANK, 0, "3TNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0040, 0x0080, 0x0018, 0x0080, 0x0018,
        true, false, true, false, false, false,
        true, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        400, 950, 7, 5, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::TURRET_CANNON, WeaponType::TURRET_CANNON, 0,
        4, OwnerFlag::SOVIET, 50, -1, 5, 0, PrereqFlag::FACTORY,
        true, true, false, false, false, false, false
    },

    // Medium Tank (2TNK)
    {
        UnitType::MTANK2, 0, "2TNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0030, 0x00C0, 0x0000, 0x00C0, 0x0000,
        true, false, true, false, false, false,
        true, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        400, 800, 8, 5, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::TURRET_CANNON, WeaponType::NONE, 0,
        6, OwnerFlag::ALLIES, 40, -1, 5, 0, PrereqFlag::FACTORY,
        true, true, false, false, false, false, false
    },

    // Mammoth Tank (4TNK)
    {
        UnitType::HTANK, 0, "4TNK",
        AnimType::ART_EXP1, RemapType::NORMAL,
        0x0020, 0x00C0, 0x0028, 0x0008, 0x0040,
        true, false, true, false, false, false,
        true, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        600, 1700, 4, 6, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::MAMMOTH_TUSK, WeaponType::MAMMOTH_TUSK, 0,
        10, OwnerFlag::SOVIET, 60, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::TECH,
        true, true, false, true, false, false, false
    },

    // Mobile Radar Jammer
    {
        UnitType::MRJ, 0, "MRJ",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, true, false, true, false,
        false, true, false, false, false, false, true, false,
        32, 0, MissionType::HUNT,
        110, 600, 9, 7, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        12, OwnerFlag::ALLIES, 30, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::RADAR,
        true, true, false, false, false, false, false
    },

    // Mobile Gap Generator
    {
        UnitType::MGG, 0, "MGG",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, true, false, false, false,
        false, true, false, false, true, false, false, true,
        32, 0, MissionType::HUNT,
        110, 600, 9, 4, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        11, OwnerFlag::ALLIES, 40, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::ADVANCED,
        false, true, false, false, false, false, false
    },

    // Artillery
    {
        UnitType::ARTY, 0, "ARTY",
        AnimType::ART_EXP1, RemapType::NORMAL,
        0x0040, 0x0060, 0x0000, 0x0000, 0x0000,
        true, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        32, 0, MissionType::HUNT,
        75, 600, 6, 5, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::SCUD, WeaponType::NONE, 0,
        8, OwnerFlag::ALLIES, 35, -1, 2, 0, PrereqFlag::FACTORY,
        true, true, true, false, false, false, false
    },

    // Harvester
    {
        UnitType::HARVESTER, 0, "HARV",
        AnimType::FBALL1, RemapType::ALTERNATE,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        true, true, true, true, false, false,
        false, false, false, false, true, false, false, false,
        32, 0, MissionType::HARVEST,
        600, 1400, 6, 4, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        1, OwnerFlag::ALL, 55, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::PROC,
        true, true, false, true, false, false, false
    },

    // MCV
    {
        UnitType::MCV, 0, "MCV",
        AnimType::FBALL1, RemapType::ALTERNATE,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        true, false, true, false, false, false,
        false, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        600, 2500, 6, 4, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        11, OwnerFlag::ALL, 60, -1, 5, 0, PrereqFlag::FACTORY,
        false, true, false, false, false, false, false
    },

    // Ranger (Jeep)
    {
        UnitType::JEEP, 0, "JEEP",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0030, 0x0030, 0x0000, 0x0030, 0x0000,
        true, false, false, false, false, false,
        true, false, false, false, false, false, false, false,
        32, 0, MissionType::HUNT,
        150, 600, 10, 6, ArmorType::LIGHT, SpeedType::WHEEL,
        WeaponType::M60MG, WeaponType::NONE, 0,
        3, OwnerFlag::ALLIES, 20, -1, 10, 0, PrereqFlag::FACTORY,
        false, true, false, false, false, false, false
    },

    // APC
    {
        UnitType::APC, 0, "APC",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0030, 0x0030, 0x0000, 0x0030, 0x0000,
        true, false, true, false, false, false,
        false, false, false, false, false, false, false, false,
        32, 0, MissionType::HUNT,
        200, 800, 10, 5, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::M60MG, WeaponType::NONE, 5,
        5, OwnerFlag::ALLIES, 25, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::BARRACKS,
        true, false, false, false, false, false, false
    },

    // Mine Layer
    {
        UnitType::MINELAYER, 0, "MNLY",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        true, false, true, false, false, false,
        false, false, false, false, false, false, false, false,
        32, 0, MissionType::HUNT,
        100, 800, 9, 5, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        3, OwnerFlag::ALL, 50, 5, 5, 0, PrereqFlag::FACTORY,
        true, true, false, false, false, false, false
    },

    // Convoy Truck
    {
        UnitType::TRUCK, 0, "TRUK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        32, 0, MissionType::GUARD,
        110, 500, 10, 3, ArmorType::LIGHT, SpeedType::WHEEL,
        WeaponType::NONE, WeaponType::NONE, 1,
        -1, OwnerFlag::ALL, 5, -1, 5, 0, PrereqFlag::NONE,
        false, false, false, false, false, false, false
    },

    // Ant units (special)
    {
        UnitType::ANT1, 0, "ANT1",
        AnimType::ANT_DEATH, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, true, false, false, false, true,
        false, false, false, false, true, false, false, false,
        8, 0, MissionType::HUNT,
        150, 700, 5, 2, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        -1, 0, 0, -1, 0, 0, PrereqFlag::NONE,
        false, false, false, false, false, false, false
    },
    {
        UnitType::ANT2, 0, "ANT2",
        AnimType::ANT_DEATH, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, true, false, false, false, true,
        false, false, false, false, true, false, false, false,
        8, 0, MissionType::HUNT,
        150, 700, 5, 2, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        -1, 0, 0, -1, 0, 0, PrereqFlag::NONE,
        false, false, false, false, false, false, false
    },
    {
        UnitType::ANT3, 0, "ANT3",
        AnimType::ANT_DEATH, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, true, false, false, false, true,
        false, false, false, false, true, false, false, false,
        8, 0, MissionType::HUNT,
        150, 700, 5, 2, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        -1, 0, 0, -1, 0, 0, PrereqFlag::NONE,
        false, false, false, false, false, false, false
    },

    // Aftermath units
    // Chrono Tank
    {
        UnitType::CHRONOTANK, 0, "CTNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, true, false, false, false,
        false, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        200, 2400, 8, 5, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::TURRET_CANNON, WeaponType::NONE, 0,
        -1, OwnerFlag::ALLIES, 0, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::ADVANCED,
        true, true, false, false, false, false, false
    },

    // Tesla Tank
    {
        UnitType::TESLATANK, 0, "TTNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, true, false, true, false,
        false, true, false, false, true, false, true, false,
        32, 0, MissionType::HUNT,
        200, 1500, 7, 5, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::TESLA_COIL, WeaponType::NONE, 0,
        -1, OwnerFlag::SOVIET, 0, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::TECH,
        true, true, false, false, false, false, false
    },

    // M.A.D. Tank
    {
        UnitType::MAD, 0, "QTNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, true, false, false, false,
        false, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        200, 2200, 6, 4, ArmorType::HEAVY, SpeedType::TRACK,
        WeaponType::NONE, WeaponType::NONE, 0,
        -1, OwnerFlag::SOVIET, 0, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::TECH,
        true, true, false, false, false, false, false
    },

    // Demolition Truck
    {
        UnitType::DEMOTRUCK, 0, "DTRK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        false, false, false, false, false, false,
        false, false, false, false, false, false, false, false,
        32, 0, MissionType::GUARD,
        110, 1500, 12, 2, ArmorType::LIGHT, SpeedType::WHEEL,
        WeaponType::NONE, WeaponType::NONE, 0,
        -1, OwnerFlag::SOVIET, 0, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::TECH,
        false, true, false, false, false, false, true
    },

    // Phase Transport
    {
        UnitType::PHASE, 0, "STNK",
        AnimType::FRAG1, RemapType::NORMAL,
        0x0030, 0x0030, 0x0000, 0x0030, 0x0000,
        false, false, true, false, false, false,
        true, false, false, false, true, false, false, false,
        32, 0, MissionType::HUNT,
        200, 2500, 10, 6, ArmorType::LIGHT, SpeedType::TRACK,
        WeaponType::DRAGON, WeaponType::NONE, 5,
        -1, OwnerFlag::ALLIES, 0, -1, 5, 0, PrereqFlag::FACTORY | PrereqFlag::ADVANCED,
        true, false, false, false, true, false, false
    },
};

const int UnitTypeCount = sizeof(UnitTypeDefaults) / sizeof(UnitTypeDefaults[0]);

//===========================================================================
// Mutable Unit Type Data (runtime copy)
//===========================================================================
static UnitTypeData g_unitTypes[64];  // Max unit types
static bool g_unitTypesInitialized = false;

//===========================================================================
// Helper Functions
//===========================================================================

void InitUnitTypes() {
    if (g_unitTypesInitialized) return;

    // Copy const defaults to mutable storage
    for (int i = 0; i < UnitTypeCount && i < 64; i++) {
        g_unitTypes[i] = UnitTypeDefaults[i];
    }
    g_unitTypesInitialized = true;
}

UnitTypeData* GetUnitType(UnitType type) {
    // Auto-init if not done
    if (!g_unitTypesInitialized) {
        InitUnitTypes();
    }

    for (int i = 0; i < UnitTypeCount; i++) {
        if (g_unitTypes[i].type == type) {
            return &g_unitTypes[i];
        }
    }
    return nullptr;
}

const UnitTypeData* GetUnitTypeConst(UnitType type) {
    return GetUnitType(type);
}

UnitType UnitTypeFromName(const char* name) {
    if (name == nullptr) return UnitType::NONE;

    // Auto-init if not done
    if (!g_unitTypesInitialized) {
        InitUnitTypes();
    }

    for (int i = 0; i < UnitTypeCount; i++) {
        if (strcasecmp(g_unitTypes[i].iniName, name) == 0) {
            return g_unitTypes[i].type;
        }
    }
    return UnitType::NONE;
}
