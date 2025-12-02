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

// Mission terrain data (for rendering with Terrain_RenderByID)
static const uint8_t* g_missionTerrainType = nullptr;  // Template IDs
static const uint8_t* g_missionTerrainIcon = nullptr;  // Tile indices
static const uint8_t* g_missionOverlayType = nullptr;  // Overlay types
static const uint8_t* g_missionOverlayData = nullptr;  // Overlay variants
static int g_missionMapX = 0;  // Map viewport offset
static int g_missionMapY = 0;
static bool g_useMissionTerrain = false;

// Red Alert OverlayType values (from DEFINES.H)
enum OverlayTypeRA {
    OVERLAY_RA_SANDBAG_WALL = 0,
    OVERLAY_RA_CYCLONE_WALL = 1,
    OVERLAY_RA_BRICK_WALL = 2,
    OVERLAY_RA_BARBWIRE_WALL = 3,
    OVERLAY_RA_WOOD_WALL = 4,
    OVERLAY_RA_GOLD1 = 5,       // Ore
    OVERLAY_RA_GOLD2 = 6,
    OVERLAY_RA_GOLD3 = 7,
    OVERLAY_RA_GOLD4 = 8,
    OVERLAY_RA_GEMS1 = 9,       // Gems
    OVERLAY_RA_GEMS2 = 10,
    OVERLAY_RA_GEMS3 = 11,
    OVERLAY_RA_GEMS4 = 12,
    OVERLAY_RA_V12 = 13,        // Haystacks
    OVERLAY_RA_V13 = 14,
    OVERLAY_RA_V14 = 15,        // Wheat
    OVERLAY_RA_V15 = 16,
    OVERLAY_RA_V16 = 17,        // Corn
    OVERLAY_RA_V17 = 18,
    OVERLAY_RA_V18 = 19,
    OVERLAY_RA_FLAG_SPOT = 20,
    OVERLAY_RA_WOOD_CRATE = 21,
    OVERLAY_RA_STEEL_CRATE = 22,
    OVERLAY_RA_FENCE = 23,
    OVERLAY_RA_WATER_CRATE = 24,
    OVERLAY_RA_NONE = 255
};
// Viewport dimensions (game view area, excluding sidebar)
static constexpr int GAME_VIEW_WIDTH = 560;   // w/o sidebar
static constexpr int GAME_VIEW_HEIGHT = 368;  // w/o HUD

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
            int r2 = radius * radius;
            if (dist2 <= r2 && g_cells[y][x].terrain == TERRAIN_CLEAR) {
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
            int r2 = radius * radius;
            if (dist2 <= r2 && g_cells[y][x].terrain == TERRAIN_CLEAR) {
                if (rand() % 3 != 0) {  // 67% density
                    Map_SetTerrain(x, y, TERRAIN_ORE);
                    // Ore amount: more in center, less at edges
                    int rr1 = r2 + 1;
                    int amount = ORE_MAX_AMOUNT - (dist2 * 100 / rr1);
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

void Map_LoadFromMission(const uint8_t* terrainType, const uint8_t* terrainIcon,
                         const uint8_t* overlayType, const uint8_t* overlayData,
                         int mapX, int mapY, int mapWidth, int mapHeight) {
    // Store mission terrain data for rendering
    g_missionTerrainType = terrainType;
    g_missionTerrainIcon = terrainIcon;
    g_missionOverlayType = overlayType;
    g_missionOverlayData = overlayData;
    g_missionMapX = mapX;
    g_missionMapY = mapY;
    g_useMissionTerrain = (terrainType != nullptr && terrainIcon != nullptr);

    // Create map with mission dimensions
    // Red Alert maps are 128x128 internally, but visible area is smaller
    Map_Create(mapWidth, mapHeight);

    // Map mission terrain to our TerrainType enum for passability
    // The terrainType array contains template IDs from MapPack
    // We need to map these to passability categories
    if (g_useMissionTerrain) {
        for (int y = 0; y < mapHeight; y++) {
            for (int x = 0; x < mapWidth; x++) {
                // Get cell in the full 128x128 array
                int fullX = mapX + x;
                int fullY = mapY + y;
                int cellIdx = fullY * 128 + fullX;

                if (cellIdx < 0 || cellIdx >= 128 * 128) continue;

                uint8_t templateID = terrainType[cellIdx];

                // Map template IDs to terrain types for passability
                // Based on OpenRA snow.yaml / temperat.yaml analysis:
                // 0, 255 (0xFF) = Clear
                // 1-2 = Water
                // 3-58 = Shore (partially passable)
                // 59-134 = Water cliffs (impassable)
                // 135-172 = Roads/slopes
                // 173-212 = Debris/rocks
                // 213-252 = River (water)
                // 253-260 = Bridges

                TerrainType terrain = TERRAIN_CLEAR;

                if (templateID == 0 || templateID == 255) {
                    terrain = TERRAIN_CLEAR;
                } else if (templateID >= 1 && templateID <= 2) {
                    terrain = TERRAIN_WATER;
                } else if (templateID >= 3 && templateID <= 58) {
                    // Shore tiles - check tile index for water vs land
                    // For now, treat as passable (land portion)
                    terrain = TERRAIN_CLEAR;
                } else if (templateID >= 59 && templateID <= 134) {
                    // Water cliffs - impassable
                    terrain = TERRAIN_ROCK;
                } else if (templateID >= 135 && templateID <= 172) {
                    // Roads/slopes - passable, faster movement
                    terrain = TERRAIN_ROAD;
                } else if (templateID >= 173 && templateID <= 212) {
                    // Debris/rocks - some passable, some not
                    // Treat as rough terrain (passable)
                    terrain = TERRAIN_CLEAR;
                } else if (templateID >= 213 && templateID <= 252) {
                    // River - water
                    terrain = TERRAIN_WATER;
                } else if (templateID >= 253) {
                    // Bridges (253-255) - passable over water
                    terrain = TERRAIN_BRIDGE;
                }

                g_cells[y][x].terrain = (uint8_t)terrain;
                g_cells[y][x].flags = 0;
                g_cells[y][x].height = 0;
                g_cells[y][x].oreAmount = 0;
                g_cells[y][x].unitId = -1;
                g_cells[y][x].buildingId = -1;

                // Process overlay data for ore/gems
                if (overlayType) {
                    uint8_t overlay = overlayType[cellIdx];
                    uint8_t variant = overlayData ? overlayData[cellIdx] : 0;

                    // Map Red Alert overlay types to terrain
                    bool isOre = overlay >= OVERLAY_RA_GOLD1 &&
                                 overlay <= OVERLAY_RA_GOLD4;
                    bool isGem = overlay >= OVERLAY_RA_GEMS1 &&
                                 overlay <= OVERLAY_RA_GEMS4;
                    if (isOre) {
                        // Ore (GOLD1-4) - different amounts
                        g_cells[y][x].terrain = TERRAIN_ORE;
                        int base = 50 + (overlay - OVERLAY_RA_GOLD1) * 50;
                        int bonus = (variant % 12) * 10;
                        g_cells[y][x].oreAmount = (uint8_t)(base + bonus);
                    } else if (isGem) {
                        // Gems - worth more than ore
                        g_cells[y][x].terrain = TERRAIN_GEM;
                        int base = 100 + (overlay - OVERLAY_RA_GEMS1) * 40;
                        int bonus = (variant % 4) * 20;
                        g_cells[y][x].oreAmount = (uint8_t)(base + bonus);
                    } else if (overlay >= OVERLAY_RA_SANDBAG_WALL &&
                               overlay <= OVERLAY_RA_WOOD_WALL) {
                        // Walls - impassable
                        g_cells[y][x].terrain = TERRAIN_ROCK;
                    } else if (overlay == OVERLAY_RA_FENCE) {
                        // Fence - impassable
                        g_cells[y][x].terrain = TERRAIN_ROCK;
                    }
                    // V12-V18 (vegetation) stays as clear terrain
                    // Crates handled separately (not terrain)
                }
            }
        }
    }

    // Center viewport on map
    g_viewport.x = 0;
    g_viewport.y = 0;
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
    int newX = worldX - g_viewport.width / 2;
    int newY = worldY - g_viewport.height / 2;
    Map_SetViewport(newX, newY);
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

            // Calculate screen position
            int screenX = cx * CELL_SIZE - g_viewport.x;
            int screenY = cy * CELL_SIZE - g_viewport.y;

            // Fog of war states:
            // 1. Never seen (shroud) - render black
            // 2. Seen but not visible (fog) - render terrain greyed out
            // 3. Currently visible - render terrain normally

            BOOL isRevealed = (cell->flags & CELL_FLAG_REVEALED) != 0;
            BOOL isVisible = (cell->flags & CELL_FLAG_VISIBLE) != 0;

            // Unrevealed cells (shroud) - draw black
            if (!isRevealed) {
                // Black (shroud)
                Renderer_FillRect(screenX, screenY, CELL_SIZE, CELL_SIZE, 0);
                continue;
            }

            // Revealed but not visible (fog) - show terrain but dimmed
            BOOL inFog = !isVisible;

            // Try mission terrain data first (actual map tiles)
            bool hasMissionData = g_useMissionTerrain &&
                g_missionTerrainType && g_missionTerrainIcon;
            if (hasMissionData) {
                // Get cell in the full 128x128 array
                int fullX = g_missionMapX + cx;
                int fullY = g_missionMapY + cy;
                int cellIdx = fullY * 128 + fullX;

                if (cellIdx >= 0 && cellIdx < 128 * 128) {
                    int templateID = g_missionTerrainType[cellIdx];
                    int tileIndex = g_missionTerrainIcon[cellIdx];

                    // Render using the template ID from the mission
                    bool ok = Terrain_RenderByID(templateID, tileIndex,
                                                 screenX, screenY);
                    if (ok) {
                        if (inFog) {
                            Renderer_SetAlpha(screenX, screenY,
                                              CELL_SIZE, CELL_SIZE, 128);
                        }
                        continue;
                    }
                }
            }

            // Fallback: Try procedural terrain tiles
            if (useTiles) {
                // Use cell coordinates as variant for visual variety
                int variant = (cx * 7 + cy * 13) % 20;
                bool ok = Terrain_RenderTile(cell->terrain, variant,
                                             screenX, screenY);
                if (ok) {
                    // Tile rendered - dim if in fog using alpha
                    if (inFog) {
                        Renderer_SetAlpha(screenX, screenY,
                                          CELL_SIZE, CELL_SIZE, 128);
                    }
                    continue;
                }
            }

            // Fallback: Draw colored rectangles
            uint8_t color = g_terrainColors[cell->terrain];

            // Draw cell as filled rectangle
            int sz = CELL_SIZE - 1;
            Renderer_FillRect(screenX, screenY, sz, sz, color);

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

            // Dim the cell if in fog using alpha
            if (inFog) {
                Renderer_SetAlpha(screenX, screenY, CELL_SIZE, CELL_SIZE, 128);
            }
        }
    }
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
            bool outOfBounds = cx < 0 || cx >= g_mapWidth ||
                               cy < 0 || cy >= g_mapHeight;
            if (outOfBounds) continue;

            // Circle check
            if (dx * dx + dy * dy > rangeSquared) continue;

            // Mark as revealed and visible
            uint8_t vis = CELL_FLAG_REVEALED | CELL_FLAG_VISIBLE;
            g_cells[cy][cx].flags |= vis;
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

void Map_RevealAll(void) {
    // Reveal all cells permanently
    for (int y = 0; y < g_mapHeight; y++) {
        for (int x = 0; x < g_mapWidth; x++) {
            g_cells[y][x].flags |= (CELL_FLAG_VISIBLE | CELL_FLAG_REVEALED);
        }
    }
}

void Map_RevealArea(int worldX, int worldY, int radius) {
    // Convert world coordinates to cell coordinates
    int cellX = worldX / CELL_SIZE;
    int cellY = worldY / CELL_SIZE;
    int cellRadius = radius / CELL_SIZE;
    if (cellRadius < 1) cellRadius = 1;

    // Reveal cells in a square area (simpler than circular)
    for (int dy = -cellRadius; dy <= cellRadius; dy++) {
        for (int dx = -cellRadius; dx <= cellRadius; dx++) {
            int tx = cellX + dx;
            int ty = cellY + dy;
            bool inBounds = tx >= 0 && tx < g_mapWidth &&
                            ty >= 0 && ty < g_mapHeight;
            if (inBounds) {
                uint8_t vis = CELL_FLAG_VISIBLE | CELL_FLAG_REVEALED;
                g_cells[ty][tx].flags |= vis;
            }
        }
    }
}
