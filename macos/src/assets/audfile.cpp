/**
 * Red Alert macOS Port - AUD Audio File Reader Implementation
 *
 * Uses libwestwood's AudReader for loading and decoding.
 * Provides C-style API for compatibility with existing game code.
 */

#include "audfile.h"
#include <westwood/aud.h>
#include <cstdlib>
#include <cstring>

AudData* Aud_Load(const void* data, uint32_t dataSize) {
    if (!data || dataSize == 0) {
        return nullptr;
    }

    std::span<const uint8_t> span(static_cast<const uint8_t*>(data), dataSize);
    auto result = wwd::AudReader::open(span);
    if (!result) {
        return nullptr;
    }

    auto& reader = *result;
    auto decoded = reader->decode();
    if (!decoded) {
        return nullptr;
    }

    const auto& info = reader->info();

    // Allocate output
    AudData* aud = new AudData;
    aud->sampleRate = info.sample_rate;
    aud->channels = info.channels;
    aud->sampleCount = static_cast<uint32_t>(decoded->size() / info.channels);

    // Copy samples
    aud->samples = new int16_t[decoded->size()];
    memcpy(aud->samples, decoded->data(), decoded->size() * sizeof(int16_t));

    return aud;
}

AudData* Aud_LoadFile(const char* filename) {
    if (!filename) {
        return nullptr;
    }

    auto result = wwd::AudReader::open(filename);
    if (!result) {
        return nullptr;
    }

    auto& reader = *result;
    auto decoded = reader->decode();
    if (!decoded) {
        return nullptr;
    }

    const auto& info = reader->info();

    // Allocate output
    AudData* aud = new AudData;
    aud->sampleRate = info.sample_rate;
    aud->channels = info.channels;
    aud->sampleCount = static_cast<uint32_t>(decoded->size() / info.channels);

    // Copy samples
    aud->samples = new int16_t[decoded->size()];
    memcpy(aud->samples, decoded->data(), decoded->size() * sizeof(int16_t));

    return aud;
}

void Aud_Free(AudData* aud) {
    if (aud) {
        if (aud->samples) {
            delete[] aud->samples;
        }
        delete aud;
    }
}

BOOL Aud_ConvertTo8Bit(const AudData* aud, uint8_t** outData,
                       uint32_t* outSize, uint32_t* outRate) {
    if (!aud || !outData || !outSize || !outRate) {
        return FALSE;
    }

    // Convert 16-bit signed to 8-bit unsigned
    uint32_t totalSamples = aud->sampleCount * aud->channels;
    uint8_t* data = new uint8_t[totalSamples];

    for (uint32_t i = 0; i < totalSamples; i++) {
        // Convert [-32768, 32767] to [0, 255]
        int sample = (aud->samples[i] + 32768) >> 8;
        if (sample < 0) sample = 0;
        if (sample > 255) sample = 255;
        data[i] = (uint8_t)sample;
    }

    *outData = data;
    *outSize = totalSamples;
    *outRate = aud->sampleRate;

    return TRUE;
}
