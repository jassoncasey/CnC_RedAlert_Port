/**
 * Red Alert macOS Port - Audio Implementation
 *
 * Uses AVFoundation/AudioToolbox for sound playback.
 * Supports mixing multiple simultaneous sounds via AudioUnit.
 */

#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#include "audio.h"
#include <cstring>
#include <cmath>
#include <mutex>

// Output audio format
static const Float64 kOutputSampleRate = 44100.0;
static const UInt32 kOutputChannels = 2;

// Sound channel state
typedef struct {
    const AudioSample* sample;  // Source sample (not owned)
    uint32_t position;          // Current playback position in bytes
    uint8_t volume;             // Channel volume (0-255)
    int8_t pan;                 // Pan (-128 to 127)
    BOOL loop;                  // Loop flag
    BOOL playing;               // Is this channel active?
    SoundHandle handle;         // Unique handle for this instance
} AudioChannel;

// Global state
static AudioComponentInstance g_audioUnit = nullptr;
static AudioChannel g_channels[AUDIO_MAX_CHANNELS];
static uint8_t g_masterVolume = 255;
static uint8_t g_soundVolume = 255;  // Separate volume for sound effects
static BOOL g_paused = FALSE;
static BOOL g_initialized = FALSE;
static SoundHandle g_nextHandle = 1;
static std::mutex g_audioMutex;

// Music streaming state
static MusicStreamCallback g_musicCallback = nullptr;
static void* g_musicUserdata = nullptr;
static float g_musicVolume = 1.0f;
static int16_t g_musicBuffer[4096];  // Temp buffer for music samples

// Video audio streaming state
static VideoAudioCallback g_videoCallback = nullptr;
static void* g_videoUserdata = nullptr;
static float g_videoVolume = 1.0f;
static int g_videoSampleRate = 22050;
static int16_t g_videoBuffer[8192];  // Temp buffer for video audio samples

// Convert 8-bit unsigned to float
static inline Float32 Sample8ToFloat(uint8_t sample) {
    return ((Float32)sample - 128.0f) / 128.0f;
}

// Convert 16-bit signed to float
static inline Float32 Sample16ToFloat(int16_t sample) {
    return (Float32)sample / 32768.0f;
}

// Audio render callback - mixes all active channels
static OSStatus AudioRenderCallback(
    void* inRefCon,
    AudioUnitRenderActionFlags* ioActionFlags,
    const AudioTimeStamp* inTimeStamp,
    UInt32 inBusNumber,
    UInt32 inNumberFrames,
    AudioBufferList* ioData)
{
    (void)inRefCon;
    (void)ioActionFlags;
    (void)inTimeStamp;
    (void)inBusNumber;

    Float32* leftBuffer = (Float32*)ioData->mBuffers[0].mData;
    Float32* rightBuffer = (Float32*)ioData->mBuffers[1].mData;

    // Clear buffers
    memset(leftBuffer, 0, inNumberFrames * sizeof(Float32));
    memset(rightBuffer, 0, inNumberFrames * sizeof(Float32));

    if (g_paused) {
        return noErr;
    }

    std::lock_guard<std::mutex> lock(g_audioMutex);

    Float32 masterVol = (Float32)g_masterVolume / 255.0f;
    Float32 soundVol = (Float32)g_soundVolume / 255.0f;

    for (int ch = 0; ch < AUDIO_MAX_CHANNELS; ch++) {
        AudioChannel* channel = &g_channels[ch];
        if (!channel->playing || !channel->sample) {
            continue;
        }

        const AudioSample* sample = channel->sample;
        Float32 channelVol = ((Float32)channel->volume / 255.0f) * masterVol * soundVol;

        // Calculate left/right volumes from pan
        // pan: -128 = full left, 0 = center, 127 = full right
        Float32 panNorm = (Float32)(channel->pan + 128) / 255.0f; // 0.0 to 1.0
        Float32 leftVol = channelVol * (1.0f - panNorm * 0.5f);
        Float32 rightVol = channelVol * (0.5f + panNorm * 0.5f);

        // Calculate sample rate conversion ratio
        Float64 sampleRatio = (Float64)sample->sampleRate / kOutputSampleRate;
        uint32_t bytesPerSample = sample->bitsPerSample / 8;
        uint32_t bytesPerSourceFrame = bytesPerSample * sample->channels;

        // Use fractional position for accurate resampling
        // position is stored in samples * 256 (8.8 fixed point)
        Float64 srcSamplePos = (Float64)channel->position / 256.0;

        for (UInt32 frame = 0; frame < inNumberFrames; frame++) {
            // Calculate current source sample index
            uint32_t srcSampleIdx = (uint32_t)srcSamplePos;
            uint32_t srcBytePos = srcSampleIdx * bytesPerSourceFrame;

            // Check for end of sample
            if (srcBytePos >= sample->dataSize) {
                if (channel->loop) {
                    srcSampleIdx = srcSampleIdx % (sample->dataSize / bytesPerSourceFrame);
                    srcBytePos = srcSampleIdx * bytesPerSourceFrame;
                } else {
                    channel->playing = FALSE;
                    break;
                }
            }

            // Read sample value
            Float32 sampleValue = 0.0f;
            if (sample->bitsPerSample == 8) {
                sampleValue = Sample8ToFloat(sample->data[srcBytePos]);
            } else if (sample->bitsPerSample == 16) {
                int16_t* ptr = (int16_t*)(sample->data + srcBytePos);
                sampleValue = Sample16ToFloat(*ptr);
            }

            // If stereo, handle both channels
            if (sample->channels == 2) {
                Float32 rightSample;
                if (sample->bitsPerSample == 8) {
                    rightSample = Sample8ToFloat(sample->data[srcBytePos + 1]);
                } else {
                    int16_t* ptr = (int16_t*)(sample->data + srcBytePos + 2);
                    rightSample = Sample16ToFloat(*ptr);
                }
                // Mix left sample to left, right sample to right
                leftBuffer[frame] += sampleValue * leftVol;
                rightBuffer[frame] += rightSample * rightVol;
            } else {
                // Mono - send to both channels
                leftBuffer[frame] += sampleValue * leftVol;
                rightBuffer[frame] += sampleValue * rightVol;
            }

            // Advance source position
            srcSamplePos += sampleRatio;
        }

        // Update position (in 8.8 fixed point)
        if (channel->playing) {
            channel->position = (uint32_t)(srcSamplePos * 256.0);
            uint32_t totalSamples = sample->dataSize / bytesPerSourceFrame;
            if (channel->position / 256 >= totalSamples) {
                if (channel->loop) {
                    channel->position = channel->position % (totalSamples * 256);
                } else {
                    channel->playing = FALSE;
                }
            }
        }
    }

    // Mix in music (streaming audio)
    // Music files are typically 22050 Hz mono, output is 44100 Hz stereo
    // We need to upsample 2x with linear interpolation
    if (g_musicCallback) {
        // Request half as many samples (22050 Hz source -> 44100 Hz output)
        int samplesToGet = (int)(inNumberFrames / 2) + 1;
        if (samplesToGet > 2048) samplesToGet = 2048;

        int samplesGot = g_musicCallback(g_musicBuffer, samplesToGet, g_musicUserdata);

        if (samplesGot > 0) {
            float musicVol = g_musicVolume * masterVol;

            // Upsample 2x with linear interpolation
            for (UInt32 i = 0; i < inNumberFrames; i++) {
                // Calculate source position (output 44100 maps to input 22050)
                float srcPos = (float)i * 0.5f;
                int srcIdx = (int)srcPos;
                float frac = srcPos - (float)srcIdx;

                // Get two adjacent samples for interpolation
                int16_t s0 = (srcIdx < samplesGot) ? g_musicBuffer[srcIdx] : 0;
                int16_t s1 = (srcIdx + 1 < samplesGot) ? g_musicBuffer[srcIdx + 1] : s0;

                // Linear interpolation between samples
                float interpolated = (1.0f - frac) * (float)s0 + frac * (float)s1;
                Float32 sample = interpolated / 32768.0f * musicVol;

                // Music is mono - send to both channels
                leftBuffer[i] += sample;
                rightBuffer[i] += sample;
            }
        }
    }

    // Mix in video audio (streaming from VQA)
    // Video audio is typically 22050 Hz mono, output is 44100 Hz stereo
    if (g_videoCallback) {
        // Calculate resampling ratio
        float resampleRatio = (float)g_videoSampleRate / kOutputSampleRate;
        int samplesToGet = (int)(inNumberFrames * resampleRatio) + 1;
        if (samplesToGet > 4096) samplesToGet = 4096;

        int samplesGot = g_videoCallback(g_videoBuffer, samplesToGet, g_videoUserdata);

        if (samplesGot > 0) {
            float videoVol = g_videoVolume * masterVol;

            // Resample with linear interpolation
            // Track last valid sample to avoid abrupt transitions on underrun
            int16_t lastSample = g_videoBuffer[0];
            for (UInt32 i = 0; i < inNumberFrames; i++) {
                float srcPos = (float)i * resampleRatio;
                int srcIdx = (int)srcPos;
                float frac = srcPos - (float)srcIdx;

                // Hold last sample on underrun instead of jumping to 0
                int16_t s0 = (srcIdx < samplesGot) ? g_videoBuffer[srcIdx] : lastSample;
                int16_t s1 = (srcIdx + 1 < samplesGot) ? g_videoBuffer[srcIdx + 1] : s0;
                if (srcIdx < samplesGot) lastSample = s0;

                float interpolated = (1.0f - frac) * (float)s0 + frac * (float)s1;
                Float32 sample = interpolated / 32768.0f * videoVol;

                // Video audio is mono - send to both channels
                leftBuffer[i] += sample;
                rightBuffer[i] += sample;
            }
        }
    }

    // Clamp output to prevent clipping
    for (UInt32 frame = 0; frame < inNumberFrames; frame++) {
        if (leftBuffer[frame] > 1.0f) leftBuffer[frame] = 1.0f;
        if (leftBuffer[frame] < -1.0f) leftBuffer[frame] = -1.0f;
        if (rightBuffer[frame] > 1.0f) rightBuffer[frame] = 1.0f;
        if (rightBuffer[frame] < -1.0f) rightBuffer[frame] = -1.0f;
    }

    return noErr;
}

BOOL Audio_Init(void) {
    if (g_initialized) {
        return TRUE;
    }

    // Initialize channels
    memset(g_channels, 0, sizeof(g_channels));

    // Describe the output audio unit
    AudioComponentDescription desc = {
        .componentType = kAudioUnitType_Output,
        .componentSubType = kAudioUnitSubType_DefaultOutput,
        .componentManufacturer = kAudioUnitManufacturer_Apple,
        .componentFlags = 0,
        .componentFlagsMask = 0
    };

    AudioComponent component = AudioComponentFindNext(nullptr, &desc);
    if (!component) {
        NSLog(@"Audio: Failed to find audio component");
        return FALSE;
    }

    OSStatus status = AudioComponentInstanceNew(component, &g_audioUnit);
    if (status != noErr) {
        NSLog(@"Audio: Failed to create audio unit: %d", (int)status);
        return FALSE;
    }

    // Set up audio format (non-interleaved float)
    AudioStreamBasicDescription format = {
        .mSampleRate = kOutputSampleRate,
        .mFormatID = kAudioFormatLinearPCM,
        .mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved,
        .mBytesPerPacket = sizeof(Float32),
        .mFramesPerPacket = 1,
        .mBytesPerFrame = sizeof(Float32),
        .mChannelsPerFrame = kOutputChannels,
        .mBitsPerChannel = sizeof(Float32) * 8,
        .mReserved = 0
    };

    status = AudioUnitSetProperty(g_audioUnit,
                                  kAudioUnitProperty_StreamFormat,
                                  kAudioUnitScope_Input,
                                  0,
                                  &format,
                                  sizeof(format));
    if (status != noErr) {
        NSLog(@"Audio: Failed to set stream format: %d", (int)status);
        AudioComponentInstanceDispose(g_audioUnit);
        g_audioUnit = nullptr;
        return FALSE;
    }

    // Set render callback
    AURenderCallbackStruct callback = {
        .inputProc = AudioRenderCallback,
        .inputProcRefCon = nullptr
    };

    status = AudioUnitSetProperty(g_audioUnit,
                                  kAudioUnitProperty_SetRenderCallback,
                                  kAudioUnitScope_Input,
                                  0,
                                  &callback,
                                  sizeof(callback));
    if (status != noErr) {
        NSLog(@"Audio: Failed to set render callback: %d", (int)status);
        AudioComponentInstanceDispose(g_audioUnit);
        g_audioUnit = nullptr;
        return FALSE;
    }

    // Initialize and start
    status = AudioUnitInitialize(g_audioUnit);
    if (status != noErr) {
        NSLog(@"Audio: Failed to initialize audio unit: %d", (int)status);
        AudioComponentInstanceDispose(g_audioUnit);
        g_audioUnit = nullptr;
        return FALSE;
    }

    status = AudioOutputUnitStart(g_audioUnit);
    if (status != noErr) {
        NSLog(@"Audio: Failed to start audio unit: %d", (int)status);
        AudioUnitUninitialize(g_audioUnit);
        AudioComponentInstanceDispose(g_audioUnit);
        g_audioUnit = nullptr;
        return FALSE;
    }

    g_initialized = TRUE;
    NSLog(@"Audio: Initialized (%.0f Hz, %u channels)", kOutputSampleRate, kOutputChannels);
    return TRUE;
}

void Audio_Shutdown(void) {
    if (!g_initialized) {
        return;
    }

    if (g_audioUnit) {
        AudioOutputUnitStop(g_audioUnit);
        AudioUnitUninitialize(g_audioUnit);
        AudioComponentInstanceDispose(g_audioUnit);
        g_audioUnit = nullptr;
    }

    g_initialized = FALSE;
    NSLog(@"Audio: Shutdown");
}

void Audio_Update(void) {
    // Nothing needed - mixing happens in callback
}

SoundHandle Audio_Play(const AudioSample* sample, uint8_t volume, int8_t pan, BOOL loop) {
    if (!g_initialized || !sample || !sample->data) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(g_audioMutex);

    // Find free channel
    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        if (!g_channels[i].playing) {
            g_channels[i].sample = sample;
            g_channels[i].position = 0;
            g_channels[i].volume = volume;
            g_channels[i].pan = pan;
            g_channels[i].loop = loop;
            g_channels[i].playing = TRUE;
            g_channels[i].handle = g_nextHandle++;

            if (g_nextHandle == 0) g_nextHandle = 1; // Skip 0

            return g_channels[i].handle;
        }
    }

    // No free channels
    return 0;
}

void Audio_Stop(SoundHandle handle) {
    if (!handle) return;

    std::lock_guard<std::mutex> lock(g_audioMutex);

    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        if (g_channels[i].handle == handle) {
            g_channels[i].playing = FALSE;
            g_channels[i].handle = 0;
            return;
        }
    }
}

void Audio_StopAll(void) {
    std::lock_guard<std::mutex> lock(g_audioMutex);

    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        g_channels[i].playing = FALSE;
        g_channels[i].handle = 0;
    }
}

BOOL Audio_IsPlaying(SoundHandle handle) {
    if (!handle) return FALSE;

    std::lock_guard<std::mutex> lock(g_audioMutex);

    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        if (g_channels[i].handle == handle && g_channels[i].playing) {
            return TRUE;
        }
    }
    return FALSE;
}

void Audio_SetVolume(SoundHandle handle, uint8_t volume) {
    if (!handle) return;

    std::lock_guard<std::mutex> lock(g_audioMutex);

    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        if (g_channels[i].handle == handle) {
            g_channels[i].volume = volume;
            return;
        }
    }
}

void Audio_SetPan(SoundHandle handle, int8_t pan) {
    if (!handle) return;

    std::lock_guard<std::mutex> lock(g_audioMutex);

    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        if (g_channels[i].handle == handle) {
            g_channels[i].pan = pan;
            return;
        }
    }
}

void Audio_SetMasterVolume(uint8_t volume) {
    g_masterVolume = volume;
}

uint8_t Audio_GetMasterVolume(void) {
    return g_masterVolume;
}

void Audio_SetSoundVolume(uint8_t volume) {
    g_soundVolume = volume;
}

uint8_t Audio_GetSoundVolume(void) {
    return g_soundVolume;
}

void Audio_Pause(BOOL pause) {
    g_paused = pause;
}

BOOL Audio_IsPaused(void) {
    return g_paused;
}

int Audio_GetPlayingCount(void) {
    std::lock_guard<std::mutex> lock(g_audioMutex);

    int count = 0;
    for (int i = 0; i < AUDIO_MAX_CHANNELS; i++) {
        if (g_channels[i].playing) {
            count++;
        }
    }
    return count;
}

// Generate a simple sine wave test tone
AudioSample* Audio_CreateTestTone(uint32_t frequency, uint32_t durationMs) {
    AudioSample* sample = new AudioSample;

    sample->sampleRate = 22050;
    sample->channels = 1;
    sample->bitsPerSample = 16;

    uint32_t numSamples = (sample->sampleRate * durationMs) / 1000;
    sample->dataSize = numSamples * sizeof(int16_t);
    sample->data = new uint8_t[sample->dataSize];

    int16_t* ptr = (int16_t*)sample->data;
    for (uint32_t i = 0; i < numSamples; i++) {
        // Generate sine wave with fade in/out
        double t = (double)i / sample->sampleRate;
        double wave = sin(2.0 * M_PI * frequency * t);

        // Apply envelope (fade in/out over 10ms)
        double fadeTime = 0.01;
        double envelope = 1.0;
        double sampleTime = (double)i / numSamples * durationMs / 1000.0;
        double totalTime = (double)durationMs / 1000.0;

        if (sampleTime < fadeTime) {
            envelope = sampleTime / fadeTime;
        } else if (sampleTime > totalTime - fadeTime) {
            envelope = (totalTime - sampleTime) / fadeTime;
        }

        ptr[i] = (int16_t)(wave * envelope * 16000); // Not quite full volume
    }

    return sample;
}

void Audio_FreeTestTone(AudioSample* sample) {
    if (sample) {
        delete[] sample->data;
        delete sample;
    }
}

//===========================================================================
// Music Streaming
//===========================================================================

void Audio_SetMusicCallback(MusicStreamCallback callback, void* userdata) {
    std::lock_guard<std::mutex> lock(g_audioMutex);
    g_musicCallback = callback;
    g_musicUserdata = userdata;
}

void Audio_SetMusicVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_musicVolume = volume;
}

float Audio_GetMusicVolume(void) {
    return g_musicVolume;
}

//===========================================================================
// Video Audio Streaming
//===========================================================================

void Audio_SetVideoCallback(VideoAudioCallback callback, void* userdata, int sampleRate) {
    std::lock_guard<std::mutex> lock(g_audioMutex);
    g_videoCallback = callback;
    g_videoUserdata = userdata;
    g_videoSampleRate = (sampleRate > 0) ? sampleRate : 22050;
}

void Audio_SetVideoVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    g_videoVolume = volume;
}

float Audio_GetVideoVolume(void) {
    return g_videoVolume;
}
