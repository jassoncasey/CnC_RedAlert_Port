/**
 * Red Alert macOS Port - Game UI Implementation
 *
 * Renders sidebar, radar minimap, and selection panel for gameplay.
 * Integrates with production system for building units.
 * Uses standard Westwood palette indices for colors.
 */

#include "game_ui.h"
#include "menu.h"
#include "../graphics/metal/renderer.h"
#include "../game/map.h"
#include "../game/units.h"
#include "../assets/assetloader.h"
#include "../assets/shpfile.h"
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

// Mission timer (in game frames, 15 FPS; -1 = disabled)
static int g_missionTimer = -1;

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

// Top button modes (Repair/Sell)
static bool g_repairMode = false;      // Repair mode active
static bool g_sellMode = false;        // Sell mode active

// Radar state
static bool g_radarOnline = false;     // Radar building powered
static int g_radarSweepAngle = 0;      // Current sweep position (0-359)

// Options button state
static bool g_optionsHover = false;    // Mouse over options button

//===========================================================================
// Cameo Icon Cache
//===========================================================================

// Original cameo dimensions (before 2x scaling)
#define CAMEO_WIDTH  32
#define CAMEO_HEIGHT 24

// Maximum cached cameos
#define MAX_CAMEO_CACHE 32

struct CameoCacheEntry {
    char name[16];              // Type name (e.g., "POWR", "E1")
    ShpFileHandle shp;          // Loaded SHP handle (NULL if failed)
    bool loaded;                // True if load was attempted
};

static CameoCacheEntry g_cameoCache[MAX_CAMEO_CACHE];
static int g_cameoCacheCount = 0;
static bool g_cameosInitialized = false;

/**
 * Load a cameo SHP for a given type name.
 * Cameo files are named <TYPE>ICON.SHP in CONQUER.MIX.
 */
static ShpFileHandle LoadCameoShp(const char* typeName) {
    if (!typeName || !typeName[0]) return nullptr;

    // Build cameo filename: <TYPE>ICON.SHP
    // Special handling: some types need different icon names
    char iconName[32];

    // Most cameos are <TYPE>ICON.SHP but some are different:
    // Buildings: POWIICON for POWR, PROCIICON for PROC, etc.
    // Units: E1ICNH for E1 (infantry use ICNH suffix)
    // Tanks: 1TNKICON for 1TNK, etc.

    // Try standard pattern first: <TYPE>ICON.SHP
    snprintf(iconName, sizeof(iconName), "%sICON.SHP", typeName);

    ShpFileHandle shp = Assets_LoadSHP(iconName);
    if (shp) return shp;

    // Try alternate pattern for infantry: <TYPE>ICNH.SHP
    snprintf(iconName, sizeof(iconName), "%sICNH.SHP", typeName);
    shp = Assets_LoadSHP(iconName);
    if (shp) return shp;

    // Try with I suffix: <TYPE>I.SHP (some cameos)
    snprintf(iconName, sizeof(iconName), "%sI.SHP", typeName);
    shp = Assets_LoadSHP(iconName);

    return shp;  // May be null
}

/**
 * Get or load a cameo for a type name.
 */
static ShpFileHandle GetCameo(const char* typeName) {
    if (!typeName || !typeName[0]) return nullptr;

    // Check cache first
    for (int i = 0; i < g_cameoCacheCount; i++) {
        if (strcasecmp(g_cameoCache[i].name, typeName) == 0) {
            return g_cameoCache[i].shp;  // May be null if load failed
        }
    }

    // Not in cache, try to load
    if (g_cameoCacheCount >= MAX_CAMEO_CACHE) {
        return nullptr;  // Cache full
    }

    CameoCacheEntry& entry = g_cameoCache[g_cameoCacheCount++];
    strncpy(entry.name, typeName, sizeof(entry.name) - 1);
    entry.name[sizeof(entry.name) - 1] = '\0';
    entry.shp = LoadCameoShp(typeName);
    entry.loaded = true;

    return entry.shp;
}

/**
 * Free all cached cameos.
 */
static void FreeCameoCache(void) {
    for (int i = 0; i < g_cameoCacheCount; i++) {
        if (g_cameoCache[i].shp) {
            Shp_Free(g_cameoCache[i].shp);
            g_cameoCache[i].shp = nullptr;
        }
    }
    g_cameoCacheCount = 0;
    g_cameosInitialized = false;
}

//===========================================================================
// Forward Declarations
//===========================================================================

static void GameUI_RenderTopButtons(void);
static BOOL GameUI_TopButtonsClick(int mouseX, int mouseY);
static void GameUI_RenderPowerBar(void);
static void CalcPlayerPower(int* produced, int* consumed);
static void UpdateRadarState(void);

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
static bool FindSpawnLocationNearBuilding(Building* bldg,
                                          int* outX, int* outY) {
    if (!bldg) return false;

    int centerCellX = bldg->cellX + bldg->width / 2;
    int centerCellY = bldg->cellY + bldg->height;  // Exit at bottom

    // Search in expanding rings around the building exit
    for (int radius = 0; radius <= 5; radius++) {
        for (int dy = -radius; dy <= radius; dy++) {
            for (int dx = -radius; dx <= radius; dx++) {
                // Only check cells on perimeter of current ring
                bool onPerim = abs(dx) == radius || abs(dy) == radius;
                if (radius > 0 && !onPerim) continue;

                int cx = centerCellX + dx;
                int cy = centerCellY + dy;

                int mw = Map_GetWidth(), mh = Map_GetHeight();
                if (cx < 0 || cy < 0 || cx >= mw || cy >= mh) continue;

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
// Prereq shortcuts
#define PRQ_PWR PREREQ_POWER
#define PRQ_REF PREREQ_REFINERY
#define PRQ_FAC PREREQ_FACTORY
#define PRQ_BAR PREREQ_BARRACKS

// PRQ_PR = PRQ_PWR|PRQ_REF, PRQ_PF = PRQ_PWR|PRQ_FAC
#define PRQ_PR (PRQ_PWR|PRQ_REF)
#define PRQ_PF (PRQ_PWR|PRQ_FAC)

static BuildItemDef g_structureDefs[] = {
    {"POWR", "Power",   300, 300, false, BUILDING_POWER,    2,2, PREREQ_NONE},
    {"PROC", "Refinery",2000,600, false, BUILDING_REFINERY, 3,3, PRQ_PWR},
    {"TENT", "Barracks",500, 400, false, BUILDING_BARRACKS, 2,2, PRQ_PWR},
    {"WEAP", "Factory", 2000,600, false, BUILDING_FACTORY,  3,3, PRQ_PR},
    {"DOME", "Radar",   1000,500, false, BUILDING_RADAR,    2,2, PRQ_PF},
};
static const int g_structureDefCount = 5;

// Available units with proper tech tree
// Infantry requires Barracks, Vehicles require War Factory
// Harvester requires Refinery (auto-spawned, but also buildable)
static BuildItemDef g_unitDefs[] = {
    {"E1",   "Rifle Infantry", 100, 150, true, UNIT_RIFLE,       1,1, PRQ_BAR},
    {"E2",   "Grenadier",      160, 180, true, UNIT_GRENADIER,   1,1, PRQ_BAR},
    {"E3",   "Rocket Soldier", 300, 200, true, UNIT_ROCKET,      1,1, PRQ_BAR},
    {"ENG",  "Engineer",       500, 200, true, UNIT_ENGINEER,    1,1, PRQ_BAR},
    {"HARV", "Harvester",     1400, 400, true, UNIT_HARVESTER,   1,1, PRQ_REF},
    {"1TNK", "Light Tank",     700, 300, true, UNIT_TANK_LIGHT,  1,1, PRQ_FAC},
    {"2TNK", "Medium Tank",    800, 350, true, UNIT_TANK_MEDIUM, 1,1, PRQ_FAC},
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
    FreeCameoCache();
    g_uiInitialized = false;
}

//===========================================================================
// Check Prerequisites
//===========================================================================

static bool CheckPrerequisites(const BuildItemDef* item) {
    // Check if player has all required buildings
    return (item->prerequisites & g_playerBuildings) == item->prerequisites;
}

// Get first missing prereq for item (unused - kept for tooltip support)
static const char* GetMissingPrereq(const BuildItemDef* item)
    __attribute__((unused));
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
            int mw = Map_GetWidth(), mh = Map_GetHeight();
            if (cx < 0 || cy < 0 || cx >= mw || cy >= mh) return false;

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
    int px = g_placementCellX, py = g_placementCellY;
    g_placementValid = CanPlaceAt(px, py, item->width, item->height);
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
    int px = g_placementCellX, py = g_placementCellY;
    for (int dy = 0; dy < item->height; dy++) {
        for (int dx = 0; dx < item->width; dx++) {
            MapCell* cell = Map_GetCell(px + dx, py + dy);
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

    // Update radar online status and sweep animation
    UpdateRadarState();

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

            UnitType ut = (UnitType)item->spawnType;
            Building* prodBldg = FindProductionBuilding(ut);
            if (prodBldg) {
                int bldgSpawnX, bldgSpawnY;
                if (FindSpawnLocationNearBuilding(prodBldg,
                                                  &bldgSpawnX, &bldgSpawnY)) {
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

static void DrawBeveledBox(int x, int y, int w, int h,
                           uint8_t bgColor, bool raised) {
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
            int px = g_placementCellX, py = g_placementCellY;
            MapCell* cell = Map_GetCell(px + dx, py + dy);
            bool cellValid = cell &&
                            cell->terrain != TERRAIN_WATER &&
                            cell->terrain != TERRAIN_ROCK &&
                            cell->terrain != TERRAIN_BUILDING &&
                            cell->buildingId < 0 &&
                            cell->unitId < 0;

            uint8_t cellColor = cellValid ? color : PAL_RED;

            // Draw cell outline
            int csx = cellScreenX, csy = cellScreenY;
            Renderer_DrawRect(csx, csy, CELL_SIZE, CELL_SIZE, cellColor);

            // Draw X pattern for invalid cells
            if (!cellValid) {
                int x1 = csx + 2, y1 = csy + 2;
                int x2 = csx + CELL_SIZE - 3, y2 = csy + CELL_SIZE - 3;
                Renderer_DrawLine(x1, y1, x2, y2, PAL_RED);
                Renderer_DrawLine(x2, y1, x1, y2, PAL_RED);
            }
        }
    }

    // Draw outer boundary
    int bw = width * CELL_SIZE, bh = height * CELL_SIZE;
    Renderer_DrawRect(screenX, screenY, bw, bh, color);

    // Draw building name above cursor
    if (screenY > 12) {
        Renderer_DrawText(item->name, screenX + 2, screenY - 10, color, 0);
    }
}

//===========================================================================
// Radar Status Check
//===========================================================================

// Check if player has a powered radar building
static bool CheckRadarOnline(void) {
    int produced, consumed;
    CalcPlayerPower(&produced, &consumed);
    bool hasPower = produced >= consumed;

    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bldg = Buildings_Get(i);
        if (!bldg || !bldg->active) continue;
        if (bldg->team != TEAM_PLAYER) continue;
        if (bldg->type == BUILDING_RADAR && hasPower) {
            return true;
        }
    }
    return false;
}

// Update radar state each frame
static void UpdateRadarState(void) {
    g_radarOnline = CheckRadarOnline();
    if (g_radarOnline) {
        g_radarSweepAngle = (g_radarSweepAngle + 6) % 360;  // ~60 degrees/sec
    }
}

//===========================================================================
// Main Render
//===========================================================================

void GameUI_Render(void) {
    if (!g_uiInitialized) return;

    // Draw sidebar background - dark with bevel
    int sbx = SIDEBAR_X, sby = SIDEBAR_Y;
    int sbw = SIDEBAR_WIDTH, sbh = SIDEBAR_HEIGHT;
    Renderer_FillRect(sbx, sby, sbw, sbh, PAL_BLACK);

    // Left border of sidebar (separating from game view)
    Renderer_VLine(SIDEBAR_X, 0, SIDEBAR_HEIGHT - 1, PAL_GREY);
    Renderer_VLine(SIDEBAR_X + 1, 0, SIDEBAR_HEIGHT - 1, PAL_BLACK);

    // Draw individual components
    GameUI_RenderRadar();
    GameUI_RenderTopButtons();
    GameUI_RenderSidebar();
    GameUI_RenderPowerBar();
    GameUI_RenderSelectionPanel();
    GameUI_RenderHUD();

    // Draw placement cursor (after sidebar so it's on top)
    GameUI_RenderPlacement();
}

//===========================================================================
// Input Handling
//===========================================================================

BOOL GameUI_HandleInput(int mouseX, int mouseY,
                        BOOL leftClick, BOOL rightClick) {
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

    // Check top buttons (Repair/Sell/Zoom)
    if (mouseX >= SIDEBAR_X && leftClick) {
        if (GameUI_TopButtonsClick(mouseX, mouseY)) {
            return TRUE;
        }
    }

    // Check sidebar clicks
    if (mouseX >= SIDEBAR_X) {
        if (leftClick) {
            return GameUI_SidebarClick(mouseX, mouseY, leftClick);
        }
        return TRUE;
    }

    // Check options button in game area (always update hover state)
    if (mouseX >= OPTIONS_BTN_X - 2 &&
        mouseX < OPTIONS_BTN_X + OPTIONS_BTN_WIDTH - 2 &&
        mouseY >= OPTIONS_BTN_Y - 2 &&
        mouseY < OPTIONS_BTN_Y + OPTIONS_BTN_HEIGHT - 2) {
        g_optionsHover = true;
        if (leftClick) {
            Menu_SetCurrentScreen(MENU_SCREEN_OPTIONS);
            return TRUE;
        }
    } else {
        g_optionsHover = false;
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

static void RenderRadarSweep(void) {
    // Draw rotating sweep line from center
    int centerX = RADAR_X + RADAR_WIDTH / 2;
    int centerY = RADAR_Y + RADAR_HEIGHT / 2;
    int radius = (RADAR_WIDTH < RADAR_HEIGHT ? RADAR_WIDTH : RADAR_HEIGHT) / 2 - 4;

    // Convert angle to radians
    float rad = (float)g_radarSweepAngle * 3.14159f / 180.0f;
    int endX = centerX + (int)(radius * sinf(rad));
    int endY = centerY - (int)(radius * cosf(rad));

    // Draw sweep line
    Renderer_DrawLine(centerX, centerY, endX, endY, PAL_LTGREEN);
}

void GameUI_RenderRadar(void) {
    // Draw beveled frame around radar
    DrawBeveledBox(RADAR_X - 2, RADAR_Y - 2,
                   RADAR_WIDTH + 4, RADAR_HEIGHT + 4, PAL_GREY, false);

    // Draw inner border for enhanced frame effect
    Renderer_DrawRect(RADAR_X - 1, RADAR_Y - 1,
                      RADAR_WIDTH + 2, RADAR_HEIGHT + 2, PAL_BLACK);

    Renderer_FillRect(RADAR_X, RADAR_Y, RADAR_WIDTH, RADAR_HEIGHT, PAL_BLACK);

    int mapWidth = Map_GetWidth();
    int mapHeight = Map_GetHeight();

    // Check if radar should be offline (no map or no radar building)
    if (mapWidth <= 0 || mapHeight <= 0 || !g_radarOnline) {
        // Draw static/noise pattern for offline state
        int centerX = RADAR_X + RADAR_WIDTH / 2;
        int centerY = RADAR_Y + RADAR_HEIGHT / 2;

        // Draw "RADAR OFFLINE" text
        Renderer_DrawText("RADAR", centerX - 20, centerY - 8, PAL_GREY, 0);
        Renderer_DrawText("OFFLINE", centerX - 28, centerY + 4, PAL_GREY, 0);

        // Draw some static dots for visual interest
        for (int i = 0; i < 20; i++) {
            int x = RADAR_X + (g_flashFrame * 17 + i * 31) % RADAR_WIDTH;
            int y = RADAR_Y + (g_flashFrame * 13 + i * 23) % RADAR_HEIGHT;
            Renderer_PutPixel(x, y, PAL_GREY);
        }
        return;
    }

    // Radar is online - render terrain, units, buildings
    RadarContext ctx;
    CalcRadarContext(&ctx);
    RenderRadarTerrain(&ctx);
    RenderRadarUnits(&ctx);
    RenderRadarBuildings(&ctx);
    RenderRadarViewport(&ctx);

    // Draw sweep line animation
    RenderRadarSweep();
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
// Top Buttons Implementation (Repair/Sell/Zoom)
//===========================================================================

// Button sizes (2x original: 32x9 => 64x18, 20x9 => 40x18)
#define REPAIR_BTN_WIDTH   64
#define SELL_BTN_WIDTH     40
#define ZOOM_BTN_WIDTH     40
#define TOP_BTN_HEIGHT     TOP_BUTTONS_HEIGHT

static void GameUI_RenderTopButtons(void) {
    int y = TOP_BUTTONS_Y;
    int x = SIDEBAR_X + 8;

    // Repair button
    uint8_t repairBg = g_repairMode ? PAL_LTGREEN : PAL_GREY;
    DrawBeveledBox(x, y, REPAIR_BTN_WIDTH, TOP_BTN_HEIGHT, repairBg, !g_repairMode);
    uint8_t repairTxt = g_repairMode ? PAL_BLACK : PAL_WHITE;
    Renderer_DrawText("REPAIR", x + 6, y + 4, repairTxt, 0);
    x += REPAIR_BTN_WIDTH + 4;

    // Sell button
    uint8_t sellBg = g_sellMode ? PAL_YELLOW : PAL_GREY;
    DrawBeveledBox(x, y, SELL_BTN_WIDTH, TOP_BTN_HEIGHT, sellBg, !g_sellMode);
    uint8_t sellTxt = g_sellMode ? PAL_BLACK : PAL_WHITE;
    Renderer_DrawText("SELL", x + 6, y + 4, sellTxt, 0);
    x += SELL_BTN_WIDTH + 4;

    // Zoom button (centers view on construction yard)
    DrawBeveledBox(x, y, ZOOM_BTN_WIDTH, TOP_BTN_HEIGHT, PAL_GREY, true);
    Renderer_DrawText("ZOOM", x + 4, y + 4, PAL_WHITE, 0);
}

// Handle click on top buttons
// Returns TRUE if click consumed
static BOOL GameUI_TopButtonsClick(int mouseX, int mouseY) {
    if (mouseY < TOP_BUTTONS_Y || mouseY >= TOP_BUTTONS_Y + TOP_BTN_HEIGHT) {
        return FALSE;
    }

    int x = SIDEBAR_X + 8;

    // Repair button
    if (mouseX >= x && mouseX < x + REPAIR_BTN_WIDTH) {
        g_repairMode = !g_repairMode;
        if (g_repairMode) g_sellMode = false;  // Mutually exclusive
        return TRUE;
    }
    x += REPAIR_BTN_WIDTH + 4;

    // Sell button
    if (mouseX >= x && mouseX < x + SELL_BTN_WIDTH) {
        g_sellMode = !g_sellMode;
        if (g_sellMode) g_repairMode = false;  // Mutually exclusive
        return TRUE;
    }
    x += SELL_BTN_WIDTH + 4;

    // Zoom button - centers on construction yard
    if (mouseX >= x && mouseX < x + ZOOM_BTN_WIDTH) {
        // Find player's construction yard
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            Building* bldg = Buildings_Get(i);
            if (!bldg || !bldg->active) continue;
            if (bldg->team != TEAM_PLAYER) continue;
            if (bldg->type == BUILDING_CONSTRUCTION) {
                // Center map on this building
                Map_CenterViewport(bldg->cellX * CELL_SIZE + CELL_SIZE,
                                   bldg->cellY * CELL_SIZE + CELL_SIZE);
                return TRUE;
            }
        }
        return TRUE;  // Click consumed even if no CY
    }

    return FALSE;
}

//===========================================================================
// Power Bar Implementation
//===========================================================================

// Calculate player's power production and consumption
static void CalcPlayerPower(int* produced, int* consumed) {
    int prod = 0, cons = 0;

    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bldg = Buildings_Get(i);
        if (!bldg || !bldg->active) continue;
        if (bldg->team != TEAM_PLAYER) continue;

        // Power plants produce, other buildings consume
        switch (bldg->type) {
            case BUILDING_POWER:
                prod += 100;  // Power Plant produces 100
                break;
            case BUILDING_ADV_POWER:
                prod += 200;  // Advanced Power produces 200
                break;
            case BUILDING_RADAR:
                cons += 40;
                break;
            case BUILDING_FACTORY:
                cons += 30;
                break;
            case BUILDING_BARRACKS:
                cons += 20;
                break;
            case BUILDING_REFINERY:
                cons += 30;
                break;
            case BUILDING_SILO:
                cons += 10;
                break;
            case BUILDING_TURRET:
            case BUILDING_SAM:
                cons += 20;
                break;
            case BUILDING_TECH_CENTER:
                cons += 50;
                break;
            default:
                cons += 10;  // Base consumption for other buildings
                break;
        }
    }

    *produced = prod;
    *consumed = cons;
}

static void GameUI_RenderPowerBar(void) {
    int x = POWER_BAR_X;
    int y = POWER_BAR_Y;
    int w = POWER_BAR_WIDTH;
    int h = POWER_BAR_HEIGHT;

    // Frame
    DrawBeveledBox(x, y, w, h, PAL_BLACK, false);

    // Get power stats
    int produced, consumed;
    CalcPlayerPower(&produced, &consumed);

    // Calculate fill levels (from bottom up)
    int innerH = h - 4;
    int innerY = y + 2;
    int innerX = x + 2;
    int innerW = w - 4;

    if (produced > 0) {
        // Power produced (green bar from bottom)
        int maxPower = produced + 50;  // Max includes some headroom
        int prodH = (produced * innerH) / maxPower;
        if (prodH > innerH) prodH = innerH;

        // Draw green production bar from bottom
        Renderer_FillRect(innerX, innerY + innerH - prodH,
                          innerW, prodH, PAL_LTGREEN);

        // Draw consumption line (yellow or red if over)
        int consH = (consumed * innerH) / maxPower;
        if (consH > innerH) consH = innerH;

        // Fill above consumption with red if low power
        if (consumed > produced) {
            // Low power - red zone
            int overH = ((consumed - produced) * innerH) / maxPower;
            if (overH > innerH - prodH) overH = innerH - prodH;
            Renderer_FillRect(innerX, innerY + innerH - prodH - overH,
                              innerW, overH, PAL_RED);
        }

        // Draw consumption marker line
        int consY = innerY + innerH - consH;
        if (consY >= innerY && consY < innerY + innerH) {
            uint8_t lineColor = (consumed > produced) ? PAL_RED : PAL_YELLOW;
            Renderer_HLine(innerX, consY, innerX + innerW - 1, lineColor);
        }
    } else {
        // No power - just show empty
        Renderer_FillRect(innerX, innerY, innerW, innerH, PAL_BLACK);
    }

    // Border
    Renderer_DrawRect(x + 1, y + 1, w - 2, h - 2, PAL_GREY);
}

//===========================================================================
// Sidebar Implementation - Two Column Layout
//===========================================================================

// Scroll state for each column
static int g_structureScrollTop = 0;  // Top visible structure index
static int g_unitScrollTop = 0;       // Top visible unit index

// Helper: Draw a single cameo button
static void DrawCameoButton(int x, int y, const BuildItemDef* item,
                            bool isBuilding, int progress, bool isReady) {
    bool hasPrereqs = CheckPrerequisites(item);
    bool canAfford = g_playerCredits >= item->cost;
    bool available = hasPrereqs && canAfford;

    // Background color
    uint8_t bgColor = available ? PAL_GREY : PAL_BLACK;
    if (isBuilding) bgColor = PAL_GREY;

    // Draw cameo box with bevel
    bool raised = available && !isBuilding;
    DrawBeveledBox(x, y, STRIP_ITEM_WIDTH, STRIP_ITEM_HEIGHT, bgColor, raised);

    // Try to render cameo sprite
    ShpFileHandle cameo = GetCameo(item->name);
    bool hasCameo = false;

    if (cameo) {
        const ShpFrame* frame = Shp_GetFrame(cameo, 0);
        if (frame && frame->pixels) {
            // Scale 2x (32x24 -> 64x48) to match our UI scale
            // Use ScaleBlit for proper scaling
            Renderer_ScaleBlit(frame->pixels, frame->width, frame->height,
                               x, y, STRIP_ITEM_WIDTH, STRIP_ITEM_HEIGHT, TRUE);
            hasCameo = true;

            // Dim if not available
            if (!available) {
                Renderer_DimRect(x, y, STRIP_ITEM_WIDTH, STRIP_ITEM_HEIGHT, 2);
            }
        }
    }

    // Fallback: draw text if no cameo
    if (!hasCameo) {
        uint8_t textColor = available ? PAL_WHITE : PAL_GREY;
        int textX = x + 4;
        int textY = y + 4;
        Renderer_DrawText(item->name, textX, textY, textColor, 0);
    }

    // Production status overlay
    if (isBuilding) {
        if (isReady) {
            // Ready - pulsing READY text
            bool flash = g_flashFrame < 10;
            uint8_t rdyClr = flash ? PAL_WHITE : PAL_LTGREEN;
            int txtX = x + (STRIP_ITEM_WIDTH - 40) / 2;
            Renderer_DrawText("READY", txtX, y + STRIP_ITEM_HEIGHT - 12,
                             rdyClr, 0);
        } else {
            // Progress bar at bottom
            int barW = (STRIP_ITEM_WIDTH - 8) * progress / 100;
            Renderer_FillRect(x + 4, y + STRIP_ITEM_HEIGHT - 6,
                             STRIP_ITEM_WIDTH - 8, 4, PAL_BLACK);
            if (barW > 0) {
                Renderer_FillRect(x + 4, y + STRIP_ITEM_HEIGHT - 6,
                                 barW, 4, PAL_LTGREEN);
            }
        }
    } else if (!hasPrereqs && !hasCameo) {
        // Show lock indicator (only if no cameo)
        Renderer_DrawText("---", x + 20, y + 20, PAL_GREY, 0);
    }
}

// Helper: Draw scroll arrows for a column
static void DrawScrollArrows(int x, int y, bool canScrollUp, bool canScrollDown) {
    // Up arrow
    uint8_t upColor = canScrollUp ? PAL_WHITE : PAL_GREY;
    DrawBeveledBox(x, y, SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE / 2,
                   PAL_GREY, canScrollUp);
    Renderer_DrawText("^", x + 8, y + 2, upColor, 0);

    // Down arrow
    uint8_t downColor = canScrollDown ? PAL_WHITE : PAL_GREY;
    int downY = y + SCROLL_BUTTON_SIZE / 2 + 2;
    DrawBeveledBox(x, downY, SCROLL_BUTTON_SIZE, SCROLL_BUTTON_SIZE / 2,
                   PAL_GREY, canScrollDown);
    Renderer_DrawText("v", x + 8, downY + 2, downColor, 0);
}

void GameUI_RenderSidebar(void) {
    // Column 1: Structures (left column)
    int col1X = STRIP_COL1_X;
    int startY = STRIP_Y;

    for (int row = 0; row < STRIP_ITEMS_VISIBLE; row++) {
        int idx = g_structureScrollTop + row;
        if (idx >= g_structureDefCount) break;

        const BuildItemDef* item = &g_structureDefs[idx];
        bool isBuilding = (g_structureProducing == idx);
        int progress = isBuilding ? (g_structureProgress / 100) : 0;
        bool isReady = isBuilding && g_placementMode && g_placementType == idx;

        int y = startY + row * STRIP_ITEM_HEIGHT;
        DrawCameoButton(col1X, y, item, isBuilding, progress, isReady);
    }

    // Column 2: Units (right column)
    int col2X = STRIP_COL2_X;

    for (int row = 0; row < STRIP_ITEMS_VISIBLE; row++) {
        int idx = g_unitScrollTop + row;
        if (idx >= g_unitDefCount) break;

        const BuildItemDef* item = &g_unitDefs[idx];
        bool isBuilding = (g_unitProducing == idx);
        int progress = isBuilding ? (g_unitProgress / 100) : 0;

        int y = startY + row * STRIP_ITEM_HEIGHT;
        DrawCameoButton(col2X, y, item, isBuilding, progress, false);
    }

    // Scroll arrows below each column
    int scrollY = SCROLL_BUTTONS_Y;

    // Structure scroll arrows (column 1)
    bool canScrollUpStr = g_structureScrollTop > 0;
    bool canScrollDownStr = g_structureScrollTop + STRIP_ITEMS_VISIBLE
                            < g_structureDefCount;
    DrawScrollArrows(col1X + 20, scrollY, canScrollUpStr, canScrollDownStr);

    // Unit scroll arrows (column 2)
    bool canScrollUpUnit = g_unitScrollTop > 0;
    bool canScrollDownUnit = g_unitScrollTop + STRIP_ITEMS_VISIBLE
                             < g_unitDefCount;
    DrawScrollArrows(col2X + 20, scrollY, canScrollUpUnit, canScrollDownUnit);

    // Placement hint
    if (g_placementMode) {
        int hintY = scrollY + SCROLL_BUTTON_SIZE + 4;
        Renderer_DrawText("Click to place", SIDEBAR_X + 8, hintY, PAL_WHITE, 0);
    }
}

BOOL GameUI_SidebarClick(int mouseX, int mouseY, BOOL leftClick) {
    (void)leftClick;

    // Check if click is in sidebar area
    if (mouseX < SIDEBAR_X || mouseX >= SIDEBAR_X + SIDEBAR_WIDTH) {
        return FALSE;
    }

    // Determine which column was clicked
    bool inCol1 = (mouseX >= STRIP_COL1_X &&
                   mouseX < STRIP_COL1_X + STRIP_ITEM_WIDTH);
    bool inCol2 = (mouseX >= STRIP_COL2_X &&
                   mouseX < STRIP_COL2_X + STRIP_ITEM_WIDTH);

    // Check scroll buttons first
    int scrollY = SCROLL_BUTTONS_Y;
    if (mouseY >= scrollY && mouseY < scrollY + SCROLL_BUTTON_SIZE) {
        // Scroll up buttons
        if (inCol1 && g_structureScrollTop > 0) {
            g_structureScrollTop--;
            return TRUE;
        }
        if (inCol2 && g_unitScrollTop > 0) {
            g_unitScrollTop--;
            return TRUE;
        }
    }
    if (mouseY >= scrollY + SCROLL_BUTTON_SIZE &&
        mouseY < scrollY + SCROLL_BUTTON_SIZE * 2) {
        // Scroll down buttons
        int maxStructScroll = g_structureDefCount - STRIP_ITEMS_VISIBLE;
        int maxUnitScroll = g_unitDefCount - STRIP_ITEMS_VISIBLE;
        if (inCol1 && g_structureScrollTop < maxStructScroll) {
            g_structureScrollTop++;
            return TRUE;
        }
        if (inCol2 && g_unitScrollTop < maxUnitScroll) {
            g_unitScrollTop++;
            return TRUE;
        }
    }

    // Check cameo clicks
    if (mouseY >= STRIP_Y && mouseY < SCROLL_BUTTONS_Y) {
        int row = (mouseY - STRIP_Y) / STRIP_ITEM_HEIGHT;
        if (row >= 0 && row < STRIP_ITEMS_VISIBLE) {
            // Column 1: Structures
            if (inCol1) {
                int idx = g_structureScrollTop + row;
                if (idx >= 0 && idx < g_structureDefCount) {
                    BuildItemDef* item = &g_structureDefs[idx];

                    // Check if already building or in placement mode
                    if (g_structureProducing >= 0 || g_placementMode) {
                        return TRUE;
                    }

                    // Check prerequisites
                    if (!CheckPrerequisites(item)) {
                        return TRUE;
                    }

                    // Check if player can afford it
                    if (g_playerCredits < item->cost) {
                        return TRUE;
                    }

                    // Start production
                    g_playerCredits -= item->cost;
                    g_structureProducing = idx;
                    g_structureProgress = 0;
                    return TRUE;
                }
            }

            // Column 2: Units
            if (inCol2) {
                int idx = g_unitScrollTop + row;
                if (idx >= 0 && idx < g_unitDefCount) {
                    BuildItemDef* item = &g_unitDefs[idx];

                    // Check if already building something
                    if (g_unitProducing >= 0) {
                        return TRUE;
                    }

                    // Check prerequisites
                    if (!CheckPrerequisites(item)) {
                        return TRUE;
                    }

                    // Check if player can afford it
                    if (g_playerCredits < item->cost) {
                        return TRUE;
                    }

                    // Start production
                    g_playerCredits -= item->cost;
                    g_unitProducing = idx;
                    g_unitProgress = 0;
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

//===========================================================================
// Selection Panel Implementation
//===========================================================================

void GameUI_RenderSelectionPanel(void) {
    // Panel frame
    int px = SIDEBAR_X + 3, py = SELECTION_Y;
    int pw = SIDEBAR_WIDTH - 6, ph = SELECTION_HEIGHT - 4;
    DrawBeveledBox(px, py, pw, ph, PAL_GREY, false);

    // Inner area
    int ix = SIDEBAR_X + 5, iy = SELECTION_Y + 2;
    int iw = SIDEBAR_WIDTH - 10, ih = SELECTION_HEIGHT - 8;
    Renderer_FillRect(ix, iy, iw, ih, PAL_BLACK);

    int selectedCount = Units_GetSelectedCount();

    if (selectedCount == 0) {
        int tx = SIDEBAR_X + 14, ty = SELECTION_Y + 20;
        Renderer_DrawText("No unit", tx, ty, PAL_GREY, 0);
        Renderer_DrawText("selected", tx - 2, ty + 12, PAL_GREY, 0);
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
    int maxHp = unit->maxHealth;
    int healthPct = (maxHp > 0) ? (unit->health * 100 / maxHp) : 0;
    int barWidth = SIDEBAR_WIDTH - 20;
    int greenWidth = (healthPct * barWidth) / 100;

    // Background (damage)
    Renderer_FillRect(SIDEBAR_X + 8, SELECTION_Y + 18, barWidth, 6, PAL_RED);

    // Health
    int hx = SIDEBAR_X + 8, hy = SELECTION_Y + 18;
    if (greenWidth > 0) {
        uint8_t hc = PAL_LTGREEN;
        if (healthPct <= 50) hc = PAL_YELLOW;
        if (healthPct <= 25) hc = PAL_RED;
        Renderer_FillRect(hx, hy, greenWidth, 6, hc);
    }

    // Border
    Renderer_DrawRect(SIDEBAR_X + 8, SELECTION_Y + 18, barWidth, 6, PAL_GREY);

    // Health text
    char htxt[16];
    snprintf(htxt, sizeof(htxt), "%d/%d", unit->health, maxHp);
    Renderer_DrawText(htxt, hx, SELECTION_Y + 28, PAL_WHITE, 0);

    // Multi-select count
    if (selectedCount > 1) {
        char ctxt[16];
        snprintf(ctxt, sizeof(ctxt), "+%d more", selectedCount - 1);
        Renderer_DrawText(ctxt, hx, SELECTION_Y + 40, PAL_YELLOW, 0);
    }

    // Unit state
    const char* st = "Idle";
    uint8_t sc = PAL_LTGREY;
    switch (unit->state) {
        case STATE_MOVING:     st = "Moving";  sc = PAL_LTCYAN; break;
        case STATE_ATTACKING:  st = "Attack!"; sc = PAL_RED;    break;
        case STATE_HARVESTING: st = "Harvest"; sc = PAL_YELLOW; break;
        case STATE_RETURNING:  st = "Return";  sc = PAL_LTCYAN; break;
        case STATE_DYING:      st = "Dying";   sc = PAL_RED;    break;
        default: break;
    }
    Renderer_DrawText(st, hx, SELECTION_Y + 52, sc, 0);
}

//===========================================================================
// HUD Implementation
//===========================================================================

void GameUI_RenderHUD(void) {
    // Credits display in top-left of game area
    DrawBeveledBox(CREDITS_X - 2, CREDITS_Y - 2,
                   CREDITS_WIDTH, CREDITS_HEIGHT, PAL_BLACK, false);

    char creditsText[32];
    snprintf(creditsText, sizeof(creditsText), "$%d", g_playerCredits);
    Renderer_DrawText(creditsText, CREDITS_X + 4, CREDITS_Y + 2, PAL_YELLOW, 0);

    // Mission timer display (if active) - next to credits
    if (g_missionTimer >= 0) {
        int totalSeconds = g_missionTimer / 15;  // 15 FPS
        int minutes = totalSeconds / 60;
        int seconds = totalSeconds % 60;

        char timerText[16];
        snprintf(timerText, sizeof(timerText), "%d:%02d", minutes, seconds);

        DrawBeveledBox(TIMER_X - 2, TIMER_Y - 2,
                       TIMER_WIDTH, TIMER_HEIGHT, PAL_BLACK, false);
        // Flash red when under 1 minute
        int color = (minutes == 0 && (g_flashFrame & 8)) ? PAL_RED : PAL_WHITE;
        Renderer_DrawText(timerText, TIMER_X + 4, TIMER_Y + 2, color, 0);
    }

    // Options button in top-right of game area
    DrawBeveledBox(OPTIONS_BTN_X - 2, OPTIONS_BTN_Y - 2,
                   OPTIONS_BTN_WIDTH, OPTIONS_BTN_HEIGHT,
                   g_optionsHover ? PAL_LTGREY : PAL_GREY, true);
    Renderer_DrawText("OPTIONS", OPTIONS_BTN_X + 4, OPTIONS_BTN_Y + 2,
                      PAL_WHITE, 0);
}

// Set mission timer (frames at 15 FPS, -1 to disable)
void GameUI_SetMissionTimer(int frames) {
    g_missionTimer = frames;
}

// Get mission timer
int GameUI_GetMissionTimer(void) {
    return g_missionTimer;
}

// Decrement mission timer by 1 frame
void GameUI_TickMissionTimer(void) {
    if (g_missionTimer > 0) {
        g_missionTimer--;
    }
}

//===========================================================================
// Credits Functions
//===========================================================================

int GameUI_GetCredits(void) {
    return g_playerCredits;
}

void GameUI_SetCredits(int credits) {
    g_playerCredits = credits;
    if (g_playerCredits < 0) g_playerCredits = 0;
}

void GameUI_AddCredits(int amount) {
    g_playerCredits += amount;
    if (g_playerCredits < 0) g_playerCredits = 0;
}
