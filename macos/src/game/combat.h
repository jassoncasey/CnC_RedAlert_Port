/**
 * Red Alert macOS Port - Combat System
 *
 * Damage calculation and application functions.
 * Based on original COMBAT.CPP
 */

#ifndef GAME_COMBAT_H
#define GAME_COMBAT_H

#include "types.h"
#include "weapon_types.h"
#include <cstdint>

// Forward declarations
class TechnoClass;
class ObjectClass;

//===========================================================================
// Combat Constants
//===========================================================================

// Minimum damage that can be dealt (prevents 0 damage from armor)
constexpr int MIN_DAMAGE = 1;

// Maximum damage cap
constexpr int MAX_DAMAGE = 1000;

// Distance for full damage (within this range, no falloff)
constexpr int FULL_DAMAGE_DISTANCE = 32;  // ~1/8 cell

// Spread factor for damage falloff calculation
constexpr int DEFAULT_SPREAD = 128;  // ~1/2 cell

//===========================================================================
// Core Combat Functions
//===========================================================================

/**
 * Calculate damage after applying warhead modifiers and distance falloff.
 *
 * @param damage Base damage value
 * @param warhead Warhead type (determines armor effectiveness)
 * @param armor Target's armor type
 * @param distance Distance from explosion center (leptons)
 * @return Modified damage value
 *
 * Logic:
 * 1. Apply warhead vs armor modifier (percentage)
 * 2. Apply distance falloff (further = less damage)
 * 3. Clamp to min/max range
 */
int Modify_Damage(int damage, WarheadType warhead,
                  ArmorType armor, int distance);

/**
 * Apply explosion damage to all objects in area.
 *
 * @param coord Center of explosion
 * @param damage Base damage value
 * @param source Who caused the explosion (for kill credit)
 * @param warhead Warhead type
 *
 * Collects all objects in center cell and adjacent 8 cells,
 * calculates distance to each, applies damage based on distance.
 */
void Explosion_Damage(int32_t coord, int damage,
                      TechnoClass* source, WarheadType warhead);

/**
 * Get warhead damage modifier vs armor type.
 *
 * @param warhead Warhead type
 * @param armor Armor type
 * @return Damage percentage (256 = 100%, 128 = 50%, etc.)
 */
int GetWarheadModifier(WarheadType warhead, ArmorType armor);

/**
 * Get spread factor for warhead (determines damage falloff radius).
 *
 * @param warhead Warhead type
 * @return Spread in leptons
 */
int GetWarheadSpread(WarheadType warhead);

/**
 * Check if warhead can destroy walls.
 */
bool CanDestroyWall(WarheadType warhead);

/**
 * Check if warhead can destroy wood structures.
 */
bool CanDestroyWood(WarheadType warhead);

/**
 * Check if warhead can destroy ore/tiberium.
 */
bool CanDestroyOre(WarheadType warhead);

//===========================================================================
// Combat Animation
//===========================================================================

/**
 * Get appropriate explosion animation for damage and warhead.
 *
 * @param damage Amount of damage dealt
 * @param warhead Warhead type
 * @return Animation type to play
 */
AnimType Combat_Anim(int damage, WarheadType warhead);

//===========================================================================
// Weapon Firing
//===========================================================================

/**
 * Fire a weapon from source to target.
 *
 * @param source Unit firing the weapon
 * @param weapon Weapon type to fire
 * @param targetCoord Target coordinate
 * @return true if weapon was fired successfully
 */
bool Fire_Weapon(TechnoClass* source, WeaponTypeEnum weapon,
                 int32_t targetCoord);

/**
 * Check if target is in range of weapon.
 *
 * @param source Firing unit
 * @param weapon Weapon type
 * @param targetCoord Target coordinate
 * @return true if target is in range
 */
bool In_Range(TechnoClass* source, WeaponTypeEnum weapon, int32_t targetCoord);

/**
 * Get weapon range in leptons.
 */
int GetWeaponRange(WeaponTypeEnum weapon);

/**
 * Get weapon rate of fire in ticks.
 */
int GetWeaponROF(WeaponTypeEnum weapon);

#endif // GAME_COMBAT_H
