/**
 * Red Alert macOS Port - Input System
 *
 * Keyboard and mouse input handling.
 */

#ifndef INPUT_INPUT_H
#define INPUT_INPUT_H

#include "compat/windows.h"
#include <cstdint>

// Keyboard buffer size
#define INPUT_KEY_BUFFER_SIZE 256

// Mouse button states
#define INPUT_MOUSE_LEFT    0x01
#define INPUT_MOUSE_RIGHT   0x02
#define INPUT_MOUSE_MIDDLE  0x04

// Key event structure
struct KeyEvent {
    uint16_t vkCode;        // Virtual key code (VK_*)
    uint16_t scanCode;      // Hardware scan code
    uint8_t  pressed;       // 1 = pressed, 0 = released
    uint8_t  modifiers;     // Modifier key state
};

// Modifier flags
#define INPUT_MOD_SHIFT     0x01
#define INPUT_MOD_CTRL      0x02
#define INPUT_MOD_ALT       0x04
#define INPUT_MOD_CMD       0x08  // macOS Command key

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize input system
 */
void Input_Init(void);

/**
 * Shutdown input system
 */
void Input_Shutdown(void);

/**
 * Process pending input events
 * Call once per frame from main loop.
 */
void Input_Update(void);

/**
 * Check if a key is currently pressed
 *
 * @param vkCode Virtual key code (VK_*)
 * @return TRUE if pressed
 */
BOOL Input_IsKeyDown(uint16_t vkCode);

/**
 * Check if a key was just pressed this frame
 */
BOOL Input_WasKeyPressed(uint16_t vkCode);

/**
 * Check if a key was just released this frame
 */
BOOL Input_WasKeyReleased(uint16_t vkCode);

/**
 * Get next key event from buffer
 *
 * @param event Output event structure
 * @return TRUE if event available
 */
BOOL Input_GetKeyEvent(KeyEvent* event);

/**
 * Check if any key events are pending
 */
BOOL Input_HasKeyEvents(void);

/**
 * Clear all keyboard state and events
 */
void Input_ClearKeyboard(void);

/**
 * Get current mouse X position (window coordinates)
 */
int Input_GetMouseX(void);

/**
 * Get current mouse Y position (window coordinates)
 */
int Input_GetMouseY(void);

/**
 * Get mouse button state
 *
 * @return Bit flags: INPUT_MOUSE_LEFT, INPUT_MOUSE_RIGHT, INPUT_MOUSE_MIDDLE
 */
uint8_t Input_GetMouseButtons(void);

/**
 * Check if a mouse button is pressed
 */
BOOL Input_IsMouseButtonDown(uint8_t button);

/**
 * Get mouse movement delta since last update
 */
void Input_GetMouseDelta(int* dx, int* dy);

/**
 * Convert macOS keycode to Windows virtual key code
 */
uint16_t Input_MacKeyToVK(uint16_t macKeyCode);

// Internal functions called from Objective-C event handlers
void Input_HandleKeyDown(uint16_t keyCode, uint16_t modifiers);
void Input_HandleKeyUp(uint16_t keyCode, uint16_t modifiers);
void Input_HandleMouseMove(int x, int y);
void Input_HandleMouseButton(uint8_t button, BOOL pressed);

#ifdef __cplusplus
}
#endif

#endif // INPUT_INPUT_H
