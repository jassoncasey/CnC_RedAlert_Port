/**
 * Red Alert macOS Port - PAL Palette File Reader Implementation
 *
 * Uses libwestwood's PalReader for loading.
 * Provides additional utility functions for palette manipulation.
 */

#include "palfile.h"
#include <westwood/pal.h>
#include <cstdio>
#include <cstring>

BOOL Pal_Load(const void* data, uint32_t dataSize, Palette* pal) {
    if (!data || !pal) {
        return FALSE;
    }

    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), dataSize);
    auto result = wwd::PalReader::open(span);
    if (!result) {
        return FALSE;
    }

    auto& reader = *result;
    for (int i = 0; i < 256; i++) {
        auto c = reader->color_8bit(i);
        pal->colors[i][0] = c.r;
        pal->colors[i][1] = c.g;
        pal->colors[i][2] = c.b;
        // Index 0 is typically transparent
        pal->colors[i][3] = (i == 0) ? 0 : 255;
    }

    return TRUE;
}

BOOL Pal_LoadFile(const char* filename, Palette* pal) {
    if (!filename || !pal) {
        return FALSE;
    }

    auto result = wwd::PalReader::open(filename);
    if (!result) {
        return FALSE;
    }

    auto& reader = *result;
    for (int i = 0; i < 256; i++) {
        auto c = reader->color_8bit(i);
        pal->colors[i][0] = c.r;
        pal->colors[i][1] = c.g;
        pal->colors[i][2] = c.b;
        // Index 0 is typically transparent
        pal->colors[i][3] = (i == 0) ? 0 : 255;
    }

    return TRUE;
}

void Pal_InitGrayscale(Palette* pal) {
    if (!pal) return;

    for (int i = 0; i < 256; i++) {
        pal->colors[i][0] = (uint8_t)i;
        pal->colors[i][1] = (uint8_t)i;
        pal->colors[i][2] = (uint8_t)i;
        pal->colors[i][3] = (i == 0) ? 0 : 255;
    }
}

void Pal_InitDefault(Palette* pal) {
    if (!pal) return;

    // Initialize with a basic VGA-like palette
    // First 16 colors are standard CGA/EGA colors
    static const uint8_t basicColors[16][3] = {
        {   0,   0,   0 },  // 0: Black (transparent)
        {   0,   0, 170 },  // 1: Blue
        {   0, 170,   0 },  // 2: Green
        {   0, 170, 170 },  // 3: Cyan
        { 170,   0,   0 },  // 4: Red
        { 170,   0, 170 },  // 5: Magenta
        { 170,  85,   0 },  // 6: Brown
        { 170, 170, 170 },  // 7: Light Gray
        {  85,  85,  85 },  // 8: Dark Gray
        {  85,  85, 255 },  // 9: Light Blue
        {  85, 255,  85 },  // 10: Light Green
        {  85, 255, 255 },  // 11: Light Cyan
        { 255,  85,  85 },  // 12: Light Red
        { 255,  85, 255 },  // 13: Light Magenta
        { 255, 255,  85 },  // 14: Yellow
        { 255, 255, 255 },  // 15: White
    };

    for (int i = 0; i < 16; i++) {
        pal->colors[i][0] = basicColors[i][0];
        pal->colors[i][1] = basicColors[i][1];
        pal->colors[i][2] = basicColors[i][2];
        pal->colors[i][3] = (i == 0) ? 0 : 255;
    }

    // Fill remaining colors with grayscale ramp
    for (int i = 16; i < 256; i++) {
        uint8_t gray = (uint8_t)((i - 16) * 255 / 239);
        pal->colors[i][0] = gray;
        pal->colors[i][1] = gray;
        pal->colors[i][2] = gray;
        pal->colors[i][3] = 255;
    }
}

void Pal_Remap(const Palette* src, Palette* dst, const uint8_t* remap) {
    if (!src || !dst || !remap) return;

    for (int i = 0; i < 256; i++) {
        uint8_t newIndex = remap[i];
        dst->colors[i][0] = src->colors[newIndex][0];
        dst->colors[i][1] = src->colors[newIndex][1];
        dst->colors[i][2] = src->colors[newIndex][2];
        dst->colors[i][3] = src->colors[newIndex][3];
    }
}

void Pal_Blend(const Palette* pal1, const Palette* pal2,
               Palette* dst, uint8_t blend) {
    if (!pal1 || !pal2 || !dst) return;

    uint16_t b1 = 255 - blend;
    uint16_t b2 = blend;

    for (int i = 0; i < 256; i++) {
        uint8_t* d = dst->colors[i];
        const uint8_t* c1 = pal1->colors[i];
        const uint8_t* c2 = pal2->colors[i];
        d[0] = (uint8_t)((c1[0] * b1 + c2[0] * b2) / 255);
        d[1] = (uint8_t)((c1[1] * b1 + c2[1] * b2) / 255);
        d[2] = (uint8_t)((c1[2] * b1 + c2[2] * b2) / 255);
        d[3] = (uint8_t)((c1[3] * b1 + c2[3] * b2) / 255);
    }
}

void Pal_FadeToBlack(const Palette* src, Palette* dst, uint8_t fade) {
    if (!src || !dst) return;

    uint16_t intensity = 255 - fade;

    for (int i = 0; i < 256; i++) {
        dst->colors[i][0] = (uint8_t)(src->colors[i][0] * intensity / 255);
        dst->colors[i][1] = (uint8_t)(src->colors[i][1] * intensity / 255);
        dst->colors[i][2] = (uint8_t)(src->colors[i][2] * intensity / 255);
        dst->colors[i][3] = src->colors[i][3];  // Keep alpha unchanged
    }
}

void Pal_Copy(const Palette* src, Palette* dst) {
    if (!src || !dst) return;
    memcpy(dst, src, sizeof(Palette));
}
