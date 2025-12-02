/**
 * Debug hash calculation
 */
#include "assets/mixfile.h"
#include <cstdio>
#include <cstring>
#include <cctype>

// Reference implementation from OpenRA
uint32_t OpenRAHash(const char* name) {
    int len = strlen(name);
    int padding = (len % 4 != 0) ? (4 - len % 4) : 0;
    int paddedLen = len + padding;
    
    char buffer[256];
    for (int i = 0; i < len; i++) {
        buffer[i] = toupper(name[i]);
    }
    for (int i = len; i < paddedLen; i++) {
        buffer[i] = '\0';
    }
    
    uint32_t result = 0;
    for (int i = 0; i < paddedLen; i += 4) {
        uint32_t val = (uint8_t)buffer[i] |
                       ((uint32_t)(uint8_t)buffer[i + 1] << 8) |
                       ((uint32_t)(uint8_t)buffer[i + 2] << 16) |
                       ((uint32_t)(uint8_t)buffer[i + 3] << 24);
        result = ((result << 1) | (result >> 31)) + val;
    }
    return result;
}

int main() {
    const char* names[] = {
        "apc.shp", "APC.SHP",
        "arty.shp", "ARTY.SHP",
        "powr.shp", "POWR.SHP",
        "weap.shp", "WEAP.SHP",
        "pbox.shp", "PBOX.SHP",
        "harv.shp", "HARV.SHP",  // We know this works from bits
        "fact.shp", "FACT.SHP",  // We know this works from bits
        "1tnk.shp", "1TNK.SHP",  // This works from MIX
        nullptr
    };
    
    printf("=== Hash comparison ===\n");
    printf("%-12s  %-12s  %-12s  %s\n", "Name", "Mix_CRC", "OpenRA", "Match?");
    
    for (int i = 0; names[i]; i++) {
        uint32_t mixCrc = Mix_CalculateCRC(names[i]);
        uint32_t oraCrc = OpenRAHash(names[i]);
        printf("%-12s  0x%08X  0x%08X  %s\n", names[i], mixCrc, oraCrc, 
               mixCrc == oraCrc ? "YES" : "NO");
    }

    return 0;
}
