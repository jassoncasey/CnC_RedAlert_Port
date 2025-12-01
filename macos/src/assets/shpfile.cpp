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

// LCW/Format80 decompression
// This is a variant of LZSS used by Westwood
static int DecompressLCW(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
    const uint8_t* srcEnd = src + srcSize;
    uint8_t* dstStart = dst;
    uint8_t* dstEnd = dst + dstSize;

    while (src < srcEnd && dst < dstEnd) {
        uint8_t cmd = *src++;

        if (cmd == 0) {
            // End of data
            break;
        } else if ((cmd & 0x80) == 0) {
            // Short copy from source (relative)
            // cmd = 0CCCCCPP PPPPPPPP
            // Copy CCC+3 bytes from position dst-PPP
            if (src >= srcEnd) break;
            int count = ((cmd >> 4) & 0x07) + 3;
            int offset = ((cmd & 0x0F) << 8) | *src++;
            const uint8_t* copySrc = dst - offset;
            if (copySrc < dstStart || dst + count > dstEnd) break;
            while (count-- > 0) {
                *dst++ = *copySrc++;
            }
        } else if ((cmd & 0xC0) == 0x80) {
            // Short literal run
            // cmd = 10CCCCCC
            // Copy CCC bytes literally from source
            int count = cmd & 0x3F;
            if (count == 0) break; // End marker
            if (src + count > srcEnd || dst + count > dstEnd) break;
            memcpy(dst, src, count);
            src += count;
            dst += count;
        } else if ((cmd & 0xC0) == 0xC0) {
            // Various commands based on next bits
            if ((cmd & 0xFE) == 0xFE) {
                // Long run of single byte
                // FE CC CC VV or FF CC CC VV
                if (src + 3 > srcEnd) break;
                int count = src[0] | (src[1] << 8);
                uint8_t value = src[2];
                src += 3;
                if (dst + count > dstEnd) count = (int)(dstEnd - dst);
                memset(dst, value, count);
                dst += count;
            } else if ((cmd & 0x3F) == 0x3E) {
                // Medium literal run
                // cmd = 11111110 CC CC
                if (src + 2 > srcEnd) break;
                int count = src[0] | (src[1] << 8);
                src += 2;
                if (src + count > srcEnd || dst + count > dstEnd) break;
                memcpy(dst, src, count);
                src += count;
                dst += count;
            } else if ((cmd & 0x3F) == 0x3F) {
                // Long copy from source (absolute)
                if (src + 4 > srcEnd) break;
                int count = src[0] | (src[1] << 8);
                int offset = src[2] | (src[3] << 8);
                src += 4;
                const uint8_t* copySrc = dstStart + offset;
                if (copySrc < dstStart || copySrc >= dst || dst + count > dstEnd) break;
                while (count-- > 0) {
                    *dst++ = *copySrc++;
                }
            } else {
                // Short run of single byte
                // cmd = 11CCCCCC VV
                int count = (cmd & 0x3F) + 3;
                if (src >= srcEnd) break;
                uint8_t value = *src++;
                if (dst + count > dstEnd) count = (int)(dstEnd - dst);
                memset(dst, value, count);
                dst += count;
            }
        }
    }

    return (int)(dst - dstStart);
}

// Format40 decompression (XOR delta)
static int DecompressFormat40(const uint8_t* src, uint8_t* dst, const uint8_t* ref, int srcSize, int dstSize) {
    const uint8_t* srcEnd = src + srcSize;
    uint8_t* dstStart = dst;
    uint8_t* dstEnd = dst + dstSize;

    // Copy reference frame first
    if (ref && ref != dst) {
        memcpy(dst, ref, dstSize);
    }

    while (src < srcEnd && dst < dstEnd) {
        uint8_t cmd = *src++;

        if (cmd == 0) {
            // End of data
            break;
        } else if ((cmd & 0x80) == 0) {
            // Skip cmd bytes
            dst += cmd;
        } else if (cmd == 0x80) {
            // Extended command
            if (src + 2 > srcEnd) break;
            uint16_t count = src[0] | (src[1] << 8);
            src += 2;
            if (count == 0) {
                // End of data
                break;
            } else if ((count & 0x8000) == 0) {
                // Skip count bytes
                dst += count;
            } else if ((count & 0x4000) == 0) {
                // XOR count & 0x3FFF bytes with source
                count &= 0x3FFF;
                if (src + count > srcEnd || dst + count > dstEnd) break;
                while (count-- > 0) {
                    *dst++ ^= *src++;
                }
            } else {
                // Fill count & 0x3FFF bytes with XOR value
                count &= 0x3FFF;
                if (src >= srcEnd) break;
                uint8_t value = *src++;
                if (dst + count > dstEnd) count = (uint16_t)(dstEnd - dst);
                while (count-- > 0) {
                    *dst++ ^= value;
                }
            }
        } else {
            // XOR (cmd & 0x7F) bytes with source
            int count = cmd & 0x7F;
            if (src + count > srcEnd || dst + count > dstEnd) break;
            while (count-- > 0) {
                *dst++ ^= *src++;
            }
        }
    }

    return (int)(dst - dstStart);
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

    // Temporary buffers for decompression
    int maxPixels = header->width * header->height;
    uint8_t* tempBuffer = new uint8_t[maxPixels];

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
        memset(tempBuffer, 0, maxPixels);

        if (format == 0x00) {
            // Uncompressed raw pixels
            size_t copySize = (frameDataSize < (size_t)framePixels) ? frameDataSize : framePixels;
            if (frameData + copySize <= bytes + dataSize) {
                memcpy(shp->frames[i].pixels, frameData, copySize);
            }
        } else if (format == SHP_FORMAT_LCW) {
            // 0x80: LCW compressed
            DecompressLCW(frameData, tempBuffer, frameDataSize, framePixels);
            memcpy(shp->frames[i].pixels, tempBuffer, framePixels);
        } else if (format == SHP_FORMAT_XORPREV) {
            // 0x20: XOR with previous frame
            // First decompress with LCW, then XOR with reference
            DecompressLCW(frameData, tempBuffer, frameDataSize, framePixels);

            // XOR with previous frame (i-1) if it exists
            if (i > 0) {
                for (int p = 0; p < framePixels; p++) {
                    shp->frames[i].pixels[p] = decodedFrames[i-1][p] ^ tempBuffer[p];
                }
            } else {
                memcpy(shp->frames[i].pixels, tempBuffer, framePixels);
            }
        } else if (format == SHP_FORMAT_XORLCW) {
            // 0x40: XOR with LCW-referenced frame
            // The refOffset tells us which frame to reference
            uint16_t refIdx = frameOffsets[i].refOffset;

            // First decompress this frame's data
            DecompressLCW(frameData, tempBuffer, frameDataSize, framePixels);

            // XOR with reference frame
            if (refIdx < i && refIdx < header->frameCount) {
                for (int p = 0; p < framePixels; p++) {
                    shp->frames[i].pixels[p] = decodedFrames[refIdx][p] ^ tempBuffer[p];
                }
            } else {
                memcpy(shp->frames[i].pixels, tempBuffer, framePixels);
            }
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
    delete[] tempBuffer;
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
