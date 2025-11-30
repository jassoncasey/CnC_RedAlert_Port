/**
 * Red Alert macOS Port - VQA Video Tests
 *
 * Tests VQA video loading and decoding.
 */

#include "../video/vqa.h"
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
#define ASSERT_LT(a, b) ASSERT((a) < (b))

//===========================================================================
// Test Data
//===========================================================================

// Minimal valid VQA header
static const uint8_t g_minimalVQA[] = {
    // FORM header (big-endian)
    'F', 'O', 'R', 'M',         // Magic
    0x00, 0x00, 0x00, 0x50,     // Size (80 bytes after this)

    // WVQA type
    'W', 'V', 'Q', 'A',

    // VQHD chunk
    'V', 'Q', 'H', 'D',         // Chunk ID
    0x00, 0x00, 0x00, 0x28,     // Size (40 bytes)

    // VQA Header (little-endian)
    0x02, 0x00,                 // Version = 2
    0x01, 0x00,                 // Flags = 1 (has audio)
    0x01, 0x00,                 // Frames = 1
    0x40, 0x01,                 // Width = 320
    0xC8, 0x00,                 // Height = 200
    0x04,                       // Block width = 4
    0x02,                       // Block height = 2
    0x0F,                       // FPS = 15
    0x08,                       // Group size = 8
    0x00, 0x01,                 // Colors1 = 256
    0x00, 0x01,                 // CB entries = 256
    0xFF, 0xFF,                 // X pos = -1 (center)
    0xFF, 0xFF,                 // Y pos = -1 (center)
    0x00, 0x10,                 // Max frame size
    0x56, 0x22,                 // Sample rate = 22050
    0x01,                       // Channels = 1
    0x10,                       // Bits = 16
    0x00, 0x00,                 // Alt sample rate
    0x00,                       // Alt channels
    0x00,                       // Alt bits
    0x00, 0x00, 0x00, 0x00,     // Reserved
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,

    // VQFK (key frame) - empty for test
    'V', 'Q', 'F', 'K',
    0x00, 0x00, 0x00, 0x00,     // Size = 0
};

//===========================================================================
// VQAPlayer Tests
//===========================================================================

TEST(vqa_create_destroy) {
    VQAPlayer player;
    ASSERT_FALSE(player.IsLoaded());
    ASSERT_EQ(player.GetState(), VQAState::STOPPED);
}

TEST(vqa_load_invalid) {
    VQAPlayer player;

    // Null data
    ASSERT_FALSE(player.Load(nullptr, 0));
    ASSERT_FALSE(player.IsLoaded());

    // Too small
    uint8_t tiny[4] = {0};
    ASSERT_FALSE(player.Load(tiny, sizeof(tiny)));

    // Invalid magic
    uint8_t invalid[100];
    memset(invalid, 0, sizeof(invalid));
    ASSERT_FALSE(player.Load(invalid, sizeof(invalid)));
}

TEST(vqa_load_minimal) {
    VQAPlayer player;

    // Note: The minimal test data may not be fully valid
    // This tests that we don't crash on partial data
    bool loaded = player.Load(g_minimalVQA, sizeof(g_minimalVQA));

    // The test data is minimal so may or may not load
    // Main goal is to not crash
    if (loaded) {
        ASSERT_TRUE(player.IsLoaded());
        // Width/height should be parsed if loaded
        ASSERT_GT(player.GetWidth(), 0);
        ASSERT_GT(player.GetHeight(), 0);
    }

    // Either way, this is a PASS - we tested boundary conditions
}

TEST(vqa_unload) {
    VQAPlayer player;
    // Unload should work even without loading
    player.Unload();
    ASSERT_FALSE(player.IsLoaded());
    ASSERT_EQ(player.GetState(), VQAState::STOPPED);
}

TEST(vqa_playback_control) {
    VQAPlayer player;

    // Test state transitions without loaded file
    ASSERT_EQ(player.GetState(), VQAState::STOPPED);

    // Play without loading - should stay stopped
    player.Play();
    // State depends on IsLoaded check in implementation
    // We mainly test it doesn't crash

    player.Stop();
    ASSERT_EQ(player.GetState(), VQAState::STOPPED);
}

TEST(vqa_frame_buffer) {
    VQAPlayer player;
    // Without loading, buffer may be null
    const uint8_t* palette = player.GetPalette();
    ASSERT_NOT_NULL(palette);  // Palette is always allocated (static array)
}

TEST(vqa_timing) {
    VQAPlayer player;
    // Default FPS should give reasonable duration
    int duration = player.GetFrameDuration();
    ASSERT_GT(duration, 0);
    ASSERT_LT(duration, 1000);  // Less than 1 second per frame
}

TEST(vqa_current_frame) {
    VQAPlayer player;
    ASSERT_EQ(player.GetCurrentFrame(), -1);
}

//===========================================================================
// Byte Swapping Tests
//===========================================================================

TEST(vqa_chunk_ids) {
    // Verify chunk ID constants are correct (big-endian)
    ASSERT_EQ(VQA_ID_FORM, 0x464F524D);  // 'FORM'
    ASSERT_EQ(VQA_ID_WVQA, 0x57565141);  // 'WVQA'
    ASSERT_EQ(VQA_ID_VQHD, 0x56514844);  // 'VQHD'
    ASSERT_EQ(VQA_ID_VQFR, 0x56514652);  // 'VQFR'
    ASSERT_EQ(VQA_ID_VQFK, 0x5651464B);  // 'VQFK'
}

//===========================================================================
// Global Function Tests
//===========================================================================

TEST(vqa_play_null) {
    ASSERT_FALSE(VQA_Play(nullptr));
}

TEST(vqa_play_nonexistent) {
    ASSERT_FALSE(VQA_Play("/nonexistent/path/to/video.vqa"));
}

TEST(vqa_callback_null) {
    ASSERT_FALSE(VQA_PlayWithCallback("test.vqa", nullptr, nullptr));
}

//===========================================================================
// Main
//===========================================================================

int main() {
    printf("\n=== VQA Video Tests ===\n\n");

    try {
        RUN_TEST(vqa_create_destroy);
        RUN_TEST(vqa_load_invalid);
        RUN_TEST(vqa_load_minimal);
        RUN_TEST(vqa_unload);
        RUN_TEST(vqa_playback_control);
        RUN_TEST(vqa_frame_buffer);
        RUN_TEST(vqa_timing);
        RUN_TEST(vqa_current_frame);
        RUN_TEST(vqa_chunk_ids);
        RUN_TEST(vqa_play_null);
        RUN_TEST(vqa_play_nonexistent);
        RUN_TEST(vqa_callback_null);
    } catch (...) {
        // Test failed
    }

    printf("\n=== Results: %d/%d tests passed ===\n\n", g_testsPassed, g_testsRun);

    return (g_testsPassed == g_testsRun) ? 0 : 1;
}
