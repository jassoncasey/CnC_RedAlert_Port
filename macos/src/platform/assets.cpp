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
 * Create a development palette with:
 * - Index 0: Black (transparent)
 * - Index 1-15: Basic colors
 * - Index 16-255: Grayscale gradient
 */
void StubAssets_CreatePalette(Palette* palette) {
    if (!palette) return;

    // Clear to black
    memset(palette->colors, 0, sizeof(palette->colors));

    // Standard VGA-like colors for first 16 entries
    // 0: Black (transparent)
    palette->colors[0][0] = 0;   palette->colors[0][1] = 0;   palette->colors[0][2] = 0;
    // 1: Dark Blue
    palette->colors[1][0] = 0;   palette->colors[1][1] = 0;   palette->colors[1][2] = 170;
    // 2: Dark Green
    palette->colors[2][0] = 0;   palette->colors[2][1] = 170; palette->colors[2][2] = 0;
    // 3: Dark Cyan
    palette->colors[3][0] = 0;   palette->colors[3][1] = 170; palette->colors[3][2] = 170;
    // 4: Dark Red
    palette->colors[4][0] = 170; palette->colors[4][1] = 0;   palette->colors[4][2] = 0;
    // 5: Dark Magenta
    palette->colors[5][0] = 170; palette->colors[5][1] = 0;   palette->colors[5][2] = 170;
    // 6: Brown/Orange
    palette->colors[6][0] = 170; palette->colors[6][1] = 85;  palette->colors[6][2] = 0;
    // 7: Light Gray
    palette->colors[7][0] = 170; palette->colors[7][1] = 170; palette->colors[7][2] = 170;
    // 8: Dark Gray
    palette->colors[8][0] = 85;  palette->colors[8][1] = 85;  palette->colors[8][2] = 85;
    // 9: Bright Blue
    palette->colors[9][0] = 85;  palette->colors[9][1] = 85;  palette->colors[9][2] = 255;
    // 10: Bright Green
    palette->colors[10][0] = 85;  palette->colors[10][1] = 255; palette->colors[10][2] = 85;
    // 11: Bright Cyan
    palette->colors[11][0] = 85;  palette->colors[11][1] = 255; palette->colors[11][2] = 255;
    // 12: Bright Red
    palette->colors[12][0] = 255; palette->colors[12][1] = 85;  palette->colors[12][2] = 85;
    // 13: Bright Magenta
    palette->colors[13][0] = 255; palette->colors[13][1] = 85;  palette->colors[13][2] = 255;
    // 14: Yellow
    palette->colors[14][0] = 255; palette->colors[14][1] = 255; palette->colors[14][2] = 85;
    // 15: White
    palette->colors[15][0] = 255; palette->colors[15][1] = 255; palette->colors[15][2] = 255;

    // Fill remaining with grayscale gradient (16-255)
    for (int i = 16; i < 256; i++) {
        // Map 16-255 to 0-255 grayscale
        uint8_t gray = (uint8_t)(((i - 16) * 255) / 239);
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
