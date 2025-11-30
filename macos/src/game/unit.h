/**
 * Red Alert macOS Port - Unit Class (Vehicles)
 *
 * Ground vehicles that can move, fight, and transport.
 * In original: DriveClass -> UnitClass
 * We simplify: FootClass -> UnitClass (combining Drive functionality)
 *
 * Based on original UNIT.H and DRIVE.H
 */

#ifndef GAME_UNIT_H
#define GAME_UNIT_H

#include "object.h"
#include "unit_types.h"

// Maximum units in game
constexpr int UNIT_MAX = 500;

// Track animation states
constexpr int TRACK_STAGES = 8;     // Track animation frames

// Harvester states
enum class HarvestState : int8_t {
    IDLE = 0,
    APPROACH,       // Approaching ore field
    HARVESTING,     // Actively harvesting
    DUMPING,        // Dumping ore at refinery
    RETURN          // Returning to refinery
};

/**
 * UnitClass - Ground vehicles
 *
 * Units are ground vehicles that:
 * - Use track or wheel movement
 * - Can have turrets that rotate independently
 * - Some can transport infantry (APC)
 * - Some harvest ore (Harvester)
 * - Can be deployed (MCV -> Construction Yard)
 */
class UnitClass : public FootClass {
public:
    //-----------------------------------------------------------------------
    // Unit-specific state
    //-----------------------------------------------------------------------

    // Type reference
    UnitType type_;

    // Track animation
    int8_t trackStage_;         // Current track animation frame
    int8_t trackCounter_;       // Track animation timer

    // Turret control (separate from body facing)
    bool isTurretRotating_;     // Turret is turning
    DirType turretDesiredFacing_;

    // Flag tracking
    bool isHarvesting_;         // Currently harvesting ore
    bool isDeploying_;          // MCV deploying
    bool isReturning_;          // Returning to base
    bool hasParachute_;         // Dropped via paradrop (temporary immunity)

    // Harvester state
    HarvestState harvestState_;
    int16_t oreLoad_;           // Current ore load (0-100)
    int16_t gemsLoad_;          // Current gems load (0-100)
    int16_t harvestTimer_;      // Time until next ore pickup

    // Tilt (for visual effect on slopes)
    int8_t tiltX_;              // X axis tilt
    int8_t tiltY_;              // Y axis tilt

    // Transport passengers (for APC)
    int8_t passengerCount_;

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    UnitClass();
    UnitClass(UnitType type, HousesType house);
    virtual ~UnitClass() = default;

    // Initialize from type
    void Init(UnitType type, HousesType house);

    //-----------------------------------------------------------------------
    // Type queries
    //-----------------------------------------------------------------------

    const UnitTypeData* TypeClass() const;
    const char* Name() const override;

    // Is this a harvester?
    bool IsHarvester() const;

    // Can this unit crush infantry?
    bool IsCrusher() const;

    // Does this unit have a turret?
    bool HasTurret() const;

    // Is this an APC?
    bool IsTransport() const;

    // Is this an MCV?
    bool IsMCV() const;

    //-----------------------------------------------------------------------
    // Position and movement
    //-----------------------------------------------------------------------

    // Override movement for tracked/wheeled handling
    virtual bool StartDrive(int32_t destination) override;
    virtual bool StopDrive() override;
    virtual MoveType CanEnterCell(int16_t cell, FacingType facing = FacingType::NORTH) const override;
    virtual int TopSpeed() const override;

    // Track/wheel animation
    void AnimateTracks();

    // Tilt based on terrain
    void CalculateTilt();

    //-----------------------------------------------------------------------
    // Turret control
    //-----------------------------------------------------------------------

    // Set turret facing target
    void SetTurretFacing(DirType facing);

    // Get turret facing for rendering
    int TurretShapeOffset() const;

    // Update turret rotation
    void UpdateTurret();

    //-----------------------------------------------------------------------
    // Combat
    //-----------------------------------------------------------------------

    virtual bool CanFire() const override;
    virtual int WeaponRange(int weapon = 0) const override;
    virtual int RearmTime(int weapon = 0) const override;
    virtual ResultType TakeDamage(int& damage, int distance, WarheadType warhead,
                                   TechnoClass* source = nullptr, bool forced = false) override;

    //-----------------------------------------------------------------------
    // Harvester operations
    //-----------------------------------------------------------------------

    // Start harvesting at current location
    bool StartHarvest();

    // Return to refinery
    bool ReturnToRefinery();

    // Dump ore at refinery
    int DumpOre();

    // Is harvester full?
    bool IsOreLoadFull() const { return oreLoad_ + gemsLoad_ >= 100; }

    // Harvester AI logic
    void HarvesterAI();

    //-----------------------------------------------------------------------
    // Transport operations
    //-----------------------------------------------------------------------

    // Load infantry into transport
    bool LoadPassenger(ObjectClass* passenger);

    // Unload all passengers
    bool UnloadPassengers();

    // Get current passenger count
    int PassengerCount() const { return passengerCount_; }

    //-----------------------------------------------------------------------
    // MCV operations
    //-----------------------------------------------------------------------

    // Deploy MCV into Construction Yard
    bool Deploy();

    // Can deploy at current location?
    bool CanDeploy() const;

    //-----------------------------------------------------------------------
    // Animation
    //-----------------------------------------------------------------------

    // Get current shape number for rendering
    int ShapeNumber() const override;

    //-----------------------------------------------------------------------
    // Mission handlers
    //-----------------------------------------------------------------------
    virtual int MissionAttack() override;
    virtual int MissionGuard() override;
    virtual int MissionMove() override;
    virtual int MissionHunt() override;
    virtual int MissionHarvest() override;
    virtual int MissionUnload() override;

    //-----------------------------------------------------------------------
    // AI processing
    //-----------------------------------------------------------------------
    virtual void AI() override;
    virtual void PerCellProcess(PCPType pcp) override;

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
    // Movement smoothing
    void SmoothMovement();

    // Check for infantry to crush
    void CheckCrush();
};

//===========================================================================
// Unit Pool - Global container for all units
//===========================================================================
extern ObjectPool<UnitClass, UNIT_MAX> Units;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Create new unit of given type
 */
UnitClass* CreateUnit(UnitType type, HousesType house, CELL cell);

#endif // GAME_UNIT_H
