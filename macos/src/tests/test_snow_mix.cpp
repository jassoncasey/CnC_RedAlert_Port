/**
 * Test: Verify SNOW.MIX can be opened and has clear1.sno
 */
#include "assets/mixfile.h"
#include <cstdio>

int main() {
    printf("=== Testing standalone snow.mix ===\n");
    
    MixFileHandle mix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix");
    if (!mix) {
        printf("ERROR: Could not open snow.mix\n");
        return 1;
    }
    
    printf("Opened snow.mix (%d files)\n", Mix_GetFileCount(mix));
    
    // Search for template files
    const char* files[] = {
        "clear1.sno", "CLEAR1.SNO",
        "water1.sno", "WATER1.SNO",
        "shore01.sno", "SHORE01.SNO",
        nullptr
    };
    
    for (int i = 0; files[i]; i++) {
        if (Mix_FileExists(mix, files[i])) {
            printf("  FOUND: %s (%u bytes)\n", files[i], Mix_GetFileSize(mix, files[i]));
        }
    }
    
    Mix_Close(mix);
    printf("\n=== Test Complete ===\n");
    return 0;
}
