/**
 * Red Alert macOS Port - Menu System Implementation
 *
 * Styled to match the original Westwood Red Alert menus.
 */

#include "menu.h"
#include "graphics/metal/renderer.h"
#include "input/input.h"
#include "audio/audio.h"
#include "video/music.h"
#include "game/gameloop.h"
#include "assets/assetloader.h"
#include "compat/assets.h"
#include <cstring>
#include <cstdlib>

// Westwood-style palette indices for SNOW.PAL
// Standard VGA/Westwood palette layout:
// 0 = black (transparent), 1-15 = grayscale ramp
// 16-31 = reds, 32-47 = oranges, 48-63 = yellows
// 112-127 = red ramp, 176-191 = blue ramp
#define PAL_BLACK       0
#define PAL_DARKGREY    4     // Dark grey
#define PAL_GREY        8     // Medium grey
#define PAL_LTGREY      12    // Light grey
#define PAL_WHITE       15    // Bright white
#define PAL_RED         122   // Bright red
#define PAL_DARKRED     118   // Dark red
#define PAL_GOLD        223   // Gold/yellow (high end of yellow ramp)
#define PAL_YELLOW      220   // Yellow
#define PAL_GREEN       172   // Green
#define PAL_DARKGREEN   168
#define PAL_BLUE        186   // Blue
#define PAL_DARKBLUE    180

// Button color scheme - Westwood-style red/grey buttons
#define BTN_FACE        8     // Button face color (medium grey)
#define BTN_HIGHLIGHT   12    // Top/left edge (light grey)
#define BTN_SHADOW      2     // Bottom/right edge (dark)
#define BTN_TEXT        15    // Normal text (white)
#define BTN_TEXT_HOVER  223   // Hovered text (gold/yellow)
#define BTN_TEXT_DISABLED 6   // Disabled text (medium grey)
#define BTN_FACE_HOVER  10    // Lighter grey on hover
#define BTN_FACE_PRESSED 3    // Dark when pressed

// Global state
static MenuScreen g_currentScreen = MENU_SCREEN_NONE;
static Menu* g_mainMenu = nullptr;
static Menu* g_optionsMenu = nullptr;
static Menu* g_campaignMenu = nullptr;
static Menu* g_difficultyMenu = nullptr;
static NewGameCallback g_newGameCallback = nullptr;
static StartCampaignCallback g_startCampaignCallback = nullptr;
static MenuCampaignChoice g_selectedCampaign = CAMPAIGN_NONE;
static MenuDifficultyChoice g_selectedDifficulty = DIFFICULTY_NORMAL;

// Briefing state
static char g_briefingName[128] = "";
static char g_briefingText[1024] = "";
static BriefingConfirmCallback g_briefingConfirmCallback = nullptr;

// Sound effects for menus
static AudioSample* g_clickSound = nullptr;
static AudioSample* g_hoverSound = nullptr;

// Animation frame counter
static int g_menuFrame = 0;

// Forward declarations for callbacks
static void OnMainMenuButton(int itemId, int value);
static void OnOptionsButton(int itemId, int value);
static void OnCampaignButton(int itemId, int value);
static void OnDifficultyButton(int itemId, int value);

// Button IDs
enum {
    BTN_NEW_GAME = 1,
    BTN_LOAD_GAME,
    BTN_MULTIPLAYER,
    BTN_OPTIONS,
    BTN_CREDITS,
    BTN_EXIT,
    BTN_BACK,
    SLD_SOUND_VOL,
    SLD_MUSIC_VOL,
    TGL_FULLSCREEN,
    BTN_ALLIED_CAMPAIGN,
    BTN_SOVIET_CAMPAIGN,
    BTN_SKIRMISH,
    BTN_EASY,
    BTN_NORMAL,
    BTN_HARD
};

//===========================================================================
// Background Rendering
//===========================================================================

static void DrawMenuBackground(void) {
    // Dark background with subtle gradient
    Renderer_Clear(PAL_BLACK);

    // Draw subtle dark grey frame around menu area
    for (int y = 80; y < 340; y++) {
        // Subtle gradient - darker at edges
        uint8_t color = (y < 100 || y > 320) ? 1 : 2;
        Renderer_HLine(80, 560, y, color);
    }

    // Title banner area (top section)
    // Gradient red banner from dark to bright
    for (int y = 15; y < 75; y++) {
        // Red gradient - darker at top, brighter in middle
        int intensity = 115 + (y - 15) / 4;  // 115-130 range
        if (y > 50) intensity = 130 - (y - 50) / 4;  // Darken again
        if (intensity < 115) intensity = 115;
        if (intensity > 127) intensity = 127;
        Renderer_HLine(80, 560, y, (uint8_t)intensity);
    }

    // Banner border - beveled effect
    // Top highlight
    Renderer_HLine(80, 560, 14, 127);   // Bright red top
    Renderer_HLine(80, 560, 15, 124);   // Slightly darker
    // Bottom shadow
    Renderer_HLine(80, 560, 75, 112);   // Dark red bottom
    Renderer_HLine(80, 560, 76, 1);     // Very dark
    // Side borders
    Renderer_VLine(79, 14, 76, 127);    // Left bright
    Renderer_VLine(80, 14, 76, 124);
    Renderer_VLine(560, 14, 76, 112);   // Right dark
    Renderer_VLine(561, 14, 76, 1);

    // Bottom decorative bar
    for (int y = 360; y < 385; y++) {
        uint8_t color = (y == 360 || y == 384) ? 4 : 2;
        Renderer_HLine(80, 560, y, color);
    }
}

//===========================================================================
// 3D Beveled Button Drawing
//===========================================================================

static void DrawBeveledButton(int x, int y, int w, int h, bool pressed, bool hover, bool enabled) {
    // Button face
    uint8_t faceColor;
    if (!enabled) {
        faceColor = BTN_FACE;
    } else if (pressed) {
        faceColor = BTN_FACE_PRESSED;
    } else if (hover) {
        faceColor = BTN_FACE_HOVER;
    } else {
        faceColor = BTN_FACE;
    }

    Renderer_FillRect(x + 1, y + 1, w - 2, h - 2, faceColor);

    if (!pressed) {
        // Raised button - light top/left, dark bottom/right
        // Top edge (highlight)
        Renderer_HLine(x, x + w - 1, y, BTN_HIGHLIGHT);
        Renderer_HLine(x + 1, x + w - 2, y + 1, BTN_HIGHLIGHT);

        // Left edge (highlight)
        Renderer_VLine(x, y, y + h - 1, BTN_HIGHLIGHT);
        Renderer_VLine(x + 1, y + 1, y + h - 2, BTN_HIGHLIGHT);

        // Bottom edge (shadow)
        Renderer_HLine(x, x + w - 1, y + h - 1, BTN_SHADOW);
        Renderer_HLine(x + 1, x + w - 2, y + h - 2, BTN_SHADOW);

        // Right edge (shadow)
        Renderer_VLine(x + w - 1, y, y + h - 1, BTN_SHADOW);
        Renderer_VLine(x + w - 2, y + 1, y + h - 2, BTN_SHADOW);
    } else {
        // Pressed button - dark top/left, light bottom/right (inverted)
        // Top edge (shadow)
        Renderer_HLine(x, x + w - 1, y, BTN_SHADOW);
        Renderer_HLine(x + 1, x + w - 2, y + 1, BTN_SHADOW);

        // Left edge (shadow)
        Renderer_VLine(x, y, y + h - 1, BTN_SHADOW);
        Renderer_VLine(x + 1, y + 1, y + h - 2, BTN_SHADOW);

        // Bottom edge (highlight)
        Renderer_HLine(x, x + w - 1, y + h - 1, BTN_HIGHLIGHT);

        // Right edge (highlight)
        Renderer_VLine(x + w - 1, y, y + h - 1, BTN_HIGHLIGHT);
    }
}

//===========================================================================
// Menu Initialization
//===========================================================================

void Menu_Init(void) {
    // Create click sound (short high beep)
    g_clickSound = Audio_CreateTestTone(880, 50);
    g_hoverSound = Audio_CreateTestTone(440, 30);

    // Create main menu
    g_mainMenu = Menu_Create("RED ALERT");
    Menu_SetColors(g_mainMenu, PAL_BLACK, BTN_TEXT, BTN_TEXT_HOVER, BTN_TEXT_DISABLED);

    int centerX = 320;
    int btnWidth = 180;
    int btnHeight = 24;
    int startY = 120;
    int spacing = 32;

    // Title labels are handled specially in render

    Menu_AddButton(g_mainMenu, BTN_NEW_GAME, "START NEW GAME", centerX - btnWidth/2, startY, btnWidth, btnHeight, OnMainMenuButton);
    Menu_AddButton(g_mainMenu, BTN_LOAD_GAME, "LOAD MISSION", centerX - btnWidth/2, startY + spacing, btnWidth, btnHeight, OnMainMenuButton);
    Menu_SetItemEnabled(g_mainMenu, BTN_LOAD_GAME, FALSE); // Not implemented
    Menu_AddButton(g_mainMenu, BTN_MULTIPLAYER, "MULTIPLAYER GAME", centerX - btnWidth/2, startY + spacing*2, btnWidth, btnHeight, OnMainMenuButton);
    Menu_SetItemEnabled(g_mainMenu, BTN_MULTIPLAYER, FALSE); // Not implemented

    Menu_AddButton(g_mainMenu, BTN_OPTIONS, "OPTIONS", centerX - btnWidth/2, startY + spacing*3, btnWidth, btnHeight, OnMainMenuButton);
    Menu_AddButton(g_mainMenu, BTN_CREDITS, "INTRO & CREDITS", centerX - btnWidth/2, startY + spacing*4, btnWidth, btnHeight, OnMainMenuButton);
    Menu_AddButton(g_mainMenu, BTN_EXIT, "EXIT GAME", centerX - btnWidth/2, startY + spacing*5 + 16, btnWidth, btnHeight, OnMainMenuButton);

    // Create options menu
    g_optionsMenu = Menu_Create("OPTIONS");
    Menu_SetColors(g_optionsMenu, PAL_BLACK, BTN_TEXT, BTN_TEXT_HOVER, BTN_TEXT_DISABLED);

    Menu_AddSlider(g_optionsMenu, SLD_SOUND_VOL, "SOUND VOLUME", centerX - 100, 140, 200, 0, 255, 255, OnOptionsButton);
    Menu_AddSlider(g_optionsMenu, SLD_MUSIC_VOL, "MUSIC VOLUME", centerX - 100, 200, 200, 0, 255, 200, OnOptionsButton);
    Menu_AddToggle(g_optionsMenu, TGL_FULLSCREEN, "FULLSCREEN", centerX - 80, 260, FALSE, OnOptionsButton);

    Menu_AddButton(g_optionsMenu, BTN_BACK, "BACK", centerX - btnWidth/2, 320, btnWidth, btnHeight, OnOptionsButton);

    // Create campaign selection menu
    g_campaignMenu = Menu_Create("SELECT CAMPAIGN");
    Menu_SetColors(g_campaignMenu, PAL_BLACK, BTN_TEXT, BTN_TEXT_HOVER, BTN_TEXT_DISABLED);

    Menu_AddButton(g_campaignMenu, BTN_ALLIED_CAMPAIGN, "ALLIED CAMPAIGN", centerX - btnWidth/2, 140, btnWidth, btnHeight, OnCampaignButton);
    Menu_AddButton(g_campaignMenu, BTN_SOVIET_CAMPAIGN, "SOVIET CAMPAIGN", centerX - btnWidth/2, 140 + spacing, btnWidth, btnHeight, OnCampaignButton);
    Menu_AddButton(g_campaignMenu, BTN_SKIRMISH, "SKIRMISH BATTLE", centerX - btnWidth/2, 140 + spacing*2, btnWidth, btnHeight, OnCampaignButton);
    Menu_AddButton(g_campaignMenu, BTN_BACK, "BACK", centerX - btnWidth/2, 140 + spacing*4, btnWidth, btnHeight, OnCampaignButton);

    // Create difficulty selection menu
    g_difficultyMenu = Menu_Create("SELECT DIFFICULTY");
    Menu_SetColors(g_difficultyMenu, PAL_BLACK, BTN_TEXT, BTN_TEXT_HOVER, BTN_TEXT_DISABLED);

    Menu_AddButton(g_difficultyMenu, BTN_EASY, "EASY", centerX - btnWidth/2, 140, btnWidth, btnHeight, OnDifficultyButton);
    Menu_AddButton(g_difficultyMenu, BTN_NORMAL, "NORMAL", centerX - btnWidth/2, 140 + spacing, btnWidth, btnHeight, OnDifficultyButton);
    Menu_AddButton(g_difficultyMenu, BTN_HARD, "HARD", centerX - btnWidth/2, 140 + spacing*2, btnWidth, btnHeight, OnDifficultyButton);
    Menu_AddButton(g_difficultyMenu, BTN_BACK, "BACK", centerX - btnWidth/2, 140 + spacing*4, btnWidth, btnHeight, OnDifficultyButton);
}

void Menu_Shutdown(void) {
    if (g_mainMenu) {
        Menu_Destroy(g_mainMenu);
        g_mainMenu = nullptr;
    }
    if (g_optionsMenu) {
        Menu_Destroy(g_optionsMenu);
        g_optionsMenu = nullptr;
    }
    if (g_campaignMenu) {
        Menu_Destroy(g_campaignMenu);
        g_campaignMenu = nullptr;
    }
    if (g_difficultyMenu) {
        Menu_Destroy(g_difficultyMenu);
        g_difficultyMenu = nullptr;
    }
    if (g_clickSound) {
        Audio_FreeTestTone(g_clickSound);
        g_clickSound = nullptr;
    }
    if (g_hoverSound) {
        Audio_FreeTestTone(g_hoverSound);
        g_hoverSound = nullptr;
    }
}

Menu* Menu_Create(const char* title) {
    Menu* menu = new Menu;
    memset(menu, 0, sizeof(Menu));
    menu->title = title;
    menu->selectedIndex = -1;
    menu->hoveredIndex = -1;
    menu->active = TRUE;
    menu->bgColor = 0;
    menu->textColor = BTN_TEXT;
    menu->highlightColor = BTN_TEXT_HOVER;
    menu->disabledColor = BTN_TEXT_DISABLED;
    return menu;
}

void Menu_Destroy(Menu* menu) {
    if (menu) {
        delete menu;
    }
}

static int Menu_FindFreeSlot(Menu* menu) {
    if (menu->itemCount >= MENU_MAX_ITEMS) return -1;
    return menu->itemCount++;
}

int Menu_AddButton(Menu* menu, int id, const char* text, int x, int y, int width, int height, MenuCallback callback) {
    int idx = Menu_FindFreeSlot(menu);
    if (idx < 0) return -1;

    MenuItem* item = &menu->items[idx];
    item->type = MENU_ITEM_BUTTON;
    item->id = id;
    item->text = text;
    item->x = x;
    item->y = y;
    item->width = width;
    item->height = height;
    item->enabled = TRUE;
    item->visible = TRUE;
    item->state = MENU_STATE_NORMAL;
    item->callback = callback;

    return idx;
}

int Menu_AddLabel(Menu* menu, int id, const char* text, int x, int y) {
    int idx = Menu_FindFreeSlot(menu);
    if (idx < 0) return -1;

    MenuItem* item = &menu->items[idx];
    item->type = MENU_ITEM_LABEL;
    item->id = id;
    item->text = text;
    item->x = x;
    item->y = y;
    item->enabled = FALSE; // Labels aren't interactive
    item->visible = TRUE;

    return idx;
}

int Menu_AddSeparator(Menu* menu, int y) {
    int idx = Menu_FindFreeSlot(menu);
    if (idx < 0) return -1;

    MenuItem* item = &menu->items[idx];
    item->type = MENU_ITEM_SEPARATOR;
    item->y = y;
    item->enabled = FALSE;
    item->visible = TRUE;

    return idx;
}

int Menu_AddSlider(Menu* menu, int id, const char* text, int x, int y, int width,
                   int minVal, int maxVal, int currentVal, MenuCallback callback) {
    int idx = Menu_FindFreeSlot(menu);
    if (idx < 0) return -1;

    MenuItem* item = &menu->items[idx];
    item->type = MENU_ITEM_SLIDER;
    item->id = id;
    item->text = text;
    item->x = x;
    item->y = y;
    item->width = width;
    item->height = 20;
    item->minValue = minVal;
    item->maxValue = maxVal;
    item->value = currentVal;
    item->enabled = TRUE;
    item->visible = TRUE;
    item->callback = callback;

    return idx;
}

int Menu_AddToggle(Menu* menu, int id, const char* text, int x, int y, BOOL currentVal, MenuCallback callback) {
    int idx = Menu_FindFreeSlot(menu);
    if (idx < 0) return -1;

    MenuItem* item = &menu->items[idx];
    item->type = MENU_ITEM_TOGGLE;
    item->id = id;
    item->text = text;
    item->x = x;
    item->y = y;
    item->width = 200;
    item->height = 25;
    item->value = currentVal ? 1 : 0;
    item->enabled = TRUE;
    item->visible = TRUE;
    item->callback = callback;

    return idx;
}

void Menu_SetColors(Menu* menu, uint8_t bg, uint8_t text, uint8_t highlight, uint8_t disabled) {
    menu->bgColor = bg;
    menu->textColor = text;
    menu->highlightColor = highlight;
    menu->disabledColor = disabled;
}

static MenuItem* Menu_FindItem(Menu* menu, int id) {
    for (int i = 0; i < menu->itemCount; i++) {
        if (menu->items[i].id == id) {
            return &menu->items[i];
        }
    }
    return nullptr;
}

void Menu_SetItemEnabled(Menu* menu, int id, BOOL enabled) {
    MenuItem* item = Menu_FindItem(menu, id);
    if (item) {
        item->enabled = enabled;
        if (!enabled) item->state = MENU_STATE_DISABLED;
        else item->state = MENU_STATE_NORMAL;
    }
}

void Menu_SetItemVisible(Menu* menu, int id, BOOL visible) {
    MenuItem* item = Menu_FindItem(menu, id);
    if (item) item->visible = visible;
}

void Menu_SetItemValue(Menu* menu, int id, int value) {
    MenuItem* item = Menu_FindItem(menu, id);
    if (item) {
        if (item->type == MENU_ITEM_SLIDER) {
            if (value < item->minValue) value = item->minValue;
            if (value > item->maxValue) value = item->maxValue;
        }
        item->value = value;
    }
}

int Menu_GetItemValue(Menu* menu, int id) {
    MenuItem* item = Menu_FindItem(menu, id);
    return item ? item->value : 0;
}

void Menu_Update(Menu* menu) {
    if (!menu || !menu->active) return;

    g_menuFrame++;

    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    BOOL leftDown = (Input_GetMouseButtons() & INPUT_MOUSE_LEFT) != 0;
    static BOOL wasLeftDown = FALSE;
    BOOL leftClicked = !leftDown && wasLeftDown;

    Menu_HandleMouse(menu, mx, my, leftDown, leftClicked);

    // Keyboard navigation
    if (Input_WasKeyPressed(VK_UP)) {
        Menu_HandleKey(menu, VK_UP);
    }
    if (Input_WasKeyPressed(VK_DOWN)) {
        Menu_HandleKey(menu, VK_DOWN);
    }
    if (Input_WasKeyPressed(VK_LEFT)) {
        Menu_HandleKey(menu, VK_LEFT);
    }
    if (Input_WasKeyPressed(VK_RIGHT)) {
        Menu_HandleKey(menu, VK_RIGHT);
    }
    if (Input_WasKeyPressed(VK_RETURN) || Input_WasKeyPressed(VK_SPACE)) {
        Menu_HandleKey(menu, VK_RETURN);
    }

    wasLeftDown = leftDown;
}

void Menu_Render(Menu* menu) {
    if (!menu) return;

    // Set menu palette - terrain palettes (SNOW.PAL etc) don't have correct UI colors
    Palette menuPal;
    StubAssets_CreatePalette(&menuPal);
    Renderer_SetPalette(&menuPal);

    // Draw styled background
    DrawMenuBackground();

    // Draw title text in banner
    // "COMMAND & CONQUER" in gold/yellow
    Renderer_DrawText("COMMAND & CONQUER", 220, 28, PAL_GOLD, 0);
    // "RED ALERT" in white below (would be larger in original)
    Renderer_DrawText("RED ALERT", 268, 48, PAL_WHITE, 0);

    // Draw menu title if not main menu
    if (menu->title && strcmp(menu->title, "RED ALERT") != 0) {
        int titleLen = (int)strlen(menu->title) * 8;
        int titleX = 320 - titleLen / 2;
        Renderer_DrawText(menu->title, titleX, 92, PAL_YELLOW, 0);
    }

    // Draw menu items
    for (int i = 0; i < menu->itemCount; i++) {
        MenuItem* item = &menu->items[i];
        if (!item->visible) continue;

        bool isHovered = (item->state == MENU_STATE_HOVER || i == menu->selectedIndex);
        bool isPressed = (item->state == MENU_STATE_PRESSED);

        uint8_t textColor = menu->textColor;
        if (!item->enabled) textColor = menu->disabledColor;
        else if (isHovered) textColor = menu->highlightColor;

        switch (item->type) {
            case MENU_ITEM_BUTTON: {
                // Draw 3D beveled button
                DrawBeveledButton(item->x, item->y, item->width, item->height,
                                  isPressed, isHovered, item->enabled);

                // Center text (offset by 1 when pressed for depth effect)
                int textLen = (int)strlen(item->text) * 8;
                int textX = item->x + (item->width - textLen) / 2;
                int textY = item->y + (item->height - 8) / 2;
                if (isPressed) {
                    textX++;
                    textY++;
                }
                Renderer_DrawText(item->text, textX, textY, textColor, 0);
                break;
            }

            case MENU_ITEM_LABEL:
                Renderer_DrawText(item->text, item->x, item->y, textColor, 0);
                break;

            case MENU_ITEM_SEPARATOR:
                // Beveled separator line
                Renderer_HLine(120, 520, item->y, BTN_SHADOW);
                Renderer_HLine(120, 520, item->y + 1, BTN_HIGHLIGHT);
                break;

            case MENU_ITEM_SLIDER: {
                // Label
                Renderer_DrawText(item->text, item->x, item->y, textColor, 0);

                // Slider track (sunken)
                int trackY = item->y + 20;
                int trackH = 12;

                // Sunken border
                Renderer_HLine(item->x, item->x + item->width, trackY, BTN_SHADOW);
                Renderer_VLine(item->x, trackY, trackY + trackH, BTN_SHADOW);
                Renderer_HLine(item->x, item->x + item->width, trackY + trackH, BTN_HIGHLIGHT);
                Renderer_VLine(item->x + item->width, trackY, trackY + trackH, BTN_HIGHLIGHT);

                // Track fill (dark)
                Renderer_FillRect(item->x + 1, trackY + 1, item->width - 1, trackH - 1, 1);

                // Value bar (green)
                int fillWidth = (item->value - item->minValue) * (item->width - 2) / (item->maxValue - item->minValue);
                if (fillWidth > 0) {
                    Renderer_FillRect(item->x + 1, trackY + 1, fillWidth, trackH - 1, PAL_GREEN);
                }

                // Value text
                char valText[16];
                snprintf(valText, sizeof(valText), "%d", item->value);
                Renderer_DrawText(valText, item->x + item->width + 10, trackY + 2, textColor, 0);
                break;
            }

            case MENU_ITEM_TOGGLE: {
                // Checkbox (sunken box)
                int boxSize = 14;
                int boxY = item->y + 4;

                // Sunken border
                Renderer_HLine(item->x, item->x + boxSize, boxY, BTN_SHADOW);
                Renderer_VLine(item->x, boxY, boxY + boxSize, BTN_SHADOW);
                Renderer_HLine(item->x, item->x + boxSize, boxY + boxSize, BTN_HIGHLIGHT);
                Renderer_VLine(item->x + boxSize, boxY, boxY + boxSize, BTN_HIGHLIGHT);

                // Box fill
                Renderer_FillRect(item->x + 1, boxY + 1, boxSize - 1, boxSize - 1, 1);

                // Checkmark
                if (item->value) {
                    Renderer_FillRect(item->x + 3, boxY + 3, boxSize - 5, boxSize - 5, PAL_GREEN);
                }

                // Label
                Renderer_DrawText(item->text, item->x + boxSize + 10, item->y + 6, textColor, 0);
                break;
            }
        }
    }

    // Version info at bottom
    Renderer_DrawText("MACOS PORT - M45", 260, 370, PAL_DARKGREY, 0);
}

void Menu_HandleKey(Menu* menu, int vkCode) {
    if (!menu || !menu->active) return;

    // Find next/prev selectable item
    auto findSelectable = [menu](int start, int dir) -> int {
        int idx = start;
        for (int i = 0; i < menu->itemCount; i++) {
            idx += dir;
            if (idx < 0) idx = menu->itemCount - 1;
            if (idx >= menu->itemCount) idx = 0;
            if (menu->items[idx].enabled && menu->items[idx].visible) {
                return idx;
            }
        }
        return -1;
    };

    switch (vkCode) {
        case VK_UP:
            menu->selectedIndex = findSelectable(menu->selectedIndex, -1);
            if (g_hoverSound) Audio_Play(g_hoverSound, 100, 0, FALSE);
            break;

        case VK_DOWN:
            menu->selectedIndex = findSelectable(menu->selectedIndex, 1);
            if (g_hoverSound) Audio_Play(g_hoverSound, 100, 0, FALSE);
            break;

        case VK_LEFT:
            if (menu->selectedIndex >= 0) {
                MenuItem* item = &menu->items[menu->selectedIndex];
                if (item->type == MENU_ITEM_SLIDER) {
                    int step = (item->maxValue - item->minValue) / 10;
                    if (step < 1) step = 1;
                    item->value -= step;
                    if (item->value < item->minValue) item->value = item->minValue;
                    if (item->callback) item->callback(item->id, item->value);
                }
            }
            break;

        case VK_RIGHT:
            if (menu->selectedIndex >= 0) {
                MenuItem* item = &menu->items[menu->selectedIndex];
                if (item->type == MENU_ITEM_SLIDER) {
                    int step = (item->maxValue - item->minValue) / 10;
                    if (step < 1) step = 1;
                    item->value += step;
                    if (item->value > item->maxValue) item->value = item->maxValue;
                    if (item->callback) item->callback(item->id, item->value);
                }
            }
            break;

        case VK_RETURN:
        case VK_SPACE:
            if (menu->selectedIndex >= 0) {
                MenuItem* item = &menu->items[menu->selectedIndex];
                if (item->type == MENU_ITEM_BUTTON && item->callback) {
                    if (g_clickSound) Audio_Play(g_clickSound, 150, 0, FALSE);
                    item->callback(item->id, 0);
                } else if (item->type == MENU_ITEM_TOGGLE) {
                    item->value = !item->value;
                    if (g_clickSound) Audio_Play(g_clickSound, 150, 0, FALSE);
                    if (item->callback) item->callback(item->id, item->value);
                }
            }
            break;
    }
}

void Menu_HandleMouse(Menu* menu, int mouseX, int mouseY, BOOL leftDown, BOOL leftClicked) {
    if (!menu || !menu->active) return;

    int prevHovered = menu->hoveredIndex;
    menu->hoveredIndex = -1;

    for (int i = 0; i < menu->itemCount; i++) {
        MenuItem* item = &menu->items[i];
        if (!item->visible || !item->enabled) continue;

        // Check bounds based on item type
        int hitX = item->x;
        int hitY = item->y;
        int hitW = item->width;
        int hitH = item->height;

        if (item->type == MENU_ITEM_SLIDER) {
            hitH = 35; // Include label
        } else if (item->type == MENU_ITEM_TOGGLE) {
            hitW = 200;
        } else if (item->type == MENU_ITEM_LABEL || item->type == MENU_ITEM_SEPARATOR) {
            continue; // Not interactive
        }

        if (mouseX >= hitX && mouseX < hitX + hitW &&
            mouseY >= hitY && mouseY < hitY + hitH) {

            menu->hoveredIndex = i;
            item->state = leftDown ? MENU_STATE_PRESSED : MENU_STATE_HOVER;

            // Handle click
            if (leftClicked) {
                if (item->type == MENU_ITEM_BUTTON) {
                    if (g_clickSound) Audio_Play(g_clickSound, 150, 0, FALSE);
                    if (item->callback) item->callback(item->id, 0);
                } else if (item->type == MENU_ITEM_TOGGLE) {
                    item->value = !item->value;
                    if (g_clickSound) Audio_Play(g_clickSound, 150, 0, FALSE);
                    if (item->callback) item->callback(item->id, item->value);
                }
            }

            // Handle slider dragging
            if (item->type == MENU_ITEM_SLIDER && leftDown) {
                int trackY = item->y + 20;
                if (mouseY >= trackY && mouseY < trackY + 12) {
                    int relX = mouseX - item->x;
                    if (relX < 0) relX = 0;
                    if (relX > item->width) relX = item->width;
                    item->value = item->minValue + (relX * (item->maxValue - item->minValue)) / item->width;
                    if (item->callback) item->callback(item->id, item->value);
                }
            }
        } else {
            if (item->state != MENU_STATE_DISABLED) {
                item->state = MENU_STATE_NORMAL;
            }
        }
    }

    // Play hover sound when entering a new item
    if (menu->hoveredIndex != prevHovered && menu->hoveredIndex >= 0) {
        if (g_hoverSound) Audio_Play(g_hoverSound, 80, 0, FALSE);
    }
}

MenuScreen Menu_GetCurrentScreen(void) {
    return g_currentScreen;
}

void Menu_SetCurrentScreen(MenuScreen screen) {
    g_currentScreen = screen;
}

Menu* Menu_GetMainMenu(void) {
    return g_mainMenu;
}

Menu* Menu_GetOptionsMenu(void) {
    return g_optionsMenu;
}

// Callbacks
static void OnMainMenuButton(int itemId, int value) {
    (void)value;

    switch (itemId) {
        case BTN_NEW_GAME:
            // Go to campaign selection instead of directly starting
            Menu_SetCurrentScreen(MENU_SCREEN_CAMPAIGN_SELECT);
            break;

        case BTN_LOAD_GAME:
            // Not implemented
            break;

        case BTN_OPTIONS:
            Menu_SetCurrentScreen(MENU_SCREEN_OPTIONS);
            break;

        case BTN_CREDITS:
            Menu_SetCurrentScreen(MENU_SCREEN_CREDITS);
            break;

        case BTN_EXIT:
            // Signal quit
            GameLoop_Quit();
            break;
    }
}

static void OnCampaignButton(int itemId, int value) {
    (void)value;

    switch (itemId) {
        case BTN_ALLIED_CAMPAIGN:
            g_selectedCampaign = CAMPAIGN_ALLIED;
            Menu_SetCurrentScreen(MENU_SCREEN_DIFFICULTY_SELECT);
            break;

        case BTN_SOVIET_CAMPAIGN:
            g_selectedCampaign = CAMPAIGN_SOVIET;
            Menu_SetCurrentScreen(MENU_SCREEN_DIFFICULTY_SELECT);
            break;

        case BTN_SKIRMISH:
            // Skirmish = demo mode (for now)
            g_selectedCampaign = CAMPAIGN_NONE;
            Menu_SetCurrentScreen(MENU_SCREEN_NONE);
            if (g_newGameCallback) {
                g_newGameCallback();
            }
            break;

        case BTN_BACK:
            Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
            break;
    }
}

static void OnDifficultyButton(int itemId, int value) {
    (void)value;

    switch (itemId) {
        case BTN_EASY:
            g_selectedDifficulty = DIFFICULTY_EASY;
            Menu_SetCurrentScreen(MENU_SCREEN_NONE);
            if (g_startCampaignCallback) {
                g_startCampaignCallback((int)g_selectedCampaign, (int)g_selectedDifficulty);
            } else if (g_newGameCallback) {
                g_newGameCallback();
            }
            break;

        case BTN_NORMAL:
            g_selectedDifficulty = DIFFICULTY_NORMAL;
            Menu_SetCurrentScreen(MENU_SCREEN_NONE);
            if (g_startCampaignCallback) {
                g_startCampaignCallback((int)g_selectedCampaign, (int)g_selectedDifficulty);
            } else if (g_newGameCallback) {
                g_newGameCallback();
            }
            break;

        case BTN_HARD:
            g_selectedDifficulty = DIFFICULTY_HARD;
            Menu_SetCurrentScreen(MENU_SCREEN_NONE);
            if (g_startCampaignCallback) {
                g_startCampaignCallback((int)g_selectedCampaign, (int)g_selectedDifficulty);
            } else if (g_newGameCallback) {
                g_newGameCallback();
            }
            break;

        case BTN_BACK:
            Menu_SetCurrentScreen(MENU_SCREEN_CAMPAIGN_SELECT);
            break;
    }
}

static void OnOptionsButton(int itemId, int value) {
    switch (itemId) {
        case SLD_SOUND_VOL:
            // Sound volume controls sound effects only (not music)
            Audio_SetSoundVolume((uint8_t)value);
            break;

        case SLD_MUSIC_VOL:
            // Music volume controls background music
            Music_SetVolume((float)value / 255.0f);
            break;

        case TGL_FULLSCREEN:
            // TODO: Toggle fullscreen
            break;

        case BTN_BACK:
            Menu_SetCurrentScreen(MENU_SCREEN_MAIN);
            break;
    }
}

void Menu_SetNewGameCallback(NewGameCallback callback) {
    g_newGameCallback = callback;
}

void Menu_SetStartCampaignCallback(StartCampaignCallback callback) {
    g_startCampaignCallback = callback;
}

Menu* Menu_GetCampaignMenu(void) {
    return g_campaignMenu;
}

Menu* Menu_GetDifficultyMenu(void) {
    return g_difficultyMenu;
}

MenuCampaignChoice Menu_GetSelectedCampaign(void) {
    return g_selectedCampaign;
}

MenuDifficultyChoice Menu_GetSelectedDifficulty(void) {
    return g_selectedDifficulty;
}

//===========================================================================
// Briefing Screen
//===========================================================================

void Menu_SetBriefing(const char* missionName, const char* briefingText) {
    if (missionName) {
        strncpy(g_briefingName, missionName, sizeof(g_briefingName) - 1);
        g_briefingName[sizeof(g_briefingName) - 1] = '\0';
    } else {
        g_briefingName[0] = '\0';
    }

    if (briefingText) {
        strncpy(g_briefingText, briefingText, sizeof(g_briefingText) - 1);
        g_briefingText[sizeof(g_briefingText) - 1] = '\0';
    } else {
        g_briefingText[0] = '\0';
    }
}

void Menu_SetBriefingConfirmCallback(BriefingConfirmCallback callback) {
    g_briefingConfirmCallback = callback;
}

const char* Menu_GetBriefingName(void) {
    return g_briefingName;
}

const char* Menu_GetBriefingText(void) {
    return g_briefingText;
}

// Word-wrap text rendering helper
static int RenderWrappedText(const char* text, int x, int y, int maxWidth, uint8_t color) {
    if (!text || !*text) return y;

    const int charWidth = 8;
    const int lineHeight = 12;
    int maxCharsPerLine = maxWidth / charWidth;
    if (maxCharsPerLine < 10) maxCharsPerLine = 10;

    char line[256];
    int lineLen = 0;
    int currentY = y;
    const char* ptr = text;

    while (*ptr) {
        // Find end of current word
        const char* wordStart = ptr;
        while (*ptr && *ptr != ' ' && *ptr != '\n') ptr++;
        int wordLen = (int)(ptr - wordStart);

        // Check if word fits on current line
        if (lineLen + wordLen + (lineLen > 0 ? 1 : 0) <= maxCharsPerLine) {
            // Add space if not start of line
            if (lineLen > 0) {
                line[lineLen++] = ' ';
            }
            // Add word
            memcpy(line + lineLen, wordStart, wordLen);
            lineLen += wordLen;
        } else {
            // Output current line and start new one
            if (lineLen > 0) {
                line[lineLen] = '\0';
                Renderer_DrawText(line, x, currentY, color, 0);
                currentY += lineHeight;
            }
            // Start new line with current word
            memcpy(line, wordStart, wordLen);
            lineLen = wordLen;
        }

        // Handle newlines
        if (*ptr == '\n') {
            line[lineLen] = '\0';
            if (lineLen > 0) {
                Renderer_DrawText(line, x, currentY, color, 0);
            }
            currentY += lineHeight;
            lineLen = 0;
            ptr++;
        }

        // Skip spaces
        while (*ptr == ' ') ptr++;
    }

    // Output remaining line
    if (lineLen > 0) {
        line[lineLen] = '\0';
        Renderer_DrawText(line, x, currentY, color, 0);
        currentY += lineHeight;
    }

    return currentY;
}

void Menu_RenderBriefing(void) {
    // Use stub palette for menu UI - terrain palettes (SNOW.PAL etc) don't have correct UI colors
    // Create a local palette each frame so we don't affect gameplay
    Palette menuPal;
    StubAssets_CreatePalette(&menuPal);
    Renderer_SetPalette(&menuPal);

    // Reset clipping to full screen in case video left it restricted
    Renderer_ResetClip();

    // Dark background
    Renderer_Clear(PAL_BLACK);

    // Title banner (dark red gradient like other menus)
    for (int y = 10; y < 60; y++) {
        int intensity = 115 + (y - 10) / 3;
        if (y > 40) intensity = 125 - (y - 40) / 3;
        if (intensity < 115) intensity = 115;
        if (intensity > 125) intensity = 125;
        Renderer_HLine(40, 600, y, (uint8_t)intensity);
    }

    // Banner borders
    Renderer_HLine(40, 600, 9, 127);
    Renderer_HLine(40, 600, 60, 112);
    Renderer_VLine(39, 9, 60, 127);
    Renderer_VLine(600, 9, 60, 112);

    // Mission name in banner
    Renderer_DrawText("MISSION BRIEFING", 240, 20, PAL_GOLD, 0);
    if (g_briefingName[0]) {
        int nameLen = (int)strlen(g_briefingName) * 8;
        int nameX = 320 - nameLen / 2;
        Renderer_DrawText(g_briefingName, nameX, 40, PAL_WHITE, 0);
    }

    // Briefing text area (bordered box)
    int boxX = 40;
    int boxY = 70;
    int boxW = 560;
    int boxH = 260;

    // Sunken border for text area
    Renderer_HLine(boxX, boxX + boxW, boxY, BTN_SHADOW);
    Renderer_VLine(boxX, boxY, boxY + boxH, BTN_SHADOW);
    Renderer_HLine(boxX, boxX + boxW, boxY + boxH, BTN_HIGHLIGHT);
    Renderer_VLine(boxX + boxW, boxY, boxY + boxH, BTN_HIGHLIGHT);

    // Dark fill
    Renderer_FillRect(boxX + 1, boxY + 1, boxW - 1, boxH - 1, 1);

    // Render briefing text with word-wrap
    if (g_briefingText[0]) {
        RenderWrappedText(g_briefingText, boxX + 10, boxY + 10, boxW - 20, PAL_WHITE);
    }

    // Action buttons at bottom
    int btnY = 350;
    int btnW = 180;
    int btnH = 24;
    int centerX = 320;

    // "COMMENCE" button
    DrawBeveledButton(centerX - btnW/2, btnY, btnW, btnH, false, true, true);
    Renderer_DrawText("COMMENCE", centerX - 32, btnY + 8, PAL_GOLD, 0);

    // Instructions
    Renderer_DrawText("PRESS ENTER OR CLICK TO BEGIN MISSION", 150, 385, PAL_GREY, 0);

    // === DEBUG: Removed - calibration was successful ===
}

void Menu_UpdateBriefing(void) {
    // Check for confirmation input
    bool confirmed = false;

    // Enter or Space to confirm
    if (Input_WasKeyPressed(VK_RETURN) || Input_WasKeyPressed(VK_SPACE)) {
        confirmed = true;
    }

    // Mouse click on button
    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    uint8_t buttons = Input_GetMouseButtons();
    static bool wasLeftDown = false;
    bool leftDown = (buttons & INPUT_MOUSE_LEFT) != 0;
    bool leftClicked = !leftDown && wasLeftDown;
    wasLeftDown = leftDown;

    // Check if click in button area
    int btnY = 350;
    int btnW = 180;
    int btnH = 24;
    int btnX = 320 - btnW/2;

    if (leftClicked && mx >= btnX && mx < btnX + btnW &&
        my >= btnY && my < btnY + btnH) {
        confirmed = true;
    }

    // Also allow clicking anywhere to proceed (more user-friendly)
    if (leftClicked) {
        confirmed = true;
    }

    if (confirmed) {
        if (g_clickSound) Audio_Play(g_clickSound, 150, 0, FALSE);
        Menu_SetCurrentScreen(MENU_SCREEN_NONE);
        if (g_briefingConfirmCallback) {
            g_briefingConfirmCallback();
        }
    }

    // ESC goes back to campaign/difficulty selection
    if (Input_WasKeyPressed(VK_ESCAPE)) {
        if (g_clickSound) Audio_Play(g_clickSound, 100, 0, FALSE);
        // Go back to difficulty selection
        Menu_SetCurrentScreen(MENU_SCREEN_DIFFICULTY_SELECT);
    }
}

//===========================================================================
// Video Playback
//===========================================================================

#include "video/vqa.h"
#include "assets/assetloader.h"
#include <mutex>
// GetTickCount() is in compat/windows.h (already included via menu.h)

// Video playback state
static VQAPlayer* g_videoPlayer = nullptr;
static void* g_videoData = nullptr;
static VideoCompleteCallback g_videoCallback = nullptr;
static BOOL g_videoSkippable = TRUE;
static DWORD g_videoLastTime = 0;

// Video palette converted to renderer format
static Palette g_videoPalette;

// Video audio circular buffer (thread-safe)
static const int VIDEO_AUDIO_BUFFER_SIZE = 65536;  // ~1.5 sec at 22050 Hz
static int16_t g_videoAudioBuffer[VIDEO_AUDIO_BUFFER_SIZE];
static volatile int g_videoAudioWritePos = 0;
static volatile int g_videoAudioReadPos = 0;
static std::mutex g_videoAudioMutex;

// Track last sample for smooth transitions on underrun
static int16_t g_lastVideoSample = 0;

// Video audio callback for audio system
static int VideoAudioStreamCallback(int16_t* buffer, int sampleCount, void* userdata) {
    (void)userdata;
    std::lock_guard<std::mutex> lock(g_videoAudioMutex);

    int available = g_videoAudioWritePos - g_videoAudioReadPos;
    if (available < 0) available += VIDEO_AUDIO_BUFFER_SIZE;

    int toRead = (sampleCount < available) ? sampleCount : available;

    for (int i = 0; i < toRead; i++) {
        buffer[i] = g_videoAudioBuffer[(g_videoAudioReadPos + i) % VIDEO_AUDIO_BUFFER_SIZE];
    }
    g_videoAudioReadPos = (g_videoAudioReadPos + toRead) % VIDEO_AUDIO_BUFFER_SIZE;

    // Remember last sample for smooth transition
    if (toRead > 0) {
        g_lastVideoSample = buffer[toRead - 1];
    }

    // Fill remaining with last sample to avoid clicks (instead of jumping to 0)
    for (int i = toRead; i < sampleCount; i++) {
        buffer[i] = g_lastVideoSample;
    }

    return toRead;
}

// Add audio samples to the circular buffer
static void QueueVideoAudio(const int16_t* samples, int count) {
    std::lock_guard<std::mutex> lock(g_videoAudioMutex);

    for (int i = 0; i < count; i++) {
        int nextPos = (g_videoAudioWritePos + 1) % VIDEO_AUDIO_BUFFER_SIZE;
        if (nextPos == g_videoAudioReadPos) {
            // Buffer full - drop samples
            break;
        }
        g_videoAudioBuffer[g_videoAudioWritePos] = samples[i];
        g_videoAudioWritePos = nextPos;
    }
}

void Menu_PlayVideo(const char* name, VideoCompleteCallback onComplete, BOOL skippable) {
    // Clean up any previous video
    Menu_StopVideo();

    if (!name) return;

    // Load VQA data from assets
    uint32_t dataSize = 0;
    g_videoData = Assets_LoadVQA(name, &dataSize);
    if (!g_videoData) {
        printf("Video: Failed to load %s\n", name);
        // Call completion callback immediately if video not found
        if (onComplete) onComplete();
        return;
    }

    // Create player and load video
    g_videoPlayer = new VQAPlayer();
    if (!g_videoPlayer->Load(g_videoData, dataSize)) {
        printf("Video: Failed to parse %s\n", name);
        delete g_videoPlayer;
        g_videoPlayer = nullptr;
        free(g_videoData);
        g_videoData = nullptr;
        if (onComplete) onComplete();
        return;
    }

    printf("Video: Playing %s (%dx%d, %d frames, %d fps)\n",
           name, g_videoPlayer->GetWidth(), g_videoPlayer->GetHeight(),
           g_videoPlayer->GetFrameCount(), g_videoPlayer->GetFPS());

    g_videoCallback = onComplete;
    g_videoSkippable = skippable;
    g_videoLastTime = GetTickCount();

    // Set up video audio if available
    if (g_videoPlayer->HasAudio()) {
        // Reset audio buffer
        g_videoAudioWritePos = 0;
        g_videoAudioReadPos = 0;
        g_lastVideoSample = 0;

        // Register audio callback with sample rate
        int sampleRate = g_videoPlayer->GetAudioSampleRate();
        Audio_SetVideoCallback(VideoAudioStreamCallback, nullptr, sampleRate);

        printf("Video: Audio enabled (%d Hz, %d ch)\n",
               sampleRate, g_videoPlayer->GetAudioChannels());
    }

    // Start playback
    g_videoPlayer->Play();

    // Decode first frame
    g_videoPlayer->NextFrame();

    // Queue first frame's audio
    if (g_videoPlayer->HasAudio()) {
        static int16_t tempAudio[8192];
        int samples = g_videoPlayer->GetAudioSamples(tempAudio, 8192);
        if (samples > 0) {
            QueueVideoAudio(tempAudio, samples);
        }
    }

    // Set up initial palette
    if (g_videoPlayer->PaletteChanged()) {
        const uint8_t* vqaPal = g_videoPlayer->GetPalette();
        for (int i = 0; i < 256; i++) {
            g_videoPalette.colors[i][0] = vqaPal[i * 3 + 0];  // R
            g_videoPalette.colors[i][1] = vqaPal[i * 3 + 1];  // G
            g_videoPalette.colors[i][2] = vqaPal[i * 3 + 2];  // B
        }
    }

    // Switch to video screen
    Menu_SetCurrentScreen(MENU_SCREEN_VIDEO);
}

void Menu_UpdateVideo(void) {
    if (!g_videoPlayer) return;

    // Check for skip input
    if (g_videoSkippable) {
        // Any key or mouse click skips
        if (Input_WasKeyPressed(VK_ESCAPE) || Input_WasKeyPressed(VK_RETURN) ||
            Input_WasKeyPressed(VK_SPACE) || (Input_GetMouseButtons() & INPUT_MOUSE_LEFT)) {
            Menu_StopVideo();
            return;
        }
    }

    // Update video timing
    DWORD now = GetTickCount();
    int elapsed = (int)(now - g_videoLastTime);
    g_videoLastTime = now;

    // Advance video if needed
    if (g_videoPlayer->Update(elapsed)) {
        // New frame decoded - check for palette change
        if (g_videoPlayer->PaletteChanged()) {
            const uint8_t* vqaPal = g_videoPlayer->GetPalette();
            for (int i = 0; i < 256; i++) {
                g_videoPalette.colors[i][0] = vqaPal[i * 3 + 0];  // R
                g_videoPalette.colors[i][1] = vqaPal[i * 3 + 1];  // G
                g_videoPalette.colors[i][2] = vqaPal[i * 3 + 2];  // B
            }
        }

        // Queue audio for this frame
        if (g_videoPlayer->HasAudio()) {
            static int16_t tempAudio[8192];
            int samples = g_videoPlayer->GetAudioSamples(tempAudio, 8192);
            if (samples > 0) {
                QueueVideoAudio(tempAudio, samples);
            }
        }
    }

    // Check if video finished
    if (g_videoPlayer->GetState() == VQAState::FINISHED ||
        g_videoPlayer->GetState() == VQAState::ERROR) {
        Menu_StopVideo();
    }
}

void Menu_RenderVideo(void) {
    if (!g_videoPlayer) {
        Renderer_Clear(0);
        return;
    }

    // Set video palette
    Renderer_SetPalette(&g_videoPalette);

    // Get frame buffer
    const uint8_t* frameBuffer = g_videoPlayer->GetFrameBuffer();
    int vidWidth = g_videoPlayer->GetWidth();
    int vidHeight = g_videoPlayer->GetHeight();

    // Clear screen
    Renderer_Clear(0);

    // Center video on screen
    int screenW = Renderer_GetWidth();
    int screenH = Renderer_GetHeight();
    int destX = (screenW - vidWidth) / 2;
    int destY = (screenH - vidHeight) / 2;

    // Blit video frame to framebuffer
    if (frameBuffer) {
        Renderer_Blit(frameBuffer, vidWidth, vidHeight, destX, destY, FALSE);
    }

    // Show "Press any key to skip" if skippable
    if (g_videoSkippable) {
        Renderer_DrawText("Press any key to skip", 220, 385, 15, 0);
    }
}

BOOL Menu_IsVideoPlaying(void) {
    return g_videoPlayer != nullptr &&
           (g_videoPlayer->GetState() == VQAState::PLAYING ||
            g_videoPlayer->GetState() == VQAState::PAUSED);
}

void Menu_StopVideo(void) {
    VideoCompleteCallback callback = g_videoCallback;

    // Stop video audio
    Audio_SetVideoCallback(nullptr, nullptr, 0);

    // Clean up
    if (g_videoPlayer) {
        delete g_videoPlayer;
        g_videoPlayer = nullptr;
    }
    if (g_videoData) {
        free(g_videoData);
        g_videoData = nullptr;
    }
    g_videoCallback = nullptr;

    // Restore palette (video may have set its own palette)
    // Always use stub palette for menus - it has proper UI colors
    // (SNOW.PAL is for terrain, not menus)
    Palette restored;
    StubAssets_CreatePalette(&restored);
    Renderer_SetPalette(&restored);
    printf("Menu_StopVideo: Restored stub palette [15]=%d,%d,%d [122]=%d,%d,%d [223]=%d,%d,%d\n",
           restored.colors[15][0], restored.colors[15][1], restored.colors[15][2],
           restored.colors[122][0], restored.colors[122][1], restored.colors[122][2],
           restored.colors[223][0], restored.colors[223][1], restored.colors[223][2]);

    // Clear framebuffer to remove video residue
    Renderer_ResetClip();
    Renderer_Clear(PAL_BLACK);

    // Call completion callback
    if (callback) {
        callback();
    }
}
