/**
 * Red Alert macOS Port - Weapon Type Definitions
 *
 * Static data tables for weapon, warhead, and bullet types.
 * Ported from original WEAPON.H and DEFINES.H
 */

#ifndef GAME_WEAPON_TYPES_H
#define GAME_WEAPON_TYPES_H

#include "types.h"
#include "unit_types.h"  // For AnimType

//===========================================================================
// Bullet Types - Projectile types
//===========================================================================
enum class BulletType : int8_t {
    NONE = -1,
    INVISIBLE = 0,  // Instant hit (small arms)
    CANNON,         // Tank cannon shell
    ACK,            // Anti-aircraft bullet
    TORPEDO,        // Naval torpedo
    FROG,           // FROG missile
    HEAT_SEEKER,    // Heat-seeking missile
    LASER_GUIDED,   // Laser-guided missile
    LOBBED,         // Lobbed projectile (grenade)
    BOMBLET,        // Bomb cluster
    BALLISTIC,      // Ballistic missile
    PARACHUTE,      // Parachute bomb
    FIREBALL,       // Fireball
    DOG,            // Dog attack (melee)
    CATAPULT,       // Catapult projectile
    AAMISSILE,      // Air-to-air missile
    GPS_SATELLITE,  // GPS satellite
    NUKE_UP,        // Nuclear missile (upward)
    NUKE_DOWN,      // Nuclear missile (downward)

    COUNT,
    FIRST = 0
};

//===========================================================================
// Complete Weapon Types - All weapons in game
//===========================================================================
enum class WeaponTypeEnum : int8_t {
    NONE = -1,
    COLT45 = 0,     // Pistol (Tanya)
    ACK_ACK,        // Anti-aircraft gun
    VULCAN,         // Vulcan cannon
    MAVERICK,       // Maverick missile
    CAMERA,         // Spy plane camera
    FIREBALL,       // Fireball launcher
    RIFLE,          // Rifle (infantry)
    CHAIN_GUN,      // Chain gun
    PISTOL,         // Pistol (spy, civilian)
    M16,            // M16 rifle
    DRAGON,         // Dragon missile
    HELLFIRE,       // Hellfire missile
    GRENADE,        // Hand grenade
    GUN_75MM,       // 75mm cannon
    GUN_90MM,       // 90mm cannon
    GUN_105MM,      // 105mm cannon
    GUN_120MM,      // 120mm cannon
    TURRET_GUN,     // Turret gun
    MAMMOTH_TUSK,   // Mammoth tank missiles
    GUN_155MM,      // 155mm cannon
    M60MG,          // M60 machine gun
    NAPALM,         // Napalm
    TESLA_ZAP,      // Tesla coil
    NIKE,           // Nike missile
    GUN_8INCH,      // 8-inch gun
    STINGER,        // Stinger missile
    TORPEDO,        // Torpedo
    GUN_2INCH,      // 2-inch gun
    DEPTH_CHARGE,   // Depth charge
    PARA_BOMB,      // Parachute bomb
    DOG_JAW,        // Dog attack
    HEAL,           // Medic heal
    SCUD,           // Scud missile
    FLAMER,         // Flamethrower
    REDEYE,         // Redeye missile

    // Ants
    MANDIBLE,       // Ant mandible

    // Aftermath
    PORTA_TESLA,    // Portable tesla (shock trooper)
    GOOD_WRENCH,    // Mechanic repair
    SUB_SCUD,       // Submarine scud
    TTANK_ZAP,      // Tesla tank zap
    AP_TUSK,        // Armor-piercing tusk
    DEMO_CHARGE,    // Demolition charge
    CARRIER,        // Aircraft carrier missiles

    COUNT,
    FIRST = 0
};

//===========================================================================
// Warhead Types - Damage types
//===========================================================================
enum class WarheadTypeEnum : int8_t {
    NONE = -1,
    SA = 0,         // Small arms - good against infantry
    HE,             // High explosive - good against buildings & infantry
    AP,             // Armor piercing - good against armor
    FIRE,           // Incendiary - good against flammables
    HOLLOW_POINT,   // Sniper bullet
    TESLA,          // Electrocution
    DOG,            // Dog attack
    NUKE,           // Nuclear
    MECHANICAL,     // Repair (mechanic)

    COUNT,
    FIRST = 0
};

//===========================================================================
// Warhead Type Data - Damage modifiers against armor types
//===========================================================================
struct WarheadTypeData {
    WarheadTypeEnum type;
    const char* iniName;
    int16_t spread;             // Damage spread radius (leptons)
    bool isWallDestroyer;       // Can destroy walls
    bool isWoodDestroyer;       // Can destroy wood
    bool isTiberiumDestroyer;   // Can destroy ore
    bool causesExplosion;       // Causes explosion animation

    // Damage modifiers vs armor types (percentage, 256 = 100%)
    int16_t vsNone;             // vs unarmored (infantry)
    int16_t vsWood;             // vs wood structures
    int16_t vsLight;            // vs light armor
    int16_t vsHeavy;            // vs heavy armor
    int16_t vsConcrete;         // vs concrete
};

//===========================================================================
// Bullet Type Data - Projectile properties
//===========================================================================
struct BulletTypeData {
    BulletType type;
    const char* iniName;
    bool isHigh;                // Flies high (immune to ground fire)
    bool isShadow;              // Casts shadow
    bool isArcing;              // Arcs to target
    bool isDropping;            // Drops from above
    bool isInvisible;           // Invisible projectile
    bool isProximityFused;      // Explodes near target
    bool isFlameEquipped;       // Leaves flame trail
    bool isFueled;              // Has fuel (limited range)
    bool isFacingless;          // No facing direction
    bool isInaccurate;          // Has spread
    bool isTranslucent;         // Semi-transparent
    bool isAntiAircraft;        // Can hit aircraft
    bool isAntiGround;          // Can hit ground units
    bool isAntiSubWarfare;      // Can hit submarines
    bool isDegenerate;          // Loses speed over time
    int8_t rotationStages;      // Number of rotation frames
    AnimType explosion;         // Explosion animation
};

//===========================================================================
// Weapon Type Data - Complete weapon definition
//===========================================================================
struct WeaponTypeData {
    WeaponTypeEnum type;
    const char* iniName;

    // Weapon properties
    BulletType bullet;          // Projectile type
    WarheadTypeEnum warhead;    // Warhead type
    int16_t damage;             // Base damage
    int16_t range;              // Range in leptons
    int16_t rateOfFire;         // Ticks between shots
    uint8_t burst;              // Number of rapid shots
    uint8_t speed;              // Projectile speed (255 = instant hit)

    // Flags
    bool isTurboBoosted;        // Speed boost vs aircraft
    bool isSuppressed;          // Avoid hitting friendlies
    bool isCamera;              // Reveals map
    bool isElectric;            // Needs charge-up (Tesla)

    // Visual/audio
    AnimType fireAnim;          // Muzzle flash animation
    int8_t soundId;             // Firing sound ID
};

//===========================================================================
// Data Tables (const defaults - do not modify directly)
//===========================================================================
extern const WarheadTypeData WarheadTypeDefaults[];
extern const int WarheadTypeCount;

extern const BulletTypeData BulletTypeDefaults[];
extern const int BulletTypeCount;

extern const WeaponTypeData WeaponTypeDefaults[];
extern const int WeaponTypeCount;

//===========================================================================
// Initialization Functions
//===========================================================================

/**
 * Initialize mutable weapon type data from defaults.
 * Call once at startup before loading RULES.INI.
 */
void InitWeaponTypes();

/**
 * Initialize mutable warhead type data from defaults.
 * Call once at startup before loading RULES.INI.
 */
void InitWarheadTypes();

/**
 * Initialize mutable bullet type data from defaults.
 * Call once at startup before loading RULES.INI.
 */
void InitBulletTypes();

//===========================================================================
// Getter Functions (mutable data for INI overrides)
//===========================================================================

WarheadTypeData* GetWarheadType(WarheadTypeEnum type);
BulletTypeData* GetBulletType(BulletType type);
WeaponTypeData* GetWeaponType(WeaponTypeEnum type);

/**
 * Get const data for read-only access
 */
const WarheadTypeData* GetWarheadTypeConst(WarheadTypeEnum type);
const BulletTypeData* GetBulletTypeConst(BulletType type);
const WeaponTypeData* GetWeaponTypeConst(WeaponTypeEnum type);

WeaponTypeEnum WeaponTypeFromName(const char* name);
WarheadTypeEnum WarheadTypeFromName(const char* name);
BulletType BulletTypeFromName(const char* name);

/**
 * Calculate damage vs a specific armor type
 */
int CalculateDamage(int baseDamage, WarheadTypeEnum warhead, ArmorType armor);

#endif // GAME_WEAPON_TYPES_H
