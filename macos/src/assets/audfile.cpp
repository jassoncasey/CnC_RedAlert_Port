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
static int16_t DecodeIMANibble(uint8_t nibble, int16_t* predictor, int* stepIndex) {
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
static int DecodeIMAChunk(const uint8_t* src, int srcSize, int16_t* dst, int dstSize, int16_t* predictor, int* stepIndex) {
    int samplesWritten = 0;
    int maxSamples = dstSize / sizeof(int16_t);

    for (int i = 0; i < srcSize && samplesWritten < maxSamples; i++) {
        uint8_t byte = src[i];

        // Low nibble first
        if (samplesWritten < maxSamples) {
            dst[samplesWritten++] = DecodeIMANibble(byte & 0x0F, predictor, stepIndex);
        }
        // High nibble
        if (samplesWritten < maxSamples) {
            dst[samplesWritten++] = DecodeIMANibble((byte >> 4) & 0x0F, predictor, stepIndex);
        }
    }

    return samplesWritten * sizeof(int16_t);
}

// Decode Westwood ADPCM (type 1) - simpler 4-bit ADPCM
static const int8_t g_wsIndexAdj[8] = { -1, -1, -1, -1, 2, 4, 6, 8 };
static const int16_t g_wsStepSize[4] = { 4, 2, 1, 1 };

static int DecodeWestwoodChunk(const uint8_t* src, int srcSize, int16_t* dst, int maxSamples) {
    int samplesWritten = 0;
    int16_t predictor = 0;
    int step = 0;

    for (int i = 0; i < srcSize && samplesWritten < maxSamples; i++) {
        uint8_t byte = src[i];

        for (int nibbleIdx = 0; nibbleIdx < 2 && samplesWritten < maxSamples; nibbleIdx++) {
            uint8_t nibble = (nibbleIdx == 0) ? (byte & 0x0F) : ((byte >> 4) & 0x0F);

            int diff = (nibble & 0x07) * g_wsStepSize[step & 3];
            if (nibble & 0x08) diff = -diff;

            predictor += (int16_t)diff;
            if (predictor > 32767) predictor = 32767;
            if (predictor < -32768) predictor = -32768;

            dst[samplesWritten++] = predictor;

            step += g_wsIndexAdj[nibble & 0x07];
            if (step < 0) step = 0;
            if (step > 3) step = 3;
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
    if (header->uncompSize == 0 || header->uncompSize > 50 * 1024 * 1024) { // 50MB max
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

        while (srcPtr + sizeof(AudChunkHeader) <= srcEnd && samplesRemaining > 0) {
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

        int decoded = DecodeWestwoodChunk(srcPtr, compSize, dstPtr, samplesRemaining);
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

BOOL Aud_ConvertTo8Bit(const AudData* aud, uint8_t** outData, uint32_t* outSize, uint32_t* outRate) {
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
