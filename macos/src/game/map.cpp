/**
 * Red Alert macOS Port - Map/Terrain System Implementation
 */

#include "map.h"
#include "terrain.h"
#include "graphics/metal/renderer.h"
#include <cstdlib>
#include <cstring>
#include <cmath>

// Map state
static MapCell g_cells[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
static int g_mapWidth = 0;
static int g_mapHeight = 0;
static bool g_fogEnabled = true;  // Fog of war enabled by default
// Viewport dimensions (game view area, excluding sidebar)
static constexpr int GAME_VIEW_WIDTH = 560;   // Screen width minus sidebar
static constexpr int GAME_VIEW_HEIGHT = 368;  // Screen height minus HUD and control bars

static Viewport g_viewport = {0, 0, GAME_VIEW_WIDTH, GAME_VIEW_HEIGHT};

// Terrain colors (indexed, 8-bit palette style)
static const uint8_t g_terrainColors[TERRAIN_COUNT] = {
    2,      // CLEAR - green
    1,      // WATER - blue
    8,      // ROCK - dark gray
    10,     // TREE - dark green
    7,      // ROAD - light gray
    6,      // BRIDGE - brown
    15,     // BUILDING - white
    14,     // ORE - yellow
    13,     // GEM - magenta
};

void Map_Init(void) {
    memset(g_cells, 0, sizeof(g_cells));
    g_mapWidth = 0;
    g_mapHeight = 0;
}

void Map_Shutdown(void) {
    // Nothing to free currently
}

void Map_Create(int width, int height) {
    if (width > MAP_MAX_WIDTH) width = MAP_MAX_WIDTH;
    if (height > MAP_MAX_HEIGHT) height = MAP_MAX_HEIGHT;
    if (width < 1) width = 1;
    if (height < 1) height = 1;

    g_mapWidth = width;
    g_mapHeight = height;

    // Initialize all cells to clear terrain
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            g_cells[y][x].terrain = TERRAIN_CLEAR;
            g_cells[y][x].flags = 0;
            g_cells[y][x].height = 0;
            g_cells[y][x].oreAmount = 0;
            g_cells[y][x].unitId = -1;
            g_cells[y][x].buildingId = -1;
        }
    }

    // Reset viewport
    g_viewport.x = 0;
    g_viewport.y = 0;
    g_viewport.width = GAME_VIEW_WIDTH;
    g_viewport.height = GAME_VIEW_HEIGHT;
}

// Helper: Create a forest cluster
static void AddForestCluster(int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = cx + dx;
            int y = cy + dy;
            if (x < 0 || x >= g_mapWidth || y < 0 || y >= g_mapHeight) continue;
            // Circular falloff with randomness
            int dist2 = dx * dx + dy * dy;
            if (dist2 <= radius * radius && g_cells[y][x].terrain == TERRAIN_CLEAR) {
                // Higher chance near center
                if (rand() % (radius * radius + 1) > dist2 / 2) {
                    Map_SetTerrain(x, y, TERRAIN_TREE);
                }
            }
        }
    }
}

// Helper: Create a rock ridge
static void AddRockRidge(int x1, int y1, int x2, int y2, int thickness) {
    // Draw a thick line of rocks
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = dx - dy;
    int x = x1, y = y1;

    while (true) {
        // Add rocks in a cluster around this point
        for (int ty = y - thickness / 2; ty <= y + thickness / 2; ty++) {
            for (int tx = x - thickness / 2; tx <= x + thickness / 2; tx++) {
                if (tx >= 0 && tx < g_mapWidth && ty >= 0 && ty < g_mapHeight) {
                    if (g_cells[ty][tx].terrain == TERRAIN_CLEAR) {
                        Map_SetTerrain(tx, ty, TERRAIN_ROCK);
                    }
                }
            }
        }

        if (x == x2 && y == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x += sx; }
        if (e2 < dx) { err += dx; y += sy; }
    }
}

// Helper: Create ore field
static void AddOreField(int cx, int cy, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int x = cx + dx;
            int y = cy + dy;
            if (x < 0 || x >= g_mapWidth || y < 0 || y >= g_mapHeight) continue;
            int dist2 = dx * dx + dy * dy;
            if (dist2 <= radius * radius && g_cells[y][x].terrain == TERRAIN_CLEAR) {
                if (rand() % 3 != 0) {  // 67% density
                    Map_SetTerrain(x, y, TERRAIN_ORE);
                    // Set ore amount - more in center, less at edges
                    int amount = ORE_MAX_AMOUNT - (dist2 * 100 / (radius * radius + 1));
                    if (amount < 50) amount = 50 + rand() % 50;
                    g_cells[y][x].oreAmount = (uint8_t)amount;
                }
            }
        }
    }
}

void Map_GenerateDemo(void) {
    // Create a 64x64 demo map - Eastern European winter terrain
    Map_Create(64, 64);

    // Seed for reproducible terrain
    srand(12345);

    // === WATER FEATURES ===
    // Large frozen lake in southeast quadrant
    for (int y = 40; y < 58; y++) {
        for (int x = 38; x < 58; x++) {
            int dx = x - 48;
            int dy = y - 49;
            // Irregular lake shape using multiple ellipses
            if (dx * dx + dy * dy * 1.5 < 100 ||
                (dx + 5) * (dx + 5) + (dy - 3) * (dy - 3) < 40) {
                Map_SetTerrain(x, y, TERRAIN_WATER);
            }
        }
    }

    // River flowing from northwest into the lake
    float rx = 5.0f;
    for (int y = 0; y < 50; y++) {
        // Meandering river path
        rx += sinf(y * 0.15f) * 0.8f + 0.5f;
        int riverX = (int)rx;
        for (int w = 0; w < 2; w++) {
            if (riverX + w >= 0 && riverX + w < g_mapWidth) {
                Map_SetTerrain(riverX + w, y, TERRAIN_WATER);
            }
        }
    }

    // Small pond in northwest
    for (int y = 8; y < 14; y++) {
        for (int x = 48; x < 56; x++) {
            int dx = x - 52;
            int dy = y - 11;
            if (dx * dx + dy * dy < 12) {
                Map_SetTerrain(x, y, TERRAIN_WATER);
            }
        }
    }

    // === ROCKY TERRAIN ===
    // Mountain ridge running diagonally across upper portion
    AddRockRidge(2, 20, 30, 8, 2);
    AddRockRidge(30, 8, 45, 5, 2);

    // Cliff face near southern lake
    AddRockRidge(30, 50, 38, 55, 2);

    // Scattered rock outcrops
    AddRockRidge(55, 25, 60, 30, 1);
    AddRockRidge(10, 55, 18, 58, 2);

    // === FORESTS ===
    // Large forest in northwest
    AddForestCluster(12, 8, 5);
    AddForestCluster(8, 12, 4);

    // Forest belt along western edge
    AddForestCluster(4, 35, 4);
    AddForestCluster(6, 45, 5);

    // Forest near eastern edge
    AddForestCluster(58, 15, 4);
    AddForestCluster(55, 35, 3);

    // Scattered tree clusters
    AddForestCluster(25, 25, 3);
    AddForestCluster(40, 20, 2);
    AddForestCluster(20, 55, 3);

    // === ROADS ===
    // Main road from west to east
    for (int x = 0; x < 40; x++) {
        int y = 32 + (int)(sinf(x * 0.1f) * 2);
        Map_SetTerrain(x, y, TERRAIN_ROAD);
    }

    // Road from north
    for (int y = 0; y < 32; y++) {
        int x = 28 + (y > 15 ? (y - 15) / 4 : 0);
        Map_SetTerrain(x, y, TERRAIN_ROAD);
    }

    // Road to southern base area
    for (int y = 32; y < 52; y++) {
        Map_SetTerrain(20, y, TERRAIN_ROAD);
    }

    // === BRIDGES ===
    // Bridge over river where main road crosses (around x=21 at y=32)
    Map_SetTerrain(21, 32, TERRAIN_BRIDGE);
    Map_SetTerrain(22, 32, TERRAIN_BRIDGE);

    // Bridge where north road crosses river
    Map_SetTerrain(13, 10, TERRAIN_BRIDGE);
    Map_SetTerrain(14, 10, TERRAIN_BRIDGE);

    // === ORE FIELDS ===
    // Ore deposits - logical mining locations near rocky areas
    AddOreField(52, 28, 3);  // Near eastern rocks
    AddOreField(8, 50, 3);   // Near southwestern area
    AddOreField(35, 15, 2);  // Near mountain ridge

    // Gems near the lake (rare resource)
    Map_SetTerrain(35, 48, TERRAIN_GEM);
    Map_SetTerrain(36, 47, TERRAIN_GEM);
    Map_SetTerrain(35, 47, TERRAIN_GEM);

    // Don't reveal everything - fog of war will handle visibility
    // Initial visibility is set by units/buildings when they spawn
}

int Map_GetWidth(void) {
    return g_mapWidth;
}

int Map_GetHeight(void) {
    return g_mapHeight;
}

MapCell* Map_GetCell(int cellX, int cellY) {
    if (cellX < 0 || cellX >= g_mapWidth || cellY < 0 || cellY >= g_mapHeight) {
        return nullptr;
    }
    return &g_cells[cellY][cellX];
}

void Map_SetTerrain(int cellX, int cellY, TerrainType terrain) {
    MapCell* cell = Map_GetCell(cellX, cellY);
    if (cell) {
        cell->terrain = (uint8_t)terrain;
    }
}

BOOL Map_IsPassable(int cellX, int cellY) {
    MapCell* cell = Map_GetCell(cellX, cellY);
    if (!cell) return FALSE;

    switch (cell->terrain) {
        case TERRAIN_CLEAR:
        case TERRAIN_TREE:
        case TERRAIN_ROAD:
        case TERRAIN_BRIDGE:
        case TERRAIN_ORE:
        case TERRAIN_GEM:
            return (cell->flags & CELL_FLAG_OCCUPIED) == 0;
        default:
            return FALSE;
    }
}

BOOL Map_IsWaterPassable(int cellX, int cellY) {
    MapCell* cell = Map_GetCell(cellX, cellY);
    if (!cell) return FALSE;

    return cell->terrain == TERRAIN_WATER;
}

void Map_WorldToCell(int worldX, int worldY, int* cellX, int* cellY) {
    if (cellX) *cellX = worldX / CELL_SIZE;
    if (cellY) *cellY = worldY / CELL_SIZE;
}

void Map_CellToWorld(int cellX, int cellY, int* worldX, int* worldY) {
    if (worldX) *worldX = cellX * CELL_SIZE + CELL_SIZE / 2;
    if (worldY) *worldY = cellY * CELL_SIZE + CELL_SIZE / 2;
}

Viewport* Map_GetViewport(void) {
    return &g_viewport;
}

void Map_SetViewport(int x, int y) {
    int maxX = g_mapWidth * CELL_SIZE - g_viewport.width;
    int maxY = g_mapHeight * CELL_SIZE - g_viewport.height;

    if (maxX < 0) maxX = 0;
    if (maxY < 0) maxY = 0;

    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x > maxX) x = maxX;
    if (y > maxY) y = maxY;

    g_viewport.x = x;
    g_viewport.y = y;
}

void Map_ScrollViewport(int dx, int dy) {
    Map_SetViewport(g_viewport.x + dx, g_viewport.y + dy);
}

void Map_CenterViewport(int worldX, int worldY) {
    Map_SetViewport(worldX - g_viewport.width / 2, worldY - g_viewport.height / 2);
}

BOOL Map_IsInViewport(int worldX, int worldY) {
    return worldX >= g_viewport.x && worldX < g_viewport.x + g_viewport.width &&
           worldY >= g_viewport.y && worldY < g_viewport.y + g_viewport.height;
}

void Map_ScreenToWorld(int screenX, int screenY, int* worldX, int* worldY) {
    if (worldX) *worldX = screenX + g_viewport.x;
    if (worldY) *worldY = screenY + g_viewport.y;
}

void Map_WorldToScreen(int worldX, int worldY, int* screenX, int* screenY) {
    if (screenX) *screenX = worldX - g_viewport.x;
    if (screenY) *screenY = worldY - g_viewport.y;
}

void Map_Render(void) {
    if (g_mapWidth == 0 || g_mapHeight == 0) return;

    // Calculate visible cell range
    int startCellX = g_viewport.x / CELL_SIZE;
    int startCellY = g_viewport.y / CELL_SIZE;
    int endCellX = (g_viewport.x + g_viewport.width) / CELL_SIZE + 1;
    int endCellY = (g_viewport.y + g_viewport.height) / CELL_SIZE + 1;

    if (startCellX < 0) startCellX = 0;
    if (startCellY < 0) startCellY = 0;
    if (endCellX > g_mapWidth) endCellX = g_mapWidth;
    if (endCellY > g_mapHeight) endCellY = g_mapHeight;

    // Check if terrain tiles are available
    BOOL useTiles = Terrain_Available();

    // Render each visible cell
    for (int cy = startCellY; cy < endCellY; cy++) {
        for (int cx = startCellX; cx < endCellX; cx++) {
            MapCell* cell = &g_cells[cy][cx];

            // Skip unrevealed cells (fog of war)
            if (!(cell->flags & CELL_FLAG_REVEALED)) {
                continue;
            }

            // Calculate screen position
            int screenX = cx * CELL_SIZE - g_viewport.x;
            int screenY = cy * CELL_SIZE - g_viewport.y;

            // Fog of war darkening
            BOOL inFog = !(cell->flags & CELL_FLAG_VISIBLE);

            // Try to render with terrain tiles first
            if (useTiles) {
                // Use cell coordinates as variant for visual variety
                int variant = (cx * 7 + cy * 13) % 20;
                if (Terrain_RenderTile(cell->terrain, variant, screenX, screenY)) {
                    // Tile rendered successfully
                    if (inFog) {
                        // Darken the tile for fog of war
                        Renderer_FillRect(screenX, screenY, CELL_SIZE, CELL_SIZE, 0x80); // Semi-transparent black
                    }
                    continue;
                }
            }

            // Fallback: Draw colored rectangles
            uint8_t color = g_terrainColors[cell->terrain];
            if (inFog) {
                color = 8; // Dark gray for fog
            }

            // Draw cell as filled rectangle
            Renderer_FillRect(screenX, screenY, CELL_SIZE - 1, CELL_SIZE - 1, color);

            // Add some visual variety based on terrain type
            if (cell->terrain == TERRAIN_TREE) {
                // Draw a simple tree shape
                Renderer_FillRect(screenX + 8, screenY + 4, 8, 12, 10);
                Renderer_FillRect(screenX + 10, screenY + 16, 4, 6, 6);
            } else if (cell->terrain == TERRAIN_ORE) {
                // Draw ore sparkles
                Renderer_PutPixel(screenX + 6, screenY + 6, 14);
                Renderer_PutPixel(screenX + 12, screenY + 10, 14);
                Renderer_PutPixel(screenX + 18, screenY + 8, 14);
            } else if (cell->terrain == TERRAIN_ROCK) {
                // Draw rock texture
                Renderer_PutPixel(screenX + 4, screenY + 8, 7);
                Renderer_PutPixel(screenX + 12, screenY + 4, 7);
                Renderer_PutPixel(screenX + 16, screenY + 14, 7);
            }
        }
    }

    // Draw grid lines (optional, for debug)
    #if 0
    for (int cx = startCellX; cx <= endCellX; cx++) {
        int screenX = cx * CELL_SIZE - g_viewport.x;
        Render_DrawLine(screenX, 0, screenX, g_viewport.height, 8);
    }
    for (int cy = startCellY; cy <= endCellY; cy++) {
        int screenY = cy * CELL_SIZE - g_viewport.y;
        Render_DrawLine(0, screenY, g_viewport.width, screenY, 8);
    }
    #endif
}

void Map_Update(void) {
    // Future: animate water, ore sparkles, etc.
}

//===========================================================================
// Fog of War Functions
//===========================================================================

void Map_ClearVisibility(void) {
    if (!g_fogEnabled) return;

    // Clear VISIBLE flag on all cells (REVEALED stays set)
    for (int y = 0; y < g_mapHeight; y++) {
        for (int x = 0; x < g_mapWidth; x++) {
            g_cells[y][x].flags &= ~CELL_FLAG_VISIBLE;
        }
    }
}

void Map_RevealAround(int cellX, int cellY, int sightRange, int team) {
    // Only player team reveals fog
    if (team != 1) return;  // TEAM_PLAYER = 1

    // If fog disabled, just mark everything visible
    if (!g_fogEnabled) {
        for (int y = 0; y < g_mapHeight; y++) {
            for (int x = 0; x < g_mapWidth; x++) {
                g_cells[y][x].flags |= CELL_FLAG_REVEALED | CELL_FLAG_VISIBLE;
            }
        }
        return;
    }

    // Reveal in a circular area
    int rangeSquared = sightRange * sightRange;
    for (int dy = -sightRange; dy <= sightRange; dy++) {
        for (int dx = -sightRange; dx <= sightRange; dx++) {
            int cx = cellX + dx;
            int cy = cellY + dy;

            // Bounds check
            if (cx < 0 || cx >= g_mapWidth || cy < 0 || cy >= g_mapHeight) continue;

            // Circle check
            if (dx * dx + dy * dy > rangeSquared) continue;

            // Mark as revealed and visible
            g_cells[cy][cx].flags |= CELL_FLAG_REVEALED | CELL_FLAG_VISIBLE;
        }
    }
}

BOOL Map_IsCellVisible(int cellX, int cellY) {
    if (!g_fogEnabled) return TRUE;

    MapCell* cell = Map_GetCell(cellX, cellY);
    if (!cell) return FALSE;
    return (cell->flags & CELL_FLAG_VISIBLE) != 0;
}

BOOL Map_IsCellRevealed(int cellX, int cellY) {
    if (!g_fogEnabled) return TRUE;

    MapCell* cell = Map_GetCell(cellX, cellY);
    if (!cell) return FALSE;
    return (cell->flags & CELL_FLAG_REVEALED) != 0;
}

void Map_SetFogEnabled(BOOL enabled) {
    g_fogEnabled = enabled;
    if (!enabled) {
        // When disabling fog, reveal entire map
        for (int y = 0; y < g_mapHeight; y++) {
            for (int x = 0; x < g_mapWidth; x++) {
                g_cells[y][x].flags |= CELL_FLAG_REVEALED | CELL_FLAG_VISIBLE;
            }
        }
    }
}

BOOL Map_IsFogEnabled(void) {
    return g_fogEnabled;
}
