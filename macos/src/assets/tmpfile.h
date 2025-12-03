/**
 * Red Alert macOS Port - TMP Terrain Template File Reader
 *
 * TMP files contain terrain tiles (24x24 pixels each in RA).
 * Format (Red Alert):
 *   Header (32 bytes):
 *     uint16_t width       - tile width (24)
 *     uint16_t height      - tile height (24)
 *     uint16_t tileCount   - number of tiles
 *     uint16_t reserved[5]
 *     uint32_t imgStart    - offset to image data
 *     uint32_t reserved2[2]
 *     uint32_t indexEnd    - end of index table
 *     uint32_t reserved3
 *     uint32_t indexStart  - start of index table
 *
 *   Index table (at indexStart):
 *     uint8_t[count] - tile indices (255 = empty tile)
 *
 *   Image data (at imgStart):
 *     Uncompressed 8-bit indexed pixels, width*height per tile
 */

#ifndef TMPFILE_H
#define TMPFILE_H

#include "compat/windows.h"
#include <cstdint>

// Terrain tile frame
typedef struct {
    uint8_t* pixels;    // width * height pixels (8-bit indexed)
    uint16_t width;
    uint16_t height;
} TmpTile;

// Opaque handle to loaded TMP file
struct TmpFile;
typedef struct TmpFile* TmpFileHandle;

// Load TMP file from memory buffer
TmpFileHandle Tmp_Load(const void* data, uint32_t dataSize);

// Load TMP file from disk
TmpFileHandle Tmp_LoadFile(const char* filename);

// Free TMP file
void Tmp_Free(TmpFileHandle tmp);

// Get tile count
int Tmp_GetTileCount(TmpFileHandle tmp);

// Get tile by index (returns nullptr for empty tiles)
const TmpTile* Tmp_GetTile(TmpFileHandle tmp, int index);

// Get tile dimensions
uint16_t Tmp_GetTileWidth(TmpFileHandle tmp);
uint16_t Tmp_GetTileHeight(TmpFileHandle tmp);

#endif // TMPFILE_H
