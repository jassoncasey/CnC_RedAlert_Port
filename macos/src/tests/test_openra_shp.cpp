/**
 * Test loading OpenRA bits SHP files directly
 */
#include "assets/shpfile.h"
#include <cstdio>
#include <cstdlib>

int main() {
    printf("=== Testing OpenRA bits SHP files ===\n\n");

    // Read fact.shp directly from file
    FILE* f = fopen("/Users/jasson/workspace/CnC_Red_Alert/OpenRA/mods/ra/bits/fact.shp", "rb");
    if (!f) {
        printf("ERROR: Cannot open fact.shp\n");
        return 1;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    void* data = malloc(size);
    fread(data, 1, size, f);
    fclose(f);
    
    printf("fact.shp: %ld bytes\n", size);
    
    // Try to load it
    ShpFileHandle shp = Shp_Load(data, size);
    if (shp) {
        printf("  Loaded! %d frames\n", Shp_GetFrameCount(shp));
        const ShpFrame* frame = Shp_GetFrame(shp, 0);
        if (frame) {
            printf("  Frame 0: %dx%d\n", frame->width, frame->height);
        }
        Shp_Free(shp);
    } else {
        printf("  FAILED to load\n");
        // Print first bytes for debugging
        uint8_t* bytes = (uint8_t*)data;
        printf("  First 16 bytes: ");
        for (int i = 0; i < 16; i++) printf("%02X ", bytes[i]);
        printf("\n");
    }
    
    free(data);

    // Try harv.shp
    f = fopen("/Users/jasson/workspace/CnC_Red_Alert/OpenRA/mods/ra/bits/harv.shp", "rb");
    if (f) {
        fseek(f, 0, SEEK_END);
        size = ftell(f);
        fseek(f, 0, SEEK_SET);
        data = malloc(size);
        fread(data, 1, size, f);
        fclose(f);
        
        printf("\nharv.shp: %ld bytes\n", size);
        shp = Shp_Load(data, size);
        if (shp) {
            printf("  Loaded! %d frames\n", Shp_GetFrameCount(shp));
            Shp_Free(shp);
        } else {
            printf("  FAILED to load\n");
        }
        free(data);
    }

    return 0;
}
