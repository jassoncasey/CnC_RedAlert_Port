/**
 * Red Alert macOS Port - Game UI
 *
 * Renders sidebar, radar minimap, and selection panel for gameplay.
 * Uses palette-based rendering to integrate with the demo game.
 */

#ifndef UI_GAME_UI_H
#define UI_GAME_UI_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

//===========================================================================
// Screen Layout Constants (640x400, scaled 2x from 320x200)
//===========================================================================
// Original Red Alert used 320x200 base resolution. We use 640x400.
// All coordinates are 2x the original values (RESFACTOR=2).

// Game view (main terrain/unit area)
#define GAME_VIEW_X         0
#define GAME_VIEW_Y         0
#define GAME_VIEW_WIDTH     480          // 240 * 2 (sidebar starts at 240)
#define GAME_VIEW_HEIGHT    400

// Sidebar (right side) - original: X=240, Y=0, W=80
#define SIDEBAR_X           480          // 240 * 2
#define SIDEBAR_Y           0
#define SIDEBAR_WIDTH       160          // 80 * 2
#define SIDEBAR_HEIGHT      400          // Full height

// Radar (top of sidebar) - original: 64x64 at top
#define RADAR_X             (SIDEBAR_X + 8)
#define RADAR_Y             8
#define RADAR_WIDTH         144          // ~72 * 2
#define RADAR_HEIGHT        144

// Top buttons (Repair/Sell/Zoom) - below radar
#define TOP_BUTTONS_Y       (RADAR_Y + RADAR_HEIGHT + 4)
#define TOP_BUTTONS_HEIGHT  18           // 9 * 2

// Build strips (2 columns, below top buttons)
#define STRIP_Y             (TOP_BUTTONS_Y + TOP_BUTTONS_HEIGHT + 4)
#define STRIP_COLUMN_WIDTH  64           // 32 * 2 (cameo width)
#define STRIP_ITEM_WIDTH    64           // 32 * 2
#define STRIP_ITEM_HEIGHT   48           // 24 * 2
#define STRIP_ITEMS_VISIBLE 4            // 4 visible rows
#define STRIP_COLUMNS       2            // 2 columns
#define STRIP_COL1_X        (SIDEBAR_X + 8)
#define STRIP_COL2_X        (SIDEBAR_X + 8 + STRIP_COLUMN_WIDTH + 8)

// Scroll buttons (below strips)
#define SCROLL_BUTTONS_Y    (STRIP_Y + STRIP_ITEMS_VISIBLE * STRIP_ITEM_HEIGHT)
#define SCROLL_BUTTON_SIZE  24           // 12 * 2

// Selection panel (bottom of sidebar)
#define SELECTION_Y         (SIDEBAR_HEIGHT - 80)
#define SELECTION_HEIGHT    80

// Power bar (right edge of sidebar)
#define POWER_BAR_X         (SIDEBAR_X + SIDEBAR_WIDTH - 16)
#define POWER_BAR_Y         (TOP_BUTTONS_Y)
#define POWER_BAR_WIDTH     12
#define POWER_BAR_HEIGHT    (SELECTION_Y - TOP_BUTTONS_Y - 8)

// Credits display (top-left of game view)
#define CREDITS_X           8
#define CREDITS_Y           4
#define CREDITS_WIDTH       100
#define CREDITS_HEIGHT      16

// Options button (top-right of game view, before sidebar)
#define OPTIONS_BTN_WIDTH   56
#define OPTIONS_BTN_HEIGHT  16
#define OPTIONS_BTN_X       (SIDEBAR_X - OPTIONS_BTN_WIDTH - 8)
#define OPTIONS_BTN_Y       4

// Mission timer (next to credits)
#define TIMER_X             (CREDITS_X + CREDITS_WIDTH + 16)
#define TIMER_Y             CREDITS_Y
#define TIMER_WIDTH         48
#define TIMER_HEIGHT        16

//===========================================================================
// Palette Color Indices
//===========================================================================

// Standard UI colors (from snow.pal)
#define UI_COLOR_BLACK      0
#define UI_COLOR_DARK_GRAY  8
#define UI_COLOR_GRAY       7
#define UI_COLOR_LIGHT_GRAY 15
#define UI_COLOR_WHITE      15

#define UI_COLOR_RED        12      // Enemy/damage
#define UI_COLOR_GREEN      10      // Player/health
#define UI_COLOR_BLUE       9       // Water/selection
#define UI_COLOR_YELLOW     14      // Credits/highlights

#define UI_COLOR_PLAYER     10      // Player units (green)
#define UI_COLOR_ENEMY      12      // Enemy units (red)
#define UI_COLOR_NEUTRAL    6       // Neutral (cyan/gray)

//===========================================================================
// Functions
//===========================================================================

/**
 * Initialize the game UI system
 */
void GameUI_Init(void);

/**
 * Shutdown the game UI system
 */
void GameUI_Shutdown(void);

/**
 * Update UI logic (animations, selections)
 */
void GameUI_Update(void);

/**
 * Render all UI elements
 */
void GameUI_Render(void);

/**
 * Handle input in UI area
 * @return TRUE if input was consumed by UI
 */
BOOL GameUI_HandleInput(int mouseX, int mouseY,
                        BOOL leftClick, BOOL rightClick);

//===========================================================================
// Radar Functions
//===========================================================================

/**
 * Render the radar minimap
 */
void GameUI_RenderRadar(void);

/**
 * Handle click on radar
 * @return TRUE if click was consumed
 */
BOOL GameUI_RadarClick(int mouseX, int mouseY);

/**
 * Convert radar click to world coordinates
 */
void GameUI_RadarToWorld(int radarX, int radarY, int* worldX, int* worldY);

//===========================================================================
// Sidebar Functions
//===========================================================================

/**
 * Render the sidebar (build strips)
 */
void GameUI_RenderSidebar(void);

/**
 * Handle click on sidebar
 * @return TRUE if click was consumed
 */
BOOL GameUI_SidebarClick(int mouseX, int mouseY, BOOL leftClick);

//===========================================================================
// Selection Panel Functions
//===========================================================================

/**
 * Render the selection panel (unit info)
 */
void GameUI_RenderSelectionPanel(void);

//===========================================================================
// HUD Functions
//===========================================================================

/**
 * Render the top HUD bar
 */
void GameUI_RenderHUD(void);

//===========================================================================
// Mission Timer Functions (UI-1)
//===========================================================================

/**
 * Set mission timer (frames at 15 FPS). -1 disables timer.
 */
void GameUI_SetMissionTimer(int frames);

/**
 * Get current mission timer value (-1 if disabled)
 */
int GameUI_GetMissionTimer(void);

/**
 * Tick the mission timer (call once per frame)
 */
void GameUI_TickMissionTimer(void);

//===========================================================================
// Placement Functions
//===========================================================================

/**
 * Check if we're in building placement mode
 */
bool GameUI_IsPlacementMode(void);

/**
 * Update placement cursor position based on mouse
 */
void GameUI_UpdatePlacement(int mouseX, int mouseY);

/**
 * Handle ESC key to cancel placement
 * @return true if ESC was handled (placement was cancelled)
 */
bool GameUI_HandleEscape(void);

/**
 * Render the placement footprint cursor
 */
void GameUI_RenderPlacement(void);

//===========================================================================
// Credits Functions
//===========================================================================

/**
 * Get current player credits
 */
int GameUI_GetCredits(void);

/**
 * Set player credits
 */
void GameUI_SetCredits(int credits);

/**
 * Add to player credits
 */
void GameUI_AddCredits(int amount);

#ifdef __cplusplus
}
#endif

#endif // UI_GAME_UI_H
