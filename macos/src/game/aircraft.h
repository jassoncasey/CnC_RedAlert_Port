/**
 * Red Alert macOS Port - Aircraft Class
 *
 * Flying units that can attack, transport, and scout.
 * In original: FootClass + FlyClass -> AircraftClass
 * We simplify: FootClass -> AircraftClass (with flight logic)
 *
 * Based on original AIRCRAFT.H and FLY.H
 */

#ifndef GAME_AIRCRAFT_H
#define GAME_AIRCRAFT_H

#include "object.h"
#include "aircraft_types.h"

// Maximum aircraft in game
constexpr int AIRCRAFT_MAX = 100;

// Flight level (leptons above ground)
constexpr int FLIGHT_LEVEL = 256;

// Landing/takeoff stages
constexpr int LANDING_STAGES = 8;

// Flight state
enum class FlightState : int8_t {
    GROUNDED = 0,   // On ground
    TAKING_OFF,     // Taking off
    FLYING,         // In flight
    HOVERING,       // Hovering in place
    LANDING,        // Landing
    ATTACKING       // Attack run
};

/**
 * AircraftClass - Flying units
 *
 * Aircraft:
 * - Move in 3D space (height above ground)
 * - Can take off and land at appropriate structures
 * - Fixed-wing planes fly in patterns
 * - Helicopters can hover and strafe
 * - Some can transport infantry
 */
class AircraftClass : public FootClass {
public:
    //-----------------------------------------------------------------------
    // Aircraft-specific state
    //-----------------------------------------------------------------------

    // Type reference
    AircraftType type_;

    // Flight state
    FlightState flightState_;
    int16_t altitude_;          // Current altitude (leptons)
    int16_t targetAltitude_;    // Target altitude
    int16_t descentRate_;       // Rate of descent/ascent

    // Landing/takeoff
    int8_t landingStage_;       // Animation stage for landing
    uint32_t landingTarget_;    // Building/cell we're landing at

    // Flight control
    bool isReturning_;          // Returning to base
    bool isLanding_;            // In landing sequence
    bool isFlying_;             // Currently airborne
    bool hasAmmo_;              // Has ammunition remaining

    // Rotor animation (helicopters)
    int8_t rotorFrame_;         // Current rotor frame
    int8_t rotorCounter_;       // Animation timer

    // Passengers (transports)
    int8_t passengerCount_;

    //-----------------------------------------------------------------------
    // Construction
    //-----------------------------------------------------------------------
    AircraftClass();
    AircraftClass(AircraftType type, HousesType house);
    virtual ~AircraftClass() = default;

    // Initialize from type
    void Init(AircraftType type, HousesType house);

    //-----------------------------------------------------------------------
    // Type queries
    //-----------------------------------------------------------------------

    const AircraftTypeData* TypeClass() const;
    const char* Name() const override;

    // Is this a helicopter?
    bool IsHelicopter() const;

    // Is this a fixed-wing plane?
    bool IsFixedWing() const;

    // Is this a transport?
    bool IsTransport() const;

    // Can this aircraft hover?
    bool CanHover() const;

    //-----------------------------------------------------------------------
    // Position and movement
    //-----------------------------------------------------------------------

    // Get center coordinate (accounts for altitude)
    virtual int32_t CenterCoord() const override;

    // Override for 3D movement
    virtual bool StartDrive(int32_t destination) override;
    virtual bool StopDrive() override;
    virtual MoveType CanEnterCell(int16_t cell, FacingType facing = FacingType::NORTH) const override;
    virtual int TopSpeed() const override;

    // Altitude control
    void SetAltitude(int altitude);
    int GetAltitude() const { return altitude_; }
    bool IsAirborne() const { return altitude_ > 0; }

    //-----------------------------------------------------------------------
    // Flight control
    //-----------------------------------------------------------------------

    // Take off from current location
    bool TakeOff();

    // Land at target
    bool Land(uint32_t target = 0);

    // Return to base (helipad/airstrip)
    bool ReturnToBase();

    // Find landing site
    uint32_t FindLandingSite() const;

    // Update flight physics
    void ProcessFlight();

    //-----------------------------------------------------------------------
    // Combat
    //-----------------------------------------------------------------------

    virtual bool CanFire() const override;
    virtual int WeaponRange(int weapon = 0) const override;
    virtual int RearmTime(int weapon = 0) const override;
    virtual ResultType TakeDamage(int& damage, int distance, WarheadType warhead,
                                   TechnoClass* source = nullptr, bool forced = false) override;

    // Reload ammunition
    bool Rearm();

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
    // Animation
    //-----------------------------------------------------------------------

    // Get current shape number for rendering
    int ShapeNumber() const override;

    // Rotor animation (helicopters)
    void AnimateRotor();

    //-----------------------------------------------------------------------
    // Mission handlers
    //-----------------------------------------------------------------------
    virtual int MissionAttack() override;
    virtual int MissionGuard() override;
    virtual int MissionMove() override;
    virtual int MissionHunt() override;
    virtual int MissionUnload() override;
    virtual int MissionReturn() override;
    virtual int MissionEnter() override;

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
    // Flight physics
    void UpdateAltitude();
    void UpdateFlightPath();

    // Landing sequence
    void ProcessLanding();
    void ProcessTakeoff();
};

//===========================================================================
// Aircraft Pool - Global container for all aircraft
//===========================================================================
extern ObjectPool<AircraftClass, AIRCRAFT_MAX> Aircraft;

//===========================================================================
// Helper Functions
//===========================================================================

/**
 * Create new aircraft of given type
 */
AircraftClass* CreateAircraft(AircraftType type, HousesType house, CELL cell);

#endif // GAME_AIRCRAFT_H
