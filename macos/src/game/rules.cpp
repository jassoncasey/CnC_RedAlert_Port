/**
 * Red Alert macOS Port - Rules Processing Implementation
 *
 * Loads game rules from RULES.INI and applies them to type data tables.
 */

#include "rules.h"
#include "infantry_types.h"
#include "unit_types.h"
#include "building_types.h"
#include "weapon_types.h"
#include <cstring>
#include <cstdio>

// INI getter macros (to stay within 80 columns)
#define GET_INT(s, k, v)   ini_.GetInt(s, k, v)
#define GET_BOOL(s, k, v)  ini_.GetBool(s, k, v)
#define GET_FIXED(s, k, v) ini_.GetFixed(s, k, v)

//===========================================================================
// Global Rules Instance
//===========================================================================
RulesClass Rules;

//===========================================================================
// Construction
//===========================================================================
RulesClass::RulesClass() : countryCount_(0) {
    SetDefaults();
}

void RulesClass::SetDefaults() {
    // General defaults
    general_.crateMinimum = 1;
    general_.crateMaximum = 255;
    general_.crateRadius = 3.0f;
    general_.crateRegen = 3.0f;
    general_.waterCrateChance = 0.20f;
    general_.soloCrateMoney = 2000;

    general_.chronoDuration = 3.0f;
    general_.chronoKillCargo = true;
    general_.chronoTechLevel = 12;
    general_.gpsTechLevel = 8;
    general_.gapRadius = 10;
    general_.gapRegenInterval = 0.1f;
    general_.ironCurtainDuration = 0.75f;
    general_.paraTech = 5;
    general_.parabombTech = 8;
    general_.radarJamRadius = 15;
    general_.spyPlaneTech = 5;
    general_.badgerBombCount = 1;

    general_.quakeChance = 0.20f;
    general_.quakeDamage = 0.33f;
    general_.vortexChance = 0.20f;
    general_.vortexDamage = 200;
    general_.vortexRange = 10;
    general_.vortexSpeed = 10;

    general_.refundPercent = 0.50f;
    general_.reloadRate = 0.04f;
    general_.repairPercent = 0.20f;
    general_.repairRate = 0.016f;
    general_.repairStep = 7;
    general_.uRepairPercent = 0.20f;
    general_.uRepairStep = 10;

    general_.turboBoost = 1.5f;
    general_.apMineDamage = 1000;
    general_.avMineDamage = 1200;
    general_.atomDamage = 1000;
    general_.ballisticScatter = 1.0f;
    general_.bridgeStrength = 1000;
    general_.c4Delay = 0.03f;
    general_.crushDistance = 1.5f;
    general_.expSpread = 0.3f;
    general_.fireSuppress = 1.0f;
    general_.homingScatter = 2.0f;
    general_.maxDamage = 1000;
    general_.minDamage = 1;
    general_.oreExplosive = false;
    general_.playerAutoCrush = false;
    general_.playerReturnFire = false;
    general_.playerScatter = false;
    general_.proneDamage = 0.50f;
    general_.treeTargeting = false;
    general_.incomingSpeed = 10;

    general_.bailCount = 28;
    general_.buildSpeed = 0.8f;
    general_.buildupTime = 0.06f;
    general_.gemValue = 50;
    general_.goldValue = 25;
    general_.growthRate = 2.0f;
    general_.oreGrows = true;
    general_.oreSpreads = true;
    general_.oreTruckRate = 1.0f;
    general_.separateAircraft = false;
    general_.survivorRate = 0.4f;

    general_.allyReveal = true;
    general_.conditionRed = 0.25f;
    general_.conditionYellow = 0.50f;
    general_.dropZoneRadius = 4;
    general_.enemyHealth = true;
    general_.gravity = 3;
    general_.idleActionFrequency = 0.1f;
    general_.messageDelay = 0.6f;
    general_.movieTime = 0.06f;
    general_.namedCivilians = false;
    general_.savourDelay = 0.03f;
    general_.shroudRate = 4.0f;
    general_.speakDelay = 2.0f;
    general_.timerWarning = 2.0f;
    general_.flashLowPower = true;

    general_.curleyShuffle = false;
    general_.baseBias = 2.0f;
    general_.baseDefenseDelay = 0.25f;
    general_.closeEnough = 2.75f;
    general_.damageDelay = 1.0f;
    general_.gameSpeedBias = 1.0f;
    general_.lzScanRadius = 16;
    general_.mineAware = true;
    general_.stray = 2.0f;
    general_.submergeDelay = 0.02f;
    general_.suspendDelay = 2.0f;
    general_.suspendPriority = 20;
    general_.teamDelay = 0.6f;

    general_.fineDiffControl = false;
    general_.mcvUndeploy = false;

    // IQ defaults
    iq_.maxLevels = 5;
    iq_.superWeapons = 4;
    iq_.production = 5;
    iq_.guardArea = 4;
    iq_.repairSell = 1;
    iq_.autoCrush = 2;
    iq_.scatter = 3;
    iq_.contentScan = 4;
    iq_.aircraft = 4;
    iq_.harvester = 2;
    iq_.sellBack = 2;

    // Difficulty defaults
    diffEasy_ = {
        1.2f, 1.2f, 1.2f, 0.8f, 1.2f, 0.8f, 0.8f, 0.001f, 0.001f, true, false
    };
    diffNormal_ = {
        1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.02f, 0.03f, true, false
    };
    diffHard_ = {
        0.9f, 0.9f, 0.9f, 1.2f, 0.8f, 1.2f, 1.1f, 0.02f, 0.03f, true, true
    };
}

//===========================================================================
// Loading
//===========================================================================
bool RulesClass::Load(const char* filename) {
    if (!ini_.Load(filename)) {
        return false;
    }
    return Process();
}

bool RulesClass::LoadFromBuffer(const char* data, size_t size) {
    if (!ini_.LoadFromBuffer(data, size)) {
        return false;
    }
    return Process();
}

bool RulesClass::Process() {
    ProcessGeneral();
    ProcessIQ();
    ProcessDifficulty();
    ProcessCountries();
    ProcessInfantry();
    ProcessUnits();
    ProcessBuildings();
    ProcessWeapons();
    ProcessWarheads();
    ProcessProjectiles();
    return true;
}

//===========================================================================
// Process [General] section
//===========================================================================
void RulesClass::ProcessGeneral() {
    const char* s = "General";
    GeneralRules& g = general_;

    // Crates
    g.crateMinimum = GET_INT(s, "CrateMinimum", g.crateMinimum);
    g.crateMaximum = GET_INT(s, "CrateMaximum", g.crateMaximum);
    g.crateRadius = GET_FIXED(s, "CrateRadius", g.crateRadius);
    g.crateRegen = GET_FIXED(s, "CrateRegen", g.crateRegen);
    g.waterCrateChance = GET_FIXED(s, "WaterCrateChance", g.waterCrateChance);
    g.soloCrateMoney = GET_INT(s, "SoloCrateMoney", g.soloCrateMoney);

    // Special weapons
    g.chronoDuration = GET_FIXED(s, "ChronoDuration", g.chronoDuration);
    g.chronoKillCargo = GET_BOOL(s, "ChronoKillCargo", g.chronoKillCargo);
    g.chronoTechLevel = GET_INT(s, "ChronoTechLevel", g.chronoTechLevel);
    g.gpsTechLevel = GET_INT(s, "GPSTechLevel", g.gpsTechLevel);
    g.gapRadius = GET_INT(s, "GapRadius", g.gapRadius);
    g.gapRegenInterval = GET_FIXED(s, "GapRegenInterval", g.gapRegenInterval);
    g.ironCurtainDuration = GET_FIXED(s, "IronCurtain", g.ironCurtainDuration);
    g.paraTech = GET_INT(s, "ParaTech", g.paraTech);
    g.parabombTech = GET_INT(s, "ParabombTech", g.parabombTech);
    g.radarJamRadius = GET_INT(s, "RadarJamRadius", g.radarJamRadius);
    g.spyPlaneTech = GET_INT(s, "SpyPlaneTech", g.spyPlaneTech);
    g.badgerBombCount = GET_INT(s, "BadgerBombCount", g.badgerBombCount);

    // Chrono side effects
    g.quakeChance = GET_FIXED(s, "QuakeChance", g.quakeChance);
    g.quakeDamage = GET_FIXED(s, "QuakeDamage", g.quakeDamage);
    g.vortexChance = GET_FIXED(s, "VortexChance", g.vortexChance);
    g.vortexDamage = GET_INT(s, "VortexDamage", g.vortexDamage);
    g.vortexRange = GET_INT(s, "VortexRange", g.vortexRange);
    g.vortexSpeed = GET_INT(s, "VortexSpeed", g.vortexSpeed);

    // Repair and refit
    g.refundPercent = GET_FIXED(s, "RefundPercent", g.refundPercent);
    g.reloadRate = GET_FIXED(s, "ReloadRate", g.reloadRate);
    g.repairPercent = GET_FIXED(s, "RepairPercent", g.repairPercent);
    g.repairRate = GET_FIXED(s, "RepairRate", g.repairRate);
    g.repairStep = GET_INT(s, "RepairStep", g.repairStep);
    g.uRepairPercent = GET_FIXED(s, "URepairPercent", g.uRepairPercent);
    g.uRepairStep = GET_INT(s, "URepairStep", g.uRepairStep);

    // Combat and damage
    g.turboBoost = GET_FIXED(s, "TurboBoost", g.turboBoost);
    g.apMineDamage = GET_INT(s, "APMineDamage", g.apMineDamage);
    g.avMineDamage = GET_INT(s, "AVMineDamage", g.avMineDamage);
    g.atomDamage = GET_INT(s, "AtomDamage", g.atomDamage);
    g.ballisticScatter = GET_FIXED(s, "BallisticScatter", g.ballisticScatter);
    g.bridgeStrength = GET_INT(s, "BridgeStrength", g.bridgeStrength);
    g.c4Delay = GET_FIXED(s, "C4Delay", g.c4Delay);
    g.crushDistance = GET_FIXED(s, "Crush", g.crushDistance);
    g.expSpread = GET_FIXED(s, "ExpSpread", g.expSpread);
    g.fireSuppress = GET_FIXED(s, "FireSupress", g.fireSuppress);
    g.homingScatter = GET_FIXED(s, "HomingScatter", g.homingScatter);
    g.maxDamage = GET_INT(s, "MaxDamage", g.maxDamage);
    g.minDamage = GET_INT(s, "MinDamage", g.minDamage);
    g.oreExplosive = GET_BOOL(s, "OreExplosive", g.oreExplosive);
    g.playerAutoCrush = GET_BOOL(s, "PlayerAutoCrush", g.playerAutoCrush);
    g.playerReturnFire = GET_BOOL(s, "PlayerReturnFire", g.playerReturnFire);
    g.playerScatter = GET_BOOL(s, "PlayerScatter", g.playerScatter);
    g.proneDamage = GET_FIXED(s, "ProneDamage", g.proneDamage);
    g.treeTargeting = GET_BOOL(s, "TreeTargeting", g.treeTargeting);
    g.incomingSpeed = GET_INT(s, "Incoming", g.incomingSpeed);

    // Income and production
    g.bailCount = GET_INT(s, "BailCount", g.bailCount);
    g.buildSpeed = GET_FIXED(s, "BuildSpeed", g.buildSpeed);
    g.buildupTime = GET_FIXED(s, "BuildupTime", g.buildupTime);
    g.gemValue = GET_INT(s, "GemValue", g.gemValue);
    g.goldValue = GET_INT(s, "GoldValue", g.goldValue);
    g.growthRate = GET_FIXED(s, "GrowthRate", g.growthRate);
    g.oreGrows = GET_BOOL(s, "OreGrows", g.oreGrows);
    g.oreSpreads = GET_BOOL(s, "OreSpreads", g.oreSpreads);
    g.oreTruckRate = GET_FIXED(s, "OreTruckRate", g.oreTruckRate);
    g.separateAircraft = GET_BOOL(s, "SeparateAircraft", g.separateAircraft);
    g.survivorRate = GET_FIXED(s, "SurvivorRate", g.survivorRate);

    // Audio/visual
    g.allyReveal = GET_BOOL(s, "AllyReveal", g.allyReveal);
    g.conditionRed = GET_FIXED(s, "ConditionRed", g.conditionRed);
    g.conditionYellow = GET_FIXED(s, "ConditionYellow", g.conditionYellow);
    g.dropZoneRadius = GET_INT(s, "DropZoneRadius", g.dropZoneRadius);
    g.enemyHealth = GET_BOOL(s, "EnemyHealth", g.enemyHealth);
    g.gravity = GET_INT(s, "Gravity", g.gravity);
    g.idleActionFrequency = GET_FIXED(s, "IdleActionFrequency",
                                       g.idleActionFrequency);
    g.messageDelay = GET_FIXED(s, "MessageDelay", g.messageDelay);
    g.movieTime = GET_FIXED(s, "MovieTime", g.movieTime);
    g.namedCivilians = GET_BOOL(s, "NamedCivilians", g.namedCivilians);
    g.savourDelay = GET_FIXED(s, "SavourDelay", g.savourDelay);
    g.shroudRate = GET_FIXED(s, "ShroudRate", g.shroudRate);
    g.speakDelay = GET_FIXED(s, "SpeakDelay", g.speakDelay);
    g.timerWarning = GET_FIXED(s, "TimerWarning", g.timerWarning);
    g.flashLowPower = GET_BOOL(s, "FlashLowPower", g.flashLowPower);

    // Computer controls
    g.curleyShuffle = GET_BOOL(s, "CurleyShuffle", g.curleyShuffle);
    g.baseBias = GET_FIXED(s, "BaseBias", g.baseBias);
    g.baseDefenseDelay = GET_FIXED(s, "BaseDefenseDelay", g.baseDefenseDelay);
    g.closeEnough = GET_FIXED(s, "CloseEnough", g.closeEnough);
    g.damageDelay = GET_FIXED(s, "DamageDelay", g.damageDelay);
    g.gameSpeedBias = GET_FIXED(s, "GameSpeeBias", g.gameSpeedBias);  // typo
    g.lzScanRadius = GET_INT(s, "LZScanRadius", g.lzScanRadius);
    g.mineAware = GET_BOOL(s, "MineAware", g.mineAware);
    g.stray = GET_FIXED(s, "Stray", g.stray);
    g.submergeDelay = GET_FIXED(s, "SubmergeDelay", g.submergeDelay);
    g.suspendDelay = GET_FIXED(s, "SuspendDelay", g.suspendDelay);
    g.suspendPriority = GET_INT(s, "SuspendPriority", g.suspendPriority);
    g.teamDelay = GET_FIXED(s, "TeamDelay", g.teamDelay);

    // Misc
    g.fineDiffControl = GET_BOOL(s, "FineDiffControl", g.fineDiffControl);
    g.mcvUndeploy = GET_BOOL(s, "MCVUndeploy", g.mcvUndeploy);
}

//===========================================================================
// Process [IQ] section
//===========================================================================
void RulesClass::ProcessIQ() {
    const char* s = "IQ";
    IQSettings& q = iq_;

    q.maxLevels = GET_INT(s, "MaxIQLevels", q.maxLevels);
    q.superWeapons = GET_INT(s, "SuperWeapons", q.superWeapons);
    q.production = GET_INT(s, "Production", q.production);
    q.guardArea = GET_INT(s, "GuardArea", q.guardArea);
    q.repairSell = GET_INT(s, "RepairSell", q.repairSell);
    q.autoCrush = GET_INT(s, "AutoCrush", q.autoCrush);
    q.scatter = GET_INT(s, "Scatter", q.scatter);
    q.contentScan = GET_INT(s, "ContentScan", q.contentScan);
    q.aircraft = GET_INT(s, "Aircraft", q.aircraft);
    q.harvester = GET_INT(s, "Harvester", q.harvester);
    q.sellBack = GET_INT(s, "SellBack", q.sellBack);
}

// Helper: load difficulty settings for a section
static void LoadDiff(INIClass& ini, const char* s, DifficultySettings& d) {
    d.firepower = ini.GetFixed(s, "Firepower", d.firepower);
    d.groundSpeed = ini.GetFixed(s, "Groundspeed", d.groundSpeed);
    d.airSpeed = ini.GetFixed(s, "Airspeed", d.airSpeed);
    d.buildTime = ini.GetFixed(s, "BuildTime", d.buildTime);
    d.armor = ini.GetFixed(s, "Armor", d.armor);
    d.rof = ini.GetFixed(s, "ROF", d.rof);
    d.cost = ini.GetFixed(s, "Cost", d.cost);
    d.repairDelay = ini.GetFixed(s, "RepairDelay", d.repairDelay);
    d.buildDelay = ini.GetFixed(s, "BuildDelay", d.buildDelay);
    d.destroyWalls = ini.GetBool(s, "DestroyWalls", d.destroyWalls);
    d.contentScan = ini.GetBool(s, "ContentScan", d.contentScan);
}

//===========================================================================
// Process difficulty sections [Easy], [Normal], [Hard]
//===========================================================================
void RulesClass::ProcessDifficulty() {
    LoadDiff(ini_, "Easy", diffEasy_);
    LoadDiff(ini_, "Normal", diffNormal_);
    LoadDiff(ini_, "Difficult", diffHard_);
}

//===========================================================================
// Process country sections
//===========================================================================
void RulesClass::ProcessCountries() {
    static const char* countryNames[] = {
        "England", "Germany", "France", "Ukraine", "USSR",
        "Greece", "Turkey", "Spain"
    };

    countryCount_ = 0;
    for (const char* name : countryNames) {
        if (countryCount_ >= MAX_COUNTRIES) break;
        if (!ini_.SectionPresent(name)) continue;

        CountrySettings& c = countries_[countryCount_++];
        c.name = name;
        c.firepower = ini_.GetFixed(name, "Firepower", 1.0f);
        c.groundSpeed = ini_.GetFixed(name, "Groundspeed", 1.0f);
        c.airSpeed = ini_.GetFixed(name, "Airspeed", 1.0f);
        c.armor = ini_.GetFixed(name, "Armor", 1.0f);
        c.rof = ini_.GetFixed(name, "ROF", 1.0f);
        c.cost = ini_.GetFixed(name, "Cost", 1.0f);
        c.buildTime = ini_.GetFixed(name, "BuildTime", 1.0f);
    }
}

//===========================================================================
// Process infantry type sections
//===========================================================================
void RulesClass::ProcessInfantry() {
    // Infantry type data is in static tables, but RULES.INI can override
    // For now, just log that we would process infantry sections
    // In a full implementation, this would modify InfantryTypes[]

    // Example of what would be processed:
    // [E1] - Rifle Infantry
    // [E2] - Grenadier
    // etc.

    // Since our InfantryTypes[] is const, we would need mutable runtime
    // data. This is left as a future enhancement when game objects are
    // fully implemented.
}

//===========================================================================
// Process unit type sections
//===========================================================================
void RulesClass::ProcessUnits() {
    // Similar to infantry - unit type data is in static tables
    // RULES.INI overrides would be applied to mutable runtime data
}

//===========================================================================
// Process building type sections
//===========================================================================
void RulesClass::ProcessBuildings() {
    // Similar pattern - building types from static tables with INI overrides
}

//===========================================================================
// Process weapon sections
//===========================================================================
void RulesClass::ProcessWeapons() {
    // Weapon sections like [Colt45], [75mm], [M1Carbine] etc.
    // These override static weapon data
}

//===========================================================================
// Process warhead sections
//===========================================================================
void RulesClass::ProcessWarheads() {
    // Warhead sections like [SA], [HE], [AP] etc.
    // These define damage modifiers vs armor types
}

//===========================================================================
// Process projectile sections
//===========================================================================
void RulesClass::ProcessProjectiles() {
    // Projectile sections like [Invisible], [Ack], [Cannon] etc.
}

//===========================================================================
// Get difficulty settings by level
//===========================================================================
const DifficultySettings& RulesClass::GetDifficulty(int level) const {
    switch (level) {
        case 0: return diffEasy_;
        case 1: return diffNormal_;
        case 2:
        default: return diffHard_;
    }
}

//===========================================================================
// Get country settings by name
//===========================================================================
const CountrySettings* RulesClass::GetCountry(const char* name) const {
    if (name == nullptr) return nullptr;

    for (int i = 0; i < countryCount_; i++) {
        if (strcasecmp(countries_[i].name, name) == 0) {
            return &countries_[i];
        }
    }
    return nullptr;
}

//===========================================================================
// Parse helpers
//===========================================================================
ArmorType RulesClass::ParseArmor(const char* name) const {
    if (name == nullptr) return ArmorType::NONE;

    if (strcasecmp(name, "none") == 0) return ArmorType::NONE;
    if (strcasecmp(name, "wood") == 0) return ArmorType::WOOD;
    if (strcasecmp(name, "light") == 0) return ArmorType::LIGHT;
    if (strcasecmp(name, "heavy") == 0) return ArmorType::HEAVY;
    if (strcasecmp(name, "concrete") == 0) return ArmorType::CONCRETE;

    return ArmorType::NONE;
}

SpeedType RulesClass::ParseSpeed(const char* name) const {
    if (name == nullptr) return SpeedType::FOOT;

    if (strcasecmp(name, "foot") == 0) return SpeedType::FOOT;
    if (strcasecmp(name, "track") == 0) return SpeedType::TRACK;
    if (strcasecmp(name, "wheel") == 0) return SpeedType::WHEEL;
    if (strcasecmp(name, "float") == 0) return SpeedType::FLOAT;
    if (strcasecmp(name, "winged") == 0) return SpeedType::WINGED;
    // Note: HOVER not in current SpeedType enum, treat as FLOAT for hovercraft
    if (strcasecmp(name, "hover") == 0) return SpeedType::FLOAT;

    return SpeedType::FOOT;
}

WeaponType RulesClass::ParseWeapon(const char* name) const {
    if (name == nullptr) return WeaponType::NONE;

    // Map weapon name to WeaponType enum
    if (strcasecmp(name, "Colt45") == 0) return WeaponType::COLT45;
    if (strcasecmp(name, "M1Carbine") == 0) return WeaponType::M1CARBINE;
    if (strcasecmp(name, "ZSU-23") == 0) return WeaponType::ZSU23;
    if (strcasecmp(name, "Vulcan") == 0) return WeaponType::VULCAN;
    if (strcasecmp(name, "Maverick") == 0) return WeaponType::MAVERICK;
    if (strcasecmp(name, "Camera") == 0) return WeaponType::CAMERA;
    if (strcasecmp(name, "FireballLauncher") == 0) return WeaponType::FIREBALL;
    if (strcasecmp(name, "Sniper") == 0) return WeaponType::SNIPER;
    if (strcasecmp(name, "ChainGun") == 0) return WeaponType::CHAINGUN;
    if (strcasecmp(name, "Pistol") == 0) return WeaponType::PISTOL;
    if (strcasecmp(name, "Dragon") == 0) return WeaponType::DRAGON;
    if (strcasecmp(name, "Hellfire") == 0) return WeaponType::HELLFIRE;
    if (strcasecmp(name, "Grenade") == 0) return WeaponType::GRENADE;
    if (strcasecmp(name, "M60mg") == 0) return WeaponType::M60MG;
    if (strcasecmp(name, "Tomahawk") == 0) return WeaponType::TOMAHAWK;
    if (strcasecmp(name, "TOW") == 0) return WeaponType::TOW;
    if (strcasecmp(name, "MammothTusk") == 0) return WeaponType::MAMMOTH_TUSK;
    if (strcasecmp(name, "155mm") == 0) return WeaponType::M_CANNON;
    if (strcasecmp(name, "105mm") == 0) return WeaponType::TURRET_CANNON;
    if (strcasecmp(name, "DepthCharge") == 0) return WeaponType::DEPTH_CHARGE;
    if (strcasecmp(name, "Torpedo") == 0) return WeaponType::TORPEDO;
    if (strcasecmp(name, "AAGun") == 0) return WeaponType::AA_CANNON;
    if (strcasecmp(name, "TeslaZap") == 0) return WeaponType::TESLA_COIL;
    if (strcasecmp(name, "Nike") == 0) return WeaponType::NIKE;
    if (strcasecmp(name, "8Inch") == 0) return WeaponType::SCUD;
    if (strcasecmp(name, "Stinger") == 0) return WeaponType::STINGER;
    if (strcasecmp(name, "FireDeath") == 0) return WeaponType::FIRE;
    if (strcasecmp(name, "DogJaw") == 0) return WeaponType::DOG_JAW;
    if (strcasecmp(name, "Heal") == 0) return WeaponType::HEAL_MISSILE;

    return WeaponType::NONE;
}

//===========================================================================
// Initialize rules from RULES.INI
//===========================================================================
bool InitRules() {
    // Try to load from resources directory first
    if (Rules.Load("resources/RULES.INI")) {
        printf("Loaded RULES.INI from resources/\n");
        return true;
    }

    // Try app bundle resources
    if (Rules.Load("../Resources/RULES.INI")) {
        printf("Loaded RULES.INI from app bundle\n");
        return true;
    }

    // Try current directory
    if (Rules.Load("RULES.INI")) {
        printf("Loaded RULES.INI from current directory\n");
        return true;
    }

    printf("Warning: RULES.INI not found, using defaults\n");
    return false;
}
