/**
 * Test AUD header parsing
 * Extracts AUD files from MIX and dumps raw header bytes
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../assets/mixfile.h"
#include "../platform/asset_paths.h"

#pragma pack(push, 1)
struct AudHeader {
    uint16_t sampleRate;
    uint32_t size;          // Compressed size
    uint32_t uncompSize;    // Uncompressed size
    uint8_t  flags;         // Bit 0: stereo, Bit 1: 16-bit
    uint8_t  compression;   // 1 = Westwood, 99 = IMA ADPCM
};
#pragma pack(pop)

static void dumpBytes(const uint8_t* data, int count) {
    for (int i = 0; i < count; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (count % 16 != 0) printf("\n");
}

int main(int argc, char** argv) {
    // Find asset path using correct API
    char assetPath[512];
    if (!Assets_FindPath(assetPath, sizeof(assetPath))) {
        printf("ERROR: Can't find assets directory\n");
        return 1;
    }
    printf("Assets path: %s\n", assetPath);

    // Try to open AUD.MIX directly
    char audMixPath[512];
    snprintf(audMixPath, sizeof(audMixPath), "%s/AUD.MIX", assetPath);

    printf("Opening: %s\n", audMixPath);
    MixFileHandle audMix = Mix_Open(audMixPath);
    if (!audMix) {
        printf("ERROR: Can't open AUD.MIX\n");
        return 1;
    }
    printf("Opened AUD.MIX successfully\n");

    // List file count
    int fileCount = Mix_GetFileCount(audMix);
    printf("AUD.MIX contains %d files\n\n", fileCount);

    // Enumerate files by CRC and dump their headers
    printf("\n=== Enumerating AUD.MIX entries by index ===\n\n");

    int maxToShow = (fileCount > 10) ? 10 : fileCount;
    for (int i = 0; i < maxToShow; i++) {
        uint32_t crc = 0, size = 0;
        if (!Mix_GetEntryByIndex(audMix, i, &crc, &size)) {
            printf("Entry %d: Failed to get info\n", i);
            continue;
        }

        printf("Entry %d: CRC=0x%08X, Size=%u bytes\n", i, crc, size);

        // Try to read by CRC
        uint32_t fileSize = 0;
        void* data = Mix_AllocReadFileByCRC(audMix, crc, &fileSize);
        if (!data || fileSize < sizeof(AudHeader)) {
            printf("  Could not read data\n\n");
            continue;
        }

        const uint8_t* bytes = (const uint8_t*)data;
        const AudHeader* hdr = (const AudHeader*)data;

        printf("  Raw header bytes (first 16):\n  ");
        dumpBytes(bytes, 16);

        printf("  Parsed header:\n");
        printf("    sampleRate:   %u (0x%04X)\n", hdr->sampleRate, hdr->sampleRate);
        printf("    size:         %u (0x%08X)\n", hdr->size, hdr->size);
        printf("    uncompSize:   %u (0x%08X)\n", hdr->uncompSize, hdr->uncompSize);
        printf("    flags:        %u (stereo=%d, 16bit=%d)\n",
               hdr->flags, hdr->flags & 1, (hdr->flags >> 1) & 1);
        printf("    compression:  %u\n", hdr->compression);

        // Calculate duration
        int channels = (hdr->flags & 1) ? 2 : 1;
        int bytesPerSample = (hdr->flags & 2) ? 2 : 1;
        uint32_t sampleCount = hdr->uncompSize / bytesPerSample / channels;
        float duration = (float)sampleCount / (float)hdr->sampleRate;

        printf("    Calculated: %u samples, %.2f seconds\n\n", sampleCount, duration);

        free(data);
    }

    Mix_Close(audMix);

    return 0;
}
