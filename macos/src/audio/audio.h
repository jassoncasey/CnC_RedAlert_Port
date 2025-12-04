/**
 * Red Alert macOS Port - Audio System
 *
 * Compatibility header - forwards to ra-media library.
 */

#ifndef AUDIO_AUDIO_H
#define AUDIO_AUDIO_H

#include "compat/windows.h"
#include <wwd/audio.h>

// Maximum channels alias
#define AUDIO_MAX_CHANNELS WWD_AUDIO_MAX_CHANNELS

// Type aliases
typedef WwdAudioSample AudioSample;
typedef WwdSoundHandle SoundHandle;

#ifdef __cplusplus
extern "C" {
#endif

// Function aliases - map game API to ra-media API
static inline BOOL Audio_Init(void) {
    return Wwd_Audio_Init();
}

static inline void Audio_Shutdown(void) {
    Wwd_Audio_Shutdown();
}

static inline void Audio_Update(void) {
    Wwd_Audio_Update();
}

static inline SoundHandle Audio_Play(const AudioSample* sample, uint8_t volume,
                                     int8_t pan, BOOL loop) {
    return Wwd_Audio_Play((const WwdAudioSample*)sample, volume, pan, loop);
}

static inline void Audio_Stop(SoundHandle handle) {
    Wwd_Audio_Stop(handle);
}

static inline void Audio_StopAll(void) {
    Wwd_Audio_StopAll();
}

static inline BOOL Audio_IsPlaying(SoundHandle handle) {
    return Wwd_Audio_IsPlaying(handle);
}

static inline void Audio_SetVolume(SoundHandle handle, uint8_t volume) {
    Wwd_Audio_SetVolume(handle, volume);
}

static inline void Audio_SetPan(SoundHandle handle, int8_t pan) {
    Wwd_Audio_SetPan(handle, pan);
}

static inline void Audio_SetMasterVolume(uint8_t volume) {
    Wwd_Audio_SetMasterVolume(volume);
}

static inline uint8_t Audio_GetMasterVolume(void) {
    return Wwd_Audio_GetMasterVolume();
}

static inline void Audio_SetSoundVolume(uint8_t volume) {
    Wwd_Audio_SetSoundVolume(volume);
}

static inline uint8_t Audio_GetSoundVolume(void) {
    return Wwd_Audio_GetSoundVolume();
}

static inline void Audio_Pause(BOOL pause) {
    Wwd_Audio_Pause(pause);
}

static inline BOOL Audio_IsPaused(void) {
    return Wwd_Audio_IsPaused();
}

static inline int Audio_GetPlayingCount(void) {
    return Wwd_Audio_GetPlayingCount();
}

static inline AudioSample* Audio_CreateTestTone(uint32_t frequency,
                                                uint32_t durationMs) {
    return (AudioSample*)Wwd_Audio_CreateTestTone(frequency, durationMs);
}

static inline void Audio_FreeTestTone(AudioSample* sample) {
    Wwd_Audio_FreeTestTone((WwdAudioSample*)sample);
}

// Music streaming callback type alias
typedef WwdMusicStreamCallback MusicStreamCallback;

static inline void Audio_SetMusicCallback(MusicStreamCallback callback,
                                          void* userdata) {
    Wwd_Audio_SetMusicCallback(callback, userdata);
}

static inline void Audio_SetMusicVolume(float volume) {
    Wwd_Audio_SetMusicVolume(volume);
}

static inline float Audio_GetMusicVolume(void) {
    return Wwd_Audio_GetMusicVolume();
}

// Video audio callback type alias
typedef WwdVideoAudioCallback VideoAudioCallback;

static inline void Audio_SetVideoCallback(VideoAudioCallback callback,
                                          void* userdata, int sampleRate) {
    Wwd_Audio_SetVideoCallback(callback, userdata, sampleRate);
}

static inline void Audio_SetVideoVolume(float volume) {
    Wwd_Audio_SetVideoVolume(volume);
}

static inline float Audio_GetVideoVolume(void) {
    return Wwd_Audio_GetVideoVolume();
}

#ifdef __cplusplus
}
#endif

#endif // AUDIO_AUDIO_H
