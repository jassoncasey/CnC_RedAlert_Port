# WIN32LIB Directory Research

## Overview

**Purpose:** Windows 32-bit platform abstraction library for Command & Conquer Red Alert

**Statistics:**
- Total files: ~434 source files
- Purpose: Wraps Windows 95 APIs, DirectDraw, DirectSound

**Relationship to WWFLAT32:** WIN32LIB is the Windows-specific layer that sits on top of WWFLAT32's optimized routines.

---

## Subdirectory Analysis

### AUDIO/ (36 files)
**Purpose:** Sound system with DirectSound integration

**Key Files:**
- `SOUNDINT.H/CPP` - Sound interrupt/streaming
- `SOUNDIO.CPP` - Sound I/O operations
- `SOUNDLCK.CPP` - Thread-safe locking
- `SOSCODEC.ASM` - SOS audio codec (16-bit ADPCM)
- `AUDUNCMP.ASM` - Audio decompression

**Windows APIs:**
- DirectSound (LPDIRECTSOUNDBUFFER)
- Critical Sections
- Threading (CreateThread)

**macOS Replacement:** CoreAudio / AVAudioEngine

---

### DRAWBUFF/ (34 files)
**Purpose:** Graphics buffer management, 2D drawing primitives

**Key Files:**
- `GBUFFER.H/CPP` - GraphicBufferClass
- `BUFFER.CPP` - Buffer management
- `ICONCACH.CPP` - Sprite caching
- Assembly blitters: `BITBLIT.ASM`, `CLEAR.ASM`, `FILLRECT.ASM`, `SCALE.ASM`
- `STAMP.ASM` - Tile stamping
- `REMAP.ASM` - Palette remapping
- `SHADOW.ASM` - Shadow rendering

**Windows APIs:**
- DirectDraw (LPDIRECTDRAW, LPDIRECTDRAWSURFACE)
- Lock/Unlock pattern for surface access

**Class Hierarchy:**
```
BufferClass
  ├── GraphicBufferClass
  │     └── VideoBufferClass
  └── GraphicViewPortClass
        └── VideoViewPortClass
```

**macOS Replacement:** Metal textures and render passes

---

### KEYBOARD/ (22 files)
**Purpose:** Keyboard and mouse input

**Key Files:**
- `KEYBOARD.H` - WWKeyboardClass
- Virtual key mappings

**Windows APIs:**
- WM_KEYDOWN, WM_KEYUP messages
- GetKeyState(), GetAsyncKeyState()

**macOS Replacement:** AppKit NSEvent handling

---

### MEM/ (19 files)
**Purpose:** Memory management

**Key Files:**
- `WWMEM.H` - Memory API
- `ALLOC.CPP` - Custom allocator
- `MEM_COPY.ASM` - Optimized memcpy
- `VMPAGEIN.ASM` - Virtual memory

**Windows APIs:**
- GlobalAlloc/GlobalLock
- VirtualAlloc/VirtualFree

**macOS Replacement:** Standard malloc/free, mmap()

---

### MISC/ (24 files)
**Purpose:** Utilities and DirectDraw wrapper

**Key Files:**
- `DDRAW.CPP` - DirectDraw initialization
- `CRC.ASM` - CRC calculation
- `RANDOM.ASM` - RNG
- `FACING8/16.ASM` - Directional calculations
- `FADING.ASM` - Color fading

**macOS Replacement:** Metal initialization, standard math

---

### PALETTE/ (8 files)
**Purpose:** 256-color palette management

**Windows APIs:**
- DirectDraw palette objects

**macOS Replacement:** Metal texture lookup or convert to 32-bit RGBA

---

### SHAPE/ (23 files)
**Purpose:** Sprite/shape rendering (Westwood format)

**Key Files:**
- `SHAPE.H` - Shape definitions
- `DRAWSHP.ASM` - Main shape drawing
- `DS_*.ASM` - Drawing mode variants (scaled, rotated, remapped, shadowed)
- `GETSHAPE.CPP` - Shape extraction
- `PRIOINIT.CPP` - Priority system

**Drawing Flags:**
- Horizontal/vertical flip
- Scaling
- Color fading
- Predator effect (cloaking)
- Ghost/shadow rendering
- Color remapping

**macOS Replacement:** Metal shaders for all effects

---

### TIMER/ (8 files)
**Purpose:** High-resolution timing

**Key Files:**
- `TIMER.H/CPP` - TimerClass, CountDownTimerClass
- `WinTimerClass` - Multimedia timer wrapper

**Windows APIs:**
- timeSetEvent()
- QueryPerformanceCounter()

**macOS Replacement:** mach_absolute_time(), CVDisplayLink

---

### FONT/ (8 files)
**Purpose:** Bitmap font rendering

**Key Files:**
- `FONT.H/CPP` - Font class
- `LOADFONT.CPP` - Font loading
- `TEXTPRNT.ASM` - Text printing

**macOS Replacement:** Load font data, render to Metal

---

### IFF/ (14 files)
**Purpose:** Image format support (IFF, PCX, LBM)

**Key Files:**
- `IFF.H/CPP` - IFF handling
- `LOADPCX.CPP` - PCX loading
- `LCWCOMP.ASM` / `LCWUNCMP.ASM` - LCW compression

**macOS Replacement:** Keep LCW decoder, use ImageIO for modern formats

---

### RAWFILE/ (9 files)
**Purpose:** File I/O abstraction

**Key Files:**
- `RAWFILE.H/CPP` - Raw file access
- `CCFILE.CPP` - C&C file handling

**Windows APIs:**
- CreateFile, ReadFile, WriteFile

**macOS Replacement:** POSIX open/read/write/close

---

### WW_WIN/ (9 files)
**Purpose:** Windows-specific functionality

**Key Files:**
- `WW_WIN.H` - Windows wrappers
- `WINDOWS.CPP` - Window management

**macOS Replacement:** AppKit NSWindow, NSApplication

---

### TILE/ (6 files)
**Purpose:** Tile-based map rendering

**macOS Replacement:** Metal texture arrays, instanced rendering

---

### WSA/ (6 files)
**Purpose:** Westwood Animation format

**Key Files:**
- `WSA.H/CPP` - Animation player
- `XORDELTA.ASM` - XOR delta compression

**macOS Replacement:** Port XOR delta codec to C++

---

### Other Directories
- **MOVIE/** (4 files) - Video playback
- **PLAYCD/** - CD audio
- **MONO/** - Debug monochrome output (legacy)
- **DIPTHONG/** - Speech/phoneme (unused)
- **PROFILE/** - Performance profiling
- **WINCOMM/** - Network communication
- **EXAMPLE/** - Sample code
- **SRCDEBUG/** (169 files) - Debug builds

---

## Assembly File Count

~60-70 ASM files total across subdirectories:
- Graphics blitting: ~20 files
- Shape rendering: ~15 files
- Audio codec: ~5 files
- Memory operations: ~3 files
- Compression: ~3 files
- Utilities: ~10 files

**Port Strategy:** Rewrite in C++ first, optimize with SIMD later if needed.

---

## API Replacement Table

| Windows API | macOS Equivalent |
|-------------|------------------|
| DirectDraw | Metal |
| DirectSound | CoreAudio / AVAudioEngine |
| CreateWindow | AppKit NSWindow |
| GetMessage/DispatchMessage | NSApplication run loop |
| timeSetEvent | dispatch_source_t |
| QueryPerformanceCounter | mach_absolute_time() |
| CreateFile/ReadFile | open()/read() |
| GlobalAlloc | malloc() |
| VirtualAlloc | mmap() |
| Critical Sections | pthread_mutex / os_unfair_lock |

---

## Port Priority

### Phase 1: Foundation
1. TIMER - Timing system
2. MEM - Memory allocation
3. RAWFILE - File I/O

### Phase 2: Input
4. KEYBOARD - Input handling

### Phase 3: Graphics
5. DRAWBUFF/GBUFFER - Buffer system
6. SHAPE - Sprite rendering
7. PALETTE - Color management

### Phase 4: Audio
8. AUDIO - Sound system

### Phase 5: Formats
9. IFF - Image loading
10. WSA - Animations

---

## File Format Decoders to Preserve

- Shape format (sprites)
- LCW compression
- SOS/Westwood audio codec
- WSA animation format
- Palette files (.PAL)
- Font files

---

## Risk Assessment

**High Risk:**
- Assembly blitter performance
- Shape rendering accuracy (complex format)
- Audio codec correctness

**Medium Risk:**
- Timing precision (60 Hz requirement)
- Input latency

**Low Risk:**
- File I/O (straightforward POSIX)
- Window management (standard AppKit)
