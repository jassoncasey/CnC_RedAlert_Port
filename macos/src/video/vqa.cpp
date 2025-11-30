/**
 * Red Alert macOS Port - VQA Video Player Implementation
 *
 * Implements Westwood's VQA (Vector Quantized Animation) decoder.
 * Based on original WINVQ source code analysis.
 */

#include "vqa.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

//===========================================================================
// Byte Swapping (IFF uses big-endian)
//===========================================================================

uint32_t VQAPlayer::SwapBE32(uint32_t val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return ((val & 0xFF000000) >> 24) |
           ((val & 0x00FF0000) >> 8) |
           ((val & 0x0000FF00) << 8) |
           ((val & 0x000000FF) << 24);
#else
    return val;
#endif
}

uint16_t VQAPlayer::SwapBE16(uint16_t val) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (val >> 8) | (val << 8);
#else
    return val;
#endif
}

//===========================================================================
// LCW Decompression
//===========================================================================

int VQAPlayer::DecompressLCW(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
    const uint8_t* srcEnd = src + srcSize;
    uint8_t* dstStart = dst;
    uint8_t* dstEnd = dst + dstSize;

    while (src < srcEnd && dst < dstEnd) {
        uint8_t cmd = *src++;

        if (cmd == 0) {
            // End of data
            break;
        } else if ((cmd & 0x80) == 0) {
            // Short copy from destination (relative)
            // 0CCCPPPP PPPPPPPP - copy C+3 bytes from dst-P
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
            // 10CCCCCC - copy C bytes literally
            int count = cmd & 0x3F;
            if (count == 0) break; // End marker
            if (src + count > srcEnd || dst + count > dstEnd) break;
            memcpy(dst, src, count);
            dst += count;
            src += count;
        } else if ((cmd & 0xE0) == 0xC0) {
            // Long copy from destination (relative)
            // 110PPPPP PPPPPPPP CCCCCCCC - copy C+3 from dst-P
            if (src + 2 > srcEnd) break;
            int offset = ((cmd & 0x1F) << 8) | *src++;
            int count = *src++ + 3;
            const uint8_t* copySrc = dst - offset;
            if (copySrc < dstStart || dst + count > dstEnd) break;
            while (count-- > 0) {
                *dst++ = *copySrc++;
            }
        } else if ((cmd & 0xFC) == 0xFC) {
            // Long run of same byte
            // 111111CC CCCCCCCC VVVVVVVV - fill C+3 with V
            if (src + 2 > srcEnd) break;
            int count = ((cmd & 0x03) << 8) | *src++;
            count += 3;
            uint8_t value = *src++;
            if (dst + count > dstEnd) count = (int)(dstEnd - dst);
            memset(dst, value, count);
            dst += count;
        } else if ((cmd & 0xFE) == 0xFE) {
            // Very long literal run
            // 1111111C CCCCCCCC - copy C bytes literally
            if (src + 1 > srcEnd) break;
            int count = ((cmd & 0x01) << 8) | *src++;
            if (src + count > srcEnd || dst + count > dstEnd) break;
            memcpy(dst, src, count);
            dst += count;
            src += count;
        } else {
            // Long copy from destination (absolute)
            // 111PPPPP PPPPPPPP CCCCCCCC - copy C+3 from absolute P
            if (src + 2 > srcEnd) break;
            int offset = ((cmd & 0x1F) << 8) | *src++;
            int count = *src++ + 3;
            const uint8_t* copySrc = dstStart + offset;
            if (copySrc >= dstEnd || dst + count > dstEnd) break;
            while (count-- > 0) {
                *dst++ = *copySrc++;
            }
        }
    }

    return (int)(dst - dstStart);
}

//===========================================================================
// RLE Decompression for Vector Pointers
//===========================================================================

int VQAPlayer::DecompressRLE(const uint8_t* src, uint8_t* dst, int srcSize, int dstSize) {
    const uint8_t* srcEnd = src + srcSize;
    uint8_t* dstStart = dst;
    uint8_t* dstEnd = dst + dstSize;

    while (src < srcEnd && dst < dstEnd) {
        uint8_t cmd = *src++;

        if (cmd == 0) {
            // End marker
            break;
        } else if (cmd < 0x80) {
            // Literal run: copy cmd bytes
            int count = cmd;
            if (src + count > srcEnd || dst + count > dstEnd) break;
            memcpy(dst, src, count);
            dst += count;
            src += count;
        } else {
            // Repeat run: repeat next byte (cmd - 0x80) times
            int count = cmd - 0x80;
            if (src >= srcEnd || dst + count > dstEnd) break;
            uint8_t value = *src++;
            memset(dst, value, count);
            dst += count;
        }
    }

    return (int)(dst - dstStart);
}

//===========================================================================
// VQAPlayer Constructor/Destructor
//===========================================================================

VQAPlayer::VQAPlayer()
    : data_(nullptr)
    , dataSize_(0)
    , ownsData_(false)
    , frameOffsets_(nullptr)
    , state_(VQAState::STOPPED)
    , currentFrame_(-1)
    , timeAccumulator_(0)
    , frameBuffer_(nullptr)
    , frameBufferSize_(0)
    , paletteChanged_(false)
    , codebook_(nullptr)
    , codebookSize_(0)
    , codebookEntries_(0)
    , audioPredictor_(0)
    , audioStepIndex_(0)
    , audioBuffer_(nullptr)
    , audioBufferSize_(0)
    , audioSamplesReady_(0)
    , decompBuffer_(nullptr)
    , decompBufferSize_(0)
{
    memset(&header_, 0, sizeof(header_));
    memset(palette_, 0, sizeof(palette_));
}

VQAPlayer::~VQAPlayer() {
    Unload();
}

//===========================================================================
// File Operations
//===========================================================================

bool VQAPlayer::Load(const char* filename) {
    Unload();

    FILE* f = fopen(filename, "rb");
    if (!f) {
        return false;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0 || size > 100 * 1024 * 1024) {  // 100MB max
        fclose(f);
        return false;
    }

    uint8_t* data = new uint8_t[size];
    size_t bytesRead = fread(data, 1, size, f);
    fclose(f);

    if (bytesRead != (size_t)size) {
        delete[] data;
        return false;
    }

    data_ = data;
    dataSize_ = (uint32_t)size;
    ownsData_ = true;

    if (!ParseHeader()) {
        Unload();
        return false;
    }

    return true;
}

bool VQAPlayer::Load(const void* data, uint32_t size) {
    Unload();

    if (!data || size < sizeof(IFFChunk) * 2 + sizeof(VQAHeader)) {
        return false;
    }

    data_ = (const uint8_t*)data;
    dataSize_ = size;
    ownsData_ = false;

    if (!ParseHeader()) {
        Unload();
        return false;
    }

    return true;
}

void VQAPlayer::Unload() {
    if (ownsData_ && data_) {
        delete[] data_;
    }
    data_ = nullptr;
    dataSize_ = 0;
    ownsData_ = false;

    delete[] frameOffsets_;
    frameOffsets_ = nullptr;

    delete[] frameBuffer_;
    frameBuffer_ = nullptr;
    frameBufferSize_ = 0;

    delete[] codebook_;
    codebook_ = nullptr;
    codebookSize_ = 0;

    delete[] audioBuffer_;
    audioBuffer_ = nullptr;
    audioBufferSize_ = 0;

    delete[] decompBuffer_;
    decompBuffer_ = nullptr;
    decompBufferSize_ = 0;

    memset(&header_, 0, sizeof(header_));
    memset(palette_, 0, sizeof(palette_));

    state_ = VQAState::STOPPED;
    currentFrame_ = -1;
    timeAccumulator_ = 0;
}

//===========================================================================
// Header Parsing
//===========================================================================

bool VQAPlayer::ParseHeader() {
    if (!data_ || dataSize_ < 12) {
        return false;
    }

    const uint8_t* ptr = data_;
    const uint8_t* end = data_ + dataSize_;

    // Check FORM header
    const IFFChunk* form = (const IFFChunk*)ptr;
    if (SwapBE32(form->id) != VQA_ID_FORM) {
        return false;
    }
    ptr += 8;

    // Check WVQA type
    uint32_t type = SwapBE32(*(uint32_t*)ptr);
    if (type != VQA_ID_WVQA) {
        return false;
    }
    ptr += 4;

    // Find VQHD chunk
    bool foundHeader = false;
    while (ptr + 8 <= end) {
        const IFFChunk* chunk = (const IFFChunk*)ptr;
        uint32_t chunkId = SwapBE32(chunk->id);
        uint32_t chunkSize = SwapBE32(chunk->size);
        ptr += 8;

        if (ptr + chunkSize > end) break;

        if (chunkId == VQA_ID_VQHD && chunkSize >= sizeof(VQAHeader)) {
            // Copy header (little-endian in file, matching x86)
            memcpy(&header_, ptr, sizeof(VQAHeader));
            foundHeader = true;
        } else if (chunkId == VQA_ID_FINF) {
            // Parse frame index
            if (!ParseFrameIndex()) {
                return false;
            }
        }

        // Move to next chunk (pad to even boundary)
        ptr += chunkSize;
        if (chunkSize & 1) ptr++;

        // Stop after we have what we need
        if (foundHeader && frameOffsets_) break;
    }

    if (!foundHeader) {
        return false;
    }

    // Validate header
    if (header_.width == 0 || header_.height == 0 ||
        header_.width > VQA_MAX_WIDTH || header_.height > VQA_MAX_HEIGHT ||
        header_.frames == 0 || header_.fps == 0) {
        return false;
    }

    // Allocate frame buffer
    frameBufferSize_ = header_.width * header_.height;
    frameBuffer_ = new uint8_t[frameBufferSize_];
    memset(frameBuffer_, 0, frameBufferSize_);

    // Allocate codebook (max size: entries * blockW * blockH)
    int blockSize = header_.blockWidth * header_.blockHeight;
    if (blockSize == 0) blockSize = 8;  // Default 4x2
    codebookSize_ = (header_.cbEntries > 0 ? header_.cbEntries : 0x10000) * blockSize;
    codebook_ = new uint8_t[codebookSize_];
    memset(codebook_, 0, codebookSize_);

    // Allocate decompression buffer
    decompBufferSize_ = std::max(frameBufferSize_ * 2, codebookSize_);
    decompBuffer_ = new uint8_t[decompBufferSize_];

    // Allocate audio buffer if needed
    if (header_.flags & VQAHDF_AUDIO) {
        // Enough for ~1 second of audio
        audioBufferSize_ = header_.sampleRate * header_.channels * 2;
        audioBuffer_ = new int16_t[audioBufferSize_];
    }

    return true;
}

bool VQAPlayer::ParseFrameIndex() {
    // FINF contains 32-bit offsets for each frame
    // Already positioned at FINF data

    if (header_.frames == 0) {
        return false;
    }

    frameOffsets_ = new uint32_t[header_.frames];

    // Note: Frame offsets in FINF are stored differently in different VQA versions
    // For now, we'll scan for VQFR/VQFK chunks instead

    return true;
}

//===========================================================================
// Playback Control
//===========================================================================

void VQAPlayer::Play() {
    if (!IsLoaded()) return;

    if (state_ == VQAState::STOPPED || state_ == VQAState::FINISHED) {
        currentFrame_ = -1;
        timeAccumulator_ = 0;
    }

    state_ = VQAState::PLAYING;
}

void VQAPlayer::Pause() {
    if (state_ == VQAState::PLAYING) {
        state_ = VQAState::PAUSED;
    }
}

void VQAPlayer::Stop() {
    state_ = VQAState::STOPPED;
    currentFrame_ = -1;
    timeAccumulator_ = 0;
    memset(frameBuffer_, 0, frameBufferSize_);
}

bool VQAPlayer::NextFrame() {
    if (!IsLoaded()) return false;

    int nextFrame = currentFrame_ + 1;
    if (nextFrame >= header_.frames) {
        state_ = VQAState::FINISHED;
        return false;
    }

    if (!DecodeFrame(nextFrame)) {
        state_ = VQAState::ERROR;
        return false;
    }

    currentFrame_ = nextFrame;
    return true;
}

bool VQAPlayer::SeekFrame(int frame) {
    if (!IsLoaded() || frame < 0 || frame >= header_.frames) {
        return false;
    }

    // For seeking backwards, we need to decode from last keyframe
    // For now, simple forward seek
    if (frame <= currentFrame_) {
        currentFrame_ = -1;
        memset(frameBuffer_, 0, frameBufferSize_);
    }

    while (currentFrame_ < frame) {
        if (!NextFrame()) return false;
    }

    return true;
}

bool VQAPlayer::Update(int elapsedMs) {
    if (state_ != VQAState::PLAYING) {
        return false;
    }

    timeAccumulator_ += elapsedMs;
    int frameDuration = GetFrameDuration();

    if (timeAccumulator_ >= frameDuration) {
        timeAccumulator_ -= frameDuration;

        if (!NextFrame()) {
            return false;
        }
        return true;
    }

    return false;
}

//===========================================================================
// Frame Decoding
//===========================================================================

bool VQAPlayer::DecodeFrame(int frameNum) {
    if (!data_ || frameNum < 0 || frameNum >= header_.frames) {
        return false;
    }

    paletteChanged_ = false;
    audioSamplesReady_ = 0;

    // Scan through file to find frame
    const uint8_t* ptr = data_ + 12;  // Skip FORM header + WVQA
    const uint8_t* end = data_ + dataSize_;

    int currentFrameIndex = -1;

    while (ptr + 8 <= end) {
        const IFFChunk* chunk = (const IFFChunk*)ptr;
        uint32_t chunkId = SwapBE32(chunk->id);
        uint32_t chunkSize = SwapBE32(chunk->size);
        ptr += 8;

        if (ptr + chunkSize > end) break;

        // Is this a frame chunk?
        if (chunkId == VQA_ID_VQFR || chunkId == VQA_ID_VQFK) {
            currentFrameIndex++;

            if (currentFrameIndex == frameNum) {
                // Decode this frame's sub-chunks
                const uint8_t* framePtr = ptr;
                const uint8_t* frameEnd = ptr + chunkSize;

                while (framePtr + 8 <= frameEnd) {
                    const IFFChunk* subChunk = (const IFFChunk*)framePtr;
                    uint32_t subId = SwapBE32(subChunk->id);
                    uint32_t subSize = SwapBE32(subChunk->size);
                    framePtr += 8;

                    if (framePtr + subSize > frameEnd) break;

                    // Decode sub-chunk based on type
                    switch (subId) {
                        case VQA_ID_CBF0:
                            DecodeCodebook(framePtr, subSize, false, false);
                            break;
                        case VQA_ID_CBFZ:
                            DecodeCodebook(framePtr, subSize, true, false);
                            break;
                        case VQA_ID_CBP0:
                            DecodeCodebook(framePtr, subSize, false, true);
                            break;
                        case VQA_ID_CBPZ:
                            DecodeCodebook(framePtr, subSize, true, true);
                            break;
                        case VQA_ID_VPT0:
                        case VQA_ID_VPTZ:
                        case VQA_ID_VPTR:
                        case VQA_ID_VPRZ:
                            DecodePointers(framePtr, subSize, subId);
                            break;
                        case VQA_ID_CPL0:
                            DecodePalette(framePtr, subSize, false);
                            break;
                        case VQA_ID_CPLZ:
                            DecodePalette(framePtr, subSize, true);
                            break;
                        case VQA_ID_SND0:
                        case VQA_ID_SND1:
                        case VQA_ID_SND2:
                            DecodeAudio(framePtr, subSize, subId);
                            break;
                    }

                    // Move to next sub-chunk (pad to even)
                    framePtr += subSize;
                    if (subSize & 1) framePtr++;
                }

                return true;
            }
        }

        // Move to next chunk (pad to even)
        ptr += chunkSize;
        if (chunkSize & 1) ptr++;
    }

    return false;
}

bool VQAPlayer::DecodeCodebook(const uint8_t* data, uint32_t size, bool compressed, bool partial) {
    if (!data || size == 0) return false;

    uint8_t* dst = codebook_;
    int dstSize = codebookSize_;

    if (partial) {
        // Partial codebook updates append to existing
        // Find end of current codebook based on entries
        int blockSize = header_.blockWidth * header_.blockHeight;
        if (blockSize == 0) blockSize = 8;
        dst = codebook_ + codebookEntries_ * blockSize;
        dstSize = codebookSize_ - codebookEntries_ * blockSize;
    }

    if (compressed) {
        int decompSize = DecompressLCW(data, decompBuffer_, size, decompBufferSize_);
        if (decompSize > 0 && decompSize <= dstSize) {
            memcpy(dst, decompBuffer_, decompSize);

            int blockSize = header_.blockWidth * header_.blockHeight;
            if (blockSize == 0) blockSize = 8;
            if (partial) {
                codebookEntries_ += decompSize / blockSize;
            } else {
                codebookEntries_ = decompSize / blockSize;
            }
        }
    } else {
        int copySize = std::min((int)size, dstSize);
        memcpy(dst, data, copySize);

        int blockSize = header_.blockWidth * header_.blockHeight;
        if (blockSize == 0) blockSize = 8;
        if (partial) {
            codebookEntries_ += copySize / blockSize;
        } else {
            codebookEntries_ = copySize / blockSize;
        }
    }

    return true;
}

bool VQAPlayer::DecodePointers(const uint8_t* data, uint32_t size, uint32_t chunkId) {
    if (!data || size == 0) return false;

    const uint8_t* pointerData = data;
    int pointerSize = size;

    // Decompress if needed
    if (chunkId == VQA_ID_VPTZ || chunkId == VQA_ID_VPRZ) {
        int decompSize = DecompressLCW(data, decompBuffer_, size, decompBufferSize_);
        if (decompSize <= 0) return false;
        pointerData = decompBuffer_;
        pointerSize = decompSize;
    }

    // Further RLE decompress VPTR/VPRZ
    if (chunkId == VQA_ID_VPTR || chunkId == VQA_ID_VPRZ) {
        // VPTR uses additional RLE compression
        uint8_t* rleBuffer = decompBuffer_ + decompBufferSize_ / 2;
        int rleSize = DecompressRLE(pointerData, rleBuffer, pointerSize, decompBufferSize_ / 2);
        if (rleSize <= 0) return false;
        pointerData = rleBuffer;
        pointerSize = rleSize;
    }

    // Decode using vector quantization
    UnVQ_4x2(pointerData, pointerSize / 2);  // 2 bytes per pointer

    return true;
}

bool VQAPlayer::DecodePalette(const uint8_t* data, uint32_t size, bool compressed) {
    if (!data || size == 0) return false;

    if (compressed) {
        int decompSize = DecompressLCW(data, decompBuffer_, size, decompBufferSize_);
        if (decompSize < 768) return false;

        // Scale 6-bit to 8-bit
        for (int i = 0; i < 768; i++) {
            palette_[i] = (decompBuffer_[i] & 0x3F) << 2;
        }
    } else {
        if (size < 768) return false;

        // Scale 6-bit to 8-bit
        for (int i = 0; i < 768; i++) {
            palette_[i] = (data[i] & 0x3F) << 2;
        }
    }

    paletteChanged_ = true;
    return true;
}

bool VQAPlayer::DecodeAudio(const uint8_t* data, uint32_t size, uint32_t chunkId) {
    if (!data || size == 0 || !audioBuffer_) return false;

    // IMA ADPCM step table
    static const int stepTable[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };

    static const int indexTable[16] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    audioSamplesReady_ = 0;

    if (chunkId == VQA_ID_SND0) {
        // Uncompressed audio
        int samples = size / 2;  // 16-bit
        if (samples > audioBufferSize_) samples = audioBufferSize_;
        memcpy(audioBuffer_, data, samples * 2);
        audioSamplesReady_ = samples;
    } else if (chunkId == VQA_ID_SND2) {
        // IMA ADPCM
        int16_t predictor = audioPredictor_;
        int stepIndex = audioStepIndex_;
        int sampleIdx = 0;

        for (uint32_t i = 0; i < size && sampleIdx < audioBufferSize_; i++) {
            uint8_t byte = data[i];

            for (int nibbleIdx = 0; nibbleIdx < 2 && sampleIdx < audioBufferSize_; nibbleIdx++) {
                uint8_t nibble = (nibbleIdx == 0) ? (byte & 0x0F) : ((byte >> 4) & 0x0F);

                int step = stepTable[stepIndex];
                int diff = step >> 3;
                if (nibble & 1) diff += step >> 2;
                if (nibble & 2) diff += step >> 1;
                if (nibble & 4) diff += step;
                if (nibble & 8) diff = -diff;

                int newPred = predictor + diff;
                if (newPred > 32767) newPred = 32767;
                if (newPred < -32768) newPred = -32768;
                predictor = (int16_t)newPred;

                int newIdx = stepIndex + indexTable[nibble];
                if (newIdx < 0) newIdx = 0;
                if (newIdx > 88) newIdx = 88;
                stepIndex = newIdx;

                audioBuffer_[sampleIdx++] = predictor;
            }
        }

        audioPredictor_ = predictor;
        audioStepIndex_ = stepIndex;
        audioSamplesReady_ = sampleIdx;
    }

    return true;
}

//===========================================================================
// Vector Quantization Decoder (4x2 blocks)
//===========================================================================

void VQAPlayer::UnVQ_4x2(const uint8_t* pointers, int pointerCount) {
    if (!frameBuffer_ || !codebook_ || !pointers) return;

    int blockWidth = header_.blockWidth > 0 ? header_.blockWidth : 4;
    int blockHeight = header_.blockHeight > 0 ? header_.blockHeight : 2;
    int blockSize = blockWidth * blockHeight;

    int blocksX = header_.width / blockWidth;
    int blocksY = header_.height / blockHeight;
    int totalBlocks = blocksX * blocksY;

    const uint16_t* ptrData = (const uint16_t*)pointers;
    int blockIdx = 0;

    for (int i = 0; i < pointerCount && blockIdx < totalBlocks; i++) {
        uint16_t ptr = ptrData[i];

        // Handle special pointer codes
        if ((ptr & 0xFF00) == 0xFF00) {
            // Skip blocks
            int skipCount = ptr & 0x00FF;
            blockIdx += skipCount;
        } else if ((ptr & 0xFF00) == 0xFE00) {
            // Fill with solid color
            int color = ptr & 0x00FF;
            int bx = blockIdx % blocksX;
            int by = blockIdx / blocksX;
            int px = bx * blockWidth;
            int py = by * blockHeight;

            for (int y = 0; y < blockHeight && py + y < header_.height; y++) {
                uint8_t* dst = frameBuffer_ + (py + y) * header_.width + px;
                for (int x = 0; x < blockWidth && px + x < header_.width; x++) {
                    dst[x] = color;
                }
            }
            blockIdx++;
        } else {
            // Normal codebook lookup
            int cbIndex = ptr;
            if (cbIndex < codebookEntries_) {
                const uint8_t* block = codebook_ + cbIndex * blockSize;
                int bx = blockIdx % blocksX;
                int by = blockIdx / blocksX;
                int px = bx * blockWidth;
                int py = by * blockHeight;

                for (int y = 0; y < blockHeight && py + y < header_.height; y++) {
                    uint8_t* dst = frameBuffer_ + (py + y) * header_.width + px;
                    const uint8_t* src = block + y * blockWidth;
                    for (int x = 0; x < blockWidth && px + x < header_.width; x++) {
                        dst[x] = src[x];
                    }
                }
            }
            blockIdx++;
        }
    }
}

//===========================================================================
// Audio Access
//===========================================================================

int VQAPlayer::GetAudioSamples(int16_t* buffer, int maxSamples) {
    if (!buffer || maxSamples <= 0 || audioSamplesReady_ <= 0) {
        return 0;
    }

    int samples = std::min(audioSamplesReady_, maxSamples);
    memcpy(buffer, audioBuffer_, samples * sizeof(int16_t));
    return samples;
}

//===========================================================================
// Global Functions
//===========================================================================

bool VQA_Play(const char* filename) {
    VQAPlayer player;
    if (!player.Load(filename)) {
        return false;
    }

    player.Play();
    while (player.GetState() == VQAState::PLAYING) {
        if (!player.NextFrame()) {
            break;
        }
        // In a real implementation, we'd render the frame and wait
    }

    return true;
}

bool VQA_PlayWithCallback(const char* filename, VQAFrameCallback callback, void* userData) {
    if (!callback) return false;

    VQAPlayer player;
    if (!player.Load(filename)) {
        return false;
    }

    player.Play();
    while (player.GetState() == VQAState::PLAYING) {
        if (!player.NextFrame()) {
            break;
        }

        if (!callback(player.GetFrameBuffer(), player.GetPalette(),
                      player.GetWidth(), player.GetHeight(), userData)) {
            break;
        }
    }

    return true;
}
