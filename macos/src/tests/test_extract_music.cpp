/**
 * Test extracting music from SCORES.MIX and dumping to file
 */
#include "assets/assetloader.h"
#include <cstdio>
#include <cstdlib>

int main(int argc, char* argv[]) {
    const char* trackName = argc > 1 ? argv[1] : "TWIN.AUD";
    const char* outputPath = argc > 2 ? argv[2] : "/tmp/extracted.aud";

    printf("Initializing asset loader...\n");
    if (!Assets_Init()) {
        printf("ERROR: Failed to initialize asset loader\n");
        return 1;
    }

    printf("Checking for music archive...\n");
    if (!Assets_HasMusic()) {
        printf("ERROR: SCORES.MIX not available\n");
        Assets_Shutdown();
        return 1;
    }

    printf("Loading music track: %s\n", trackName);
    uint32_t size = 0;
    void* data = Assets_LoadMusic(trackName, &size);

    if (!data || size == 0) {
        printf("ERROR: Failed to load music track\n");
        Assets_Shutdown();
        return 1;
    }

    printf("Loaded %u bytes\n", size);

    // Write to file
    printf("Writing to: %s\n", outputPath);
    FILE* f = fopen(outputPath, "wb");
    if (!f) {
        printf("ERROR: Cannot open output file\n");
        free(data);
        Assets_Shutdown();
        return 1;
    }

    fwrite(data, 1, size, f);
    fclose(f);

    printf("SUCCESS: Wrote %u bytes\n", size);

    free(data);
    Assets_Shutdown();
    return 0;
}
