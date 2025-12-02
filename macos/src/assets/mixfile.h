/**
 * Red Alert macOS Port - MIX File Reader
 *
 * Reads Westwood MIX archive files.
 * MIX files contain multiple sub-files identified by CRC hash.
 */

#ifndef ASSETS_MIXFILE_H
#define ASSETS_MIXFILE_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Handle to an open MIX file
typedef struct MixFile* MixFileHandle;

/**
 * Open a MIX archive file
 * @param filename  Path to the .MIX file
 * @return Handle to the MIX file, or NULL on failure
 */
MixFileHandle Mix_Open(const char* filename);

/**
 * Open a MIX archive from memory
 * @param data      Pointer to MIX data in memory
 * @param size      Size of the data
 * @param ownsData  If TRUE, Mix_Close will free the data
 * @return Handle to the MIX file, or NULL on failure
 */
MixFileHandle Mix_OpenMemory(const void* data, uint32_t size, BOOL ownsData);

/**
 * Close a MIX archive
 */
void Mix_Close(MixFileHandle mix);

/**
 * Get the number of files in the archive
 */
int Mix_GetFileCount(MixFileHandle mix);

/**
 * Check if a file exists in the archive (by filename)
 * Uses CRC lookup.
 * @param name  Filename (case insensitive, e.g. "CONQUER.MIX")
 */
BOOL Mix_FileExists(MixFileHandle mix, const char* name);

/**
 * Get the size of a file in the archive
 * @return Size in bytes, or 0 if not found
 */
uint32_t Mix_GetFileSize(MixFileHandle mix, const char* name);

/**
 * Read a file from the archive
 * @param name    Filename to extract
 * @param buffer  Pre-allocated buffer to read into
 * @param bufSize Size of buffer
 * @return Bytes read, or 0 on failure
 */
uint32_t Mix_ReadFile(MixFileHandle mix, const char* name,
                      void* buffer, uint32_t bufSize);

/**
 * Allocate and read a file from the archive
 * Caller must free the returned pointer with free()
 * @param name      Filename to extract
 * @param outSize   [out] Size of the returned data
 * @return Pointer to allocated data, or NULL on failure
 */
void* Mix_AllocReadFile(MixFileHandle mix, const char* name, uint32_t* outSize);

/**
 * Calculate CRC hash for a filename
 * Westwood uses a custom CRC algorithm.
 */
uint32_t Mix_CalculateCRC(const char* name);

/**
 * Check if a file exists by CRC
 */
BOOL Mix_FileExistsByCRC(MixFileHandle mix, uint32_t crc);

/**
 * Read file by CRC
 */
uint32_t Mix_ReadFileByCRC(MixFileHandle mix, uint32_t crc,
                           void* buffer, uint32_t bufSize);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_MIXFILE_H
