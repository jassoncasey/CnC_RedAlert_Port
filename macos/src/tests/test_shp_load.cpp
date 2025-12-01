/**
 * Test SHP Loading from Game Archives
 *
 * Loads a real game sprite and verifies the SHP parser works correctly.
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "assets/mixfile.h"
#include "assets/shpfile.h"
#include "assets/palfile.h"

// Helper to find and load a file from the game archives
static void* LoadFromArchives(const char* filename, uint32_t* outSize) {
    *outSize = 0;

    // Archive search order (most specific to least)
    struct {
        const char* topLevel;
        const char* nested;
    } searchPaths[] = {
        {"../../assets/REDALERT.MIX", "HIRES.MIX"},    // Infantry sprites
        {"../../assets/MAIN_ALLIED.MIX", "CONQUER.MIX"}, // Vehicle/building sprites
        {"../../assets/REDALERT.MIX", "LOCAL.MIX"},    // INI files, palettes
        {"../../assets/REDALERT.MIX", "LORES.MIX"},    // Low-res sprites
        {nullptr, nullptr}
    };

    for (int i = 0; searchPaths[i].topLevel; i++) {
        MixFileHandle topMix = Mix_Open(searchPaths[i].topLevel);
        if (!topMix) continue;

        // Try nested archive
        uint32_t nestedSize = 0;
        void* nestedData = Mix_AllocReadFile(topMix, searchPaths[i].nested, &nestedSize);
        if (nestedData) {
            MixFileHandle nestedMix = Mix_OpenMemory(nestedData, nestedSize, TRUE);
            if (nestedMix) {
                if (Mix_FileExists(nestedMix, filename)) {
                    void* result = Mix_AllocReadFile(nestedMix, filename, outSize);
                    Mix_Close(nestedMix);
                    Mix_Close(topMix);
                    return result;
                }
                Mix_Close(nestedMix);
            }
        }

        Mix_Close(topMix);
    }

    return nullptr;
}

int main() {
    printf("===========================================\n");
    printf("SHP Loading Test\n");
    printf("===========================================\n\n");

    // Test 1: Load a tank sprite (1TNK.SHP from CONQUER.MIX)
    printf("Test 1: Loading 1TNK.SHP (tank sprite)...\n");
    {
        uint32_t size = 0;
        void* data = LoadFromArchives("1TNK.SHP", &size);
        if (!data) {
            printf("  FAILED: Could not find 1TNK.SHP\n");
        } else {
            printf("  Found: %u bytes\n", size);

            ShpFileHandle shp = Shp_Load(data, size);
            free(data);

            if (!shp) {
                printf("  FAILED: Could not parse SHP\n");
            } else {
                int frames = Shp_GetFrameCount(shp);
                uint16_t w = Shp_GetMaxWidth(shp);
                uint16_t h = Shp_GetMaxHeight(shp);
                printf("  SUCCESS: %d frames, max size %dx%d\n", frames, w, h);

                // Check a few frames
                for (int f = 0; f < 3 && f < frames; f++) {
                    const ShpFrame* frame = Shp_GetFrame(shp, f);
                    if (frame && frame->pixels) {
                        printf("  Frame %d: %dx%d, offset (%d, %d)\n",
                               f, frame->width, frame->height,
                               frame->offsetX, frame->offsetY);

                        // Count non-zero pixels
                        int nonZero = 0;
                        int total = frame->width * frame->height;
                        for (int i = 0; i < total; i++) {
                            if (frame->pixels[i] != 0) nonZero++;
                        }
                        printf("    Non-transparent pixels: %d/%d (%.1f%%)\n",
                               nonZero, total, 100.0f * nonZero / total);
                    }
                }

                Shp_Free(shp);
            }
        }
    }

    // Test 2: Load an infantry sprite (E1.SHP from HIRES.MIX)
    printf("\nTest 2: Loading E1.SHP (infantry sprite)...\n");
    {
        uint32_t size = 0;
        void* data = LoadFromArchives("E1.SHP", &size);
        if (!data) {
            printf("  FAILED: Could not find E1.SHP\n");
        } else {
            printf("  Found: %u bytes\n", size);

            ShpFileHandle shp = Shp_Load(data, size);
            free(data);

            if (!shp) {
                printf("  FAILED: Could not parse SHP\n");
            } else {
                int frames = Shp_GetFrameCount(shp);
                uint16_t w = Shp_GetMaxWidth(shp);
                uint16_t h = Shp_GetMaxHeight(shp);
                printf("  SUCCESS: %d frames, max size %dx%d\n", frames, w, h);

                // Infantry have lots of animation frames
                // Check every 10th frame
                for (int i = 0; i < frames; i += 10) {
                    const ShpFrame* frame = Shp_GetFrame(shp, i);
                    if (frame && frame->pixels) {
                        printf("  Frame %d: %dx%d\n", i, frame->width, frame->height);
                    }
                }

                Shp_Free(shp);
            }
        }
    }

    // Test 3: Load helicopter (HELI.SHP)
    printf("\nTest 3: Loading HELI.SHP (helicopter sprite)...\n");
    {
        uint32_t size = 0;
        void* data = LoadFromArchives("HELI.SHP", &size);
        if (!data) {
            printf("  FAILED: Could not find HELI.SHP\n");
        } else {
            printf("  Found: %u bytes\n", size);

            ShpFileHandle shp = Shp_Load(data, size);
            free(data);

            if (!shp) {
                printf("  FAILED: Could not parse SHP\n");
            } else {
                int frames = Shp_GetFrameCount(shp);
                uint16_t w = Shp_GetMaxWidth(shp);
                uint16_t h = Shp_GetMaxHeight(shp);
                printf("  SUCCESS: %d frames, max size %dx%d\n", frames, w, h);
                Shp_Free(shp);
            }
        }
    }

    // Test 4: Load and apply palette
    printf("\nTest 4: Loading SNOW.PAL palette...\n");
    {
        uint32_t size = 0;
        void* data = LoadFromArchives("SNOW.PAL", &size);
        if (!data) {
            printf("  FAILED: Could not find SNOW.PAL\n");
        } else {
            printf("  Found: %u bytes\n", size);
            if (size == 768) {
                uint8_t* pal = (uint8_t*)data;
                printf("  First 10 colors (RGB 6-bit):\n");
                for (int i = 0; i < 10; i++) {
                    int r = pal[i*3+0];
                    int g = pal[i*3+1];
                    int b = pal[i*3+2];
                    printf("    [%d] R=%d G=%d B=%d\n", i, r, g, b);
                }
                printf("  SUCCESS: Valid 256-color palette\n");
            }
            free(data);
        }
    }

    printf("\n===========================================\n");
    printf("Test Complete\n");
    printf("===========================================\n");

    return 0;
}
