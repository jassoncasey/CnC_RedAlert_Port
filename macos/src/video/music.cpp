/**
 * Red Alert macOS Port - Music System Implementation
 *
 * Streams music from AUD files with seamless looping and crossfading.
 */

#include "music.h"
#include "assets/assetloader.h"
#include "audio/audio.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Helper to clamp float value (avoid std::min/max due to macro conflicts)
static inline float ClampFloat(float val, float minVal, float maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

//===========================================================================
// Music Track Database
//===========================================================================

// T=theme, F=file, N=name, B=bpm, A=action, AL=allied, SO=soviet
#define TT ThemeType
#define T true
#define F false
static const MusicTrackInfo g_musicTracks[] = {
    {TT::BIGFOOT,         "BIGF226M.AUD", "Big Foot",         226,T,T,T},
    {TT::CRUSH,           "CRUS226M.AUD", "Crush",            226,T,T,T},
    {TT::FACE_THE_ENEMY_1,"FAC1226M.AUD", "Face the Enemy 1", 226,T,T,T},
    {TT::FACE_THE_ENEMY_2,"FAC2226M.AUD", "Face the Enemy 2", 226,T,T,T},
    {TT::HELL_MARCH,      "HELL226M.AUD", "Hell March",       226,T,T,T},
    {TT::RUN_FOR_YOUR_LIFE,"RUN1226M.AUD","Run for Your Life",226,T,T,T},
    {TT::SMASH,           "SMSH226M.AUD", "Smash",            226,T,T,T},
    {TT::TRENCHES,        "TREN226M.AUD", "Trenches",         226,T,T,T},
    {TT::WORKMEN,         "WORK226M.AUD", "Workmen",          226,T,T,T},
    {TT::AWAIT,           "AWAIT.AUD",    "Await",            180,F,T,T},
    {TT::DENSE,           "DENSE_R.AUD",  "Dense",            180,F,T,T},
    {TT::FOGGER,          "FOGGER.AUD",   "Fogger",           180,F,T,T},
    {TT::MUDHAND,         "MUDHAND.AUD",  "Mud Hand",         180,T,T,T},
    {TT::RADIO,           "RADIO.AUD",    "Radio",            180,T,T,T},
    {TT::TWIN_GUNS,       "TWIN.AUD",     "Twin Guns",        180,T,T,T},
    {TT::VECTOR,          "VECTOR1A.AUD", "Vector",           180,T,T,T},
};
#undef TT
#undef T
#undef F

static const int g_musicTrackCount =
    sizeof(g_musicTracks) / sizeof(g_musicTracks[0]);

const MusicTrackInfo* Music_GetTrackInfo(ThemeType theme) {
    for (int i = 0; i < g_musicTrackCount; i++) {
        if (g_musicTracks[i].theme == theme) {
            return &g_musicTracks[i];
        }
    }
    return nullptr;
}

const MusicTrackInfo* Music_GetTrackByIndex(int index) {
    if (index >= 0 && index < g_musicTrackCount) {
        return &g_musicTracks[index];
    }
    return nullptr;
}

int Music_GetTrackCount() {
    return g_musicTrackCount;
}

//===========================================================================
// Global Music State
//===========================================================================

static MusicStreamer g_musicStreamer;
static MusicState g_musicState = MusicState::STOPPED;
static ThemeType g_currentTheme = ThemeType::NONE;
static float g_musicVolume = 1.0f;
static bool g_musicEnabled = true;
static std::vector<ThemeType> g_musicQueue;

// Fade state
static float g_fadeTargetVolume = 1.0f;
static float g_fadeStartVolume = 1.0f;
static int g_fadeDuration = 0;
static int g_fadeElapsed = 0;
static bool g_fading = false;
static bool g_stopAfterFade = false;

// Audio callback for streaming music
static int MusicAudioCallback(int16_t* buffer, int sampleCount,
                              void* userdata) {
    (void)userdata;
    return g_musicStreamer.FillBuffer(buffer, sampleCount);
}

//===========================================================================
// Music System Functions
//===========================================================================

void Music_Init() {
    g_musicState = MusicState::STOPPED;
    g_currentTheme = ThemeType::NONE;
    g_musicVolume = 1.0f;
    g_musicEnabled = true;
    g_musicQueue.clear();

    // Register with audio system
    Audio_SetMusicCallback(MusicAudioCallback, nullptr);
    Audio_SetMusicVolume(g_musicVolume);

    printf("Music: Initialized\n");
    fflush(stdout);
    if (Assets_HasMusic()) {
        printf("Music: SCORES.MIX available\n");
    } else {
        printf("Music: SCORES.MIX not found (music disabled)\n");
    }
    fflush(stdout);
}

void Music_Shutdown() {
    Music_Stop(false);
    g_musicStreamer.Unload();
    g_musicQueue.clear();

    // Unregister from audio system
    Audio_SetMusicCallback(nullptr, nullptr);
    printf("Music: Shutdown\n");
}

bool Music_Play(ThemeType theme, bool loop, bool crossfade) {
    if (!g_musicEnabled) return false;

    const MusicTrackInfo* track = Music_GetTrackInfo(theme);
    if (!track) return false;

    return Music_PlayFile(track->filename, loop, crossfade);
}

bool Music_PlayFile(const char* filename, bool loop, bool crossfade) {
    if (!g_musicEnabled || !filename) return false;

    // If crossfading and currently playing, start fade out
    if (crossfade && g_musicState == MusicState::PLAYING) {
        // Queue the new track and fade out current
        // For now, just stop and start new
    }

    g_musicStreamer.Unload();

    // Try to load from asset system (SCORES.MIX) or direct file
    if (!g_musicStreamer.Load(filename)) {
        g_musicState = MusicState::STOPPED;
        return false;
    }

    g_musicStreamer.SetVolume(g_musicVolume);
    g_musicStreamer.Start(loop);

    g_musicState = MusicState::PLAYING;

    return true;
}

void Music_Stop(bool fadeOut, int fadeMs) {
    if (fadeOut && g_musicState == MusicState::PLAYING) {
        g_fadeTargetVolume = 0.0f;
        g_fadeStartVolume = g_musicVolume;
        g_fadeDuration = fadeMs;
        g_fadeElapsed = 0;
        g_fading = true;
        g_stopAfterFade = true;
        g_musicState = MusicState::FADING_OUT;
    } else {
        g_musicStreamer.Stop();
        g_musicState = MusicState::STOPPED;
        g_currentTheme = ThemeType::NONE;
    }
}

void Music_Pause() {
    if (g_musicState == MusicState::PLAYING) {
        g_musicStreamer.Pause();
        g_musicState = MusicState::PAUSED;
    }
}

void Music_Resume() {
    if (g_musicState == MusicState::PAUSED) {
        g_musicStreamer.Resume();
        g_musicState = MusicState::PLAYING;
    }
}

MusicState Music_GetState() {
    return g_musicState;
}

bool Music_IsPlaying() {
    return g_musicState == MusicState::PLAYING ||
           g_musicState == MusicState::FADING_IN;
}

bool Music_IsPaused() {
    return g_musicState == MusicState::PAUSED;
}

ThemeType Music_GetCurrentTheme() {
    return g_currentTheme;
}

void Music_SetVolume(float volume) {
    g_musicVolume = ClampFloat(volume, 0.0f, 1.0f);
    if (!g_fading) {
        g_musicStreamer.SetVolume(g_musicVolume);
    }
    // Also update audio system volume
    Audio_SetMusicVolume(g_musicVolume);
}

float Music_GetVolume() {
    return g_musicVolume;
}

void Music_FadeVolume(float targetVolume, int durationMs) {
    g_fadeTargetVolume = ClampFloat(targetVolume, 0.0f, 1.0f);
    g_fadeStartVolume = g_musicVolume;
    g_fadeDuration = durationMs;
    g_fadeElapsed = 0;
    g_fading = true;
    g_stopAfterFade = false;

    if (targetVolume > g_musicVolume) {
        g_musicState = MusicState::FADING_IN;
    } else {
        g_musicState = MusicState::FADING_OUT;
    }
}

void Music_Update(int elapsedMs) {
    // Handle fading
    if (g_fading && g_fadeDuration > 0) {
        g_fadeElapsed += elapsedMs;

        if (g_fadeElapsed >= g_fadeDuration) {
            g_musicVolume = g_fadeTargetVolume;
            g_fading = false;

            if (g_stopAfterFade) {
                g_musicStreamer.Stop();
                g_musicState = MusicState::STOPPED;
                g_currentTheme = ThemeType::NONE;
            } else {
                g_musicState = MusicState::PLAYING;
            }
        } else {
            float t = (float)g_fadeElapsed / (float)g_fadeDuration;
            float delta = g_fadeTargetVolume - g_fadeStartVolume;
            g_musicVolume = g_fadeStartVolume + delta * t;
        }

        g_musicStreamer.SetVolume(g_musicVolume);
    }

    // Check if track finished (non-looping)
    if (g_musicState == MusicState::PLAYING && !g_musicStreamer.IsPlaying()) {
        // Track finished, play next from queue
        if (!g_musicQueue.empty()) {
            ThemeType next = g_musicQueue.front();
            g_musicQueue.erase(g_musicQueue.begin());
            Music_Play(next, true, false);
        } else {
            g_musicState = MusicState::STOPPED;
            g_currentTheme = ThemeType::NONE;
        }
    }
}

//===========================================================================
// Queue Functions
//===========================================================================

void Music_QueueTrack(ThemeType theme) {
    g_musicQueue.push_back(theme);
}

void Music_ClearQueue() {
    g_musicQueue.clear();
}

void Music_ShuffleQueue() {
    // Simple shuffle
    for (size_t i = g_musicQueue.size() - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);
        std::swap(g_musicQueue[i], g_musicQueue[j]);
    }
}

int Music_GetQueueLength() {
    return (int)g_musicQueue.size();
}

//===========================================================================
// Playlist Functions
//===========================================================================

void Music_PlayAll(bool shuffle) {
    Music_ClearQueue();

    // Add all action tracks to queue
    for (int i = 0; i < g_musicTrackCount; i++) {
        if (g_musicTracks[i].isAction) {
            g_musicQueue.push_back(g_musicTracks[i].theme);
        }
    }

    if (shuffle) {
        Music_ShuffleQueue();
    }

    // Start first track
    if (!g_musicQueue.empty()) {
        ThemeType first = g_musicQueue.front();
        g_musicQueue.erase(g_musicQueue.begin());
        Music_Play(first, false, false);  // Don't loop individual tracks
    }
}

void Music_PlayRandom() {
    // Count action tracks
    int count = 0;
    for (int i = 0; i < g_musicTrackCount; i++) {
        if (g_musicTracks[i].isAction) count++;
    }

    if (count == 0) return;

    int pick = rand() % count;
    int idx = 0;
    for (int i = 0; i < g_musicTrackCount; i++) {
        if (g_musicTracks[i].isAction) {
            if (idx == pick) {
                Music_Play(g_musicTracks[i].theme, true, false);
                return;
            }
            idx++;
        }
    }
}

void Music_Next() {
    if (!g_musicQueue.empty()) {
        ThemeType next = g_musicQueue.front();
        g_musicQueue.erase(g_musicQueue.begin());
        Music_Play(next, true, true);
    }
}

void Music_Previous() {
    // Not implemented - would need history tracking
}

//===========================================================================
// Settings
//===========================================================================

void Music_Enable(bool enabled) {
    g_musicEnabled = enabled;
    if (!enabled && g_musicState == MusicState::PLAYING) {
        Music_Stop(true, 500);
    }
}

bool Music_IsEnabled() {
    return g_musicEnabled;
}

void Music_SetSampleRate(int rate) {
    // For quality settings - affects decoding/resampling
    (void)rate;
}

//===========================================================================
// MusicStreamer Implementation
//===========================================================================

// AUD file header
#pragma pack(push, 1)
struct AUDHeader {
    uint16_t sampleRate;
    uint32_t size;          // Compressed size
    uint32_t uncompSize;    // Uncompressed size
    uint8_t  flags;         // Bit 0: stereo, Bit 1: 16-bit
    uint8_t  compression;   // 1 = Westwood, 99 = IMA ADPCM
};

struct AUDChunkHeader {
    uint16_t compSize;
    uint16_t uncompSize;
    uint32_t id;
};
#pragma pack(pop)

// IMA ADPCM tables
static const int g_imaStepTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const int g_imaIndexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

MusicStreamer::MusicStreamer()
    : fileData_(nullptr)
    , fileSize_(0)
    , ownsData_(false)
    , sampleRate_(22050)
    , channels_(1)
    , totalSamples_(0)
    , compressionType_(99)
    , playing_(false)
    , paused_(false)
    , looping_(true)
    , volume_(1.0f)
    , currentSample_(0)
    , decodePtr_(nullptr)
    , decodeEnd_(nullptr)
    , adpcmPredictor_(0)
    , adpcmStepIndex_(0)
    , currentChunkData_(nullptr)
    , currentChunkSamples_(0)
    , currentChunkPos_(0)
{
}

MusicStreamer::~MusicStreamer() {
    Unload();
}

bool MusicStreamer::Load(const char* filename) {
    Unload();

    // Try asset loader first (SCORES.MIX)
    uint32_t size = 0;
    void* data = Assets_LoadMusic(filename, &size);

    if (!data) {
        // Fallback: try direct file access
        FILE* f = fopen(filename, "rb");
        if (!f) return false;

        fseek(f, 0, SEEK_END);
        long fsize = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (fsize < (long)sizeof(AUDHeader)) {
            fclose(f);
            return false;
        }

        data = new uint8_t[fsize];
        size_t readBytes = fread(data, 1, fsize, f);
        fclose(f);

        if (readBytes != (size_t)fsize) {
            delete[] (uint8_t*)data;
            return false;
        }
        size = (uint32_t)fsize;
    }

    if (size < sizeof(AUDHeader)) {
        free(data);
        return false;
    }

    fileData_ = (uint8_t*)data;
    fileSize_ = size;
    ownsData_ = true;

    // Parse header
    const AUDHeader* hdr = (const AUDHeader*)fileData_;

    sampleRate_ = hdr->sampleRate;
    channels_ = (hdr->flags & 0x01) ? 2 : 1;
    compressionType_ = hdr->compression;

    // Calculate total samples
    int bytesPerSample = (hdr->flags & 0x02) ? 2 : 1;
    totalSamples_ = hdr->uncompSize / bytesPerSample / channels_;

    // Set up decode pointers
    decodePtr_ = fileData_ + sizeof(AUDHeader);
    decodeEnd_ = fileData_ + fileSize_;

    printf("Music: Loaded %s (%u KB, %d Hz, %s)\n",
           filename, size / 1024, sampleRate_,
           compressionType_ == 99 ? "IMA ADPCM" : "Westwood");

    return true;
}

void MusicStreamer::Unload() {
    Stop();

    if (ownsData_ && fileData_) {
        delete[] fileData_;
    }
    fileData_ = nullptr;
    fileSize_ = 0;
    ownsData_ = false;

    sampleRate_ = 22050;
    channels_ = 1;
    totalSamples_ = 0;
    compressionType_ = 99;
}

void MusicStreamer::Start(bool loop) {
    if (!IsLoaded()) return;

    looping_ = loop;
    playing_ = true;
    paused_ = false;
    currentSample_ = 0;

    ResetDecodeState();
}

void MusicStreamer::Stop() {
    playing_ = false;
    paused_ = false;
}

void MusicStreamer::Pause() {
    if (playing_) {
        paused_ = true;
    }
}

void MusicStreamer::Resume() {
    if (playing_ && paused_) {
        paused_ = false;
    }
}

void MusicStreamer::SetVolume(float vol) {
    volume_ = ClampFloat(vol, 0.0f, 1.0f);
}

void MusicStreamer::Seek(int samplePosition) {
    // Reset to beginning and decode to position
    currentSample_ = 0;
    ResetDecodeState();

    // For accurate seeking, we'd need to decode through
    // For now, just reset
    (void)samplePosition;
}

void MusicStreamer::ResetDecodeState() {
    decodePtr_ = fileData_ + sizeof(AUDHeader);
    // For Westwood ADPCM (codec 1): sample starts at 0x80 (8-bit center)
    // For IMA ADPCM (codec 99): predictor starts at 0
    adpcmPredictor_ = (compressionType_ == 1) ? 0x80 : 0;
    adpcmStepIndex_ = 0;

    // Reset partial chunk state
    currentChunkData_ = nullptr;
    currentChunkSamples_ = 0;
    currentChunkPos_ = 0;
}

int MusicStreamer::FillBuffer(int16_t* buffer, int sampleCount) {
    if (!playing_ || paused_ || !buffer || sampleCount <= 0) {
        return 0;
    }

    int samplesWritten = 0;

    while (samplesWritten < sampleCount) {
        int decoded = 0;

        if (compressionType_ == 99) {
            int16_t* dst = buffer + samplesWritten;
            int remain = sampleCount - samplesWritten;
            decoded = DecodeIMA(dst, remain);
        } else if (compressionType_ == 1) {
            int16_t* dst = buffer + samplesWritten;
            int remain = sampleCount - samplesWritten;
            decoded = DecodeWestwood(dst, remain);
        }

        if (decoded == 0) {
            // End of data
            if (looping_) {
                ResetDecodeState();
                currentSample_ = 0;
            } else {
                playing_ = false;
                break;
            }
        }

        samplesWritten += decoded;
        currentSample_ += decoded;
    }

    // Apply volume
    if (volume_ < 1.0f) {
        for (int i = 0; i < samplesWritten; i++) {
            buffer[i] = (int16_t)(buffer[i] * volume_);
        }
    }

    return samplesWritten;
}

int MusicStreamer::DecodeIMA(int16_t* output, int maxSamples) {
    int samples = 0;

    // AUD IMA ADPCM chunk format (from XCC reference):
    // 2 bytes: compressed chunk size (size_in)
    // 2 bytes: uncompressed output size in bytes (size_out)
    // 4 bytes: signature (0x0000DEAF)
    // N bytes: compressed audio data
    //
    // Key insight from XCC: decoder iterates by OUTPUT sample count,
    // which is size_out / 2 (for 16-bit samples).
    //
    // IMPORTANT: We must handle partial chunk decoding across calls.
    // The audio callback may request fewer samples than a full chunk.

    while (samples < maxSamples) {
        // If we have a partial chunk in progress, continue from there
        if (currentChunkData_ == nullptr) {
            // Need to start a new chunk
            if (decodePtr_ + sizeof(AUDChunkHeader) > decodeEnd_) {
                break;  // No more data
            }

            const AUDChunkHeader* chunk = (const AUDChunkHeader*)decodePtr_;
            uint16_t compSize = chunk->compSize;
            uint16_t uncompSize = chunk->uncompSize;

            decodePtr_ += sizeof(AUDChunkHeader);

            if (decodePtr_ + compSize > decodeEnd_) {
                break;  // Truncated chunk
            }

            // Set up partial chunk state
            currentChunkData_ = decodePtr_;
            currentChunkSamples_ = uncompSize / 2;  // 16-bit = 2 bytes/sample
            currentChunkPos_ = 0;

            // Move decode pointer past this chunk's data (for next chunk)
            decodePtr_ += compSize;
        }

        // Decode samples from current chunk
        while (currentChunkPos_ < currentChunkSamples_ && samples < maxSamples) {
            int si = currentChunkPos_++;

            // Get nibble: even sample = low nibble, odd sample = high nibble
            uint8_t byte = currentChunkData_[si >> 1];
            uint8_t code = (si & 1) ? (byte >> 4) : (byte & 0x0F);

            int step = g_imaStepTable[adpcmStepIndex_];
            int diff = step >> 3;
            if (code & 1) diff += step >> 2;
            if (code & 2) diff += step >> 1;
            if (code & 4) diff += step;

            if (code & 8) {
                adpcmPredictor_ -= diff;
                if (adpcmPredictor_ < -32768)
                    adpcmPredictor_ = -32768;
            } else {
                adpcmPredictor_ += diff;
                if (adpcmPredictor_ > 32767)
                    adpcmPredictor_ = 32767;
            }

            output[samples++] = adpcmPredictor_;

            // Update step index using only low 3 bits of code
            adpcmStepIndex_ += g_imaIndexTable[code & 7];
            if (adpcmStepIndex_ < 0)
                adpcmStepIndex_ = 0;
            else if (adpcmStepIndex_ > 88)
                adpcmStepIndex_ = 88;
        }

        // If we finished this chunk, clear partial state
        if (currentChunkPos_ >= currentChunkSamples_) {
            currentChunkData_ = nullptr;
            currentChunkSamples_ = 0;
            currentChunkPos_ = 0;
        }
    }

    return samples;
}

int MusicStreamer::DecodeWestwood(int16_t* output, int maxSamples) {
    // Westwood ADPCM decoder - based on XCC reference implementation
    // Mode 0: 2-bit deltas (4 samples per byte)
    // Mode 1: 4-bit deltas (2 samples per byte)
    // Mode 2: Raw or 5-bit signed delta
    // Mode 3: RLE repeat
    static const int ws_step_table2[] = {-2, -1, 0, 1};
    static const int ws_step_table4[] = {
        -9, -8, -6, -5, -4, -3, -2, -1,
         0,  1,  2,  3,  4,  5,  6,  8
    };

    int samples = 0;
    int sample = adpcmPredictor_;  // 8-bit value 0-255, start at 0x80

    // Process chunks until done or buffer full
    size_t hdrSize = sizeof(AUDChunkHeader);
    while (samples < maxSamples && decodePtr_ + hdrSize <= decodeEnd_) {
        const AUDChunkHeader* chunk = (const AUDChunkHeader*)decodePtr_;
        uint16_t compSize = chunk->compSize;
        uint16_t uncompSize = chunk->uncompSize;

        decodePtr_ += sizeof(AUDChunkHeader);
        const uint8_t* chunkEnd = decodePtr_ + compSize;

        if (chunkEnd > decodeEnd_) break;

        // If sizes match, data is uncompressed
        if (compSize == uncompSize) {
            while (decodePtr_ < chunkEnd && samples < maxSamples) {
                // Convert 8-bit unsigned to 16-bit signed
                sample = *decodePtr_++;
                output[samples++] = (int16_t)((sample - 128) << 8);
            }
            continue;
        }

        // Decode Westwood ADPCM
        while (decodePtr_ < chunkEnd && samples < maxSamples) {
            uint8_t cmd = *decodePtr_++;
            int count = cmd & 0x3F;
            int mode = cmd >> 6;

            switch (mode) {
            case 0:  // 2-bit deltas: 4 samples per byte
                for (int i = 0; i <= count && decodePtr_ < chunkEnd &&
                     samples < maxSamples; i++) {
                    uint8_t code = *decodePtr_++;
                    for (int j = 0; j < 4 && samples < maxSamples; j++) {
                        sample += ws_step_table2[(code >> (j * 2)) & 3];
                        if (sample < 0) sample = 0;
                        if (sample > 255) sample = 255;
                        output[samples++] = (int16_t)((sample - 128) << 8);
                    }
                }
                break;

            case 1:  // 4-bit deltas: 2 samples per byte
                for (int i = 0; i <= count && decodePtr_ < chunkEnd &&
                     samples < maxSamples; i++) {
                    uint8_t code = *decodePtr_++;
                    // Low nibble
                    sample += ws_step_table4[code & 0x0F];
                    if (sample < 0) sample = 0;
                    if (sample > 255) sample = 255;
                    output[samples++] = (int16_t)((sample - 128) << 8);
                    // High nibble
                    if (samples < maxSamples) {
                        sample += ws_step_table4[code >> 4];
                        if (sample < 0) sample = 0;
                        if (sample > 255) sample = 255;
                        output[samples++] = (int16_t)((sample - 128) << 8);
                    }
                }
                break;

            case 2:  // Raw samples or 5-bit signed delta
                if (count & 0x20) {
                    // 5-bit signed delta (sign-extend from 6 bits)
                    int delta = (int8_t)(cmd << 2) >> 2;
                    sample += delta;
                    if (sample < 0) sample = 0;
                    if (sample > 255) sample = 255;
                    output[samples++] = (int16_t)((sample - 128) << 8);
                } else {
                    // Raw samples
                    count++;
                    while (count > 0 && decodePtr_ < chunkEnd &&
                           samples < maxSamples) {
                        sample = *decodePtr_++;
                        output[samples++] = (int16_t)((sample - 128) << 8);
                        count--;
                    }
                }
                break;

            case 3:  // RLE repeat
                count++;
                while (count > 0 && samples < maxSamples) {
                    output[samples++] = (int16_t)((sample - 128) << 8);
                    count--;
                }
                break;
            }
        }

        decodePtr_ = chunkEnd;  // Ensure we're at chunk boundary
    }

    adpcmPredictor_ = sample;
    return samples;
}
