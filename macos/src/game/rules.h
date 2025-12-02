/**
 * Red Alert macOS Port - Rules Processing
 *
 * Loads game rules from RULES.INI and applies them to type data tables.
 * Based on original RULES.CPP but using modern INI parser.
 */

#ifndef GAME_RULES_H
#define GAME_RULES_H

#include "ini.h"
#include "types.h"
#include <cstdint>

//===========================================================================
// Difficulty Settings - Per-difficulty multipliers
//===========================================================================
struct DifficultySettings {
    float firepower;        // Damage multiplier
    float groundSpeed;      // Ground unit speed multiplier
    float airSpeed;         // Air unit speed multiplier
    float buildTime;        // Build time multiplier
    float armor;            // Armor multiplier
    float rof;              // Rate of fire multiplier (larger = slower)
    float cost;             // Cost multiplier
    float repairDelay;      // Delay between repairs (minutes)
    float buildDelay;       // Delay between builds (minutes)
    bool destroyWalls;      // AI destroys walls
    bool contentScan;       // AI considers transport contents
};

//===========================================================================
// Country Settings - Per-country multipliers (multiplayer only)
//===========================================================================
struct CountrySettings {
    const char* name;       // Country name
    float firepower;
    float groundSpeed;
    float airSpeed;
    float armor;
    float rof;
    float cost;
    float buildTime;
};

//===========================================================================
// IQ Settings - Computer AI intelligence levels
//===========================================================================
struct IQSettings {
    int maxLevels;          // Number of IQ levels
    int superWeapons;       // IQ to auto-fire super weapons
    int production;         // IQ for auto production
    int guardArea;          // IQ for guard area mode
    int repairSell;         // IQ for repair/sell decisions
    int autoCrush;          // IQ for auto-crush infantry
    int scatter;            // IQ for scattering from threats
    int contentScan;        // IQ for checking transport contents
    int aircraft;           // IQ for auto-replacing aircraft
    int harvester;          // IQ for auto-replacing harvesters
    int sellBack;           // IQ for selling buildings
};

//===========================================================================
// General Rules - Global game settings from [General]
//===========================================================================
struct GeneralRules {
    // Crates
    int crateMinimum;
    int crateMaximum;
    float crateRadius;
    float crateRegen;
    float waterCrateChance;
    int soloCrateMoney;

    // Special weapons
    float chronoDuration;
    bool chronoKillCargo;
    int chronoTechLevel;
    int gpsTechLevel;
    int gapRadius;
    float gapRegenInterval;
    float ironCurtainDuration;
    int paraTech;
    int parabombTech;
    int radarJamRadius;
    int spyPlaneTech;
    int badgerBombCount;

    // Chrono side effects
    float quakeChance;
    float quakeDamage;
    float vortexChance;
    int vortexDamage;
    int vortexRange;
    int vortexSpeed;

    // Repair and refit
    float refundPercent;
    float reloadRate;
    float repairPercent;
    float repairRate;
    int repairStep;
    float uRepairPercent;
    int uRepairStep;

    // Combat and damage
    float turboBoost;
    int apMineDamage;
    int avMineDamage;
    int atomDamage;
    float ballisticScatter;
    int bridgeStrength;
    float c4Delay;
    float crushDistance;
    float expSpread;
    float fireSuppress;
    float homingScatter;
    int maxDamage;
    int minDamage;
    bool oreExplosive;
    bool playerAutoCrush;
    bool playerReturnFire;
    bool playerScatter;
    float proneDamage;
    bool treeTargeting;
    int incomingSpeed;

    // Income and production
    int bailCount;
    float buildSpeed;
    float buildupTime;
    int gemValue;
    int goldValue;
    float growthRate;
    bool oreGrows;
    bool oreSpreads;
    float oreTruckRate;
    bool separateAircraft;
    float survivorRate;

    // Audio/visual
    bool allyReveal;
    float conditionRed;
    float conditionYellow;
    int dropZoneRadius;
    bool enemyHealth;
    int gravity;
    float idleActionFrequency;
    float messageDelay;
    float movieTime;
    bool namedCivilians;
    float savourDelay;
    float shroudRate;
    float speakDelay;
    float timerWarning;
    bool flashLowPower;

    // Computer controls
    bool curleyShuffle;
    float baseBias;
    float baseDefenseDelay;
    float closeEnough;
    float damageDelay;
    float gameSpeedBias;
    int lzScanRadius;
    bool mineAware;
    float stray;
    float submergeDelay;
    float suspendDelay;
    int suspendPriority;
    float teamDelay;

    // Misc
    bool fineDiffControl;
    bool mcvUndeploy;
};

//===========================================================================
// RulesClass - Main rules processor
//===========================================================================
class RulesClass {
public:
    RulesClass();
    ~RulesClass() = default;

    /**
     * Load rules from an INI file path
     */
    bool Load(const char* filename);

    /**
     * Load rules from a memory buffer
     */
    bool LoadFromBuffer(const char* data, size_t size);

    /**
     * Process all rules and apply to type tables
     */
    bool Process();

    /**
     * Check if rules are loaded
     */
    bool IsLoaded() const { return ini_.IsLoaded(); }

    // Access to settings
    const GeneralRules& General() const { return general_; }
    const IQSettings& IQ() const { return iq_; }
    const DifficultySettings& GetDifficulty(int level) const;
    const CountrySettings* GetCountry(const char* name) const;

private:
    // INI file data
    INIClass ini_;

    // Processed settings
    GeneralRules general_;
    IQSettings iq_;
    DifficultySettings diffEasy_;
    DifficultySettings diffNormal_;
    DifficultySettings diffHard_;

    // Country settings (multiplayer)
    static constexpr int MAX_COUNTRIES = 10;
    CountrySettings countries_[MAX_COUNTRIES];
    int countryCount_;

    // Processing functions
    void ProcessGeneral();
    void ProcessIQ();
    void ProcessDifficulty();
    void ProcessCountries();
    void ProcessInfantry();
    void ProcessUnits();
    void ProcessBuildings();
    void ProcessWeapons();
    void ProcessWarheads();
    void ProcessProjectiles();

    // Helper functions
    ArmorType ParseArmor(const char* name) const;
    WeaponType ParseWeapon(const char* name) const;
    SpeedType ParseSpeed(const char* name) const;
    uint32_t ParseOwners(const char* str) const;
    uint32_t ParsePrereqs(const char* str) const;

    // Set defaults
    void SetDefaults();
};

//===========================================================================
// Global Rules Instance
//===========================================================================
extern RulesClass Rules;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Initialize rules system with RULES.INI from resources
 */
bool InitRules();

/**
 * Get ore/gem values from RULES.INI (for use without including full rules.h)
 */
int Rules_GetGoldValue();
int Rules_GetGemValue();

#endif // GAME_RULES_H
