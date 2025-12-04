/**
 * Red Alert Asset Categorization
 *
 * Provides categorization of game assets based on filename patterns
 * derived from the original game and OpenRA analysis.
 */

#ifndef RA_CATEGORY_H
#define RA_CATEGORY_H

#include <string_view>

namespace ra {

/**
 * Asset categories matching the original game's type hierarchy.
 *
 * Based on CnC_Remastered_Collection ObjectTypeClass hierarchy and
 * OpenRA's Encyclopedia categories.
 */
enum class AssetCategory {
    // === UNITS ===
    Infantry,       // E1-E7, SPY, THF, MEDI, DOG, civilians
    Vehicle,        // Tanks, APCs, harvesters, artillery
    Aircraft,       // Helicopters, planes
    Vessel,         // Ships, submarines

    // === STRUCTURES ===
    Building,       // Production, tech, resource buildings
    Defense,        // Turrets, walls, mines

    // === TERRAIN ===
    Terrain,        // Theater tiles (temperate, snow, interior)
    Overlay,        // Ore, gems, crates
    Smudge,         // Craters, scorch marks, bibs

    // === EFFECTS ===
    Animation,      // Explosions, fire, smoke
    Projectile,     // Bullets, missiles, bombs

    // === UI ===
    Cameo,          // Sidebar build icons
    Cursor,         // Mouse cursors
    Interface,      // Menus, fonts, logos, palettes

    // === AUDIO ===
    Music,          // Background music tracks
    SoundEffect,    // Weapon sounds, explosions
    Voice,          // EVA speech, unit responses

    // === VIDEO ===
    Cutscene,       // FMV videos

    // === DATA ===
    Rules,          // INI configuration files

    // === FALLBACK ===
    Unknown         // Unrecognized assets
};

/**
 * Theater types for terrain-specific categorization.
 */
enum class Theater {
    Temperate,      // Green terrain (TEMPERAT.MIX)
    Snow,           // Winter terrain (SNOW.MIX)
    Interior,       // Indoor/dungeon (INTERIOR.MIX)
    Unknown
};

/**
 * Categorize an asset by its filename.
 *
 * @param filename  Asset filename (e.g., "E1.SHP", "POWR.SHP")
 * @param theater   Optional theater context for terrain tiles
 * @return          The asset's category
 */
AssetCategory categorize(std::string_view filename,
                         Theater theater = Theater::Unknown);

/**
 * Get human-readable name for a category.
 */
const char* category_name(AssetCategory cat);

/**
 * Get human-readable name for a theater.
 */
const char* theater_name(Theater theater);

/**
 * Detect theater from a MIX filename.
 *
 * @param mix_name  MIX file name (e.g., "SNOW.MIX", "TEMPERAT.MIX")
 * @return          The detected theater, or Theater::Unknown
 */
Theater detect_theater(std::string_view mix_name);

} // namespace ra

#endif // RA_CATEGORY_H
