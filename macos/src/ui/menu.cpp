/**
 * Red Alert macOS Port - Menu System Implementation
 *
 * Styled to match the original Westwood Red Alert menus.
 */

#include "menu.h"
#include "graphics/metal/renderer.h"
#include "input/input.h"
#include "audio/audio.h"
#include "game/gameloop.h"
#include <cstring>
#include <cstdlib>

// Westwood-style palette indices (approximate)
// These match the snow.pal colors
#define PAL_BLACK       0
#define PAL_DARKGREY    1
#define PAL_GREY        13
#define PAL_LTGREY      14
#define PAL_WHITE       15
#define PAL_RED         120   // Bright red
#define PAL_DARKRED     123   // Dark red
#define PAL_GOLD        127   // Gold/yellow
#define PAL_GREEN       159   // Green
#define PAL_DARKGREEN   161
#define PAL_BLUE        180   // Blue
#define PAL_DARKBLUE    183

// Button color scheme
#define BTN_FACE        13    // Button face color (grey)
#define BTN_HIGHLIGHT   15    // Top/left edge (white)
#define BTN_SHADOW      1     // Bottom/right edge (dark)
#define BTN_TEXT        15    // Normal text
#define BTN_TEXT_HOVER  127   // Hovered text (gold)
#define BTN_TEXT_DISABLED 7   // Disabled text
#define BTN_FACE_HOVER  14    // Lighter grey on hover
#define BTN_FACE_PRESSED 1    // Dark when pressed

// Global state
static MenuScreen g_currentScreen = MENU_SCREEN_NONE;
static Menu* g_mainMenu = nullptr;
static Menu* g_optionsMenu = nullptr;
static NewGameCallback g_newGameCallback = nullptr;

// Sound effects for menus
static AudioSample* g_clickSound = nullptr;
static AudioSample* g_hoverSound = nullptr;

// Animation frame counter
static int g_menuFrame = 0;

// Forward declarations for callbacks
static void OnMainMenuButton(int itemId, int value);
static void OnOptionsButton(int itemId, int value);

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
    TGL_FULLSCREEN
};

//===========================================================================
// Background Rendering
//===========================================================================

static void DrawMenuBackground(void) {
    // Dark gradient background - simulate with horizontal bands
    for (int y = 0; y < 400; y++) {
        // Gradient from very dark at top to slightly less dark at bottom
        uint8_t color;
        if (y < 80) {
            color = 0;  // Black at top
        } else if (y < 320) {
            color = 1;  // Dark grey for main area
        } else {
            color = 0;  // Black at bottom
        }
        Renderer_HLine(0, 639, y, color);
    }

    // Title banner area (top 80 pixels)
    // Red gradient banner
    for (int y = 20; y < 70; y++) {
        uint8_t intensity = (y < 45) ? 120 : 123;  // Red shades
        Renderer_HLine(100, 540, y, intensity);
    }

    // Banner border
    Renderer_HLine(100, 540, 19, PAL_DARKRED);
    Renderer_HLine(100, 540, 70, PAL_DARKRED);
    Renderer_VLine(100, 19, 70, PAL_DARKRED);
    Renderer_VLine(540, 19, 70, PAL_DARKRED);

    // Inner highlight
    Renderer_HLine(101, 539, 20, PAL_RED);
    Renderer_VLine(101, 20, 69, PAL_RED);
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

    // Draw styled background
    DrawMenuBackground();

    // Draw title text in banner
    // "COMMAND & CONQUER" in gold
    Renderer_DrawText("COMMAND & CONQUER", 220, 30, PAL_GOLD, 0);
    // "RED ALERT" larger below
    Renderer_DrawText("RED ALERT", 268, 48, PAL_WHITE, 0);

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
    Renderer_DrawText("MACOS PORT - M34", 260, 380, BTN_TEXT_DISABLED, 0);
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
            Menu_SetCurrentScreen(MENU_SCREEN_NONE);
            if (g_newGameCallback) {
                g_newGameCallback();
            }
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

static void OnOptionsButton(int itemId, int value) {
    switch (itemId) {
        case SLD_SOUND_VOL:
            Audio_SetMasterVolume((uint8_t)value);
            break;

        case SLD_MUSIC_VOL:
            // TODO: Set music volume
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
