/**
 * Red Alert macOS Port - Music System
 *
 * Streams background music from AUD files with seamless looping.
 * Integrates with the CoreAudio system in audio.mm.
 *
 * Music files in Red Alert are stored as AUD (IMA ADPCM compressed).
 * They're typically 22050 Hz mono, streamed to avoid loading all at once.
 */

#ifndef VIDEO_MUSIC_H
#define VIDEO_MUSIC_H

#include <cstdint>

//===========================================================================
// Music Theme Types (duplicated from scenario.h to avoid header conflicts)
//===========================================================================

enum class ThemeType : int8_t {
    NONE = -1,
    BIGFOOT = 0,
    CRUSH,
    FACE_THE_ENEMY_1,
    FACE_THE_ENEMY_2,
    HELL_MARCH,
    RUN_FOR_YOUR_LIFE,
    SMASH,
    TRENCHES,
    WORKMEN,
    AWAIT,
    DENSE,
    FOGGER,
    MUDHAND,
    RADIO,
    TWIN_GUNS,
    VECTOR,

    COUNT
};

//===========================================================================
// Music Track Information
//===========================================================================

struct MusicTrackInfo {
    ThemeType theme;            // Theme enum value
    const char* filename;       // AUD filename (without path)
    const char* title;          // Display title
    int lengthSeconds;          // Approximate length
    bool isAction;              // Action/combat music
    bool availableAllied;       // Available in Allied campaign
    bool availableSoviet;       // Available in Soviet campaign
};

// Get track info for a theme
const MusicTrackInfo* Music_GetTrackInfo(ThemeType theme);

// Get track info by index (for iteration)
const MusicTrackInfo* Music_GetTrackByIndex(int index);

// Get total track count
int Music_GetTrackCount();

//===========================================================================
// Music Playback State
//===========================================================================

enum class MusicState : int8_t {
    STOPPED = 0,
    PLAYING,
    PAUSED,
    FADING_OUT,
    FADING_IN
};

//===========================================================================
// Music System Functions
//===========================================================================

// Initialize music system (called once at startup)
void Music_Init();

// Shutdown music system (called at exit)
void Music_Shutdown();

// Play a music track
// If crossfade is true and music is playing, crossfade to new track
bool Music_Play(ThemeType theme, bool loop = true, bool crossfade = false);

// Play a track by filename (for custom tracks)
bool Music_PlayFile(const char* filename, bool loop = true, bool crossfade = false);

// Stop current music
// If fadeOut is true, fade out over fadeMs milliseconds
void Music_Stop(bool fadeOut = false, int fadeMs = 1000);

// Pause/resume music
void Music_Pause();
void Music_Resume();

// Check playback state
MusicState Music_GetState();
bool Music_IsPlaying();
bool Music_IsPaused();

// Get current track
ThemeType Music_GetCurrentTheme();

// Volume control (0.0 to 1.0)
void Music_SetVolume(float volume);
float Music_GetVolume();

// Fade volume over time
void Music_FadeVolume(float targetVolume, int durationMs);

// Update music system (call each frame)
// Handles streaming, fading, looping
void Music_Update(int elapsedMs);

//===========================================================================
// Music Queue System
//===========================================================================

// Queue tracks to play after current finishes
void Music_QueueTrack(ThemeType theme);

// Clear the queue
void Music_ClearQueue();

// Shuffle queue
void Music_ShuffleQueue();

// Get queue length
int Music_GetQueueLength();

//===========================================================================
// Playlist Functions
//===========================================================================

// Play all tracks in sequence
void Music_PlayAll(bool shuffle = false);

// Play random track from available
void Music_PlayRandom();

// Skip to next track in queue
void Music_Next();

// Go to previous track (if recently changed)
void Music_Previous();

//===========================================================================
// Settings
//===========================================================================

// Enable/disable music globally
void Music_Enable(bool enabled);
bool Music_IsEnabled();

// Set preferred sample rate (for quality vs performance)
void Music_SetSampleRate(int rate);  // 22050 or 44100

//===========================================================================
// Music Streaming Implementation (internal)
//===========================================================================

class MusicStreamer {
public:
    MusicStreamer();
    ~MusicStreamer();

    // Prevent copying
    MusicStreamer(const MusicStreamer&) = delete;
    MusicStreamer& operator=(const MusicStreamer&) = delete;

    // Load a track (prepares for streaming, doesn't start playback)
    bool Load(const char* filename);

    // Unload current track
    void Unload();

    // Check if track is loaded
    bool IsLoaded() const { return fileData_ != nullptr; }

    // Start/stop streaming
    void Start(bool loop = true);
    void Stop();

    // Pause/resume
    void Pause();
    void Resume();

    // Get state
    bool IsPlaying() const { return playing_; }
    bool IsPaused() const { return paused_; }
    bool IsLooping() const { return looping_; }

    // Volume (0.0 to 1.0)
    void SetVolume(float vol);
    float GetVolume() const { return volume_; }

    // Fill audio buffer with decoded samples
    // Returns number of samples written (0 if done and not looping)
    int FillBuffer(int16_t* buffer, int sampleCount);

    // Get track info
    int GetSampleRate() const { return sampleRate_; }
    int GetChannels() const { return channels_; }
    int GetTotalSamples() const { return totalSamples_; }
    int GetCurrentPosition() const { return currentSample_; }

    // Seek to position (in samples)
    void Seek(int samplePosition);

private:
    // File data
    uint8_t* fileData_;
    uint32_t fileSize_;
    bool ownsData_;

    // Format info
    int sampleRate_;
    int channels_;
    int totalSamples_;
    int compressionType_;

    // Streaming state
    bool playing_;
    bool paused_;
    bool looping_;
    float volume_;

    // Decode state
    int currentSample_;
    const uint8_t* decodePtr_;
    const uint8_t* decodeEnd_;

    // ADPCM state
    int16_t adpcmPredictor_;
    int adpcmStepIndex_;

    // Decode methods
    int DecodeIMA(int16_t* output, int maxSamples);
    int DecodeWestwood(int16_t* output, int maxSamples);
    void ResetDecodeState();
};

#endif // VIDEO_MUSIC_H
