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

//===========================================================================
// Build Item Definitions
//===========================================================================

struct BuildItemDef {
    const char* name;
    const char* fullName;
    int cost;
    int buildTime;  // Frames to complete at normal speed
    bool isUnit;    // true = unit, false = structure
    int spawnType;  // UnitType or BuildingType to spawn
    bool requiresBarracks;
    bool requiresFactory;
};

// Available structures (simplified tech tree for demo)
static BuildItemDef g_structureDefs[] = {
    {"POWR", "Power Plant", 300, 300, false, BUILDING_POWER, false, false},
    {"PROC", "Refinery", 2000, 600, false, BUILDING_REFINERY, false, false},
    {"TENT", "Barracks", 500, 400, false, BUILDING_BARRACKS, false, false},
    {"WEAP", "War Factory", 2000, 600, false, BUILDING_FACTORY, true, false},
};
static const int g_structureDefCount = 4;

// Available units (simplified tech tree)
static BuildItemDef g_unitDefs[] = {
    {"E1", "Rifle Infantry", 100, 150, true, UNIT_RIFLE, true, false},
    {"E2", "Grenadier", 160, 180, true, UNIT_GRENADIER, true, false},
    {"E3", "Rocket Soldier", 300, 200, true, UNIT_ROCKET, true, false},
    {"ENG", "Engineer", 500, 200, true, UNIT_ENGINEER, true, false},
    {"1TNK", "Light Tank", 700, 300, true, UNIT_TANK_LIGHT, false, true},
    {"2TNK", "Medium Tank", 800, 350, true, UNIT_TANK_MEDIUM, false, true},
};
static const int g_unitDefCount = 6;

// Player has these buildings (for prerequisites)
static bool g_hasBarracks = true;   // Demo starts with barracks
static bool g_hasFactory = true;    // Demo starts with factory

//===========================================================================
// Initialization
//===========================================================================

void GameUI_Init(void) {
    g_uiInitialized = true;
    g_radarPulse = 0;
    g_flashFrame = 0;
    g_playerCredits = 5000;
    g_structureProducing = -1;
    g_structureProgress = 0;
    g_unitProducing = -1;
    g_unitProgress = 0;
    g_hasBarracks = true;
    g_hasFactory = true;
}

void GameUI_Shutdown(void) {
    g_uiInitialized = false;
}

//===========================================================================
// Check Prerequisites
//===========================================================================

static bool CheckPrerequisites(const BuildItemDef* item) {
    if (item->requiresBarracks && !g_hasBarracks) {
        return false;
    }
    if (item->requiresFactory && !g_hasFactory) {
        return false;
    }
    return true;
}

//===========================================================================
// Update
//===========================================================================

void GameUI_Update(void) {
    g_radarPulse = (g_radarPulse + 1) % 30;
    g_flashFrame = (g_flashFrame + 1) % 20;

    // Update structure production
    if (g_structureProducing >= 0) {
        BuildItemDef* item = &g_structureDefs[g_structureProducing];

        // Calculate progress increment (100% over buildTime frames)
        int progressPerFrame = (100 * 100) / item->buildTime;  // Fixed point
        g_structureProgress += progressPerFrame;

        if (g_structureProgress >= 10000) {  // 100.00%
            // Structure complete - for demo, just finish
            // Real implementation would enter placement mode
            g_structureProducing = -1;
            g_structureProgress = 0;
        }
    }

    // Update unit production
    if (g_unitProducing >= 0) {
        BuildItemDef* item = &g_unitDefs[g_unitProducing];

        // Calculate progress increment
        int progressPerFrame = (100 * 100) / item->buildTime;
        g_unitProgress += progressPerFrame;

        if (g_unitProgress >= 10000) {  // 100.00%
            // Unit complete - spawn it!
            int spawnX = 150 + (rand() % 100);
            int spawnY = 500 + (rand() % 100);

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
}

//===========================================================================
// Input Handling
//===========================================================================

BOOL GameUI_HandleInput(int mouseX, int mouseY, BOOL leftClick, BOOL rightClick) {
    (void)rightClick;

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

void GameUI_RenderRadar(void) {
    int mapWidth = Map_GetWidth();
    int mapHeight = Map_GetHeight();

    // Draw radar frame
    DrawBeveledBox(RADAR_X - 2, RADAR_Y - 2, RADAR_WIDTH + 4, RADAR_HEIGHT + 4, PAL_GREY, false);

    // Inner background
    Renderer_FillRect(RADAR_X, RADAR_Y, RADAR_WIDTH, RADAR_HEIGHT, PAL_BLACK);

    if (mapWidth <= 0 || mapHeight <= 0) {
        // No map - show offline
        Renderer_DrawText("RADAR", RADAR_X + 14, RADAR_Y + 28, PAL_GREY, 0);
        Renderer_DrawText("OFFLINE", RADAR_X + 10, RADAR_Y + 40, PAL_GREY, 0);
        return;
    }

    // Calculate scale to fit map in radar
    float scaleX = (float)(RADAR_WIDTH - 4) / (float)(mapWidth);
    float scaleY = (float)(RADAR_HEIGHT - 4) / (float)(mapHeight);
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    // Calculate centered offset
    int displayWidth = (int)(mapWidth * scale);
    int displayHeight = (int)(mapHeight * scale);
    int offsetX = RADAR_X + 2 + (RADAR_WIDTH - 4 - displayWidth) / 2;
    int offsetY = RADAR_Y + 2 + (RADAR_HEIGHT - 4 - displayHeight) / 2;

    // Draw terrain
    for (int cy = 0; cy < mapHeight; cy++) {
        for (int cx = 0; cx < mapWidth; cx++) {
            MapCell* cell = Map_GetCell(cx, cy);
            if (!cell) continue;

            uint8_t color;
            switch (cell->terrain) {
                case TERRAIN_WATER:
                    color = PAL_BLUE;
                    break;
                case TERRAIN_ROCK:
                    color = PAL_GREY;
                    break;
                case TERRAIN_TREE:
                    color = PAL_GREEN;
                    break;
                case TERRAIN_ROAD:
                case TERRAIN_BRIDGE:
                    color = PAL_LTGREY;
                    break;
                case TERRAIN_ORE:
                case TERRAIN_GEM:
                    color = PAL_YELLOW;
                    break;
                default:
                    color = PAL_BROWN;
                    break;
            }

            int px = offsetX + (int)(cx * scale);
            int py = offsetY + (int)(cy * scale);
            Renderer_PutPixel(px, py, color);
        }
    }

    // Draw units
    for (int i = 0; i < MAX_UNITS; i++) {
        Unit* unit = Units_Get(i);
        if (!unit || !unit->active) continue;

        int cellX = unit->worldX / CELL_SIZE;
        int cellY = unit->worldY / CELL_SIZE;

        int px = offsetX + (int)(cellX * scale);
        int py = offsetY + (int)(cellY * scale);

        uint8_t color = (unit->team == TEAM_PLAYER) ? PAL_LTGREEN : PAL_RED;

        // Draw 2x2 dot for visibility
        Renderer_FillRect(px, py, 2, 2, color);
    }

    // Draw buildings
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bldg = Buildings_Get(i);
        if (!bldg || !bldg->active) continue;

        int px = offsetX + (int)(bldg->cellX * scale);
        int py = offsetY + (int)(bldg->cellY * scale);
        int pw = (int)(bldg->width * scale);
        int ph = (int)(bldg->height * scale);
        if (pw < 2) pw = 2;
        if (ph < 2) ph = 2;

        uint8_t color = (bldg->team == TEAM_PLAYER) ? PAL_LTGREEN : PAL_RED;
        Renderer_FillRect(px, py, pw, ph, color);
    }

    // Draw viewport rectangle
    Viewport* vp = Map_GetViewport();
    if (vp) {
        int vpCellX = vp->x / CELL_SIZE;
        int vpCellY = vp->y / CELL_SIZE;
        int vpCellW = vp->width / CELL_SIZE;
        int vpCellH = vp->height / CELL_SIZE;

        int vpx = offsetX + (int)(vpCellX * scale);
        int vpy = offsetY + (int)(vpCellY * scale);
        int vpw = (int)(vpCellW * scale);
        int vph = (int)(vpCellH * scale);

        // Pulsing viewport rectangle
        uint8_t cursorColor = (g_radarPulse < 15) ? PAL_WHITE : PAL_LTGREEN;
        Renderer_DrawRect(vpx, vpy, vpw, vph, cursorColor);
    }
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

void GameUI_RenderSidebar(void) {
    int startY = STRIP_Y;

    // Section: STRUCTURES
    DrawBeveledBox(SIDEBAR_X + 3, startY, SIDEBAR_WIDTH - 6, 12, PAL_GREY, true);
    Renderer_DrawText("STRUCTURES", SIDEBAR_X + 8, startY + 2, PAL_BLACK, 0);
    startY += 14;

    // Structure buttons
    for (int i = 0; i < 4 && i < g_structureDefCount; i++) {
        const BuildItemDef* item = &g_structureDefs[i];
        bool available = CheckPrerequisites(item) && g_playerCredits >= item->cost;
        bool isBuilding = (g_structureProducing == i);
        int progress = isBuilding ? (g_structureProgress / 100) : 0;  // Convert from fixed point

        uint8_t bgColor = available ? PAL_GREY : PAL_BLACK;
        uint8_t textColor = available ? PAL_WHITE : PAL_GREY;

        // Button with 3D effect
        DrawBeveledBox(SIDEBAR_X + 4, startY, SIDEBAR_WIDTH - 8, 20, bgColor, available && !isBuilding);

        // Item name
        Renderer_DrawText(item->name, SIDEBAR_X + 8, startY + 2, textColor, 0);

        // Cost or progress
        if (isBuilding) {
            char progressStr[16];
            snprintf(progressStr, sizeof(progressStr), "%d%%", progress);
            Renderer_DrawText(progressStr, SIDEBAR_X + 8, startY + 11, PAL_LTGREEN, 0);

            // Progress bar
            int barW = ((SIDEBAR_WIDTH - 16) * progress) / 100;
            Renderer_FillRect(SIDEBAR_X + 8, startY + 16, barW, 2, PAL_LTGREEN);
        } else if (available) {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%d", item->cost);
            Renderer_DrawText(costStr, SIDEBAR_X + 8, startY + 11, PAL_YELLOW, 0);
        }

        startY += 22;
    }

    startY += 4;

    // Section: UNITS
    DrawBeveledBox(SIDEBAR_X + 3, startY, SIDEBAR_WIDTH - 6, 12, PAL_GREY, true);
    Renderer_DrawText("UNITS", SIDEBAR_X + 8, startY + 2, PAL_BLACK, 0);
    startY += 14;

    // Unit buttons
    for (int i = 0; i < 4 && i < g_unitDefCount; i++) {
        const BuildItemDef* item = &g_unitDefs[i];
        bool available = CheckPrerequisites(item) && g_playerCredits >= item->cost;
        bool isBuilding = (g_unitProducing == i);
        int progress = isBuilding ? (g_unitProgress / 100) : 0;

        uint8_t bgColor = available ? PAL_GREY : PAL_BLACK;
        uint8_t textColor = available ? PAL_WHITE : PAL_GREY;

        DrawBeveledBox(SIDEBAR_X + 4, startY, SIDEBAR_WIDTH - 8, 20, bgColor, available && !isBuilding);

        Renderer_DrawText(item->name, SIDEBAR_X + 8, startY + 2, textColor, 0);

        if (isBuilding) {
            char progressStr[16];
            snprintf(progressStr, sizeof(progressStr), "%d%%", progress);
            Renderer_DrawText(progressStr, SIDEBAR_X + 8, startY + 11, PAL_LTGREEN, 0);

            int barW = ((SIDEBAR_WIDTH - 16) * progress) / 100;
            Renderer_FillRect(SIDEBAR_X + 8, startY + 16, barW, 2, PAL_LTGREEN);
        } else if (available) {
            char costStr[16];
            snprintf(costStr, sizeof(costStr), "$%d", item->cost);
            Renderer_DrawText(costStr, SIDEBAR_X + 8, startY + 11, PAL_YELLOW, 0);
        }

        startY += 22;
    }
}

BOOL GameUI_SidebarClick(int mouseX, int mouseY, BOOL leftClick) {
    (void)leftClick;
    (void)mouseX;

    int startY = STRIP_Y + 14;  // After "STRUCTURES" header

    // Check structure buttons
    for (int i = 0; i < 4 && i < g_structureDefCount; i++) {
        if (mouseY >= startY && mouseY < startY + 20) {
            BuildItemDef* item = &g_structureDefs[i];

            // Check if already building something
            if (g_structureProducing >= 0) {
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
            g_structureProducing = i;
            g_structureProgress = 0;
            return TRUE;
        }
        startY += 22;
    }

    startY += 4 + 14;  // Skip gap and "UNITS" header

    // Check unit buttons
    for (int i = 0; i < 4 && i < g_unitDefCount; i++) {
        if (mouseY >= startY && mouseY < startY + 20) {
            BuildItemDef* item = &g_unitDefs[i];

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
            g_unitProducing = i;
            g_unitProgress = 0;
            return TRUE;
        }
        startY += 22;
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
