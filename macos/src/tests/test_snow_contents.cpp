/**
 * Dump all entries from snow.mix to see what terrain files exist
 */
#include <cstdio>
#include <cstdint>

#pragma pack(push, 1)
struct MixHeader { int16_t count; int32_t dataSize; };
struct MixEntry { uint32_t crc; uint32_t offset; uint32_t size; };
#pragma pack(pop)

int main() {
    FILE* f = fopen("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix", "rb");
    if (!f) {
        printf("ERROR: Cannot open snow.mix\n");
        return 1;
    }
    
    // Check first word - if 0, it's RA format
    uint16_t firstWord;
    fread(&firstWord, 2, 1, f);
    
    long headerOffset = (firstWord == 0) ? 4 : 0;
    fseek(f, headerOffset, SEEK_SET);
    
    MixHeader header;
    fread(&header, sizeof(header), 1, f);
    
    printf("snow.mix: %d files, data size: %d\n\n", header.count, header.dataSize);
    
    MixEntry* entries = new MixEntry[header.count];
    fread(entries, sizeof(MixEntry) * header.count, 1, f);
    
    printf("Entries by size (looking for terrain-sized files - ~576 bytes for 24x24):\n");
    printf("%-12s  %-8s  %-8s\n", "CRC", "Offset", "Size");
    
    for (int i = 0; i < header.count; i++) {
        // Terrain tiles are typically small - 576 bytes = 24*24 pixels
        // TMP headers add some overhead
        if (entries[i].size > 50 && entries[i].size < 20000) {
            printf("0x%08X  %8u  %8u\n", entries[i].crc, entries[i].offset, entries[i].size);
        }
    }
    
    // Also list files by size ranges
    printf("\n=== File count by size range ===\n");
    int tiny = 0, small = 0, medium = 0, large = 0;
    for (int i = 0; i < header.count; i++) {
        if (entries[i].size < 100) tiny++;
        else if (entries[i].size < 1000) small++;
        else if (entries[i].size < 10000) medium++;
        else large++;
    }
    printf("Tiny (<100): %d\n", tiny);
    printf("Small (100-1000): %d\n", small);
    printf("Medium (1000-10000): %d\n", medium);
    printf("Large (>10000): %d\n", large);
    
    delete[] entries;
    fclose(f);
    return 0;
}
