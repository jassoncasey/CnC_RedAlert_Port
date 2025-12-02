/**
 * Red Alert macOS Port - Audio System
 *
 * CoreAudio-based sound effect playback.
 * Supports mixing multiple simultaneous sounds.
 */

#ifndef AUDIO_AUDIO_H
#define AUDIO_AUDIO_H

#include "compat/windows.h"
#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum simultaneous sounds
#define AUDIO_MAX_CHANNELS 16

// Sound handle (0 = invalid)
typedef uint32_t SoundHandle;

// Audio sample format (matching Red Alert's AUD files)
typedef struct {
    uint8_t* data;          // Raw PCM data (signed 16-bit or unsigned 8-bit)
    uint32_t dataSize;      // Size in bytes
    uint32_t sampleRate;    // Samples per second (typically 22050)
    uint8_t  channels;      // 1 = mono, 2 = stereo
    uint8_t  bitsPerSample; // 8 or 16
} AudioSample;

/**
 * Initialize the audio system
 * @return TRUE on success
 */
BOOL Audio_Init(void);

/**
 * Shutdown the audio system
 */
void Audio_Shutdown(void);

/**
 * Update audio system (call each frame)
 * Handles sound completion callbacks, etc.
 */
void Audio_Update(void);

/**
 * Play a sound effect
 * @param sample   The audio sample to play
 * @param volume   Volume level (0-255, 255 = full)
 * @param pan      Pan position (-128 = left, 0 = center, 127 = right)
 * @param loop     If TRUE, loop the sound
 * @return Handle to the playing sound, or 0 on failure
 */
SoundHandle Audio_Play(const AudioSample* sample, uint8_t volume,
                       int8_t pan, BOOL loop);

/**
 * Stop a playing sound
 */
void Audio_Stop(SoundHandle handle);

/**
 * Stop all sounds
 */
void Audio_StopAll(void);

/**
 * Check if a sound is still playing
 */
BOOL Audio_IsPlaying(SoundHandle handle);

/**
 * Set volume of a playing sound
 */
void Audio_SetVolume(SoundHandle handle, uint8_t volume);

/**
 * Set pan of a playing sound
 */
void Audio_SetPan(SoundHandle handle, int8_t pan);

/**
 * Set master volume (affects all sounds)
 * @param volume  0-255
 */
void Audio_SetMasterVolume(uint8_t volume);

/**
 * Get master volume
 */
uint8_t Audio_GetMasterVolume(void);

/**
 * Set sound effects volume (only affects sound channels, not music)
 * @param volume  0-255
 */
void Audio_SetSoundVolume(uint8_t volume);

/**
 * Get sound effects volume
 */
uint8_t Audio_GetSoundVolume(void);

/**
 * Pause/unpause all audio
 */
void Audio_Pause(BOOL pause);

/**
 * Check if audio is paused
 */
BOOL Audio_IsPaused(void);

/**
 * Get number of currently playing sounds
 */
int Audio_GetPlayingCount(void);

// Helper to create a simple tone for testing
AudioSample* Audio_CreateTestTone(uint32_t frequency, uint32_t durationMs);

// Free a test tone created by Audio_CreateTestTone
void Audio_FreeTestTone(AudioSample* sample);

//===========================================================================
// Music Streaming Support
//===========================================================================

/**
 * Callback type for music streaming
 * @param buffer      Output buffer for 16-bit signed PCM samples
 * @param sampleCount Number of samples to fill
 * @return Number of samples actually filled (0 if finished)
 */
typedef int (*MusicStreamCallback)(int16_t* buffer, int sampleCount,
                                   void* userdata);

/**
 * Set the music streaming callback
 * @param callback  Function that fills audio buffer
 * @param userdata  User data passed to callback
 */
void Audio_SetMusicCallback(MusicStreamCallback callback, void* userdata);

/**
 * Set music volume (0.0 to 1.0)
 */
void Audio_SetMusicVolume(float volume);

/**
 * Get music volume
 */
float Audio_GetMusicVolume(void);

//===========================================================================
// Video Audio Streaming Support
//===========================================================================

/**
 * Callback type for video audio streaming (same signature as music)
 */
typedef int (*VideoAudioCallback)(int16_t* buffer, int sampleCount,
                                  void* userdata);

/**
 * Set the video audio streaming callback
 * @param callback  Function that fills audio buffer (NULL to disable)
 * @param userdata  User data passed to callback
 * @param sampleRate Sample rate of video audio (e.g., 22050)
 */
void Audio_SetVideoCallback(VideoAudioCallback callback,
                            void* userdata, int sampleRate);

/**
 * Set video audio volume (0.0 to 1.0)
 */
void Audio_SetVideoVolume(float volume);

/**
 * Get video audio volume
 */
float Audio_GetVideoVolume(void);

#ifdef __cplusplus
}
#endif

#endif // AUDIO_AUDIO_H
