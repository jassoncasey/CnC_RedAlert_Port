/**
 * Red Alert macOS Port - SHP Sprite File Reader Implementation
 *
 * TD/RA SHP file format:
 *   Header (14 bytes):
 *     uint16_t frameCount
 *     uint16_t unknown1 (usually 0)
 *     uint16_t unknown2 (usually 0)
 *     uint16_t width      - Frame width (all frames same size)
 *     uint16_t height     - Frame height
 *     uint32_t largestFrameSize
 *
 *   Frame offset table (8 bytes per entry, frameCount+2 entries):
 *     uint32_t offsetAndFormat  - Low 24 bits: file offset, High 8 bits: format
 *     uint16_t refOffset        - Reference frame index (for XOR formats)
 *     uint16_t refFormat        - Reference format
 *
 *   Compressed frame data follows the offset table
 *
 * Format types (high byte of offsetAndFormat):
 *     0x00 - Uncompressed raw pixels
 *     0x20 - XORPrev (LCW + XOR with previous frame)
 *     0x40 - XORLCW (LCW + XOR with referenced frame)
 *     0x80 - LCW compressed
 */

#include "shpfile.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// TD/RA SHP file format (differs from later C&C games)
#pragma pack(push, 1)
struct ShpHeader {
    uint16_t frameCount;
    uint16_t unknown1;      // Usually 0
    uint16_t unknown2;      // Usually 0
    uint16_t width;         // Max frame width
    uint16_t height;        // Max frame height
    uint32_t largestFrameSize;  // Note: 4 bytes, not 2
};  // 14 bytes total

// Per-frame offset entry (8 bytes each)
struct ShpFrameOffset {
    // Low 24 bits = file offset, high 8 bits = format type
    // Format: 0x20=XORPrev, 0x40=XORLCW, 0x80=LCW
    uint32_t offsetAndFormat;
    uint16_t refOffset;     // Reference frame offset (for XOR formats)
    uint16_t refFormat;     // Reference format
};
#pragma pack(pop)

// Format types
constexpr uint8_t SHP_FORMAT_XORPREV = 0x20;
constexpr uint8_t SHP_FORMAT_XORLCW  = 0x40;
constexpr uint8_t SHP_FORMAT_LCW     = 0x80;

struct ShpFile {
    ShpFrame* frames;
    int frameCount;
    uint16_t maxWidth;
    uint16_t maxHeight;
};

// LCW/Format80 decompression (Lempel-Castle-Welch)
// Based on OpenRA's LCWCompression.cs implementation
static int DecompressLCW(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
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
                // 11111111 CCCC CCCC PPPP PPPP - Copy CCCC bytes from absolute position PPPP
                if (srcIdx + 4 > srcSize) break;
                int count = src[srcIdx] | (src[srcIdx + 1] << 8);
                int srcPos = src[srcIdx + 2] | (src[srcIdx + 3] << 8);
                srcIdx += 4;

                if (srcPos >= destIdx) break;
                if (destIdx + count > dstSize) break;

                for (int i = 0; i < count; i++)
                    dst[destIdx++] = dst[srcPos++];
            } else {
                // Case 3: Short copy from previous output (absolute) with short count
                // 11CCCCCC PPPP PPPP - Copy (C+3) bytes from absolute position PPPP
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

// XOR Delta decompression (Format40)
// Based on OpenRA's XORDeltaCompression.cs implementation
// This applies XOR delta to an existing destination buffer (which should contain reference frame)
static int DecompressXORDelta(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
    int srcIdx = 0;
    int destIdx = 0;

    while (srcIdx < srcSize) {
        uint8_t cmd = src[srcIdx++];

        if ((cmd & 0x80) == 0) {
            // Low bit clear
            int count = cmd & 0x7F;
            if (count == 0) {
                // Case 6: XOR fill
                // 00000000 CC VV - XOR CC bytes with value VV
                if (srcIdx + 2 > srcSize) break;
                count = src[srcIdx++];
                uint8_t value = src[srcIdx++];
                for (int end = destIdx + count; destIdx < end && destIdx < dstSize; destIdx++)
                    dst[destIdx] ^= value;
            } else {
                // Case 5: XOR literal
                // 0CCCCCCC [data] - XOR C bytes with source data
                if (srcIdx + count > srcSize) break;
                for (int end = destIdx + count; destIdx < end && destIdx < dstSize; destIdx++)
                    dst[destIdx] ^= src[srcIdx++];
            }
        } else {
            // High bit set
            int count = cmd & 0x7F;
            if (count == 0) {
                // Extended command
                if (srcIdx + 2 > srcSize) break;
                uint16_t word = src[srcIdx] | (src[srcIdx + 1] << 8);
                srcIdx += 2;

                if (word == 0) {
                    // End of data
                    break;
                } else if ((word & 0x8000) == 0) {
                    // Case 2: Skip bytes
                    destIdx += word & 0x7FFF;
                } else if ((word & 0x4000) == 0) {
                    // Case 3: XOR literal (long)
                    count = word & 0x3FFF;
                    if (srcIdx + count > srcSize) break;
                    for (int end = destIdx + count; destIdx < end && destIdx < dstSize; destIdx++)
                        dst[destIdx] ^= src[srcIdx++];
                } else {
                    // Case 4: XOR fill (long)
                    count = word & 0x3FFF;
                    if (srcIdx >= srcSize) break;
                    uint8_t value = src[srcIdx++];
                    for (int end = destIdx + count; destIdx < end && destIdx < dstSize; destIdx++)
                        dst[destIdx] ^= value;
                }
            } else {
                // Case 1: Skip bytes (short)
                destIdx += count;
            }
        }
    }

    return destIdx;
}

ShpFileHandle Shp_Load(const void* data, uint32_t dataSize) {
    if (!data || dataSize < sizeof(ShpHeader)) {
        return nullptr;
    }

    const uint8_t* bytes = (const uint8_t*)data;
    const ShpHeader* header = (const ShpHeader*)bytes;

    // Sanity checks
    if (header->frameCount == 0 || header->frameCount > 1000) {
        return nullptr;
    }
    if (header->width == 0 || header->width > 640) {
        return nullptr;
    }
    if (header->height == 0 || header->height > 480) {
        return nullptr;
    }

    // Frame offset entries are 8 bytes each (ShpFrameOffset struct)
    // There are (frameCount + 2) entries: frameCount frames + EOF marker + zero marker
    size_t offsetsStart = sizeof(ShpHeader);  // 14 bytes
    size_t offsetsSize = (header->frameCount + 2) * sizeof(ShpFrameOffset);  // 8 bytes each
    if (offsetsStart + offsetsSize > dataSize) {
        return nullptr;
    }

    const ShpFrameOffset* frameOffsets = (const ShpFrameOffset*)(bytes + offsetsStart);

    // Allocate SHP structure
    ShpFile* shp = new ShpFile;
    shp->frameCount = header->frameCount;
    shp->maxWidth = header->width;
    shp->maxHeight = header->height;
    shp->frames = new ShpFrame[header->frameCount];
    memset(shp->frames, 0, sizeof(ShpFrame) * header->frameCount);

    int maxPixels = header->width * header->height;

    // Store decoded frames for XOR reference (need all frames, not just previous)
    uint8_t** decodedFrames = new uint8_t*[header->frameCount];
    for (int i = 0; i < header->frameCount; i++) {
        decodedFrames[i] = new uint8_t[maxPixels];
        memset(decodedFrames[i], 0, maxPixels);
    }

    // Load each frame
    for (int i = 0; i < header->frameCount; i++) {
        // Extract offset (low 24 bits) and format (high 8 bits)
        uint32_t offsetAndFormat = frameOffsets[i].offsetAndFormat;
        uint32_t frameOffset = offsetAndFormat & 0x00FFFFFF;
        uint8_t format = (offsetAndFormat >> 24) & 0xFF;

        if (frameOffset == 0 || frameOffset >= dataSize) {
            // Empty or invalid frame
            shp->frames[i].pixels = nullptr;
            shp->frames[i].width = 0;
            shp->frames[i].height = 0;
            continue;
        }

        // Frame data starts directly at the offset (no per-frame header in TD format)
        const uint8_t* frameData = bytes + frameOffset;

        // Calculate frame data size from next offset
        uint32_t nextOffset;
        if (i + 1 < header->frameCount) {
            nextOffset = frameOffsets[i + 1].offsetAndFormat & 0x00FFFFFF;
        } else {
            // Use EOF marker
            nextOffset = frameOffsets[header->frameCount].offsetAndFormat & 0x00FFFFFF;
        }

        if (nextOffset <= frameOffset) {
            nextOffset = dataSize;
        }
        uint32_t frameDataSize = nextOffset - frameOffset;

        // All frames have the same dimensions in TD SHP
        uint16_t fw = header->width;
        uint16_t fh = header->height;
        int framePixels = fw * fh;

        shp->frames[i].width = fw;
        shp->frames[i].height = fh;
        shp->frames[i].offsetX = 0;
        shp->frames[i].offsetY = 0;
        shp->frames[i].pixels = new uint8_t[framePixels];

        // Decompress based on format type
        memset(shp->frames[i].pixels, 0, framePixels);

        if (format == 0x00) {
            // Uncompressed raw pixels
            size_t copySize = (frameDataSize < (size_t)framePixels) ? frameDataSize : framePixels;
            if (frameData + copySize <= bytes + dataSize) {
                memcpy(shp->frames[i].pixels, frameData, copySize);
            }
        } else if (format == SHP_FORMAT_LCW) {
            // 0x80: LCW compressed (pure LCW, no XOR)
            DecompressLCW(frameData, shp->frames[i].pixels, frameDataSize, framePixels);
        } else if (format == SHP_FORMAT_XORPREV) {
            // 0x20: XOR delta with previous frame
            // Copy previous frame, then apply XOR delta
            if (i > 0) {
                memcpy(shp->frames[i].pixels, decodedFrames[i-1], framePixels);
            } else {
                memset(shp->frames[i].pixels, 0, framePixels);
            }
            DecompressXORDelta(frameData, shp->frames[i].pixels, frameDataSize, framePixels);
        } else if (format == SHP_FORMAT_XORLCW) {
            // 0x40: XOR delta with referenced frame (by file offset)
            // Need to find reference frame by matching offset
            uint32_t refFileOffset = frameOffsets[i].refOffset;
            int refIdx = -1;

            // Find frame index that matches refOffset
            for (int j = 0; j < i; j++) {
                uint32_t frameOff = frameOffsets[j].offsetAndFormat & 0x00FFFFFF;
                if (frameOff == refFileOffset) {
                    refIdx = j;
                    break;
                }
            }

            // Copy reference frame, then apply XOR delta
            if (refIdx >= 0 && refIdx < i) {
                memcpy(shp->frames[i].pixels, decodedFrames[refIdx], framePixels);
            } else {
                memset(shp->frames[i].pixels, 0, framePixels);
            }
            DecompressXORDelta(frameData, shp->frames[i].pixels, frameDataSize, framePixels);
        } else {
            // Unknown format, try as raw
            size_t copySize = (frameDataSize < (size_t)framePixels) ? frameDataSize : framePixels;
            if (frameData + copySize <= bytes + dataSize) {
                memcpy(shp->frames[i].pixels, frameData, copySize);
            } else {
                memset(shp->frames[i].pixels, 0, framePixels);
            }
        }

        // Store decoded frame for XOR reference
        memcpy(decodedFrames[i], shp->frames[i].pixels, framePixels);
    }

    // Clean up temporary buffers
    for (int i = 0; i < header->frameCount; i++) {
        delete[] decodedFrames[i];
    }
    delete[] decodedFrames;

    return shp;
}

ShpFileHandle Shp_LoadFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return nullptr;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 10 * 1024 * 1024) { // 10MB max
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

    ShpFileHandle shp = Shp_Load(data, (uint32_t)size);
    delete[] data;

    return shp;
}

void Shp_Free(ShpFileHandle shp) {
    if (shp) {
        if (shp->frames) {
            for (int i = 0; i < shp->frameCount; i++) {
                if (shp->frames[i].pixels) {
                    delete[] shp->frames[i].pixels;
                }
            }
            delete[] shp->frames;
        }
        delete shp;
    }
}

int Shp_GetFrameCount(ShpFileHandle shp) {
    return shp ? shp->frameCount : 0;
}

const ShpFrame* Shp_GetFrame(ShpFileHandle shp, int index) {
    if (!shp || index < 0 || index >= shp->frameCount) {
        return nullptr;
    }
    return &shp->frames[index];
}

uint16_t Shp_GetMaxWidth(ShpFileHandle shp) {
    return shp ? shp->maxWidth : 0;
}

uint16_t Shp_GetMaxHeight(ShpFileHandle shp) {
    return shp ? shp->maxHeight : 0;
}
