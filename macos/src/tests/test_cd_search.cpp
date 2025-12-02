/**
 * Test: Search for sprites in CD1 MAIN.MIX -> CONQUER.MIX
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstdlib>

const char* missingSprites[] = {
    "mcv.shp", "harv.shp", "harvempty.shp", "harvhalf.shp",
    "arty.shp", "jeep.shp", "apc.shp", "mnly.shp",
    "fact.shp", "powr.shp", "apwr.shp", "weap.shp",
    "tent.shp", "barr.shp", "pbox.shp", "gun.shp",
    "sam.shp", "msub.shp", "mig.shp", "yak.shp",
    nullptr
};

int main() {
    printf("=== Searching in CD1 MAIN.MIX ===\n\n");

    MixFileHandle mainMix = Mix_Open("/Volumes/CD1/MAIN.MIX");
    if (!mainMix) {
        printf("ERROR: Cannot open MAIN.MIX\n");
        return 1;
    }
    printf("Opened MAIN.MIX (%d files)\n", Mix_GetFileCount(mainMix));

    // Try to extract CONQUER.MIX from MAIN.MIX
    uint32_t conquerSize = 0;
    void* conquerData = Mix_AllocReadFile(mainMix, "CONQUER.MIX", &conquerSize);
    if (conquerData) {
        printf("Extracted CONQUER.MIX (%u bytes)\n", conquerSize);
        
        MixFileHandle conquerMix = Mix_OpenMemory(conquerData, conquerSize, FALSE);
        if (conquerMix) {
            printf("Opened CONQUER.MIX (%d files)\n", Mix_GetFileCount(conquerMix));
            
            printf("\nSearching for missing sprites:\n");
            for (int i = 0; missingSprites[i]; i++) {
                if (Mix_FileExists(conquerMix, missingSprites[i])) {
                    printf("  %-16s FOUND! %6u bytes\n", missingSprites[i],
                           Mix_GetFileSize(conquerMix, missingSprites[i]));
                }
            }
            Mix_Close(conquerMix);
        }
        free(conquerData);
    } else {
        printf("CONQUER.MIX not found in MAIN.MIX\n");
    }

    // Also check for HIRES.MIX
    uint32_t hiresSize = 0;
    void* hiresData = Mix_AllocReadFile(mainMix, "HIRES.MIX", &hiresSize);
    if (hiresData) {
        printf("\nExtracted HIRES.MIX (%u bytes)\n", hiresSize);
        
        MixFileHandle hiresMix = Mix_OpenMemory(hiresData, hiresSize, FALSE);
        if (hiresMix) {
            printf("Opened HIRES.MIX (%d files)\n", Mix_GetFileCount(hiresMix));
            
            printf("\nSearching for infantry:\n");
            const char* infantry[] = {"spy.shp", "thf.shp", "dog.shp", "tany.shp", nullptr};
            for (int i = 0; infantry[i]; i++) {
                if (Mix_FileExists(hiresMix, infantry[i])) {
                    printf("  %-16s FOUND! %6u bytes\n", infantry[i],
                           Mix_GetFileSize(hiresMix, infantry[i]));
                }
            }
            Mix_Close(hiresMix);
        }
        free(hiresData);
    }

    Mix_Close(mainMix);
    return 0;
}
