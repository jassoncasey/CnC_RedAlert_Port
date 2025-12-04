/**
 * wwd-media - Audio System
 *
 * CoreAudio-based sound effect and music playback.
 * Supports mixing multiple simultaneous sounds.
 */

#ifndef WWD_AUDIO_H
#define WWD_AUDIO_H

#include "wwd/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the audio system
 * @return WWD_TRUE on success
 */
WwdBool Wwd_Audio_Init(void);

/**
 * Shutdown the audio system
 */
void Wwd_Audio_Shutdown(void);

/**
 * Update audio system (call each frame)
 * Handles sound completion callbacks, etc.
 */
void Wwd_Audio_Update(void);

/**
 * Play a sound effect
 * @param sample   The audio sample to play
 * @param volume   Volume level (0-255, 255 = full)
 * @param pan      Pan position (-128 = left, 0 = center, 127 = right)
 * @param loop     If WWD_TRUE, loop the sound
 * @return Handle to the playing sound, or 0 on failure
 */
WwdSoundHandle Wwd_Audio_Play(const WwdAudioSample* sample, uint8_t volume,
                            int8_t pan, WwdBool loop);

/**
 * Stop a playing sound
 */
void Wwd_Audio_Stop(WwdSoundHandle handle);

/**
 * Stop all sounds
 */
void Wwd_Audio_StopAll(void);

/**
 * Check if a sound is still playing
 */
WwdBool Wwd_Audio_IsPlaying(WwdSoundHandle handle);

/**
 * Set volume of a playing sound
 */
void Wwd_Audio_SetVolume(WwdSoundHandle handle, uint8_t volume);

/**
 * Set pan of a playing sound
 */
void Wwd_Audio_SetPan(WwdSoundHandle handle, int8_t pan);

/**
 * Set master volume (affects all sounds)
 * @param volume  0-255
 */
void Wwd_Audio_SetMasterVolume(uint8_t volume);

/**
 * Get master volume
 */
uint8_t Wwd_Audio_GetMasterVolume(void);

/**
 * Set sound effects volume (only affects sound channels, not music)
 * @param volume  0-255
 */
void Wwd_Audio_SetSoundVolume(uint8_t volume);

/**
 * Get sound effects volume
 */
uint8_t Wwd_Audio_GetSoundVolume(void);

/**
 * Pause/unpause all audio
 */
void Wwd_Audio_Pause(WwdBool pause);

/**
 * Check if audio is paused
 */
WwdBool Wwd_Audio_IsPaused(void);

/**
 * Get number of currently playing sounds
 */
int Wwd_Audio_GetPlayingCount(void);

/**
 * Create a simple tone for testing
 */
WwdAudioSample* Wwd_Audio_CreateTestTone(uint32_t frequency, uint32_t durationMs);

/**
 * Free a test tone created by Wwd_Audio_CreateTestTone
 */
void Wwd_Audio_FreeTestTone(WwdAudioSample* sample);

//===========================================================================
// Music Streaming Support
//===========================================================================

/**
 * Callback type for music streaming
 * @param buffer      Output buffer for 16-bit signed PCM samples
 * @param sampleCount Number of samples to fill
 * @return Number of samples actually filled (0 if finished)
 */
typedef int (*WwdMusicStreamCallback)(int16_t* buffer, int sampleCount,
                                     void* userdata);

/**
 * Set the music streaming callback
 * @param callback  Function that fills audio buffer
 * @param userdata  User data passed to callback
 */
void Wwd_Audio_SetMusicCallback(WwdMusicStreamCallback callback, void* userdata);

/**
 * Set music volume (0.0 to 1.0)
 */
void Wwd_Audio_SetMusicVolume(float volume);

/**
 * Get music volume
 */
float Wwd_Audio_GetMusicVolume(void);

//===========================================================================
// Video Audio Streaming Support
//===========================================================================

/**
 * Callback type for video audio streaming (same signature as music)
 */
typedef int (*WwdVideoAudioCallback)(int16_t* buffer, int sampleCount,
                                    void* userdata);

/**
 * Set the video audio streaming callback
 * @param callback   Function that fills audio buffer (NULL to disable)
 * @param userdata   User data passed to callback
 * @param sampleRate Sample rate of video audio (e.g., 22050)
 */
void Wwd_Audio_SetVideoCallback(WwdVideoAudioCallback callback,
                               void* userdata, int sampleRate);

/**
 * Set video audio volume (0.0 to 1.0)
 */
void Wwd_Audio_SetVideoVolume(float volume);

/**
 * Get video audio volume
 */
float Wwd_Audio_GetVideoVolume(void);

#ifdef __cplusplus
}
#endif

#endif // WWD_AUDIO_H
