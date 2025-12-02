/**
 * Test loading terrain templates from snow.mix
 */
#include "assets/mixfile.h"
#include <cstdio>

const char* templates[] = {
    "clear1.sno", "CLEAR1.SNO",
    "w1.sno", "W1.SNO",
    "w2.sno", "W2.SNO",
    "sh01.sno", "SH01.SNO",
    "sh02.sno", "SH02.SNO",
    "sh03.sno", "SH03.SNO",
    "sh04.sno", "SH04.SNO",
    "d01.sno", "D01.SNO",   // Debris
    "d02.sno", "D02.SNO",
    "rv01.sno", "RV01.SNO", // River
    "br1.sno", "BR1.SNO",   // Bridge
    "s01.sno", "S01.SNO",   // Roads/cliffs
    nullptr
};

int main() {
    MixFileHandle snowMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix");
    if (!snowMix) {
        printf("ERROR: Cannot open snow.mix\n");
        return 1;
    }
    
    printf("snow.mix opened (%d files)\n\n", Mix_GetFileCount(snowMix));
    printf("Testing terrain template names:\n");
    
    for (int i = 0; templates[i]; i++) {
        uint32_t crc = Mix_CalculateCRC(templates[i]);
        bool found = Mix_FileExists(snowMix, templates[i]);
        uint32_t size = found ? Mix_GetFileSize(snowMix, templates[i]) : 0;
        
        printf("  %-14s CRC=0x%08X  %s", templates[i], crc, found ? "FOUND" : "NOT FOUND");
        if (found) printf(" (%u bytes)", size);
        printf("\n");
    }
    
    Mix_Close(snowMix);
    return 0;
}
