/**
 * Search CD2 MAIN.MIX for missing sprites
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstdlib>

const char* targetSprites[] = {
    "mcv.shp", "harv.shp", "fact.shp", "powr.shp", "weap.shp",
    "tent.shp", "barr.shp", "apc.shp", "arty.shp", "jeep.shp",
    "dog.shp", "spy.shp", "thf.shp", "mig.shp", "yak.shp",
    nullptr
};

void SearchMix(MixFileHandle mix, const char* name) {
    if (!mix) return;
    printf("  %s (%d files): ", name, Mix_GetFileCount(mix));
    int found = 0;
    for (int i = 0; targetSprites[i]; i++) {
        if (Mix_FileExists(mix, targetSprites[i])) found++;
    }
    printf("%d targets found\n", found);
    if (found > 0) {
        for (int i = 0; targetSprites[i]; i++) {
            if (Mix_FileExists(mix, targetSprites[i])) {
                printf("    %s (%u bytes)\n", targetSprites[i], 
                       Mix_GetFileSize(mix, targetSprites[i]));
            }
        }
    }
}

int main() {
    printf("=== Searching CD2 MAIN.MIX ===\n\n");

    MixFileHandle mainMix = Mix_Open("/Volumes/CD2/MAIN.MIX");
    if (!mainMix) {
        printf("ERROR: Cannot open CD2 MAIN.MIX\n");
        return 1;
    }
    printf("Opened CD2 MAIN.MIX (%d files)\n\n", Mix_GetFileCount(mainMix));

    // Check nested archives
    const char* archives[] = {"CONQUER.MIX", "HIRES.MIX", "EXPAND.MIX", "EXPAND2.MIX", 
                              "SOVIET.MIX", "ALLIES.MIX", nullptr};
    
    for (int i = 0; archives[i]; i++) {
        if (Mix_FileExists(mainMix, archives[i])) {
            printf("Found %s (%u bytes)\n", archives[i], Mix_GetFileSize(mainMix, archives[i]));
            uint32_t size = 0;
            void* data = Mix_AllocReadFile(mainMix, archives[i], &size);
            if (data) {
                MixFileHandle nested = Mix_OpenMemory(data, size, FALSE);
                if (nested) {
                    SearchMix(nested, archives[i]);
                    Mix_Close(nested);
                }
                free(data);
            }
        }
    }

    Mix_Close(mainMix);
    return 0;
}
