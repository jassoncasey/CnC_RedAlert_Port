/**
 * Red Alert macOS Port - Music System Tests
 *
 * Tests music streaming and playback system.
 */

#include "../video/music.h"
#include <cstdio>
#include <cstring>

//===========================================================================
// Test Framework
//===========================================================================

static int g_testsRun = 0;
static int g_testsPassed = 0;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    printf("  %-50s ", #name); \
    fflush(stdout); \
    test_##name(); \
    g_testsRun++; \
    g_testsPassed++; \
    printf("[PASS]\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("[FAIL]\n    Assertion failed: %s\n    At %s:%d\n", #cond, __FILE__, __LINE__); \
        throw "assertion failed"; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(x) ASSERT(x)
#define ASSERT_FALSE(x) ASSERT(!(x))
#define ASSERT_NULL(x) ASSERT((x) == nullptr)
#define ASSERT_NOT_NULL(x) ASSERT((x) != nullptr)
#define ASSERT_GT(a, b) ASSERT((a) > (b))
#define ASSERT_GE(a, b) ASSERT((a) >= (b))
#define ASSERT_LT(a, b) ASSERT((a) < (b))
#define ASSERT_LE(a, b) ASSERT((a) <= (b))

//===========================================================================
// Track Database Tests
//===========================================================================

TEST(track_count) {
    int count = Music_GetTrackCount();
    ASSERT_GT(count, 0);
    ASSERT_LE(count, 100);  // Reasonable upper bound
}

TEST(track_info_by_theme) {
    const MusicTrackInfo* info = Music_GetTrackInfo(ThemeType::HELL_MARCH);
    ASSERT_NOT_NULL(info);
    ASSERT_EQ(info->theme, ThemeType::HELL_MARCH);
    ASSERT_NOT_NULL(info->filename);
    ASSERT_NOT_NULL(info->title);
}

TEST(track_info_by_index) {
    const MusicTrackInfo* info = Music_GetTrackByIndex(0);
    ASSERT_NOT_NULL(info);
    ASSERT_NOT_NULL(info->filename);

    // Invalid index
    info = Music_GetTrackByIndex(-1);
    ASSERT_NULL(info);

    info = Music_GetTrackByIndex(1000);
    ASSERT_NULL(info);
}

TEST(track_info_invalid_theme) {
    const MusicTrackInfo* info = Music_GetTrackInfo(ThemeType::NONE);
    ASSERT_NULL(info);
}

TEST(track_hell_march) {
    // Hell March is the iconic Red Alert track
    const MusicTrackInfo* info = Music_GetTrackInfo(ThemeType::HELL_MARCH);
    ASSERT_NOT_NULL(info);
    ASSERT_TRUE(strcmp(info->title, "Hell March") == 0);
    ASSERT_TRUE(info->isAction);
    ASSERT_TRUE(info->availableAllied);
    ASSERT_TRUE(info->availableSoviet);
}

//===========================================================================
// Music State Tests
//===========================================================================

TEST(music_init_state) {
    Music_Init();

    ASSERT_EQ(Music_GetState(), MusicState::STOPPED);
    ASSERT_FALSE(Music_IsPlaying());
    ASSERT_FALSE(Music_IsPaused());
    ASSERT_EQ(Music_GetCurrentTheme(), ThemeType::NONE);
}

TEST(music_volume) {
    Music_Init();

    Music_SetVolume(0.5f);
    ASSERT_EQ(Music_GetVolume(), 0.5f);

    Music_SetVolume(0.0f);
    ASSERT_EQ(Music_GetVolume(), 0.0f);

    Music_SetVolume(1.0f);
    ASSERT_EQ(Music_GetVolume(), 1.0f);

    // Clamp to valid range
    Music_SetVolume(-0.5f);
    ASSERT_EQ(Music_GetVolume(), 0.0f);

    Music_SetVolume(2.0f);
    ASSERT_EQ(Music_GetVolume(), 1.0f);
}

TEST(music_enable_disable) {
    Music_Init();

    ASSERT_TRUE(Music_IsEnabled());

    Music_Enable(false);
    ASSERT_FALSE(Music_IsEnabled());

    Music_Enable(true);
    ASSERT_TRUE(Music_IsEnabled());
}

//===========================================================================
// Queue Tests
//===========================================================================

TEST(music_queue_empty) {
    Music_Init();
    Music_ClearQueue();

    ASSERT_EQ(Music_GetQueueLength(), 0);
}

TEST(music_queue_add) {
    Music_Init();
    Music_ClearQueue();

    Music_QueueTrack(ThemeType::HELL_MARCH);
    ASSERT_EQ(Music_GetQueueLength(), 1);

    Music_QueueTrack(ThemeType::BIGFOOT);
    ASSERT_EQ(Music_GetQueueLength(), 2);

    Music_QueueTrack(ThemeType::CRUSH);
    ASSERT_EQ(Music_GetQueueLength(), 3);
}

TEST(music_queue_clear) {
    Music_Init();

    Music_QueueTrack(ThemeType::HELL_MARCH);
    Music_QueueTrack(ThemeType::BIGFOOT);
    ASSERT_GT(Music_GetQueueLength(), 0);

    Music_ClearQueue();
    ASSERT_EQ(Music_GetQueueLength(), 0);
}

//===========================================================================
// MusicStreamer Tests
//===========================================================================

TEST(streamer_create_destroy) {
    MusicStreamer streamer;
    ASSERT_FALSE(streamer.IsLoaded());
    ASSERT_FALSE(streamer.IsPlaying());
    ASSERT_FALSE(streamer.IsPaused());
}

TEST(streamer_load_nonexistent) {
    MusicStreamer streamer;
    bool loaded = streamer.Load("/nonexistent/path/to/music.aud");
    ASSERT_FALSE(loaded);
    ASSERT_FALSE(streamer.IsLoaded());
}

TEST(streamer_unload) {
    MusicStreamer streamer;
    streamer.Unload();  // Should not crash
    ASSERT_FALSE(streamer.IsLoaded());
}

TEST(streamer_volume) {
    MusicStreamer streamer;

    streamer.SetVolume(0.5f);
    ASSERT_EQ(streamer.GetVolume(), 0.5f);

    streamer.SetVolume(0.0f);
    ASSERT_EQ(streamer.GetVolume(), 0.0f);

    streamer.SetVolume(1.0f);
    ASSERT_EQ(streamer.GetVolume(), 1.0f);

    // Clamp
    streamer.SetVolume(-1.0f);
    ASSERT_EQ(streamer.GetVolume(), 0.0f);

    streamer.SetVolume(5.0f);
    ASSERT_EQ(streamer.GetVolume(), 1.0f);
}

TEST(streamer_playback_control) {
    MusicStreamer streamer;

    // Can't play without loading
    streamer.Start(true);
    ASSERT_FALSE(streamer.IsPlaying());

    streamer.Stop();
    ASSERT_FALSE(streamer.IsPlaying());

    streamer.Pause();
    ASSERT_FALSE(streamer.IsPaused());  // Wasn't playing
}

TEST(streamer_defaults) {
    MusicStreamer streamer;

    ASSERT_EQ(streamer.GetSampleRate(), 22050);
    ASSERT_EQ(streamer.GetChannels(), 1);
    ASSERT_EQ(streamer.GetTotalSamples(), 0);
    ASSERT_EQ(streamer.GetCurrentPosition(), 0);
}

TEST(streamer_fill_unloaded) {
    MusicStreamer streamer;

    int16_t buffer[256];
    int filled = streamer.FillBuffer(buffer, 256);
    ASSERT_EQ(filled, 0);
}

//===========================================================================
// Integration Tests
//===========================================================================

TEST(music_stop_when_not_playing) {
    Music_Init();
    Music_Stop(false);  // Should not crash
    ASSERT_EQ(Music_GetState(), MusicState::STOPPED);
}

TEST(music_pause_when_not_playing) {
    Music_Init();
    Music_Pause();  // Should not crash
    ASSERT_EQ(Music_GetState(), MusicState::STOPPED);  // Still stopped
}

TEST(music_resume_when_not_paused) {
    Music_Init();
    Music_Resume();  // Should not crash
    ASSERT_EQ(Music_GetState(), MusicState::STOPPED);
}

TEST(music_play_null_file) {
    Music_Init();
    bool result = Music_PlayFile(nullptr, true, false);
    ASSERT_FALSE(result);
}

TEST(music_play_nonexistent) {
    Music_Init();
    bool result = Music_PlayFile("/nonexistent/music.aud", true, false);
    ASSERT_FALSE(result);
}

TEST(music_update_stopped) {
    Music_Init();
    Music_Update(100);  // Should not crash
    ASSERT_EQ(Music_GetState(), MusicState::STOPPED);
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("\n=== Music System Tests ===\n\n");

    try {
        // Track database tests
        RUN_TEST(track_count);
        RUN_TEST(track_info_by_theme);
        RUN_TEST(track_info_by_index);
        RUN_TEST(track_info_invalid_theme);
        RUN_TEST(track_hell_march);

        // Music state tests
        RUN_TEST(music_init_state);
        RUN_TEST(music_volume);
        RUN_TEST(music_enable_disable);

        // Queue tests
        RUN_TEST(music_queue_empty);
        RUN_TEST(music_queue_add);
        RUN_TEST(music_queue_clear);

        // Streamer tests
        RUN_TEST(streamer_create_destroy);
        RUN_TEST(streamer_load_nonexistent);
        RUN_TEST(streamer_unload);
        RUN_TEST(streamer_volume);
        RUN_TEST(streamer_playback_control);
        RUN_TEST(streamer_defaults);
        RUN_TEST(streamer_fill_unloaded);

        // Integration tests
        RUN_TEST(music_stop_when_not_playing);
        RUN_TEST(music_pause_when_not_playing);
        RUN_TEST(music_resume_when_not_paused);
        RUN_TEST(music_play_null_file);
        RUN_TEST(music_play_nonexistent);
        RUN_TEST(music_update_stopped);

    } catch (...) {
        // Test failed
    }

    printf("\n=== Results: %d/%d tests passed ===\n\n", g_testsPassed, g_testsRun);

    Music_Shutdown();

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
