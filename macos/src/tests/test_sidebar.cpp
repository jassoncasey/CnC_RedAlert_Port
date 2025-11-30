/**
 * Red Alert macOS Port - Sidebar & Factory System Tests
 *
 * Tests the production queue, sidebar management, and factory system.
 */

#include "../game/sidebar.h"
#include "../game/factory.h"
#include "../game/house.h"
#include "../game/infantry_types.h"
#include "../game/unit_types.h"
#include "../game/building_types.h"
#include "../game/aircraft_types.h"
#include <cstdio>
#include <cstring>

//===========================================================================
// Test Framework
//===========================================================================

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) \
    static void test_##name(); \
    static struct Test_##name { \
        Test_##name() { \
            printf("  Testing " #name "..."); \
            test_##name(); \
            printf(" OK\n"); \
            tests_passed++; \
        } \
    } test_##name##_instance; \
    static void test_##name()

#define ASSERT(condition) \
    do { \
        if (!(condition)) { \
            printf(" FAIL\n    Assertion failed: " #condition "\n    at %s:%d\n", \
                   __FILE__, __LINE__); \
            tests_failed++; \
            return; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            printf(" FAIL\n    Assertion failed: " #a " == " #b "\n    at %s:%d\n", \
                   __FILE__, __LINE__); \
            tests_failed++; \
            return; \
        } \
    } while(0)

//===========================================================================
// Factory Tests
//===========================================================================

TEST(factory_construction) {
    Init_Factories();

    for (int i = 0; i < FACTORY_MAX; i++) {
        ASSERT(!Factories[i].isActive_);
        ASSERT_EQ(Factories[i].id_, i);
    }
    ASSERT_EQ(FactoryCount, 0);
}

TEST(factory_create_destroy) {
    Init_Factories();

    FactoryClass* f1 = Create_Factory();
    ASSERT(f1 != nullptr);
    ASSERT_EQ(FactoryCount, 1);

    FactoryClass* f2 = Create_Factory();
    ASSERT(f2 != nullptr);
    ASSERT_EQ(FactoryCount, 2);
    ASSERT(f1 != f2);

    Destroy_Factory(f1);
    ASSERT_EQ(FactoryCount, 1);

    Destroy_Factory(f2);
    ASSERT_EQ(FactoryCount, 0);
}

TEST(factory_set_infantry) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 10000;

    FactoryClass* factory = Create_Factory();
    ASSERT(factory != nullptr);

    // Set up to produce a rifle infantry
    bool result = factory->Set(RTTIType::INFANTRY,
                               static_cast<int>(InfantryType::E1),
                               house);
    ASSERT(result);
    ASSERT(factory->isActive_);
    ASSERT_EQ(factory->productionType_, RTTIType::INFANTRY);
    ASSERT_EQ(factory->productionId_, static_cast<int>(InfantryType::E1));
    ASSERT(factory->balance_ > 0);  // Has a cost
    ASSERT(factory->rate_ > 0);     // Has a production rate
}

TEST(factory_set_unit) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 10000;

    FactoryClass* factory = Create_Factory();
    ASSERT(factory != nullptr);

    // Set up to produce a medium tank
    bool result = factory->Set(RTTIType::UNIT,
                               static_cast<int>(UnitType::MTANK2),
                               house);
    ASSERT(result);
    ASSERT_EQ(factory->productionType_, RTTIType::UNIT);
}

TEST(factory_set_building) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 10000;

    FactoryClass* factory = Create_Factory();
    ASSERT(factory != nullptr);

    // Set up to produce a power plant
    bool result = factory->Set(RTTIType::BUILDING,
                               static_cast<int>(BuildingType::POWER),
                               house);
    ASSERT(result);
    ASSERT_EQ(factory->productionType_, RTTIType::BUILDING);
}

TEST(factory_production_cycle) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 100000;

    FactoryClass* factory = Create_Factory();
    factory->Set(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1), house);

    // Start production
    ASSERT(!factory->Is_Building());
    factory->Start();
    ASSERT(!factory->isSuspended_);

    // Run many ticks to complete production
    int startCredits = house->credits_;
    for (int tick = 0; tick < 10000 && !factory->Has_Completed(); tick++) {
        factory->AI();
    }

    ASSERT(factory->Has_Completed());
    ASSERT_EQ(factory->Completion(), 100);

    // Money should have been spent
    ASSERT(house->credits_ < startCredits);
}

TEST(factory_suspend_resume) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 100000;

    FactoryClass* factory = Create_Factory();
    factory->Set(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1), house);
    factory->Start();

    // Run a few ticks (but not enough to complete)
    for (int i = 0; i < 20; i++) {
        factory->AI();
    }
    int stageBeforeSuspend = factory->stage_;
    ASSERT(stageBeforeSuspend > 0);

    // Suspend
    factory->Suspend();
    ASSERT(factory->isSuspended_);

    // Run more ticks - should not progress
    for (int i = 0; i < 100; i++) {
        factory->AI();
    }
    ASSERT_EQ(factory->stage_, stageBeforeSuspend);

    // Resume
    factory->Start();
    ASSERT(!factory->isSuspended_);

    // Run more ticks - should progress
    for (int i = 0; i < 100; i++) {
        factory->AI();
    }
    ASSERT(factory->stage_ > stageBeforeSuspend);
}

TEST(factory_abandon_refund) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 100000;

    FactoryClass* factory = Create_Factory();
    factory->Set(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1), house);
    factory->Start();

    // Run a few ticks to spend some money (but not enough to complete)
    int startCredits = house->credits_;
    for (int i = 0; i < 20; i++) {
        factory->AI();
    }
    int creditsAfterProduction = house->credits_;
    ASSERT(creditsAfterProduction < startCredits);

    // Abandon - should get refund
    factory->Abandon();
    ASSERT(house->credits_ > creditsAfterProduction);  // Got refund
    ASSERT(!factory->isActive_);
}

TEST(factory_insufficient_funds) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 10;  // Very little money

    FactoryClass* factory = Create_Factory();
    factory->Set(RTTIType::UNIT, static_cast<int>(UnitType::MTANK2), house);
    factory->Start();

    // Production should stall due to insufficient funds
    int initialStage = factory->stage_;
    for (int i = 0; i < 1000; i++) {
        factory->AI();
    }

    // Should not have progressed much (or at all) without money
    // Stage might be 0 or 1 depending on timing
    ASSERT(!factory->Has_Completed());
}

//===========================================================================
// StripClass Tests
//===========================================================================

TEST(strip_construction) {
    StripClass strip;
    strip.Init(0, 100, 200);

    ASSERT_EQ(strip.id_, 0);
    ASSERT_EQ(strip.x_, 100);
    ASSERT_EQ(strip.y_, 200);
    ASSERT_EQ(strip.buildableCount_, 0);
    ASSERT_EQ(strip.topIndex_, 0);
    ASSERT_EQ(strip.flasher_, -1);
}

TEST(strip_add_remove) {
    StripClass strip;
    strip.Init(0, 0, 0);

    // Add items
    ASSERT(strip.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1)));
    ASSERT_EQ(strip.buildableCount_, 1);

    ASSERT(strip.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E2)));
    ASSERT_EQ(strip.buildableCount_, 2);

    // Can't add duplicates
    ASSERT(!strip.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1)));
    ASSERT_EQ(strip.buildableCount_, 2);

    // Remove items
    ASSERT(strip.Remove(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1)));
    ASSERT_EQ(strip.buildableCount_, 1);

    // Can't remove non-existent
    ASSERT(!strip.Remove(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1)));
}

TEST(strip_find) {
    StripClass strip;
    strip.Init(0, 0, 0);

    strip.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1));
    strip.Add(RTTIType::UNIT, static_cast<int>(UnitType::MTANK2));
    strip.Add(RTTIType::BUILDING, static_cast<int>(BuildingType::POWER));

    ASSERT_EQ(strip.Find(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1)), 0);
    ASSERT_EQ(strip.Find(RTTIType::UNIT, static_cast<int>(UnitType::MTANK2)), 1);
    ASSERT_EQ(strip.Find(RTTIType::BUILDING, static_cast<int>(BuildingType::POWER)), 2);
    ASSERT_EQ(strip.Find(RTTIType::AIRCRAFT, 0), -1);  // Not found
}

TEST(strip_factory_link) {
    Init_Factories();
    StripClass strip;
    strip.Init(0, 0, 0);

    strip.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1));
    strip.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E2));

    // Link a factory
    strip.Factory_Link(5, RTTIType::INFANTRY, static_cast<int>(InfantryType::E1));
    ASSERT_EQ(strip.buildables_[0].factoryIndex, 5);
    ASSERT_EQ(strip.buildables_[1].factoryIndex, -1);

    // Unlink
    strip.Factory_Unlink(5);
    ASSERT_EQ(strip.buildables_[0].factoryIndex, -1);
}

TEST(strip_scroll) {
    StripClass strip;
    strip.Init(0, 0, 0);

    // Add more items than visible
    for (int i = 0; i < 8; i++) {
        strip.Add(RTTIType::INFANTRY, i);
    }
    ASSERT_EQ(strip.buildableCount_, 8);

    // Initially at top
    ASSERT_EQ(strip.topIndex_, 0);
    ASSERT(!strip.Can_Scroll_Up());
    ASSERT(strip.Can_Scroll_Down());

    // Scroll down
    ASSERT(strip.Scroll(false));
    // Process scroll animation
    while (strip.isScrolling_) {
        strip.AI(nullptr);
    }
    ASSERT_EQ(strip.topIndex_, 1);

    // Now can scroll up
    ASSERT(strip.Can_Scroll_Up());

    // Scroll up
    ASSERT(strip.Scroll(true));
    while (strip.isScrolling_) {
        strip.AI(nullptr);
    }
    ASSERT_EQ(strip.topIndex_, 0);
}

//===========================================================================
// SidebarClass Tests
//===========================================================================

TEST(sidebar_construction) {
    Sidebar.Init();

    ASSERT(!Sidebar.isActive_);
    ASSERT(!Sidebar.isRepairActive_);
    ASSERT(!Sidebar.isUpgradeActive_);
}

TEST(sidebar_activate) {
    Sidebar.Init();

    // Activate
    Sidebar.Activate(1);
    ASSERT(Sidebar.isActive_);

    // Deactivate
    Sidebar.Activate(0);
    ASSERT(!Sidebar.isActive_);

    // Toggle
    Sidebar.Activate(-1);
    ASSERT(Sidebar.isActive_);
    Sidebar.Activate(-1);
    ASSERT(!Sidebar.isActive_);
}

TEST(sidebar_which_column) {
    // Structures go in column 0
    ASSERT_EQ(SidebarClass::Which_Column(RTTIType::BUILDING), 0);
    ASSERT_EQ(SidebarClass::Which_Column(RTTIType::SPECIAL), 0);

    // Units go in column 1
    ASSERT_EQ(SidebarClass::Which_Column(RTTIType::INFANTRY), 1);
    ASSERT_EQ(SidebarClass::Which_Column(RTTIType::UNIT), 1);
    ASSERT_EQ(SidebarClass::Which_Column(RTTIType::AIRCRAFT), 1);
    ASSERT_EQ(SidebarClass::Which_Column(RTTIType::VESSEL), 1);
}

TEST(sidebar_add_remove) {
    Sidebar.Init();

    // Add building to column 0
    ASSERT(Sidebar.Add(RTTIType::BUILDING, static_cast<int>(BuildingType::POWER)));
    ASSERT_EQ(Sidebar.columns_[0].buildableCount_, 1);
    ASSERT_EQ(Sidebar.columns_[1].buildableCount_, 0);

    // Add infantry to column 1
    ASSERT(Sidebar.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1)));
    ASSERT_EQ(Sidebar.columns_[0].buildableCount_, 1);
    ASSERT_EQ(Sidebar.columns_[1].buildableCount_, 1);

    // Remove
    ASSERT(Sidebar.Remove(RTTIType::BUILDING, static_cast<int>(BuildingType::POWER)));
    ASSERT_EQ(Sidebar.columns_[0].buildableCount_, 0);
}

TEST(sidebar_control_buttons) {
    Sidebar.Init();

    // Toggle repair
    ASSERT(!Sidebar.isRepairActive_);
    Sidebar.Toggle_Repair();
    ASSERT(Sidebar.isRepairActive_);
    ASSERT(!Sidebar.isUpgradeActive_);

    // Toggle upgrade - should disable repair
    Sidebar.Toggle_Upgrade();
    ASSERT(!Sidebar.isRepairActive_);
    ASSERT(Sidebar.isUpgradeActive_);

    // Toggle repair again - should disable upgrade
    Sidebar.Toggle_Repair();
    ASSERT(Sidebar.isRepairActive_);
    ASSERT(!Sidebar.isUpgradeActive_);
}

TEST(sidebar_point_in_sidebar) {
    Sidebar.Init();

    // Inside sidebar
    ASSERT(Sidebar.Point_In_Sidebar(SIDE_X + 10, SIDE_Y + 10));

    // Outside sidebar
    ASSERT(!Sidebar.Point_In_Sidebar(0, 0));
    ASSERT(!Sidebar.Point_In_Sidebar(SIDE_X - 1, SIDE_Y));
}

//===========================================================================
// Integration Tests
//===========================================================================

TEST(sidebar_production_integration) {
    Init_Factories();
    Init_Houses();
    Sidebar.Init();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 100000;
    Sidebar.Set_House(house);
    Sidebar.Activate(1);

    // Add buildables
    Sidebar.Add(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1));
    Sidebar.Add(RTTIType::BUILDING, static_cast<int>(BuildingType::POWER));

    // Click to start production (simulate left click on first infantry slot)
    // Column 1 (units) position
    int clickX = SIDE_X + COLUMN_TWO_X + 5;
    int clickY = SIDE_Y + COLUMN_TWO_Y + 5;
    bool consumed = Sidebar.Input(0, clickX, clickY, true, false);
    ASSERT(consumed);

    // Should have created a factory
    ASSERT_EQ(FactoryCount, 1);

    // Find the factory
    FactoryClass* factory = Find_Factory(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1));
    ASSERT(factory != nullptr);
    ASSERT(!factory->isSuspended_);

    // Run AI to advance production
    for (int i = 0; i < 100; i++) {
        Sidebar.AI();
    }

    // Should have made some progress
    ASSERT(factory->Completion() > 0);
}

TEST(power_affects_production_rate) {
    Init_Factories();
    Init_Houses();

    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);
    house->credits_ = 1000000;

    // Test at full power
    house->power_ = 100;
    house->drain_ = 50;  // 50% drain = full power (100% fraction)

    FactoryClass* f1 = Create_Factory();
    f1->Set(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1), house);
    int rateFullPower = f1->rate_;

    // Test at low power
    house->power_ = 10;
    house->drain_ = 100;  // More drain than power

    FactoryClass* f2 = Create_Factory();
    f2->Set(RTTIType::INFANTRY, static_cast<int>(InfantryType::E1), house);
    int rateLowPower = f2->rate_;

    // Low power should result in slower rate (higher rate value = more ticks per stage)
    ASSERT(rateLowPower >= rateFullPower);
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Red Alert Sidebar & Factory System Tests\n");
    printf("=========================================\n\n");

    printf("Factory Tests:\n");
    // Tests run automatically via static initialization

    printf("\nStripClass Tests:\n");

    printf("\nSidebarClass Tests:\n");

    printf("\nIntegration Tests:\n");

    printf("\n=========================================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("=========================================\n");

    return tests_failed > 0 ? 1 : 0;
}
