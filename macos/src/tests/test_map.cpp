/**
 * Red Alert macOS Port - Map/Cell/Pathfinding Tests
 *
 * Tests for CellClass, MapClass, and PathFinder.
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>

#include "game/types.h"
#include "game/cell.h"
#include "game/mapclass.h"
#include "game/pathfind.h"

// Simple test framework
static int g_testsPassed = 0;
static int g_testsFailed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("  %s... ", #name); \
    test_##name(); \
    printf("OK\n"); \
    g_testsPassed++; \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED at line %d: %s\n", __LINE__, #cond); \
        g_testsFailed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) do { \
    if ((a) != (b)) { \
        printf("FAILED at line %d: %s != %s\n", __LINE__, #a, #b); \
        g_testsFailed++; \
        return; \
    } \
} while(0)

//===========================================================================
// Coordinate Conversion Tests
//===========================================================================

TEST(coord_xy_cell) {
    CELL cell = XY_Cell(10, 20);
    ASSERT_EQ(Cell_X(cell), 10);
    ASSERT_EQ(Cell_Y(cell), 20);
}

TEST(coord_cell_bounds) {
    CELL cell = XY_Cell(0, 0);
    ASSERT_EQ(cell, 0);

    cell = XY_Cell(127, 127);
    ASSERT(cell > 0);
    ASSERT_EQ(Cell_X(cell), 127);
    ASSERT_EQ(Cell_Y(cell), 127);
}

TEST(coord_cell_to_coord) {
    CELL cell = XY_Cell(50, 60);
    int32_t coord = Cell_Coord(cell);

    // Convert back
    CELL result = Coord_Cell(coord);
    ASSERT_EQ(Cell_X(result), 50);
    ASSERT_EQ(Cell_Y(result), 60);
}

TEST(coord_xy_coord) {
    int32_t coord = XY_Coord(1000, 2000);
    ASSERT_EQ(Coord_X(coord), 1000);
    ASSERT_EQ(Coord_Y(coord), 2000);
}

//===========================================================================
// CellClass Tests
//===========================================================================

TEST(cell_construction) {
    CellClass cell;
    ASSERT_EQ(cell.CellNumber(), 0);
    ASSERT(!cell.IsMapped());
    ASSERT(!cell.IsVisible());
}

TEST(cell_clear) {
    CellClass cell;
    cell.SetMapped(true);
    cell.SetVisible(true);
    cell.SetOverlay(OverlayType::GOLD2);

    cell.Clear();

    ASSERT(!cell.IsMapped());
    ASSERT(!cell.IsVisible());
    ASSERT(!cell.HasOre());
}

TEST(cell_terrain_water) {
    CellClass cell;
    cell.templateType_ = TemplateType::WATER;
    cell.RecalcLandType();

    ASSERT(cell.IsWater());
    ASSERT(!cell.IsPassable(SpeedType::FOOT));
    ASSERT(!cell.IsPassable(SpeedType::TRACK));
    ASSERT(cell.IsPassable(SpeedType::FLOAT));
    ASSERT(cell.IsPassable(SpeedType::WINGED));
}

TEST(cell_terrain_clear) {
    CellClass cell;
    cell.templateType_ = TemplateType::CLEAR1;
    cell.RecalcLandType();

    ASSERT(!cell.IsWater());
    ASSERT(cell.IsPassable(SpeedType::FOOT));
    ASSERT(cell.IsPassable(SpeedType::TRACK));
    ASSERT(!cell.IsPassable(SpeedType::FLOAT));
    ASSERT(cell.IsPassable(SpeedType::WINGED));
}

TEST(cell_ore) {
    CellClass cell;
    cell.SetOverlay(OverlayType::GOLD1);

    ASSERT(cell.HasOre());
    ASSERT(!cell.HasGems());
    ASSERT_EQ(cell.OreValue(), 25);  // Stage 1 = 25 credits
}

TEST(cell_ore_stages) {
    CellClass cell;

    cell.SetOverlay(OverlayType::GOLD1);
    ASSERT_EQ(cell.OreValue(), 25);

    cell.SetOverlay(OverlayType::GOLD2);
    ASSERT_EQ(cell.OreValue(), 50);

    cell.SetOverlay(OverlayType::GOLD3);
    ASSERT_EQ(cell.OreValue(), 75);

    cell.SetOverlay(OverlayType::GOLD4);
    ASSERT_EQ(cell.OreValue(), 100);
}

TEST(cell_gems) {
    CellClass cell;
    cell.SetOverlay(OverlayType::GEMS2);

    ASSERT(!cell.HasOre());
    ASSERT(cell.HasGems());
    ASSERT_EQ(cell.OreValue(), 100);  // Stage 2 gems = 100 credits
}

TEST(cell_ore_growth) {
    CellClass cell;
    cell.SetOverlay(OverlayType::GOLD1);

    ASSERT(cell.CanOreGrow());
    ASSERT(!cell.CanOreSpread());

    cell.GrowOre();
    ASSERT_EQ(cell.OreValue(), 50);  // Now at stage 2
}

TEST(cell_ore_spread) {
    CellClass cell;
    cell.SetOverlay(OverlayType::GOLD4);

    ASSERT(!cell.CanOreGrow());
    ASSERT(cell.CanOreSpread());
}

TEST(cell_reduce_ore) {
    CellClass cell;
    cell.SetOverlay(OverlayType::GOLD4);

    int reduced = cell.ReduceOre(50);
    ASSERT_EQ(reduced, 50);
    ASSERT_EQ(cell.OreValue(), 50);  // Should be at stage 2 now
}

TEST(cell_wall) {
    CellClass cell;
    cell.SetOverlay(OverlayType::BRICK_WALL, 100);

    ASSERT(cell.IsWall());
    ASSERT_EQ(cell.GetLandType(), LandType::WALL);
    ASSERT(!cell.IsPassable(SpeedType::TRACK));
}

TEST(cell_wall_damage) {
    CellClass cell;
    cell.SetOverlay(OverlayType::BRICK_WALL, 100);

    int damaged = cell.ReduceWall(50);
    ASSERT_EQ(damaged, 50);
    ASSERT(cell.IsWall());  // Still standing

    damaged = cell.ReduceWall(100);
    ASSERT_EQ(damaged, 50);  // Only 50 remaining
    ASSERT(!cell.IsWall()); // Destroyed
}

TEST(cell_flag) {
    CellClass cell;
    ASSERT(!cell.HasFlag());

    cell.PlaceFlag(HousesType::USSR);
    ASSERT(cell.HasFlag());
    ASSERT_EQ(cell.FlagOwner(), HousesType::USSR);

    cell.RemoveFlag();
    ASSERT(!cell.HasFlag());
    ASSERT_EQ(cell.FlagOwner(), HousesType::NONE);
}

TEST(cell_visibility) {
    CellClass cell;
    ASSERT(!cell.IsMapped());
    ASSERT(!cell.IsVisible());

    cell.SetMapped(true);
    ASSERT(cell.IsMapped());
    ASSERT(!cell.IsVisible());

    cell.SetVisible(true);
    ASSERT(cell.IsVisible());
}

TEST(cell_clear_to_build) {
    CellClass cell;
    cell.templateType_ = TemplateType::CLEAR1;
    cell.RecalcLandType();

    ASSERT(cell.IsClearToBuild());

    // Can't build on water
    cell.templateType_ = TemplateType::WATER;
    cell.RecalcLandType();
    ASSERT(!cell.IsClearToBuild());
}

TEST(cell_spots) {
    CellClass cell;
    ASSERT(cell.IsSpotFree(SpotType::CENTER));
    ASSERT(cell.IsSpotFree(SpotType::UPPER_LEFT));

    // Mark center as occupied
    cell.flag_.occupy.center = 1;
    ASSERT(!cell.IsSpotFree(SpotType::CENTER));
    ASSERT(cell.IsSpotFree(SpotType::UPPER_LEFT));
}

TEST(cell_adjacent) {
    CellClass cell;
    cell.SetCellNumber(XY_Cell(64, 64));

    CELL adj = cell.AdjacentCell(FacingType::NORTH);
    ASSERT_EQ(Cell_X(adj), 64);
    ASSERT_EQ(Cell_Y(adj), 63);

    adj = cell.AdjacentCell(FacingType::EAST);
    ASSERT_EQ(Cell_X(adj), 65);
    ASSERT_EQ(Cell_Y(adj), 64);

    adj = cell.AdjacentCell(FacingType::SOUTH_EAST);
    ASSERT_EQ(Cell_X(adj), 65);
    ASSERT_EQ(Cell_Y(adj), 65);
}

TEST(cell_color) {
    CellClass cell;
    cell.templateType_ = TemplateType::WATER;
    cell.RecalcLandType();
    ASSERT_EQ(cell.CellColor(), 0x000080);  // Blue

    cell.ClearOverlay();
    cell.templateType_ = TemplateType::CLEAR1;
    cell.RecalcLandType();
    ASSERT_EQ(cell.CellColor(), 0x008000);  // Green

    cell.SetOverlay(OverlayType::GOLD1);
    ASSERT_EQ(cell.CellColor(), 0xFFFF00);  // Yellow
}

//===========================================================================
// MapClass Tests
//===========================================================================

TEST(map_init) {
    Map.OneTime();
    Map.InitClear();

    ASSERT(Map.IsValidCell(0));
    ASSERT(Map.IsValidCell(MAP_CELL_TOTAL - 1));
    ASSERT(!Map.IsValidCell(-1));
    ASSERT(!Map.IsValidCell(MAP_CELL_TOTAL));
}

TEST(map_dimensions) {
    Map.SetMapDimensions(10, 10, 100, 100);

    ASSERT_EQ(Map.MapCellX(), 10);
    ASSERT_EQ(Map.MapCellY(), 10);
    ASSERT_EQ(Map.MapCellWidth(), 100);
    ASSERT_EQ(Map.MapCellHeight(), 100);
}

TEST(map_cell_access) {
    Map.InitClear();

    CellClass& cell = Map[XY_Cell(50, 50)];
    cell.SetMapped(true);

    ASSERT(Map[XY_Cell(50, 50)].IsMapped());
}

TEST(map_in_radar) {
    Map.SetMapDimensions(10, 10, 50, 50);

    ASSERT(Map.InRadar(XY_Cell(30, 30)));
    ASSERT(!Map.InRadar(XY_Cell(5, 5)));
    ASSERT(!Map.InRadar(XY_Cell(70, 70)));
}

TEST(map_reveal) {
    Map.InitClear();

    Map.ShroudTheMap();
    ASSERT(!Map[XY_Cell(64, 64)].IsMapped());

    Map.RevealTheMap();
    ASSERT(Map[XY_Cell(64, 64)].IsMapped());
    ASSERT(Map[XY_Cell(64, 64)].IsVisible());
}

TEST(map_sight_from) {
    Map.InitClear();
    Map.SetMapDimensions(0, 0, 128, 128);
    Map.ShroudTheMap();

    Map.SightFrom(XY_Cell(64, 64), 3, nullptr);

    // Center should be visible
    ASSERT(Map[XY_Cell(64, 64)].IsMapped());

    // Nearby cells should be visible
    ASSERT(Map[XY_Cell(65, 64)].IsMapped());
    ASSERT(Map[XY_Cell(64, 65)].IsMapped());

    // Far cells should still be hidden
    ASSERT(!Map[XY_Cell(70, 70)].IsMapped());
}

TEST(map_cell_region) {
    int region1 = Map.CellRegion(XY_Cell(0, 0));
    int region2 = Map.CellRegion(XY_Cell(4, 0));
    int region3 = Map.CellRegion(XY_Cell(0, 4));

    ASSERT(region1 != region2);
    ASSERT(region1 != region3);
    ASSERT(region2 != region3);
}

TEST(map_total_value) {
    Map.InitClear();

    Map[XY_Cell(50, 50)].SetOverlay(OverlayType::GOLD4);
    Map[XY_Cell(51, 50)].SetOverlay(OverlayType::GEMS2);

    Map.RecalculateTotalValue();

    ASSERT(Map.TotalValue() > 0);
    ASSERT_EQ(Map.TotalValue(), 100 + 100);  // GOLD4=100, GEMS2=100
}

//===========================================================================
// Pathfinding Tests
//===========================================================================

TEST(path_direction) {
    CELL from = XY_Cell(50, 50);
    CELL to = XY_Cell(51, 50);  // East

    FacingType dir = Cell_Direction(from, to);
    ASSERT_EQ(dir, FacingType::EAST);

    to = XY_Cell(50, 49);  // North
    dir = Cell_Direction(from, to);
    ASSERT_EQ(dir, FacingType::NORTH);

    to = XY_Cell(51, 51);  // Southeast
    dir = Cell_Direction(from, to);
    ASSERT_EQ(dir, FacingType::SOUTH_EAST);
}

TEST(path_adjacent) {
    CELL cell = XY_Cell(50, 50);

    CELL adj = Adjacent_Cell(cell, FacingType::NORTH);
    ASSERT_EQ(Cell_X(adj), 50);
    ASSERT_EQ(Cell_Y(adj), 49);

    adj = Adjacent_Cell(cell, FacingType::EAST);
    ASSERT_EQ(Cell_X(adj), 51);
    ASSERT_EQ(Cell_Y(adj), 50);
}

TEST(path_find_simple) {
    Map.InitClear();
    Map.SetMapDimensions(0, 0, 128, 128);

    CELL start = XY_Cell(50, 50);
    CELL target = XY_Cell(55, 50);  // 5 cells east

    PathType path = Find_Path(start, target, SpeedType::TRACK);

    ASSERT(path.IsValid());
    ASSERT(path.length > 0);
    ASSERT(path.length <= 10);  // Should be around 5
}

TEST(path_find_diagonal) {
    Map.InitClear();
    Map.SetMapDimensions(0, 0, 128, 128);

    CELL start = XY_Cell(50, 50);
    CELL target = XY_Cell(55, 55);  // 5 cells SE

    PathType path = Find_Path(start, target, SpeedType::TRACK);

    ASSERT(path.IsValid());
    ASSERT(path.length > 0);
    ASSERT(path.length <= 10);  // Diagonal should be efficient
}

TEST(path_around_obstacle) {
    Map.InitClear();
    Map.SetMapDimensions(0, 0, 128, 128);

    // Create a wall
    for (int y = 48; y <= 52; y++) {
        Map[XY_Cell(52, y)].templateType_ = TemplateType::WATER;
        Map[XY_Cell(52, y)].RecalcLandType();
    }

    CELL start = XY_Cell(50, 50);
    CELL target = XY_Cell(55, 50);

    PathType path = Find_Path(start, target, SpeedType::TRACK);

    ASSERT(path.IsValid());
    // Path should go around the water
    ASSERT(path.length > 5);  // Longer than direct path
}

TEST(path_same_cell) {
    Map.InitClear();

    CELL cell = XY_Cell(50, 50);
    PathType path = Find_Path(cell, cell, SpeedType::TRACK);

    // Should return valid but empty path
    ASSERT_EQ(path.length, 0);
}

TEST(path_water_unit) {
    Map.InitClear();
    Map.SetMapDimensions(0, 0, 128, 128);

    // Create a water area
    for (int x = 48; x <= 58; x++) {
        for (int y = 48; y <= 58; y++) {
            Map[XY_Cell(x, y)].templateType_ = TemplateType::WATER;
            Map[XY_Cell(x, y)].RecalcLandType();
        }
    }

    CELL start = XY_Cell(50, 50);
    CELL target = XY_Cell(55, 55);

    // Ground unit should fail
    PathType groundPath = Find_Path(start, target, SpeedType::TRACK);
    // Note: Path might find target nearby if original target is impassable

    // Water unit should succeed
    PathType waterPath = Find_Path(start, target, SpeedType::FLOAT);
    ASSERT(waterPath.IsValid());
}

TEST(path_line_of_sight) {
    Map.InitClear();

    CELL from = XY_Cell(50, 50);
    CELL to = XY_Cell(55, 50);

    ASSERT(PathFinder::LineOfSight(from, to, SpeedType::TRACK));

    // Add obstacle
    Map[XY_Cell(52, 50)].templateType_ = TemplateType::WATER;
    Map[XY_Cell(52, 50)].RecalcLandType();

    ASSERT(!PathFinder::LineOfSight(from, to, SpeedType::TRACK));
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("Red Alert Map/Cell/Pathfinding Tests\n");
    printf("====================================\n\n");

    // Initialize map for tests
    Map.OneTime();

    printf("Coordinate Tests:\n");
    RUN_TEST(coord_xy_cell);
    RUN_TEST(coord_cell_bounds);
    RUN_TEST(coord_cell_to_coord);
    RUN_TEST(coord_xy_coord);

    printf("\nCellClass Tests:\n");
    RUN_TEST(cell_construction);
    RUN_TEST(cell_clear);
    RUN_TEST(cell_terrain_water);
    RUN_TEST(cell_terrain_clear);
    RUN_TEST(cell_ore);
    RUN_TEST(cell_ore_stages);
    RUN_TEST(cell_gems);
    RUN_TEST(cell_ore_growth);
    RUN_TEST(cell_ore_spread);
    RUN_TEST(cell_reduce_ore);
    RUN_TEST(cell_wall);
    RUN_TEST(cell_wall_damage);
    RUN_TEST(cell_flag);
    RUN_TEST(cell_visibility);
    RUN_TEST(cell_clear_to_build);
    RUN_TEST(cell_spots);
    RUN_TEST(cell_adjacent);
    RUN_TEST(cell_color);

    printf("\nMapClass Tests:\n");
    RUN_TEST(map_init);
    RUN_TEST(map_dimensions);
    RUN_TEST(map_cell_access);
    RUN_TEST(map_in_radar);
    RUN_TEST(map_reveal);
    RUN_TEST(map_sight_from);
    RUN_TEST(map_cell_region);
    RUN_TEST(map_total_value);

    printf("\nPathfinding Tests:\n");
    RUN_TEST(path_direction);
    RUN_TEST(path_adjacent);
    RUN_TEST(path_find_simple);
    RUN_TEST(path_find_diagonal);
    RUN_TEST(path_around_obstacle);
    RUN_TEST(path_same_cell);
    RUN_TEST(path_water_unit);
    RUN_TEST(path_line_of_sight);

    printf("\n====================================\n");
    printf("Results: %d passed, %d failed\n", g_testsPassed, g_testsFailed);

    return g_testsFailed > 0 ? 1 : 0;
}
