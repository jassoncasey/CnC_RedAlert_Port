/**
 * Test AssetLoader functionality
 */

#include <cstdio>
#include <cstdlib>
#include "assets/assetloader.h"
#include "assets/shpfile.h"
#include "assets/audfile.h"

int main() {
    printf("===========================================\n");
    printf("AssetLoader Test\n");
    printf("===========================================\n\n");

    // Initialize
    printf("Initializing AssetLoader...\n");
    if (!Assets_Init()) {
        printf("FAILED: Could not initialize AssetLoader\n");
        return 1;
    }
    printf("OK: AssetLoader initialized\n\n");

    // Test palette
    printf("Testing palette...\n");
    const uint8_t* pal = Assets_GetPalette();
    if (pal) {
        printf("  Palette loaded\n");
        printf("  First 5 colors (8-bit RGB):\n");
        for (int i = 0; i < 5; i++) {
            printf("    [%d] R=%d G=%d B=%d\n", i,
                   pal[i*3+0], pal[i*3+1], pal[i*3+2]);
        }
    } else {
        printf("  WARNING: No palette loaded\n");
    }
    printf("\n");

    // Test SHP loading
    printf("Testing SHP loading...\n");
    const char* shpTests[] = {"1TNK.SHP", "E1.SHP", "HELI.SHP", nullptr};
    for (int i = 0; shpTests[i]; i++) {
        ShpFileHandle shp = Assets_LoadSHP(shpTests[i]);
        if (shp) {
            printf("  %s: %d frames, %dx%d\n", shpTests[i],
                   Shp_GetFrameCount(shp),
                   Shp_GetMaxWidth(shp),
                   Shp_GetMaxHeight(shp));

            // Test RGBA conversion
            const ShpFrame* frame = Shp_GetFrame(shp, 0);
            if (frame && frame->pixels) {
                uint32_t* rgba = (uint32_t*)malloc(frame->width * frame->height * 4);
                Assets_SHPToRGBA(frame, rgba, 0);

                // Count non-transparent pixels
                int opaque = 0;
                for (int p = 0; p < frame->width * frame->height; p++) {
                    if ((rgba[p] >> 24) != 0) opaque++;
                }
                printf("    Frame 0 RGBA: %d opaque pixels\n", opaque);
                free(rgba);
            }

            Shp_Free(shp);
        } else {
            printf("  %s: NOT FOUND\n", shpTests[i]);
        }
    }
    printf("\n");

    // Test AUD loading
    printf("Testing AUD loading...\n");
    const char* audTests[] = {"CANNON1.AUD", "CHRONO2.AUD", "BUILD5.AUD", nullptr};
    for (int i = 0; audTests[i]; i++) {
        AudData* aud = Assets_LoadAUD(audTests[i]);
        if (aud) {
            printf("  %s: %u samples, %u Hz, %.2fs\n", audTests[i],
                   aud->sampleCount, aud->sampleRate,
                   (float)aud->sampleCount / aud->sampleRate);
            Aud_Free(aud);
        } else {
            printf("  %s: NOT FOUND\n", audTests[i]);
        }
    }
    printf("\n");

    // Test raw loading (RULES.INI)
    printf("Testing raw file loading...\n");
    uint32_t rulesSize = 0;
    void* rulesData = Assets_LoadRaw("RULES.INI", &rulesSize);
    if (rulesData) {
        printf("  RULES.INI: %u bytes\n", rulesSize);
        // Show first line
        char* text = (char*)rulesData;
        char firstLine[80] = {0};
        for (int i = 0; i < 79 && text[i] && text[i] != '\n' && text[i] != '\r'; i++) {
            firstLine[i] = text[i];
        }
        printf("  First line: %s\n", firstLine);
        free(rulesData);
    } else {
        printf("  RULES.INI: NOT FOUND\n");
    }
    printf("\n");

    // Shutdown
    printf("Shutting down AssetLoader...\n");
    Assets_Shutdown();
    printf("OK: AssetLoader shutdown\n");

    printf("\n===========================================\n");
    printf("Test Complete\n");
    printf("===========================================\n");

    return 0;
}
