/**
 * Red Alert macOS Port - Game-Specific Renderer Functions
 *
 * This file contains renderer functions that depend on game-specific
 * assets (like AssetLoader). These cannot be in ra-media.
 */

#include "renderer.h"
#include "compat/assets.h"

// Forward declaration for palette loading from assets
extern "C" {
    BOOL Assets_LoadPalette(const char* name, uint8_t* palette);
}

BOOL Renderer_LoadPalette(const char* name) {
    uint8_t rawPalette[768];

    if (!Assets_LoadPalette(name, rawPalette)) {
        return FALSE;
    }

    // Expand 6-bit VGA palette to 8-bit and set
    Palette palette;
    for (int i = 0; i < 256; i++) {
        uint8_t r = rawPalette[i * 3 + 0] & 0x3F;
        uint8_t g = rawPalette[i * 3 + 1] & 0x3F;
        uint8_t b = rawPalette[i * 3 + 2] & 0x3F;
        // Expand 6-bit to 8-bit: (value << 2) | (value >> 4)
        palette.colors[i][0] = (r << 2) | (r >> 4);
        palette.colors[i][1] = (g << 2) | (g >> 4);
        palette.colors[i][2] = (b << 2) | (b >> 4);
    }

    Renderer_SetPalette(&palette);
    return TRUE;
}
