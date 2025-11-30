/**
 * Red Alert macOS Port - Asset Path Management
 *
 * Searches for game assets in multiple locations.
 * See ASSETS.md for documentation.
 */

#ifndef PLATFORM_ASSET_PATHS_H
#define PLATFORM_ASSET_PATHS_H

#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Find the asset directory.
 * Searches multiple paths in priority order.
 * @param outPath   Buffer to store the found path
 * @param outSize   Size of the buffer
 * @return true if assets found, false otherwise
 */
bool Assets_FindPath(char* outPath, size_t outSize);

/**
 * Get full path to a specific asset file.
 * @param filename  Asset filename (e.g., "REDALERT.MIX")
 * @param outPath   Buffer to store the full path
 * @param outSize   Size of the buffer
 * @return true if file exists, false otherwise
 */
bool Assets_GetFilePath(const char* filename, char* outPath, size_t outSize);

/**
 * Clear the cached asset path.
 * Call this if asset directories change at runtime.
 */
void Assets_ClearCache(void);

/**
 * Print all search paths and their status.
 * Useful for debugging.
 */
void Assets_PrintSearchPaths(void);

/**
 * Verify that all required assets are present.
 * Prints status of each file.
 * @return true if all required assets found
 */
bool Assets_VerifyInstallation(void);

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_ASSET_PATHS_H
