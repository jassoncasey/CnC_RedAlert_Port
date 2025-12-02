/**
 * Red Alert macOS Port - AUD Audio File Reader Implementation
 *
 * AUD file format:
 *   Header (12 bytes):
 *     uint16_t sampleRate    - Sample rate (typically 22050)
 *     uint32_t size          - Compressed data size
 *     uint32_t uncompSize    - Uncompressed output size
 *     uint8_t  flags         - Bit 0: stereo, Bit 1: 16-bit
 *     uint8_t  compression   - 1 = Westwood, 99 = IMA ADPCM
 *
 *   Chunk format (for compression type 99):
 *     uint16_t compSize      - Compressed chunk size
 *     uint16_t uncompSize    - Uncompressed chunk size
 *     uint32_t checksum      - Data checksum (ID)
 *     uint8_t  data[]        - Compressed data
 */

#include "audfile.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma pack(push, 1)
struct AudHeader {
    uint16_t sampleRate;
    uint32_t size;          // Compressed size
    uint32_t uncompSize;    // Uncompressed size
    uint8_t  flags;         // Bit 0: stereo, Bit 1: 16-bit
    uint8_t  compression;   // 1 = Westwood, 99 = IMA ADPCM
};

struct AudChunkHeader {
    uint16_t compSize;
    uint16_t uncompSize;
    uint32_t id;
};
#pragma pack(pop)

// IMA ADPCM step table
static const int g_imaStepTable[89] = {
    7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
    19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
    50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
    130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
    876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
    2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
    5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

// IMA ADPCM index table
static const int g_imaIndexTable[16] = {
    -1, -1, -1, -1, 2, 4, 6, 8,
    -1, -1, -1, -1, 2, 4, 6, 8
};

// Decode a single IMA ADPCM nibble
static int16_t DecodeIMANibble(uint8_t nibble, int16_t* predictor,
                               int* stepIndex) {
    int step = g_imaStepTable[*stepIndex];
    int diff = step >> 3;

    if (nibble & 1) diff += step >> 2;
    if (nibble & 2) diff += step >> 1;
    if (nibble & 4) diff += step;
    if (nibble & 8) diff = -diff;

    int newPredictor = *predictor + diff;
    if (newPredictor > 32767) newPredictor = 32767;
    if (newPredictor < -32768) newPredictor = -32768;
    *predictor = (int16_t)newPredictor;

    int newIndex = *stepIndex + g_imaIndexTable[nibble];
    if (newIndex < 0) newIndex = 0;
    if (newIndex > 88) newIndex = 88;
    *stepIndex = newIndex;

    return *predictor;
}

// Decode IMA ADPCM chunk
static int DecodeIMAChunk(const uint8_t* src, int srcSize, int16_t* dst,
                          int dstSize, int16_t* predictor, int* stepIndex) {
    int samplesWritten = 0;
    int maxSamples = dstSize / sizeof(int16_t);

    for (int i = 0; i < srcSize && samplesWritten < maxSamples; i++) {
        uint8_t byte = src[i];

        // Low nibble first
        if (samplesWritten < maxSamples) {
            int16_t s = DecodeIMANibble(byte & 0x0F, predictor, stepIndex);
            dst[samplesWritten++] = s;
        }
        // High nibble
        if (samplesWritten < maxSamples) {
            uint8_t hi = (byte >> 4) & 0x0F;
            dst[samplesWritten++] = DecodeIMANibble(hi, predictor, stepIndex);
        }
    }

    return samplesWritten * sizeof(int16_t);
}

// Decode Westwood ADPCM (type 1) - based on XCC reference implementation
// Mode 0: 2-bit deltas (4 samples per byte)
// Mode 1: 4-bit deltas (2 samples per byte)
// Mode 2: Raw or 5-bit signed delta
// Mode 3: RLE repeat
static const int g_wsStepTable2[] = {-2, -1, 0, 1};
static const int g_wsStepTable4[] = {
    -9, -8, -6, -5, -4, -3, -2, -1,
     0,  1,  2,  3,  4,  5,  6,  8
};

static int DecodeWestwoodChunk(const uint8_t* src, int srcSize,
                               int16_t* dst, int maxSamples) {
    int samplesWritten = 0;
    int sample = 0x80;  // 8-bit center value
    const uint8_t* srcEnd = src + srcSize;

    while (src < srcEnd && samplesWritten < maxSamples) {
        uint8_t cmd = *src++;
        int count = cmd & 0x3F;
        int mode = cmd >> 6;

        switch (mode) {
        case 0:  // 2-bit deltas: 4 samples per byte
            for (int i = 0; i <= count && src < srcEnd &&
                 samplesWritten < maxSamples; i++) {
                uint8_t code = *src++;
                for (int j = 0; j < 4 && samplesWritten < maxSamples; j++) {
                    sample += g_wsStepTable2[(code >> (j * 2)) & 3];
                    if (sample < 0) sample = 0;
                    if (sample > 255) sample = 255;
                    dst[samplesWritten++] = (int16_t)((sample - 128) << 8);
                }
            }
            break;

        case 1:  // 4-bit deltas: 2 samples per byte
            for (int i = 0; i <= count && src < srcEnd &&
                 samplesWritten < maxSamples; i++) {
                uint8_t code = *src++;
                // Low nibble
                sample += g_wsStepTable4[code & 0x0F];
                if (sample < 0) sample = 0;
                if (sample > 255) sample = 255;
                dst[samplesWritten++] = (int16_t)((sample - 128) << 8);
                // High nibble
                if (samplesWritten < maxSamples) {
                    sample += g_wsStepTable4[code >> 4];
                    if (sample < 0) sample = 0;
                    if (sample > 255) sample = 255;
                    dst[samplesWritten++] = (int16_t)((sample - 128) << 8);
                }
            }
            break;

        case 2:  // Raw samples or 5-bit signed delta
            if (count & 0x20) {
                // 5-bit signed delta (sign-extend from 6 bits)
                int delta = (int8_t)(cmd << 2) >> 2;
                sample += delta;
                if (sample < 0) sample = 0;
                if (sample > 255) sample = 255;
                dst[samplesWritten++] = (int16_t)((sample - 128) << 8);
            } else {
                // Raw samples
                count++;
                while (count > 0 && src < srcEnd &&
                       samplesWritten < maxSamples) {
                    sample = *src++;
                    dst[samplesWritten++] = (int16_t)((sample - 128) << 8);
                    count--;
                }
            }
            break;

        case 3:  // RLE repeat
            count++;
            while (count > 0 && samplesWritten < maxSamples) {
                dst[samplesWritten++] = (int16_t)((sample - 128) << 8);
                count--;
            }
            break;
        }
    }

    return samplesWritten;
}

AudData* Aud_Load(const void* data, uint32_t dataSize) {
    if (!data || dataSize < sizeof(AudHeader)) {
        return nullptr;
    }

    const uint8_t* bytes = (const uint8_t*)data;
    const AudHeader* header = (const AudHeader*)bytes;

    // Sanity checks
    if (header->sampleRate < 4000 || header->sampleRate > 48000) {
        return nullptr;
    }
    // 50MB max
    if (header->uncompSize == 0 || header->uncompSize > 50 * 1024 * 1024) {
        return nullptr;
    }
    if (header->compression != 1 && header->compression != 99) {
        // Unsupported compression
        return nullptr;
    }

    // Determine format
    bool stereo = (header->flags & 0x01) != 0;
    bool is16bit = (header->flags & 0x02) != 0;
    int channels = stereo ? 2 : 1;

    // Calculate sample count
    uint32_t bytesPerSample = is16bit ? 2 : 1;
    uint32_t sampleCount = header->uncompSize / bytesPerSample / channels;

    // Allocate output
    AudData* aud = new AudData;
    aud->sampleRate = header->sampleRate;
    aud->channels = (uint8_t)channels;
    aud->sampleCount = sampleCount;
    aud->samples = new int16_t[sampleCount * channels];
    memset(aud->samples, 0, sampleCount * channels * sizeof(int16_t));

    const uint8_t* srcPtr = bytes + sizeof(AudHeader);
    const uint8_t* srcEnd = bytes + dataSize;
    int16_t* dstPtr = aud->samples;
    int samplesRemaining = sampleCount * channels;

    if (header->compression == 99) {
        // IMA ADPCM with chunks
        int16_t predictor = 0;
        int stepIndex = 0;

        size_t chunkSz = sizeof(AudChunkHeader);
        while (srcPtr + chunkSz <= srcEnd && samplesRemaining > 0) {
            const AudChunkHeader* chunk = (const AudChunkHeader*)srcPtr;
            srcPtr += sizeof(AudChunkHeader);

            if (srcPtr + chunk->compSize > srcEnd) break;

            int decoded = DecodeIMAChunk(srcPtr, chunk->compSize, dstPtr,
                                         samplesRemaining * sizeof(int16_t),
                                         &predictor, &stepIndex);
            int samplesDecoded = decoded / sizeof(int16_t);
            dstPtr += samplesDecoded;
            samplesRemaining -= samplesDecoded;
            srcPtr += chunk->compSize;
        }
    } else if (header->compression == 1) {
        // Westwood ADPCM - simpler format
        uint32_t compSize = header->size;
        if (srcPtr + compSize > srcEnd) {
            compSize = (uint32_t)(srcEnd - srcPtr);
        }

        int decoded = DecodeWestwoodChunk(srcPtr, compSize, dstPtr,
                                          samplesRemaining);
        (void)decoded;
    }

    return aud;
}

AudData* Aud_LoadFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return nullptr;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 50 * 1024 * 1024) { // 50MB max
        fclose(f);
        return nullptr;
    }

    uint8_t* data = new uint8_t[size];
    size_t bytesRead = fread(data, 1, size, f);
    fclose(f);

    if (bytesRead != (size_t)size) {
        delete[] data;
        return nullptr;
    }

    AudData* aud = Aud_Load(data, (uint32_t)size);
    delete[] data;

    return aud;
}

void Aud_Free(AudData* aud) {
    if (aud) {
        if (aud->samples) {
            delete[] aud->samples;
        }
        delete aud;
    }
}

BOOL Aud_ConvertTo8Bit(const AudData* aud, uint8_t** outData,
                       uint32_t* outSize, uint32_t* outRate) {
    if (!aud || !outData || !outSize || !outRate) {
        return FALSE;
    }

    // Convert 16-bit signed to 8-bit unsigned
    uint32_t totalSamples = aud->sampleCount * aud->channels;
    uint8_t* data = new uint8_t[totalSamples];

    for (uint32_t i = 0; i < totalSamples; i++) {
        // Convert [-32768, 32767] to [0, 255]
        int sample = (aud->samples[i] + 32768) >> 8;
        if (sample < 0) sample = 0;
        if (sample > 255) sample = 255;
        data[i] = (uint8_t)sample;
    }

    *outData = data;
    *outSize = totalSamples;
    *outRate = aud->sampleRate;

    return TRUE;
}
