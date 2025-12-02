/**
 * Search expand MIX files for missing sprites
 */
#include "assets/mixfile.h"
#include <cstdio>

const char* targetSprites[] = {
    "mcv.shp", "harv.shp", "fact.shp", "powr.shp", "weap.shp",
    "tent.shp", "barr.shp", "apc.shp", "arty.shp", "jeep.shp",
    "dog.shp", "spy.shp", "thf.shp", "mig.shp", "yak.shp",
    nullptr
};

void SearchMix(MixFileHandle mix, const char* name) {
    if (!mix) return;
    printf("\n%s (%d files):\n", name, Mix_GetFileCount(mix));
    for (int i = 0; targetSprites[i]; i++) {
        if (Mix_FileExists(mix, targetSprites[i])) {
            printf("  FOUND: %s (%u bytes)\n", targetSprites[i], 
                   Mix_GetFileSize(mix, targetSprites[i]));
        }
    }
}

int main() {
    printf("=== Searching expand MIX files ===\n");

    SearchMix(Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/expand/expand2.mix"), "expand2.mix");
    SearchMix(Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/expand/hires1.mix"), "hires1.mix");
    SearchMix(Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/expand/lores1.mix"), "lores1.mix");

    return 0;
}
