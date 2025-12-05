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

//===========================================================================
// Voice System - Authentic unit responses using VocType
//===========================================================================

// Forward declarations (include voice_types.h for full definitions)
enum class VocType : int16_t;
enum class VoiceVariant : int8_t;
enum class ResponseType;

/**
 * Play a voice/sound effect by VocType.
 * Loads and caches the sound on first use.
 * @param voc      Sound effect type
 * @param variant  House variant (Allied/Soviet for .V/.R files)
 * @param volume   Volume (0-255)
 * @param pan      Pan position (-128 left to 127 right)
 */
void Voice_Play(VocType voc, VoiceVariant variant, uint8_t volume, int8_t pan);

/**
 * Play a voice at world position with auto-pan.
 * @param voc      Sound effect type
 * @param variant  House variant
 * @param worldX   World X position
 * @param worldY   World Y position
 * @param volume   Base volume (0-255)
 */
void Voice_PlayAt(VocType voc, VoiceVariant variant,
                  int worldX, int worldY, uint8_t volume);

/**
 * Play a unit response (select, move, attack).
 * Automatically selects appropriate voice for unit type.
 * @param unitType    Infantry or vehicle type ID
 * @param isInfantry  TRUE if infantry, FALSE if vehicle
 * @param response    Response type (SELECT, MOVE, ATTACK)
 * @param variant     House variant for voice selection
 * @param volume      Volume (0-255)
 * @param pan         Pan position
 */
void Voice_PlayResponse(int unitType, BOOL isInfantry, ResponseType response,
                        VoiceVariant variant, uint8_t volume, int8_t pan);

/**
 * Play a unit response at world position.
 */
void Voice_PlayResponseAt(int unitType, BOOL isInfantry, ResponseType response,
                          VoiceVariant variant,
                          int worldX, int worldY, uint8_t volume);

/**
 * Preload common voice responses for faster playback.
 * Call after Sounds_Init().
 */
void Voice_Preload(void);

/**
 * Get number of cached voice samples.
 */
int Voice_GetCachedCount(void);

#ifdef __cplusplus
}
#endif

#endif // GAME_SOUNDS_H
