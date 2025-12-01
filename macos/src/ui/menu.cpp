/**
 * Red Alert macOS Port - Menu System Implementation
 */

#include "menu.h"
#include "graphics/metal/renderer.h"
#include "input/input.h"
#include "audio/audio.h"
#include "game/gameloop.h"
#include <cstring>
#include <cstdlib>

// Global state
static MenuScreen g_currentScreen = MENU_SCREEN_NONE;
static Menu* g_mainMenu = nullptr;
static Menu* g_optionsMenu = nullptr;
static NewGameCallback g_newGameCallback = nullptr;

// Sound effects for menus
static AudioSample* g_clickSound = nullptr;
static AudioSample* g_hoverSound = nullptr;

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

void Menu_Init(void) {
    // Create click sound (short high beep)
    g_clickSound = Audio_CreateTestTone(880, 50);
    g_hoverSound = Audio_CreateTestTone(440, 30);

    // Create main menu
    // Westwood palette: BLACK=12, WHITE=15, LTGREY=14, GREY=13, RED=8, LTGREEN=4
    g_mainMenu = Menu_Create("RED ALERT");
    Menu_SetColors(g_mainMenu, 12, 15, 8, 13);  // Black bg, white text, red highlight, grey disabled

    int centerX = 320;
    int btnWidth = 200;
    int btnHeight = 30;
    int startY = 100;
    int spacing = 40;

    Menu_AddLabel(g_mainMenu, 0, "COMMAND & CONQUER", centerX - 100, 40);
    Menu_AddLabel(g_mainMenu, 0, "RED ALERT", centerX - 50, 60);

    Menu_AddButton(g_mainMenu, BTN_NEW_GAME, "NEW GAME", centerX - btnWidth/2, startY, btnWidth, btnHeight, OnMainMenuButton);
    Menu_AddButton(g_mainMenu, BTN_LOAD_GAME, "LOAD GAME", centerX - btnWidth/2, startY + spacing, btnWidth, btnHeight, OnMainMenuButton);
    Menu_SetItemEnabled(g_mainMenu, BTN_LOAD_GAME, FALSE); // Not implemented
    Menu_AddButton(g_mainMenu, BTN_MULTIPLAYER, "MULTIPLAYER", centerX - btnWidth/2, startY + spacing*2, btnWidth, btnHeight, OnMainMenuButton);
    Menu_SetItemEnabled(g_mainMenu, BTN_MULTIPLAYER, FALSE); // Not implemented

    Menu_AddButton(g_mainMenu, BTN_OPTIONS, "OPTIONS", centerX - btnWidth/2, startY + spacing*3, btnWidth, btnHeight, OnMainMenuButton);
    Menu_AddButton(g_mainMenu, BTN_CREDITS, "CREDITS", centerX - btnWidth/2, startY + spacing*4, btnWidth, btnHeight, OnMainMenuButton);
    Menu_AddSeparator(g_mainMenu, startY + spacing*5);
    Menu_AddButton(g_mainMenu, BTN_EXIT, "EXIT", centerX - btnWidth/2, startY + spacing*5 + 20, btnWidth, btnHeight, OnMainMenuButton);

    Menu_AddLabel(g_mainMenu, 0, "MACOS PORT - MILESTONE 11", centerX - 100, 360);

    // Create options menu
    g_optionsMenu = Menu_Create("OPTIONS");
    Menu_SetColors(g_optionsMenu, 12, 15, 4, 13);  // Black bg, white text, green highlight, grey disabled

    Menu_AddLabel(g_optionsMenu, 0, "OPTIONS", centerX - 40, 50);

    Menu_AddSlider(g_optionsMenu, SLD_SOUND_VOL, "SOUND VOLUME", centerX - 150, 120, 200, 0, 255, 255, OnOptionsButton);
    Menu_AddSlider(g_optionsMenu, SLD_MUSIC_VOL, "MUSIC VOLUME", centerX - 150, 170, 200, 0, 255, 200, OnOptionsButton);
    Menu_AddToggle(g_optionsMenu, TGL_FULLSCREEN, "FULLSCREEN", centerX - 100, 220, FALSE, OnOptionsButton);

    Menu_AddSeparator(g_optionsMenu, 280);
    Menu_AddButton(g_optionsMenu, BTN_BACK, "BACK", centerX - btnWidth/2, 300, btnWidth, btnHeight, OnOptionsButton);
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
    menu->textColor = 15;
    menu->highlightColor = 14;
    menu->disabledColor = 7;
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

    // Draw background
    Renderer_Clear(menu->bgColor);

    // Draw items
    for (int i = 0; i < menu->itemCount; i++) {
        MenuItem* item = &menu->items[i];
        if (!item->visible) continue;

        uint8_t textColor = menu->textColor;
        if (!item->enabled) textColor = menu->disabledColor;
        else if (item->state == MENU_STATE_HOVER || i == menu->selectedIndex) textColor = menu->highlightColor;

        switch (item->type) {
            case MENU_ITEM_BUTTON: {
                // Draw button background using Westwood palette
                // BLUE=11, RED=8, LTGREEN=4, GREY=13, BLACK=12
                uint8_t bgColor = 11; // Blue for normal
                if (item->state == MENU_STATE_PRESSED) bgColor = 8; // Red when pressed
                else if (item->state == MENU_STATE_HOVER || i == menu->selectedIndex) bgColor = 4; // Green on hover

                if (!item->enabled) bgColor = 13; // Grey for disabled

                Renderer_FillRect(item->x, item->y, item->width, item->height, bgColor);
                Renderer_DrawRect(item->x, item->y, item->width, item->height, textColor);

                // Center text
                int textLen = (int)strlen(item->text) * 8;
                int textX = item->x + (item->width - textLen) / 2;
                int textY = item->y + (item->height - 8) / 2;
                Renderer_DrawText(item->text, textX, textY, textColor, 0);
                break;
            }

            case MENU_ITEM_LABEL:
                Renderer_DrawText(item->text, item->x, item->y, textColor, 0);
                break;

            case MENU_ITEM_SEPARATOR:
                Renderer_HLine(50, 590, item->y, menu->disabledColor);
                break;

            case MENU_ITEM_SLIDER: {
                // Label
                Renderer_DrawText(item->text, item->x, item->y, textColor, 0);

                // Slider track
                int trackY = item->y + 15;
                Renderer_FillRect(item->x, trackY, item->width, 10, 1);
                Renderer_DrawRect(item->x, trackY, item->width, 10, menu->disabledColor);

                // Slider fill
                int fillWidth = (item->value - item->minValue) * item->width / (item->maxValue - item->minValue);
                Renderer_FillRect(item->x, trackY, fillWidth, 10, 10);

                // Value text
                char valText[16];
                snprintf(valText, sizeof(valText), "%d", item->value);
                Renderer_DrawText(valText, item->x + item->width + 10, trackY, textColor, 0);
                break;
            }

            case MENU_ITEM_TOGGLE: {
                // Checkbox
                int boxSize = 16;
                Renderer_DrawRect(item->x, item->y + 4, boxSize, boxSize, textColor);
                if (item->value) {
                    Renderer_FillRect(item->x + 3, item->y + 7, boxSize - 6, boxSize - 6, 10);
                }

                // Label
                Renderer_DrawText(item->text, item->x + boxSize + 10, item->y + 8, textColor, 0);
                break;
            }
        }
    }
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
            hitH = 30; // Include label
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
                int trackY = item->y + 15;
                if (mouseY >= trackY && mouseY < trackY + 10) {
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
