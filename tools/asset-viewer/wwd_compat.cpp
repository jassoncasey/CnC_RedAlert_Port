/**
 * Westwood Format Compatibility Layer - Implementation
 *
 * Wraps libwestwood C++ classes with C-style API.
 */

#include "wwd_compat.h"

#include <westwood/mix.h>
#include <westwood/shp.h>
#include <westwood/aud.h>
#include <westwood/pal.h>

#include <cstring>
#include <cstdlib>
#include <vector>
#include <memory>

//===========================================================================
// MIX File Implementation
//===========================================================================

struct MixFile {
    std::unique_ptr<wwd::MixReader> reader;
    std::vector<uint8_t> ownedData;  // For memory-loaded MIX files
};

MixFileHandle Mix_Open(const char* path) {
    auto result = wwd::MixReader::open(path);
    if (!result) return nullptr;

    auto* mix = new MixFile();
    mix->reader = std::move(*result);
    return mix;
}

MixFileHandle Mix_OpenMemory(const void* data, uint32_t size, int ownsData) {
    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), size);
    auto result = wwd::MixReader::open(span);
    if (!result) {
        if (ownsData) free(const_cast<void*>(data));
        return nullptr;
    }

    auto* mix = new MixFile();
    mix->reader = std::move(*result);

    // If we own the data, copy it since libwestwood may reference it
    if (ownsData) {
        mix->ownedData.assign(static_cast<const uint8_t*>(data),
                              static_cast<const uint8_t*>(data) + size);
        free(const_cast<void*>(data));
    }

    return mix;
}

void Mix_Close(MixFileHandle mix) {
    delete mix;
}

int Mix_GetFileCount(MixFileHandle mix) {
    if (!mix || !mix->reader) return 0;
    return static_cast<int>(mix->reader->entries().size());
}

int Mix_GetEntryByIndex(MixFileHandle mix, int index,
                        uint32_t* outCRC, uint32_t* outSize) {
    if (!mix || !mix->reader) return 0;

    const auto& entries = mix->reader->entries();
    if (index < 0 || index >= static_cast<int>(entries.size())) return 0;

    if (outCRC) *outCRC = entries[index].hash;
    if (outSize) *outSize = entries[index].size;
    return 1;
}

void* Mix_AllocReadFileByCRC(MixFileHandle mix, uint32_t crc, uint32_t* outSize) {
    if (!mix || !mix->reader) return nullptr;

    const auto* entry = mix->reader->find(crc);
    if (!entry) return nullptr;

    auto result = mix->reader->read(*entry);
    if (!result) return nullptr;

    auto& data = *result;
    if (outSize) *outSize = static_cast<uint32_t>(data.size());

    void* copy = malloc(data.size());
    if (copy) {
        memcpy(copy, data.data(), data.size());
    }
    return copy;
}

uint32_t Mix_CalculateCRC(const char* filename) {
    // Red Alert uses the TD hash
    return wwd::mix_hash_td(filename);
}

const char* Mix_GetFilename(uint32_t crc) {
    // libwestwood doesn't have a global filename database
    // Return nullptr - the viewer handles unknown names
    (void)crc;
    return nullptr;
}

//===========================================================================
// SHP File Implementation
//===========================================================================

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

ShpFileHandle Shp_LoadFile(const char* path) {
    auto result = wwd::ShpReader::open(path);
    if (!result) return nullptr;
    return Shp_LoadInternal(std::move(*result));
}

ShpFileHandle Shp_Load(const void* data, uint32_t size) {
    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), size);
    auto result = wwd::ShpReader::open(span);
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

const ShpFrame* Shp_GetFrame(ShpFileHandle shp, int frameIndex) {
    if (!shp) return nullptr;
    if (frameIndex < 0 || frameIndex >= static_cast<int>(shp->frames.size())) {
        return nullptr;
    }
    return &shp->frames[frameIndex];
}

//===========================================================================
// AUD File Implementation
//===========================================================================

struct AudFile {
    std::unique_ptr<wwd::AudReader> reader;
    std::vector<int16_t> samples;
};

static AudFileHandle Aud_LoadInternal(std::unique_ptr<wwd::AudReader> reader) {
    if (!reader) return nullptr;

    auto* aud = new AudFile();
    aud->reader = std::move(reader);

    // Decode audio
    auto result = aud->reader->decode();
    if (result) {
        aud->samples = std::move(*result);
    }

    return aud;
}

AudFileHandle Aud_LoadFile(const char* path) {
    auto result = wwd::AudReader::open(path);
    if (!result) return nullptr;
    return Aud_LoadInternal(std::move(*result));
}

AudFileHandle Aud_Load(const void* data, uint32_t size) {
    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), size);
    auto result = wwd::AudReader::open(span);
    if (!result) return nullptr;
    return Aud_LoadInternal(std::move(*result));
}

void Aud_Free(AudFileHandle aud) {
    delete aud;
}

const int16_t* Aud_GetSamples(AudFileHandle aud) {
    if (!aud || aud->samples.empty()) return nullptr;
    return aud->samples.data();
}

uint32_t Aud_GetSampleCount(AudFileHandle aud) {
    if (!aud) return 0;
    return static_cast<uint32_t>(aud->samples.size());
}

uint32_t Aud_GetSampleRate(AudFileHandle aud) {
    if (!aud || !aud->reader) return 0;
    return aud->reader->info().sample_rate;
}

int Aud_GetChannels(AudFileHandle aud) {
    if (!aud || !aud->reader) return 0;
    return aud->reader->info().channels;
}

//===========================================================================
// PAL File Implementation
//===========================================================================

struct PalFile {
    std::unique_ptr<wwd::PalReader> reader;
    uint8_t colors[256 * 3];  // 8-bit RGB values
};

static PalFileHandle Pal_LoadInternal(std::unique_ptr<wwd::PalReader> reader) {
    if (!reader) return nullptr;

    auto* pal = new PalFile();
    pal->reader = std::move(reader);

    // Convert to 8-bit RGB
    for (int i = 0; i < 256; i++) {
        auto c = pal->reader->color_8bit(i);
        pal->colors[i * 3 + 0] = c.r;
        pal->colors[i * 3 + 1] = c.g;
        pal->colors[i * 3 + 2] = c.b;
    }

    return pal;
}

PalFileHandle Pal_LoadFile(const char* path) {
    auto result = wwd::PalReader::open(path);
    if (!result) return nullptr;
    return Pal_LoadInternal(std::move(*result));
}

PalFileHandle Pal_Load(const void* data, uint32_t size) {
    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), size);
    auto result = wwd::PalReader::open(span);
    if (!result) return nullptr;
    return Pal_LoadInternal(std::move(*result));
}

void Pal_Free(PalFileHandle pal) {
    delete pal;
}

const uint8_t* Pal_GetColors(PalFileHandle pal) {
    if (!pal) return nullptr;
    return pal->colors;
}
