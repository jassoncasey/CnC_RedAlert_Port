# Command & Conquer: Red Alert - Original Source Code Research

## Overview

This directory contains the original Windows/DOS source code for Command & Conquer: Red Alert (1996) by Westwood Studios. The code was released under GPL v3 by EA in 2020.

**Total Codebase:**
- ~2,100 files
- ~667,000 lines of code (C++, C, ASM)
- 74 directories

---

## Directory Structure

```
original/
├── CODE/           # Main game engine (589 files) - CRITICAL
├── WIN32LIB/       # Windows 32-bit library (568 files)
├── WWFLAT32/       # Optimized 32-bit library with ASM (400 files)
├── VQ/             # VQ video codec - DOS (183 files)
├── WINVQ/          # VQ video codec - Windows (216 files)
├── IPX/            # IPX networking (52 files) - OBSOLETE
├── TOOLS/          # Build/content tools (25 files)
├── LAUNCHER/       # Game launcher (47 files)
└── LAUNCH/         # Launcher stub (2 files)
```

Each directory contains a `research.md` file with detailed analysis.

---

## Quick Reference by Priority

### CRITICAL - Must Port
| Directory | Purpose | Effort |
|-----------|---------|--------|
| CODE/ | Game logic, AI, units, maps | High |
| WIN32LIB/SHAPE | Sprite format decoder | Medium |
| WIN32LIB/AUDIO | Audio codec (ADPCM) | Medium |
| WWFLAT32/IFF | LCW compression | Low |

### USEFUL - Reference Material
| Directory | Purpose |
|-----------|---------|
| WIN32LIB/ | Platform abstraction patterns |
| WWFLAT32/ | Algorithm implementations |
| VQ/ | Video format documentation |

### NOT NEEDED
| Directory | Reason |
|-----------|--------|
| IPX/ | Protocol obsolete |
| LAUNCHER/ | macOS uses app bundles |
| TOOLS/ | Development only |

---

## Why Original is 90x Larger Than Port

| Aspect | Original | macOS Port |
|--------|----------|------------|
| Lines of code | ~667,000 | ~7,500 |
| Game content | Complete game | Demo scaffold |
| Platform code | WIN32LIB + WWFLAT32 | Single Metal impl |
| Assembly | 357 ASM files | None (modern APIs) |
| Video codec | Full VQ impl | Stubbed |
| Networking | IPX + TCP + modem | Stubbed |
| 1996 vs 2024 | Manual everything | Modern frameworks |

The macOS port is infrastructure only. The original has:
- 40+ unit types fully implemented
- 100+ campaign missions
- Full AI system
- Complete multiplayer
- Video cutscene playback
- All UI dialogs

---

## Key Technical Insights

### Object Hierarchy
```
ObjectClass
  └── TechnoClass (combat entities)
      ├── FootClass (mobile)
      │   ├── InfantryClass
      │   ├── UnitClass (vehicles)
      │   └── AircraftClass
      └── BuildingClass
```

### Deterministic Lockstep
Multiplayer uses deterministic lockstep synchronization:
- All players simulate identical game state
- Only player inputs are transmitted
- Random number generator is seeded identically
- Any desync = game breaks

### Asset Formats
| Format | Used For |
|--------|----------|
| MIX | Archive container |
| SHP | Sprites (LCW compressed) |
| PAL | 256-color palettes |
| AUD | Audio (Westwood ADPCM) |
| VQA | Video cutscenes |
| INI | Configuration/scenarios |

### Build System
- **Compiler:** Watcom C++ 10.6
- **Assembler:** Turbo Assembler 4.0
- **Target:** DOS4GW / Windows 95

---

## Port Strategy Summary

### Phase 1: Platform Layer (Done in macOS port)
- [x] Window management (AppKit)
- [x] Rendering (Metal)
- [x] Audio (CoreAudio)
- [x] Input (AppKit events)
- [x] File I/O (POSIX)
- [x] Timing (mach_absolute_time)

### Phase 2: Asset Loading (Partially done)
- [x] MIX archives
- [x] SHP sprites
- [x] PAL palettes
- [x] AUD audio
- [ ] Full integration with game

### Phase 3: Game Logic (Not started)
- [ ] Port object classes from CODE/
- [ ] Port AI system
- [ ] Port scenario loading
- [ ] Port combat system

### Phase 4: Full Game (Not started)
- [ ] Campaign missions
- [ ] Video playback
- [ ] Multiplayer networking

---

## File Format Quick Reference

### MIX Archive
```c
struct MIXHeader {
    uint16_t numFiles;
    uint32_t dataSize;
};
struct MIXEntry {
    uint32_t crc;    // CRC of filename
    uint32_t offset;
    uint32_t size;
};
```

### SHP Sprite
- Multi-frame sprites
- LCW compressed
- 256-color indexed
- Format80/Format40 encoding

### AUD Audio
```c
struct AUDHeader {
    uint16_t sampleRate;
    uint32_t compressedSize;
    uint32_t uncompressedSize;
    uint8_t flags;        // stereo, 16-bit
    uint8_t compression;  // codec type
};
```

### VQA Video
- IFF container (FORM WVQA)
- Vector Quantization codec
- 4x2 pixel blocks typical
- LCW compressed codebooks

---

## Historical Context

Red Alert represents peak 1996 game development:
- Last major DOS games before Windows-only
- Transition to DirectX
- Hand-optimized x86 assembly
- 256-color palette graphics
- Software rendering only

The code quality is high for its era:
- Clear class hierarchy
- Consistent naming conventions
- Separated platform code
- Comprehensive comments

---

## Research Documents

Detailed analysis in each directory:
- [CODE/research.md](CODE/research.md) - Game engine
- [WIN32LIB/research.md](WIN32LIB/research.md) - Windows library
- [WWFLAT32/research.md](WWFLAT32/research.md) - Optimized library
- [VQ/research.md](VQ/research.md) - Video codec
- [WINVQ/research.md](WINVQ/research.md) - Windows video
- [IPX/research.md](IPX/research.md) - Networking
- [TOOLS/research.md](TOOLS/research.md) - Build tools
- [LAUNCHER/research.md](LAUNCHER/research.md) - Game launcher

---

## Next Steps for Port

1. **Study CODE/research.md** - Understand game architecture
2. **Extract data tables** - Port *DATA.CPP files
3. **Port base classes** - ObjectClass → TechnoClass hierarchy
4. **Integrate assets** - Connect loaders to game objects
5. **Port systems** - Map, AI, combat one at a time
6. **Test extensively** - Compare behavior to original
