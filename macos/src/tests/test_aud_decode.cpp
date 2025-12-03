/**
 * Test AUD decoding - extracts an AUD from SCORES.MIX and outputs WAV
 */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

// AUD header structure
#pragma pack(push, 1)
struct AUDHeader {
    uint16_t sampleRate;
    uint32_t size;          // Compressed size
    uint32_t uncompSize;    // Uncompressed size
    uint8_t  flags;         // Bit 0: stereo, Bit 1: 16-bit
    uint8_t  compression;   // 1 = Westwood, 99 = IMA ADPCM
};

struct AUDChunkHeader {
    uint16_t compSize;
    uint16_t uncompSize;
    uint32_t id;
};
#pragma pack(pop)

// IMA ADPCM tables
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

static const int g_imaIndexTable[8] = {-1, -1, -1, -1, 2, 4, 6, 8};

// Decode IMA ADPCM using XCC algorithm
int DecodeIMA_XCC(const uint8_t* fileData, uint32_t fileSize,
                  int16_t** outSamples, uint32_t* outSampleCount) {
    if (fileSize < sizeof(AUDHeader)) return -1;

    const AUDHeader* hdr = (const AUDHeader*)fileData;
    printf("  Sample rate: %d Hz\n", hdr->sampleRate);
    printf("  Compressed size: %u\n", hdr->size);
    printf("  Uncompressed size: %u\n", hdr->uncompSize);
    printf("  Flags: 0x%02X (stereo=%d, 16-bit=%d)\n",
           hdr->flags, hdr->flags & 1, (hdr->flags >> 1) & 1);
    printf("  Compression: %d (%s)\n", hdr->compression,
           hdr->compression == 99 ? "IMA ADPCM" :
           hdr->compression == 1 ? "Westwood ADPCM" : "Unknown");

    if (hdr->compression != 99) {
        printf("  ERROR: Only IMA ADPCM (99) supported in this test\n");
        return -1;
    }

    // Allocate output buffer
    uint32_t sampleCount = hdr->uncompSize / 2;  // 16-bit samples
    int16_t* samples = new int16_t[sampleCount];
    memset(samples, 0, sampleCount * sizeof(int16_t));

    // Decode
    int predictor = 0;
    int stepIndex = 0;
    uint32_t samplesDecoded = 0;

    const uint8_t* ptr = fileData + sizeof(AUDHeader);
    const uint8_t* end = fileData + fileSize;

    int chunkNum = 0;
    while (ptr + sizeof(AUDChunkHeader) <= end && samplesDecoded < sampleCount) {
        const AUDChunkHeader* chunk = (const AUDChunkHeader*)ptr;
        ptr += sizeof(AUDChunkHeader);

        if (ptr + chunk->compSize > end) {
            printf("  Chunk %d: truncated (need %d bytes, have %d)\n",
                   chunkNum, chunk->compSize, (int)(end - ptr));
            break;
        }

        if (chunkNum < 3) {
            printf("  Chunk %d: compSize=%d, uncompSize=%d, id=0x%08X\n",
                   chunkNum, chunk->compSize, chunk->uncompSize, chunk->id);
        }

        // Decode this chunk using XCC algorithm
        int chunkSamples = chunk->uncompSize / 2;
        const uint8_t* chunkData = ptr;

        for (int si = 0; si < chunkSamples && samplesDecoded < sampleCount; si++) {
            // Get nibble: even sample = low nibble, odd sample = high nibble
            uint8_t byte = chunkData[si >> 1];
            uint8_t code = (si & 1) ? (byte >> 4) : (byte & 0x0F);

            int step = g_imaStepTable[stepIndex];
            int diff = step >> 3;
            if (code & 1) diff += step >> 2;
            if (code & 2) diff += step >> 1;
            if (code & 4) diff += step;

            if (code & 8) {
                predictor -= diff;
                if (predictor < -32768) predictor = -32768;
            } else {
                predictor += diff;
                if (predictor > 32767) predictor = 32767;
            }

            samples[samplesDecoded++] = (int16_t)predictor;

            stepIndex += g_imaIndexTable[code & 7];
            if (stepIndex < 0) stepIndex = 0;
            else if (stepIndex > 88) stepIndex = 88;
        }

        ptr += chunk->compSize;
        chunkNum++;
    }

    printf("  Decoded %u samples (%d chunks)\n", samplesDecoded, chunkNum);

    *outSamples = samples;
    *outSampleCount = samplesDecoded;
    return 0;
}

// Write WAV file
bool WriteWAV(const char* path, const int16_t* samples, uint32_t sampleCount,
              uint32_t sampleRate, uint8_t channels) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;

    uint32_t dataSize = sampleCount * sizeof(int16_t);
    uint32_t fileSize = 36 + dataSize;
    uint16_t blockAlign = channels * 2;
    uint32_t byteRate = sampleRate * blockAlign;

    // RIFF header
    fwrite("RIFF", 4, 1, f);
    fwrite(&fileSize, 4, 1, f);
    fwrite("WAVE", 4, 1, f);

    // fmt chunk
    fwrite("fmt ", 4, 1, f);
    uint32_t fmtSize = 16;
    fwrite(&fmtSize, 4, 1, f);
    uint16_t audioFormat = 1;  // PCM
    fwrite(&audioFormat, 2, 1, f);
    uint16_t numChannels = channels;
    fwrite(&numChannels, 2, 1, f);
    fwrite(&sampleRate, 4, 1, f);
    fwrite(&byteRate, 4, 1, f);
    fwrite(&blockAlign, 2, 1, f);
    uint16_t bitsPerSample = 16;
    fwrite(&bitsPerSample, 2, 1, f);

    // data chunk
    fwrite("data", 4, 1, f);
    fwrite(&dataSize, 4, 1, f);
    fwrite(samples, sizeof(int16_t), sampleCount, f);

    fclose(f);
    return true;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <input.aud> [output.wav]\n", argv[0]);
        return 1;
    }

    const char* inputPath = argv[1];
    const char* outputPath = argc > 2 ? argv[2] : "/tmp/test_output.wav";

    printf("Loading: %s\n", inputPath);

    FILE* f = fopen(inputPath, "rb");
    if (!f) {
        printf("ERROR: Cannot open input file\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t* data = new uint8_t[size];
    fread(data, 1, size, f);
    fclose(f);

    printf("File size: %ld bytes\n", size);

    int16_t* samples = nullptr;
    uint32_t sampleCount = 0;

    if (DecodeIMA_XCC(data, size, &samples, &sampleCount) != 0) {
        delete[] data;
        return 1;
    }

    // Get sample rate from header
    const AUDHeader* hdr = (const AUDHeader*)data;

    printf("Writing: %s\n", outputPath);
    if (WriteWAV(outputPath, samples, sampleCount, hdr->sampleRate, 1)) {
        printf("SUCCESS: WAV written (%u samples, %.2f seconds)\n",
               sampleCount, (float)sampleCount / hdr->sampleRate);
    } else {
        printf("ERROR: Failed to write WAV\n");
    }

    delete[] samples;
    delete[] data;
    return 0;
}
