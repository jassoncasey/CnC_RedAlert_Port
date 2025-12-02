/**
 * Trace: Step through Mix_Open
 */
#include "assets/mixfile.h"
#include <cstdio>

// Direct re-implementation for tracing
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
    printf("=== Tracing Mix_Open ===\n");
    
    FILE* f = fopen("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix", "rb");
    if (!f) {
        printf("ERROR: Could not open file\n");
        return 1;
    }
    
    printf("File opened\n");
    
    // Step 1: Read first 2 bytes
    uint16_t firstWord16;
    if (fread(&firstWord16, sizeof(firstWord16), 1, f) != 1) {
        printf("ERROR: Could not read first word\n");
        fclose(f);
        return 1;
    }
    printf("1. firstWord16 = 0x%04x\n", firstWord16);
    
    bool isCncMix = (firstWord16 != 0);
    long headerOffset = 0;
    
    printf("2. isCncMix = %s\n", isCncMix ? "true" : "false");
    
    if (!isCncMix) {
        // Read flags
        uint16_t flags;
        if (fread(&flags, sizeof(flags), 1, f) != 1) {
            printf("ERROR: Could not read flags\n");
            fclose(f);
            return 1;
        }
        printf("3. flags = 0x%04x\n", flags);
        
        bool isEncrypted = (flags & 0x2) != 0;
        printf("4. isEncrypted = %s\n", isEncrypted ? "true" : "false");
        
        if (!isEncrypted) {
            headerOffset = 4;
        }
    }
    
    printf("5. headerOffset = %ld\n", headerOffset);
    
    // Seek to header position
    if (fseek(f, headerOffset, SEEK_SET) != 0) {
        printf("ERROR: fseek failed\n");
        fclose(f);
        return 1;
    }
    printf("6. Seeked to offset %ld\n", headerOffset);
    
    // Read header
    MixHeader header;
    if (fread(&header, sizeof(MixHeader), 1, f) != 1) {
        printf("ERROR: Could not read header\n");
        fclose(f);
        return 1;
    }
    printf("7. header.count = %d, header.dataSize = %d\n", header.count, header.dataSize);
    
    // Sanity check
    if (header.count < 0 || header.count > 10000) {
        printf("ERROR: Invalid count\n");
        fclose(f);
        return 1;
    }
    printf("8. Count is valid\n");
    
    // Read index
    size_t indexSize = header.count * sizeof(MixEntry);
    printf("9. Index size = %zu bytes\n", indexSize);
    
    MixEntry* entries = new MixEntry[header.count];
    size_t read = fread(entries, indexSize, 1, f);
    printf("10. fread returned %zu (expected 1)\n", read);
    
    if (read != 1) {
        printf("ERROR: Could not read index\n");
        delete[] entries;
        fclose(f);
        return 1;
    }
    
    printf("11. Index read successfully\n");
    printf("12. First entry: crc=0x%08x, offset=%u, size=%u\n",
           entries[0].crc, entries[0].offset, entries[0].size);
    
    delete[] entries;
    fclose(f);
    
    // Now try actual Mix_Open
    printf("\n=== Testing actual Mix_Open ===\n");
    MixFileHandle mix = Mix_Open("/Users/jasson/workspace/CnC_Red_Alert/assets/snow.mix");
    if (mix) {
        printf("SUCCESS: Mix_Open returned valid handle\n");
        printf("File count: %d\n", Mix_GetFileCount(mix));
        
        // Test file lookup
        if (Mix_FileExists(mix, "clear1.sno")) {
            printf("Found clear1.sno: %u bytes\n", Mix_GetFileSize(mix, "clear1.sno"));
        } else {
            printf("clear1.sno NOT FOUND\n");
        }
        
        Mix_Close(mix);
    } else {
        printf("FAILURE: Mix_Open returned nullptr\n");
    }
    
    printf("\n=== Done ===\n");
    return 0;
}
