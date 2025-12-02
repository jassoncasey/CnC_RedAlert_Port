/**
 * Red Alert macOS Port - AUD Audio File Reader
 *
 * AUD files contain compressed audio using IMA ADPCM or
 * Westwood's custom compression.
 */

#ifndef ASSETS_AUDFILE_H
#define ASSETS_AUDFILE_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Decoded audio data
typedef struct {
    int16_t* samples;       // 16-bit signed PCM samples
    uint32_t sampleCount;   // Number of samples
    uint32_t sampleRate;    // Sample rate (Hz)
    uint8_t channels;       // 1 = mono, 2 = stereo
} AudData;

/**
 * Load and decode AUD file from memory buffer
 * @param data     Pointer to AUD file data
 * @param dataSize Size of the data
 * @return Decoded audio data, or NULL on failure. Free with Aud_Free.
 */
AudData* Aud_Load(const void* data, uint32_t dataSize);

/**
 * Load and decode AUD file from disk
 * @param filename Path to the .AUD file
 * @return Decoded audio data, or NULL on failure. Free with Aud_Free.
 */
AudData* Aud_LoadFile(const char* filename);

/**
 * Free decoded audio data
 */
void Aud_Free(AudData* aud);

/**
 * Convert AUD data to AudioSample format for playback
 * @param aud      Decoded AUD data
 * @param outData  [out] Pointer to 8-bit audio data (caller must free)
 * @param outSize  [out] Size of output data
 * @param outRate  [out] Sample rate
 * @return TRUE on success
 */
BOOL Aud_ConvertTo8Bit(const AudData* aud, uint8_t** outData,
                       uint32_t* outSize, uint32_t* outRate);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_AUDFILE_H
