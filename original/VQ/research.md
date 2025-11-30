# VQ/WINVQ Video Codec Research

## Overview

VQ (Vector Quantization) is Westwood's video codec for Command & Conquer cutscenes.

**Two Directories:**
- **/VQ** - DOS protected mode implementation (183 files)
- **/WINVQ** - Windows 95 implementation (216 files)

Both share core algorithms but differ in platform integration.

---

## VQA Format

### File Structure (IFF Container)

```
FORM WVQA
  ├── VQHD - Header
  ├── FINF - Frame info table
  ├── NAME - Name string (optional)
  ├── CAP0 - Captions (optional)
  └── VQFR/VQFK - Frame containers (repeated)
      ├── CBF0/CBFZ - Full codebook
      ├── CBP0/CBPZ - Partial codebook
      ├── VPT0/VPTZ - Vector pointers
      ├── CPL0/CPLZ - Palette
      └── SND0/SND1/SND2 - Audio
```

### Header (VQHD)

```c
struct VQAHeader {
    unsigned short Version;      // 1 or 2
    unsigned short Flags;        // AUDIO, ALTAUDIO
    unsigned short Frames;       // Total frames
    unsigned short ImageWidth;   // Pixels
    unsigned short ImageHeight;  // Pixels
    unsigned char BlockWidth;    // Typically 4
    unsigned char BlockHeight;   // Typically 2
    unsigned char FPS;           // Frames per second
    unsigned char Groupsize;     // Frames per codebook
    unsigned short CBentries;    // Codebook entries
    unsigned short SampleRate;   // Audio Hz
    unsigned char Channels;      // 1=mono, 2=stereo
    unsigned char BitsPerSample; // 8 or 16
};
```

### Vector Quantization Encoding

1. **Codebook** - Array of pixel blocks (e.g., 4x2 pixels)
   - Full codebook (CBF): Complete replacement
   - Partial codebook (CBP): Update subset

2. **Vector Pointers** - Indices into codebook
   - Each pointer = which block to draw
   - Special codes for solid colors, runs, dumps

3. **Common Block Sizes:**
   - 2x2, 2x3, **4x2** (most common), 4x4

---

## Compression Layers

| Method | Purpose |
|--------|---------|
| LCW | Codebooks, palettes, pointers |
| RSD | Run-Skip-Dump for vector pointers |
| ZAP | Audio (proprietary) |
| ADPCM | Audio (SND2 chunks) |

---

## Key Files

### VQA32/ - Player Library

| File | Purpose |
|------|---------|
| VQAPLAY.H | Public API |
| VQAFILE.H | Format definitions |
| LOADER.CPP | Stream loading |
| DRAWER.CPP | Frame rendering |
| AUDIO.CPP | Audio playback |
| CAPTION.CPP | Subtitle rendering |
| UNVQBUFF.ASM | VQ decompression (assembly) |

### VQM32/ - Support Library

| File | Purpose |
|------|---------|
| IFF.CPP | IFF parsing |
| HUFFCMP.CPP | Huffman codec |
| SOSCODEC.ASM | Audio codec |
| MIXFILE.CPP | MIX archive support |

### HMI Integration

HMI's Sound Operating System (SOS) headers in `/VQ/INCLUDE/HMI32/`:
- SOS.H - Main API
- SOSDEFS.H - Definitions
- SOSFNCT.H - Functions

Provides hardware abstraction for DOS sound cards.

---

## WINVQ Differences

### VQAVIEW - Windows Viewer
- MAIN.CPP - Entry point
- MAINWIND.CPP - Window handling
- VQ.CPP - Playback integration
- PAL.CPP - Palette management

Uses Windows GDI/DirectDraw instead of direct VGA access.

---

## macOS Port Options

### Option 1: FFmpeg (Recommended)

**FFmpeg already has VQA support** in libavcodec!

```c
avformat_open_input(&ctx, "movie.vqa", NULL, NULL);
avcodec_find_decoder(AV_CODEC_ID_VQA);
// Decode to RGB, upload to Metal texture
// Route audio to CoreAudio
```

**Pros:**
- Mature, tested implementation
- Cross-platform
- Simple integration

**Cons:**
- External dependency
- Larger binary

### Option 2: Custom Decoder

Port VQA32 library to modern C/C++.

**Required Work:**
1. Replace HMI audio → CoreAudio
2. Replace DOS file I/O → POSIX
3. Port x86 ASM → C or ARM NEON
4. Replace VGA output → Metal
5. Port cooperative multitasking → GCD

**Pros:**
- No dependencies
- Full control

**Cons:**
- Significant effort
- Ongoing maintenance

### Option 3: Pre-convert Videos

Convert VQA → MP4/H.264 offline, use AVFoundation.

**Pros:**
- Native macOS playback
- Hardware acceleration

**Cons:**
- Modified game assets
- Storage increase

---

## Recommendation

**Use FFmpeg (Option 2)** for initial port:
1. FFmpeg's VQA decoder handles edge cases
2. Focus on game logic, not codec archaeology
3. Can statically link to avoid runtime deps
4. Easy upgrade path if needed

---

## Audio Timing

VQA uses audio for precise frame timing:
```c
VQAConfig.TimerMethod = VQA_TMETHOD_AUDIO;
```

HMI callbacks when buffers need refill provide sync.

**macOS equivalent:** CoreAudio completion callbacks or AVAudioEngine scheduling.

---

## Frame Types

From FINF flags:
- **Key Frame** (VQAFINF_KEY) - Full codebook, must draw
- **Delta Frame** - References previous codebook
- **Palette Frame** (VQAFINF_PAL) - Palette update
- **Sync Frame** (VQAFINF_SYNC) - Sync marker

---

## Summary

VQA is a 1990s-era video codec optimized for:
- Low CPU (Pentium 75 MHz target)
- 8-bit palette graphics
- DOS direct hardware access

For macOS port, FFmpeg provides the path of least resistance. Custom decoder is possible but significant work.
