/**
 * Examine TMP file contents
 */
#include "assets/mixfile.h"
#include "assets/tmpfile.h"
#include <cstdio>
#include <cstring>

int main() {
    MixFileHandle snowMix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix");
    if (!snowMix) {
        printf("ERROR: Cannot open snow.mix\n");
        return 1;
    }
    
    // Load clear1.sno
    uint32_t size = 0;
    void* data = Mix_AllocReadFile(snowMix, "clear1.sno", &size);
    
    if (!data) {
        printf("ERROR: Cannot load clear1.sno\n");
        Mix_Close(snowMix);
        return 1;
    }
    
    printf("Loaded clear1.sno: %u bytes\n", size);
    
    // Dump first 64 bytes
    printf("\nFirst 64 bytes:\n");
    uint8_t* bytes = (uint8_t*)data;
    for (uint32_t i = 0; i < 64 && i < size; i++) {
        printf("%02X ", bytes[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    
    // Examine header bytes more closely
    printf("\nHeader analysis:\n");
    uint16_t* words = (uint16_t*)bytes;
    printf("  Width: %d\n", words[0]);
    printf("  Height: %d\n", words[1]);
    printf("  TileCount: %d\n", words[2]);
    printf("  Reserved: %d %d %d %d %d\n", words[3], words[4], words[5], words[6], words[7]);
    uint32_t* dwords = (uint32_t*)(bytes + 16);
    printf("  ImgStart: %u\n", dwords[0]);
    printf("  Reserved2: %u %u\n", dwords[1], dwords[2]);
    printf("  IndexEnd: %u\n", dwords[3]);
    printf("  Reserved3: %u\n", dwords[4]);
    printf("  IndexStart: %u\n", dwords[5]);
    
    // Try to parse as TMP
    TmpFileHandle tmp = Tmp_Load(data, size);
    if (!tmp) {
        printf("\n\nERROR: Failed to parse as TMP file\n");
        free(data);
        Mix_Close(snowMix);
        return 1;
    }
    
    printf("\n\nTMP file parsed successfully!\n");
    printf("  Tile count: %d\n", Tmp_GetTileCount(tmp));
    printf("  Tile size: %dx%d\n", Tmp_GetTileWidth(tmp), Tmp_GetTileHeight(tmp));
    
    // Check first few tiles
    for (int i = 0; i < 5 && i < Tmp_GetTileCount(tmp); i++) {
        const TmpTile* tile = Tmp_GetTile(tmp, i);
        if (tile) {
            printf("  Tile %d: %dx%d pixels=%p\n", i, tile->width, tile->height, (void*)tile->pixels);
            if (tile->pixels) {
                // Print first 8 palette indices
                printf("    First 8 pixels: ");
                for (int p = 0; p < 8; p++) {
                    printf("%d ", tile->pixels[p]);
                }
                printf("\n");
            }
        }
    }
    
    Tmp_Free(tmp);
    free(data);
    Mix_Close(snowMix);
    return 0;
}
