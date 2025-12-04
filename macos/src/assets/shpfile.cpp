/**
 * Red Alert macOS Port - SHP Sprite File Reader Implementation
 *
 * Wrapper around libwestwood's ShpReader.
 * Provides C-style API for compatibility with existing game code.
 */

#include "shpfile.h"
#include <westwood/shp.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>
#include <fstream>

// Internal SHP file structure wrapping libwestwood
struct ShpFile {
    std::unique_ptr<wwd::ShpReader> reader;
    std::vector<std::vector<uint8_t>> decodedFrames;
    std::vector<ShpFrame> frames;
};

static ShpFileHandle Shp_LoadInternal(std::unique_ptr<wwd::ShpReader> reader) {
    if (!reader) return nullptr;

    auto* shp = new ShpFile();
    shp->reader = std::move(reader);

    // Pre-decode all frames
    const auto& frameInfos = shp->reader->frames();
    shp->decodedFrames.resize(frameInfos.size());
    shp->frames.resize(frameInfos.size());

    std::vector<uint8_t> deltaBuffer;

    for (size_t i = 0; i < frameInfos.size(); i++) {
        auto result = shp->reader->decode_frame(i, deltaBuffer);
        if (result) {
            shp->decodedFrames[i] = std::move(*result);
        }

        shp->frames[i].width = frameInfos[i].width;
        shp->frames[i].height = frameInfos[i].height;
        shp->frames[i].offsetX = frameInfos[i].offset_x;
        shp->frames[i].offsetY = frameInfos[i].offset_y;
        shp->frames[i].pixels = shp->decodedFrames[i].empty()
                                    ? nullptr
                                    : shp->decodedFrames[i].data();
    }

    return shp;
}

ShpFileHandle Shp_Load(const void* data, uint32_t dataSize) {
    if (!data || dataSize == 0) return nullptr;

    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), dataSize);
    auto result = wwd::ShpReader::open(span);
    if (!result) return nullptr;

    return Shp_LoadInternal(std::move(*result));
}

ShpFileHandle Shp_LoadFile(const char* filename) {
    if (!filename) return nullptr;

    auto result = wwd::ShpReader::open(filename);
    if (!result) return nullptr;

    return Shp_LoadInternal(std::move(*result));
}

void Shp_Free(ShpFileHandle shp) {
    delete shp;
}

int Shp_GetFrameCount(ShpFileHandle shp) {
    if (!shp) return 0;
    return static_cast<int>(shp->frames.size());
}

const ShpFrame* Shp_GetFrame(ShpFileHandle shp, int index) {
    if (!shp) return nullptr;
    if (index < 0 || index >= static_cast<int>(shp->frames.size())) {
        return nullptr;
    }
    return &shp->frames[index];
}

uint16_t Shp_GetMaxWidth(ShpFileHandle shp) {
    if (!shp || !shp->reader) return 0;
    return shp->reader->info().max_width;
}

uint16_t Shp_GetMaxHeight(ShpFileHandle shp) {
    if (!shp || !shp->reader) return 0;
    return shp->reader->info().max_height;
}
