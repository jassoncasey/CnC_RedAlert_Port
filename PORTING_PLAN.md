# Command & Conquer Red Alert - macOS Port Plan

## Target Platform
- **OS:** macOS 14+ (Sonoma)
- **Architecture:** ARM64 (Apple Silicon only)
- **Graphics:** Metal + AppKit
- **Audio:** CoreAudio
- **Build:** Command-line tools (clang, make)

## Repository Structure

```
CnC_Red_Alert/
├── README.md                 # Project overview
├── PORTING_PLAN.md          # This document
├── LICENSE.md               # Original license
│
├── original/                # Original Windows codebase (read-only reference)
│   ├── CODE/                # Main game source
│   ├── WIN32LIB/            # Windows library
│   ├── WWFLAT32/            # Westwood flat library
│   ├── LAUNCHER/            # Game launcher
│   ├── IPX/                 # IPX networking
│   ├── WINVQ/               # Video player
│   ├── VQ/                  # VQ codec
│   └── TOOLS/               # Build tools
│
└── macos/                   # macOS port (our work)
    ├── Makefile
    ├── src/
    │   ├── main.mm          # Entry point
    │   ├── platform/        # Platform abstractions
    │   ├── graphics/metal/  # Metal renderer
    │   ├── audio/coreaudio/ # CoreAudio backend
    │   ├── input/           # Keyboard/mouse
    │   ├── stubs/           # Stubbed features (network, MIDI)
    │   └── game/            # Copied/adapted game logic
    ├── include/compat/      # Windows API shims
    ├── resources/           # App bundle resources
    └── assets/stub/         # Placeholder test assets
```

## Deferred Scope
- **Networking** (TCP/IP, IPX, modem) - stubbed, revisit later
- **MIDI music** - stubbed, revisit later
- **Real game assets** - using stubs initially

## Asset Strategy

**Original game assets are NOT in this repository.**

For development, we create stub assets:
- Colored rectangles for sprites
- Simple tones for audio
- Placeholder palettes

For production, assets from EA's freeware release (2008) are required:
- REDALERT.MIX, MAIN.MIX, etc.
- Document extraction process when needed

---

## Milestone 0: Repository Setup ✓
**Status: Complete**

- [x] Create directory structure
- [x] Move original code to `original/`
- [x] Create `macos/` skeleton
- [x] Document structure

---

## Milestone 1: Minimal Compile
**Goal:** Single source file compiles and links on macOS

**Tasks:**
1. Create `macos/Makefile` with clang configuration
2. Create `macos/src/main.mm` - minimal AppKit app (window + event loop)
3. Create `macos/include/compat/platform.h` - base type definitions
4. Verify compile and run: window appears, Cmd+Q quits

**Verification:**
```bash
cd macos && make && ./RedAlert.app/Contents/MacOS/RedAlert
```
Window appears, quits cleanly.

**No original code yet** - just proving the build works.

---

## Milestone 2: Windows Compatibility Layer
**Goal:** Original headers can be included without errors

**Tasks:**
1. Create stub headers in `macos/include/compat/`:
   - `windows.h` - types (DWORD, HANDLE, BOOL, HWND, etc.)
   - `ddraw.h` - DirectDraw stubs
   - `dsound.h` - DirectSound stubs
   - `mmsystem.h` - Multimedia timer stubs
   - `winsock.h` - Winsock type stubs
2. Handle Watcom-specific pragmas (define away or convert)
3. Handle calling conventions (`__cdecl`, `__stdcall` → empty)

**Verification:** Can `#include` original headers from `original/CODE/` without compiler errors.

---

## Milestone 3: File I/O Abstraction
**Goal:** Game can read files using our POSIX wrapper

**Tasks:**
1. Copy `original/CODE/RAWFILE.H` and `RAWFILE.CPP` to `macos/src/game/`
2. Create `macos/src/platform/file.cpp`:
   - `CreateFile` → `open()`
   - `ReadFile` → `read()`
   - `WriteFile` → `write()`
   - `CloseHandle` → `close()`
   - Path separator handling
3. Adapt RAWFILE to use POSIX or wrap Windows calls

**Verification:** Can open, read, write, close a test file.

---

## Milestone 4: Timing System
**Goal:** Game timer works

**Tasks:**
1. Create `macos/src/platform/timing.cpp`:
   - `timeGetTime()` equivalent using `mach_absolute_time()`
   - `QueryPerformanceCounter` equivalent
   - `Sleep()` using `nanosleep()`
2. Match original timing resolution expectations

**Verification:** Timer callback fires at expected intervals.

---

## Milestone 5: Stub Assets & Resource Loading
**Goal:** Game attempts to load assets, gracefully handles stubs

**Tasks:**
1. Create minimal stub assets in `macos/assets/stub/`:
   - Stub palette (grayscale or basic colors)
   - Stub sprite (colored rectangle)
   - Stub sound (sine wave beep)
2. Copy minimal asset loading code from original
3. Implement asset path resolution
4. Return stub when real asset not found

**Verification:** Asset load calls succeed with stubs.

---

## Milestone 6: Graphics Foundation
**Goal:** Framebuffer renders to screen via Metal

**Tasks:**
1. Copy `GraphicBufferClass` / `GraphicViewPortClass` foundations
2. Create `macos/src/graphics/metal/renderer.mm`:
   - MTLDevice, MTLCommandQueue setup
   - Framebuffer texture (upload CPU buffer)
   - Fullscreen quad shader
3. Implement software framebuffer:
   - Allocate 640x480 (or 320x200) 8-bit buffer
   - Palette-to-RGBA conversion on flip
   - Upload to Metal texture
4. Render colored rectangle to verify pipeline

**Verification:** Colored rectangle appears in window.

---

## Milestone 7: Input System
**Goal:** Keyboard and mouse input works

**Tasks:**
1. Copy `WWKeyboardClass` interface from original
2. Create `macos/src/input/keyboard.mm`:
   - NSEvent handling (keyDown, keyUp, flagsChanged)
   - macOS keycode → VK_* mapping table
   - Mouse tracking (position, buttons)
3. Maintain circular buffer architecture
4. Integrate with AppKit event loop

**Verification:** Log key presses and mouse position to console.

---

## Milestone 8: Core Game Loop Integration
**Goal:** Game main loop runs with our platform layer

**Tasks:**
1. Identify minimal game loop from `original/CODE/`
2. Copy required files to `macos/src/game/` incrementally
3. Stub out subsystems not yet implemented
4. Wire up: timing → input → update → render cycle

**Verification:** Loop runs, responds to input, renders frame.

---

## Milestone 9: Basic Rendering
**Goal:** Game renders something recognizable

**Tasks:**
1. Copy sprite/shape rendering code
2. Copy palette management
3. Implement drawing primitives:
   - Put pixel, lines, rectangles
   - Blit operations
4. Render with stub assets (colored shapes)

**Verification:** Shapes render with correct colors and positions.

---

## Milestone 10: Audio Foundation
**Goal:** Sound effects play

**Tasks:**
1. Create `macos/src/audio/coreaudio/audio.mm`:
   - AudioQueue or AVAudioEngine setup
   - Simple software mixer (5 channels)
   - Sample playback (16-bit PCM)
2. Stub ADPCM decoder (or implement if simple)
3. Stub MIDI (no-op)

**Verification:** Stub beep sound plays when triggered.

---

## Milestone 11: Menu System
**Goal:** Game menus navigate correctly

**Tasks:**
1. Copy menu rendering code
2. Copy menu input handling
3. Wire up with our input system
4. Test with stub graphics

**Verification:** Can navigate menus with keyboard/mouse.

---

## Milestone 12: Real Asset Integration
**Goal:** Load actual game assets

**Tasks:**
1. Document asset extraction from freeware release
2. Implement MIX file reader
3. Implement SHP sprite loader
4. Implement AUD audio loader
5. Implement PAL palette loader

**Verification:** Title screen renders with real graphics.

---

## Milestone 13: Gameplay
**Goal:** Play a mission

**Tasks:**
1. Copy remaining game logic
2. Unit rendering and animation
3. Map/terrain rendering
4. Selection and commands
5. Basic AI

**Verification:** Start and play tutorial mission.

---

## Milestone 14: Polish
**Goal:** Releasable application

**Tasks:**
1. App bundle (Info.plist, icon)
2. Fullscreen toggle
3. Save/load game
4. Settings persistence
5. Cursor handling
6. Window resize

**Verification:** Double-click app, plays correctly, quits cleanly.

---

## Future Work

### Networking
- BSD sockets for TCP/IP
- Remove IPX (obsolete)
- LAN multiplayer

### MIDI
- CoreMIDI playback
- Soundfont for accurate music

### Intel Support
- Add x86_64 to build
- Universal binary

---

## Estimated Effort

| Milestone | Hours |
|-----------|-------|
| 1-2: Build + Compat | 40-60 |
| 3-4: File + Timing | 20-30 |
| 5: Stub Assets | 20-30 |
| 6: Graphics | 80-120 |
| 7: Input | 30-50 |
| 8: Game Loop | 40-60 |
| 9: Rendering | 60-80 |
| 10: Audio | 60-80 |
| 11: Menus | 30-40 |
| 12: Real Assets | 40-60 |
| 13: Gameplay | 100-150 |
| 14: Polish | 40-60 |
| **Total** | **560-820** |
