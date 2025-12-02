/**
 * Test: Search for tileset files in MAIN.MIX from CD2
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstdlib>

int main() {
    printf("=== Testing MAIN.MIX from CD2 ===\n\n");
    
    // Open MAIN.MIX from CD2
    MixFileHandle mainMix = Mix_Open("/Volumes/CD2/MAIN.MIX");
    if (!mainMix) {
        printf("ERROR: Could not open /Volumes/CD2/MAIN.MIX\n");
        return 1;
    }
    
    printf("Opened MAIN.MIX from CD2 (%d files)\n", Mix_GetFileCount(mainMix));
    
    // Search for tileset files
    const char* files[] = {
        "SNOW.MIX", "TEMPERAT.MIX", "INTERIOR.MIX",
        "CONQUER.MIX", "SOUNDS.MIX", "ALLIES.MIX",
        "RUSSIAN.MIX", "MOVIES2.MIX", "SCORES.MIX",
        "GENERAL.MIX",
        nullptr
    };
    
    printf("\nSearching for nested MIX files:\n");
    for (int i = 0; files[i]; i++) {
        if (Mix_FileExists(mainMix, files[i])) {
            printf("  FOUND: %s (%u bytes)\n", files[i], Mix_GetFileSize(mainMix, files[i]));
        }
    }
    
    // Extract and search SNOW.MIX
    printf("\n--- Checking SNOW.MIX ---\n");
    uint32_t size = 0;
    void* snowData = Mix_AllocReadFile(mainMix, "SNOW.MIX", &size);
    if (snowData && size > 0) {
        printf("Found SNOW.MIX (%u bytes)\n", size);
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
    
    // Check TEMPERAT.MIX
    printf("\n--- Checking TEMPERAT.MIX ---\n");
    void* tempData = Mix_AllocReadFile(mainMix, "TEMPERAT.MIX", &size);
    if (tempData && size > 0) {
        printf("Found TEMPERAT.MIX (%u bytes)\n", size);
        MixFileHandle tempMix = Mix_OpenMemory(tempData, size, TRUE);
        if (tempMix) {
            printf("TEMPERAT.MIX contains %d files\n", Mix_GetFileCount(tempMix));
            
            const char* templates[] = {
                "clear1.tem", "CLEAR1.TEM",
                nullptr
            };
            
            for (int i = 0; templates[i]; i++) {
                if (Mix_FileExists(tempMix, templates[i])) {
                    printf("  FOUND: %s (%u bytes)\n", templates[i], Mix_GetFileSize(tempMix, templates[i]));
                }
            }
            
            Mix_Close(tempMix);
        } else {
            printf("Failed to open TEMPERAT.MIX as MIX file\n");
            free(tempData);
        }
    } else {
        printf("TEMPERAT.MIX not found\n");
    }
    
    Mix_Close(mainMix);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
