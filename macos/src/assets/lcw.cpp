/**
 * Red Alert macOS Port - LCW Compression Implementation
 */

#include "lcw.h"
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
        if (padding < 2 && dstIdx < dstSize) dst[dstIdx++] = (triple >> 8) & 0xFF;
        if (padding < 1 && dstIdx < dstSize) dst[dstIdx++] = triple & 0xFF;
    }

    return dstIdx;
}

int LCW_Decompress(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
    if (!src || !dst || srcSize <= 0 || dstSize <= 0) return -1;

    int srcIdx = 0;
    int destIdx = 0;

    while (srcIdx < srcSize) {
        uint8_t cmd = src[srcIdx++];

        if ((cmd & 0x80) == 0) {
            // Case 2: Short copy from previous output (relative)
            // 0CCCPPPP PPPPPPPP - Copy (CCC+3) bytes from (dest - PPP)
            if (srcIdx >= srcSize) break;
            uint8_t secondByte = src[srcIdx++];
            int count = ((cmd & 0x70) >> 4) + 3;
            int rpos = ((cmd & 0x0F) << 8) + secondByte;

            if (destIdx + count > dstSize) break;

            int srcPos = destIdx - rpos;
            if (srcPos < 0) break;

            for (int i = 0; i < count; i++) {
                // Handle overlapping copies (RLE-like pattern)
                if (destIdx - srcPos == 1)
                    dst[destIdx + i] = dst[destIdx - 1];
                else
                    dst[destIdx + i] = dst[srcPos + i];
            }
            destIdx += count;
        } else if ((cmd & 0x40) == 0) {
            // Case 1: Literal copy from source
            // 10CCCCCC - Copy C bytes literally from source
            int count = cmd & 0x3F;
            if (count == 0) break; // End marker

            if (srcIdx + count > srcSize || destIdx + count > dstSize) break;
            memcpy(&dst[destIdx], &src[srcIdx], count);
            srcIdx += count;
            destIdx += count;
        } else {
            // 11XXXXXX commands
            int count3 = cmd & 0x3F;
            if (count3 == 0x3E) {
                // Case 4: RLE fill
                // 11111110 CCCC CCCC VV - Fill CCCC bytes with value VV
                if (srcIdx + 3 > srcSize) break;
                int count = src[srcIdx] | (src[srcIdx + 1] << 8);
                uint8_t value = src[srcIdx + 2];
                srcIdx += 3;

                if (destIdx + count > dstSize) count = dstSize - destIdx;
                memset(&dst[destIdx], value, count);
                destIdx += count;
            } else if (count3 == 0x3F) {
                // Case 5: Long copy from previous output (absolute)
                // 11111111 CCCC CCCC PPPP PPPP - Copy CCCC from absolute pos
                if (srcIdx + 4 > srcSize) break;
                int count = src[srcIdx] | (src[srcIdx + 1] << 8);
                int srcPos = src[srcIdx + 2] | (src[srcIdx + 3] << 8);
                srcIdx += 4;

                if (srcPos >= destIdx) break;
                if (destIdx + count > dstSize) break;

                for (int i = 0; i < count; i++)
                    dst[destIdx++] = dst[srcPos++];
            } else {
                // Case 3: Short copy from previous output (absolute)
                // 11CCCCCC PPPP PPPP - Copy (C+3) bytes from absolute pos
                if (srcIdx + 2 > srcSize) break;
                int count = count3 + 3;
                int srcPos = src[srcIdx] | (src[srcIdx + 1] << 8);
                srcIdx += 2;

                if (srcPos >= destIdx) break;
                if (destIdx + count > dstSize) break;

                for (int i = 0; i < count; i++)
                    dst[destIdx++] = dst[srcPos++];
            }
        }
    }

    return destIdx;
}
