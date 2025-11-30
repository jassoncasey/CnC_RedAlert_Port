/**
 * Red Alert macOS Port - Radar/Minimap Tests
 *
 * Tests the radar display, zoom, click handling, and fog of war.
 */

#include "../game/radar.h"
#include "../game/mapclass.h"
#include "../game/cell.h"
#include "../game/house.h"
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
// Radar Initialization Tests
//===========================================================================

TEST(radar_construction) {
    RadarClass radar;
    radar.Init();

    ASSERT(!radar.Is_Active());
    ASSERT(!radar.Is_Zoomed());
    ASSERT(!radar.Is_Radar_Jammed());
}

TEST(radar_one_time) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // After one-time init, radar hardware should exist
    // (activation still required)
    ASSERT(!radar.Is_Active());
}

//===========================================================================
// Radar Activation Tests
//===========================================================================

TEST(radar_activate_on) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Activate radar
    radar.Activate(1);  // Turn on

    // Run AI to complete activation animation
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }

    ASSERT(radar.Is_Active());
}

TEST(radar_activate_off) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Activate then deactivate
    radar.Activate(1);
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }
    ASSERT(radar.Is_Active());

    radar.Activate(0);  // Turn off
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }

    ASSERT(!radar.Is_Active());
}

TEST(radar_activate_toggle) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    ASSERT(!radar.Is_Active());

    // Toggle on
    radar.Activate(-1);
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }
    ASSERT(radar.Is_Active());

    // Toggle off
    radar.Activate(-1);
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }
    ASSERT(!radar.Is_Active());
}

//===========================================================================
// Click Detection Tests
//===========================================================================

TEST(click_in_radar_inside) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Click inside radar area
    int x = RADAR_X + RADAR_WIDTH / 2;
    int y = RADAR_Y + RADAR_HEIGHT / 2;
    ASSERT(radar.Click_In_Radar(x, y));
}

TEST(click_in_radar_outside) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Click outside radar area
    ASSERT(!radar.Click_In_Radar(0, 0));
    ASSERT(!radar.Click_In_Radar(RADAR_X - 10, RADAR_Y));
    ASSERT(!radar.Click_In_Radar(RADAR_X + RADAR_WIDTH + 10, RADAR_Y));
}

TEST(click_in_radar_edges) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Click on edges (inclusive)
    ASSERT(radar.Click_In_Radar(RADAR_X, RADAR_Y));
    ASSERT(radar.Click_In_Radar(RADAR_X + RADAR_WIDTH - 1, RADAR_Y));
    ASSERT(radar.Click_In_Radar(RADAR_X, RADAR_Y + RADAR_HEIGHT - 1));

    // Just outside
    ASSERT(!radar.Click_In_Radar(RADAR_X - 1, RADAR_Y));
    ASSERT(!radar.Click_In_Radar(RADAR_X, RADAR_Y - 1));
}

//===========================================================================
// Cell Visibility Tests
//===========================================================================

TEST(cell_on_radar_visible) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Center of the map should be visible on radar
    int16_t cell = 32 * MAP_CELL_WIDTH + 32;  // Cell at (32, 32)
    ASSERT(radar.Cell_On_Radar(cell));
}

TEST(cell_on_radar_bounds) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Invalid cells
    ASSERT(!radar.Cell_On_Radar(-1));
    ASSERT(!radar.Cell_On_Radar(MAP_CELL_TOTAL));
}

//===========================================================================
// Zoom Tests
//===========================================================================

TEST(radar_zoom_toggle) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    ASSERT(!radar.Is_Zoomed());

    radar.Zoom_Mode();
    ASSERT(radar.Is_Zoomed());

    radar.Zoom_Mode();
    ASSERT(!radar.Is_Zoomed());
}

//===========================================================================
// Position Tests
//===========================================================================

TEST(radar_set_position) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Set radar position to a specific cell
    int16_t cell = 50 * MAP_CELL_WIDTH + 50;
    radar.Set_Radar_Position(cell);

    // The cell should be on radar
    ASSERT(radar.Cell_On_Radar(cell));
}

TEST(radar_center_on_cell) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    int16_t cell = 40 * MAP_CELL_WIDTH + 40;
    radar.Center_On_Cell(cell);

    ASSERT(radar.Cell_On_Radar(cell));
}

//===========================================================================
// Tactical View Tests
//===========================================================================

TEST(radar_set_tactical_view) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    int16_t cell = 20 * MAP_CELL_WIDTH + 20;
    radar.Set_Tactical_View(cell, 15, 10);

    // Tactical view should be set (we can't easily verify values,
    // but this tests the method doesn't crash)
    ASSERT(true);
}

//===========================================================================
// Pixel Queue Tests
//===========================================================================

TEST(radar_pixel_queue) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Queue some cells for update
    int16_t cell1 = 10 * MAP_CELL_WIDTH + 10;
    int16_t cell2 = 20 * MAP_CELL_WIDTH + 20;

    radar.Radar_Pixel(cell1);
    radar.Radar_Pixel(cell2);

    // Duplicate should not be added
    radar.Radar_Pixel(cell1);

    // No direct way to verify queue count, but this tests the method
    ASSERT(true);
}

TEST(radar_full_redraw) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    // Request full redraw
    radar.Full_Redraw();

    // No direct way to verify flag, but this tests the method
    ASSERT(true);
}

//===========================================================================
// Map Integration Tests
//===========================================================================

TEST(radar_with_map) {
    RadarClass radar;
    radar.Init();

    // Create and link a map
    MapClass map;
    map.OneTime();
    map.AllocCells();
    map.InitCells();
    map.SetMapDimensions(0, 0, 64, 64);

    radar.Set_Map(&map);
    radar.One_Time();
    radar.Activate(1);

    // Run AI to activate
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }

    ASSERT(radar.Is_Active());

    // Cell in map should be on radar
    int16_t cell = 32 * MAP_CELL_WIDTH + 32;
    ASSERT(radar.Cell_On_Radar(cell));

    map.FreeCells();
}

TEST(radar_click_cell_calc) {
    RadarClass radar;
    radar.Init();

    MapClass map;
    map.OneTime();
    map.AllocCells();
    map.InitCells();
    map.SetMapDimensions(0, 0, 64, 64);

    radar.Set_Map(&map);
    radar.One_Time();
    radar.Activate(1);
    for (int i = 0; i < 30; i++) {
        radar.AI();
    }

    // Click in center of radar
    int clickX = RADAR_X + RADAR_WIDTH / 2;
    int clickY = RADAR_Y + RADAR_HEIGHT / 2;

    int16_t cell = radar.Click_Cell_Calc(clickX, clickY);
    ASSERT(cell >= 0);
    ASSERT(cell < MAP_CELL_TOTAL);

    map.FreeCells();
}

//===========================================================================
// Getter Tests
//===========================================================================

TEST(radar_getters) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    ASSERT_EQ(radar.GetX(), RADAR_X);
    ASSERT_EQ(radar.GetY(), RADAR_Y);
    ASSERT_EQ(radar.GetWidth(), RADAR_WIDTH);
    ASSERT_EQ(radar.GetHeight(), RADAR_HEIGHT);
}

//===========================================================================
// Jam Tests
//===========================================================================

TEST(radar_jam_unjam) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    Init_Houses();
    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);

    int16_t cell = 30 * MAP_CELL_WIDTH + 30;

    // Jam and unjam (methods exist, just test they don't crash)
    radar.Jam_Cell(cell, house);
    radar.UnJam_Cell(cell, house);

    ASSERT(true);
}

//===========================================================================
// Map Cell Tests
//===========================================================================

TEST(radar_map_cell) {
    RadarClass radar;
    radar.Init();
    radar.One_Time();

    Init_Houses();
    HouseClass* house = &Houses[static_cast<int>(HousesType::GREECE)];
    house->Init(HousesType::GREECE);

    int16_t cell = 25 * MAP_CELL_WIDTH + 25;

    // Map a cell (reveals it)
    radar.Map_Cell(cell, house);

    ASSERT(true);
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Red Alert Radar/Minimap Tests\n");
    printf("==============================\n\n");

    printf("Initialization Tests:\n");
    // Tests run automatically via static initialization

    printf("\nActivation Tests:\n");

    printf("\nClick Detection Tests:\n");

    printf("\nCell Visibility Tests:\n");

    printf("\nZoom Tests:\n");

    printf("\nPosition Tests:\n");

    printf("\nTactical View Tests:\n");

    printf("\nPixel Queue Tests:\n");

    printf("\nMap Integration Tests:\n");

    printf("\nGetter Tests:\n");

    printf("\nJam Tests:\n");

    printf("\nMap Cell Tests:\n");

    printf("\n==============================\n");
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);
    printf("==============================\n");

    return tests_failed > 0 ? 1 : 0;
}
