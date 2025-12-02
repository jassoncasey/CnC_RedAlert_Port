/**
 * Debug: Trace SNOW.MIX opening
 */
#include <cstdio>
#include <cstdint>
#include <cstdlib>

// MIX file header (6 bytes)
#pragma pack(push, 1)
struct MixHeader {
    int16_t count;      // Number of files
    int32_t dataSize;   // Total data size
};
#pragma pack(pop)

int main() {
    printf("=== Debug snow.mix ===\n");
    
    FILE* f = fopen("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix", "rb");
    if (!f) {
        printf("ERROR: Could not open file\n");
        return 1;
    }
    
    // Read first 10 bytes
    uint8_t header[10];
    fread(header, 1, 10, f);
    printf("First 10 bytes: ");
    for (int i = 0; i < 10; i++) printf("%02x ", header[i]);
    printf("\n");
    
    // OpenRA format detection
    uint16_t firstWord16 = header[0] | (header[1] << 8);
    printf("firstWord16 = 0x%04x (%d)\n", firstWord16, firstWord16);
    printf("isCncMix = %s\n", (firstWord16 != 0) ? "true" : "false");
    
    if (firstWord16 == 0) {
        uint16_t flags = header[2] | (header[3] << 8);
        printf("flags = 0x%04x\n", flags);
        printf("isEncrypted = %s\n", (flags & 0x2) ? "true" : "false");
        
        // Header should start at offset 4
        printf("\nHeader at offset 4:\n");
        int16_t count = (int16_t)(header[4] | (header[5] << 8));
        int32_t dataSize = header[6] | (header[7] << 8) | (header[8] << 16) | (header[9] << 24);
        printf("  count = %d\n", count);
        printf("  dataSize = %d\n", dataSize);
    }
    
    fclose(f);
    printf("\n=== Done ===\n");
    return 0;
}
