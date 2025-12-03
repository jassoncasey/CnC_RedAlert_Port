/**
 * Test specific bad AUD files
 * Examines raw header bytes and parsed values
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "../assets/mixfile.h"
#include "../assets/audfile.h"
#include "../platform/asset_paths.h"

static void dumpBytes(const uint8_t* data, int count) {
    for (int i = 0; i < count; i++) {
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
    if (count % 16 != 0) printf("\n");
}

#pragma pack(push, 1)
struct AudHeader {
    uint16_t sampleRate;
    uint32_t size;          // Compressed size
    uint32_t uncompSize;    // Uncompressed size
    uint8_t  flags;         // Bit 0: stereo, Bit 1: 16-bit
    uint8_t  compression;   // 1 = Westwood, 99 = IMA ADPCM
};
#pragma pack(pop)

static void analyzeAUD(const char* name, const uint8_t* data, uint32_t fileSize) {
    printf("\n=== %s ===\n", name);
    printf("File size: %u bytes\n", fileSize);
    
    if (fileSize < sizeof(AudHeader)) {
        printf("ERROR: File too small for header\n");
        return;
    }
    
    printf("Raw header bytes (first 32):\n");
    dumpBytes(data, fileSize < 32 ? fileSize : 32);
    
    const AudHeader* hdr = (const AudHeader*)data;
    
    printf("\nParsed header:\n");
    printf("  sampleRate:   %u (0x%04X)\n", hdr->sampleRate, hdr->sampleRate);
    printf("  size:         %u (0x%08X)\n", hdr->size, hdr->size);
    printf("  uncompSize:   %u (0x%08X)\n", hdr->uncompSize, hdr->uncompSize);
    printf("  flags:        %u (stereo=%d, 16bit=%d)\n",
           hdr->flags, hdr->flags & 1, (hdr->flags >> 1) & 1);
    printf("  compression:  %u", hdr->compression);
    if (hdr->compression == 1) printf(" (Westwood ADPCM)");
    else if (hdr->compression == 99) printf(" (IMA ADPCM)");
    printf("\n");
    
    // Calculate expected values
    int channels = (hdr->flags & 1) ? 2 : 1;
    int bytesPerSample = (hdr->flags & 2) ? 2 : 1;
    uint32_t sampleCount = hdr->uncompSize / bytesPerSample / channels;
    float duration = (float)sampleCount / (float)hdr->sampleRate;
    
    printf("\nCalculated:\n");
    printf("  channels:     %d\n", channels);
    printf("  bytesPerSamp: %d\n", bytesPerSample);
    printf("  sampleCount:  %u\n", sampleCount);
    printf("  duration:     %.2f seconds\n", duration);
    
    // Try to decode with our decoder
    printf("\n--- Testing Aud_Load ---\n");
    AudData* aud = Aud_Load(data, fileSize);
    if (aud) {
        float decDuration = (float)aud->sampleCount / (float)aud->sampleRate;
        printf("  Decoded OK:\n");
        printf("    sampleCount: %u\n", aud->sampleCount);
        printf("    sampleRate:  %u\n", aud->sampleRate);
        printf("    channels:    %d\n", aud->channels);
        printf("    duration:    %.2f seconds\n", decDuration);
        
        // Check if samples look reasonable (non-zero)
        int nonZero = 0;
        for (uint32_t i = 0; i < aud->sampleCount && i < 1000; i++) {
            if (aud->samples[i] != 0) nonZero++;
        }
        printf("    nonZero/1000: %d (%.1f%%)\n", nonZero, nonZero / 10.0f);
        
        Aud_Free(aud);
    } else {
        printf("  DECODE FAILED\n");
    }
}

int main() {
    char assetPath[512];
    if (!Assets_FindPath(assetPath, sizeof(assetPath))) {
        printf("ERROR: Can't find assets directory\n");
        return 1;
    }
    printf("Assets path: %s\n", assetPath);

    // Open MAIN_ALLIED.MIX (same as cd1_main.mix, just different name)
    char mainMixPath[512];
    snprintf(mainMixPath, sizeof(mainMixPath), "%s/MAIN_ALLIED.MIX", assetPath);
    
    MixFileHandle mainMix = Mix_Open(mainMixPath);
    if (!mainMix) {
        printf("ERROR: Can't open %s\n", mainMixPath);
        return 1;
    }
    printf("Opened: %s\n", mainMixPath);
    
    // Test files in SOUNDS.MIX
    uint32_t soundsSize = 0;
    void* soundsData = Mix_AllocReadFile(mainMix, "SOUNDS.MIX", &soundsSize);
    if (soundsData) {
        printf("\nOpening SOUNDS.MIX (%u bytes)\n", soundsSize);
        MixFileHandle soundsMix = Mix_OpenMemory((uint8_t*)soundsData, soundsSize, FALSE);
        if (soundsMix) {
            // GRENADE1.AUD
            uint32_t audSize = 0;
            void* audData = Mix_AllocReadFile(soundsMix, "GRENADE1.AUD", &audSize);
            if (audData) {
                analyzeAUD("SOUNDS.MIX::GRENADE1.AUD", (uint8_t*)audData, audSize);
                free(audData);
            } else {
                printf("Could not read GRENADE1.AUD from SOUNDS.MIX\n");
            }
            
            // DOGW5.AUD
            audData = Mix_AllocReadFile(soundsMix, "DOGW5.AUD", &audSize);
            if (audData) {
                analyzeAUD("SOUNDS.MIX::DOGW5.AUD", (uint8_t*)audData, audSize);
                free(audData);
            } else {
                printf("Could not read DOGW5.AUD from SOUNDS.MIX\n");
            }
            
            Mix_Close(soundsMix);
        }
        free(soundsData);
    } else {
        printf("Could not read SOUNDS.MIX\n");
    }
    
    // Test files in SCORES.MIX
    uint32_t scoresSize = 0;
    void* scoresData = Mix_AllocReadFile(mainMix, "SCORES.MIX", &scoresSize);
    if (scoresData) {
        printf("\nOpening SCORES.MIX (%u bytes)\n", scoresSize);
        MixFileHandle scoresMix = Mix_OpenMemory((uint8_t*)scoresData, scoresSize, FALSE);
        if (scoresMix) {
            // FAC1226M.AUD
            uint32_t audSize = 0;
            void* audData = Mix_AllocReadFile(scoresMix, "FAC1226M.AUD", &audSize);
            if (audData) {
                analyzeAUD("SCORES.MIX::FAC1226M.AUD", (uint8_t*)audData, audSize);
                free(audData);
            } else {
                printf("Could not read FAC1226M.AUD from SCORES.MIX\n");
            }
            
            // FAC2226M.AUD
            audData = Mix_AllocReadFile(scoresMix, "FAC2226M.AUD", &audSize);
            if (audData) {
                analyzeAUD("SCORES.MIX::FAC2226M.AUD", (uint8_t*)audData, audSize);
                free(audData);
            } else {
                printf("Could not read FAC2226M.AUD from SCORES.MIX\n");
            }
            
            Mix_Close(scoresMix);
        }
        free(scoresData);
    } else {
        printf("Could not read SCORES.MIX\n");
    }
    
    Mix_Close(mainMix);
    
    return 0;
}
