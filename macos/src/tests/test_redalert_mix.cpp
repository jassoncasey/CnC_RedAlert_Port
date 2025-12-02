/**
 * Test: Search INSTALL/REDALERT.MIX for sprites
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
    "dog.shp", "spy.shp", "thf.shp", "tany.shp",
    nullptr
};

int main() {
    printf("=== Searching INSTALL/REDALERT.MIX ===\n\n");

    MixFileHandle redalertMix = Mix_Open("/Volumes/CD1/INSTALL/REDALERT.MIX");
    if (!redalertMix) {
        printf("ERROR: Cannot open REDALERT.MIX\n");
        return 1;
    }
    printf("Opened REDALERT.MIX (%d files)\n\n", Mix_GetFileCount(redalertMix));

    // List nested archives
    const char* nestedArchives[] = {
        "CONQUER.MIX", "HIRES.MIX", "LORES.MIX", "LOCAL.MIX", 
        "SOUNDS.MIX", "SPEECH.MIX", "SNOW.MIX", "TEMPERAT.MIX", 
        "INTERIOR.MIX", "GENERAL.MIX", "DESERT.MIX",
        nullptr
    };
    
    printf("Nested archives:\n");
    for (int i = 0; nestedArchives[i]; i++) {
        if (Mix_FileExists(redalertMix, nestedArchives[i])) {
            printf("  %s: %u bytes\n", nestedArchives[i], 
                   Mix_GetFileSize(redalertMix, nestedArchives[i]));
        }
    }

    // Extract and search CONQUER.MIX
    printf("\n=== Searching nested CONQUER.MIX ===\n");
    uint32_t conquerSize = 0;
    void* conquerData = Mix_AllocReadFile(redalertMix, "CONQUER.MIX", &conquerSize);
    if (conquerData) {
        MixFileHandle conquerMix = Mix_OpenMemory(conquerData, conquerSize, FALSE);
        if (conquerMix) {
            printf("Opened CONQUER.MIX (%d files)\n", Mix_GetFileCount(conquerMix));
            for (int i = 0; missingSprites[i]; i++) {
                if (Mix_FileExists(conquerMix, missingSprites[i])) {
                    printf("  %-16s FOUND! %6u bytes\n", missingSprites[i],
                           Mix_GetFileSize(conquerMix, missingSprites[i]));
                }
            }
            Mix_Close(conquerMix);
        }
        free(conquerData);
    }

    // Extract and search HIRES.MIX
    printf("\n=== Searching nested HIRES.MIX ===\n");
    uint32_t hiresSize = 0;
    void* hiresData = Mix_AllocReadFile(redalertMix, "HIRES.MIX", &hiresSize);
    if (hiresData) {
        MixFileHandle hiresMix = Mix_OpenMemory(hiresData, hiresSize, FALSE);
        if (hiresMix) {
            printf("Opened HIRES.MIX (%d files)\n", Mix_GetFileCount(hiresMix));
            for (int i = 0; missingSprites[i]; i++) {
                if (Mix_FileExists(hiresMix, missingSprites[i])) {
                    printf("  %-16s FOUND! %6u bytes\n", missingSprites[i],
                           Mix_GetFileSize(hiresMix, missingSprites[i]));
                }
            }
            Mix_Close(hiresMix);
        }
        free(hiresData);
    }

    Mix_Close(redalertMix);
    return 0;
}
