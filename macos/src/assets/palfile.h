/**
 * Red Alert macOS Port - PAL Palette File Reader
 *
 * PAL files contain 256-color palettes.
 * Each entry is 3 bytes (R, G, B), 6-bit values (0-63).
 */

#ifndef ASSETS_PALFILE_H
#define ASSETS_PALFILE_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Palette structure (256 colors, RGBA format for easy use)
typedef struct {
    uint8_t colors[256][4];  // [index][R,G,B,A]
} Palette;

/**
 * Load PAL file from memory buffer
 * @param data     Pointer to PAL file data (768 bytes)
 * @param dataSize Size of the data
 * @param pal      [out] Palette to fill
 * @return TRUE on success, FALSE on failure
 */
BOOL Pal_Load(const void* data, uint32_t dataSize, Palette* pal);

/**
 * Load PAL file from disk
 * @param filename Path to the .PAL file
 * @param pal      [out] Palette to fill
 * @return TRUE on success, FALSE on failure
 */
BOOL Pal_LoadFile(const char* filename, Palette* pal);

/**
 * Initialize a palette to grayscale
 */
void Pal_InitGrayscale(Palette* pal);

/**
 * Initialize a palette to default RA colors
 * This provides a reasonable fallback when no PAL is available
 */
void Pal_InitDefault(Palette* pal);

/**
 * Apply palette remapping
 * @param src  Source palette
 * @param dst  Destination palette
 * @param remap Remap table (256 bytes, dst_index = remap[src_index])
 */
void Pal_Remap(const Palette* src, Palette* dst, const uint8_t* remap);

/**
 * Blend two palettes
 * @param pal1  First palette
 * @param pal2  Second palette
 * @param dst   Destination palette
 * @param blend Blend factor (0 = all pal1, 255 = all pal2)
 */
void Pal_Blend(const Palette* pal1, const Palette* pal2,
               Palette* dst, uint8_t blend);

/**
 * Fade palette to black
 * @param src   Source palette
 * @param dst   Destination palette
 * @param fade  Fade level (0 = full color, 255 = black)
 */
void Pal_FadeToBlack(const Palette* src, Palette* dst, uint8_t fade);

/**
 * Copy palette
 */
void Pal_Copy(const Palette* src, Palette* dst);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_PALFILE_H
