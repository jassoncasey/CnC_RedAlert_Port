/**
 * Red Alert macOS Port - Game UI Implementation
 *
 * Renders sidebar, radar minimap, and selection panel for gameplay.
 * Integrates with production system for building units.
 * Uses standard Westwood palette indices for colors.
 */

#include "game_ui.h"
#include "../graphics/metal/renderer.h"
#include "../game/map.h"
#include "../game/units.h"
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cstdlib>

//===========================================================================
// Westwood Standard Palette Colors (from WWSTD.H)
//===========================================================================

enum PalColor {
    PAL_TBLACK  = 0,   // Transparent black
    PAL_PURPLE  = 1,
    PAL_CYAN    = 2,
    PAL_GREEN   = 3,
    PAL_LTGREEN = 4,
    PAL_YELLOW  = 5,
    PAL_PINK    = 6,
    PAL_BROWN   = 7,
    PAL_RED     = 8,
    PAL_LTCYAN  = 9,
    PAL_LTBLUE  = 10,
    PAL_BLUE    = 11,
    PAL_BLACK   = 12,
    PAL_GREY    = 13,
    PAL_LTGREY  = 14,
    PAL_WHITE   = 15,
};

//===========================================================================
// Internal State
//===========================================================================

static bool g_uiInitialized = false;

// Animation counters
static int g_radarPulse = 0;
static int g_flashFrame = 0;

// Player credits
static int g_playerCredits = 5000;

// Production state for structures
static int g_structureProducing = -1;  // Index in g_structureDefs being built
static int g_structureProgress = 0;    // Progress 0-100

// Production state for units
static int g_unitProducing = -1;       // Index in g_unitDefs being built
static int g_unitProgress = 0;         // Progress 0-100

// Placement mode state
static bool g_placementMode = false;   // Are we placing a building?
static int g_placementType = -1;       // Index in g_structureDefs being placed
static int g_placementCellX = 0;       // Current cursor cell position
static int g_placementCellY = 0;
static bool g_placementValid = false;  // Is current placement position valid?

//===========================================================================
// Helper: Find player production building
//===========================================================================

/**
 * Find the player's production building for a unit type.
 * @param unitType The type of unit to produce
 * @return Building pointer, or nullptr if not found
 */
static Building* FindProductionBuilding(UnitType unitType) {
    // Determine which building type produces this unit
    BuildingType productionType = BUILDING_NONE;

    switch (unitType) {
        case UNIT_RIFLE:
        case UNIT_GRENADIER:
        case UNIT_ROCKET:
        case UNIT_ENGINEER:
            productionType = BUILDING_BARRACKS;
            break;
        case UNIT_HARVESTER:
            productionType = BUILDING_REFINERY;
            break;
        case UNIT_TANK_LIGHT:
        case UNIT_TANK_MEDIUM:
        case UNIT_TANK_HEAVY:
        case UNIT_APC:
        case UNIT_ARTILLERY:
            productionType = BUILDING_FACTORY;
            break;
        default:
            return nullptr;
    }

    // Find player's building of that type
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bldg = Buildings_Get(i);
        if (!bldg || !bldg->active) continue;
        if (bldg->team != TEAM_PLAYER) continue;
        if (bldg->type == productionType) {
            return bldg;
        }
    }

    return nullptr;
}

/**
 * Find a valid spawn location near a building.
 * Searches in expanding rings around the building for passable terrain.
 * @param bldg Building to spawn near
 * @param outX Output world X coordinate
 * @param outY Output world Y coordinate
 * @return true if valid location found
 */
static bool FindSpawnLocationNearBuilding(Building* bldg, int* outX, int* outY) {
    if (!bldg) return false;

    int centerCellX = bldg->cellX + bldg->width / 2;
    int centerCellY = bldg->cellY + bldg->height;  // Exit at bottom

    // Search in expanding rings around the building exit
    for (int radius = 0; radius <= 5; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Only check cells on the perimeter of current ring
                if (radius > 0 && abs(dx) != radius && abs(dy) != radius) continue;

                int cx = centerCellX + dx;
                int cy = centerCellY + dy;

                if (cx < 0 || cy < 0 || cx >= Map_GetWidth() || cy >= Map_GetHeight()) continue;

                MapCell* cell = Map_GetCell(cx, cy);
                if (!cell) continue;

                // Check if cell is passable and unoccupied
                if (cell->terrain == TERRAIN_WATER ||
                    cell->terrain == TERRAIN_ROCK ||
                    cell->terrain == TERRAIN_BUILDING) continue;
                if (cell->unitId >= 0) continue;
                if (cell->buildingId >= 0) continue;

                // Found valid spawn point
                *outX = cx * CELL_SIZE + CELL_SIZE / 2;
                *outY = cy * CELL_SIZE + CELL_SIZE / 2;
                return true;
            }
        }
    }

    return false;
}

//===========================================================================
// Build Item Definitions
//===========================================================================

// Building type flags for prerequisite tracking
enum PrereqFlag {
    PREREQ_NONE      = 0,
    PREREQ_POWER     = (1 << 0),   // Requires Power Plant
    PREREQ_BARRACKS  = (1 << 1),   // Requires Barracks
    PREREQ_REFINERY  = (1 << 2),   // Requires Ore Refinery
    PREREQ_FACTORY   = (1 << 3),   // Requires War Factory
    PREREQ_RADAR     = (1 << 4),   // Requires Radar Dome
    PREREQ_TECH      = (1 << 5),   // Requires Tech Center
    PREREQ_CONYARD   = (1 << 6),   // Requires Construction Yard
};

struct BuildItemDef {
    const char* name;
    const char* fullName;
    int cost;
    int buildTime;  // Frames to complete at normal speed
    bool isUnit;    // true = unit, false = structure
    int spawnType;  // UnitType or BuildingType to spawn
    int width;      // Building width in cells (structures only)
    int height;     // Building height in cells (structures only)
    uint32_t prerequisites; // Bitmask of PrereqFlag requirements
};

// Available structures with proper tech tree
// Original tech tree:
//   Construction Yard (starting building)
//   └─ Power Plant (no prereq)
//       ├─ Barracks (power) → Infantry
//       ├─ Ore Refinery (power) → Harvester
//       └─ War Factory (power + refinery) → Vehicles
//           └─ Radar Dome (power + factory)
//               └─ Tech Center → Advanced units
static BuildItemDef g_structureDefs[] = {
    {"POWR", "Power Plant",   300,  300, false, BUILDING_POWER,    2, 2, PREREQ_NONE},
    {"PROC", "Ore Refinery", 2000,  600, false, BUILDING_REFINERY, 3, 3, PREREQ_POWER},
    {"TENT", "Barracks",      500,  400, false, BUILDING_BARRACKS, 2, 2, PREREQ_POWER},
    {"WEAP", "War Factory",  2000,  600, false, BUILDING_FACTORY,  3, 3, PREREQ_POWER | PREREQ_REFINERY},
    {"DOME", "Radar Dome",   1000,  500, false, BUILDING_RADAR,    2, 2, PREREQ_POWER | PREREQ_FACTORY},
};
static const int g_structureDefCount = 5;

// Available units with proper tech tree
// Infantry requires Barracks, Vehicles require War Factory
// Harvester requires Refinery (auto-spawned when refinery built, but also buildable)
static BuildItemDef g_unitDefs[] = {
    {"E1",   "Rifle Infantry",  100, 150, true, UNIT_RIFLE,       1, 1, PREREQ_BARRACKS},
    {"E2",   "Grenadier",       160, 180, true, UNIT_GRENADIER,   1, 1, PREREQ_BARRACKS},
    {"E3",   "Rocket Soldier",  300, 200, true, UNIT_ROCKET,      1, 1, PREREQ_BARRACKS},
    {"ENG",  "Engineer",        500, 200, true, UNIT_ENGINEER,    1, 1, PREREQ_BARRACKS},
    {"HARV", "Harvester",      1400, 400, true, UNIT_HARVESTER,   1, 1, PREREQ_REFINERY},
    {"1TNK", "Light Tank",      700, 300, true, UNIT_TANK_LIGHT,  1, 1, PREREQ_FACTORY},
    {"2TNK", "Medium Tank",     800, 350, true, UNIT_TANK_MEDIUM, 1, 1, PREREQ_FACTORY},
};
static const int g_unitDefCount = 7;

// Player-owned buildings bitmask (dynamically tracked)
static uint32_t g_playerBuildings = 0;

//===========================================================================
// Initialization
//===========================================================================

// Scan player buildings and update prerequisite bitmask
static void UpdatePlayerBuildings(void) {
    g_playerBuildings = 0;

    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bldg = Buildings_Get(i);
        if (!bldg || !bldg->active) continue;
        if (bldg->team != TEAM_PLAYER) continue;

        switch (bldg->type) {
            case BUILDING_CONSTRUCTION:
                g_playerBuildings |= PREREQ_CONYARD;
                break;
            case BUILDING_POWER:
                g_playerBuildings |= PREREQ_POWER;
                break;
            case BUILDING_BARRACKS:
                g_playerBuildings |= PREREQ_BARRACKS;
                break;
            case BUILDING_REFINERY:
                g_playerBuildings |= PREREQ_REFINERY;
                break;
            case BUILDING_FACTORY:
                g_playerBuildings |= PREREQ_FACTORY;
                break;
            case BUILDING_RADAR:
                g_playerBuildings |= PREREQ_RADAR;
                break;
            // BUILDING_TECH doesn't exist yet - tech center would go here
            default:
                break;
        }
    }
}

void GameUI_Init(void) {
    g_uiInitialized = true;
    g_radarPulse = 0;
    g_flashFrame = 0;
    g_playerCredits = 5000;
    g_structureProducing = -1;
    g_structureProgress = 0;
    g_unitProducing = -1;
    g_unitProgress = 0;
    g_placementMode = false;
    g_placementType = -1;
    g_placementValid = false;
    g_playerBuildings = 0;

    // Initial scan of buildings
    UpdatePlayerBuildings();

    // Connect credits to harvester system
    Units_SetCreditsPtr(&g_playerCredits);
}

void GameUI_Shutdown(void) {
    g_uiInitialized = false;
}

//===========================================================================
// Check Prerequisites
//===========================================================================

static bool CheckPrerequisites(const BuildItemDef* item) {
    // Check if player has all required buildings
    return (item->prerequisites & g_playerBuildings) == item->prerequisites;
}

// Get the name of the first missing prerequisite for an item (currently unused but kept for tooltip support)
static const char* GetMissingPrereq(const BuildItemDef* item) __attribute__((unused));
static const char* GetMissingPrereq(const BuildItemDef* item) {
    uint32_t missing = item->prerequisites & ~g_playerBuildings;

    if (missing & PREREQ_POWER)    return "Power Plant";
    if (missing & PREREQ_BARRACKS) return "Barracks";
    if (missing & PREREQ_REFINERY) return "Refinery";
    if (missing & PREREQ_FACTORY)  return "War Factory";
    if (missing & PREREQ_RADAR)    return "Radar";
    if (missing & PREREQ_TECH)     return "Tech Center";
    if (missing & PREREQ_CONYARD)  return "Const. Yard";

    return nullptr;
}

//===========================================================================
// Placement Validation
//===========================================================================

/**
 * Check if a building can be placed at the given cell position.
 * Validates terrain and existing buildings/units.
 */
static bool CanPlaceAt(int cellX, int cellY, int width, int height) {
    // Check all cells the building would occupy
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int cx = cellX + dx;
            int cy = cellY + dy;

            // Check map bounds
            if (cx < 0 || cy < 0 || cx >= Map_GetWidth() || cy >= Map_GetHeight()) {
                return false;
            }

            // Check terrain is passable (not water/rock/building)
            MapCell* cell = Map_GetCell(cx, cy);
            if (!cell) return false;

            if (cell->terrain == TERRAIN_WATER ||
                cell->terrain == TERRAIN_ROCK ||
                cell->terrain == TERRAIN_BUILDING) {
                return false;
            }

            // Check no existing building at this cell
            if (cell->buildingId >= 0) {
                return false;
            }

            // Check no unit occupying cell
            if (cell->unitId >= 0) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Update placement cursor position based on mouse coordinates.
 */
void GameUI_UpdatePlacement(int mouseX, int mouseY) {
    if (!g_placementMode || g_placementType < 0) return;

    // Only update if mouse is in game area (not sidebar)
    if (mouseX >= SIDEBAR_X) return;

    // Convert screen to world coordinates
    int worldX, worldY;
    Map_ScreenToWorld(mouseX, mouseY, &worldX, &worldY);

    // Convert to cell coordinates
    g_placementCellX = worldX / CELL_SIZE;
    g_placementCellY = worldY / CELL_SIZE;

    // Validate placement
    const BuildItemDef* item = &g_structureDefs[g_placementType];
    g_placementValid = CanPlaceAt(g_placementCellX, g_placementCellY, item->width, item->height);
}

/**
 * Attempt to place the building at current cursor position.
 * @return true if building was placed
 */
static bool TryPlaceBuilding(void) {
    if (!g_placementMode || g_placementType < 0) return false;
    if (!g_placementValid) return false;

    const BuildItemDef* item = &g_structureDefs[g_placementType];

    // Spawn the building
    int id = Buildings_Spawn((BuildingType)item->spawnType, TEAM_PLAYER,
                             g_placementCellX, g_placementCellY);
    if (id < 0) return false;

    // Mark cells as occupied
    for (int dy = 0; dy < item->height; dy++) {
        for (int dx = 0; dx < item->width; dx++) {
            MapCell* cell = Map_GetCell(g_placementCellX + dx, g_placementCellY + dy);
            if (cell) {
                cell->terrain = TERRAIN_BUILDING;
                cell->buildingId = id;
            }
        }
    }

    // Update player building flags to unlock new items
    UpdatePlayerBuildings();

    // Exit placement mode
    g_placementMode = false;
    g_structureProducing = -1;
    g_structureProgress = 0;
    g_placementType = -1;

    return true;
}

/**
 * Cancel placement mode (refund if desired, or just cancel).
 */
static void CancelPlacement(void) {
    if (!g_placementMode) return;

    // Refund the cost
    if (g_placementType >= 0) {
        g_playerCredits += g_structureDefs[g_placementType].cost;
    }

    g_placementMode = false;
    g_structureProducing = -1;
    g_structureProgress = 0;
    g_placementType = -1;
}

/**
 * Check if we're in placement mode.
 */
bool GameUI_IsPlacementMode(void) {
    return g_placementMode;
}

/**
 * Handle ESC key to cancel placement.
 */
bool GameUI_HandleEscape(void) {
    if (g_placementMode) {
        CancelPlacement();
        return true;
    }
    return false;
}

//===========================================================================
// Update
//===========================================================================

void GameUI_Update(void) {
    g_radarPulse = (g_radarPulse + 1) % 30;
    g_flashFrame = (g_flashFrame + 1) % 20;

    // Refresh player building flags (in case buildings were destroyed)
    UpdatePlayerBuildings();

    // Update structure production
    if (g_structureProducing >= 0 && !g_placementMode) {
        BuildItemDef* item = &g_structureDefs[g_structureProducing];

        // Calculate progress increment (100% over buildTime frames)
        int progressPerFrame = (100 * 100) / item->buildTime;  // Fixed point
        g_structureProgress += progressPerFrame;

        if (g_structureProgress >= 10000) {  // 100.00%
            // Structure complete - enter placement mode
            g_placementMode = true;
            g_placementType = g_structureProducing;
            g_structureProgress = 10000;  // Keep at 100%
        }
    }

    // Update unit production
    if (g_unitProducing >= 0) {
        BuildItemDef* item = &g_unitDefs[g_unitProducing];

        // Calculate progress increment
        int progressPerFrame = (100 * 100) / item->buildTime;
        g_unitProgress += progressPerFrame;

        if (g_unitProgress >= 10000) {  // 100.00%
            // Unit complete - find spawn location near production building
            int spawnX = 150 + (rand() % 100);  // Fallback
            int spawnY = 500 + (rand() % 100);

            Building* prodBldg = FindProductionBuilding((UnitType)item->spawnType);
            if (prodBldg) {
                int bldgSpawnX, bldgSpawnY;
                if (FindSpawnLocationNearBuilding(prodBldg, &bldgSpawnX, &bldgSpawnY)) {
                    spawnX = bldgSpawnX;
                    spawnY = bldgSpawnY;
                }
            }

            Units_Spawn((UnitType)item->spawnType, TEAM_PLAYER, spawnX, spawnY);

            g_unitProducing = -1;
            g_unitProgress = 0;
        }
    }
}

//===========================================================================
// Helper: Draw beveled box (3D effect)
//===========================================================================

static void DrawBeveledBox(int x, int y, int w, int h, uint8_t bgColor, bool raised) {
    // Fill background
    Renderer_FillRect(x, y, w, h, bgColor);

    // 3D bevel effect
    uint8_t highlight = raised ? PAL_LTGREY : PAL_BLACK;
    uint8_t shadow = raised ? PAL_BLACK : PAL_LTGREY;

    // Top and left (highlight)
    Renderer_HLine(x, x + w - 1, y, highlight);
    Renderer_VLine(x, y, y + h - 1, highlight);

    // Bottom and right (shadow)
    Renderer_HLine(x, x + w - 1, y + h - 1, shadow);
    Renderer_VLine(x + w - 1, y, y + h - 1, shadow);
}

//===========================================================================
// Placement Footprint Rendering
//===========================================================================

void GameUI_RenderPlacement(void) {
    if (!g_placementMode || g_placementType < 0) return;

    const BuildItemDef* item = &g_structureDefs[g_placementType];
    int width = item->width;
    int height = item->height;

    // Get viewport for screen coordinate conversion
    Viewport* vp = Map_GetViewport();
    if (!vp) return;

    // Calculate screen position of top-left corner
    int worldX = g_placementCellX * CELL_SIZE;
    int worldY = g_placementCellY * CELL_SIZE;
    int screenX = worldX - vp->x;
    int screenY = worldY - vp->y;

    // Don't draw if completely off screen
    if (screenX + width * CELL_SIZE < 0 || screenX >= SIDEBAR_X) return;
    if (screenY + height * CELL_SIZE < 0 || screenY >= SIDEBAR_HEIGHT) return;

    // Choose color based on validity - pulsing effect
    uint8_t baseColor = g_placementValid ? PAL_LTGREEN : PAL_RED;
    uint8_t pulseColor = g_placementValid ? PAL_GREEN : PAL_PINK;
    uint8_t color = (g_flashFrame < 10) ? baseColor : pulseColor;

    // Draw individual cells to show footprint
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            int cellScreenX = screenX + dx * CELL_SIZE;
            int cellScreenY = screenY + dy * CELL_SIZE;

            // Skip if this cell is off screen
            if (cellScreenX < 0 || cellScreenX >= SIDEBAR_X) continue;
            if (cellScreenY < 0 || cellScreenY >= SIDEBAR_HEIGHT) continue;

            // Check if this specific cell is valid
            MapCell* cell = Map_GetCell(g_placementCellX + dx, g_placementCellY + dy);
            bool cellValid = cell &&
                            cell->terrain != TERRAIN_WATER &&
                            cell->terrain != TERRAIN_ROCK &&
                            cell->terrain != TERRAIN_BUILDING &&
                            cell->buildingId < 0 &&
                            cell->unitId < 0;

            uint8_t cellColor = cellValid ? color : PAL_RED;

            // Draw cell outline
            Renderer_DrawRect(cellScreenX, cellScreenY, CELL_SIZE, CELL_SIZE, cellColor);

            // Draw X pattern for invalid cells
            if (!cellValid) {
                Renderer_DrawLine(cellScreenX + 2, cellScreenY + 2,
                                  cellScreenX + CELL_SIZE - 3, cellScreenY + CELL_SIZE - 3, PAL_RED);
                Renderer_DrawLine(cellScreenX + CELL_SIZE - 3, cellScreenY + 2,
                                  cellScreenX + 2, cellScreenY + CELL_SIZE - 3, PAL_RED);
            }
        }
    }

    // Draw outer boundary
    Renderer_DrawRect(screenX, screenY, width * CELL_SIZE, height * CELL_SIZE, color);

    // Draw building name above cursor
    if (screenY > 12) {
        Renderer_DrawText(item->name, screenX + 2, screenY - 10, color, 0);
    }
}

//===========================================================================
// Main Render
//===========================================================================

void GameUI_Render(void) {
    if (!g_uiInitialized) return;

    // Draw sidebar background - dark with bevel
    Renderer_FillRect(SIDEBAR_X, SIDEBAR_Y, SIDEBAR_WIDTH, SIDEBAR_HEIGHT, PAL_BLACK);

    // Left border of sidebar (separating from game view)
    Renderer_VLine(SIDEBAR_X, 0, SIDEBAR_HEIGHT - 1, PAL_GREY);
    Renderer_VLine(SIDEBAR_X + 1, 0, SIDEBAR_HEIGHT - 1, PAL_BLACK);

    // Draw individual components
    GameUI_RenderRadar();
    GameUI_RenderSidebar();
    GameUI_RenderSelectionPanel();
    GameUI_RenderHUD();

    // Draw placement cursor (after sidebar so it's on top)
    GameUI_RenderPlacement();
}

//===========================================================================
// Input Handling
//===========================================================================

BOOL GameUI_HandleInput(int mouseX, int mouseY, BOOL leftClick, BOOL rightClick) {
    // Update placement cursor position (always, for smooth tracking)
    GameUI_UpdatePlacement(mouseX, mouseY);

    // Handle placement mode clicks in game area
    if (g_placementMode && mouseX < SIDEBAR_X) {
        if (leftClick) {
            // Try to place building
            if (TryPlaceBuilding()) {
                return TRUE;  // Building placed
            }
            // Invalid placement - don't consume click (user can see error)
            return TRUE;
        }
        if (rightClick) {
            // Right-click cancels placement
            CancelPlacement();
            return TRUE;
        }
        // In placement mode, game area is reserved for placement
        return TRUE;
    }

    // Check radar clicks
    if (mouseX >= RADAR_X && mouseX < RADAR_X + RADAR_WIDTH &&
        mouseY >= RADAR_Y && mouseY < RADAR_Y + RADAR_HEIGHT) {
        if (leftClick) {
            return GameUI_RadarClick(mouseX, mouseY);
        }
        return TRUE;
    }

    // Check sidebar clicks
    if (mouseX >= SIDEBAR_X) {
        if (leftClick) {
            return GameUI_SidebarClick(mouseX, mouseY, leftClick);
        }
        return TRUE;
    }

    return FALSE;
}

//===========================================================================
// Radar Implementation
//===========================================================================

// Radar rendering context shared between helper functions
struct RadarContext {
    int mapWidth, mapHeight;
    float scale;
    int offsetX, offsetY;
    bool fogEnabled;
};

static void CalcRadarContext(RadarContext* ctx) {
    ctx->mapWidth = Map_GetWidth();
    ctx->mapHeight = Map_GetHeight();
    float scaleX = (float)(RADAR_WIDTH - 4) / (float)(ctx->mapWidth);
    float scaleY = (float)(RADAR_HEIGHT - 4) / (float)(ctx->mapHeight);
    ctx->scale = (scaleX < scaleY) ? scaleX : scaleY;
    int displayW = (int)(ctx->mapWidth * ctx->scale);
    int displayH = (int)(ctx->mapHeight * ctx->scale);
    ctx->offsetX = RADAR_X + 2 + (RADAR_WIDTH - 4 - displayW) / 2;
    ctx->offsetY = RADAR_Y + 2 + (RADAR_HEIGHT - 4 - displayH) / 2;
    ctx->fogEnabled = Map_IsFogEnabled();
}

static uint8_t TerrainToRadarColor(uint8_t terrain) {
    switch (terrain) {
        case TERRAIN_WATER: return PAL_BLUE;
        case TERRAIN_ROCK:  return PAL_GREY;
        case TERRAIN_TREE:  return PAL_GREEN;
        case TERRAIN_ROAD:
        case TERRAIN_BRIDGE: return PAL_LTGREY;
        case TERRAIN_ORE:
        case TERRAIN_GEM:   return PAL_YELLOW;
        default:            return PAL_BROWN;
    }
}

static uint8_t DimColorForFog(uint8_t color) {
    if (color == PAL_BLUE || color == PAL_GREY || color == PAL_GREEN)
        return PAL_BLACK;
    if (color == PAL_LTGREY) return PAL_GREY;
    if (color == PAL_YELLOW) return PAL_BROWN;
    return PAL_BLACK;
}

static void RenderRadarTerrain(const RadarContext* ctx) {
    for (int cy = 0; cy < ctx->mapHeight; cy++) {
        for (int cx = 0; cx < ctx->mapWidth; cx++) {
            MapCell* cell = Map_GetCell(cx, cy);
            if (!cell) continue;
            int px = ctx->offsetX + (int)(cx * ctx->scale);
            int py = ctx->offsetY + (int)(cy * ctx->scale);
            if (ctx->fogEnabled && !(cell->flags & CELL_FLAG_REVEALED)) {
                Renderer_PutPixel(px, py, PAL_BLACK);
                continue;
            }
            uint8_t color = TerrainToRadarColor(cell->terrain);
            if (ctx->fogEnabled && !(cell->flags & CELL_FLAG_VISIBLE))
                color = DimColorForFog(color);
            Renderer_PutPixel(px, py, color);
        }
    }
}

static void RenderRadarUnits(const RadarContext* ctx) {
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;
        int cellX = unit->worldX / CELL_SIZE;
        int cellY = unit->worldY / CELL_SIZE;
        if (ctx->fogEnabled && unit->team != TEAM_PLAYER) {
            MapCell* cell = Map_GetCell(cellX, cellY);
            if (!cell || !(cell->flags & CELL_FLAG_VISIBLE)) continue;
        }
        int px = ctx->offsetX + (int)(cellX * ctx->scale);
        int py = ctx->offsetY + (int)(cellY * ctx->scale);
        uint8_t color = (unit->team == TEAM_PLAYER) ? PAL_LTGREEN : PAL_RED;
        Renderer_FillRect(px, py, 2, 2, color);
    }
}

static void RenderRadarBuildings(const RadarContext* ctx) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bldg = Buildings_Get(i);
        if (!bldg || !bldg->active) continue;
        if (ctx->fogEnabled && bldg->team != TEAM_PLAYER) {
            MapCell* cell = Map_GetCell(bldg->cellX, bldg->cellY);
            if (!cell || !(cell->flags & CELL_FLAG_REVEALED)) continue;
        }
        int px = ctx->offsetX + (int)(bldg->cellX * ctx->scale);
        int py = ctx->offsetY + (int)(bldg->cellY * ctx->scale);
        int pw = (int)(bldg->width * ctx->scale);
        int ph = (int)(bldg->height * ctx->scale);
        if (pw < 2) pw = 2;
        if (ph < 2) ph = 2;
        uint8_t color = (bldg->team == TEAM_PLAYER) ? PAL_LTGREEN : PAL_RED;
        Renderer_FillRect(px, py, pw, ph, color);
    }
}

static void RenderRadarViewport(const RadarContext* ctx) {
    Viewport* vp = Map_GetViewport();
    if (!vp) return;
    int vpCellX = vp->x / CELL_SIZE, vpCellY = vp->y / CELL_SIZE;
    int vpCellW = vp->width / CELL_SIZE, vpCellH = vp->height / CELL_SIZE;
    int vpx = ctx->offsetX + (int)(vpCellX * ctx->scale);
    int vpy = ctx->offsetY + (int)(vpCellY * ctx->scale);
    int vpw = (int)(vpCellW * ctx->scale);
    int vph = (int)(vpCellH * ctx->scale);
    uint8_t cursorColor = (g_radarPulse < 15) ? PAL_WHITE : PAL_LTGREEN;
    Renderer_DrawRect(vpx, vpy, vpw, vph, cursorColor);
}

void GameUI_RenderRadar(void) {
    DrawBeveledBox(RADAR_X - 2, RADAR_Y - 2,
                   RADAR_WIDTH + 4, RADAR_HEIGHT + 4, PAL_GREY, false);
    Renderer_FillRect(RADAR_X, RADAR_Y, RADAR_WIDTH, RADAR_HEIGHT, PAL_BLACK);

    int mapWidth = Map_GetWidth();
    int mapHeight = Map_GetHeight();
    if (mapWidth <= 0 || mapHeight <= 0) {
        Renderer_DrawText("RADAR", RADAR_X + 14, RADAR_Y + 28, PAL_GREY, 0);
        Renderer_DrawText("OFFLINE", RADAR_X + 10, RADAR_Y + 40, PAL_GREY, 0);
        return;
    }

    RadarContext ctx;
    CalcRadarContext(&ctx);
    RenderRadarTerrain(&ctx);
    RenderRadarUnits(&ctx);
    RenderRadarBuildings(&ctx);
    RenderRadarViewport(&ctx);
}

BOOL GameUI_RadarClick(int mouseX, int mouseY) {
    int mapWidth = Map_GetWidth();
    int mapHeight = Map_GetHeight();
    if (mapWidth <= 0 || mapHeight <= 0) return FALSE;

    float scaleX = (float)(RADAR_WIDTH - 4) / (float)(mapWidth);
    float scaleY = (float)(RADAR_HEIGHT - 4) / (float)(mapHeight);
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int displayWidth = (int)(mapWidth * scale);
    int displayHeight = (int)(mapHeight * scale);
    int offsetX = RADAR_X + 2 + (RADAR_WIDTH - 4 - displayWidth) / 2;
    int offsetY = RADAR_Y + 2 + (RADAR_HEIGHT - 4 - displayHeight) / 2;

    // Convert click to cell coordinates
    int cellX = (int)((mouseX - offsetX) / scale);
    int cellY = (int)((mouseY - offsetY) / scale);

    // Convert to world coordinates (center of cell)
    int worldX = cellX * CELL_SIZE + CELL_SIZE / 2;
    int worldY = cellY * CELL_SIZE + CELL_SIZE / 2;

    Map_CenterViewport(worldX, worldY);
    return TRUE;
}

void GameUI_RadarToWorld(int radarX, int radarY, int* worldX, int* worldY) {
    int mapWidth = Map_GetWidth();
    int mapHeight = Map_GetHeight();

    if (mapWidth <= 0 || mapHeight <= 0) {
        *worldX = 0;
        *worldY = 0;
        return;
    }

    float scaleX = (float)(RADAR_WIDTH - 4) / (float)(mapWidth);
    float scaleY = (float)(RADAR_HEIGHT - 4) / (float)(mapHeight);
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    int displayWidth = (int)(mapWidth * scale);
    int displayHeight = (int)(mapHeight * scale);
    int offsetX = RADAR_X + 2 + (RADAR_WIDTH - 4 - displayWidth) / 2;
    int offsetY = RADAR_Y + 2 + (RADAR_HEIGHT - 4 - displayHeight) / 2;

    int cellX = (int)((radarX - offsetX) / scale);
    int cellY = (int)((radarY - offsetY) / scale);

    *worldX = cellX * CELL_SIZE;
    *worldY = cellY * CELL_SIZE;
}

//===========================================================================
// Sidebar Implementation
//===========================================================================

// Button height reduced to fit all items
#define SIDEBAR_BUTTON_HEIGHT 16
#define SIDEBAR_BUTTON_SPACING 17

void GameUI_RenderSidebar(void) {
    int startY = STRIP_Y;
    int maxY = SELECTION_Y - 4;  // Don't go past selection panel

    // Section: STRUCTURES
    DrawBeveledBox(SIDEBAR_X + 3, startY, SIDEBAR_WIDTH - 6, 10, PAL_GREY, true);
    Renderer_DrawText("STRUCTURES", SIDEBAR_X + 8, startY + 1, PAL_BLACK, 0);
    startY += 12;

    // Structure buttons
    for (int i = 0; i < g_structureDefCount; i++) {
        if (startY + SIDEBAR_BUTTON_HEIGHT > maxY) break;  // Don't overflow

        const BuildItemDef* item = &g_structureDefs[i];
        bool hasPrereqs = CheckPrerequisites(item);
        bool canAfford = g_playerCredits >= item->cost;
        bool available = hasPrereqs && canAfford;
        bool isBuilding = (g_structureProducing == i);
        int progress = isBuilding ? (g_structureProgress / 100) : 0;  // Convert from fixed point

        uint8_t bgColor = available ? PAL_GREY : PAL_BLACK;
        uint8_t textColor = available ? PAL_WHITE : PAL_GREY;

        // Button with 3D effect
        DrawBeveledBox(SIDEBAR_X + 4, startY, SIDEBAR_WIDTH - 8, SIDEBAR_BUTTON_HEIGHT, bgColor, available && !isBuilding);

        // Item name and cost/status on same line
        Renderer_DrawText(item->name, SIDEBAR_X + 8, startY + 1, textColor, 0);

        // Cost, progress, or requirement (to the right of name)
        if (isBuilding) {
            if (g_placementMode && g_placementType == i) {
                // Ready for placement - pulsing text
                uint8_t readyColor = (g_flashFrame < 10) ? PAL_WHITE : PAL_LTGREEN;
                Renderer_DrawText("RDY", SIDEBAR_X + 44, startY + 1, readyColor, 0);
            } else {
                char progressStr[16];
                snprintf(progressStr, sizeof(progressStr), "%d%%", progress);
                Renderer_DrawText(progressStr, SIDEBAR_X + 44, startY + 1, PAL_LTGREEN, 0);

                // Progress bar below
                int barW = ((SIDEBAR_WIDTH - 16) * progress) / 100;
                Renderer_FillRect(SIDEBAR_X + 8, startY + 10, barW, 2, PAL_LTGREEN);
            }
        } else if (!hasPrereqs) {
            Renderer_DrawText("---", SIDEBAR_X + 44, startY + 1, PAL_GREY, 0);
        } else if (!canAfford) {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%d", item->cost);
            Renderer_DrawText(costStr, SIDEBAR_X + 8, startY + 8, PAL_RED, 0);
        } else {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%d", item->cost);
            Renderer_DrawText(costStr, SIDEBAR_X + 8, startY + 8, PAL_YELLOW, 0);
        }

        startY += SIDEBAR_BUTTON_SPACING;
    }

    // Placement hint (compact)
    if (g_placementMode) {
        Renderer_DrawText("Click to place", SIDEBAR_X + 6, startY, PAL_WHITE, 0);
        startY += 12;
    }

    startY += 2;

    // Section: UNITS
    if (startY + 12 < maxY) {
        DrawBeveledBox(SIDEBAR_X + 3, startY, SIDEBAR_WIDTH - 6, 10, PAL_GREY, true);
        Renderer_DrawText("UNITS", SIDEBAR_X + 8, startY + 1, PAL_BLACK, 0);
        startY += 12;
    }

    // Unit buttons
    for (int i = 0; i < g_unitDefCount; i++) {
        if (startY + SIDEBAR_BUTTON_HEIGHT > maxY) break;  // Don't overflow

        const BuildItemDef* item = &g_unitDefs[i];
        bool hasPrereqs = CheckPrerequisites(item);
        bool canAfford = g_playerCredits >= item->cost;
        bool available = hasPrereqs && canAfford;
        bool isBuilding = (g_unitProducing == i);
        int progress = isBuilding ? (g_unitProgress / 100) : 0;

        uint8_t bgColor = available ? PAL_GREY : PAL_BLACK;
        uint8_t textColor = available ? PAL_WHITE : PAL_GREY;

        DrawBeveledBox(SIDEBAR_X + 4, startY, SIDEBAR_WIDTH - 8, SIDEBAR_BUTTON_HEIGHT, bgColor, available && !isBuilding);

        Renderer_DrawText(item->name, SIDEBAR_X + 8, startY + 1, textColor, 0);

        if (isBuilding) {
            char progressStr[16];
            snprintf(progressStr, sizeof(progressStr), "%d%%", progress);
            Renderer_DrawText(progressStr, SIDEBAR_X + 44, startY + 1, PAL_LTGREEN, 0);

            int barW = ((SIDEBAR_WIDTH - 16) * progress) / 100;
            Renderer_FillRect(SIDEBAR_X + 8, startY + 10, barW, 2, PAL_LTGREEN);
        } else if (!hasPrereqs) {
            Renderer_DrawText("---", SIDEBAR_X + 44, startY + 1, PAL_GREY, 0);
        } else if (!canAfford) {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%d", item->cost);
            Renderer_DrawText(costStr, SIDEBAR_X + 8, startY + 8, PAL_RED, 0);
        } else {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%d", item->cost);
            Renderer_DrawText(costStr, SIDEBAR_X + 8, startY + 8, PAL_YELLOW, 0);
        }

        startY += SIDEBAR_BUTTON_SPACING;
    }
}

BOOL GameUI_SidebarClick(int mouseX, int mouseY, BOOL leftClick) {
    (void)leftClick;
    (void)mouseX;

    int startY = STRIP_Y + 12;  // After "STRUCTURES" header
    int maxY = SELECTION_Y - 4;

    // Check structure buttons
    for (int i = 0; i < g_structureDefCount; i++) {
        if (startY + SIDEBAR_BUTTON_HEIGHT > maxY) break;

        if (mouseY >= startY && mouseY < startY + SIDEBAR_BUTTON_HEIGHT) {
            BuildItemDef* item = &g_structureDefs[i];

            // Check if already building or in placement mode
            if (g_structureProducing >= 0 || g_placementMode) {
                return TRUE;
            }

            // Check prerequisites
            if (!CheckPrerequisites(item)) {
                return TRUE;  // Click consumed but no action
            }

            // Check if player can afford it
            if (g_playerCredits < item->cost) {
                return TRUE;  // Click consumed but no action
            }

            // Start production
            g_playerCredits -= item->cost;
            g_structureProducing = i;
            g_structureProgress = 0;
            return TRUE;
        }
        startY += SIDEBAR_BUTTON_SPACING;
    }

    // Skip placement hint area if visible
    if (g_placementMode) {
        startY += 12;
    }

    startY += 2 + 12;  // Skip gap and "UNITS" header

    // Check unit buttons
    for (int i = 0; i < g_unitDefCount; i++) {
        if (startY + SIDEBAR_BUTTON_HEIGHT > maxY) break;

        if (mouseY >= startY && mouseY < startY + SIDEBAR_BUTTON_HEIGHT) {
            BuildItemDef* item = &g_unitDefs[i];

            // Check if already building something
            if (g_unitProducing >= 0) {
                return TRUE;
            }

            // Check prerequisites
            if (!CheckPrerequisites(item)) {
                return TRUE;  // Click consumed but no action
            }

            // Check if player can afford it
            if (g_playerCredits < item->cost) {
                return TRUE;  // Click consumed but no action
            }

            // Start production
            g_playerCredits -= item->cost;
            g_unitProducing = i;
            g_unitProgress = 0;
            return TRUE;
        }
        startY += SIDEBAR_BUTTON_SPACING;
    }

    return FALSE;
}

//===========================================================================
// Selection Panel Implementation
//===========================================================================

void GameUI_RenderSelectionPanel(void) {
    // Panel frame
    DrawBeveledBox(SIDEBAR_X + 3, SELECTION_Y, SIDEBAR_WIDTH - 6, SELECTION_HEIGHT - 4, PAL_GREY, false);

    // Inner area
    Renderer_FillRect(SIDEBAR_X + 5, SELECTION_Y + 2, SIDEBAR_WIDTH - 10, SELECTION_HEIGHT - 8, PAL_BLACK);

    int selectedCount = Units_GetSelectedCount();

    if (selectedCount == 0) {
        Renderer_DrawText("No unit", SIDEBAR_X + 14, SELECTION_Y + 20, PAL_GREY, 0);
        Renderer_DrawText("selected", SIDEBAR_X + 12, SELECTION_Y + 32, PAL_GREY, 0);
        return;
    }

    int firstSelected = Units_GetFirstSelected();
    Unit* unit = Units_Get(firstSelected);
    if (!unit) return;

    // Unit type name
    const char* typeName = "Unknown";
    switch (unit->type) {
        case UNIT_RIFLE: typeName = "Rifle"; break;
        case UNIT_GRENADIER: typeName = "Grenadier"; break;
        case UNIT_ROCKET: typeName = "Rocket"; break;
        case UNIT_ENGINEER: typeName = "Engineer"; break;
        case UNIT_HARVESTER: typeName = "Harvester"; break;
        case UNIT_TANK_LIGHT: typeName = "Lt Tank"; break;
        case UNIT_TANK_MEDIUM: typeName = "Md Tank"; break;
        case UNIT_TANK_HEAVY: typeName = "Hv Tank"; break;
        case UNIT_APC: typeName = "APC"; break;
        case UNIT_ARTILLERY: typeName = "Artillery"; break;
        default: break;
    }

    Renderer_DrawText(typeName, SIDEBAR_X + 8, SELECTION_Y + 6, PAL_LTGREEN, 0);

    // Health bar
    int healthPct = (unit->maxHealth > 0) ? (unit->health * 100 / unit->maxHealth) : 0;
    int barWidth = SIDEBAR_WIDTH - 20;
    int greenWidth = (healthPct * barWidth) / 100;

    // Background (damage)
    Renderer_FillRect(SIDEBAR_X + 8, SELECTION_Y + 18, barWidth, 6, PAL_RED);

    // Health
    if (greenWidth > 0) {
        uint8_t healthColor = PAL_LTGREEN;
        if (healthPct <= 50) healthColor = PAL_YELLOW;
        if (healthPct <= 25) healthColor = PAL_RED;
        Renderer_FillRect(SIDEBAR_X + 8, SELECTION_Y + 18, greenWidth, 6, healthColor);
    }

    // Border
    Renderer_DrawRect(SIDEBAR_X + 8, SELECTION_Y + 18, barWidth, 6, PAL_GREY);

    // Health text
    char healthText[16];
    snprintf(healthText, sizeof(healthText), "%d/%d", unit->health, unit->maxHealth);
    Renderer_DrawText(healthText, SIDEBAR_X + 8, SELECTION_Y + 28, PAL_WHITE, 0);

    // Multi-select count
    if (selectedCount > 1) {
        char countText[16];
        snprintf(countText, sizeof(countText), "+%d more", selectedCount - 1);
        Renderer_DrawText(countText, SIDEBAR_X + 8, SELECTION_Y + 40, PAL_YELLOW, 0);
    }

    // Unit state
    const char* stateText = "Idle";
    uint8_t stateColor = PAL_LTGREY;
    switch (unit->state) {
        case STATE_MOVING: stateText = "Moving"; stateColor = PAL_LTCYAN; break;
        case STATE_ATTACKING: stateText = "Attack!"; stateColor = PAL_RED; break;
        case STATE_HARVESTING: stateText = "Harvest"; stateColor = PAL_YELLOW; break;
        case STATE_RETURNING: stateText = "Return"; stateColor = PAL_LTCYAN; break;
        case STATE_DYING: stateText = "Dying"; stateColor = PAL_RED; break;
        default: break;
    }
    Renderer_DrawText(stateText, SIDEBAR_X + 8, SELECTION_Y + 52, stateColor, 0);
}

//===========================================================================
// HUD Implementation
//===========================================================================

void GameUI_RenderHUD(void) {
    // Credits display in sidebar area at top
    DrawBeveledBox(SIDEBAR_X + 3, 1, SIDEBAR_WIDTH - 6, 14, PAL_BLACK, false);

    char creditsText[32];
    snprintf(creditsText, sizeof(creditsText), "$%d", g_playerCredits);
    Renderer_DrawText(creditsText, SIDEBAR_X + 8, 4, PAL_YELLOW, 0);
}
