/**
 * Red Alert macOS Port - Sprite Manager
 *
 * Maps unit/building types to SHP sprites and handles rendering.
 */

#ifndef GAME_SPRITES_H
#define GAME_SPRITES_H

#include "compat/windows.h"
#include "units.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize sprite system and load sprites from MIX archives.
 * Call after Assets_Init().
 * @return TRUE on success
 */
BOOL Sprites_Init(void);

/**
 * Shutdown sprite system and free all sprites.
 */
void Sprites_Shutdown(void);

/**
 * Check if sprite system has loaded any sprites.
 */
BOOL Sprites_Available(void);

/**
 * Render a unit sprite at the given screen position.
 * Falls back to basic shapes if sprite not available.
 *
 * @param type      Unit type
 * @param facing    Direction (0-7)
 * @param frame     Animation frame (0 for idle)
 * @param screenX   Center X position on screen
 * @param screenY   Center Y position on screen
 * @param teamColor Team color for remapping
 * @return TRUE if sprite was rendered, FALSE if fallback was used
 */
BOOL Sprites_RenderUnit(UnitType type, int facing, int frame,
                        int screenX, int screenY, uint8_t teamColor);

/**
 * Render a building sprite at the given screen position.
 * Falls back to basic shapes if sprite not available.
 *
 * @param type      Building type
 * @param frame     Animation frame (0 for idle)
 * @param screenX   Top-left X position on screen
 * @param screenY   Top-left Y position on screen
 * @param teamColor Team color for remapping
 * @return TRUE if sprite was rendered, FALSE if fallback was used
 */
BOOL Sprites_RenderBuilding(BuildingType type, int frame,
                            int screenX, int screenY, uint8_t teamColor);

/**
 * Get the number of frames for a unit type sprite.
 */
int Sprites_GetUnitFrameCount(UnitType type);

/**
 * Get the number of frames for a building type sprite.
 */
int Sprites_GetBuildingFrameCount(BuildingType type);

/**
 * Get the color remap table for a team color.
 * Used to apply team colors to unit/building sprites.
 *
 * @param teamColor  Team color scheme index (from HouseTypeData colorScheme)
 * @return 256-byte remap table (never NULL)
 */
const uint8_t* Sprites_GetRemapTable(uint8_t teamColor);

#ifdef __cplusplus
}
#endif

#endif // GAME_SPRITES_H
