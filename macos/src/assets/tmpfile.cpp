/**
 * Red Alert macOS Port - TMP Terrain Template File Reader Implementation
 *
 * Uses libwestwood's TmpReader for loading.
 * Provides C-style API for compatibility with existing game code.
 */

#include "tmpfile.h"
#include <westwood/tmp.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

// Internal TMP file structure wrapping libwestwood
struct TmpFile {
    std::unique_ptr<wwd::TmpReader> reader;
    std::vector<std::vector<uint8_t>> decodedTiles;
    std::vector<TmpTile> tiles;
};

static TmpFileHandle Tmp_LoadInternal(std::unique_ptr<wwd::TmpReader> reader) {
    if (!reader) return nullptr;

    auto* tmp = new TmpFile();
    tmp->reader = std::move(reader);

    // Pre-decode all tiles
    const auto& tileInfos = tmp->reader->tiles();
    tmp->decodedTiles.resize(tileInfos.size());
    tmp->tiles.resize(tileInfos.size());

    const auto& info = tmp->reader->info();

    for (size_t i = 0; i < tileInfos.size(); i++) {
        tmp->tiles[i].width = info.tile_width;
        tmp->tiles[i].height = info.tile_height;

        if (tileInfos[i].valid) {
            tmp->decodedTiles[i] = tmp->reader->decode_tile(i);
            tmp->tiles[i].pixels = tmp->decodedTiles[i].empty()
                                       ? nullptr
                                       : tmp->decodedTiles[i].data();
        } else {
            tmp->tiles[i].pixels = nullptr;
        }
    }

    return tmp;
}

TmpFileHandle Tmp_Load(const void* data, uint32_t dataSize) {
    if (!data || dataSize == 0) return nullptr;

    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), dataSize);
    auto result = wwd::TmpReader::open(span);
    if (!result) return nullptr;

    return Tmp_LoadInternal(std::move(*result));
}

TmpFileHandle Tmp_LoadFile(const char* filename) {
    if (!filename) return nullptr;

    auto result = wwd::TmpReader::open(filename);
    if (!result) return nullptr;

    return Tmp_LoadInternal(std::move(*result));
}

void Tmp_Free(TmpFileHandle tmp) {
    delete tmp;
}

int Tmp_GetTileCount(TmpFileHandle tmp) {
    if (!tmp) return 0;
    return static_cast<int>(tmp->tiles.size());
}

const TmpTile* Tmp_GetTile(TmpFileHandle tmp, int index) {
    if (!tmp || index < 0 || index >= static_cast<int>(tmp->tiles.size())) {
        return nullptr;
    }
    // Return tile even for empty tiles (pixels may be nullptr)
    // This matches original behavior - callers check tile->pixels
    return &tmp->tiles[index];
}

uint16_t Tmp_GetTileWidth(TmpFileHandle tmp) {
    if (!tmp || !tmp->reader) return 0;
    return tmp->reader->info().tile_width;
}

uint16_t Tmp_GetTileHeight(TmpFileHandle tmp) {
    if (!tmp || !tmp->reader) return 0;
    return tmp->reader->info().tile_height;
}
