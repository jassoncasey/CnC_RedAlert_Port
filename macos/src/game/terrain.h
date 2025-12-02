/**
 * Red Alert macOS Port - Terrain Tile Manager
 *
 * Manages loading and rendering of terrain tiles from game assets.
 */

#ifndef TERRAIN_H
#define TERRAIN_H

#include "compat/windows.h"
#include <cstdint>

// Initialize terrain system, load terrain tiles
BOOL Terrain_Init(void);

// Shutdown terrain system
void Terrain_Shutdown(void);

// Check if terrain tiles are available
BOOL Terrain_Available(void);

// Render a terrain tile at screen position
// terrainType: 0=clear, 1=water, 2=rock, etc.
// variant: which variant of the tile (for visual variety)
BOOL Terrain_RenderTile(int terrainType, int variant, int screenX, int screenY);

// Get tile size (24x24 for RA)
int Terrain_GetTileSize(void);

// Get number of loaded terrain sets
int Terrain_GetLoadedCount(void);

// Render terrain from MapPack data
// templateID: Template ID from MapPack data (e.g., 255=clear, 1=water)
// tileIndex: Tile index within template (from MapPack icon data)
BOOL Terrain_RenderByID(int templateID, int tileIndex,
                        int screenX, int screenY);

// Set the map theater (loads appropriate templates)
// theater: 0=temperate, 1=snow, 2=interior, 3=desert
void Terrain_SetTheater(int theater);

#endif // TERRAIN_H
