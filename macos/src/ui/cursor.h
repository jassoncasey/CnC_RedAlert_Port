/**
 * Red Alert macOS Port - Cursor System
 *
 * Handles mouse cursor rendering using MOUSE.SHP sprites.
 * Provides context-sensitive cursors (normal, attack, move, enter).
 */

#ifndef CURSOR_H
#define CURSOR_H

#include <cstdint>

// Cursor types matching original Red Alert
enum CursorType {
    CURSOR_NORMAL,   // Default arrow
    CURSOR_ATTACK,   // Crosshair for attacking enemies
    CURSOR_MOVE,     // 4-way arrow for movement
    CURSOR_ENTER     // For entering transports
};

// Initialize cursor system (loads MOUSE.SHP)
bool Cursor_Init(void);

// Shutdown cursor system
void Cursor_Shutdown(void);

// Advance cursor animation (call once per frame)
void Cursor_Update(void);

// Render cursor at current mouse position
void Cursor_Render(void);

// Get current cursor type based on mouse position and selection
CursorType Cursor_GetType(int screenX, int screenY);

#endif // CURSOR_H
