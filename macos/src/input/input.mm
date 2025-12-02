/**
 * Red Alert macOS Port - Input Implementation
 *
 * Handles keyboard and mouse input using macOS key codes.
 */

#import <Carbon/Carbon.h>  // For kVK_* key codes
#include "input.h"
#include <cstring>

// Input state
static struct {
    // Keyboard state
    uint8_t keyState[256];          // Current key state (indexed by VK code)
    uint8_t keyPressed[256];        // Keys pressed this frame
    uint8_t keyReleased[256];       // Keys released this frame

    // Key event buffer (circular)
    KeyEvent keyBuffer[INPUT_KEY_BUFFER_SIZE];
    int keyBufferHead;
    int keyBufferTail;

    // Mouse state
    int mouseX;
    int mouseY;
    int lastMouseX;
    int lastMouseY;
    uint8_t mouseButtons;

    bool initialized;
} g_input = {};

// macOS keycode to Windows VK_* mapping table
static uint16_t macToVK[128] = {0};

static void InitKeyMapping(void) {
    // Initialize all to 0
    memset(macToVK, 0, sizeof(macToVK));

    // Letters (A-Z)
    macToVK[kVK_ANSI_A] = 'A';
    macToVK[kVK_ANSI_B] = 'B';
    macToVK[kVK_ANSI_C] = 'C';
    macToVK[kVK_ANSI_D] = 'D';
    macToVK[kVK_ANSI_E] = 'E';
    macToVK[kVK_ANSI_F] = 'F';
    macToVK[kVK_ANSI_G] = 'G';
    macToVK[kVK_ANSI_H] = 'H';
    macToVK[kVK_ANSI_I] = 'I';
    macToVK[kVK_ANSI_J] = 'J';
    macToVK[kVK_ANSI_K] = 'K';
    macToVK[kVK_ANSI_L] = 'L';
    macToVK[kVK_ANSI_M] = 'M';
    macToVK[kVK_ANSI_N] = 'N';
    macToVK[kVK_ANSI_O] = 'O';
    macToVK[kVK_ANSI_P] = 'P';
    macToVK[kVK_ANSI_Q] = 'Q';
    macToVK[kVK_ANSI_R] = 'R';
    macToVK[kVK_ANSI_S] = 'S';
    macToVK[kVK_ANSI_T] = 'T';
    macToVK[kVK_ANSI_U] = 'U';
    macToVK[kVK_ANSI_V] = 'V';
    macToVK[kVK_ANSI_W] = 'W';
    macToVK[kVK_ANSI_X] = 'X';
    macToVK[kVK_ANSI_Y] = 'Y';
    macToVK[kVK_ANSI_Z] = 'Z';

    // Numbers (0-9)
    macToVK[kVK_ANSI_0] = '0';
    macToVK[kVK_ANSI_1] = '1';
    macToVK[kVK_ANSI_2] = '2';
    macToVK[kVK_ANSI_3] = '3';
    macToVK[kVK_ANSI_4] = '4';
    macToVK[kVK_ANSI_5] = '5';
    macToVK[kVK_ANSI_6] = '6';
    macToVK[kVK_ANSI_7] = '7';
    macToVK[kVK_ANSI_8] = '8';
    macToVK[kVK_ANSI_9] = '9';

    // Function keys
    macToVK[kVK_F1]  = VK_F1;
    macToVK[kVK_F2]  = VK_F2;
    macToVK[kVK_F3]  = VK_F3;
    macToVK[kVK_F4]  = VK_F4;
    macToVK[kVK_F5]  = VK_F5;
    macToVK[kVK_F6]  = VK_F6;
    macToVK[kVK_F7]  = VK_F7;
    macToVK[kVK_F8]  = VK_F8;
    macToVK[kVK_F9]  = VK_F9;
    macToVK[kVK_F10] = VK_F10;
    macToVK[kVK_F11] = VK_F11;
    macToVK[kVK_F12] = VK_F12;

    // Special keys
    macToVK[kVK_Return]    = VK_RETURN;
    macToVK[kVK_Tab]       = VK_TAB;
    macToVK[kVK_Space]     = VK_SPACE;
    macToVK[kVK_Delete]    = VK_BACK;      // Backspace on Mac
    macToVK[kVK_ForwardDelete] = VK_DELETE;
    macToVK[kVK_Escape]    = VK_ESCAPE;

    // Modifier keys
    macToVK[kVK_Shift]     = VK_SHIFT;
    macToVK[kVK_RightShift] = VK_RSHIFT;
    macToVK[kVK_Control]   = VK_CONTROL;
    macToVK[kVK_RightControl] = VK_RCONTROL;
    macToVK[kVK_Option]    = VK_MENU;      // Alt
    macToVK[kVK_RightOption] = VK_RMENU;
    // Map Cmd to Ctrl for game compatibility
    macToVK[kVK_Command]   = VK_LCONTROL;
    macToVK[kVK_CapsLock]  = VK_CAPITAL;

    // Arrow keys
    macToVK[kVK_LeftArrow]  = VK_LEFT;
    macToVK[kVK_RightArrow] = VK_RIGHT;
    macToVK[kVK_UpArrow]    = VK_UP;
    macToVK[kVK_DownArrow]  = VK_DOWN;

    // Navigation keys
    macToVK[kVK_Home]      = VK_HOME;
    macToVK[kVK_End]       = VK_END;
    macToVK[kVK_PageUp]    = VK_PRIOR;
    macToVK[kVK_PageDown]  = VK_NEXT;

    // Numpad
    macToVK[kVK_ANSI_Keypad0] = VK_NUMPAD0;
    macToVK[kVK_ANSI_Keypad1] = VK_NUMPAD1;
    macToVK[kVK_ANSI_Keypad2] = VK_NUMPAD2;
    macToVK[kVK_ANSI_Keypad3] = VK_NUMPAD3;
    macToVK[kVK_ANSI_Keypad4] = VK_NUMPAD4;
    macToVK[kVK_ANSI_Keypad5] = VK_NUMPAD5;
    macToVK[kVK_ANSI_Keypad6] = VK_NUMPAD6;
    macToVK[kVK_ANSI_Keypad7] = VK_NUMPAD7;
    macToVK[kVK_ANSI_Keypad8] = VK_NUMPAD8;
    macToVK[kVK_ANSI_Keypad9] = VK_NUMPAD9;
    macToVK[kVK_ANSI_KeypadPlus]    = VK_ADD;
    macToVK[kVK_ANSI_KeypadMinus]   = VK_SUBTRACT;
    macToVK[kVK_ANSI_KeypadMultiply] = VK_MULTIPLY;
    macToVK[kVK_ANSI_KeypadDivide]  = VK_DIVIDE;
    macToVK[kVK_ANSI_KeypadDecimal] = VK_DECIMAL;
    macToVK[kVK_ANSI_KeypadEnter]   = VK_RETURN;
}

void Input_Init(void) {
    if (g_input.initialized) return;

    memset(&g_input, 0, sizeof(g_input));
    InitKeyMapping();

    g_input.initialized = true;
}

void Input_Shutdown(void) {
    g_input.initialized = false;
}

void Input_Update(void) {
    // Clear per-frame state
    memset(g_input.keyPressed, 0, sizeof(g_input.keyPressed));
    memset(g_input.keyReleased, 0, sizeof(g_input.keyReleased));

    // Update mouse delta
    g_input.lastMouseX = g_input.mouseX;
    g_input.lastMouseY = g_input.mouseY;
}

BOOL Input_IsKeyDown(uint16_t vkCode) {
    if (vkCode >= 256) return FALSE;
    return g_input.keyState[vkCode] ? TRUE : FALSE;
}

BOOL Input_WasKeyPressed(uint16_t vkCode) {
    if (vkCode >= 256) return FALSE;
    return g_input.keyPressed[vkCode] ? TRUE : FALSE;
}

BOOL Input_WasKeyReleased(uint16_t vkCode) {
    if (vkCode >= 256) return FALSE;
    return g_input.keyReleased[vkCode] ? TRUE : FALSE;
}

BOOL Input_GetKeyEvent(KeyEvent* event) {
    if (!event || g_input.keyBufferHead == g_input.keyBufferTail) {
        return FALSE;
    }

    *event = g_input.keyBuffer[g_input.keyBufferTail];
    g_input.keyBufferTail = (g_input.keyBufferTail + 1) % INPUT_KEY_BUFFER_SIZE;
    return TRUE;
}

BOOL Input_HasKeyEvents(void) {
    return g_input.keyBufferHead != g_input.keyBufferTail;
}

void Input_ClearKeyboard(void) {
    memset(g_input.keyState, 0, sizeof(g_input.keyState));
    memset(g_input.keyPressed, 0, sizeof(g_input.keyPressed));
    memset(g_input.keyReleased, 0, sizeof(g_input.keyReleased));
    g_input.keyBufferHead = 0;
    g_input.keyBufferTail = 0;
}

int Input_GetMouseX(void) {
    return g_input.mouseX;
}

int Input_GetMouseY(void) {
    return g_input.mouseY;
}

uint8_t Input_GetMouseButtons(void) {
    return g_input.mouseButtons;
}

BOOL Input_IsMouseButtonDown(uint8_t button) {
    return (g_input.mouseButtons & button) ? TRUE : FALSE;
}

void Input_GetMouseDelta(int* dx, int* dy) {
    if (dx) *dx = g_input.mouseX - g_input.lastMouseX;
    if (dy) *dy = g_input.mouseY - g_input.lastMouseY;
}

uint16_t Input_MacKeyToVK(uint16_t macKeyCode) {
    if (macKeyCode < 128) {
        return macToVK[macKeyCode];
    }
    return 0;
}

// Convert NSEvent modifier flags to our format
// Bits: 17=Shift, 18=Control, 19=Option, 20=Command
static uint8_t ModifiersFromNS(uint16_t nsModifiers) {
    uint8_t mods = 0;
    if (nsModifiers & (1 << 17)) mods |= INPUT_MOD_SHIFT;
    if (nsModifiers & (1 << 18)) mods |= INPUT_MOD_CTRL;
    if (nsModifiers & (1 << 19)) mods |= INPUT_MOD_ALT;
    if (nsModifiers & (1 << 20)) mods |= INPUT_MOD_CMD;
    return mods;
}

void Input_HandleKeyDown(uint16_t keyCode, uint16_t modifiers) {
    uint16_t vk = Input_MacKeyToVK(keyCode);
    if (vk == 0) return;

    // Update key state
    if (vk < 256) {
        if (!g_input.keyState[vk]) {
            g_input.keyPressed[vk] = 1;
        }
        g_input.keyState[vk] = 1;
    }

    // Add to event buffer
    int nextHead = (g_input.keyBufferHead + 1) % INPUT_KEY_BUFFER_SIZE;
    if (nextHead != g_input.keyBufferTail) {  // Buffer not full
        KeyEvent* ev = &g_input.keyBuffer[g_input.keyBufferHead];
        ev->vkCode = vk;
        ev->scanCode = keyCode;
        ev->pressed = 1;
        ev->modifiers = ModifiersFromNS(modifiers);
        g_input.keyBufferHead = nextHead;
    }
}

void Input_HandleKeyUp(uint16_t keyCode, uint16_t modifiers) {
    uint16_t vk = Input_MacKeyToVK(keyCode);
    if (vk == 0) return;

    // Update key state
    if (vk < 256) {
        if (g_input.keyState[vk]) {
            g_input.keyReleased[vk] = 1;
        }
        g_input.keyState[vk] = 0;
    }

    // Add to event buffer
    int nextHead = (g_input.keyBufferHead + 1) % INPUT_KEY_BUFFER_SIZE;
    if (nextHead != g_input.keyBufferTail) {
        KeyEvent* ev = &g_input.keyBuffer[g_input.keyBufferHead];
        ev->vkCode = vk;
        ev->scanCode = keyCode;
        ev->pressed = 0;
        ev->modifiers = ModifiersFromNS(modifiers);
        g_input.keyBufferHead = nextHead;
    }
}

void Input_HandleMouseMove(int x, int y) {
    g_input.mouseX = x;
    g_input.mouseY = y;
}

void Input_HandleMouseButton(uint8_t button, BOOL pressed) {
    if (pressed) {
        g_input.mouseButtons |= button;
    } else {
        g_input.mouseButtons &= ~button;
    }
}
