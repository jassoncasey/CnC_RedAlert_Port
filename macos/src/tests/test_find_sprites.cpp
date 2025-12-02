/**
 * Test: Search for sprite files in all archives
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstdlib>

int main() {
    printf("=== Searching for sprite files ===\n");
    
    MixFileHandle mainMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/MAIN_ALLIED.MIX");
    MixFileHandle redalertMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/REDALERT.MIX");
    
    if (!mainMix && !redalertMix) {
        printf("ERROR: No archives found\n");
        return 1;
    }
    
    // Open nested archives
    MixFileHandle conquerMix = nullptr;
    MixFileHandle hiresMix = nullptr;
    
    if (mainMix) {
        uint32_t size;
        void* data = Mix_AllocReadFile(mainMix, "CONQUER.MIX", &size);
        if (data) {
            conquerMix = Mix_OpenMemory(data, size, TRUE);
            printf("Opened CONQUER.MIX (%d files)\n", Mix_GetFileCount(conquerMix));
        }
    }
    
    if (redalertMix) {
        uint32_t size;
        void* data = Mix_AllocReadFile(redalertMix, "HIRES.MIX", &size);
        if (data) {
            hiresMix = Mix_OpenMemory(data, size, TRUE);
            printf("Opened HIRES.MIX (%d files)\n", Mix_GetFileCount(hiresMix));
        }
    }
    
    // Files to search for
    const char* files[] = {
        // Unit sprites
        "HARV.SHP", "harv.shp",
        "MCV.SHP", "mcv.shp",
        "APC.SHP", "apc.shp",
        "ARTY.SHP", "arty.shp",
        "V2RL.SHP", "v2rl.shp",
        "TRUK.SHP", "truk.shp",
        "1TNK.SHP", "1tnk.shp",
        "2TNK.SHP", "2tnk.shp",
        // Building sprites
        "FACT.SHP", "fact.shp",
        "POWR.SHP", "powr.shp",
        "TENT.SHP", "tent.shp",
        "BARR.SHP", "barr.shp",
        "WEAP.SHP", "weap.shp",
        "PBOX.SHP", "pbox.shp",
        "GUN.SHP", "gun.shp",
        "SAM.SHP", "sam.shp",
        // Infantry
        "E1.SHP", "e1.shp",
        nullptr
    };
    
    printf("\nSearching for sprites:\n");
    for (int i = 0; files[i]; i++) {
        MixFileHandle searchOrder[] = {conquerMix, hiresMix, mainMix, redalertMix, nullptr};
        bool found = false;
        
        for (int j = 0; searchOrder[j]; j++) {
            if (Mix_FileExists(searchOrder[j], files[i])) {
                const char* archiveName = (searchOrder[j] == conquerMix) ? "CONQUER" :
                                          (searchOrder[j] == hiresMix) ? "HIRES" :
                                          (searchOrder[j] == mainMix) ? "MAIN" : "REDALERT";
                printf("  %-12s FOUND in %s (%u bytes)\n", files[i], archiveName, 
                       Mix_GetFileSize(searchOrder[j], files[i]));
                found = true;
                break;
            }
        }
    }
    
    if (conquerMix) Mix_Close(conquerMix);
    if (hiresMix) Mix_Close(hiresMix);
    if (mainMix) Mix_Close(mainMix);
    if (redalertMix) Mix_Close(redalertMix);
    
    printf("\n=== Done ===\n");
    return 0;
}
