/**
 * wwd-media - Common Types
 *
 * Platform-independent type definitions for the wwd-media library.
 * No external dependencies.
 */

#ifndef WWD_TYPES_H
#define WWD_TYPES_H

#include <cstdint>
#include <cstddef>

// Boolean type (avoids dependency on platform headers)
#ifndef WWD_BOOL_DEFINED
#define WWD_BOOL_DEFINED
typedef int WwdBool;
#define WWD_TRUE  1
#define WWD_FALSE 0
#endif

// Palette (256 RGB entries, 6-bit or 8-bit values)
struct WwdPalette {
    uint8_t colors[256][3];  // RGB values
};

// Audio sample format
struct WwdAudioSample {
    uint8_t* data;          // Raw PCM data (signed 16-bit or unsigned 8-bit)
    uint32_t dataSize;      // Size in bytes
    uint32_t sampleRate;    // Samples per second (typically 22050)
    uint8_t  channels;      // 1 = mono, 2 = stereo
    uint8_t  bitsPerSample; // 8 or 16
};

// Sound handle (0 = invalid)
typedef uint32_t WwdSoundHandle;

// Framebuffer dimensions (640x400 common in Westwood games)
#define WWD_FRAMEBUFFER_WIDTH  640
#define WWD_FRAMEBUFFER_HEIGHT 400

// Maximum simultaneous sounds
#define WWD_AUDIO_MAX_CHANNELS 16

#endif // WWD_TYPES_H
