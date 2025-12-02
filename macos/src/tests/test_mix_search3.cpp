/**
 * Test: Search for sprites in ALL MIX files
 */
#include "assets/mixfile.h"
#include <cstdio>

// Missing sprites from previous test
const char* missingSprites[] = {
    "mcv.shp", "harv.shp", "harvempty.shp", "harvhalf.shp",
    "arty.shp", "jeep.shp", "apc.shp", "mnly.shp",
    "fact.shp", "powr.shp", "apwr.shp", "weap.shp",
    "tent.shp", "barr.shp", "pbox.shp", "gun.shp",
    "sam.shp", "msub.shp", "mig.shp", "yak.shp",
    nullptr
};

const char* mixFiles[] = {
    "/Users/jasson/workspace/CnC_Red_Alert/assets/allies.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/hires.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/interior.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/local.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/lores.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/russian.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/sounds.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/speech.mix",
    "/Users/jasson/workspace/CnC_Red_Alert/assets/temperat.mix",
    nullptr
};

int main() {
    printf("=== Searching for missing sprites in ALL MIX files ===\n\n");

    for (int m = 0; mixFiles[m]; m++) {
        MixFileHandle mix = Mix_Open(mixFiles[m]);
        if (!mix) continue;
        
        printf("=== %s (%d files) ===\n", mixFiles[m] + 47, Mix_GetFileCount(mix));
        
        for (int i = 0; missingSprites[i]; i++) {
            if (Mix_FileExists(mix, missingSprites[i])) {
                printf("  %-16s %6u bytes\n", missingSprites[i], 
                       Mix_GetFileSize(mix, missingSprites[i]));
            }
        }
        Mix_Close(mix);
    }

    return 0;
}
