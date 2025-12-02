/**
 * Red Alert macOS Port - Weapon Type Data Tables
 *
 * Static data tables for weapon, warhead, and bullet types.
 * Ported from original source code.
 */

#include "weapon_types.h"
#include <cstring>

//===========================================================================
// Warhead Type Table
// Damage modifiers: 256 = 100%, 128 = 50%, 384 = 150%
//===========================================================================
const WarheadTypeData WarheadTypeDefaults[] = {
    // SA - Small Arms (good vs infantry)
    {
        WarheadTypeEnum::SA, "SA",
        0,              // spread
        false, false, false, false,  // wall, wood, ore, explosion
        256, 128, 128, 64, 32  // vs None, Wood, Light, Heavy, Concrete
    },
    // HE - High Explosive (good vs buildings and infantry)
    {
        WarheadTypeEnum::HE, "HE",
        48,             // spread (small area)
        true, true, false, true,
        256, 256, 192, 128, 192
    },
    // AP - Armor Piercing (good vs armor)
    {
        WarheadTypeEnum::AP, "AP",
        0,
        false, false, false, true,
        64, 128, 256, 256, 128
    },
    // Fire - Incendiary (good vs flammables)
    {
        WarheadTypeEnum::FIRE, "Fire",
        24,
        false, true, true, true,
        256, 384, 192, 64, 64
    },
    // HollowPoint - Sniper (extremely good vs infantry)
    {
        WarheadTypeEnum::HOLLOW_POINT, "HollowPoint",
        0,
        false, false, false, false,
        384, 64, 32, 16, 16
    },
    // Tesla - Electrocution (very good vs infantry)
    {
        WarheadTypeEnum::TESLA, "Tesla",
        24,
        true, false, false, true,
        384, 192, 128, 128, 128
    },
    // Dog - Attack dog (lethal to infantry only)
    {
        WarheadTypeEnum::DOG, "Dog",
        0,
        false, false, false, false,
        512, 0, 0, 0, 0
    },
    // Nuke - Nuclear (good vs everything)
    {
        WarheadTypeEnum::NUKE, "Nuke",
        256,            // large spread
        true, true, true, true,
        384, 384, 256, 192, 256
    },
    // Mechanical - Repair weapon (heals vehicles)
    {
        WarheadTypeEnum::MECHANICAL, "Mechanical",
        0,
        false, false, false, false,
        0, 0, 256, 256, 0  // Only affects vehicles
    },
};

const int WarheadTypeCount =
    sizeof(WarheadTypeDefaults) / sizeof(WarheadTypeDefaults[0]);

//===========================================================================
// Bullet Type Table
//===========================================================================
const BulletTypeData BulletTypeDefaults[] = {
    // Invisible - instant hit
    {
        BulletType::INVISIBLE, "Invisible",
        false, false, false, false, true, false, false, false,
        true, false, false, true, true, false, false,
        0, AnimType::NONE
    },
    // Cannon - tank shell
    {
        BulletType::CANNON, "Cannon",
        false, true, false, false, false, false, false, false,
        false, false, false, true, true, false, false,
        32, AnimType::VEH_HIT1
    },
    // ACK - AA bullet
    {
        BulletType::ACK, "AAGun",
        true, false, false, false, false, true, false, false,
        true, true, false, true, false, false, false,
        0, AnimType::PIFF
    },
    // Torpedo
    {
        BulletType::TORPEDO, "Torpedo",
        false, false, false, false, false, false, false, true,
        false, false, false, false, true, true, false,
        16, AnimType::VEH_HIT2
    },
    // FROG missile
    {
        BulletType::FROG, "FROG",
        true, true, false, false, false, false, false, true,
        false, true, false, true, true, false, false,
        32, AnimType::FRAG1
    },
    // Heat seeker missile
    {
        BulletType::HEAT_SEEKER, "Missile",
        true, true, false, false, false, false, false, true,
        false, false, false, true, true, false, false,
        32, AnimType::FRAG1
    },
    // Laser guided missile
    {
        BulletType::LASER_GUIDED, "LaserGuided",
        true, true, false, false, false, false, false, true,
        false, false, false, true, true, false, false,
        32, AnimType::FRAG1
    },
    // Lobbed (grenade)
    {
        BulletType::LOBBED, "Grenade",
        false, true, true, false, false, false, false, false,
        false, true, false, false, true, false, true,
        0, AnimType::VEH_HIT2
    },
    // Bomblet
    {
        BulletType::BOMBLET, "Bomblet",
        true, true, false, true, false, false, false, false,
        false, false, false, false, true, false, false,
        0, AnimType::FRAG1
    },
    // Ballistic (V2, SCUD)
    {
        BulletType::BALLISTIC, "Ballistic",
        true, true, false, true, false, false, false, true,
        false, true, false, false, true, false, true,
        32, AnimType::FBALL1
    },
    // Parachute bomb
    {
        BulletType::PARACHUTE, "Parachute",
        true, true, false, true, false, false, false, false,
        false, false, false, false, true, false, false,
        0, AnimType::FBALL1
    },
    // Fireball
    {
        BulletType::FIREBALL, "Fireball",
        false, true, false, false, false, false, true, true,
        false, true, true, false, true, false, false,
        8, AnimType::NAPALM1
    },
    // Dog attack
    {
        BulletType::DOG, "Dog",
        false, false, false, false, true, false, false, false,
        true, false, false, false, true, false, false,
        0, AnimType::NONE
    },
    // Catapult
    {
        BulletType::CATAPULT, "Catapult",
        false, true, true, false, false, false, false, false,
        false, true, false, false, true, false, true,
        0, AnimType::FBALL1
    },
    // AA missile
    {
        BulletType::AAMISSILE, "AAMissile",
        true, true, false, false, false, true, false, true,
        false, false, false, true, false, false, false,
        32, AnimType::FRAG1
    },
};

const int BulletTypeCount =
    sizeof(BulletTypeDefaults) / sizeof(BulletTypeDefaults[0]);

//===========================================================================
// Weapon Type Table
//===========================================================================
const WeaponTypeData WeaponTypeDefaults[] = {
    // Colt45 - Tanya's pistol
    {
        WeaponTypeEnum::COLT45, "Colt45",
        BulletType::INVISIBLE, WarheadTypeEnum::HOLLOW_POINT,
        25, 4 * 256, 40, 2, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 0
    },
    // ACK_ACK - Anti-aircraft gun
    {
        WeaponTypeEnum::ACK_ACK, "AAGun",
        BulletType::ACK, WarheadTypeEnum::AP,
        25, 8 * 256, 20, 2, 40,
        true, false, false, false,
        AnimType::MUZZLE_FLASH, 1
    },
    // Vulcan - Vulcan cannon
    {
        WeaponTypeEnum::VULCAN, "Vulcan",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        15, 6 * 256, 15, 1, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 2
    },
    // Maverick - Maverick missile
    {
        WeaponTypeEnum::MAVERICK, "Maverick",
        BulletType::HEAT_SEEKER, WarheadTypeEnum::AP,
        60, 8 * 256, 60, 1, 24,
        true, false, false, false,
        AnimType::NONE, 3
    },
    // Camera - Spy plane camera
    {
        WeaponTypeEnum::CAMERA, "Camera",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        0, 0, 0, 0, 0,
        false, false, true, false,
        AnimType::NONE, -1
    },
    // Fireball
    {
        WeaponTypeEnum::FIREBALL, "Fireball",
        BulletType::FIREBALL, WarheadTypeEnum::FIRE,
        50, 4 * 256, 65, 1, 16,
        false, false, false, false,
        AnimType::NONE, 4
    },
    // Rifle - Infantry rifle
    {
        WeaponTypeEnum::RIFLE, "M1Carbine",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        15, 5 * 256, 50, 1, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 5
    },
    // Chain gun
    {
        WeaponTypeEnum::CHAIN_GUN, "ChainGun",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        25, 5 * 256, 30, 1, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 6
    },
    // Pistol - Spy/civilian pistol
    {
        WeaponTypeEnum::PISTOL, "Pistol",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        8, 4 * 256, 50, 1, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 0
    },
    // M16
    {
        WeaponTypeEnum::M16, "M16",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        20, 5 * 256, 30, 1, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 5
    },
    // Dragon missile (rocket soldier)
    {
        WeaponTypeEnum::DRAGON, "Dragon",
        BulletType::HEAT_SEEKER, WarheadTypeEnum::AP,
        30, 5 * 256, 80, 1, 16,
        false, false, false, false,
        AnimType::NONE, 7
    },
    // Hellfire missile (Apache)
    {
        WeaponTypeEnum::HELLFIRE, "Hellfire",
        BulletType::HEAT_SEEKER, WarheadTypeEnum::AP,
        45, 7 * 256, 60, 1, 24,
        false, false, false, false,
        AnimType::NONE, 8
    },
    // Grenade
    {
        WeaponTypeEnum::GRENADE, "Grenade",
        BulletType::LOBBED, WarheadTypeEnum::HE,
        35, 4 * 256, 60, 1, 12,
        false, true, false, false,
        AnimType::NONE, 9
    },
    // 75mm cannon
    {
        WeaponTypeEnum::GUN_75MM, "75mm",
        BulletType::CANNON, WarheadTypeEnum::AP,
        25, 5 * 256, 50, 1, 24,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 10
    },
    // 90mm cannon
    {
        WeaponTypeEnum::GUN_90MM, "90mm",
        BulletType::CANNON, WarheadTypeEnum::AP,
        30, 5 * 256, 50, 1, 24,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 10
    },
    // 105mm cannon
    {
        WeaponTypeEnum::GUN_105MM, "105mm",
        BulletType::CANNON, WarheadTypeEnum::AP,
        40, 6 * 256, 50, 1, 24,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 11
    },
    // 120mm cannon
    {
        WeaponTypeEnum::GUN_120MM, "120mm",
        BulletType::CANNON, WarheadTypeEnum::AP,
        50, 6 * 256, 50, 1, 24,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 11
    },
    // Turret gun
    {
        WeaponTypeEnum::TURRET_GUN, "TurretGun",
        BulletType::CANNON, WarheadTypeEnum::AP,
        40, 5 * 256, 35, 1, 24,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 12
    },
    // Mammoth tusk missiles
    {
        WeaponTypeEnum::MAMMOTH_TUSK, "MammothTusk",
        BulletType::HEAT_SEEKER, WarheadTypeEnum::AP,
        75, 6 * 256, 80, 2, 20,
        true, false, false, false,
        AnimType::NONE, 13
    },
    // 155mm (artillery)
    {
        WeaponTypeEnum::GUN_155MM, "155mm",
        BulletType::BALLISTIC, WarheadTypeEnum::HE,
        150, 12 * 256, 100, 1, 16,
        false, true, false, false,
        AnimType::MUZZLE_FLASH, 14
    },
    // M60 machine gun
    {
        WeaponTypeEnum::M60MG, "M60mg",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        15, 5 * 256, 20, 1, 255,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 15
    },
    // Napalm
    {
        WeaponTypeEnum::NAPALM, "Napalm",
        BulletType::FIREBALL, WarheadTypeEnum::FIRE,
        100, 6 * 256, 70, 1, 16,
        false, false, false, false,
        AnimType::NONE, 16
    },
    // Tesla zap
    {
        WeaponTypeEnum::TESLA_ZAP, "TeslaZap",
        BulletType::INVISIBLE, WarheadTypeEnum::TESLA,
        150, 6 * 256, 90, 1, 255,
        false, true, false, true,
        AnimType::NONE, 17
    },
    // Nike missile
    {
        WeaponTypeEnum::NIKE, "Nike",
        BulletType::HEAT_SEEKER, WarheadTypeEnum::HE,
        100, 10 * 256, 50, 2, 24,
        true, false, false, false,
        AnimType::NONE, 18
    },
    // 8-inch gun (cruiser)
    {
        WeaponTypeEnum::GUN_8INCH, "8Inch",
        BulletType::BALLISTIC, WarheadTypeEnum::HE,
        200, 16 * 256, 80, 2, 16,
        false, true, false, false,
        AnimType::MUZZLE_FLASH, 19
    },
    // Stinger missile
    {
        WeaponTypeEnum::STINGER, "Stinger",
        BulletType::AAMISSILE, WarheadTypeEnum::HE,
        40, 8 * 256, 50, 2, 24,
        true, false, false, false,
        AnimType::NONE, 20
    },
    // Torpedo
    {
        WeaponTypeEnum::TORPEDO, "Torpedo",
        BulletType::TORPEDO, WarheadTypeEnum::AP,
        50, 8 * 256, 100, 2, 12,
        false, false, false, false,
        AnimType::NONE, 21
    },
    // 2-inch gun
    {
        WeaponTypeEnum::GUN_2INCH, "2Inch",
        BulletType::CANNON, WarheadTypeEnum::SA,
        15, 5 * 256, 30, 1, 24,
        false, false, false, false,
        AnimType::MUZZLE_FLASH, 22
    },
    // Depth charge
    {
        WeaponTypeEnum::DEPTH_CHARGE, "DepthCharge",
        BulletType::TORPEDO, WarheadTypeEnum::AP,
        40, 5 * 256, 50, 1, 8,
        false, false, false, false,
        AnimType::NONE, 23
    },
    // Parachute bomb
    {
        WeaponTypeEnum::PARA_BOMB, "ParaBomb",
        BulletType::PARACHUTE, WarheadTypeEnum::HE,
        100, 0, 120, 1, 8,
        false, false, false, false,
        AnimType::NONE, 24
    },
    // Dog jaw
    {
        WeaponTypeEnum::DOG_JAW, "DogJaw",
        BulletType::DOG, WarheadTypeEnum::DOG,
        20, 1 * 256 + 128, 15, 1, 255,
        false, false, false, false,
        AnimType::NONE, 25
    },
    // Heal (medic)
    {
        WeaponTypeEnum::HEAL, "Heal",
        BulletType::INVISIBLE, WarheadTypeEnum::SA,
        -50, 2 * 256, 50, 1, 255,  // Negative damage = healing
        false, false, false, false,
        AnimType::NONE, 26
    },
    // Scud missile (V2)
    {
        WeaponTypeEnum::SCUD, "Scud",
        BulletType::BALLISTIC, WarheadTypeEnum::HE,
        200, 10 * 256, 100, 1, 20,
        false, true, false, false,
        AnimType::NONE, 27
    },
    // Flamethrower
    {
        WeaponTypeEnum::FLAMER, "Flamer",
        BulletType::FIREBALL, WarheadTypeEnum::FIRE,
        35, 3 * 256, 50, 1, 8,
        false, false, false, false,
        AnimType::NONE, 28
    },
    // Redeye missile
    {
        WeaponTypeEnum::REDEYE, "RedEye",
        BulletType::AAMISSILE, WarheadTypeEnum::AP,
        30, 6 * 256, 60, 1, 20,
        true, false, false, false,
        AnimType::NONE, 29
    },
    // Ant mandible
    {
        WeaponTypeEnum::MANDIBLE, "Mandible",
        BulletType::DOG, WarheadTypeEnum::SA,
        50, 2 * 256, 50, 1, 255,
        false, false, false, false,
        AnimType::NONE, 30
    },
    // Portable Tesla (shock trooper)
    {
        WeaponTypeEnum::PORTA_TESLA, "PortaTesla",
        BulletType::INVISIBLE, WarheadTypeEnum::TESLA,
        100, 4 * 256, 80, 1, 255,
        false, false, false, true,
        AnimType::NONE, 31
    },
    // Mechanic repair
    {
        WeaponTypeEnum::GOOD_WRENCH, "Mechanic",
        BulletType::INVISIBLE, WarheadTypeEnum::MECHANICAL,
        -50, 2 * 256, 30, 1, 255,  // Negative = repair
        false, false, false, false,
        AnimType::NONE, 32
    },
};

const int WeaponTypeCount =
    sizeof(WeaponTypeDefaults) / sizeof(WeaponTypeDefaults[0]);

//===========================================================================
// Mutable Type Data (runtime copies for INI overrides)
//===========================================================================
static WarheadTypeData g_warheadTypes[16];
static bool g_warheadTypesInitialized = false;

static BulletTypeData g_bulletTypes[32];
static bool g_bulletTypesInitialized = false;

static WeaponTypeData g_weaponTypes[64];
static bool g_weaponTypesInitialized = false;

//===========================================================================
// Initialization Functions
//===========================================================================

void InitWarheadTypes() {
    if (g_warheadTypesInitialized) return;
    for (int i = 0; i < WarheadTypeCount && i < 16; i++) {
        g_warheadTypes[i] = WarheadTypeDefaults[i];
    }
    g_warheadTypesInitialized = true;
}

void InitBulletTypes() {
    if (g_bulletTypesInitialized) return;
    for (int i = 0; i < BulletTypeCount && i < 32; i++) {
        g_bulletTypes[i] = BulletTypeDefaults[i];
    }
    g_bulletTypesInitialized = true;
}

void InitWeaponTypes() {
    if (g_weaponTypesInitialized) return;
    for (int i = 0; i < WeaponTypeCount && i < 64; i++) {
        g_weaponTypes[i] = WeaponTypeDefaults[i];
    }
    g_weaponTypesInitialized = true;
}

//===========================================================================
// Getter Functions (mutable data)
//===========================================================================

WarheadTypeData* GetWarheadType(WarheadTypeEnum type) {
    if (!g_warheadTypesInitialized) InitWarheadTypes();
    for (int i = 0; i < WarheadTypeCount && i < 16; i++) {
        if (g_warheadTypes[i].type == type) {
            return &g_warheadTypes[i];
        }
    }
    return nullptr;
}

BulletTypeData* GetBulletType(BulletType type) {
    if (!g_bulletTypesInitialized) InitBulletTypes();
    for (int i = 0; i < BulletTypeCount && i < 32; i++) {
        if (g_bulletTypes[i].type == type) {
            return &g_bulletTypes[i];
        }
    }
    return nullptr;
}

WeaponTypeData* GetWeaponType(WeaponTypeEnum type) {
    if (!g_weaponTypesInitialized) InitWeaponTypes();
    for (int i = 0; i < WeaponTypeCount && i < 64; i++) {
        if (g_weaponTypes[i].type == type) {
            return &g_weaponTypes[i];
        }
    }
    return nullptr;
}

//===========================================================================
// Const Getter Functions (read-only access)
//===========================================================================

const WarheadTypeData* GetWarheadTypeConst(WarheadTypeEnum type) {
    return GetWarheadType(type);
}

const BulletTypeData* GetBulletTypeConst(BulletType type) {
    return GetBulletType(type);
}

const WeaponTypeData* GetWeaponTypeConst(WeaponTypeEnum type) {
    return GetWeaponType(type);
}

//===========================================================================
// Name Lookup Functions
//===========================================================================

WeaponTypeEnum WeaponTypeFromName(const char* name) {
    if (name == nullptr) return WeaponTypeEnum::NONE;
    if (!g_weaponTypesInitialized) InitWeaponTypes();
    for (int i = 0; i < WeaponTypeCount && i < 64; i++) {
        if (strcasecmp(g_weaponTypes[i].iniName, name) == 0) {
            return g_weaponTypes[i].type;
        }
    }
    return WeaponTypeEnum::NONE;
}

WarheadTypeEnum WarheadTypeFromName(const char* name) {
    if (name == nullptr) return WarheadTypeEnum::NONE;
    if (!g_warheadTypesInitialized) InitWarheadTypes();
    for (int i = 0; i < WarheadTypeCount && i < 16; i++) {
        if (strcasecmp(g_warheadTypes[i].iniName, name) == 0) {
            return g_warheadTypes[i].type;
        }
    }
    return WarheadTypeEnum::NONE;
}

BulletType BulletTypeFromName(const char* name) {
    if (name == nullptr) return BulletType::NONE;
    if (!g_bulletTypesInitialized) InitBulletTypes();
    for (int i = 0; i < BulletTypeCount && i < 32; i++) {
        if (strcasecmp(g_bulletTypes[i].iniName, name) == 0) {
            return g_bulletTypes[i].type;
        }
    }
    return BulletType::NONE;
}

//===========================================================================
// Damage Calculation
//===========================================================================

int CalculateDamage(int baseDamage, WarheadTypeEnum warhead, ArmorType armor) {
    const WarheadTypeData* wh = GetWarheadTypeConst(warhead);
    if (wh == nullptr) return baseDamage;

    int modifier = 256;  // 100% default
    switch (armor) {
        case ArmorType::NONE:     modifier = wh->vsNone; break;
        case ArmorType::WOOD:     modifier = wh->vsWood; break;
        case ArmorType::LIGHT:    modifier = wh->vsLight; break;
        case ArmorType::HEAVY:    modifier = wh->vsHeavy; break;
        case ArmorType::CONCRETE: modifier = wh->vsConcrete; break;
        default: break;
    }

    return (baseDamage * modifier) / 256;
}
