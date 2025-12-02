/**
 * Red Alert macOS Port - House/Faction Management
 *
 * HouseTypeClass - Constant faction data (colors, names, side)
 * HouseClass - Runtime faction state (units, buildings, resources, AI)
 *
 * Based on original HOUSE.H (~916 lines)
 */

#ifndef GAME_HOUSE_H
#define GAME_HOUSE_H

#include "types.h"
#include <cstdint>

// Forward declarations
class TechnoClass;
class TechnoTypeClass;
class BuildingClass;
class UnitClass;
class InfantryClass;
class AircraftClass;
class TeamClass;
class TeamTypeClass;

//===========================================================================
// Constants
//===========================================================================

// Maximum number of houses in game
constexpr int HOUSE_MAX = 16;

// AI difficulty settings
enum class DifficultyType : int8_t {
    EASY = 0,
    NORMAL,
    HARD,

    COUNT
};

// House state machine
enum class HouseStateType : int8_t {
    NONE = 0,
    BUILDUP,        // Building up base
    BROKE,          // Out of money
    THREATENED,     // Under attack
    ATTACKED,       // Being actively attacked
    ENDGAME,        // Final assault

    COUNT
};

// AI urgency levels for strategy evaluation
enum class UrgencyType : int8_t {
    NONE = 0,
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL,

    COUNT
};

// Strategy types the AI evaluates
enum class StrategyType : int8_t {
    BUILD_POWER = 0,
    BUILD_DEFENSE,
    BUILD_INCOME,
    BUILD_ENGINEER,
    BUILD_OFFENSE,
    RAISE_POWER,
    RAISE_MONEY,
    FIRE_SALE,
    ATTACK,
    ALL_IN,

    COUNT
};

// Quarry types for attack targeting
enum class QuarryType : int8_t {
    NONE = -1,
    ANYTHING = 0,       // Any valid target
    BUILDINGS,          // Target buildings
    HARVESTERS,         // Target harvesters
    INFANTRY,           // Target infantry
    VEHICLES,           // Target vehicles
    FACTORIES,          // Target production buildings
    DEFENSE,            // Target defensive structures
    THREAT,             // Target nearest threat
    POWER,              // Target power plants
    TIBERIUM,           // Target refineries/silos

    COUNT
};

// Side affiliation
enum class SideType : int8_t {
    NONE = -1,
    ALLIED = 0,
    SOVIET,
    NEUTRAL,

    COUNT
};

//===========================================================================
// HouseTypeClass - Constant faction data
//===========================================================================

struct HouseTypeData {
    const char* iniName;        // INI section name
    const char* fullName;       // Display name
    const char* suffix;         // 3-char suffix for assets
    SideType side;              // Allied/Soviet/Neutral
    uint8_t colorScheme;        // Primary color index
    uint8_t brightColor;        // Bright color for radar
    int16_t firepower;          // Firepower bonus (256 = 100%)
    int16_t groundSpeed;        // Ground speed bonus
    int16_t airSpeed;           // Air speed bonus
    int16_t armor;              // Armor bonus
    int16_t rof;                // Rate of fire bonus
    int16_t cost;               // Cost multiplier
    int16_t buildTime;          // Build time multiplier
};

const HouseTypeData* GetHouseType(HousesType type);
HousesType HouseTypeFromName(const char* name);

//===========================================================================
// HouseClass - Runtime faction state
//===========================================================================

class HouseClass {
public:
    //-----------------------------------------------------------------------
    // Identity
    //-----------------------------------------------------------------------
    HousesType type_;                   // House type
    int16_t id_;                        // Unique instance ID
    bool isActive_;                     // In game
    bool isHuman_;                      // Human player
    bool isPlayerControl_;              // Player can control
    bool isDefeated_;                   // Has lost
    bool isToWin_;                      // Has won
    bool isToLose_;                     // Scheduled to lose
    bool isAlerted_;                    // Enemy spotted
    bool isDiscovered_;                 // Has been discovered by player
    bool isMaxedOut_;                   // Production capacity maxed
    bool isStarted_;                    // Production enabled (BEGIN_PROD)
    bool isBaseBuilding_;               // Building a base (skirmish mode)

    //-----------------------------------------------------------------------
    // Alliances
    //-----------------------------------------------------------------------
    uint32_t allies_;                   // Bitfield of allied houses

    //-----------------------------------------------------------------------
    // Resources
    //-----------------------------------------------------------------------
    int32_t credits_;                   // Available credits
    int32_t tiberium_;                  // Tiberium in storage
    int32_t capacity_;                  // Storage capacity
    int32_t drain_;                     // Power drain
    int32_t power_;                     // Power generation

    //-----------------------------------------------------------------------
    // Production tracking
    //-----------------------------------------------------------------------
    int32_t bKilled_;                   // Buildings killed
    int32_t uKilled_;                   // Units killed
    int32_t iKilled_;                   // Infantry killed
    int32_t aKilled_;                   // Aircraft killed
    int32_t bLost_;                     // Buildings lost
    int32_t uLost_;                     // Units lost
    int32_t iLost_;                     // Infantry lost
    int32_t aLost_;                     // Aircraft lost
    int32_t bBuilt_;                    // Buildings built
    int32_t uBuilt_;                    // Units built
    int32_t iBuilt_;                    // Infantry built
    int32_t aBuilt_;                    // Aircraft built
    int32_t harvested_;                 // Total credits harvested

    //-----------------------------------------------------------------------
    // Unit inventory (bitfields)
    //-----------------------------------------------------------------------
    uint64_t bScan_;                    // Building types owned (bit per type)
    uint64_t uScan_;                    // Unit types owned
    uint64_t iScan_;                    // Infantry types owned
    uint64_t aScan_;                    // Aircraft types owned
    uint64_t vScan_;                    // Vessel types owned

    //-----------------------------------------------------------------------
    // AI State
    //-----------------------------------------------------------------------
    DifficultyType difficulty_;         // AI difficulty
    HouseStateType state_;              // Current state machine state
    int16_t alertTimer_;                // Ticks until alert expires
    int16_t aiTimer_;                   // Ticks until next AI think

    // Strategy urgency levels (evaluated by Expert_AI)
    UrgencyType urgency_[static_cast<int>(StrategyType::COUNT)];

    // Current suggested actions
    int8_t buildBuilding_;              // Building type to build
    int8_t buildUnit_;                  // Unit type to build
    int8_t buildInfantry_;              // Infantry type to build
    int8_t buildAircraft_;              // Aircraft type to build

    //-----------------------------------------------------------------------
    // Attack tracking
    //-----------------------------------------------------------------------
    HousesType enemy_;                  // Primary enemy house
    HousesType lastAttacker_;           // Who last attacked us
    int32_t lastAttackFrame_;           // When we were last attacked

    //-----------------------------------------------------------------------
    // Base location
    //-----------------------------------------------------------------------
    int32_t baseCenter_;                // Coordinate of base center
    int16_t baseRadius_;                // Radius of base area

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    HouseClass();
    HouseClass(HousesType type);
    ~HouseClass() = default;

    void Init(HousesType type);

    //-----------------------------------------------------------------------
    // Type queries
    //-----------------------------------------------------------------------
    const HouseTypeData* TypeClass() const;
    const char* Name() const;
    SideType Side() const;
    bool IsAllied() const;
    bool IsSoviet() const;

    //-----------------------------------------------------------------------
    // Alliance queries
    //-----------------------------------------------------------------------
    bool Is_Ally(HousesType house) const;
    bool Is_Ally(const HouseClass* house) const;
    void Make_Ally(HousesType house);
    void Make_Enemy(HousesType house);

    //-----------------------------------------------------------------------
    // Resource management
    //-----------------------------------------------------------------------
    int Available_Money() const { return credits_ + tiberium_; }
    bool Spend_Money(int amount);
    void Refund_Money(int amount);
    void Harvest_Tiberium(int amount, int storage);
    int Power_Fraction() const;  // Returns 0-256 (256 = 100% power)

    //-----------------------------------------------------------------------
    // Production
    //-----------------------------------------------------------------------
    uint32_t Get_Prereqs_Met() const;
    bool Can_Build(int type, RTTIType rtti) const;
    int Cost_Of(int type, RTTIType rtti) const;

    //-----------------------------------------------------------------------
    // Unit tracking
    //-----------------------------------------------------------------------
    void Tracking_Add(TechnoClass* object);
    void Tracking_Remove(TechnoClass* object);

    //-----------------------------------------------------------------------
    // AI
    //-----------------------------------------------------------------------
    void AI();                          // Per-frame AI processing
    void Expert_AI();                   // High-level strategy evaluation
    int AI_Unit();                      // Decide what unit to build
    int AI_Infantry();                  // Decide what infantry to build
    int AI_Building();                  // Decide what building to build
    int AI_Aircraft();                  // Decide what aircraft to build
    UrgencyType Check_Build_Power() const;
    UrgencyType Check_Build_Defense() const;
    UrgencyType Check_Build_Offense() const;
    UrgencyType Check_Attack() const;
    UrgencyType Check_Fire_Sale() const;

    //-----------------------------------------------------------------------
    // Production Control
    //-----------------------------------------------------------------------
    void Begin_Production() { isStarted_ = true; }
    const TechnoTypeClass* Suggest_New_Object(RTTIType rtti) const;

    HousesType Find_Enemy() const;      // Select best enemy
    int32_t Find_Cell_In_Zone(int zone) const;  // Find cell in threat zone

    //-----------------------------------------------------------------------
    // Team management
    //-----------------------------------------------------------------------
    TeamTypeClass* Suggested_New_Team(bool alert) const;
    int Team_Count() const;
    void Recruit(TechnoClass* object);

    //-----------------------------------------------------------------------
    // Combat callbacks
    //-----------------------------------------------------------------------
    void Attacked(TechnoClass* source);
    void Destroyed(TechnoClass* object);
    void Killed(TechnoClass* object);

    //-----------------------------------------------------------------------
    // Static
    //-----------------------------------------------------------------------
    static HouseClass* As_Pointer(HousesType type);
};

//===========================================================================
// Global House Array
//===========================================================================
extern HouseClass Houses[HOUSE_MAX];
extern HouseClass* PlayerPtr;           // Human player's house
extern int HouseCount;                  // Number of active houses

//===========================================================================
// Helper Functions
//===========================================================================

HouseClass* Find_House(HousesType type);
void Init_Houses();

#endif // GAME_HOUSE_H
