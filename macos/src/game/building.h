/**
 * Red Alert macOS Port - Building Class
 *
 * Structures that can produce units, provide power, and defend.
 * Extends TechnoClass directly (buildings don't move).
 *
 * Based on original BUILDING.H
 */

#ifndef GAME_BUILDING_H
#define GAME_BUILDING_H

#include "object.h"
#include "building_types.h"

// Maximum buildings in game
constexpr int BUILDING_MAX = 500;

// Building animation states
enum class BStateType : int8_t {
    CONSTRUCTION = 0,   // Under construction
    IDLE,               // Normal idle state
    ACTIVE,             // Active (producing, firing)
    FULL,               // Storage full
    AUX1,               // Auxiliary state 1
    AUX2,               // Auxiliary state 2

    COUNT
};

// Factory production state
enum class FactoryState : int8_t {
    IDLE = 0,
    BUILDING,       // Building something
    HOLDING,        // Production on hold
    READY,          // Production complete, waiting for placement/exit
    SUSPENDED       // Suspended (low power)
};

/**
 * BuildingClass - Structures
 *
 * Buildings are static structures that:
 * - Occupy multiple cells (1x1 to 3x3)
 * - Can produce units, infantry, or aircraft
 * - Generate or consume power
 * - Have turrets for defense
 * - Can be captured by engineers
 */
class BuildingClass : public TechnoClass {
public:
    //-----------------------------------------------------------------------
    // Building-specific state
    //-----------------------------------------------------------------------

    // Type reference
    BuildingType type_;

    // Building state
    BStateType bstate_;         // Current animation state
    BStateType bstateTarget_;   // Target animation state

    // Animation
    int16_t frame_;             // Current animation frame
    int16_t stageCount_;        // Frame counter
    int16_t animStage_;         // Animation stage (for multi-stage buildings)

    // Factory state
    FactoryState factoryState_;
    int16_t productionProgress_;  // 0-100% production progress
    RTTIType producingType_;      // What RTTI type being produced
    int16_t producingIndex_;      // Index of item being produced

    // Power state
    bool isPowered_;            // Has sufficient power
    bool isRepairing_;          // Being repaired

    // Special flags
    bool hasCharged_;           // Chronosphere/Iron Curtain charged
    bool isCapturable_;         // Can be captured by engineer
    bool isGoingToBlow_;        // Destruction imminent
    bool isSurvivorless_;       // No survivors on destruction

    // Countdown timers
    int16_t countdownTimer_;    // General purpose timer
    int16_t chargeTimer_;       // Superweapon charge timer

    // Last target for turret buildings
    uint32_t lastTargetCoord_;

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    BuildingClass();
    BuildingClass(BuildingType type, HousesType house);
    virtual ~BuildingClass() = default;

    // Initialize from type
    void Init(BuildingType type, HousesType house);

    //-----------------------------------------------------------------------
    // Type queries
    //-----------------------------------------------------------------------

    const BuildingTypeData* TypeClass() const;
    const char* Name() const override;

    // Is this a factory?
    bool IsFactory() const;
    RTTIType FactoryType() const;

    // Is this a power plant?
    bool IsPowerPlant() const;
    int PowerOutput() const;

    // Is this a refinery?
    bool IsRefinery() const;

    // Is this a wall?
    bool IsWall() const;

    // Does this building have a turret?
    bool HasTurret() const;

    // Get building size
    void GetSize(int& width, int& height) const;

    //-----------------------------------------------------------------------
    // Cell occupation
    //-----------------------------------------------------------------------

    // Get list of cells this building occupies
    virtual const int16_t* OccupyList(bool placement = false) const override;

    // Get center coordinate
    virtual int32_t CenterCoord() const override;

    // Get exit coordinate for produced units
    virtual int32_t ExitCoord() const override;

    // Check if can place at location
    bool CanPlaceAt(CELL cell) const;

    //-----------------------------------------------------------------------
    // Factory operations
    //-----------------------------------------------------------------------

    // Start production
    bool StartProduction(RTTIType type, int index);

    // Cancel production
    bool CancelProduction();

    // Pause/resume production
    bool PauseProduction();
    bool ResumeProduction();

    // Get production progress (0-100)
    int ProductionProgress() const { return productionProgress_; }

    // Check if production ready
    bool IsProductionReady() const { return factoryState_ == FactoryState::READY; }

    // Complete production (create unit/building)
    bool CompleteProduction();

    //-----------------------------------------------------------------------
    // Power system
    //-----------------------------------------------------------------------

    // Update power state
    void UpdatePower();

    // Get power output (positive = generates, negative = consumes)
    int GetPower() const;

    //-----------------------------------------------------------------------
    // Combat
    //-----------------------------------------------------------------------

    virtual bool CanFire() const override;
    virtual int WeaponRange(int weapon = 0) const override;
    virtual int RearmTime(int weapon = 0) const override;
    virtual ResultType TakeDamage(int& damage, int distance, WarheadType warhead,
                                   TechnoClass* source = nullptr, bool forced = false) override;

    // Sell building
    bool Sell();

    // Repair building
    bool StartRepair();
    bool StopRepair();

    // Capture building
    bool Capture(HousesType newOwner);

    //-----------------------------------------------------------------------
    // Animation
    //-----------------------------------------------------------------------

    // Get current shape number for rendering
    int ShapeNumber() const override;

    // Set building animation state
    void SetBState(BStateType state);

    // Update animation
    void UpdateAnimation();

    //-----------------------------------------------------------------------
    // Mission handlers
    //-----------------------------------------------------------------------
    virtual int MissionAttack() override;
    virtual int MissionGuard() override;
    virtual int MissionConstruction() override;
    virtual int MissionDeconstruction() override;
    virtual int MissionRepair() override;

    //-----------------------------------------------------------------------
    // AI processing
    //-----------------------------------------------------------------------
    virtual void AI() override;

    //-----------------------------------------------------------------------
    // Rendering
    //-----------------------------------------------------------------------
    virtual void DrawIt(int x, int y, int window) const override;

    //-----------------------------------------------------------------------
    // Limbo/Unlimbo
    //-----------------------------------------------------------------------
    virtual bool Limbo() override;
    virtual bool Unlimbo(int32_t coord, DirType facing = DirType::N) override;

private:
    // Process factory production
    void FactoryAI();

    // Process turret aiming
    void TurretAI();

    // Check for repair completion
    void RepairAI();

    // Fire animation
    void DoFireAnimation();
};

//===========================================================================
// Building Pool - Global container for all buildings
//===========================================================================
extern ObjectPool<BuildingClass, BUILDING_MAX> Buildings;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Create new building of given type
 */
BuildingClass* CreateBuilding(BuildingType type, HousesType house, CELL cell);

/**
 * Check if building can be placed at location
 */
bool CanPlaceBuildingAt(BuildingType type, CELL cell, HousesType house);

#endif // GAME_BUILDING_H
