# WWFLAT32 Directory Research

## Overview

**Purpose:** 32-bit flat-model optimized library with heavy x86 assembly optimization

**Statistics:**
- Total files: ~400
- Assembly files: 118 (29.5% of codebase!)
- C++ files: 76
- Headers: 94

**Why This Exists:** WWFLAT32 contains the performance-critical routines in hand-optimized x86 assembly. WIN32LIB wraps these for Windows API integration.

---

## Key Directories

### VIDEO/ (9 files)
**Purpose:** Video mode management

**Key Files:**
- `VESAREAL.ASM` - Real mode VESA BIOS wrapper
- `VESAHOOK.ASM` - Protected→real mode transition
- `VERTBLNK.ASM` - VBlank detection
- `VIDEO.CPP` - Video mode enumeration

**Supported Modes:**
- CGA 320x200 4-color
- EGA 320x200 16-color
- MCGA/VGA 320x200 256-color (Mode 13h)
- VESA: 640x400, 640x480, 800x600, 1024x768

**macOS:** Completely replaced by Metal

---

### MCGAPRIM/ (48 files) - LARGEST
**Purpose:** Core 2D graphics primitives

**Assembly Files (performance-critical):**
- `BITBLIT.ASM` - Blitting with Cohen-Sutherland clipping
- `SCALE.ASM` - Bilinear/nearest scaling
- `REMAP.ASM` - Palette remapping (team colors)
- `SHADOW.ASM` - Shadow rendering
- `STAMP.ASM` - Icon/sprite stamping
- `CLEAR.ASM` - Fast memory clearing
- `DRAWLINE.ASM` - Line drawing
- `FILLRECT.ASM` - Rectangle filling
- `TXTPRNT.ASM` - Text rendering
- `GETPIX/PUTPIX.ASM` - Pixel access
- `TOBUFF/TOPAGE.ASM` - Buffer transfers
- `V*.ASM` - VESA-optimized variants

**Dual Implementation:**
- Linear framebuffer (MCGA_*)
- VESA banked memory (Vesa_*)
- Function pointers switch at runtime

**macOS:** All replaced by Metal shaders

---

### SHAPE/ (20 files)
**Purpose:** Westwood sprite format rendering

**Key Files:**
- `DRAWSHP.ASM` - Master shape dispatcher
- `DS_DN.ASM` - Draw Normal
- `DS_DR.ASM` - Draw Remapped
- `DS_DS.ASM` - Draw Scaled
- `DS_DSR.ASM` - Draw Scaled Remapped
- `DS_RS.ASM` - Draw Reversed (flip)
- `DS_TABLE.ASM` - Function dispatch table
- `SETSHAPE.ASM` - Shape buffer setup
- `GETSHAPE.CPP` - Shape extraction
- `PRIOINIT.CPP` - Priority system

**Shape Format:**
- 256-color compressed (LCW)
- 16-color compact
- Uncompressed
- Variable bit-depth

**Rendering Flags:**
- Flip (H/V), Scale, Center
- Fading, Predator (cloaking), Ghost
- Shadow, Priority, Color remap

**macOS:** Keep format decoder, rewrite renderer in Metal

---

### AUDIO/ (25 files)
**Purpose:** Audio playback with HMI SOS

**Key Files:**
- `SOSCODEC.ASM` - 4-bit ADPCM codec
- `AUDUNCMP.ASM` - AUD file decompression
- `SOUNDINT.CPP/H` - Sound interface
- `SOUNDIO.CPP` - Sound I/O
- `SOS*.H` - HMI Sound Operating System headers

**AUD Format:**
```c
struct AUDHeader {
    short Rate;           // Sample rate (Hz)
    long Size;            // Compressed size
    long UncompSize;      // Uncompressed size
    unsigned char Flags;  // 1=stereo, 2=16-bit
    unsigned char Compression;
};
```

**Supported Hardware (1996):**
- Sound Blaster / Pro
- Pro-Audio Spectrum
- Adlib / Adlib Gold
- Tandy, PC Speaker

**macOS:** CoreAudio + custom ADPCM decoder

---

### KEYBOARD/ (14 files)
**Purpose:** Keyboard/mouse input via interrupts

**Key Files:**
- `KEYBOARD.ASM` - Keyboard handler
- `KEYIPROT.ASM` - Protected mode interrupt
- `KEYIREAL.ASM` - Real mode wrapper
- `MOUSE.ASM` - Mouse integration
- `PAGFAULT.ASM` - Page fault handler

**macOS:** AppKit NSEvent (completely different model)

---

### IFF/ (15 files)
**Purpose:** Image format support

**Key Files:**
- `LCWUNCMP.ASM` - LCW decompression
- `LCWCOMP.ASM` - LCW compression
- `IFF.CPP` - IFF/ILBM support
- `LOADPCX.CPP` - PCX loading

**LCW Compression Format:**
```
0xxxyyyy,yyyyyyyy  - Back y bytes, run x+3
10xxxxxx,n1,n2...  - Copy next x+1 bytes
11xxxxxx,w1        - Run x+3 from offset w1
11111111,w1,w2     - Copy w1 bytes from w2
11111110,w1,b1     - Run byte b1 for w1
10000000           - End of data
```

**macOS:** Port LCW to C++, useful for game assets

---

### WSA/ (4 files)
**Purpose:** Westwood Animation format

**Key Files:**
- `XORDELTA.ASM` - XOR delta decompression
- `WSA.CPP/H` - Animation player

**macOS:** Port XOR delta to C++

---

### Other Directories

| Directory | Files | Purpose |
|-----------|-------|---------|
| SVGAPRIM/ | 15 | SVGA-optimized primitives |
| FONT/ | 10 | Font rendering |
| MISC/ | 27 | Utilities (CRC, RNG, fading) |
| FILE/ | 23 | DOS file I/O |
| TIMER/ | 9 | Timer interrupts |
| PALETTE/ | 6 | VGA DAC programming |
| MEM/ | 11 | Memory management |
| TILE/ | 5 | Tile/icon sets |
| MONO/ | 7 | Debug monochrome output |
| PLAYCD/ | 7 | CD audio |
| DESCMGMT/ | 7 | DPMI descriptor tables |
| INCLUDE/ | 55 | Public headers |

---

## Notable Assembly Programmers

From file headers:
- **Julio R. Jerez** - BITBLIT.ASM, memory, VESA
- **Phil W. Gorrow** - SCALE.ASM, REMAP.ASM, audio, shapes
- **Joe L. Bostic** - FADING.ASM, core algorithms
- **Scott K. Bowen** - XORDELTA.ASM, WSA
- **Bill Randolph** - Shape system design
- **Christopher Yates** - LCW compression (1990)
- **HMI, Inc.** - SOS audio codec

---

## Assembly Optimization Techniques

1. **Register Discipline**
   - esi/edi for source/dest pointers
   - ecx for loop counters
   - Minimal stack usage

2. **Cohen-Sutherland Clipping**
   - Reference: "Computer Graphics: Principles and Practice" (Foley et al.)
   - Efficient bit-field region codes

3. **String Instructions**
   - REP MOVSB/MOVSW/MOVSD for bulk copies
   - REP STOSB for clearing

4. **Pentium Optimizations**
   - /5s compiler flag
   - Instruction pairing
   - Cache-line alignment

5. **VESA Banking**
   - Pre-computed bank table
   - Minimizes real mode switches

---

## Port Strategy

### Completely Obsolete
- VIDEO/ - VGA/VESA modes
- MCGAPRIM/, SVGAPRIM/ - Software rasterization
- KEYBOARD/ - Hardware interrupts
- FILE/ - DOS interrupts
- DESCMGMT/ - DPMI
- MONO/ - Debug hardware
- PLAYCD/ - CD audio hardware

### Keep Algorithm, Rewrite Platform
- SHAPE/ - Keep format, Metal renderer
- AUDIO/ - Keep codec, CoreAudio playback
- IFF/ - Keep LCW decoder
- WSA/ - Keep XOR delta
- TIMER/ - Keep concepts, mach_absolute_time()

### Still Useful
- LCW compression algorithm
- ADPCM audio codec
- XOR delta animation
- Shape format specification
- Facing calculations (game logic)

---

## Performance Context

**1996 Target:**
- Pentium 75-100 MHz
- 8-16 MB RAM
- Software rendering at 320x200

**Modern Advantage:**
- CPU 100-200x faster
- GPU 1000x+ faster
- Assembly optimizations mostly irrelevant
- Cache hierarchy more important

**What Still Matters:**
- Algorithm complexity
- Memory access patterns
- GPU batch rendering
- Asset decompression (CPU-bound)

---

## Recommendations

**DO:**
- Preserve asset formats (LCW, AUD, WSA, Shape)
- Port decoders to portable C++
- Use Metal for all rendering
- Keep game logic bit-identical

**DON'T:**
- Try to preserve assembly
- Use palette-based rendering
- Access hardware directly
- Use real mode/DPMI

---

## Historical Significance

WWFLAT32 represents peak DOS game development (1994-1996):
- Last generation before hardware 3D
- Hand-optimized assembly for every hot path
- Deep x86 and VGA hardware knowledge
- Transition era: DOS → Windows 95

This library is a time capsule of mid-1990s PC game engineering craft.
