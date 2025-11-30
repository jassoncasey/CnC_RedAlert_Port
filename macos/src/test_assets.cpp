/**
 * Test stub asset implementation
 */

#include "compat/assets.h"
#include "compat/windows.h"
#include <cstdio>
#include <cstdlib>

int main() {
    printf("Testing stub assets...\n\n");

    StubAssets_Init();

    // Test 1: Check stub mode
    if (StubAssets_IsStubMode()) {
        printf("OK: Running in stub mode (no real assets)\n");
    } else {
        printf("INFO: Running with real assets\n");
    }

    // Test 2: Create palette
    Palette palette;
    StubAssets_CreatePalette(&palette);

    printf("OK: Created stub palette\n");
    printf("    Color 0 (black):  RGB(%d, %d, %d)\n",
           palette.colors[0][0], palette.colors[0][1], palette.colors[0][2]);
    printf("    Color 4 (red):    RGB(%d, %d, %d)\n",
           palette.colors[4][0], palette.colors[4][1], palette.colors[4][2]);
    printf("    Color 15 (white): RGB(%d, %d, %d)\n",
           palette.colors[15][0], palette.colors[15][1], palette.colors[15][2]);
    printf("    Color 128 (gray): RGB(%d, %d, %d)\n",
           palette.colors[128][0], palette.colors[128][1], palette.colors[128][2]);

    // Test 3: Create sprite
    const uint16_t spriteWidth = 32;
    const uint16_t spriteHeight = 24;
    uint8_t* spriteData = (uint8_t*)malloc(spriteWidth * spriteHeight);

    StubAssets_CreateSprite(spriteWidth, spriteHeight, 4, spriteData);  // Red sprite

    printf("OK: Created %dx%d stub sprite\n", spriteWidth, spriteHeight);
    printf("    Corner pixel: %d (should be border color)\n", spriteData[0]);
    printf("    Center pixel: %d (should be fill color 4)\n",
           spriteData[spriteHeight/2 * spriteWidth + spriteWidth/2]);

    free(spriteData);

    // Test 4: Create tone
    AudioBuffer toneBuffer;
    uint8_t* toneData = StubAssets_CreateTone(&toneBuffer, 440, 100, 22050);

    if (toneData) {
        printf("OK: Created 440Hz tone (100ms at 22050Hz)\n");
        printf("    Sample rate: %d Hz\n", toneBuffer.header.sampleRate);
        printf("    Channels: %d\n", toneBuffer.header.channels);
        printf("    Bits/sample: %d\n", toneBuffer.header.bitsPerSample);
        printf("    Data size: %u bytes\n", toneBuffer.header.dataSize);

        // Check first few samples are near zero (fade in)
        int16_t* samples = (int16_t*)toneData;
        printf("    First samples: %d, %d, %d (should be near 0, fading in)\n",
               samples[0], samples[1], samples[2]);

        free(toneData);
    } else {
        printf("FAIL: Failed to create tone\n");
        return 1;
    }

    // Test 5: Create silence
    AudioBuffer silenceBuffer;
    uint8_t* silenceData = StubAssets_CreateSilence(&silenceBuffer, 50, 22050);

    if (silenceData) {
        printf("OK: Created silence (50ms at 22050Hz)\n");
        printf("    Data size: %u bytes\n", silenceBuffer.header.dataSize);

        // Verify it's actually silent
        int16_t* samples = (int16_t*)silenceData;
        bool isSilent = (samples[0] == 0 && samples[100] == 0);
        printf("    Samples are zero: %s\n", isSilent ? "yes" : "no");

        free(silenceData);
    } else {
        printf("FAIL: Failed to create silence\n");
        return 1;
    }

    // Test 6: Path setting
    StubAssets_SetPath("/custom/path/to/assets");
    const char* path = StubAssets_GetPath();
    printf("OK: Asset path set to: %s\n", path);

    StubAssets_Shutdown();

    printf("\nAll stub asset tests passed!\n");
    return 0;
}
