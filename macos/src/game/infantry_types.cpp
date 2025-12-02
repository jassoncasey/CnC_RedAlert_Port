/**
 * Red Alert macOS Port - Infantry Type Data Tables
 *
 * Static data tables for infantry types.
 * Ported from original IDATA.CPP
 */

#include "infantry_types.h"
#include <cstring>

//===========================================================================
// Animation Control Tables - Frame data for each animation state
// Format: { startFrame, frameCount, frameJump }
//===========================================================================

const DoInfoStruct DogDoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {0,   1,  1},   // STAND_GUARD
    {0,   1,  1},   // PRONE (N/A)
    {8,   6,  6},   // WALK
    {104, 14, 14},  // FIRE_WEAPON
    {0,   0,  0},   // LIE_DOWN (N/A)
    {56,  6,  6},   // CRAWL
    {0,   0,  0},   // GET_UP
    {104, 14, 14},  // FIRE_PRONE
    {216, 18, 0},   // IDLE1
    {216, 18, 0},   // IDLE2
    {235, 7,  0},   // GUN_DEATH
    {242, 9,  0},   // EXPLOSION_DEATH
    {242, 9,  0},   // EXPLOSION2_DEATH
    {242, 9,  0},   // GRENADE_DEATH
    {251, 14, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1
    {0,   1,  0},   // SALUTE1
    {0,   1,  0},   // GESTURE2
    {0,   1,  0},   // SALUTE2
    {106, 12, 14},  // DOG_MAUL
};

const DoInfoStruct E1DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {192, 1,  8},   // PRONE
    {16,  6,  6},   // WALK
    {64,  8,  8},   // FIRE_WEAPON
    {128, 2,  2},   // LIE_DOWN
    {144, 4,  4},   // CRAWL
    {176, 2,  2},   // GET_UP
    {192, 6,  8},   // FIRE_PRONE
    {256, 16, 0},   // IDLE1
    {272, 16, 0},   // IDLE2
    {288, 8,  0},   // GUN_DEATH
    {304, 8,  0},   // EXPLOSION_DEATH
    {304, 8,  0},   // EXPLOSION2_DEATH
    {312, 12, 0},   // GRENADE_DEATH
    {324, 18, 0},   // FIRE_DEATH
    {342, 3,  3},   // GESTURE1
    {366, 3,  3},   // SALUTE1
    {390, 3,  3},   // GESTURE2
    {414, 3,  3},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct E2DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {288, 1,  12},  // PRONE
    {16,  6,  6},   // WALK
    {64,  20, 20},  // FIRE_WEAPON
    {224, 2,  2},   // LIE_DOWN
    {240, 4,  4},   // CRAWL
    {272, 2,  2},   // GET_UP
    {288, 8,  12},  // FIRE_PRONE
    {384, 16, 0},   // IDLE1
    {400, 16, 0},   // IDLE2
    {416, 8,  0},   // GUN_DEATH
    {432, 8,  0},   // EXPLOSION_DEATH
    {432, 8,  0},   // EXPLOSION2_DEATH
    {440, 12, 0},   // GRENADE_DEATH
    {452, 18, 0},   // FIRE_DEATH
    {470, 3,  3},   // GESTURE1
    {494, 3,  3},   // SALUTE1
    {518, 3,  3},   // GESTURE2
    {542, 3,  3},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct E3DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {192, 1,  10},  // PRONE
    {16,  6,  6},   // WALK
    {64,  8,  8},   // FIRE_WEAPON
    {128, 2,  2},   // LIE_DOWN
    {144, 4,  4},   // CRAWL
    {176, 2,  2},   // GET_UP
    {192, 10, 10},  // FIRE_PRONE
    {272, 16, 0},   // IDLE1
    {288, 16, 0},   // IDLE2
    {304, 8,  0},   // GUN_DEATH
    {320, 8,  0},   // EXPLOSION_DEATH
    {320, 8,  0},   // EXPLOSION2_DEATH
    {328, 12, 0},   // GRENADE_DEATH
    {340, 18, 0},   // FIRE_DEATH
    {358, 3,  3},   // GESTURE1
    {382, 3,  3},   // SALUTE1
    {406, 3,  3},   // GESTURE2
    {430, 3,  3},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct E4DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {256, 1,  16},  // PRONE
    {16,  6,  6},   // WALK
    {64,  16, 16},  // FIRE_WEAPON
    {192, 2,  2},   // LIE_DOWN
    {208, 4,  4},   // CRAWL
    {240, 2,  2},   // GET_UP
    {256, 16, 16},  // FIRE_PRONE
    {384, 16, 0},   // IDLE1
    {400, 16, 0},   // IDLE2
    {416, 8,  0},   // GUN_DEATH
    {432, 8,  0},   // EXPLOSION_DEATH
    {432, 8,  0},   // EXPLOSION2_DEATH
    {440, 12, 0},   // GRENADE_DEATH
    {452, 18, 0},   // FIRE_DEATH
    {470, 3,  3},   // GESTURE1
    {494, 3,  3},   // SALUTE1
    {518, 3,  3},   // GESTURE2
    {542, 3,  3},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct E6DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {82,  1,  4},   // PRONE
    {16,  6,  6},   // WALK
    {0,   0,  0},   // FIRE_WEAPON (N/A - engineer)
    {67,  2,  2},   // LIE_DOWN
    {82,  4,  4},   // CRAWL
    {114, 2,  2},   // GET_UP
    {0,   0,  0},   // FIRE_PRONE (N/A)
    {130, 16, 0},   // IDLE1
    {130, 16, 0},   // IDLE2
    {146, 8,  0},   // GUN_DEATH
    {154, 8,  0},   // EXPLOSION_DEATH
    {162, 8,  0},   // EXPLOSION2_DEATH
    {162, 12, 0},   // GRENADE_DEATH
    {182, 18, 0},   // FIRE_DEATH
    {200, 3,  3},   // GESTURE1
    {224, 3,  3},   // SALUTE1
    {200, 3,  3},   // GESTURE2
    {224, 3,  3},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct E7DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {0,   1,  1},   // STAND_GUARD
    {128, 1,  4},   // PRONE
    {8,   6,  6},   // WALK
    {56,  7,  7},   // FIRE_WEAPON
    {113, 2,  2},   // LIE_DOWN
    {128, 4,  4},   // CRAWL
    {161, 2,  2},   // GET_UP
    {176, 7,  7},   // FIRE_PRONE
    {232, 17, 0},   // IDLE1
    {249, 13, 0},   // IDLE2
    {262, 8,  0},   // GUN_DEATH
    {270, 8,  0},   // EXPLOSION_DEATH
    {278, 8,  0},   // EXPLOSION2_DEATH
    {286, 12, 0},   // GRENADE_DEATH
    {298, 18, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1
    {0,   1,  0},   // SALUTE1
    {0,   1,  0},   // GESTURE2
    {0,   1,  0},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct SpyDoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {144, 1,  4},   // PRONE
    {16,  6,  6},   // WALK
    {64,  8,  8},   // FIRE_WEAPON
    {128, 2,  2},   // LIE_DOWN
    {144, 4,  4},   // CRAWL
    {176, 2,  2},   // GET_UP
    {192, 8,  8},   // FIRE_PRONE
    {256, 14, 0},   // IDLE1
    {270, 18, 0},   // IDLE2
    {288, 8,  0},   // GUN_DEATH
    {296, 8,  0},   // EXPLOSION_DEATH
    {304, 8,  0},   // EXPLOSION2_DEATH
    {312, 12, 0},   // GRENADE_DEATH
    {324, 18, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1
    {0,   1,  0},   // SALUTE1
    {0,   1,  0},   // GESTURE2
    {0,   1,  0},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct E9DoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {8,   1,  1},   // STAND_GUARD
    {72,  1,  4},   // PRONE
    {8,   6,  6},   // WALK
    {0,   0,  0},   // FIRE_WEAPON (N/A - thief)
    {56,  2,  2},   // LIE_DOWN
    {72,  4,  4},   // CRAWL
    {108, 2,  2},   // GET_UP
    {0,   0,  0},   // FIRE_PRONE (N/A)
    {120, 19, 0},   // IDLE1
    {120, 19, 0},   // IDLE2
    {139, 8,  0},   // GUN_DEATH
    {147, 8,  0},   // EXPLOSION_DEATH
    {155, 8,  0},   // EXPLOSION2_DEATH
    {163, 12, 0},   // GRENADE_DEATH
    {175, 18, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1
    {0,   1,  0},   // SALUTE1
    {0,   1,  0},   // GESTURE2
    {0,   1,  0},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct MedicDoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {0,   1,  1},   // STAND_GUARD
    {130, 1,  4},   // PRONE
    {8,   6,  6},   // WALK
    {56,  28, 0},   // FIRE_WEAPON (heal)
    {114, 2,  2},   // LIE_DOWN
    {130, 4,  4},   // CRAWL
    {162, 2,  2},   // GET_UP
    {56,  28, 0},   // FIRE_PRONE
    {178, 15, 0},   // IDLE1
    {178, 15, 0},   // IDLE2
    {193, 8,  0},   // GUN_DEATH
    {210, 8,  0},   // EXPLOSION_DEATH
    {202, 8,  0},   // EXPLOSION2_DEATH
    {217, 12, 0},   // GRENADE_DEATH
    {229, 18, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1
    {0,   1,  0},   // SALUTE1
    {0,   1,  0},   // GESTURE2
    {0,   1,  0},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct GeneralDoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {0,   1,  1},   // STAND_GUARD
    {104, 1,  4},   // PRONE
    {8,   6,  6},   // WALK
    {56,  4,  4},   // FIRE_WEAPON
    {88,  2,  2},   // LIE_DOWN
    {104, 4,  4},   // CRAWL
    {136, 2,  2},   // GET_UP
    {152, 4,  4},   // FIRE_PRONE
    {184, 26, 0},   // IDLE1
    {184, 26, 0},   // IDLE2
    {210, 8,  0},   // GUN_DEATH
    {226, 8,  0},   // EXPLOSION_DEATH
    {218, 8,  0},   // EXPLOSION2_DEATH
    {234, 12, 0},   // GRENADE_DEATH
    {246, 18, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1
    {0,   1,  0},   // SALUTE1
    {0,   1,  0},   // GESTURE2
    {0,   1,  0},   // SALUTE2
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct CivilianDoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {0,   1,  1},   // STAND_GUARD
    {0,   1,  1},   // PRONE (N/A)
    {56,  6,  6},   // WALK
    {120, 4,  4},   // FIRE_WEAPON
    {0,   1,  1},   // LIE_DOWN (N/A)
    {8,   6,  6},   // CRAWL
    {0,   1,  1},   // GET_UP (N/A)
    {120, 4,  4},   // FIRE_PRONE
    {104, 10, 0},   // IDLE1
    {114, 6,  0},   // IDLE2
    {152, 8,  0},   // GUN_DEATH
    {160, 8,  0},   // EXPLOSION_DEATH
    {160, 8,  0},   // EXPLOSION2_DEATH
    {168, 12, 0},   // GRENADE_DEATH
    {180, 18, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1 (N/A)
    {0,   1,  0},   // SALUTE1 (N/A)
    {0,   1,  0},   // GESTURE2 (N/A)
    {0,   1,  0},   // SALUTE2 (N/A)
    {0,   0,  0},   // DOG_MAUL (N/A)
};

const DoInfoStruct EinsteinDoControls[static_cast<int>(DoType::COUNT)] = {
    {0,   1,  1},   // STAND_READY
    {0,   1,  1},   // STAND_GUARD
    {0,   1,  1},   // PRONE (N/A)
    {56,  6,  6},   // WALK
    {113, 4,  4},   // FIRE_WEAPON
    {0,   1,  1},   // LIE_DOWN (N/A)
    {8,   6,  6},   // CRAWL
    {0,   1,  1},   // GET_UP (N/A)
    {0,   0,  0},   // FIRE_PRONE
    {104, 16, 0},   // IDLE1
    {104, 16, 0},   // IDLE2
    {120, 8,  0},   // GUN_DEATH
    {128, 8,  0},   // EXPLOSION_DEATH
    {136, 12, 0},   // EXPLOSION2_DEATH
    {136, 12, 0},   // GRENADE_DEATH
    {148, 17, 0},   // FIRE_DEATH
    {0,   1,  0},   // GESTURE1 (N/A)
    {0,   1,  0},   // SALUTE1 (N/A)
    {0,   1,  0},   // GESTURE2 (N/A)
    {0,   1,  0},   // SALUTE2 (N/A)
    {0,   0,  0},   // DOG_MAUL (N/A)
};

//===========================================================================
// Infantry Type Table - Static data for all infantry types
//===========================================================================

// Note: Combat stats (strength, cost, etc.) would normally be loaded
// from RULES.INI. These are placeholder values matching original game.

const InfantryTypeData InfantryTypeDefaults[] = {
    // E1 - Rifle Infantry
    {
        InfantryType::E1, 0, "E1",
        0x0035, 0x0010,                     // offsets
        false, true, false, false, false, false,  // flags
        PipType::FULL, 2, 2,                // pip, fire frames
        50, 100, 4, 4,                      // strength, cost, speed, sight
        ArmorType::NONE,                    // armor
        WeaponType::M1CARBINE, WeaponType::NONE,  // weapons
        1, OwnerFlag::ALL, 5, -1, 0,        // tech, owner, pts, ammo, guard
        false, false, false, false, false, false  // flags
    },
    // E2 - Grenadier
    {
        InfantryType::E2, 0, "E2",
        0x0035, 0x0010,
        false, true, false, false, false, false,
        PipType::FULL, 14, 6,
        50, 160, 5, 4,
        ArmorType::NONE,
        WeaponType::GRENADE, WeaponType::NONE,
        1, OwnerFlag::SOVIET, 10, -1, 0,
        false, false, false, false, true, false
    },
    // E3 - Rocket Soldier
    {
        InfantryType::E3, 0, "E3",
        0x0035, 0x0010,
        false, true, false, false, false, false,
        PipType::FULL, 3, 3,
        45, 300, 3, 4,
        ArmorType::NONE,
        WeaponType::DRAGON, WeaponType::NONE,
        2, OwnerFlag::ALLIES, 10, -1, 0,
        false, false, false, false, false, true
    },
    // E4 - Flamethrower
    {
        InfantryType::E4, 0, "E4",
        0x0035, 0x0010,
        false, true, false, false, false, false,
        PipType::FULL, 2, 0,
        40, 300, 3, 4,
        ArmorType::NONE,
        WeaponType::FIRE, WeaponType::NONE,
        6, OwnerFlag::SOVIET, 15, -1, 0,
        false, false, false, false, true, false
    },
    // E6 - Engineer
    {
        InfantryType::RENOVATOR, 0, "E6",
        0x0035, 0x0010,
        false, false, false, false, false, false,
        PipType::ENGINEER, 3, 3,
        25, 500, 4, 4,
        ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        5, OwnerFlag::ALL, 20, -1, 0,
        false, true, false, false, false, false
    },
    // E7 - Tanya
    {
        InfantryType::TANYA, 0, "E7",
        0x0035, 0x0010,
        true, true, false, false, false, false,
        PipType::COMMANDO, 2, 2,
        100, 1200, 5, 6,
        ArmorType::NONE,
        WeaponType::COLT45, WeaponType::COLT45,
        11, OwnerFlag::ALLIES, 25, -1, 0,
        false, true, true, false, false, true
    },
    // SPY
    {
        InfantryType::SPY, 0, "SPY",
        0x0035, 0x0010,
        false, false, false, false, false, false,
        PipType::ENGINEER, 3, 3,
        25, 500, 4, 5,
        ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        6, OwnerFlag::ALLIES, 15, -1, 0,
        false, true, false, false, false, false
    },
    // THF - Thief
    {
        InfantryType::THIEF, 0, "THF",
        0x0035, 0x0010,
        false, false, false, false, false, false,
        PipType::ENGINEER, 3, 3,
        25, 500, 4, 5,
        ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        11, OwnerFlag::ALLIES, 10, -1, 0,
        false, true, false, false, false, false
    },
    // MEDI - Medic
    {
        InfantryType::MEDIC, 0, "MEDI",
        0x0035, 0x0010,
        false, true, false, false, false, false,
        PipType::ENGINEER, 25, 25,
        80, 800, 4, 3,
        ArmorType::NONE,
        WeaponType::HEAL_MISSILE, WeaponType::NONE,
        2, OwnerFlag::ALLIES, 15, -1, 0,
        false, false, false, false, false, false
    },
    // GNRL - General
    {
        InfantryType::GENERAL, 0, "GNRL",
        0x0035, 0x0010,
        false, true, false, false, false, false,
        PipType::ENGINEER, 2, 2,
        80, 0, 5, 3,
        ArmorType::NONE,
        WeaponType::PISTOL, WeaponType::NONE,
        -1, OwnerFlag::ALL, 15, 10, 0,
        false, true, false, false, false, false
    },
    // DOG - Attack Dog
    {
        InfantryType::DOG, 0, "DOG",
        0x0015, 0x0010,
        false, false, false, false, false, false,
        PipType::FULL, 1, 1,
        12, 200, 4, 5,
        ArmorType::NONE,
        WeaponType::DOG_JAW, WeaponType::NONE,
        3, OwnerFlag::SOVIET, 5, -1, 7,
        false, false, false, true, false, false
    },
    // C1 - Civilian
    {
        InfantryType::C1, 0, "C1",
        0x0035, 0x0010,
        false, false, true, false, true, false,
        PipType::CIVILIAN, 2, 0,
        25, 10, 5, 2,
        ArmorType::NONE,
        WeaponType::PISTOL, WeaponType::NONE,
        -1, OwnerFlag::ALL, 1, 10, 0,
        true, false, false, false, false, false
    },
    // C2 - Civilian
    {
        InfantryType::C2, 0, "C2",
        0x0035, 0x0010,
        false, false, true, true, true, false,
        PipType::CIVILIAN, 2, 0,
        25, 10, 5, 2,
        ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        -1, OwnerFlag::ALL, 1, -1, 0,
        true, false, false, false, false, false
    },
    // C3 - Civilian (female)
    {
        InfantryType::C3, 0, "C3",
        0x0035, 0x0010,
        true, false, true, false, true, false,
        PipType::CIVILIAN, 2, 0,
        25, 10, 5, 2,
        ArmorType::NONE,
        WeaponType::NONE, WeaponType::NONE,
        -1, OwnerFlag::ALL, 1, -1, 0,
        true, false, false, false, false, false
    },
    // C4-C10, Einstein, Delphi, Chan would follow same pattern...
};

const int InfantryTypeCount =
    sizeof(InfantryTypeDefaults) / sizeof(InfantryTypeDefaults[0]);

//===========================================================================
// Mutable Infantry Type Data (runtime copy)
//===========================================================================
static InfantryTypeData g_infantryTypes[64];  // Max infantry types
static bool g_infantryTypesInitialized = false;

//===========================================================================
// Helper Functions
//===========================================================================

void InitInfantryTypes() {
    if (g_infantryTypesInitialized) return;

    // Copy const defaults to mutable storage
    for (int i = 0; i < InfantryTypeCount && i < 64; i++) {
        g_infantryTypes[i] = InfantryTypeDefaults[i];
    }
    g_infantryTypesInitialized = true;
}

InfantryTypeData* GetInfantryType(InfantryType type) {
    // Auto-init if not done
    if (!g_infantryTypesInitialized) {
        InitInfantryTypes();
    }

    for (int i = 0; i < InfantryTypeCount; i++) {
        if (g_infantryTypes[i].type == type) {
            return &g_infantryTypes[i];
        }
    }
    return nullptr;
}

const InfantryTypeData* GetInfantryTypeConst(InfantryType type) {
    return GetInfantryType(type);
}

InfantryType InfantryTypeFromName(const char* name) {
    if (name == nullptr) return InfantryType::NONE;

    // Auto-init if not done
    if (!g_infantryTypesInitialized) {
        InitInfantryTypes();
    }

    for (int i = 0; i < InfantryTypeCount; i++) {
        if (strcasecmp(g_infantryTypes[i].iniName, name) == 0) {
            return g_infantryTypes[i].type;
        }
    }
    return InfantryType::NONE;
}

const DoInfoStruct* GetInfantryDoControls(InfantryType type) {
    switch (type) {
        case InfantryType::DOG:       return DogDoControls;
        case InfantryType::E1:        return E1DoControls;
        case InfantryType::E2:        return E2DoControls;
        case InfantryType::E3:        return E3DoControls;
        case InfantryType::E4:        return E4DoControls;
        case InfantryType::RENOVATOR: return E6DoControls;
        case InfantryType::TANYA:     return E7DoControls;
        case InfantryType::SPY:       return SpyDoControls;
        case InfantryType::THIEF:     return E9DoControls;
        case InfantryType::MEDIC:     return MedicDoControls;
        case InfantryType::GENERAL:   return GeneralDoControls;
        case InfantryType::EINSTEIN:  return EinsteinDoControls;
        case InfantryType::DELPHI:    return CivilianDoControls;
        case InfantryType::CHAN:      return EinsteinDoControls;
        // Civilians use CivilianDoControls
        case InfantryType::C1:
        case InfantryType::C2:
        case InfantryType::C3:
        case InfantryType::C4:
        case InfantryType::C5:
        case InfantryType::C6:
        case InfantryType::C7:
        case InfantryType::C8:
        case InfantryType::C9:
        case InfantryType::C10:
            return CivilianDoControls;
        default:
            return E1DoControls;  // Default fallback
    }
}
