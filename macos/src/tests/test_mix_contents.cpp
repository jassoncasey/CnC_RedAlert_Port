/**
 * Test: Search for tileset files in MIX archives
 */
#include "assets/mixfile.h"
#include "assets/assetloader.h"
#include <cstdio>
#include <cstdlib>

// Known tileset file names to search for
static const char* g_tilesetFiles[] = {
    // Snow tileset
    "SNOW.MIX", "snow.mix",
    "clear1.sno", "CLEAR1.SNO",
    "water1.sno", "WATER1.SNO",
    // Temperate tileset
    "TEMPERAT.MIX", "temperat.mix",
    "clear1.tem", "CLEAR1.TEM",
    // Interior
    "INTERIOR.MIX", "interior.mix",
    // Also check for general files
    "CONQUER.MIX", "HIRES.MIX", "LOCAL.MIX", "SOUNDS.MIX",
    nullptr
};

static MixFileHandle g_mainMix = nullptr;
static MixFileHandle g_redalertMix = nullptr;

bool TestSearchForFile(MixFileHandle mix, const char* mixName, const char* filename) {
    if (!mix) return false;
    if (Mix_FileExists(mix, filename)) {
        printf("  FOUND: %s in %s (size: %u)\n", filename, mixName, Mix_GetFileSize(mix, filename));
        return true;
    }
    return false;
}

void TestNestedMix(MixFileHandle parent, const char* parentName, const char* nestedName) {
    if (!parent) return;
    
    uint32_t size = 0;
    void* data = Mix_AllocReadFile(parent, nestedName, &size);
    if (!data) {
        printf("  Nested %s not found in %s\n", nestedName, parentName);
        return;
    }
    
    printf("  Nested %s found in %s (%u bytes)\n", nestedName, parentName, size);
    MixFileHandle nested = Mix_OpenMemory(data, size, TRUE);
    if (!nested) {
        printf("    Failed to open as MIX\n");
        free(data);
        return;
    }
    
    printf("    Contains %d files\n", Mix_GetFileCount(nested));
    
    // Search for tileset files in nested MIX
    for (int i = 0; g_tilesetFiles[i]; i++) {
        TestSearchForFile(nested, nestedName, g_tilesetFiles[i]);
    }
    
    Mix_Close(nested);
}

int main() {
    printf("=== MIX Content Search Test ===\n\n");
    
    // Open MAIN_ALLIED.MIX
    const char* mainPaths[] = {
        "/Users/jasson/workspace/CnC_Red_Alert/assets/MAIN_ALLIED.MIX",
        "../assets/MAIN_ALLIED.MIX",
        nullptr
    };
    
    for (int i = 0; mainPaths[i] && !g_mainMix; i++) {
        g_mainMix = Mix_Open(mainPaths[i]);
        if (g_mainMix) printf("Opened %s\n", mainPaths[i]);
    }
    
    // Open REDALERT.MIX
    const char* raPaths[] = {
        "/Users/jasson/workspace/CnC_Red_Alert/assets/REDALERT.MIX",
        "../assets/REDALERT.MIX",
        nullptr
    };
    
    for (int i = 0; raPaths[i] && !g_redalertMix; i++) {
        g_redalertMix = Mix_Open(raPaths[i]);
        if (g_redalertMix) printf("Opened %s\n", raPaths[i]);
    }
    
    if (!g_mainMix && !g_redalertMix) {
        printf("ERROR: No MIX files found!\n");
        return 1;
    }
    
    printf("\n--- Searching MAIN_ALLIED.MIX ---\n");
    if (g_mainMix) {
        printf("File count: %d\n", Mix_GetFileCount(g_mainMix));
        for (int i = 0; g_tilesetFiles[i]; i++) {
            TestSearchForFile(g_mainMix, "MAIN", g_tilesetFiles[i]);
        }
        
        // Try nested archives
        TestNestedMix(g_mainMix, "MAIN", "CONQUER.MIX");
        TestNestedMix(g_mainMix, "MAIN", "SNOW.MIX");
        TestNestedMix(g_mainMix, "MAIN", "TEMPERAT.MIX");
    }
    
    printf("\n--- Searching REDALERT.MIX ---\n");
    if (g_redalertMix) {
        printf("File count: %d\n", Mix_GetFileCount(g_redalertMix));
        for (int i = 0; g_tilesetFiles[i]; i++) {
            TestSearchForFile(g_redalertMix, "REDALERT", g_tilesetFiles[i]);
        }
        
        // Try nested archives
        TestNestedMix(g_redalertMix, "REDALERT", "HIRES.MIX");
        TestNestedMix(g_redalertMix, "REDALERT", "LOCAL.MIX");
        TestNestedMix(g_redalertMix, "REDALERT", "SNOW.MIX");
        TestNestedMix(g_redalertMix, "REDALERT", "TEMPERAT.MIX");
    }
    
    // Cleanup
    if (g_mainMix) Mix_Close(g_mainMix);
    if (g_redalertMix) Mix_Close(g_redalertMix);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
