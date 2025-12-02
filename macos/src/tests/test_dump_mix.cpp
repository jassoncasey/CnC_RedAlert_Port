/**
 * Dump first 50 entries from conquer.mix to see what's there
 */
#include <cstdio>
#include <cstdint>

#pragma pack(push, 1)
struct MixHeader {
    int16_t count;
    int32_t dataSize;
};

struct MixEntry {
    uint32_t crc;
    uint32_t offset;
    uint32_t size;
};
#pragma pack(pop)

int main() {
    FILE* f = fopen("/Users/jasson/workspace/CnC_Red_Alert/assets/conquer.mix", "rb");
    if (!f) {
        printf("ERROR: Cannot open conquer.mix\n");
        return 1;
    }
    
    // Check first word - if 0, it's RA format
    uint16_t firstWord;
    fread(&firstWord, 2, 1, f);
    
    long headerOffset = 0;
    if (firstWord == 0) {
        // RA format - skip 2 more bytes (flags)
        headerOffset = 4;
    }
    
    fseek(f, headerOffset, SEEK_SET);
    
    MixHeader header;
    fread(&header, sizeof(header), 1, f);
    
    printf("conquer.mix: %d files, data size: %d\n\n", header.count, header.dataSize);
    
    // Read all entries
    MixEntry* entries = new MixEntry[header.count];
    fread(entries, sizeof(MixEntry) * header.count, 1, f);
    
    // Sort by CRC and print first 50
    printf("First 50 entries (sorted by CRC):\n");
    printf("%-12s  %-8s  %-8s\n", "CRC", "Offset", "Size");
    for (int i = 0; i < 50 && i < header.count; i++) {
        printf("0x%08X  %8u  %8u\n", entries[i].crc, entries[i].offset, entries[i].size);
    }
    
    // Look for specific CRCs
    printf("\nSearching for target CRCs:\n");
    uint32_t targets[] = {
        0x5CD6E8D5,  // apc.shp
        0x02F0F7B0,  // arty.shp
        0xF4F6F1CE,  // powr.shp
        0xF0CADDDC,  // weap.shp
        0xE6E4FB90,  // 1tnk.shp (we know this exists)
        0
    };
    const char* names[] = {"apc.shp", "arty.shp", "powr.shp", "weap.shp", "1tnk.shp", nullptr};
    
    for (int t = 0; targets[t]; t++) {
        bool found = false;
        for (int i = 0; i < header.count; i++) {
            if (entries[i].crc == targets[t]) {
                printf("  %s (0x%08X): FOUND at index %d, offset %u, size %u\n",
                       names[t], targets[t], i, entries[i].offset, entries[i].size);
                found = true;
                break;
            }
        }
        if (!found) {
            printf("  %s (0x%08X): NOT FOUND\n", names[t], targets[t]);
        }
    }
    
    delete[] entries;
    fclose(f);
    return 0;
}
