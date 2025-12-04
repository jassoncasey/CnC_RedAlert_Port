/**
 * Red Alert macOS Port - Cursor System Implementation
 *
 * Loads MOUSE.SHP and renders context-sensitive cursors.
 */

#include "cursor.h"
#include "game/units.h"
#include "game/map.h"
#include "input/input.h"
#include "assets/shpfile.h"
#include "assets/assetloader.h"
#include "graphics/metal/renderer.h"

// Cursor frame definitions from original Red Alert (MOUSE.SHP)
struct CursorDef {
    int startFrame;
    int frameCount;
    int hotspotX;
    int hotspotY;
};

static const CursorDef g_cursorDefs[] = {
    { 0,  1, 0, 0 },    // CURSOR_NORMAL (arrow)
    { 21, 8, 14, 14 },  // CURSOR_ATTACK (crosshair, animated)
    { 10, 4, 14, 14 },  // CURSOR_MOVE (4-way arrow, animated)
    { 15, 6, 14, 14 },  // CURSOR_ENTER (select, animated)
};

// Mouse cursor sprite handle
static ShpFileHandle g_mouseCursor = nullptr;
static int g_cursorAnimFrame = 0;

// Helper: find enemy building at a cell position
static Building* GetEnemyBuildingAtCell(int cellX, int cellY) {
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        Building* bld = Buildings_Get(i);
        if (!bld || !bld->active) continue;
        if (bld->team != TEAM_ENEMY) continue;
        if (cellX >= bld->cellX && cellX < bld->cellX + bld->width &&
            cellY >= bld->cellY && cellY < bld->cellY + bld->height) {
            return bld;
        }
    }
    return nullptr;
}

bool Cursor_Init(void) {
    g_mouseCursor = Assets_LoadSHP("MOUSE.SHP");
    g_cursorAnimFrame = 0;
    return g_mouseCursor != nullptr;
}

void Cursor_Shutdown(void) {
    if (g_mouseCursor) {
        Shp_Free(g_mouseCursor);
        g_mouseCursor = nullptr;
    }
}

void Cursor_Update(void) {
    g_cursorAnimFrame++;
}

CursorType Cursor_GetType(int mx, int my) {
    // No units selected = normal cursor
    if (Units_GetSelectedCount() == 0) return CURSOR_NORMAL;

    // Get first selected unit for comparison
    int selId = Units_GetFirstSelected();
    Unit* sel = Units_Get(selId);
    if (!sel) return CURSOR_NORMAL;

    // Check for units under cursor
    int targetId = Units_GetAtScreen(mx, my);
    Unit* target = Units_Get(targetId);

    if (target) {
        // Enemy unit: attack cursor (if we can attack)
        if (target->team == TEAM_ENEMY) {
            if (sel->attackDamage > 0) {
                return CURSOR_ATTACK;
            }
        }
        // Friendly transport: enter cursor
        else if (target->team == sel->team &&
                 Units_IsTransport((UnitType)target->type) &&
                 Units_IsLoadable((UnitType)sel->type)) {
            return CURSOR_ENTER;
        }
    }

    // Check for enemy buildings under cursor
    if (sel->attackDamage > 0) {
        int worldX, worldY;
        Map_ScreenToWorld(mx, my, &worldX, &worldY);
        int cellX = worldX / CELL_SIZE;
        int cellY = worldY / CELL_SIZE;
        Building* enemyBld = GetEnemyBuildingAtCell(cellX, cellY);
        if (enemyBld) {
            return CURSOR_ATTACK;
        }
    }

    return CURSOR_MOVE;  // Default move cursor when units selected
}

void Cursor_Render(void) {
    int mx = Input_GetMouseX();
    int my = Input_GetMouseY();
    CursorType ctype = Cursor_GetType(mx, my);

    // Try to render sprite cursor if MOUSE.SHP is loaded
    if (g_mouseCursor && ctype <= CURSOR_ENTER) {
        const CursorDef& def = g_cursorDefs[ctype];
        int frameCount = Shp_GetFrameCount(g_mouseCursor);

        // Calculate animation frame (cycle through frames)
        int animOffset = 0;
        if (def.frameCount > 1) {
            animOffset = (g_cursorAnimFrame / 8) % def.frameCount;
        }
        int frameIndex = def.startFrame + animOffset;

        if (frameIndex >= frameCount) {
            frameIndex = def.startFrame;
        }

        const ShpFrame* frame = Shp_GetFrame(g_mouseCursor, frameIndex);
        if (frame && frame->pixels) {
            int drawX = mx - def.hotspotX;
            int drawY = my - def.hotspotY;
            Renderer_Blit(frame->pixels, frame->width, frame->height,
                          drawX, drawY, TRUE);
            return;
        }
    }

    // Fallback: draw primitive shapes if sprite not available
    uint8_t color = 15;
    switch (ctype) {
        case CURSOR_ATTACK:
            color = 4;  // Red
            Renderer_DrawLine(mx - 6, my - 6, mx + 6, my + 6, color);
            Renderer_DrawLine(mx - 6, my + 6, mx + 6, my - 6, color);
            Renderer_DrawLine(mx - 8, my, mx + 8, my, color);
            Renderer_DrawLine(mx, my - 8, mx, my + 8, color);
            return;
        case CURSOR_ENTER:
            color = 10;  // Green
            Renderer_DrawRect(mx - 6, my - 6, 12, 12, color);
            Renderer_DrawLine(mx - 3, my, mx + 3, my, color);
            Renderer_DrawLine(mx, my - 3, mx, my + 3, color);
            return;
        case CURSOR_MOVE:
            color = 14;  // Yellow
            break;
        default:
            color = 15;
            break;
    }
    Renderer_DrawLine(mx - 8, my, mx + 8, my, color);
    Renderer_DrawLine(mx, my - 8, mx, my + 8, color);
}
