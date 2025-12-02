/**
 * Red Alert macOS Port - Map/Terrain System
 *
 * Handles the game map, terrain, and scrolling viewport.
 */

#ifndef GAME_MAP_H
#define GAME_MAP_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Map dimensions (in cells)
#define MAP_MAX_WIDTH   128
#define MAP_MAX_HEIGHT  128
#define CELL_SIZE       24      // Pixels per cell

// Terrain types
typedef enum {
    TERRAIN_CLEAR = 0,      // Passable ground
    TERRAIN_WATER,          // Impassable water
    TERRAIN_ROCK,           // Impassable rock/cliff
    TERRAIN_TREE,           // Passable but provides cover
    TERRAIN_ROAD,           // Fast movement
    TERRAIN_BRIDGE,         // Passable over water
    TERRAIN_BUILDING,       // Occupied by structure
    TERRAIN_ORE,            // Harvestable ore
    TERRAIN_GEM,            // Harvestable gems
    TERRAIN_COUNT
} TerrainType;

// Cell flags
#define CELL_FLAG_OCCUPIED      0x01    // Unit present
#define CELL_FLAG_REVEALED      0x02    // Fog of war revealed
#define CELL_FLAG_VISIBLE       0x04    // Currently visible

// Ore constants
#define ORE_MAX_AMOUNT      255     // Maximum ore per cell

// Map cell structure
typedef struct {
    uint8_t terrain;        // TerrainType
    uint8_t flags;          // Cell flags
    uint8_t height;         // Elevation (0-3)
    uint8_t oreAmount;      // Ore amount (0-255)
    int16_t unitId;         // Unit occupying cell (-1 if none)
    int16_t buildingId;     // Building on cell (-1 if none)
} MapCell;

// Viewport for scrolling
typedef struct {
    int32_t x;              // Top-left X in world pixels
    int32_t y;              // Top-left Y in world pixels
    int32_t width;          // Viewport width in pixels
    int32_t height;         // Viewport height in pixels
} Viewport;

/**
 * Initialize the map system
 */
void Map_Init(void);

/**
 * Shutdown the map system
 */
void Map_Shutdown(void);

/**
 * Create a new map
 * @param width   Map width in cells
 * @param height  Map height in cells
 */
void Map_Create(int width, int height);

/**
 * Generate a simple demo map
 */
void Map_GenerateDemo(void);

/**
 * Load map from mission terrain data
 * @param terrainType  Template type array (128*128 bytes, 0xFF=clear)
 * @param terrainIcon  Tile index array (128*128 bytes)
 * @param mapX         Map viewport X offset
 * @param mapY         Map viewport Y offset
 * @param mapWidth     Visible map width in cells
 * @param mapHeight    Visible map height in cells
 */
void Map_LoadFromMission(const uint8_t* terrainType, const uint8_t* terrainIcon,
                         int mapX, int mapY, int mapWidth, int mapHeight);

/**
 * Get map dimensions
 */
int Map_GetWidth(void);
int Map_GetHeight(void);

/**
 * Get cell at coordinates
 * @return Pointer to cell, or NULL if out of bounds
 */
MapCell* Map_GetCell(int cellX, int cellY);

/**
 * Set terrain at cell
 */
void Map_SetTerrain(int cellX, int cellY, TerrainType terrain);

/**
 * Check if cell is passable for ground units
 */
BOOL Map_IsPassable(int cellX, int cellY);

/**
 * Check if cell is passable for water units
 */
BOOL Map_IsWaterPassable(int cellX, int cellY);

/**
 * Convert world coordinates to cell coordinates
 */
void Map_WorldToCell(int worldX, int worldY, int* cellX, int* cellY);

/**
 * Convert cell coordinates to world coordinates (center of cell)
 */
void Map_CellToWorld(int cellX, int cellY, int* worldX, int* worldY);

/**
 * Get the viewport
 */
Viewport* Map_GetViewport(void);

/**
 * Set viewport position (clamped to map bounds)
 */
void Map_SetViewport(int x, int y);

/**
 * Scroll viewport by delta
 */
void Map_ScrollViewport(int dx, int dy);

/**
 * Center viewport on a world position
 */
void Map_CenterViewport(int worldX, int worldY);

/**
 * Check if a world position is visible in viewport
 */
BOOL Map_IsInViewport(int worldX, int worldY);

/**
 * Convert screen coordinates to world coordinates
 */
void Map_ScreenToWorld(int screenX, int screenY, int* worldX, int* worldY);

/**
 * Convert world coordinates to screen coordinates
 */
void Map_WorldToScreen(int worldX, int worldY, int* screenX, int* screenY);

/**
 * Render the map terrain
 */
void Map_Render(void);

/**
 * Update map state (animations, etc.)
 */
void Map_Update(void);

/**
 * Fog of War: Clear all visibility flags (call each frame before revealing)
 */
void Map_ClearVisibility(void);

/**
 * Fog of War: Reveal cells around a point
 * @param cellX      Center cell X
 * @param cellY      Center cell Y
 * @param sightRange Sight range in cells
 * @param team       Team doing the revealing (TEAM_PLAYER reveals for player)
 */
void Map_RevealAround(int cellX, int cellY, int sightRange, int team);

/**
 * Fog of War: Check if a cell is currently visible to player
 */
BOOL Map_IsCellVisible(int cellX, int cellY);

/**
 * Fog of War: Check if a cell has ever been revealed to player
 */
BOOL Map_IsCellRevealed(int cellX, int cellY);

/**
 * Fog of War: Enable or disable fog of war globally
 */
void Map_SetFogEnabled(BOOL enabled);

/**
 * Fog of War: Check if fog of war is enabled
 */
BOOL Map_IsFogEnabled(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_MAP_H
