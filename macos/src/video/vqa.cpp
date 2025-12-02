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
// LCW Decompression (Format80) - Based on OpenRA implementation
//===========================================================================

int VQAPlayer::DecompressLCW(const uint8_t* src, uint8_t* dst,
                             int srcSize, int dstSize) {
    const uint8_t* srcEnd = src + srcSize;
    int destIndex = 0;

    while (src < srcEnd && destIndex < dstSize) {
        uint8_t cmd = *src++;

        if ((cmd & 0x80) == 0) {
            // Case 2: Copy from relative position in output
            // 0CCCPPPP PPPPPPPP - copy (CCC + 3) bytes from dst[current - PPP]
            if (src >= srcEnd) break;
            uint8_t secondByte = *src++;
            int count = ((cmd & 0x70) >> 4) + 3;
            int rpos = ((cmd & 0x0F) << 8) + secondByte;

            if (destIndex + count > dstSize) break;

            // Copy bytes, one at a time (handles overlapping)
            int srcIdx = destIndex - rpos;
            if (srcIdx < 0) break;
            for (int i = 0; i < count; i++) {
                if (destIndex - srcIdx == 1)
                    dst[destIndex + i] = dst[destIndex - 1];
                else
                    dst[destIndex + i] = dst[srcIdx + i];
            }
            destIndex += count;
        } else if ((cmd & 0x40) == 0) {
            // Case 1: Literal copy
            // 10CCCCCC - copy C bytes from source (C=0 = end marker)
            int count = cmd & 0x3F;
            if (count == 0) break; // End marker

            if (src + count > srcEnd || destIndex + count > dstSize) break;
            memcpy(dst + destIndex, src, count);
            src += count;
            destIndex += count;
        } else {
            int count3 = cmd & 0x3F;
            if (count3 == 0x3E) {
                // Case 4: Fill with byte value
                // 11111110 LLLLLLLL LLLLLLLL VVVVVVVV - fill L bytes with V
                if (src + 3 > srcEnd) break;
                int count = src[0] | (src[1] << 8);
                src += 2;
                uint8_t color = *src++;

                if (destIndex + count > dstSize) break;
                memset(dst + destIndex, color, count);
                destIndex += count;
            } else {
                // Case 3 or 5: Copy from absolute position
                // Case 3: 11CCCCCC PP PP - copy (C+3) from abs P
                // Case 5: 11111111 LL LL PP PP - copy L from abs P
                int count;
                if (count3 == 0x3F) {
                    // Case 5: Long copy
                    if (src + 4 > srcEnd) break;
                    count = src[0] | (src[1] << 8);
                    src += 2;
                } else {
                    // Case 3: Short copy
                    count = count3 + 3;
                }

                if (src + 2 > srcEnd) break;
                int srcIndex = src[0] | (src[1] << 8);
                src += 2;

                if (srcIndex >= destIndex || destIndex + count > dstSize) break;

                // Copy bytes one at a time
                for (int i = 0; i < count; i++) {
                    dst[destIndex++] = dst[srcIndex++];
                }
            }
        }
    }

    return destIndex;
}

//===========================================================================
// RLE Decompression for Vector Pointers
//===========================================================================

int VQAPlayer::DecompressRLE(const uint8_t* src, uint8_t* dst,
                             int srcSize, int dstSize) {
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
    , cbpBuffer_(nullptr)
    , cbpBufferSize_(0)
    , cbpOffset_(0)
    , cbpCount_(0)
    , cbpIsCompressed_(false)
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

    delete[] cbpBuffer_;
    cbpBuffer_ = nullptr;
    cbpBufferSize_ = 0;
    cbpOffset_ = 0;
    cbpCount_ = 0;
    cbpIsCompressed_ = false;

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
        printf("VQA: ParseHeader - invalid data or size (%u)\n", dataSize_);
        return false;
    }

    const uint8_t* ptr = data_;
    const uint8_t* end = data_ + dataSize_;

    // Check FORM header
    const IFFChunk* form = (const IFFChunk*)ptr;
    uint32_t formId = SwapBE32(form->id);
    if (formId != VQA_ID_FORM) {
        printf("VQA: bad FORM: 0x%08X (want 0x%08X)\n",
               formId, VQA_ID_FORM);
        printf("VQA: First 16 bytes: "
               "%02X %02X %02X %02X %02X %02X %02X %02X "
               "%02X %02X %02X %02X %02X %02X %02X %02X\n",
               data_[0], data_[1], data_[2], data_[3],
               data_[4], data_[5], data_[6], data_[7],
               data_[8], data_[9], data_[10], data_[11],
               data_[12], data_[13], data_[14], data_[15]);
        return false;
    }
    ptr += 8;

    // Check WVQA type
    uint32_t type = SwapBE32(*(uint32_t*)ptr);
    if (type != VQA_ID_WVQA) {
        printf("VQA: bad WVQA: 0x%08X (want 0x%08X)\n",
               type, VQA_ID_WVQA);
        return false;
    }
    ptr += 4;
    printf("VQA: ParseHeader - FORM/WVQA OK\n");

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
    int cbEntries = header_.cbEntries > 0 ? header_.cbEntries : 0x10000;
    codebookSize_ = cbEntries * blockSize;
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

    // Allocate CBP accumulation buffer (codebook size for partial updates)
    cbpBufferSize_ = codebookSize_;
    cbpBuffer_ = new uint8_t[cbpBufferSize_];
    memset(cbpBuffer_, 0, cbpBufferSize_);
    cbpOffset_ = 0;
    cbpCount_ = 0;
    cbpIsCompressed_ = false;

    return true;
}

bool VQAPlayer::ParseFrameIndex() {
    // FINF contains 32-bit offsets for each frame
    // Already positioned at FINF data

    if (header_.frames == 0) {
        return false;
    }

    frameOffsets_ = new uint32_t[header_.frames];

    // Note: FINF offsets stored differently in different VQA versions
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

    // Note: ADPCM state persists across frames as video audio is
    // one continuous ADPCM stream (matches OpenRA behavior).

    // Apply accumulated partial codebook at START of frame
    // (CBP chunks from prior frames applied before this frame)
    ApplyAccumulatedCodebook();

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

        // Handle audio chunks at top level (they come BEFORE their VQFR/VQFK)
        // Audio at currentFrameIndex=-1 belongs to frame 0
        // Audio at currentFrameIndex=0 belongs to frame 1, etc.
        // So: audio belongs to frame (currentFrameIndex + 1)
        // We need to decode ALL audio chunks for the target frame
        bool isAudio = (chunkId == VQA_ID_SND0 ||
                        chunkId == VQA_ID_SND1 ||
                        chunkId == VQA_ID_SND2);
        if (isAudio) {
            if (currentFrameIndex + 1 == frameNum) {
                // Audio for target frame - decode and accumulate
                DecodeAudio(ptr, chunkSize, chunkId);
            }
        }

        // Is this a frame chunk?
        if (chunkId == VQA_ID_VQFR || chunkId == VQA_ID_VQFK) {
            currentFrameIndex++;

            // If we just passed our target frame, we're done
            if (currentFrameIndex > frameNum) {
                return true;
            }

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

bool VQAPlayer::DecodeCodebook(const uint8_t* data, uint32_t size,
                               bool compressed, bool partial) {
    if (!data || size == 0) return false;

    int blockSize = header_.blockWidth * header_.blockHeight;
    if (blockSize == 0) blockSize = 8;

    if (partial) {
        // Partial codebook update (CBP0/CBPZ) - accumulate chunks
        // Chunks collected across frames and applied together
        // when all parts are received (determined by header_.groupSize)

        // Append to CBP accumulation buffer
        if (cbpOffset_ + (int)size <= cbpBufferSize_) {
            memcpy(cbpBuffer_ + cbpOffset_, data, size);
            cbpOffset_ += size;
        }
        cbpCount_++;
        cbpIsCompressed_ = compressed;  // Track for final decompress

        return true;
    }

    // Full codebook (CBF0/CBFZ) - decompress and replace immediately
    // Also reset CBP accumulation state since we have a new full codebook
    cbpOffset_ = 0;
    cbpCount_ = 0;

    if (compressed) {
        int ds = DecompressLCW(data, decompBuffer_, size, decompBufferSize_);
        if (ds > 0 && ds <= codebookSize_) {
            memcpy(codebook_, decompBuffer_, ds);
            codebookEntries_ = ds / blockSize;
        } else {
            return false;
        }
    } else {
        int copySize = std::min((int)size, codebookSize_);
        memcpy(codebook_, data, copySize);
        codebookEntries_ = copySize / blockSize;
    }

    return true;
}

void VQAPlayer::ApplyAccumulatedCodebook() {
    // Apply accumulated CBP chunks if we have collected enough parts
    // groupSize tells how many CBP chunks make a complete codebook

    int partsNeeded = header_.groupSize;
    if (partsNeeded == 0) partsNeeded = 8;  // Default if not specified

    if (cbpCount_ >= partsNeeded && cbpOffset_ > 0) {
        int blockSize = header_.blockWidth * header_.blockHeight;
        if (blockSize == 0) blockSize = 8;

        if (cbpIsCompressed_) {
            // Decompress the accumulated CBPZ data
            int ds = DecompressLCW(cbpBuffer_, decompBuffer_,
                                   cbpOffset_, decompBufferSize_);
            if (ds > 0 && ds <= codebookSize_) {
                memcpy(codebook_, decompBuffer_, ds);
                codebookEntries_ = ds / blockSize;
            }
        } else {
            // CBP0 - uncompressed, just copy
            int copySize = std::min(cbpOffset_, codebookSize_);
            memcpy(codebook_, cbpBuffer_, copySize);
            codebookEntries_ = copySize / blockSize;
        }

        // Reset accumulation state
        cbpOffset_ = 0;
        cbpCount_ = 0;
    }
}

bool VQAPlayer::DecodePointers(const uint8_t* data, uint32_t size,
                               uint32_t chunkId) {
    if (!data || size == 0) return false;

    const uint8_t* pointerData = data;
    int pointerSize = size;

    // Decompress if needed
    if (chunkId == VQA_ID_VPTZ || chunkId == VQA_ID_VPRZ) {
        int ds = DecompressLCW(data, decompBuffer_, size, decompBufferSize_);
        if (ds <= 0) return false;
        pointerData = decompBuffer_;
        pointerSize = ds;
    }

    // Further RLE decompress VPTR/VPRZ
    if (chunkId == VQA_ID_VPTR || chunkId == VQA_ID_VPRZ) {
        // VPTR uses additional RLE compression
        int halfSize = decompBufferSize_ / 2;
        uint8_t* rleBuffer = decompBuffer_ + halfSize;
        int rleSize = DecompressRLE(pointerData, rleBuffer,
                                    pointerSize, halfSize);
        if (rleSize <= 0) return false;
        pointerData = rleBuffer;
        pointerSize = rleSize;
    }

    // Calculate block counts
    int blockWidth = header_.blockWidth > 0 ? header_.blockWidth : 4;
    int blockHeight = header_.blockHeight > 0 ? header_.blockHeight : 2;
    int blocksX = header_.width / blockWidth;
    int blocksY = header_.height / blockHeight;
    int totalBlocks = blocksX * blocksY;

    // Decode using vector quantization
    // pointerSize should be 2 * totalBlocks (lo bytes + hi bytes)
    UnVQ_4x2(pointerData, totalBlocks);

    return true;
}

bool VQAPlayer::DecodePalette(const uint8_t* data, uint32_t size,
                              bool compressed) {
    if (!data || size == 0) return false;

    const uint8_t* palData = data;

    if (compressed) {
        int ds = DecompressLCW(data, decompBuffer_, size, decompBufferSize_);
        if (ds < 768) {
            return false;
        }
        palData = decompBuffer_;
    } else {
        if (size < 768) {
            return false;
        }
    }

    // VQA palette is 6-bit VGA format (0-63)
    // Scale to 8-bit (0-255) by multiplying by 4
    for (int i = 0; i < 768; i++) {
        uint8_t val = palData[i];
        // Check if already 8-bit (values > 63) or 6-bit
        if (val > 63) {
            palette_[i] = val;
        } else {
            // 6-bit, scale up: (val << 2) | (val >> 4) gives proper scaling
            palette_[i] = (val << 2) | (val >> 4);
        }
    }

    paletteChanged_ = true;
    return true;
}

bool VQAPlayer::DecodeAudio(const uint8_t* data, uint32_t size,
                            uint32_t chunkId) {
    if (!data || size == 0 || !audioBuffer_) return false;

    // FIXME: VQA audio still has minor static/distortion artifacts.
    // Possible causes:
    // 1. Buffer underruns during real-time streaming (video decodes per-frame,
    //    audio consumes continuously at 44100 Hz)
    // 2. ADPCM decoder differences from original Westwood implementation
    // 3. Resampling (22050->44100) may introduce artifacts
    // 4. Timing jitter between video frame decode and audio consumption
    // Consider: pre-buffer more audio, or decode all upfront like OpenRA

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

    // Don't reset audioSamplesReady_ - append to existing samples
    // This allows multiple audio chunks per frame to be accumulated

    if (chunkId == VQA_ID_SND0) {
        // Uncompressed audio - append to buffer
        int samples = size / 2;  // 16-bit
        int remaining = audioBufferSize_ - audioSamplesReady_;
        if (samples > remaining) samples = remaining;
        if (samples > 0) {
            memcpy(audioBuffer_ + audioSamplesReady_, data, samples * 2);
            audioSamplesReady_ += samples;
        }
    } else if (chunkId == VQA_ID_SND2) {
        // IMA ADPCM - decode and append to buffer
        int16_t predictor = audioPredictor_;
        int stepIndex = audioStepIndex_;
        int sampleIdx = audioSamplesReady_;  // Start from current position

        for (uint32_t i = 0; i < size && sampleIdx < audioBufferSize_; i++) {
            uint8_t byte = data[i];

            for (int ni = 0; ni < 2 && sampleIdx < audioBufferSize_; ni++) {
                uint8_t nibble = ni == 0 ? byte & 0x0F : (byte >> 4) & 0x0F;

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

    // VQA stores pointer data as two halves:
    // - First half: low bytes (block index or color)
    // - Second half: high bytes (modifier)
    // Combined as: (mod * 256 + px) for codebook lookup
    // Special case: mod == 0x0F means px is a literal palette color
    const uint8_t* lowBytes = pointers;
    const uint8_t* highBytes = pointers + totalBlocks;

    int maxBlocks = std::min(totalBlocks, pointerCount);
    for (int blockIdx = 0; blockIdx < maxBlocks; blockIdx++) {
        int bx = blockIdx % blocksX;
        int by = blockIdx / blocksX;
        int px = bx * blockWidth;
        int py = by * blockHeight;

        uint8_t lo = lowBytes[blockIdx];
        uint8_t hi = highBytes[blockIdx];

        if (hi == 0x0F) {
            // Special case: lo is a literal palette color - fill block with it
            for (int y = 0; y < blockHeight && py + y < header_.height; y++) {
                uint8_t* dst = frameBuffer_ + (py + y) * header_.width + px;
                for (int x = 0; x < blockWidth && px + x < header_.width; x++) {
                    dst[x] = lo;
                }
            }
        } else {
            // Normal codebook lookup: index = hi * 256 + lo
            int cbIndex = hi * 256 + lo;
            if (cbIndex < codebookEntries_ && cbIndex >= 0) {
                const uint8_t* block = codebook_ + cbIndex * blockSize;

                int yMax = std::min(blockHeight, header_.height - py);
                int xMax = std::min(blockWidth, header_.width - px);
                for (int y = 0; y < yMax; y++) {
                    uint8_t* dst = frameBuffer_ + (py + y) * header_.width + px;
                    const uint8_t* src = block + y * blockWidth;
                    for (int x = 0; x < xMax; x++) {
                        dst[x] = src[x];
                    }
                }
            }
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

bool VQA_PlayWithCallback(const char* filename, VQAFrameCallback callback,
                          void* userData) {
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
