/**
 * Red Alert macOS Port - Asset Loader
 *
 * Central asset loading facility for game sprites, sounds, and palettes.
 * Handles nested MIX archives and caches frequently used assets.
 */

#ifndef ASSETS_ASSETLOADER_H
#define ASSETS_ASSETLOADER_H

#include "compat/windows.h"
#include "assets/shpfile.h"
#include "assets/audfile.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the asset loader.
 * Opens the main game archives and prepares for loading.
 * @return TRUE on success, FALSE if required archives not found
 */
BOOL Assets_Init(void);

/**
 * Shutdown the asset loader.
 * Closes all archives and frees cached assets.
 */
void Assets_Shutdown(void);

/**
 * Load a SHP sprite file from game archives.
 * Searches in CONQUER.MIX, HIRES.MIX, etc.
 * @param name  Filename (e.g., "1TNK.SHP", "E1.SHP")
 * @return SHP handle, or NULL if not found. Caller must free with Shp_Free.
 */
ShpFileHandle Assets_LoadSHP(const char* name);

/**
 * Load an AUD sound file from game archives.
 * Searches in SOUNDS.MIX, etc.
 * @param name  Filename (e.g., "CANNON1.AUD", "CHRONO2.AUD")
 * @return AUD data, or NULL if not found. Caller must free with Aud_Free.
 */
AudData* Assets_LoadAUD(const char* name);

/**
 * Load a palette from game archives.
 * @param name  Filename (e.g., "TEMPERAT.PAL", "SNOW.PAL")
 * @param palette  Output 256-entry RGB palette (768 bytes, 6-bit values)
 * @return TRUE on success
 */
BOOL Assets_LoadPalette(const char* name, uint8_t* palette);

/**
 * Get the current game palette (expanded to 8-bit RGB).
 * @return Pointer to 256 RGB entries (768 bytes), or NULL if not loaded
 */
const uint8_t* Assets_GetPalette(void);

/**
 * Set the current game palette.
 * Expands 6-bit VGA values to 8-bit.
 * @param palette  256-entry RGB palette (768 bytes, 6-bit values)
 */
void Assets_SetPalette(const uint8_t* palette);

/**
 * Convert SHP frame to 32-bit RGBA using current palette.
 * @param frame     SHP frame data
 * @param output    Output buffer (width * height * 4 bytes)
 * @param transparent  Palette index to treat as transparent (usually 0)
 */
void Assets_SHPToRGBA(const ShpFrame* frame, uint32_t* output,
                      uint8_t transparent);

/**
 * Load raw file data from game archives.
 * @param name      Filename
 * @param outSize   Output size
 * @return Allocated data, or NULL if not found. Caller must free().
 */
void* Assets_LoadRaw(const char* name, uint32_t* outSize);

/**
 * Load VQA video data from MOVIES MIX archive.
 * @param name      Video filename (e.g., "PROLOG.VQA", "ALLY1.VQA")
 * @param outSize   Output size
 * @return Allocated data, or NULL if not found. Caller must free().
 */
void* Assets_LoadVQA(const char* name, uint32_t* outSize);

/**
 * Check if movies archive is available.
 * @return TRUE if MOVIES.MIX or MOVIES2.MIX is available
 */
BOOL Assets_HasMovies(void);

/**
 * Load music track data from SCORES.MIX archive.
 * @param name      Music filename (e.g., "HELL226M.AUD", "BIGF226M.AUD")
 * @param outSize   Output size
 * @return Allocated data, or NULL if not found. Caller must free().
 */
void* Assets_LoadMusic(const char* name, uint32_t* outSize);

/**
 * Check if music archive is available.
 * @return TRUE if SCORES.MIX is available
 */
BOOL Assets_HasMusic(void);

/**
 * Theater types for map rendering.
 */
typedef enum {
    THEATER_TEMPERATE = 0,
    THEATER_SNOW = 1,
    THEATER_INTERIOR = 2,
    THEATER_DESERT = 3
} TheaterType;

/**
 * Set the active theater for terrain loading.
 * Loads the appropriate palette and prioritizes the theater's MIX.
 * @param theater  Theater type (TEMPERATE, SNOW, INTERIOR, DESERT)
 * @return TRUE if theater assets are available
 */
BOOL Assets_SetTheater(TheaterType theater);

/**
 * Get the current active theater.
 * @return Current theater type
 */
TheaterType Assets_GetTheater(void);

/**
 * Load terrain template data from current theater's MIX.
 * @param name      Template filename (e.g., "CLEAR1.TEM", "SNOW01.TEM")
 * @param outSize   Output size
 * @return Allocated data, or NULL if not found. Caller must free().
 */
void* Assets_LoadTemplate(const char* name, uint32_t* outSize);

#ifdef __cplusplus
}
#endif

#endif // ASSETS_ASSETLOADER_H
