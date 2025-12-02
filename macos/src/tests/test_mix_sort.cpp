/**
 * Check MIX sort order
 */
#include <cstdio>
#include <cstdint>
#include <algorithm>

#pragma pack(push, 1)
struct MixHeader { int16_t count; int32_t dataSize; };
struct MixEntry { uint32_t crc; uint32_t offset; uint32_t size; };
#pragma pack(pop)

int main() {
    FILE* f = fopen("/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix", "rb");
    if (!f) return 1;
    
    uint16_t firstWord;
    fread(&firstWord, 2, 1, f);
    long headerOffset = (firstWord == 0) ? 4 : 0;
    fseek(f, headerOffset, SEEK_SET);
    
    MixHeader header;
    fread(&header, sizeof(header), 1, f);
    
    MixEntry* entries = new MixEntry[header.count];
    fread(entries, sizeof(MixEntry) * header.count, 1, f);
    
    printf("File count: %d\n\n", header.count);
    
    // Check if sorted by unsigned or signed
    bool sortedUnsigned = true;
    bool sortedSigned = true;
    
    for (int i = 1; i < header.count; i++) {
        if (entries[i].crc < entries[i-1].crc) sortedUnsigned = false;
        if ((int32_t)entries[i].crc < (int32_t)entries[i-1].crc) sortedSigned = false;
    }
    
    printf("Sorted by unsigned CRC: %s\n", sortedUnsigned ? "YES" : "NO");
    printf("Sorted by signed CRC: %s\n", sortedSigned ? "YES" : "NO");
    
    // Print CRCs around index 0, 100, 200 to see pattern
    printf("\nCRCs at various indices:\n");
    int indices[] = {0, 50, 100, 150, 200, header.count-1};
    for (int i : indices) {
        if (i < header.count) {
            printf("  [%3d] 0x%08X (signed: %d)\n", i, entries[i].crc, (int32_t)entries[i].crc);
        }
    }
    
    // Linear search for our target
    printf("\nLinear search for apc.shp (0x5CD6E8D5):\n");
    uint32_t target = 0x5CD6E8D5;
    for (int i = 0; i < header.count; i++) {
        if (entries[i].crc == target) {
            printf("  Found at index %d!\n", i);
        }
    }
    
    delete[] entries;
    fclose(f);
    return 0;
}
