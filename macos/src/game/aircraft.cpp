/**
 * Red Alert macOS Port - Aircraft Class Implementation
 *
 * Based on original AIRCRAFT.CPP and FLY.CPP (~8K lines combined)
 */

#include "aircraft.h"
#include "mapclass.h"
#include "cell.h"
#include <cmath>
#include <cstring>

//===========================================================================
// Global Aircraft Pool
//===========================================================================
ObjectPool<AircraftClass, AIRCRAFT_MAX> Aircraft;

//===========================================================================
// Construction
//===========================================================================

AircraftClass::AircraftClass()
    : FootClass(RTTIType::AIRCRAFT, 0)
    , type_(AircraftType::NONE)
    , flightState_(FlightState::GROUNDED)
    , altitude_(0)
    , targetAltitude_(0)
    , descentRate_(8)
    , landingStage_(0)
    , landingTarget_(0)
    , isReturning_(false)
    , isLanding_(false)
    , isFlying_(false)
    , hasAmmo_(true)
    , rotorFrame_(0)
    , rotorCounter_(0)
    , passengerCount_(0)
{
}

AircraftClass::AircraftClass(AircraftType type, HousesType house)
    : FootClass(RTTIType::AIRCRAFT, 0)
    , type_(AircraftType::NONE)
    , flightState_(FlightState::GROUNDED)
    , altitude_(0)
    , targetAltitude_(0)
    , descentRate_(8)
    , landingStage_(0)
    , landingTarget_(0)
    , isReturning_(false)
    , isLanding_(false)
    , isFlying_(false)
    , hasAmmo_(true)
    , rotorFrame_(0)
    , rotorCounter_(0)
    , passengerCount_(0)
{
    Init(type, house);
}

void AircraftClass::Init(AircraftType type, HousesType house) {
    type_ = type;
    SetHouse(house);

    const AircraftTypeData* typeData = TypeClass();
    if (typeData) {
        strength_ = typeData->strength;
        ammo_ = typeData->ammo;
        hasAmmo_ = (ammo_ > 0 || ammo_ == -1);
    }

    // Aircraft start grounded
    flightState_ = FlightState::GROUNDED;
    altitude_ = 0;
}

//===========================================================================
// Type Queries
//===========================================================================

const AircraftTypeData* AircraftClass::TypeClass() const {
    return GetAircraftType(type_);
}

const char* AircraftClass::Name() const {
    const AircraftTypeData* typeData = TypeClass();
    if (typeData) {
        return typeData->iniName;
    }
    return "AIRCRAFT";
}

bool AircraftClass::IsHelicopter() const {
    const AircraftTypeData* typeData = TypeClass();
    return typeData ? !typeData->isFixedWing : false;
}

bool AircraftClass::IsFixedWing() const {
    const AircraftTypeData* typeData = TypeClass();
    return typeData ? typeData->isFixedWing : false;
}

bool AircraftClass::IsTransport() const {
    const AircraftTypeData* typeData = TypeClass();
    return typeData ? (typeData->passengers > 0) : false;
}

bool AircraftClass::CanHover() const {
    const AircraftTypeData* typeData = TypeClass();
    return typeData ? typeData->canHover : false;
}

//===========================================================================
// Position and Movement
//===========================================================================

int32_t AircraftClass::CenterCoord() const {
    // Include altitude in coordinate for proper targeting
    // Altitude stored separately, but coord can be used with height_
    return coord_;
}

bool AircraftClass::StartDrive(int32_t destination) {
    // Aircraft must be airborne to move
    if (!IsAirborne()) {
        TakeOff();
    }

    if (!FootClass::StartDrive(destination)) return false;

    flightState_ = FlightState::FLYING;
    return true;
}

bool AircraftClass::StopDrive() {
    if (!FootClass::StopDrive()) return false;

    // Helicopters hover when stopped
    if (CanHover() && IsAirborne()) {
        flightState_ = FlightState::HOVERING;
    }

    return true;
}

MoveType AircraftClass::CanEnterCell(int16_t cell, FacingType facing) const {
    // Aircraft can fly over anything
    (void)cell;
    (void)facing;
    return MoveType::OK;
}

int AircraftClass::TopSpeed() const {
    const AircraftTypeData* typeData = TypeClass();
    if (!typeData) return 80;

    int baseSpeed = typeData->speed;

    // Fixed-wing aircraft are faster
    if (IsFixedWing()) {
        return baseSpeed;
    }

    return baseSpeed;
}

void AircraftClass::SetAltitude(int altitude) {
    targetAltitude_ = static_cast<int16_t>(altitude);
}

//===========================================================================
// Flight Control
//===========================================================================

bool AircraftClass::TakeOff() {
    if (IsAirborne()) return false;

    flightState_ = FlightState::TAKING_OFF;
    targetAltitude_ = FLIGHT_LEVEL;
    landingStage_ = 0;

    return true;
}

bool AircraftClass::Land(uint32_t target) {
    if (!IsAirborne()) return false;

    landingTarget_ = target;
    flightState_ = FlightState::LANDING;
    targetAltitude_ = 0;
    isLanding_ = true;
    landingStage_ = 0;

    return true;
}

bool AircraftClass::ReturnToBase() {
    if (isReturning_) return false;

    isReturning_ = true;
    uint32_t base = FindLandingSite();
    if (base != 0) {
        navCom_ = base;
        isNewNavCom_ = true;
        AssignMission(MissionType::RETURN);
    }

    return true;
}

uint32_t AircraftClass::FindLandingSite() const {
    // Would search for appropriate helipad or airstrip
    // For now, return 0 (would need building system integration)
    return 0;
}

void AircraftClass::ProcessFlight() {
    // Update altitude
    UpdateAltitude();

    // Update position based on flight state
    switch (flightState_) {
        case FlightState::GROUNDED:
            // No movement while grounded
            break;

        case FlightState::TAKING_OFF:
            ProcessTakeoff();
            break;

        case FlightState::FLYING:
            UpdateFlightPath();
            break;

        case FlightState::HOVERING:
            // Helicopters hover - slight wobble
            break;

        case FlightState::LANDING:
            ProcessLanding();
            break;

        case FlightState::ATTACKING:
            // Attack run - handled by mission
            break;
    }
}

void AircraftClass::UpdateAltitude() {
    if (altitude_ < targetAltitude_) {
        altitude_ += descentRate_;
        if (altitude_ > targetAltitude_) {
            altitude_ = targetAltitude_;
        }
    } else if (altitude_ > targetAltitude_) {
        altitude_ -= descentRate_;
        if (altitude_ < targetAltitude_) {
            altitude_ = targetAltitude_;
        }
    }

    // Update height member
    height_ = altitude_;
}

void AircraftClass::UpdateFlightPath() {
    if (!isDriving_) return;

    // Movement handled by FootClass
    // Aircraft move faster than ground units
}

void AircraftClass::ProcessTakeoff() {
    landingStage_++;

    if (altitude_ >= FLIGHT_LEVEL) {
        flightState_ = CanHover() ? FlightState::HOVERING : FlightState::FLYING;
        isFlying_ = true;
    }
}

void AircraftClass::ProcessLanding() {
    landingStage_++;

    if (altitude_ <= 0) {
        altitude_ = 0;
        flightState_ = FlightState::GROUNDED;
        isFlying_ = false;
        isLanding_ = false;
        landingStage_ = 0;

        // Rearm at landing site
        Rearm();
    }
}

//===========================================================================
// Combat
//===========================================================================

bool AircraftClass::CanFire() const {
    if (!TechnoClass::CanFire()) return false;

    // Must have ammo
    if (!hasAmmo_) return false;

    // Must be airborne (except for some weapons)
    if (!IsAirborne()) return false;

    return true;
}

int AircraftClass::WeaponRange(int weapon) const {
    const AircraftTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Helicopters have shorter range
    if (IsHelicopter()) {
        return 4 * LEPTONS_PER_CELL;
    }

    // Fixed-wing attack range
    return 6 * LEPTONS_PER_CELL;
}

int AircraftClass::RearmTime(int weapon) const {
    const AircraftTypeData* typeData = TypeClass();
    if (!typeData) return 60;

    return 30;  // Aircraft fire quickly
}

ResultType AircraftClass::TakeDamage(int& damage, int distance,
                                      WarheadType warhead,
                                      TechnoClass* source, bool forced) {
    // Aircraft are vulnerable to AA
    if (warhead == WarheadType::AP) {
        damage = damage * 3 / 2;  // Extra damage from AP (AA weapons)
    }

    return FootClass::TakeDamage(damage, distance, warhead, source, forced);
}

bool AircraftClass::Rearm() {
    const AircraftTypeData* typeData = TypeClass();
    if (!typeData) return false;

    ammo_ = typeData->ammo;
    hasAmmo_ = (ammo_ > 0 || ammo_ == -1);
    return true;
}

//===========================================================================
// Transport Operations
//===========================================================================

bool AircraftClass::LoadPassenger(ObjectClass* passenger) {
    if (!IsTransport()) return false;
    if (passenger == nullptr) return false;

    const AircraftTypeData* typeData = TypeClass();
    if (!typeData) return false;

    if (passengerCount_ >= typeData->passengers) {
        return false;  // Full
    }

    passengerCount_++;
    passenger->Limbo();

    return true;
}

bool AircraftClass::UnloadPassengers() {
    if (!IsTransport()) return false;
    if (passengerCount_ == 0) return false;
    if (IsAirborne()) return false;  // Must land first

    // Would unload passengers one by one
    passengerCount_ = 0;

    return true;
}

//===========================================================================
// Animation
//===========================================================================

int AircraftClass::ShapeNumber() const {
    const AircraftTypeData* typeData = TypeClass();
    if (!typeData) return 0;

    // Calculate frame from facing
    int stages = typeData->rotationStages;
    int facing = static_cast<int>(bodyFacing_) / (256 / stages);
    if (facing >= stages) facing = 0;

    return facing;
}

void AircraftClass::AnimateRotor() {
    const AircraftTypeData* typeData = TypeClass();
    if (!typeData || !typeData->hasRotor) return;

    rotorCounter_++;
    if (rotorCounter_ >= 2) {
        rotorCounter_ = 0;
        rotorFrame_++;
        if (rotorFrame_ >= 4) {
            rotorFrame_ = 0;
        }
    }
}

//===========================================================================
// Mission Handlers
//===========================================================================

int AircraftClass::MissionAttack() {
    // Ensure airborne
    if (!IsAirborne()) {
        TakeOff();
        return 15;
    }

    // Face target
    if (tarCom_ != 0) {
        uint8_t dir = DirectionTo(tarCom_);
        bodyFacingTarget_ = static_cast<DirType>(dir);
    }

    flightState_ = FlightState::ATTACKING;

    // After attack, check ammo
    if (ammo_ == 0) {
        ReturnToBase();
    }

    return 15;
}

int AircraftClass::MissionGuard() {
    // Helicopters hover
    if (CanHover() && IsAirborne()) {
        flightState_ = FlightState::HOVERING;
    }

    return 30;
}

int AircraftClass::MissionMove() {
    // Ensure airborne
    if (!IsAirborne()) {
        TakeOff();
    }

    flightState_ = FlightState::FLYING;

    // Check if reached destination
    if (!isDriving_ && navCom_ == 0) {
        if (CanHover()) {
            flightState_ = FlightState::HOVERING;
        }
        SetMission(MissionType::GUARD);
    }

    return 15;
}

int AircraftClass::MissionHunt() {
    // Ensure airborne
    if (!IsAirborne()) {
        TakeOff();
        return 15;
    }

    // Actively seek enemies
    flightState_ = FlightState::FLYING;

    return 60;
}

int AircraftClass::MissionUnload() {
    if (!IsTransport()) {
        SetMission(MissionType::GUARD);
        return 15;
    }

    // Must land first
    if (IsAirborne()) {
        Land();
        return 15;
    }

    // Unload passengers
    UnloadPassengers();
    SetMission(MissionType::GUARD);

    return 30;
}

int AircraftClass::MissionReturn() {
    // Return to base
    if (!IsAirborne()) {
        // Already landed, mission complete
        isReturning_ = false;
        SetMission(MissionType::GUARD);
        return 15;
    }

    // Check if at landing site
    if (navCom_ == 0 || landingTarget_ != 0) {
        Land(landingTarget_);
    }

    return 15;
}

int AircraftClass::MissionEnter() {
    // Land at target building
    if (IsAirborne()) {
        Land(landingTarget_);
    }
    return 15;
}

//===========================================================================
// AI Processing
//===========================================================================

void AircraftClass::AI() {
    FootClass::AI();

    // Process flight physics
    ProcessFlight();

    // Animate rotor for helicopters
    if (IsHelicopter()) {
        AnimateRotor();
    }

    // Check if out of ammo and should return
    const AircraftTypeData* typeData = TypeClass();
    if (typeData && typeData->ammo > 0 && ammo_ == 0 && !isReturning_) {
        ReturnToBase();
    }
}

//===========================================================================
// Rendering
//===========================================================================

void AircraftClass::DrawIt(int x, int y, int window) const {
    // Would render aircraft sprite at given position
    // Need to account for altitude in rendering
    (void)x;
    (void)y;
    (void)window;
}

//===========================================================================
// Limbo/Unlimbo
//===========================================================================

bool AircraftClass::Limbo() {
    return FootClass::Limbo();
}

bool AircraftClass::Unlimbo(int32_t coord, DirType facing) {
    if (!FootClass::Unlimbo(coord, facing)) return false;

    // Set initial facing
    bodyFacing_ = facing;
    bodyFacingTarget_ = facing;

    // Start grounded
    altitude_ = 0;
    flightState_ = FlightState::GROUNDED;

    return true;
}

//===========================================================================
// Helper Functions
//===========================================================================

AircraftClass* CreateAircraft(AircraftType type, HousesType house, CELL cell) {
    AircraftClass* aircraft = Aircraft.Allocate();
    if (aircraft) {
        aircraft->Init(type, house);
        aircraft->Unlimbo(Cell_Coord(cell), DirType::S);
    }
    return aircraft;
}
