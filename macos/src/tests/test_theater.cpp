/**
 * Test theater support
 * Verifies palette loading and theater switching
 */

#include <cstdio>
#include "assets/assetloader.h"

int main() {
    printf("Testing theater support...\n\n");

    // Initialize assets
    if (!Assets_Init()) {
        printf("FAIL: Could not initialize assets\n");
        return 1;
    }

    // Test each theater
    const char* theaterNames[] = {
        "TEMPERATE", "SNOW", "INTERIOR", "DESERT"
    };

    for (int t = 0; t < 4; t++) {
        TheaterType theater = (TheaterType)t;
        printf("=== Testing %s theater ===\n", theaterNames[t]);

        if (Assets_SetTheater(theater)) {
            printf("  Set theater: OK\n");
            printf("  Current theater: %d\n", Assets_GetTheater());

            // Check if palette was loaded
            const uint8_t* pal = Assets_GetPalette();
            if (pal) {
                // Show some palette entries
                printf("  Palette loaded: YES\n");
                printf("  Sample colors:\n");
                printf("    Index 0:   R=%d G=%d B=%d\n",
                       pal[0], pal[1], pal[2]);
                printf("    Index 15:  R=%d G=%d B=%d\n",
                       pal[45], pal[46], pal[47]);
                printf("    Index 127: R=%d G=%d B=%d\n",
                       pal[381], pal[382], pal[383]);
            } else {
                printf("  Palette loaded: NO\n");
            }

            // Try loading a template from this theater
            uint32_t size = 0;
            void* data = Assets_LoadTemplate("CLEAR1.TEM", &size);
            if (data) {
                printf("  CLEAR1.TEM: %u bytes\n", size);
                free(data);
            } else {
                printf("  CLEAR1.TEM: not found\n");
            }
        } else {
            printf("  Set theater: FAILED (assets not available)\n");
        }
        printf("\n");
    }

    Assets_Shutdown();
    printf("Test complete.\n");
    return 0;
}
