/**
 * Red Alert macOS Port - LCW Compression Implementation
 *
 * Uses libwestwood for LCW decompression.
 * Provides Base64 decode utility.
 */

#include "lcw.h"
#include <westwood/lcw.h>
#include <cstring>

// Base64 decode table
static const int8_t b64_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

int Base64_Decode(const char* src, int srcLen, uint8_t* dst, int dstSize) {
    if (!src || !dst || srcLen <= 0 || dstSize <= 0) return -1;

    int dstIdx = 0;
    int srcIdx = 0;

    while (srcIdx < srcLen) {
        // Skip whitespace and newlines
        while (srcIdx < srcLen && (src[srcIdx] == '\n' || src[srcIdx] == '\r' ||
                                   src[srcIdx] == ' ' || src[srcIdx] == '\t')) {
            srcIdx++;
        }
        if (srcIdx >= srcLen) break;

        // Get 4 base64 characters
        int vals[4] = {0, 0, 0, 0};
        int padding = 0;

        for (int i = 0; i < 4 && srcIdx < srcLen; i++) {
            char c = src[srcIdx++];
            if (c == '=') {
                vals[i] = 0;
                padding++;
            } else {
                int8_t v = b64_table[(unsigned char)c];
                if (v < 0) {
                    // Skip invalid characters
                    i--;
                    continue;
                }
                vals[i] = v;
            }
        }

        // Decode 4 base64 chars to 3 bytes
        uint32_t triple = (vals[0] << 18) | (vals[1] << 12) |
                          (vals[2] << 6) | vals[3];

        if (dstIdx < dstSize) dst[dstIdx++] = (triple >> 16) & 0xFF;
        bool mid = padding < 2 && dstIdx < dstSize;
        if (mid) dst[dstIdx++] = (triple >> 8) & 0xFF;
        if (padding < 1 && dstIdx < dstSize) dst[dstIdx++] = triple & 0xFF;
    }

    return dstIdx;
}

int LCW_Decompress(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
    if (!src || !dst || srcSize <= 0 || dstSize <= 0) return -1;

    std::span<const uint8_t> input(src, srcSize);
    std::span<uint8_t> output(dst, dstSize);

    auto result = wwd::lcw_decompress(input, output);
    if (!result) {
        return -1;
    }

    return static_cast<int>(*result);
}
