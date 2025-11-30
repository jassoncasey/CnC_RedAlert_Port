# Command & Conquer: Red Alert - Source Code Archeology

## Historical Context

**Release Date:** October 31, 1996
**Developer:** Westwood Studios (Las Vegas, Nevada)
**Publisher:** Virgin Interactive
**Source Release:** February 27, 2025 (GPL v3 by EA)
**Original Source:** 2008 Freeware release promoted Red Alert 3

This document analyzes the original Red Alert source code from a software engineering perspective, examining the development practices, patterns, and technical decisions of mid-1990s game development.

---

## Development Platform & Build System

### Target Platforms
- **Primary:** MS-DOS (DOS4GW protected mode)
- **Secondary:** Windows 95 (Win32)

### Compilers & Tools
| Tool | Version | Purpose |
|------|---------|---------|
| Watcom C++ | 10.6 | Primary compiler (DOS/Win32) |
| Turbo Assembler | 4.0 | x86 assembly (TASM) |
| Microsoft Visual C++ | 4.x | Windows builds |
| NMAKE | - | Build automation |

### Build Configuration
The build system uses `.MAK` files with conditional compilation:
- `#ifdef WIN32` - Windows-specific code
- `#ifdef DOS4GW` - DOS protected mode code
- Platform detection via compiler predefined macros

---

## Codebase Metrics

### Size Comparison
| Metric | Original | macOS Port | Ratio |
|--------|----------|------------|-------|
| Source files | ~2,100 | ~45 | 47:1 |
| Lines of code | ~667,000 | ~7,500 | 89:1 |
| Assembly files | 357 | 0 | - |
| Header files | ~450 | ~35 | 13:1 |

### Why Original is 90x Larger

1. **Complete Game vs Scaffold**
   - Original: 40+ unit types, 100+ missions, full campaigns
   - Port: Demo with 12 unit types, procedural map

2. **Redundant Platform Code**
   - `WIN32LIB/` (568 files) - Windows abstraction layer
   - `WWFLAT32/` (400 files) - DOS library with ASM
   - Modern port: Single platform, single implementation

3. **Hand-Optimized Assembly (357 files)**
   - Graphics blitting routines
   - Audio mixing
   - LCW/RLE decompression
   - Memory operations
   - Modern equivalent: ~20 lines in Metal shader

4. **Video Codec Implementation**
   - `VQ/` + `WINVQ/` (399 files) - Vector quantization codec
   - Modern equivalent: FFmpeg single-line decode

5. **Multiple Networking Implementations**
   - IPX (52 files) - Obsolete Novell protocol
   - Modem support
   - Serial cable support
   - Modern equivalent: BSD sockets + GameKit

6. **1996 vs 2024 API Surface**
   - Original: Everything from scratch (memory management, file I/O, input)
   - Modern: OS provides abstracted services

---

## Software Engineering Approach

### Code Organization Philosophy

The codebase follows a **layered architecture**:

```
┌─────────────────────────────────┐
│         Game Logic (CODE/)      │  High-level
├─────────────────────────────────┤
│    Platform Layer (WIN32LIB/)   │  Abstraction
├─────────────────────────────────┤
│  Optimized Core (WWFLAT32/)     │  Performance
├─────────────────────────────────┤
│      Hardware (ASM/Direct)      │  Low-level
└─────────────────────────────────┘
```

### Naming Conventions

**Classes:** PascalCase with Type suffix
- `InfantryClass`, `BuildingClass`, `UnitClass`

**Functions:** PascalCase, often prefixed
- `Map_Get_Cell()`, `Unit_Move_Towards()`

**Constants:** SCREAMING_SNAKE_CASE
- `MAX_UNITS`, `CELL_SIZE`, `TICKS_PER_SECOND`

**Member Variables:** Often Hungarian notation
- `lpBuffer` (long pointer), `dwSize` (DWORD), `bActive` (boolean)

### File Organization

**By subsystem with paired .H/.CPP files:**
```
UNIT.H     → Class declaration, constants
UNIT.CPP   → Implementation
UNITDATA.CPP → Static data tables (separate for cache efficiency)
```

**Data separation pattern:**
Every major class has a companion `*DATA.CPP` file containing:
- Type definitions (stats, costs, speeds)
- Text strings (UI labels, descriptions)
- Art/sound filename references
- Lookup tables

This separation allows:
- Data changes without recompiling logic
- Easy modding via data file edits
- Better instruction cache usage

---

## Architectural Patterns

### 1. Object Hierarchy

```cpp
ObjectClass           // Base: position, health, ID
    │
    └── MissionClass  // AI: mission queue, current order
            │
            └── TechnoClass     // Combat: armor, weapons, owner
                    │
                    ├── FootClass       // Movement: speed, path
                    │       │
                    │       ├── InfantryClass
                    │       ├── UnitClass (vehicles)
                    │       └── AircraftClass
                    │
                    └── BuildingClass   // Static structures
```

Each level adds specific functionality. Movement code is only in `FootClass`, weapon code only in `TechnoClass`.

### 2. Target Union Pattern

```cpp
typedef union {
    void* Ptr;           // Generic pointer
    CELL Cell;           // Map cell coordinate
    ObjectClass* Object; // Specific target
    int32_t Value;       // Numeric value
} TARGET;
```

Compact 32-bit representation allows:
- Targets to be cells OR objects
- Network-safe transmission (cells by coordinate)
- Minimal memory footprint

### 3. Deterministic Lockstep

Multiplayer uses peer-to-peer deterministic simulation:

```
Frame N:
  1. Collect local inputs
  2. Send inputs to all peers
  3. Wait for all peer inputs
  4. Execute frame with combined inputs
  5. Update random seed deterministically
```

**Requirements:**
- Identical floating-point results (fixed-point math used)
- Same random sequence (shared RNG seed)
- Identical data tables
- Frame-locked execution

### 4. Factory Pattern for Object Creation

```cpp
ObjectClass* Factory_Create(RTTIType type, HouseClass* owner) {
    switch (type) {
        case RTTI_INFANTRY: return new InfantryClass(owner);
        case RTTI_UNIT:     return new UnitClass(owner);
        case RTTI_BUILDING: return new BuildingClass(owner);
        // ...
    }
}
```

### 5. Pool Allocators

Fixed-size arrays with active flags:
```cpp
InfantryClass Infantry[MAX_INFANTRY];
bool InfantryActive[MAX_INFANTRY];

int Infantry_Create() {
    for (int i = 0; i < MAX_INFANTRY; i++) {
        if (!InfantryActive[i]) {
            InfantryActive[i] = true;
            return i;
        }
    }
    return -1;
}
```

Benefits:
- No heap fragmentation
- O(n) allocation (acceptable for 1996)
- Predictable memory usage
- Network-sync-safe (no pointer drift)

---

## Abstractions & Patterns

### Platform Abstraction (WIN32LIB)

Complete isolation from hardware:

| Original API | Abstraction |
|-------------|-------------|
| Windows GDI | `GraphicViewPortClass` |
| DirectDraw | `GraphicBufferClass` |
| DirectSound | `SoundClass` |
| Windows Input | `KeyboardClass`, `MouseClass` |
| Windows Timer | `TimerClass` |
| File I/O | `FileClass`, `BufferIOClass` |

This allowed the same game code to target DOS and Windows with only library changes.

### Coordinate Systems

```cpp
// Cell coordinates (map grid)
typedef int16_t CELL;           // 0 to MAP_SIZE-1

// Lepton coordinates (sub-cell precision)
typedef int32_t LEPTON;         // 256 leptons per cell

// World coordinates (pixels for rendering)
typedef int32_t WORLD;          // Varies by zoom

// Screen coordinates (display position)
typedef int16_t SCREEN;         // 0 to SCREEN_WIDTH
```

Conversion macros:
```cpp
#define CELL_TO_LEPTON(c)  ((c) << 8)
#define LEPTON_TO_CELL(l)  ((l) >> 8)
#define LEPTON_TO_PIXEL(l) ((l) >> 4)
```

### Animation System

Frame-based animation with runtime blending:
```cpp
struct ShapeFileStruct {
    int16_t FrameCount;
    int16_t Width;
    int16_t Height;
    uint8_t* FrameData[];  // Compressed frames
};
```

Drawing uses color remapping for team colors:
```cpp
void Draw_Shape(
    void* shape,
    int frame,
    int x, int y,
    uint8_t* remap_table  // 256-byte team color remap
);
```

---

## Performance Techniques

### 1. Fixed-Point Mathematics

```cpp
// 16.16 fixed point
typedef int32_t FIXED;
#define FIXED_SHIFT 16
#define FIXED_ONE   (1 << FIXED_SHIFT)

FIXED Fixed_Mul(FIXED a, FIXED b) {
    return ((int64_t)a * b) >> FIXED_SHIFT;
}
```

Avoids FPU for determinism and speed.

### 2. Lookup Tables

```cpp
// Precomputed sine table (256 entries for 360 degrees)
static int16_t SinTable[256];

// Distance approximation (avoid sqrt)
static uint8_t DistanceTable[256][256];

// Sprite shadow offsets by facing
static int8_t ShadowOffset[8][2];
```

### 3. Dirty Rectangles

Only redraw changed screen regions:
```cpp
struct DirtyRect {
    int16_t x, y, w, h;
};
DirtyRect DirtyRects[MAX_DIRTY];
int DirtyCount;
```

### 4. Assembly Hot Paths

Critical inner loops in hand-tuned x86:
- `LCWCOMP.ASM` - LCW decompression
- `XORDELTA.ASM` - Frame differencing
- `KEYFBUFF.ASM` - Keyboard buffer handling
- `AUDMIX.ASM` - Audio mixing

---

## Asset Formats

### MIX Archives
Container format with CRC-based lookup:
```cpp
struct MixHeader {
    uint16_t num_files;
    uint32_t data_size;
};
struct MixEntry {
    uint32_t crc;      // CRC32 of filename (lowercase)
    uint32_t offset;
    uint32_t size;
};
```

Files identified by CRC, not name string (saves memory).

### SHP Sprites
Multi-frame sprite format:
- LCW compression (Westwood proprietary)
- Format80: RLE + raw hybrid
- Format40: XOR delta from previous frame
- 256-color indexed

### PAL Palettes
256-color palettes (768 bytes):
- 6-bit per channel (0-63) in original
- Scale to 8-bit: `color8 = (color6 << 2) | (color6 >> 4)`

### AUD Audio
Westwood ADPCM variant:
- IMA ADPCM-like compression
- Variable sample rates
- Flags for stereo/16-bit

---

## Obtaining Game Assets

### 2008 Freeware Release

EA released Red Alert as freeware on August 31, 2008 to promote Red Alert 3.

**Download Sources:**
- [Internet Archive](https://archive.org/details/command-and-conquer-red-alert) - ISO images
- [CnC Communications Center](https://cnc-comm.com/red-alert/downloads/the-game) - Windows installers
- [ModDB](https://www.moddb.com/games/cc-red-alert/downloads) - Various formats

**ISO Contents:**
- `CD1_ALLIED_DISC.ISO` (624 MB) - Allied campaign + shared data
- `CD2_SOVIET_DISC.ISO` (646 MB) - Soviet campaign

**Required MIX Files for Port:**
| File | Contents |
|------|----------|
| `REDALERT.MIX` | Core game data |
| `LOCAL.MIX` | Localized strings |
| `CONQUER.MIX` | Main game assets |
| `GENERAL.MIX` | General assets |
| `INTERIOR.MIX` | Building interiors |
| `MOVIES.MIX` | Video cutscenes |
| `SCORES.MIX` | Music |
| `SOUNDS.MIX` | Sound effects |

### Modern Alternatives

- **[OpenRA](https://www.openra.net/)** - Open source reimplementation
- **[CnCNet](https://cncnet.org/)** - Multiplayer revival with modern patches

---

## Lessons for Modern Port

### What to Preserve
1. **Game feel** - Fixed timestep timing, unit responsiveness
2. **Deterministic simulation** - For multiplayer compatibility
3. **Data-driven design** - Type definitions in tables
4. **Object hierarchy** - Clean separation of concerns

### What to Replace
1. **Assembly** - Modern compilers optimize well
2. **Platform code** - Use native APIs (Metal, CoreAudio)
3. **Video codec** - Use FFmpeg or platform decoder
4. **Networking** - BSD sockets, GameKit, Steam

### What to Simplify
1. **Memory management** - Modern allocators are good
2. **File I/O** - POSIX APIs work fine
3. **Input handling** - AppKit events are sufficient
4. **Threading** - GCD instead of manual threads

---

## Code Quality Observations

### Strengths
- Clear class hierarchy
- Consistent naming conventions
- Good separation of platform code
- Comprehensive comments (especially headers)
- Data-driven design enables modding

### Weaknesses (by modern standards)
- Global state everywhere
- Tight coupling between systems
- Magic numbers in some areas
- Copy-paste code in platform variants

### Historical Context
For 1996, this code is **excellent**:
- Ships on time
- Runs on target hardware
- Supports modding
- Maintainable by original team
- Cross-platform (DOS + Windows)

---

## References

- Original source: `original/` directory
- Individual directory analysis: `original/*/research.md`
- EA source release: GPL v3, February 2025
- OpenRA reverse engineering documentation
