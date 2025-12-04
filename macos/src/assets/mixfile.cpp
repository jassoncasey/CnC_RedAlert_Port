/**
 * Red Alert macOS Port - MIX File Reader Implementation
 *
 * Wrapper around libwestwood's MixReader.
 * Provides C-style API for compatibility with existing game code.
 */

#include "mixfile.h"
#include <westwood/mix.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <vector>

// Internal MIX file structure wrapping libwestwood
struct MixFile {
    std::unique_ptr<wwd::MixReader> reader;
    std::vector<uint8_t> ownedData;  // For memory-loaded MIX files
};

uint32_t Mix_CalculateCRC(const char* name) {
    // libwestwood uses mix_hash_td for Red Alert
    return wwd::mix_hash_td(name);
}

MixFileHandle Mix_Open(const char* filename) {
    auto result = wwd::MixReader::open(filename);
    if (!result) {
        return nullptr;
    }

    auto* mix = new MixFile();
    mix->reader = std::move(*result);
    return mix;
}

MixFileHandle Mix_OpenMemory(const void* data, uint32_t size, BOOL ownsData) {
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

BOOL Mix_FileExists(MixFileHandle mix, const char* name) {
    return Mix_FileExistsByCRC(mix, Mix_CalculateCRC(name));
}

BOOL Mix_FileExistsByCRC(MixFileHandle mix, uint32_t crc) {
    if (!mix || !mix->reader) return FALSE;
    return mix->reader->find(crc) != nullptr;
}

uint32_t Mix_GetFileSize(MixFileHandle mix, const char* name) {
    if (!mix || !mix->reader) return 0;
    const auto* entry = mix->reader->find(Mix_CalculateCRC(name));
    return entry ? entry->size : 0;
}

uint32_t Mix_ReadFileByCRC(MixFileHandle mix, uint32_t crc,
                           void* buffer, uint32_t bufSize) {
    if (!mix || !mix->reader || !buffer) return 0;

    const auto* entry = mix->reader->find(crc);
    if (!entry) return 0;

    auto result = mix->reader->read(*entry);
    if (!result) return 0;

    uint32_t copySize = (entry->size < bufSize) ? entry->size : bufSize;
    memcpy(buffer, result->data(), copySize);
    return copySize;
}

uint32_t Mix_ReadFile(MixFileHandle mix, const char* name,
                      void* buffer, uint32_t bufSize) {
    return Mix_ReadFileByCRC(mix, Mix_CalculateCRC(name), buffer, bufSize);
}

void* Mix_AllocReadFile(MixFileHandle mix, const char* name,
                        uint32_t* outSize) {
    if (!mix || !mix->reader) return nullptr;

    const auto* entry = mix->reader->find(Mix_CalculateCRC(name));
    if (!entry) return nullptr;

    auto result = mix->reader->read(*entry);
    if (!result) return nullptr;

    void* buffer = malloc(result->size());
    if (!buffer) return nullptr;

    memcpy(buffer, result->data(), result->size());
    if (outSize) *outSize = static_cast<uint32_t>(result->size());
    return buffer;
}

BOOL Mix_GetEntryByIndex(MixFileHandle mix, int index,
                         uint32_t* outCRC, uint32_t* outSize) {
    if (!mix || !mix->reader) return FALSE;

    const auto& entries = mix->reader->entries();
    if (index < 0 || index >= static_cast<int>(entries.size())) return FALSE;

    if (outCRC) *outCRC = entries[index].hash;
    if (outSize) *outSize = entries[index].size;
    return TRUE;
}

void* Mix_AllocReadFileByCRC(MixFileHandle mix, uint32_t crc,
                             uint32_t* outSize) {
    if (!mix || !mix->reader) return nullptr;

    const auto* entry = mix->reader->find(crc);
    if (!entry) return nullptr;

    auto result = mix->reader->read(*entry);
    if (!result) return nullptr;

    void* buffer = malloc(result->size());
    if (!buffer) return nullptr;

    memcpy(buffer, result->data(), result->size());
    if (outSize) *outSize = static_cast<uint32_t>(result->size());
    return buffer;
}
