/**
 * Test: Search for tileset files in MAIN.MIX from CD
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstdlib>

int main() {
    printf("=== Testing MAIN.MIX from CD ===\n\n");
    
    // Open MAIN.MIX from CD
    MixFileHandle mainMix = Mix_Open("/Volumes/CD1/MAIN.MIX");
    if (!mainMix) {
        printf("ERROR: Could not open /Volumes/CD1/MAIN.MIX\n");
        return 1;
    }
    
    printf("Opened MAIN.MIX from CD (%d files)\n", Mix_GetFileCount(mainMix));
    
    // Search for tileset files
    const char* files[] = {
        "SNOW.MIX", "snow.mix",
        "TEMPERAT.MIX", "temperat.mix",
        "INTERIOR.MIX", "interior.mix",
        "CONQUER.MIX", "conquer.mix",
        "SOUNDS.MIX", "sounds.mix",
        "ALLIES.MIX", "allies.mix",
        "RUSSIAN.MIX", "russian.mix",
        "MOVIES1.MIX", "movies1.mix",
        "SCORES.MIX", "scores.mix",
        "GENERAL.MIX", "general.mix",
        nullptr
    };
    
    printf("\nSearching for nested MIX files:\n");
    for (int i = 0; files[i]; i++) {
        if (Mix_FileExists(mainMix, files[i])) {
            printf("  FOUND: %s (%u bytes)\n", files[i], Mix_GetFileSize(mainMix, files[i]));
        }
    }
    
    // Extract and search SNOW.MIX
    printf("\n--- Extracting SNOW.MIX ---\n");
    uint32_t size = 0;
    void* snowData = Mix_AllocReadFile(mainMix, "SNOW.MIX", &size);
    if (snowData && size > 0) {
        printf("Extracted SNOW.MIX (%u bytes)\n", size);
        MixFileHandle snowMix = Mix_OpenMemory(snowData, size, TRUE);
        if (snowMix) {
            printf("SNOW.MIX contains %d files\n", Mix_GetFileCount(snowMix));
            
            // Search for template files
            const char* templates[] = {
                "clear1.sno", "CLEAR1.SNO",
                "water1.sno", "WATER1.SNO",
                "shore01.sno", "SHORE01.SNO",
                nullptr
            };
            
            for (int i = 0; templates[i]; i++) {
                if (Mix_FileExists(snowMix, templates[i])) {
                    printf("  FOUND: %s (%u bytes)\n", templates[i], Mix_GetFileSize(snowMix, templates[i]));
                }
            }
            
            Mix_Close(snowMix);
        } else {
            printf("Failed to open SNOW.MIX as MIX file\n");
            free(snowData);
        }
    } else {
        printf("SNOW.MIX not found in MAIN.MIX\n");
    }
    
    Mix_Close(mainMix);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
