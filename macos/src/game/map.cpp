/**
 * Red Alert macOS Port - Map/Terrain System Implementation
 */

#include "map.h"
#include "graphics/metal/renderer.h"
#include <cstdlib>
#include <cstring>

// Map state
static MapCell g_cells[MAP_MAX_HEIGHT][MAP_MAX_WIDTH];
static int g_mapWidth = 0;
static int g_mapHeight = 0;
static Viewport g_viewport = {0, 0, 640, 400};

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
            g_cells[y][x].overlay = 0;
            g_cells[y][x].unitId = -1;
            g_cells[y][x].buildingId = -1;
        }
    }

    // Reset viewport
    g_viewport.x = 0;
    g_viewport.y = 0;
}

void Map_GenerateDemo(void) {
    // Create a 64x64 demo map
    Map_Create(64, 64);

    // Simple procedural generation
    // Add some water (lake in center-ish)
    for (int y = 20; y < 30; y++) {
        for (int x = 25; x < 40; x++) {
            // Oval shape
            int dx = x - 32;
            int dy = y - 25;
            if (dx * dx + dy * dy * 2 < 80) {
                Map_SetTerrain(x, y, TERRAIN_WATER);
            }
        }
    }

    // Add a river
    for (int y = 0; y < 20; y++) {
        int rx = 30 + (y % 4) - 1;
        Map_SetTerrain(rx, y, TERRAIN_WATER);
        Map_SetTerrain(rx + 1, y, TERRAIN_WATER);
    }

    // Add some rocks/cliffs
    for (int i = 0; i < 30; i++) {
        int rx = rand() % g_mapWidth;
        int ry = rand() % g_mapHeight;
        if (g_cells[ry][rx].terrain == TERRAIN_CLEAR) {
            Map_SetTerrain(rx, ry, TERRAIN_ROCK);
            // Cluster nearby
            if (rx > 0) Map_SetTerrain(rx - 1, ry, TERRAIN_ROCK);
            if (rx < g_mapWidth - 1) Map_SetTerrain(rx + 1, ry, TERRAIN_ROCK);
        }
    }

    // Add trees/forests
    for (int i = 0; i < 80; i++) {
        int rx = rand() % g_mapWidth;
        int ry = rand() % g_mapHeight;
        if (g_cells[ry][rx].terrain == TERRAIN_CLEAR) {
            Map_SetTerrain(rx, ry, TERRAIN_TREE);
        }
    }

    // Add a road
    for (int x = 0; x < 20; x++) {
        Map_SetTerrain(x, 32, TERRAIN_ROAD);
    }
    for (int y = 32; y < 50; y++) {
        Map_SetTerrain(19, y, TERRAIN_ROAD);
    }

    // Add ore fields
    for (int i = 0; i < 5; i++) {
        int ox = 45 + rand() % 15;
        int oy = 40 + rand() % 15;
        for (int dy = -2; dy <= 2; dy++) {
            for (int dx = -2; dx <= 2; dx++) {
                int tx = ox + dx;
                int ty = oy + dy;
                if (tx >= 0 && tx < g_mapWidth && ty >= 0 && ty < g_mapHeight) {
                    if (g_cells[ty][tx].terrain == TERRAIN_CLEAR && rand() % 3 != 0) {
                        Map_SetTerrain(tx, ty, TERRAIN_ORE);
                    }
                }
            }
        }
    }

    // Add a bridge over the river
    Map_SetTerrain(30, 10, TERRAIN_BRIDGE);
    Map_SetTerrain(31, 10, TERRAIN_BRIDGE);

    // Reveal everything for demo
    for (int y = 0; y < g_mapHeight; y++) {
        for (int x = 0; x < g_mapWidth; x++) {
            g_cells[y][x].flags |= CELL_FLAG_REVEALED | CELL_FLAG_VISIBLE;
        }
    }
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

            // Get terrain color
            uint8_t color = g_terrainColors[cell->terrain];

            // Darken if not currently visible (fog of war - revealed but not visible)
            if (!(cell->flags & CELL_FLAG_VISIBLE)) {
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
