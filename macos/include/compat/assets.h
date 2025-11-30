/**
 * Red Alert macOS Port - Asset Types and Stub Interface
 *
 * Defines asset types and provides stub asset generation for development.
 */

#ifndef COMPAT_ASSETS_H
#define COMPAT_ASSETS_H

#include "platform.h"
#include <cstdint>

// Asset type identifiers
enum AssetType {
    ASSET_PALETTE,      // 256-color palette
    ASSET_SPRITE,       // SHP sprite
    ASSET_SOUND,        // AUD audio
    ASSET_MAP,          // Map data
    ASSET_STRINGS,      // String table
    ASSET_UNKNOWN
};

// Palette (256 RGB entries)
struct Palette {
    uint8_t colors[256][3];  // RGB values
};

// Sprite header (simplified from SHP format)
struct SpriteHeader {
    uint16_t width;
    uint16_t height;
    uint16_t frameCount;
    uint16_t flags;
};

// Sprite frame
struct SpriteFrame {
    uint16_t width;
    uint16_t height;
    int16_t offsetX;
    int16_t offsetY;
    uint8_t* data;       // Indexed color data (palette indices)
};

// Audio header (simplified from AUD format)
struct AudioHeader {
    uint16_t sampleRate;
    uint8_t  channels;
    uint8_t  bitsPerSample;
    uint32_t dataSize;
};

// Audio buffer
struct AudioBuffer {
    AudioHeader header;
    uint8_t* data;       // PCM audio data
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize stub asset system
 */
void StubAssets_Init(void);

/**
 * Shutdown stub asset system
 */
void StubAssets_Shutdown(void);

/**
 * Generate a stub palette
 * Creates a grayscale palette with basic colors in first 16 entries.
 */
void StubAssets_CreatePalette(Palette* palette);

/**
 * Generate a stub sprite
 * Creates a colored rectangle.
 *
 * @param width    Sprite width
 * @param height   Sprite height
 * @param colorIdx Palette index for fill color
 * @param data     Output buffer (must be width*height bytes)
 */
void StubAssets_CreateSprite(uint16_t width, uint16_t height, uint8_t colorIdx, uint8_t* data);

/**
 * Generate a stub audio buffer
 * Creates a simple sine wave tone.
 *
 * @param buffer       Output audio buffer (header will be filled)
 * @param frequency    Tone frequency in Hz
 * @param durationMs   Duration in milliseconds
 * @param sampleRate   Sample rate (e.g., 22050)
 * @return Pointer to allocated audio data (caller must free)
 */
uint8_t* StubAssets_CreateTone(AudioBuffer* buffer, uint16_t frequency,
                                uint16_t durationMs, uint16_t sampleRate);

/**
 * Generate silence
 *
 * @param buffer       Output audio buffer
 * @param durationMs   Duration in milliseconds
 * @param sampleRate   Sample rate
 * @return Pointer to allocated audio data (caller must free)
 */
uint8_t* StubAssets_CreateSilence(AudioBuffer* buffer, uint16_t durationMs, uint16_t sampleRate);

/**
 * Check if running with stub assets
 * Returns TRUE if real game assets are not available.
 */
BOOL StubAssets_IsStubMode(void);

/**
 * Set the asset search path
 */
void StubAssets_SetPath(const char* path);

/**
 * Get asset search path
 */
const char* StubAssets_GetPath(void);

#ifdef __cplusplus
}
#endif

#endif // COMPAT_ASSETS_H
