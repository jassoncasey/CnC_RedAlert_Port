/**
 * Red Alert macOS Port - TMP Terrain Template File Reader Implementation
 */

#include "tmpfile.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>

// Red Alert TMP file format
#pragma pack(push, 1)
struct TmpHeader {
    uint16_t width;         // Tile width (24)
    uint16_t height;        // Tile height (24)
    uint16_t tileCount;     // Number of tiles in this template
    uint16_t reserved1[5];  // Usually 0
    uint32_t imgStart;      // Offset to image data
    uint32_t reserved2[2];  // Usually 0
    uint32_t indexEnd;      // End of index table offset
    uint32_t reserved3;     // Usually 0
    uint32_t indexStart;    // Start of index table offset
};  // 40 bytes total
#pragma pack(pop)

// Magic bytes to identify RA TMP format
constexpr uint16_t TMP_RA_MAGIC = 0x2c73;

struct TmpFile {
    TmpTile* tiles;
    int tileCount;
    uint16_t tileWidth;
    uint16_t tileHeight;
};

static bool IsTmpRA(const uint8_t* data, uint32_t dataSize) {
    if (dataSize < 40) return false;

    // Check for RA TMP signature at offset 24-25
    uint16_t magic = data[24] | (data[25] << 8);

    // Also check header values are reasonable
    const TmpHeader* hdr = (const TmpHeader*)data;
    if (hdr->width == 0 || hdr->width > 48) return false;
    if (hdr->height == 0 || hdr->height > 48) return false;
    if (hdr->tileCount == 0 || hdr->tileCount > 256) return false;

    return magic == TMP_RA_MAGIC || (hdr->width == 24 && hdr->height == 24);
}

TmpFileHandle Tmp_Load(const void* data, uint32_t dataSize) {
    if (!data || dataSize < sizeof(TmpHeader)) {
        return nullptr;
    }

    const uint8_t* bytes = (const uint8_t*)data;

    if (!IsTmpRA(bytes, dataSize)) {
        return nullptr;
    }

    const TmpHeader* header = (const TmpHeader*)bytes;

    uint16_t width = header->width;
    uint16_t height = header->height;
    uint32_t imgStart = header->imgStart;
    uint32_t indexStart = header->indexStart;
    uint32_t indexEnd = header->indexEnd;

    // Validate offsets
    if (indexStart >= dataSize || indexEnd > dataSize || imgStart >= dataSize) {
        return nullptr;
    }

    int count = indexEnd - indexStart;
    if (count <= 0 || count > 256) {
        return nullptr;
    }

    // Allocate TMP structure
    TmpFile* tmp = new TmpFile;
    tmp->tileCount = count;
    tmp->tileWidth = width;
    tmp->tileHeight = height;
    tmp->tiles = new TmpTile[count];
    memset(tmp->tiles, 0, sizeof(TmpTile) * count);

    int tileSize = width * height;

    // Read index table and load tiles
    const uint8_t* indexTable = bytes + indexStart;

    for (int i = 0; i < count; i++) {
        uint8_t tileIdx = indexTable[i];

        if (tileIdx == 255) {
            // Empty tile
            tmp->tiles[i].pixels = nullptr;
            tmp->tiles[i].width = width;
            tmp->tiles[i].height = height;
        } else {
            // Calculate tile data position
            uint32_t tileOffset = imgStart + tileIdx * tileSize;

            if (tileOffset + tileSize <= dataSize) {
                tmp->tiles[i].pixels = new uint8_t[tileSize];
                tmp->tiles[i].width = width;
                tmp->tiles[i].height = height;
                memcpy(tmp->tiles[i].pixels, bytes + tileOffset, tileSize);
            } else {
                tmp->tiles[i].pixels = nullptr;
                tmp->tiles[i].width = width;
                tmp->tiles[i].height = height;
            }
        }
    }

    return tmp;
}

TmpFileHandle Tmp_LoadFile(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) return nullptr;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 1024 * 1024) { // 1MB max for terrain
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

    TmpFileHandle tmp = Tmp_Load(data, (uint32_t)size);
    delete[] data;

    return tmp;
}

void Tmp_Free(TmpFileHandle tmp) {
    if (tmp) {
        if (tmp->tiles) {
            for (int i = 0; i < tmp->tileCount; i++) {
                if (tmp->tiles[i].pixels) {
                    delete[] tmp->tiles[i].pixels;
                }
            }
            delete[] tmp->tiles;
        }
        delete tmp;
    }
}

int Tmp_GetTileCount(TmpFileHandle tmp) {
    return tmp ? tmp->tileCount : 0;
}

const TmpTile* Tmp_GetTile(TmpFileHandle tmp, int index) {
    if (!tmp || index < 0 || index >= tmp->tileCount) {
        return nullptr;
    }
    return &tmp->tiles[index];
}

uint16_t Tmp_GetTileWidth(TmpFileHandle tmp) {
    return tmp ? tmp->tileWidth : 0;
}

uint16_t Tmp_GetTileHeight(TmpFileHandle tmp) {
    return tmp ? tmp->tileHeight : 0;
}
