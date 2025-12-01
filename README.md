# Command & Conquer: Red Alert - macOS Port

## What Is This?

**A stress test for AI code assistants.** I wanted to understand how tools like Claude Code perform on medium-to-large tasks with minimal hand-holding. Instead of starting from scratch (where I'd obsess over design), I chose an existing codebase where the architecture is already established. The constraint is the point.

### Why This Project?

- **Existing design** - The original Red Alert source provides the blueprint. I'm not making architectural decisions; I'm seeing if the AI can follow an established pattern.
- **Real complexity** - 520 source files, ~200K lines of 1996 C++. Not a toy problem.
- **Clear success criteria** - Either the game runs or it doesn't.
- **Limited guidance** - I give high-level direction ("implement the sidebar"), not step-by-step instructions.

### The Challenge

| Source | Target |
|--------|--------|
| Windows 95/NT | macOS 14+ (Sonoma) |
| x86 assembly | ARM64 (Apple Silicon) |
| DirectDraw/GDI | Metal |
| DirectSound | CoreAudio |
| Win32 API | AppKit/Cocoa |
| Watcom C++ (1995) | Modern C++17 |

---

## Current Status

**Phase 1 Complete** - Full menu flow with intro video, campaign selection, mission briefings, and playable demo mission.

### What Works

| Feature | Status | Notes |
|---------|--------|-------|
| Metal Renderer | ✓ | 640x400 with palette-based rendering, fog of war |
| CoreAudio | ✓ | 44.1kHz stereo, 16-channel mixing |
| Input | ✓ | Mouse selection, keyboard commands (P=pause, F=fullscreen) |
| Terrain | ✓ | Loads tiles from snow.mix |
| Sprites | ✓ | Loads SHP sprites from conquer.mix |
| A* Pathfinding | ✓ | Units navigate around obstacles |
| Combat | ✓ | Units attack, take damage, die |
| Menu System | ✓ | Main menu, options, volume controls, credits |
| Sidebar UI | ✓ | Radar minimap, build strips, selection panel |
| VQA Cutscenes | ✓ | Full intro video, pre-mission briefing videos |
| Background Music | ✓ | IMA ADPCM streaming from SCORES.MIX |
| Campaign Flow | ✓ | Allied/Soviet selection, difficulty, briefings |

### What's Next (Phase 2)

| Feature | Status | What's Needed |
|---------|--------|---------------|
| **Building Placement** | Not started | Grid snapping, build radius, prerequisites |
| **Production System** | Built, not wired | Queue units/buildings, spend credits |
| **Fog of War** | Alpha-based | Full shroud tracking per cell |
| **AI Opponent** | Built, not wired | Base building, attacks, difficulty levels |
| **Real Missions** | INI parsing | Load actual SCU/SCG scenarios |

~500 unit tests pass for systems that aren't connected to the game loop yet.

---

## Building

```bash
cd macos
make
./RedAlert.app/Contents/MacOS/RedAlert
```

Or to create a distributable:

```bash
make dist   # Creates signed zip
make dmg    # Creates DMG image
```

---

## Game Assets

**No game assets included** - this is just the engine.

### Quick Setup

1. Download the freeware release from [Internet Archive](https://archive.org/details/command-and-conquer-red-alert)
2. Extract MIX files to `./assets/` or `~/Library/Application Support/RedAlert/assets/`
3. Run the game

### Required Files

| File | Source | Notes |
|------|--------|-------|
| conquer.mix | CD1/INSTALL/ | Unit sprites, sounds |
| snow.mix | CD1/INSTALL/ | Snow terrain tiles |
| local.mix | CD1/INSTALL/ | Palettes, UI elements |

### Legal Status

- **Source Code:** GPL v3 - Released by EA, February 2025
- **Game Assets:** Freeware - Released by EA, August 2008

---

## Project Structure

```
CnC_Red_Alert/
├── README.md              # This file
├── PORTING_PLAN.md        # Milestone roadmap (M33-M50)
├── COMPLETED.md           # Archived completed work (M0-M32)
├── archeology.md          # Original source analysis
│
├── original/              # Original Windows source (read-only reference)
│   ├── CODE/             # Main game (~520 files)
│   └── */research.md     # Per-directory analysis
│
├── macos/                 # macOS port
│   ├── Makefile
│   └── src/
│       ├── main.mm       # Entry point
│       ├── platform/     # macOS abstractions
│       ├── graphics/     # Metal renderer
│       ├── audio/        # CoreAudio
│       ├── input/        # Event handling
│       ├── assets/       # MIX/SHP/PAL/AUD loaders
│       ├── crypto/       # Blowfish for encrypted MIX
│       ├── game/         # Game logic (~400 tests)
│       ├── video/        # VQA playback
│       └── ui/           # Menu and game UI
│
└── assets/               # Game data (gitignored)
```

---

## Technical Notes

### Capability Mapping

The approach is understanding what the original code *does*, not translating it line-by-line:

| Capability | Windows | macOS |
|------------|---------|-------|
| Frame rendering | DirectDraw surfaces | Metal textures |
| Audio mixing | DirectSound buffers | AudioUnit graph |
| Input | Win32 messages | NSEvent |
| Timers | QueryPerformanceCounter | mach_absolute_time |
| Archives | Custom MIX reader | Same format, new reader |

### What Makes This Hard

1. **No cross-platform layer** - Direct Windows→macOS, not Windows→SDL→macOS
2. **Native APIs only** - Metal, CoreAudio, AppKit (no portability crutches)
3. **Format compatibility** - Must read original MIX/SHP/PAL/AUD files
4. **Encrypted assets** - Some MIX files use Blowfish encryption

---

## Related Work

[OpenRA](https://github.com/OpenRA/OpenRA) is a mature, production-quality reimplementation of C&C games. If you want to *play* Red Alert, use OpenRA.

This project is different:
- **Purpose:** Testing AI capabilities, not building a product
- **Language:** C++ (matching original) vs C#
- **Platform:** macOS-only vs cross-platform
- **Development:** AI-assisted with minimal guidance

---

## The Experiment

I'm testing:

1. **Can an AI handle a large existing codebase?** - Not greenfield, but following established patterns
2. **Can it work with minimal direction?** - "Implement the radar" not "write function X that does Y"
3. **Can it maintain coherence across sessions?** - 50+ files, interconnected systems
4. **Where does it struggle?** - What kinds of tasks need more human involvement?

The original source is the design document. I'm interested in how well the tool can execute against an existing architecture without me micromanaging every decision.

---

## License

GPL v3 - Original source released by Electronic Arts, February 2025.
