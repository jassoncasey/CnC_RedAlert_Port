/**
 * Red Alert macOS Port - Menu System
 *
 * Simple menu/UI widget system for game menus.
 */

#ifndef UI_MENU_H
#define UI_MENU_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum items per menu
#define MENU_MAX_ITEMS 16

// Menu item types
typedef enum {
    MENU_ITEM_BUTTON,      // Clickable button
    MENU_ITEM_LABEL,       // Static text
    MENU_ITEM_SEPARATOR,   // Visual separator line
    MENU_ITEM_SLIDER,      // Value slider (0-100)
    MENU_ITEM_TOGGLE       // On/off toggle
} MenuItemType;

// Menu item state
typedef enum {
    MENU_STATE_NORMAL,
    MENU_STATE_HOVER,
    MENU_STATE_PRESSED,
    MENU_STATE_DISABLED
} MenuItemState;

// Callback for menu actions
typedef void (*MenuCallback)(int itemId, int value);

// Menu item definition
typedef struct {
    MenuItemType type;
    int id;                 // Unique ID for callbacks
    const char* text;       // Display text
    int x, y;               // Position
    int width, height;      // Size
    int value;              // Current value (for sliders/toggles)
    int minValue, maxValue; // Range (for sliders)
    BOOL enabled;           // Is item interactive?
    BOOL visible;           // Is item visible?
    MenuItemState state;    // Current visual state
    MenuCallback callback;  // Action callback
} MenuItem;

// Menu definition
typedef struct {
    const char* title;
    MenuItem items[MENU_MAX_ITEMS];
    int itemCount;
    int selectedIndex;      // Currently selected item (-1 = none)
    int hoveredIndex;       // Currently hovered item (-1 = none)
    BOOL active;            // Is menu currently active?
    uint8_t bgColor;        // Background color
    uint8_t textColor;      // Default text color
    uint8_t highlightColor; // Selected/hover color
    uint8_t disabledColor;  // Disabled text color
} Menu;

// Menu screen IDs
typedef enum {
    MENU_SCREEN_NONE,
    MENU_SCREEN_MAIN,
    MENU_SCREEN_CAMPAIGN_SELECT,
    MENU_SCREEN_DIFFICULTY_SELECT,
    MENU_SCREEN_BRIEFING,
    MENU_SCREEN_OPTIONS,
    MENU_SCREEN_CREDITS,
    MENU_SCREEN_INGAME
} MenuScreen;

// Campaign selection
typedef enum {
    CAMPAIGN_NONE = 0,
    CAMPAIGN_ALLIED,
    CAMPAIGN_SOVIET
} MenuCampaignChoice;

// Difficulty selection
typedef enum {
    DIFFICULTY_EASY = 0,
    DIFFICULTY_NORMAL,
    DIFFICULTY_HARD
} MenuDifficultyChoice;

/**
 * Initialize the menu system
 */
void Menu_Init(void);

/**
 * Shutdown the menu system
 */
void Menu_Shutdown(void);

/**
 * Create a new menu
 */
Menu* Menu_Create(const char* title);

/**
 * Destroy a menu
 */
void Menu_Destroy(Menu* menu);

/**
 * Add a button to a menu
 */
int Menu_AddButton(Menu* menu, int id, const char* text, int x, int y, int width, int height, MenuCallback callback);

/**
 * Add a label to a menu
 */
int Menu_AddLabel(Menu* menu, int id, const char* text, int x, int y);

/**
 * Add a separator line
 */
int Menu_AddSeparator(Menu* menu, int y);

/**
 * Add a slider
 */
int Menu_AddSlider(Menu* menu, int id, const char* text, int x, int y, int width,
                   int minVal, int maxVal, int currentVal, MenuCallback callback);

/**
 * Add a toggle (checkbox)
 */
int Menu_AddToggle(Menu* menu, int id, const char* text, int x, int y, BOOL currentVal, MenuCallback callback);

/**
 * Set menu colors
 */
void Menu_SetColors(Menu* menu, uint8_t bg, uint8_t text, uint8_t highlight, uint8_t disabled);

/**
 * Enable/disable a menu item
 */
void Menu_SetItemEnabled(Menu* menu, int id, BOOL enabled);

/**
 * Show/hide a menu item
 */
void Menu_SetItemVisible(Menu* menu, int id, BOOL visible);

/**
 * Set item value (for sliders/toggles)
 */
void Menu_SetItemValue(Menu* menu, int id, int value);

/**
 * Get item value
 */
int Menu_GetItemValue(Menu* menu, int id);

/**
 * Update menu (handle input)
 * Call once per frame.
 */
void Menu_Update(Menu* menu);

/**
 * Render a menu
 */
void Menu_Render(Menu* menu);

/**
 * Handle keyboard navigation
 */
void Menu_HandleKey(Menu* menu, int vkCode);

/**
 * Handle mouse input
 */
void Menu_HandleMouse(Menu* menu, int mouseX, int mouseY, BOOL leftDown, BOOL leftClicked);

/**
 * Get current menu screen
 */
MenuScreen Menu_GetCurrentScreen(void);

/**
 * Set current menu screen
 */
void Menu_SetCurrentScreen(MenuScreen screen);

/**
 * Get the main menu
 */
Menu* Menu_GetMainMenu(void);

/**
 * Get the options menu
 */
Menu* Menu_GetOptionsMenu(void);

/**
 * Get the campaign selection menu
 */
Menu* Menu_GetCampaignMenu(void);

/**
 * Get the difficulty selection menu
 */
Menu* Menu_GetDifficultyMenu(void);

/**
 * Get the selected campaign
 */
MenuCampaignChoice Menu_GetSelectedCampaign(void);

/**
 * Get the selected difficulty
 */
MenuDifficultyChoice Menu_GetSelectedDifficulty(void);

/**
 * Set callback for starting a new game
 * Callback receives campaign (1=allied, 2=soviet) and difficulty (0-2)
 */
typedef void (*NewGameCallback)(void);
typedef void (*StartCampaignCallback)(int campaign, int difficulty);
void Menu_SetNewGameCallback(NewGameCallback callback);
void Menu_SetStartCampaignCallback(StartCampaignCallback callback);

/**
 * Set up briefing screen
 * @param missionName   Mission title (e.g., "Allied Mission 1")
 * @param briefingText  Mission briefing text
 * @param callback      Called when user proceeds from briefing
 */
void Menu_SetBriefing(const char* missionName, const char* briefingText);

/**
 * Set callback for when user confirms briefing and starts mission
 */
typedef void (*BriefingConfirmCallback)(void);
void Menu_SetBriefingConfirmCallback(BriefingConfirmCallback callback);

/**
 * Get current briefing data
 */
const char* Menu_GetBriefingName(void);
const char* Menu_GetBriefingText(void);

/**
 * Render the briefing screen
 */
void Menu_RenderBriefing(void);

/**
 * Update the briefing screen (handle input)
 */
void Menu_UpdateBriefing(void);

#ifdef __cplusplus
}
#endif

#endif // UI_MENU_H
