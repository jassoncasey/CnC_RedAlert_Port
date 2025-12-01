/**
 * Red Alert macOS Port - Sound Manager
 *
 * Loads and plays game sound effects from AUD files in MIX archives.
 */

#ifndef GAME_SOUNDS_H
#define GAME_SOUNDS_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Sound effect types
typedef enum {
    SFX_NONE = 0,
    // Combat sounds
    SFX_GUN_SHOT,       // Machine gun
    SFX_CANNON,         // Tank cannon
    SFX_ROCKET,         // Rocket launcher
    SFX_EXPLOSION_SM,   // Small explosion
    SFX_EXPLOSION_LG,   // Large explosion
    // Unit sounds
    SFX_UNIT_SELECT,    // Unit selected
    SFX_UNIT_MOVE,      // Unit ordered to move
    SFX_UNIT_ATTACK,    // Unit ordered to attack
    SFX_UNIT_DIE,       // Unit death
    // Building sounds
    SFX_BUILD_COMPLETE, // Building complete
    SFX_SELL,           // Structure sold
    SFX_POWER_DOWN,     // Low power
    // UI sounds
    SFX_CLICK,          // Button click
    SFX_RADAR_ON,       // Radar online
    SFX_MONEY,          // Credits received
    SFX_COUNT
} SoundEffect;

/**
 * Initialize sound system and load sounds from MIX archives.
 * Call after Assets_Init().
 * @return TRUE on success
 */
BOOL Sounds_Init(void);

/**
 * Shutdown sound system and free all samples.
 */
void Sounds_Shutdown(void);

/**
 * Check if sound system has loaded any sounds.
 */
BOOL Sounds_Available(void);

/**
 * Play a sound effect.
 * @param sfx     Sound effect to play
 * @param volume  Volume (0-255)
 * @param pan     Pan position (-128 left to 127 right, 0 center)
 */
void Sounds_Play(SoundEffect sfx, uint8_t volume, int8_t pan);

/**
 * Play a sound effect at world position (auto-pans based on camera).
 * @param sfx      Sound effect to play
 * @param worldX   World X position
 * @param worldY   World Y position
 * @param volume   Base volume (0-255)
 */
void Sounds_PlayAt(SoundEffect sfx, int worldX, int worldY, uint8_t volume);

/**
 * Get the number of loaded sounds.
 */
int Sounds_GetLoadedCount(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_SOUNDS_H
