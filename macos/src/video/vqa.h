/**
 * Red Alert macOS Port - VQA Video Player
 *
 * Plays Westwood VQA (Vector Quantized Animation) files.
 * VQA uses IFF container format with vector quantization video codec.
 *
 * File structure:
 *   FORM/WVQA container
 *   ├── VQHD - Header (40 bytes)
 *   ├── FINF - Frame index table
 *   ├── VQFK - Key frames (full codebook)
 *   │   ├── CBF0/CBFZ - Codebook (uncompressed/LCW)
 *   │   ├── VPT0/VPTZ/VPTR - Vector pointers
 *   │   ├── CPL0/CPLZ - Palette
 *   │   └── SND0/SND1/SND2 - Audio
 *   └── VQFR - Delta frames (partial codebook updates)
 *
 * Video: 8-bit palettized, 4x2 blocks, vector quantization
 * Audio: PCM, Zap, or IMA ADPCM compressed
 */

#ifndef VIDEO_VQA_H
#define VIDEO_VQA_H

#include <cstdint>

//===========================================================================
// Constants
//===========================================================================

// IFF chunk IDs (stored as big-endian in files)
constexpr uint32_t VQA_ID_FORM = 0x464F524D;  // 'FORM'
constexpr uint32_t VQA_ID_WVQA = 0x57565141;  // 'WVQA'
constexpr uint32_t VQA_ID_VQHD = 0x56514844;  // 'VQHD'
constexpr uint32_t VQA_ID_FINF = 0x46494E46;  // 'FINF'
constexpr uint32_t VQA_ID_VQFR = 0x56514652;  // 'VQFR' - regular frame
constexpr uint32_t VQA_ID_VQFK = 0x5651464B;  // 'VQFK' - key frame

// Codebook chunks
constexpr uint32_t VQA_ID_CBF0 = 0x43424630;  // 'CBF0' - full codebook
constexpr uint32_t VQA_ID_CBFZ = 0x4342465A;  // 'CBFZ' - full codebook LCW
constexpr uint32_t VQA_ID_CBP0 = 0x43425030;  // 'CBP0' - partial codebook
constexpr uint32_t VQA_ID_CBPZ = 0x4342505A;  // 'CBPZ' - partial codebook LCW

// Vector pointer chunks
constexpr uint32_t VQA_ID_VPT0 = 0x56505430;  // 'VPT0' - pointers uncompressed
constexpr uint32_t VQA_ID_VPTZ = 0x5650545A;  // 'VPTZ' - pointers LCW
constexpr uint32_t VQA_ID_VPTR = 0x56505452;  // 'VPTR' - pointers RLE
constexpr uint32_t VQA_ID_VPRZ = 0x5650525A;  // 'VPRZ' - pointers RLE+LCW

// Palette chunks
constexpr uint32_t VQA_ID_CPL0 = 0x43504C30;  // 'CPL0' - palette
constexpr uint32_t VQA_ID_CPLZ = 0x43504C5A;  // 'CPLZ' - palette LCW

// Audio chunks
constexpr uint32_t VQA_ID_SND0 = 0x534E4430;  // 'SND0' - audio uncompressed
constexpr uint32_t VQA_ID_SND1 = 0x534E4431;  // 'SND1' - audio Zap
constexpr uint32_t VQA_ID_SND2 = 0x534E4432;  // 'SND2' - audio ADPCM

// Header flags
constexpr uint16_t VQAHDF_AUDIO     = 0x0001;  // Has primary audio
constexpr uint16_t VQAHDF_ALTAUDIO  = 0x0002;  // Has alternate audio

// Maximum values
constexpr int VQA_MAX_CODEBOOK_ENTRIES = 0x10000;  // 64K entries max
constexpr int VQA_MAX_WIDTH = 640;
constexpr int VQA_MAX_HEIGHT = 480;

//===========================================================================
// VQA Header Structure (40 bytes)
//===========================================================================

#pragma pack(push, 1)
struct VQAHeader {
    uint16_t version;           // VQA version (1 or 2)
    uint16_t flags;             // VQAHDF_* flags
    uint16_t frames;            // Total frame count
    uint16_t width;             // Image width in pixels
    uint16_t height;            // Image height in pixels
    uint8_t  blockWidth;        // VQ block width (typically 4)
    uint8_t  blockHeight;       // VQ block height (typically 2)
    uint8_t  fps;               // Frames per second
    uint8_t  groupSize;         // Frames per codebook update
    uint16_t colors1;           // Number of 1-color blocks
    uint16_t cbEntries;         // Number of codebook entries
    uint16_t xPos;              // X position (-1 = center)
    uint16_t yPos;              // Y position (-1 = center)
    uint16_t maxFrameSize;      // Largest frame size
    uint16_t sampleRate;        // Audio sample rate (Hz)
    uint8_t  channels;          // Audio channels (1=mono, 2=stereo)
    uint8_t  bitsPerSample;     // Audio bits (8 or 16)
    uint16_t altSampleRate;     // Alternate audio rate
    uint8_t  altChannels;       // Alternate audio channels
    uint8_t  altBitsPerSample;  // Alternate audio bits
    uint16_t reserved[5];       // Reserved for future use
};

// IFF chunk header
struct IFFChunk {
    uint32_t id;                // Chunk ID (big-endian)
    uint32_t size;              // Chunk size (big-endian)
};
#pragma pack(pop)

//===========================================================================
// VQA Playback State
//===========================================================================

enum class VQAState : int8_t {
    STOPPED = 0,
    PLAYING,
    PAUSED,
    FINISHED,
    ERROR
};

//===========================================================================
// VQA Player Class
//===========================================================================

class VQAPlayer {
public:
    VQAPlayer();
    ~VQAPlayer();

    // Prevent copying
    VQAPlayer(const VQAPlayer&) = delete;
    VQAPlayer& operator=(const VQAPlayer&) = delete;

    //-----------------------------------------------------------------------
    // File Operations
    //-----------------------------------------------------------------------

    // Load VQA from file
    bool Load(const char* filename);

    // Load VQA from memory
    bool Load(const void* data, uint32_t size);

    // Unload current video
    void Unload();

    // Check if video is loaded
    bool IsLoaded() const { return data_ != nullptr; }

    //-----------------------------------------------------------------------
    // Playback Control
    //-----------------------------------------------------------------------

    // Start/resume playback
    void Play();

    // Pause playback
    void Pause();

    // Stop playback (reset to beginning)
    void Stop();

    // Advance to next frame (returns false if at end)
    bool NextFrame();

    // Seek to specific frame
    bool SeekFrame(int frame);

    // Get playback state
    VQAState GetState() const { return state_; }

    //-----------------------------------------------------------------------
    // Frame Access
    //-----------------------------------------------------------------------

    // Get current frame number (0-based)
    int GetCurrentFrame() const { return currentFrame_; }

    // Get total frame count
    int GetFrameCount() const { return header_.frames; }

    // Get video dimensions
    int GetWidth() const { return header_.width; }
    int GetHeight() const { return header_.height; }

    // Get frames per second
    int GetFPS() const { return header_.fps; }

    // Get current frame buffer (8-bit palettized)
    const uint8_t* GetFrameBuffer() const { return frameBuffer_; }

    // Get current palette (256 RGB triplets)
    const uint8_t* GetPalette() const { return palette_; }

    // Check if palette changed this frame
    bool PaletteChanged() const { return paletteChanged_; }

    //-----------------------------------------------------------------------
    // Audio Access
    //-----------------------------------------------------------------------

    // Check if video has audio
    bool HasAudio() const { return (header_.flags & VQAHDF_AUDIO) != 0; }

    // Get audio parameters
    int GetAudioSampleRate() const { return header_.sampleRate; }
    int GetAudioChannels() const { return header_.channels; }
    int GetAudioBitsPerSample() const { return header_.bitsPerSample; }

    // Get audio samples for current frame (returns sample count)
    int GetAudioSamples(int16_t* buffer, int maxSamples);

    //-----------------------------------------------------------------------
    // Timing
    //-----------------------------------------------------------------------

    // Get frame duration in milliseconds
    int GetFrameDuration() const {
        return header_.fps > 0 ? 1000 / header_.fps : 67;  // ~15fps default
    }

    // Update playback based on elapsed time (ms)
    // Returns true if a new frame is ready
    bool Update(int elapsedMs);

private:
    // File data
    const uint8_t* data_;
    uint32_t dataSize_;
    bool ownsData_;

    // Parsed header
    VQAHeader header_;

    // Frame index table
    uint32_t* frameOffsets_;

    // Playback state
    VQAState state_;
    int currentFrame_;
    int timeAccumulator_;

    // Frame buffer (8-bit palettized)
    uint8_t* frameBuffer_;
    int frameBufferSize_;

    // Palette (256 * 3 bytes RGB)
    uint8_t palette_[768];
    bool paletteChanged_;

    // Codebook
    uint8_t* codebook_;
    int codebookSize_;
    int codebookEntries_;

    // Audio state
    int16_t audioPredictor_;
    int audioStepIndex_;
    int16_t* audioBuffer_;
    int audioBufferSize_;
    int audioSamplesReady_;

    // Decompression buffer
    uint8_t* decompBuffer_;
    int decompBufferSize_;

    // Partial codebook (CBP) accumulation state
    uint8_t* cbpBuffer_;        // Buffer to accumulate partial codebook chunks
    int cbpBufferSize_;         // Size of cbpBuffer_
    int cbpOffset_;             // Current write offset in cbpBuffer_
    int cbpCount_;              // Number of CBP chunks accumulated
    bool cbpIsCompressed_;      // True if chunks are CBPZ (compressed)

    //-----------------------------------------------------------------------
    // Internal Methods
    //-----------------------------------------------------------------------

    // Parse file structure
    bool ParseHeader();
    bool ParseFrameIndex();

    // Decode a frame
    bool DecodeFrame(int frameNum);

    // Decode frame components
    bool DecodeCodebook(const uint8_t* data, uint32_t size,
                        bool compressed, bool partial);
    bool DecodePointers(const uint8_t* data, uint32_t size, uint32_t chunkId);
    bool DecodePalette(const uint8_t* data, uint32_t size, bool compressed);
    bool DecodeAudio(const uint8_t* data, uint32_t size, uint32_t chunkId);

    // Apply accumulated partial codebook if complete
    void ApplyAccumulatedCodebook();

    // Vector quantization decoder
    void UnVQ_4x2(const uint8_t* pointers, int pointerCount);

    // Byte swapping for big-endian IFF format
    static uint32_t SwapBE32(uint32_t val);
    static uint16_t SwapBE16(uint16_t val);

    // LCW decompression
    static int DecompressLCW(const uint8_t* src, uint8_t* dst,
                             int srcSize, int dstSize);

    // RLE decompression for vector pointers
    static int DecompressRLE(const uint8_t* src, uint8_t* dst,
                             int srcSize, int dstSize);
};

//===========================================================================
// Global Functions
//===========================================================================

// Play a VQA file (blocking, full playback)
// Returns true if played successfully
bool VQA_Play(const char* filename);

// Play VQA with callback for each frame
// Callback receives frame buffer, palette, width, height
// Return false from callback to stop playback
typedef bool (*VQAFrameCallback)(const uint8_t* frame, const uint8_t* palette,
                                  int width, int height, void* userData);
bool VQA_PlayWithCallback(const char* filename, VQAFrameCallback callback,
                          void* userData);

#endif // VIDEO_VQA_H
