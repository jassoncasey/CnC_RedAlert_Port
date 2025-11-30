/**
 * Red Alert macOS Port - SHP Sprite File Reader Implementation
 *
 * SHP file format (TD/RA style):
 *   Header:
 *     uint16_t frameCount
 *     uint16_t unknown1 (usually 0)
 *     uint16_t unknown2 (usually 0)
 *     uint16_t width      - Maximum frame width
 *     uint16_t height     - Maximum frame height
 *     uint16_t unknown3   - Delta/largest frame size
 *   Frame offsets (frameCount + 2 entries):
 *     uint32_t offset[frameCount+2] - Offsets to each frame, plus 2 end markers
 *   Frame data:
 *     Each frame starts with a header, then pixel data
 *
 * Frame header (8 bytes):
 *     uint16_t offsetX
 *     uint16_t offsetY
 *     uint16_t width
 *     uint16_t height
 *     uint8_t  compressionType
 *     uint8_t  unknown
 *     uint16_t unknown2 (refOffset or 0)
 *     uint16_t unknown3
 *
 * Compression types:
 *     0x00 - Uncompressed (just raw pixels)
 *     0x20 - LCW compressed (Format20)
 *     0x40 - Format40 (XOR with previous frame)
 *     0x80 - Format80 (LCW variant)
 */

#include "shpfile.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

#pragma pack(push, 1)
struct ShpHeader {
    uint16_t frameCount;
    uint16_t unknown1;
    uint16_t unknown2;
    uint16_t width;
    uint16_t height;
    uint16_t largestFrameSize;
};

struct ShpFrameHeader {
    uint16_t offsetX;
    uint16_t offsetY;
    uint16_t width;
    uint16_t height;
    uint8_t  compressionType;
    uint8_t  unknown;
    uint16_t refOffset;
    uint16_t unknown2;
};
#pragma pack(pop)

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

    // Read frame offsets
    size_t offsetsStart = sizeof(ShpHeader);
    size_t offsetsSize = (header->frameCount + 2) * sizeof(uint32_t);
    if (offsetsStart + offsetsSize > dataSize) {
        return nullptr;
    }

    const uint32_t* offsets = (const uint32_t*)(bytes + offsetsStart);

    // Allocate SHP structure
    ShpFile* shp = new ShpFile;
    shp->frameCount = header->frameCount;
    shp->maxWidth = header->width;
    shp->maxHeight = header->height;
    shp->frames = new ShpFrame[header->frameCount];
    memset(shp->frames, 0, sizeof(ShpFrame) * header->frameCount);

    // Temporary buffer for decompression
    int maxPixels = header->width * header->height;
    uint8_t* tempBuffer = new uint8_t[maxPixels];
    uint8_t* prevFrame = new uint8_t[maxPixels];
    memset(prevFrame, 0, maxPixels);

    // Load each frame
    for (int i = 0; i < header->frameCount; i++) {
        uint32_t frameOffset = offsets[i];
        if (frameOffset == 0 || frameOffset + sizeof(ShpFrameHeader) > dataSize) {
            // Empty or invalid frame
            shp->frames[i].pixels = nullptr;
            shp->frames[i].width = 0;
            shp->frames[i].height = 0;
            continue;
        }

        const ShpFrameHeader* frameHdr = (const ShpFrameHeader*)(bytes + frameOffset);
        const uint8_t* frameData = bytes + frameOffset + sizeof(ShpFrameHeader);
        uint32_t frameDataSize = (i + 1 < header->frameCount) ?
            (offsets[i + 1] - frameOffset - sizeof(ShpFrameHeader)) :
            (dataSize - frameOffset - sizeof(ShpFrameHeader));

        // Frame dimensions
        uint16_t fw = frameHdr->width;
        uint16_t fh = frameHdr->height;
        if (fw == 0 || fh == 0 || fw > header->width || fh > header->height) {
            shp->frames[i].pixels = nullptr;
            shp->frames[i].width = 0;
            shp->frames[i].height = 0;
            continue;
        }

        int framePixels = fw * fh;
        shp->frames[i].width = fw;
        shp->frames[i].height = fh;
        shp->frames[i].offsetX = (int16_t)frameHdr->offsetX;
        shp->frames[i].offsetY = (int16_t)frameHdr->offsetY;
        shp->frames[i].pixels = new uint8_t[framePixels];

        // Decompress based on type
        uint8_t compType = frameHdr->compressionType;
        memset(tempBuffer, 0, maxPixels);

        if (compType == 0x00) {
            // Uncompressed
            if (frameData + framePixels <= bytes + dataSize) {
                memcpy(shp->frames[i].pixels, frameData, framePixels);
            }
        } else if (compType == 0x80 || compType == 0x20) {
            // LCW compressed
            DecompressLCW(frameData, tempBuffer, frameDataSize, framePixels);
            memcpy(shp->frames[i].pixels, tempBuffer, framePixels);
        } else if (compType == 0x40) {
            // XOR delta (relative to previous frame)
            memcpy(tempBuffer, prevFrame, framePixels);
            DecompressFormat40(frameData, tempBuffer, nullptr, frameDataSize, framePixels);
            memcpy(shp->frames[i].pixels, tempBuffer, framePixels);
        } else {
            // Unknown compression, try as raw
            if (frameData + framePixels <= bytes + dataSize) {
                memcpy(shp->frames[i].pixels, frameData, framePixels);
            } else {
                memset(shp->frames[i].pixels, 0, framePixels);
            }
        }

        // Store for delta frames
        memcpy(prevFrame, shp->frames[i].pixels, framePixels);
    }

    delete[] tempBuffer;
    delete[] prevFrame;

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
