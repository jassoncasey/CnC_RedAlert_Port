/**
 * Test: Full search of MAIN.MIX contents
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstdlib>

const char* targetSprites[] = {
    "mcv.shp", "harv.shp", "fact.shp", "powr.shp", "weap.shp",
    "tent.shp", "barr.shp", "apc.shp", "arty.shp", "jeep.shp",
    nullptr
};

void SearchMix(MixFileHandle mix, const char* name, int depth) {
    if (!mix) return;
    printf("%*sOpened %s (%d files)\n", depth*2, "", name, Mix_GetFileCount(mix));
    
    // Search for our target sprites
    for (int i = 0; targetSprites[i]; i++) {
        if (Mix_FileExists(mix, targetSprites[i])) {
            printf("%*s  >>> FOUND %s (%u bytes) <<<\n", depth*2, "", 
                   targetSprites[i], Mix_GetFileSize(mix, targetSprites[i]));
        }
    }
}

int main() {
    printf("=== Full search of CD1 MAIN.MIX ===\n\n");

    MixFileHandle mainMix = Mix_Open("/Volumes/CD1/MAIN.MIX");
    if (!mainMix) {
        printf("ERROR: Cannot open MAIN.MIX\n");
        return 1;
    }
    
    SearchMix(mainMix, "MAIN.MIX", 0);

    // List of potential nested archives
    const char* archives[] = {
        "CONQUER.MIX", "HIRES.MIX", "LORES.MIX", "LOCAL.MIX",
        "SOUNDS.MIX", "SPEECH.MIX", "SNOW.MIX", "TEMPERAT.MIX",
        "INTERIOR.MIX", "GENERAL.MIX", "DESERT.MIX", "EXPAND.MIX",
        "EXPAND2.MIX", "REDALERT.MIX", "ALLIES.MIX", "SOVIET.MIX",
        "NEUTRAL.MIX", "MOVIES.MIX", "SCORES.MIX",
        nullptr
    };
    
    printf("\nNested archives in MAIN.MIX:\n");
    for (int i = 0; archives[i]; i++) {
        if (Mix_FileExists(mainMix, archives[i])) {
            uint32_t size = Mix_GetFileSize(mainMix, archives[i]);
            printf("  %s: %u bytes\n", archives[i], size);
            
            // Try to open and search
            uint32_t actualSize = 0;
            void* data = Mix_AllocReadFile(mainMix, archives[i], &actualSize);
            if (data) {
                MixFileHandle nested = Mix_OpenMemory(data, actualSize, FALSE);
                SearchMix(nested, archives[i], 1);
                if (nested) Mix_Close(nested);
                free(data);
            }
        }
    }

    Mix_Close(mainMix);
    return 0;
}
