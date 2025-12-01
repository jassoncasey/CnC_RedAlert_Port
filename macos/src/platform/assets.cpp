/**
 * Red Alert macOS Port - Stub Asset Implementation
 *
 * Generates placeholder assets for development without real game files.
 */

#include "compat/assets.h"
#include "compat/windows.h"

#include <cstdlib>
#include <cstring>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Asset path storage
static char g_assetPath[MAX_PATH] = "./assets";
static bool g_stubMode = true;

void StubAssets_Init(void) {
    // Check if real assets exist
    // For now, always stub mode until we implement MIX loading
    g_stubMode = true;
}

void StubAssets_Shutdown(void) {
    // Nothing to clean up currently
}

/**
 * Create a stub palette for the game
 * Uses standard Westwood-style palette layout matching menu color expectations:
 * - Index 0: Black (transparent)
 * - Index 1-15: Grayscale ramp (menu buttons use these)
 * - Index 112-127: Red ramp (menu banners)
 * - Index 168-175: Green ramp
 * - Index 176-191: Blue ramp
 * - Index 216-223: Yellow/gold ramp (menu text highlights)
 */
void StubAssets_CreatePalette(Palette* palette) {
    if (!palette) return;

    // Clear to black
    memset(palette->colors, 0, sizeof(palette->colors));

    // Grayscale ramp for indices 0-15 (used by menu buttons)
    // This matches menu.cpp's expectations: BTN_SHADOW=2, BTN_FACE=8, BTN_HIGHLIGHT=12, PAL_WHITE=15
    for (int i = 0; i <= 15; i++) {
        uint8_t gray = (uint8_t)((i * 255) / 15);
        palette->colors[i][0] = gray;
        palette->colors[i][1] = gray;
        palette->colors[i][2] = gray;
    }

    // Red ramp for indices 112-127 (used for menu banner gradients)
    // Menu uses indices ~115-127 for red gradient backgrounds
    for (int i = 112; i <= 127; i++) {
        int level = i - 112;  // 0-15
        uint8_t r = (uint8_t)(80 + (level * 175) / 15);  // 80-255
        uint8_t g = (uint8_t)((level * 40) / 15);        // 0-40
        uint8_t b = (uint8_t)((level * 40) / 15);        // 0-40
        palette->colors[i][0] = r;
        palette->colors[i][1] = g;
        palette->colors[i][2] = b;
    }

    // Green ramp for indices 168-175
    for (int i = 168; i <= 175; i++) {
        int level = i - 168;  // 0-7
        uint8_t g = (uint8_t)(100 + (level * 155) / 7);
        palette->colors[i][0] = (uint8_t)(level * 10);
        palette->colors[i][1] = g;
        palette->colors[i][2] = (uint8_t)(level * 10);
    }

    // Blue ramp for indices 176-191
    for (int i = 176; i <= 191; i++) {
        int level = i - 176;  // 0-15
        uint8_t b = (uint8_t)(80 + (level * 175) / 15);
        palette->colors[i][0] = (uint8_t)((level * 60) / 15);
        palette->colors[i][1] = (uint8_t)((level * 100) / 15);
        palette->colors[i][2] = b;
    }

    // Yellow/Gold ramp for indices 216-223 (used for PAL_GOLD=223, PAL_YELLOW=220)
    for (int i = 216; i <= 223; i++) {
        int level = i - 216;  // 0-7
        uint8_t r = (uint8_t)(180 + (level * 75) / 7);   // 180-255
        uint8_t g = (uint8_t)(140 + (level * 115) / 7);  // 140-255
        uint8_t b = (uint8_t)((level * 60) / 7);         // 0-60
        palette->colors[i][0] = r;
        palette->colors[i][1] = g;
        palette->colors[i][2] = b;
    }

    // Fill remaining indices with darker grayscale to avoid pure black
    // This prevents other palette indices from being invisible
    for (int i = 16; i < 112; i++) {
        uint8_t gray = (uint8_t)(20 + ((i - 16) * 60) / 96);
        palette->colors[i][0] = gray;
        palette->colors[i][1] = gray;
        palette->colors[i][2] = gray;
    }
    for (int i = 128; i < 168; i++) {
        uint8_t gray = (uint8_t)(40 + ((i - 128) * 60) / 40);
        palette->colors[i][0] = gray;
        palette->colors[i][1] = gray;
        palette->colors[i][2] = gray;
    }
    for (int i = 192; i < 216; i++) {
        uint8_t gray = (uint8_t)(60 + ((i - 192) * 60) / 24);
        palette->colors[i][0] = gray;
        palette->colors[i][1] = gray;
        palette->colors[i][2] = gray;
    }
    for (int i = 224; i < 256; i++) {
        uint8_t gray = (uint8_t)(80 + ((i - 224) * 80) / 32);
        palette->colors[i][0] = gray;
        palette->colors[i][1] = gray;
        palette->colors[i][2] = gray;
    }
}

/**
 * Create a simple colored rectangle sprite
 */
void StubAssets_CreateSprite(uint16_t width, uint16_t height, uint8_t colorIdx, uint8_t* data) {
    if (!data || width == 0 || height == 0) return;

    // Fill with solid color
    memset(data, colorIdx, width * height);

    // Add a 1-pixel border in a contrasting color
    uint8_t borderColor = (colorIdx < 128) ? 15 : 0;  // White or black border

    // Top and bottom edges
    for (uint16_t x = 0; x < width; x++) {
        data[x] = borderColor;                           // Top edge
        data[(height - 1) * width + x] = borderColor;    // Bottom edge
    }

    // Left and right edges
    for (uint16_t y = 0; y < height; y++) {
        data[y * width] = borderColor;                   // Left edge
        data[y * width + width - 1] = borderColor;       // Right edge
    }
}

/**
 * Create a sine wave tone
 */
uint8_t* StubAssets_CreateTone(AudioBuffer* buffer, uint16_t frequency,
                                uint16_t durationMs, uint16_t sampleRate) {
    if (!buffer || sampleRate == 0) return nullptr;

    // Calculate buffer size
    uint32_t numSamples = (uint32_t)sampleRate * durationMs / 1000;

    // Set up header (16-bit mono PCM)
    buffer->header.sampleRate = sampleRate;
    buffer->header.channels = 1;
    buffer->header.bitsPerSample = 16;
    buffer->header.dataSize = numSamples * 2;  // 2 bytes per sample

    // Allocate audio data
    int16_t* samples = (int16_t*)malloc(buffer->header.dataSize);
    if (!samples) return nullptr;

    // Generate sine wave
    double phase = 0.0;
    double phaseIncrement = 2.0 * M_PI * frequency / sampleRate;

    for (uint32_t i = 0; i < numSamples; i++) {
        // Generate sine wave with amplitude envelope (fade in/out)
        double envelope = 1.0;

        // Fade in first 10ms
        uint32_t fadeInSamples = sampleRate / 100;
        if (i < fadeInSamples) {
            envelope = (double)i / fadeInSamples;
        }

        // Fade out last 10ms
        uint32_t fadeOutStart = numSamples - fadeInSamples;
        if (i > fadeOutStart) {
            envelope = (double)(numSamples - i) / fadeInSamples;
        }

        // Generate sample (50% amplitude to avoid clipping)
        double sample = sin(phase) * 16384.0 * envelope;
        samples[i] = (int16_t)sample;

        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI) {
            phase -= 2.0 * M_PI;
        }
    }

    buffer->data = (uint8_t*)samples;
    return buffer->data;
}

/**
 * Create silence
 */
uint8_t* StubAssets_CreateSilence(AudioBuffer* buffer, uint16_t durationMs, uint16_t sampleRate) {
    if (!buffer || sampleRate == 0) return nullptr;

    // Calculate buffer size
    uint32_t numSamples = (uint32_t)sampleRate * durationMs / 1000;

    // Set up header (16-bit mono PCM)
    buffer->header.sampleRate = sampleRate;
    buffer->header.channels = 1;
    buffer->header.bitsPerSample = 16;
    buffer->header.dataSize = numSamples * 2;

    // Allocate and zero
    buffer->data = (uint8_t*)calloc(numSamples, 2);
    return buffer->data;
}

BOOL StubAssets_IsStubMode(void) {
    return g_stubMode ? TRUE : FALSE;
}

void StubAssets_SetPath(const char* path) {
    if (path) {
        strncpy(g_assetPath, path, MAX_PATH - 1);
        g_assetPath[MAX_PATH - 1] = '\0';
    }
}

const char* StubAssets_GetPath(void) {
    return g_assetPath;
}
