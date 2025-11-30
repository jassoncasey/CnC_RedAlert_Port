/**
 * Red Alert macOS Port - SHP Sprite File Reader
 *
 * SHP files contain multiple frames of sprites/shapes.
 * Each frame can be compressed or uncompressed.
 */

#ifndef ASSETS_SHPFILE_H
#define ASSETS_SHPFILE_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Handle to a loaded SHP file
typedef struct ShpFile* ShpFileHandle;

// Individual frame data
typedef struct {
    uint8_t* pixels;        // 8-bit indexed pixel data
    uint16_t width;
    uint16_t height;
    int16_t offsetX;        // Hotspot offset X
    int16_t offsetY;        // Hotspot offset Y
} ShpFrame;

/**
 * Load SHP file from memory buffer
 * @param data     Pointer to SHP file data
 * @param dataSize Size of the data
 * @return Handle to the SHP, or NULL on failure
 */
ShpFileHandle Shp_Load(const void* data, uint32_t dataSize);

/**
 * Load SHP file from disk
 * @param filename Path to the .SHP file
 * @return Handle to the SHP, or NULL on failure
 */
ShpFileHandle Shp_LoadFile(const char* filename);

/**
 * Free a loaded SHP file
 */
void Shp_Free(ShpFileHandle shp);

/**
 * Get the number of frames in the SHP
 */
int Shp_GetFrameCount(ShpFileHandle shp);

/**
 * Get a specific frame from the SHP
 * @param index  Frame index (0-based)
 * @return Pointer to frame data, or NULL if invalid
 */
const ShpFrame* Shp_GetFrame(ShpFileHandle shp, int index);

/**
 * Get the maximum width across all frames
 */
uint16_t Shp_GetMaxWidth(ShpFileHandle shp);

/**
 * Get the maximum height across all frames
 */
uint16_t Shp_GetMaxHeight(ShpFileHandle shp);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_SHPFILE_H
