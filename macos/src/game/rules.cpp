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
    diffEasy_ = {1.2f, 1.2f, 1.2f, 0.8f, 1.2f, 0.8f, 0.8f, 0.001f, 0.001f, true, false};
    diffNormal_ = {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.02f, 0.03f, true, false};
    diffHard_ = {0.9f, 0.9f, 0.9f, 1.2f, 0.8f, 1.2f, 1.1f, 0.02f, 0.03f, true, true};
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
    const char* section = "General";

    // Crates
    general_.crateMinimum = ini_.GetInt(section, "CrateMinimum", general_.crateMinimum);
    general_.crateMaximum = ini_.GetInt(section, "CrateMaximum", general_.crateMaximum);
    general_.crateRadius = ini_.GetFixed(section, "CrateRadius", general_.crateRadius);
    general_.crateRegen = ini_.GetFixed(section, "CrateRegen", general_.crateRegen);
    general_.waterCrateChance = ini_.GetFixed(section, "WaterCrateChance", general_.waterCrateChance);
    general_.soloCrateMoney = ini_.GetInt(section, "SoloCrateMoney", general_.soloCrateMoney);

    // Special weapons
    general_.chronoDuration = ini_.GetFixed(section, "ChronoDuration", general_.chronoDuration);
    general_.chronoKillCargo = ini_.GetBool(section, "ChronoKillCargo", general_.chronoKillCargo);
    general_.chronoTechLevel = ini_.GetInt(section, "ChronoTechLevel", general_.chronoTechLevel);
    general_.gpsTechLevel = ini_.GetInt(section, "GPSTechLevel", general_.gpsTechLevel);
    general_.gapRadius = ini_.GetInt(section, "GapRadius", general_.gapRadius);
    general_.gapRegenInterval = ini_.GetFixed(section, "GapRegenInterval", general_.gapRegenInterval);
    general_.ironCurtainDuration = ini_.GetFixed(section, "IronCurtain", general_.ironCurtainDuration);
    general_.paraTech = ini_.GetInt(section, "ParaTech", general_.paraTech);
    general_.parabombTech = ini_.GetInt(section, "ParabombTech", general_.parabombTech);
    general_.radarJamRadius = ini_.GetInt(section, "RadarJamRadius", general_.radarJamRadius);
    general_.spyPlaneTech = ini_.GetInt(section, "SpyPlaneTech", general_.spyPlaneTech);
    general_.badgerBombCount = ini_.GetInt(section, "BadgerBombCount", general_.badgerBombCount);

    // Chrono side effects
    general_.quakeChance = ini_.GetFixed(section, "QuakeChance", general_.quakeChance);
    general_.quakeDamage = ini_.GetFixed(section, "QuakeDamage", general_.quakeDamage);
    general_.vortexChance = ini_.GetFixed(section, "VortexChance", general_.vortexChance);
    general_.vortexDamage = ini_.GetInt(section, "VortexDamage", general_.vortexDamage);
    general_.vortexRange = ini_.GetInt(section, "VortexRange", general_.vortexRange);
    general_.vortexSpeed = ini_.GetInt(section, "VortexSpeed", general_.vortexSpeed);

    // Repair and refit
    general_.refundPercent = ini_.GetFixed(section, "RefundPercent", general_.refundPercent);
    general_.reloadRate = ini_.GetFixed(section, "ReloadRate", general_.reloadRate);
    general_.repairPercent = ini_.GetFixed(section, "RepairPercent", general_.repairPercent);
    general_.repairRate = ini_.GetFixed(section, "RepairRate", general_.repairRate);
    general_.repairStep = ini_.GetInt(section, "RepairStep", general_.repairStep);
    general_.uRepairPercent = ini_.GetFixed(section, "URepairPercent", general_.uRepairPercent);
    general_.uRepairStep = ini_.GetInt(section, "URepairStep", general_.uRepairStep);

    // Combat and damage
    general_.turboBoost = ini_.GetFixed(section, "TurboBoost", general_.turboBoost);
    general_.apMineDamage = ini_.GetInt(section, "APMineDamage", general_.apMineDamage);
    general_.avMineDamage = ini_.GetInt(section, "AVMineDamage", general_.avMineDamage);
    general_.atomDamage = ini_.GetInt(section, "AtomDamage", general_.atomDamage);
    general_.ballisticScatter = ini_.GetFixed(section, "BallisticScatter", general_.ballisticScatter);
    general_.bridgeStrength = ini_.GetInt(section, "BridgeStrength", general_.bridgeStrength);
    general_.c4Delay = ini_.GetFixed(section, "C4Delay", general_.c4Delay);
    general_.crushDistance = ini_.GetFixed(section, "Crush", general_.crushDistance);
    general_.expSpread = ini_.GetFixed(section, "ExpSpread", general_.expSpread);
    general_.fireSuppress = ini_.GetFixed(section, "FireSupress", general_.fireSuppress);
    general_.homingScatter = ini_.GetFixed(section, "HomingScatter", general_.homingScatter);
    general_.maxDamage = ini_.GetInt(section, "MaxDamage", general_.maxDamage);
    general_.minDamage = ini_.GetInt(section, "MinDamage", general_.minDamage);
    general_.oreExplosive = ini_.GetBool(section, "OreExplosive", general_.oreExplosive);
    general_.playerAutoCrush = ini_.GetBool(section, "PlayerAutoCrush", general_.playerAutoCrush);
    general_.playerReturnFire = ini_.GetBool(section, "PlayerReturnFire", general_.playerReturnFire);
    general_.playerScatter = ini_.GetBool(section, "PlayerScatter", general_.playerScatter);
    general_.proneDamage = ini_.GetFixed(section, "ProneDamage", general_.proneDamage);
    general_.treeTargeting = ini_.GetBool(section, "TreeTargeting", general_.treeTargeting);
    general_.incomingSpeed = ini_.GetInt(section, "Incoming", general_.incomingSpeed);

    // Income and production
    general_.bailCount = ini_.GetInt(section, "BailCount", general_.bailCount);
    general_.buildSpeed = ini_.GetFixed(section, "BuildSpeed", general_.buildSpeed);
    general_.buildupTime = ini_.GetFixed(section, "BuildupTime", general_.buildupTime);
    general_.gemValue = ini_.GetInt(section, "GemValue", general_.gemValue);
    general_.goldValue = ini_.GetInt(section, "GoldValue", general_.goldValue);
    general_.growthRate = ini_.GetFixed(section, "GrowthRate", general_.growthRate);
    general_.oreGrows = ini_.GetBool(section, "OreGrows", general_.oreGrows);
    general_.oreSpreads = ini_.GetBool(section, "OreSpreads", general_.oreSpreads);
    general_.oreTruckRate = ini_.GetFixed(section, "OreTruckRate", general_.oreTruckRate);
    general_.separateAircraft = ini_.GetBool(section, "SeparateAircraft", general_.separateAircraft);
    general_.survivorRate = ini_.GetFixed(section, "SurvivorRate", general_.survivorRate);

    // Audio/visual
    general_.allyReveal = ini_.GetBool(section, "AllyReveal", general_.allyReveal);
    general_.conditionRed = ini_.GetFixed(section, "ConditionRed", general_.conditionRed);
    general_.conditionYellow = ini_.GetFixed(section, "ConditionYellow", general_.conditionYellow);
    general_.dropZoneRadius = ini_.GetInt(section, "DropZoneRadius", general_.dropZoneRadius);
    general_.enemyHealth = ini_.GetBool(section, "EnemyHealth", general_.enemyHealth);
    general_.gravity = ini_.GetInt(section, "Gravity", general_.gravity);
    general_.idleActionFrequency = ini_.GetFixed(section, "IdleActionFrequency", general_.idleActionFrequency);
    general_.messageDelay = ini_.GetFixed(section, "MessageDelay", general_.messageDelay);
    general_.movieTime = ini_.GetFixed(section, "MovieTime", general_.movieTime);
    general_.namedCivilians = ini_.GetBool(section, "NamedCivilians", general_.namedCivilians);
    general_.savourDelay = ini_.GetFixed(section, "SavourDelay", general_.savourDelay);
    general_.shroudRate = ini_.GetFixed(section, "ShroudRate", general_.shroudRate);
    general_.speakDelay = ini_.GetFixed(section, "SpeakDelay", general_.speakDelay);
    general_.timerWarning = ini_.GetFixed(section, "TimerWarning", general_.timerWarning);
    general_.flashLowPower = ini_.GetBool(section, "FlashLowPower", general_.flashLowPower);

    // Computer controls
    general_.curleyShuffle = ini_.GetBool(section, "CurleyShuffle", general_.curleyShuffle);
    general_.baseBias = ini_.GetFixed(section, "BaseBias", general_.baseBias);
    general_.baseDefenseDelay = ini_.GetFixed(section, "BaseDefenseDelay", general_.baseDefenseDelay);
    general_.closeEnough = ini_.GetFixed(section, "CloseEnough", general_.closeEnough);
    general_.damageDelay = ini_.GetFixed(section, "DamageDelay", general_.damageDelay);
    general_.gameSpeedBias = ini_.GetFixed(section, "GameSpeeBias", general_.gameSpeedBias);  // Note: typo in original
    general_.lzScanRadius = ini_.GetInt(section, "LZScanRadius", general_.lzScanRadius);
    general_.mineAware = ini_.GetBool(section, "MineAware", general_.mineAware);
    general_.stray = ini_.GetFixed(section, "Stray", general_.stray);
    general_.submergeDelay = ini_.GetFixed(section, "SubmergeDelay", general_.submergeDelay);
    general_.suspendDelay = ini_.GetFixed(section, "SuspendDelay", general_.suspendDelay);
    general_.suspendPriority = ini_.GetInt(section, "SuspendPriority", general_.suspendPriority);
    general_.teamDelay = ini_.GetFixed(section, "TeamDelay", general_.teamDelay);

    // Misc
    general_.fineDiffControl = ini_.GetBool(section, "FineDiffControl", general_.fineDiffControl);
    general_.mcvUndeploy = ini_.GetBool(section, "MCVUndeploy", general_.mcvUndeploy);
}

//===========================================================================
// Process [IQ] section
//===========================================================================
void RulesClass::ProcessIQ() {
    const char* section = "IQ";

    iq_.maxLevels = ini_.GetInt(section, "MaxIQLevels", iq_.maxLevels);
    iq_.superWeapons = ini_.GetInt(section, "SuperWeapons", iq_.superWeapons);
    iq_.production = ini_.GetInt(section, "Production", iq_.production);
    iq_.guardArea = ini_.GetInt(section, "GuardArea", iq_.guardArea);
    iq_.repairSell = ini_.GetInt(section, "RepairSell", iq_.repairSell);
    iq_.autoCrush = ini_.GetInt(section, "AutoCrush", iq_.autoCrush);
    iq_.scatter = ini_.GetInt(section, "Scatter", iq_.scatter);
    iq_.contentScan = ini_.GetInt(section, "ContentScan", iq_.contentScan);
    iq_.aircraft = ini_.GetInt(section, "Aircraft", iq_.aircraft);
    iq_.harvester = ini_.GetInt(section, "Harvester", iq_.harvester);
    iq_.sellBack = ini_.GetInt(section, "SellBack", iq_.sellBack);
}

//===========================================================================
// Process difficulty sections [Easy], [Normal], [Hard]
//===========================================================================
void RulesClass::ProcessDifficulty() {
    // Easy
    const char* section = "Easy";
    diffEasy_.firepower = ini_.GetFixed(section, "Firepower", diffEasy_.firepower);
    diffEasy_.groundSpeed = ini_.GetFixed(section, "Groundspeed", diffEasy_.groundSpeed);
    diffEasy_.airSpeed = ini_.GetFixed(section, "Airspeed", diffEasy_.airSpeed);
    diffEasy_.buildTime = ini_.GetFixed(section, "BuildTime", diffEasy_.buildTime);
    diffEasy_.armor = ini_.GetFixed(section, "Armor", diffEasy_.armor);
    diffEasy_.rof = ini_.GetFixed(section, "ROF", diffEasy_.rof);
    diffEasy_.cost = ini_.GetFixed(section, "Cost", diffEasy_.cost);
    diffEasy_.repairDelay = ini_.GetFixed(section, "RepairDelay", diffEasy_.repairDelay);
    diffEasy_.buildDelay = ini_.GetFixed(section, "BuildDelay", diffEasy_.buildDelay);
    diffEasy_.destroyWalls = ini_.GetBool(section, "DestroyWalls", diffEasy_.destroyWalls);
    diffEasy_.contentScan = ini_.GetBool(section, "ContentScan", diffEasy_.contentScan);

    // Normal
    section = "Normal";
    diffNormal_.firepower = ini_.GetFixed(section, "Firepower", diffNormal_.firepower);
    diffNormal_.groundSpeed = ini_.GetFixed(section, "Groundspeed", diffNormal_.groundSpeed);
    diffNormal_.airSpeed = ini_.GetFixed(section, "Airspeed", diffNormal_.airSpeed);
    diffNormal_.buildTime = ini_.GetFixed(section, "BuildTime", diffNormal_.buildTime);
    diffNormal_.armor = ini_.GetFixed(section, "Armor", diffNormal_.armor);
    diffNormal_.rof = ini_.GetFixed(section, "ROF", diffNormal_.rof);
    diffNormal_.cost = ini_.GetFixed(section, "Cost", diffNormal_.cost);
    diffNormal_.repairDelay = ini_.GetFixed(section, "RepairDelay", diffNormal_.repairDelay);
    diffNormal_.buildDelay = ini_.GetFixed(section, "BuildDelay", diffNormal_.buildDelay);
    diffNormal_.destroyWalls = ini_.GetBool(section, "DestroyWalls", diffNormal_.destroyWalls);
    diffNormal_.contentScan = ini_.GetBool(section, "ContentScan", diffNormal_.contentScan);

    // Hard (Difficult)
    section = "Difficult";
    diffHard_.firepower = ini_.GetFixed(section, "Firepower", diffHard_.firepower);
    diffHard_.groundSpeed = ini_.GetFixed(section, "Groundspeed", diffHard_.groundSpeed);
    diffHard_.airSpeed = ini_.GetFixed(section, "Airspeed", diffHard_.airSpeed);
    diffHard_.buildTime = ini_.GetFixed(section, "BuildTime", diffHard_.buildTime);
    diffHard_.armor = ini_.GetFixed(section, "Armor", diffHard_.armor);
    diffHard_.rof = ini_.GetFixed(section, "ROF", diffHard_.rof);
    diffHard_.cost = ini_.GetFixed(section, "Cost", diffHard_.cost);
    diffHard_.repairDelay = ini_.GetFixed(section, "RepairDelay", diffHard_.repairDelay);
    diffHard_.buildDelay = ini_.GetFixed(section, "BuildDelay", diffHard_.buildDelay);
    diffHard_.destroyWalls = ini_.GetBool(section, "DestroyWalls", diffHard_.destroyWalls);
    diffHard_.contentScan = ini_.GetBool(section, "ContentScan", diffHard_.contentScan);
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

    // Since our InfantryTypes[] is const, we would need mutable runtime data
    // This is left as a future enhancement when game objects are fully implemented
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
