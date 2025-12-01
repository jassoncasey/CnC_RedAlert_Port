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
// Screen Layout Constants (640x400)
//===========================================================================

// Game view (main terrain/unit area)
#define GAME_VIEW_X         0
#define GAME_VIEW_Y         16         // Below HUD bar
#define GAME_VIEW_WIDTH     560
#define GAME_VIEW_HEIGHT    368

// Sidebar (right side)
#define SIDEBAR_X           560
#define SIDEBAR_Y           0
#define SIDEBAR_WIDTH       80
#define SIDEBAR_HEIGHT      400

// Radar (top-right in sidebar)
#define RADAR_X             564
#define RADAR_Y             4
#define RADAR_WIDTH         72
#define RADAR_HEIGHT        72

// Build strips (in sidebar, below radar)
#define STRIP_Y             80
#define STRIP_HEIGHT        240
#define STRIP_ITEM_WIDTH    32
#define STRIP_ITEM_HEIGHT   24
#define STRIP_ITEMS_VISIBLE 4

// Selection panel (bottom of sidebar)
#define SELECTION_Y         324
#define SELECTION_HEIGHT    72

// HUD bar (top of screen)
#define HUD_X               0
#define HUD_Y               0
#define HUD_WIDTH           640
#define HUD_HEIGHT          16

// Control bar (bottom of screen)
#define CONTROL_Y           384
#define CONTROL_HEIGHT      16

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
BOOL GameUI_HandleInput(int mouseX, int mouseY, BOOL leftClick, BOOL rightClick);

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

#ifdef __cplusplus
}
#endif

#endif // UI_GAME_UI_H
