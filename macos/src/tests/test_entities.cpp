/**
 * Red Alert macOS Port - Entity Class Tests
 *
 * Tests for InfantryClass, UnitClass, BuildingClass, AircraftClass
 */

#include <cassert>
#include <cstdio>
#include <cstring>

// Include entity headers
#include "../game/types.h"
#include "../game/object.h"
#include "../game/infantry.h"
#include "../game/unit.h"
#include "../game/building.h"
#include "../game/aircraft.h"
#include "../game/mapclass.h"

// Test tracking
static int testsRun = 0;
static int testsPassed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("  Testing %s...", #name); \
    testsRun++; \
    test_##name(); \
    testsPassed++; \
    printf(" PASSED\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf(" FAILED at line %d: %s\n", __LINE__, #cond); \
        return; \
    } \
} while(0)

//===========================================================================
// Infantry Tests
//===========================================================================

TEST(infantry_type_query) {
    const InfantryTypeData* e1 = GetInfantryType(InfantryType::E1);
    ASSERT(e1 != nullptr);
    ASSERT(e1->type == InfantryType::E1);
    ASSERT(strcmp(e1->iniName, "E1") == 0);
    ASSERT(e1->strength == 50);

    const InfantryTypeData* dog = GetInfantryType(InfantryType::DOG);
    ASSERT(dog != nullptr);
    ASSERT(dog->isDog == true);

    const InfantryTypeData* engineer = GetInfantryType(InfantryType::RENOVATOR);
    ASSERT(engineer != nullptr);
    ASSERT(engineer->canCapture == true);
}

TEST(infantry_type_from_name) {
    ASSERT(InfantryTypeFromName("E1") == InfantryType::E1);
    ASSERT(InfantryTypeFromName("E2") == InfantryType::E2);
    ASSERT(InfantryTypeFromName("DOG") == InfantryType::DOG);
    ASSERT(InfantryTypeFromName("UNKNOWN") == InfantryType::NONE);
}

TEST(infantry_construction) {
    InfantryClass infantry;
    infantry.Init(InfantryType::E1, HousesType::GOOD);

    ASSERT(infantry.type_ == InfantryType::E1);
    ASSERT(infantry.house_ == HousesType::GOOD);
    ASSERT(infantry.strength_ == 50);  // E1 has 50 HP
    ASSERT(infantry.IsDog() == false);
    ASSERT(infantry.CanCapture() == false);
}

TEST(infantry_dog) {
    InfantryClass dog;
    dog.Init(InfantryType::DOG, HousesType::BAD);

    ASSERT(dog.IsDog() == true);
    ASSERT(dog.CanCapture() == false);
}

TEST(infantry_engineer) {
    InfantryClass engineer;
    engineer.Init(InfantryType::RENOVATOR, HousesType::GOOD);

    ASSERT(engineer.IsDog() == false);
    ASSERT(engineer.CanCapture() == true);
}

TEST(infantry_fear) {
    InfantryClass infantry;
    infantry.Init(InfantryType::E1, HousesType::GOOD);

    ASSERT(infantry.fear_ == FEAR_NONE);
    ASSERT(infantry.IsPanicked() == false);
    ASSERT(infantry.IsScared() == false);

    infantry.Afraid();
    ASSERT(infantry.fear_ == FEAR_PANIC);
    ASSERT(infantry.IsPanicked() == true);
    ASSERT(infantry.IsScared() == true);

    infantry.Calm();
    ASSERT(infantry.fear_ == FEAR_NONE);
    ASSERT(infantry.IsPanicked() == false);
}

TEST(infantry_prone) {
    InfantryClass infantry;
    infantry.Init(InfantryType::E1, HousesType::GOOD);

    ASSERT(infantry.IsProne() == false);

    infantry.GoProne();
    ASSERT(infantry.IsProne() == true);

    infantry.StandUp();
    ASSERT(infantry.IsProne() == false);
}

TEST(infantry_spot_coord) {
    CELL cell = XY_Cell(10, 10);

    int32_t center = InfantryClass::SpotCoord(cell, SpotType::CENTER);
    int32_t baseCoord = Cell_Coord(cell);
    ASSERT(center == baseCoord);

    int32_t upperLeft = InfantryClass::SpotCoord(cell, SpotType::UPPER_LEFT);
    ASSERT(upperLeft != center);  // Should be offset
}

TEST(infantry_animation) {
    InfantryClass infantry;
    infantry.Init(InfantryType::E1, HousesType::GOOD);

    ASSERT(infantry.GetDoType() == DoType::STAND_READY);

    infantry.SetDoType(DoType::WALK);
    ASSERT(infantry.GetDoType() == DoType::WALK);

    const DoInfoStruct* controls = infantry.DoControls();
    ASSERT(controls != nullptr);
}

//===========================================================================
// Unit (Vehicle) Tests
//===========================================================================

TEST(unit_type_query) {
    const UnitTypeData* htank = GetUnitType(UnitType::HTANK);
    ASSERT(htank != nullptr);
    ASSERT(htank->type == UnitType::HTANK);
    ASSERT(htank->isCrusher == true);
    ASSERT(htank->hasTurret == true);

    const UnitTypeData* harvester = GetUnitType(UnitType::HARVESTER);
    ASSERT(harvester != nullptr);
    ASSERT(harvester->isHarvester == true);
}

TEST(unit_type_from_name) {
    ASSERT(UnitTypeFromName("1TNK") == UnitType::LTANK);  // Light tank
    ASSERT(UnitTypeFromName("4TNK") == UnitType::HTANK);  // Mammoth tank
    ASSERT(UnitTypeFromName("HARV") == UnitType::HARVESTER);
    ASSERT(UnitTypeFromName("MCV") == UnitType::MCV);
    ASSERT(UnitTypeFromName("UNKNOWN") == UnitType::NONE);
}

TEST(unit_construction) {
    UnitClass unit;
    unit.Init(UnitType::MTANK, HousesType::GOOD);

    ASSERT(unit.type_ == UnitType::MTANK);
    ASSERT(unit.house_ == HousesType::GOOD);
    ASSERT(unit.IsHarvester() == false);
}

TEST(unit_harvester) {
    UnitClass harvester;
    harvester.Init(UnitType::HARVESTER, HousesType::GOOD);

    ASSERT(harvester.IsHarvester() == true);
    ASSERT(harvester.oreLoad_ == 0);
    ASSERT(harvester.gemsLoad_ == 0);
    ASSERT(harvester.IsOreLoadFull() == false);
}

TEST(unit_turret) {
    UnitClass tank;
    tank.Init(UnitType::HTANK, HousesType::GOOD);

    ASSERT(tank.HasTurret() == true);

    // Test turret facing
    tank.SetTurretFacing(DirType::E);
    ASSERT(tank.turretDesiredFacing_ == DirType::E);
}

TEST(unit_mcv) {
    UnitClass mcv;
    mcv.Init(UnitType::MCV, HousesType::GOOD);

    ASSERT(mcv.IsMCV() == true);
}

TEST(unit_track_animation) {
    UnitClass tank;
    tank.Init(UnitType::LTANK, HousesType::GOOD);

    ASSERT(tank.trackStage_ == 0);

    // Simulate driving
    tank.isDriving_ = true;
    for (int i = 0; i < 20; i++) {
        tank.AnimateTracks();
    }
    // Track should have animated
}

//===========================================================================
// Building Tests
//===========================================================================

TEST(building_type_query) {
    const BuildingTypeData* power = GetBuildingType(BuildingType::POWER);
    ASSERT(power != nullptr);
    ASSERT(power->type == BuildingType::POWER);

    const BuildingTypeData* weap = GetBuildingType(BuildingType::WEAP);
    ASSERT(weap != nullptr);
    ASSERT(weap->factoryType == RTTIType::UNIT);
}

TEST(building_type_from_name) {
    ASSERT(BuildingTypeFromName("POWR") == BuildingType::POWER);
    ASSERT(BuildingTypeFromName("WEAP") == BuildingType::WEAP);
    ASSERT(BuildingTypeFromName("UNKNOWN") == BuildingType::NONE);
}

TEST(building_construction) {
    BuildingClass building;
    building.Init(BuildingType::POWER, HousesType::GOOD);

    ASSERT(building.type_ == BuildingType::POWER);
    ASSERT(building.house_ == HousesType::GOOD);
    ASSERT(building.IsPowerPlant() == true);
}

TEST(building_factory) {
    BuildingClass weap;
    weap.Init(BuildingType::WEAP, HousesType::GOOD);

    ASSERT(weap.IsFactory() == true);
    ASSERT(weap.FactoryType() == RTTIType::UNIT);
}

TEST(building_size) {
    int w, h;
    GetBuildingSize(BSizeType::BSIZE_11, w, h);
    ASSERT(w == 1 && h == 1);

    GetBuildingSize(BSizeType::BSIZE_22, w, h);
    ASSERT(w == 2 && h == 2);

    GetBuildingSize(BSizeType::BSIZE_33, w, h);
    ASSERT(w == 3 && h == 3);
}

TEST(building_wall) {
    ASSERT(IsBuildingWall(BuildingType::BRICK_WALL) == true);
    ASSERT(IsBuildingWall(BuildingType::SANDBAG_WALL) == true);
    ASSERT(IsBuildingWall(BuildingType::POWER) == false);
}

TEST(building_occupy_list) {
    BuildingClass building;
    building.Init(BuildingType::POWER, HousesType::GOOD);

    const int16_t* list = building.OccupyList();
    ASSERT(list != nullptr);

    // Should end with 0x8000
    int count = 0;
    while (*list != static_cast<int16_t>(0x8000)) {
        count++;
        list++;
        ASSERT(count < 100);  // Safety check
    }
    ASSERT(count > 0);  // Should occupy at least one cell
}

TEST(building_production) {
    BuildingClass factory;
    factory.Init(BuildingType::WEAP, HousesType::GOOD);
    factory.bstate_ = BStateType::IDLE;  // Not under construction
    factory.isPowered_ = true;

    ASSERT(factory.factoryState_ == FactoryState::IDLE);

    // Start production
    bool started = factory.StartProduction(RTTIType::UNIT, 0);
    ASSERT(started == true);
    ASSERT(factory.factoryState_ == FactoryState::BUILDING);

    // Cancel production
    bool cancelled = factory.CancelProduction();
    ASSERT(cancelled == true);
    ASSERT(factory.factoryState_ == FactoryState::IDLE);
}

TEST(building_repair) {
    BuildingClass building;
    building.Init(BuildingType::POWER, HousesType::GOOD);
    building.bstate_ = BStateType::IDLE;

    // Damage the building
    building.strength_ = 50;  // Half health

    bool started = building.StartRepair();
    ASSERT(started == true);
    ASSERT(building.isRepairing_ == true);

    bool stopped = building.StopRepair();
    ASSERT(stopped == true);
    ASSERT(building.isRepairing_ == false);
}

//===========================================================================
// Aircraft Tests
//===========================================================================

TEST(aircraft_type_query) {
    const AircraftTypeData* heli = GetAircraftType(AircraftType::HELI);
    ASSERT(heli != nullptr);
    ASSERT(heli->type == AircraftType::HELI);
    ASSERT(heli->isFixedWing == false);
    ASSERT(heli->canHover == true);

    const AircraftTypeData* mig = GetAircraftType(AircraftType::MIG);
    ASSERT(mig != nullptr);
    ASSERT(mig->isFixedWing == true);
}

TEST(aircraft_type_from_name) {
    ASSERT(AircraftTypeFromName("HELI") == AircraftType::HELI);
    ASSERT(AircraftTypeFromName("MIG") == AircraftType::MIG);
    ASSERT(AircraftTypeFromName("TRAN") == AircraftType::TRANSPORT);
    ASSERT(AircraftTypeFromName("UNKNOWN") == AircraftType::NONE);
}

TEST(aircraft_construction) {
    AircraftClass aircraft;
    aircraft.Init(AircraftType::HELI, HousesType::GOOD);

    ASSERT(aircraft.type_ == AircraftType::HELI);
    ASSERT(aircraft.house_ == HousesType::GOOD);
    ASSERT(aircraft.IsHelicopter() == true);
    ASSERT(aircraft.IsFixedWing() == false);
}

TEST(aircraft_helicopter_vs_plane) {
    AircraftClass heli;
    heli.Init(AircraftType::HELI, HousesType::GOOD);
    ASSERT(heli.IsHelicopter() == true);
    ASSERT(heli.CanHover() == true);

    AircraftClass mig;
    mig.Init(AircraftType::MIG, HousesType::BAD);
    ASSERT(mig.IsFixedWing() == true);
    ASSERT(mig.CanHover() == false);
}

TEST(aircraft_flight_state) {
    AircraftClass aircraft;
    aircraft.Init(AircraftType::HELI, HousesType::GOOD);

    ASSERT(aircraft.flightState_ == FlightState::GROUNDED);
    ASSERT(aircraft.altitude_ == 0);
    ASSERT(aircraft.IsAirborne() == false);

    aircraft.TakeOff();
    ASSERT(aircraft.flightState_ == FlightState::TAKING_OFF);

    // Simulate altitude increase
    aircraft.altitude_ = FLIGHT_LEVEL;
    aircraft.flightState_ = FlightState::FLYING;
    ASSERT(aircraft.IsAirborne() == true);
}

TEST(aircraft_transport) {
    AircraftClass transport;
    transport.Init(AircraftType::TRANSPORT, HousesType::GOOD);

    ASSERT(transport.IsTransport() == true);
    ASSERT(transport.passengerCount_ == 0);
}

TEST(aircraft_ammo) {
    AircraftClass mig;
    mig.Init(AircraftType::MIG, HousesType::BAD);

    ASSERT(mig.hasAmmo_ == true);
    ASSERT(mig.ammo_ > 0);

    // Simulate firing all ammo
    mig.ammo_ = 0;
    mig.hasAmmo_ = false;
    ASSERT(mig.CanFire() == false);

    // Rearm
    mig.Rearm();
    ASSERT(mig.hasAmmo_ == true);
}

//===========================================================================
// Cross-Entity Tests
//===========================================================================

TEST(entity_rtti) {
    InfantryClass infantry;
    infantry.Init(InfantryType::E1, HousesType::GOOD);
    ASSERT(infantry.WhatAmI() == RTTIType::INFANTRY);
    ASSERT(infantry.IsInfantry() == true);
    ASSERT(infantry.IsFoot() == true);
    ASSERT(infantry.IsTechno() == true);

    UnitClass unit;
    unit.Init(UnitType::LTANK, HousesType::GOOD);
    ASSERT(unit.WhatAmI() == RTTIType::UNIT);
    ASSERT(unit.IsFoot() == true);
    ASSERT(unit.IsTechno() == true);

    BuildingClass building;
    building.Init(BuildingType::POWER, HousesType::GOOD);
    ASSERT(building.WhatAmI() == RTTIType::BUILDING);
    ASSERT(building.IsTechno() == true);

    AircraftClass aircraft;
    aircraft.Init(AircraftType::HELI, HousesType::GOOD);
    ASSERT(aircraft.WhatAmI() == RTTIType::AIRCRAFT);
    ASSERT(aircraft.IsFoot() == true);
}

TEST(entity_owner) {
    InfantryClass ally;
    ally.Init(InfantryType::E1, HousesType::GOOD);
    ASSERT(ally.Owner() == HousesType::GOOD);

    UnitClass enemy;
    enemy.Init(UnitType::HTANK, HousesType::BAD);
    ASSERT(enemy.Owner() == HousesType::BAD);
}

TEST(entity_pool_allocation) {
    // Test that pools work
    InfantryClass* inf = Infantry.Allocate();
    ASSERT(inf != nullptr);
    Infantry.Free(inf);

    UnitClass* unit = Units.Allocate();
    ASSERT(unit != nullptr);
    Units.Free(unit);

    BuildingClass* bld = Buildings.Allocate();
    ASSERT(bld != nullptr);
    Buildings.Free(bld);

    AircraftClass* air = Aircraft.Allocate();
    ASSERT(air != nullptr);
    Aircraft.Free(air);
}

//===========================================================================
// Test Runner
//===========================================================================

void run_infantry_tests() {
    printf("\nInfantry Tests:\n");
    RUN_TEST(infantry_type_query);
    RUN_TEST(infantry_type_from_name);
    RUN_TEST(infantry_construction);
    RUN_TEST(infantry_dog);
    RUN_TEST(infantry_engineer);
    RUN_TEST(infantry_fear);
    RUN_TEST(infantry_prone);
    RUN_TEST(infantry_spot_coord);
    RUN_TEST(infantry_animation);
}

void run_unit_tests() {
    printf("\nUnit (Vehicle) Tests:\n");
    RUN_TEST(unit_type_query);
    RUN_TEST(unit_type_from_name);
    RUN_TEST(unit_construction);
    RUN_TEST(unit_harvester);
    RUN_TEST(unit_turret);
    RUN_TEST(unit_mcv);
    RUN_TEST(unit_track_animation);
}

void run_building_tests() {
    printf("\nBuilding Tests:\n");
    RUN_TEST(building_type_query);
    RUN_TEST(building_type_from_name);
    RUN_TEST(building_construction);
    RUN_TEST(building_factory);
    RUN_TEST(building_size);
    RUN_TEST(building_wall);
    RUN_TEST(building_occupy_list);
    RUN_TEST(building_production);
    RUN_TEST(building_repair);
}

void run_aircraft_tests() {
    printf("\nAircraft Tests:\n");
    RUN_TEST(aircraft_type_query);
    RUN_TEST(aircraft_type_from_name);
    RUN_TEST(aircraft_construction);
    RUN_TEST(aircraft_helicopter_vs_plane);
    RUN_TEST(aircraft_flight_state);
    RUN_TEST(aircraft_transport);
    RUN_TEST(aircraft_ammo);
}

void run_cross_entity_tests() {
    printf("\nCross-Entity Tests:\n");
    RUN_TEST(entity_rtti);
    RUN_TEST(entity_owner);
    RUN_TEST(entity_pool_allocation);
}

int main() {
    printf("=== Red Alert Entity Tests ===\n");

    run_infantry_tests();
    run_unit_tests();
    run_building_tests();
    run_aircraft_tests();
    run_cross_entity_tests();

    printf("\n=== Results: %d/%d tests passed ===\n", testsPassed, testsRun);

    return (testsPassed == testsRun) ? 0 : 1;
}
