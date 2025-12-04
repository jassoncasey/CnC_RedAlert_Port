/**
 * Westwood Format Compatibility Layer
 *
 * Provides C-style API wrapping libwestwood C++ classes.
 * This allows asset_viewer to use libwestwood without major refactoring.
 */

#ifndef WWD_COMPAT_H
#define WWD_COMPAT_H

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

//===========================================================================
// MIX File API
//===========================================================================

typedef struct MixFile* MixFileHandle;

// Open a MIX file from disk
MixFileHandle Mix_Open(const char* path);

// Open a MIX file from memory
// If ownsData is true, the memory will be freed when the handle is closed
MixFileHandle Mix_OpenMemory(const void* data, uint32_t size, int ownsData);

// Close a MIX file
void Mix_Close(MixFileHandle mix);

// Get number of files in the archive
int Mix_GetFileCount(MixFileHandle mix);

// Get entry info by index
int Mix_GetEntryByIndex(MixFileHandle mix, int index,
                        uint32_t* outCRC, uint32_t* outSize);

// Read file by CRC, allocating memory (caller must free)
void* Mix_AllocReadFileByCRC(MixFileHandle mix, uint32_t crc, uint32_t* outSize);

// Calculate CRC hash for a filename (Red Alert style)
uint32_t Mix_CalculateCRC(const char* filename);

// Resolve CRC to filename (if known)
const char* Mix_GetFilename(uint32_t crc);

//===========================================================================
// SHP File API
//===========================================================================

typedef struct ShpFile* ShpFileHandle;

// Frame info structure
typedef struct {
    int width;
    int height;
    int offsetX;
    int offsetY;
    const uint8_t* pixels;  // Decoded frame data
} ShpFrame;

// Load SHP from file
ShpFileHandle Shp_LoadFile(const char* path);

// Load SHP from memory
ShpFileHandle Shp_Load(const void* data, uint32_t size);

// Free SHP
void Shp_Free(ShpFileHandle shp);

// Get frame count
int Shp_GetFrameCount(ShpFileHandle shp);

// Get frame info (returns pointer to internal frame data)
const ShpFrame* Shp_GetFrame(ShpFileHandle shp, int frameIndex);

//===========================================================================
// AUD File API
//===========================================================================

typedef struct AudFile* AudFileHandle;

// Load AUD from file
AudFileHandle Aud_LoadFile(const char* path);

// Load AUD from memory
AudFileHandle Aud_Load(const void* data, uint32_t size);

// Free AUD
void Aud_Free(AudFileHandle aud);

// Get decoded PCM data (16-bit signed)
const int16_t* Aud_GetSamples(AudFileHandle aud);

// Get sample count
uint32_t Aud_GetSampleCount(AudFileHandle aud);

// Get sample rate
uint32_t Aud_GetSampleRate(AudFileHandle aud);

// Get channel count
int Aud_GetChannels(AudFileHandle aud);

//===========================================================================
// TMP File API (Terrain Templates)
//===========================================================================

typedef struct TmpFile* TmpFileHandle;

// Terrain tile info
typedef struct {
    uint8_t* pixels;    // width * height pixels (8-bit indexed)
    uint16_t width;
    uint16_t height;
} TmpTile;

// Load TMP from file
TmpFileHandle Tmp_LoadFile(const char* path);

// Load TMP from memory
TmpFileHandle Tmp_Load(const void* data, uint32_t size);

// Free TMP
void Tmp_Free(TmpFileHandle tmp);

// Get tile count
int Tmp_GetTileCount(TmpFileHandle tmp);

// Get tile by index (returns nullptr for empty tiles)
const TmpTile* Tmp_GetTile(TmpFileHandle tmp, int index);

// Get tile dimensions
uint16_t Tmp_GetTileWidth(TmpFileHandle tmp);
uint16_t Tmp_GetTileHeight(TmpFileHandle tmp);

//===========================================================================
// PAL File API
//===========================================================================

typedef struct PalFile* PalFileHandle;

// Load PAL from file
PalFileHandle Pal_LoadFile(const char* path);

// Load PAL from memory
PalFileHandle Pal_Load(const void* data, uint32_t size);

// Free PAL
void Pal_Free(PalFileHandle pal);

// Get palette colors (256 * 3 bytes, RGB, 8-bit values)
const uint8_t* Pal_GetColors(PalFileHandle pal);

#ifdef __cplusplus
}
#endif

#endif // WWD_COMPAT_H
