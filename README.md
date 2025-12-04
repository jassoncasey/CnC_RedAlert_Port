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

### Roadmap: Demo → Full Game

| Phase | Description | Status |
|-------|-------------|--------|
| **A** | Stabilization (fix audio/UI bugs) | Not Started |
| **B** | Real Missions (load actual campaign maps) | Not Started |
| **C** | Campaign Progression (all 44 missions) | Not Started |
| **D** | Skirmish Mode (random maps vs AI) | Not Started |
| **E** | Polish (voices, animations, aircraft) | Not Started |

See [PORTING_PLAN.md](PORTING_PLAN.md) for detailed task breakdown and
[ISSUES.md](ISSUES.md) for known issues.

---

## Building

```bash
# Clone with submodules
git clone --recurse-submodules <repo-url>

# Or if already cloned:
git submodule update --init

# Build
cd macos
make
./RedAlert.app/Contents/MacOS/RedAlert
```

Or to create a distributable:

```bash
make dist   # Creates signed zip
make dmg    # Creates DMG image
```

### Asset Viewer

A visual inspection tool for exploring Westwood game assets. Useful for
understanding file formats, verifying asset coverage, and debugging.

**Features:**
- Browse MIX archives recursively (MIX within MIX)
- Preview sprites with animation playback
- Play audio files (IMA ADPCM, uncompressed)
- Play VQA video files with audio
- Inspect palettes
- View terrain templates
- Filename database for CRC-to-name mapping

**Building and Running:**

```bash
cd tools/asset-viewer
make
./build/asset-viewer
```

**Usage:**
1. Launch the viewer
2. Click "Select Directory" to choose your assets folder
3. Browse the tree view to explore MIX files and their contents
4. Select any asset to preview it (sprites animate, audio plays)
5. Use Tab 2 for systematic asset review by type

**Supported Formats:**

| Format | Preview |
|--------|---------|
| MIX | Archive browser with recursive scanning |
| SHP | Sprite preview with animation, frame-by-frame |
| PAL | Color palette grid display |
| AUD | Audio playback |
| VQA | Video playback with audio |
| TMP | Terrain tile preview |
| INI | Text display |

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

- **Source Code:** GPL v3 - Released by EA, May 2020
- **Game Assets:** Freeware - Released by EA, August 2008

---

## Project Structure

```
CnC_Red_Alert/
├── README.md              # This file
├── CLAUDE.md              # Claude Code configuration
├── PORTING_PLAN.md        # Remaining work
├── ISSUES.md              # Known issues
├── ASSETS.md              # Asset file documentation
├── archeology.md          # Original source analysis
│
├── parts/                 # Claude Code standards
│   ├── workflow.md        # Interrogative vs imperative modes
│   ├── files.md           # Directory scope, tmp/ isolation
│   ├── agent.md           # Response style, tool usage
│   ├── code.md            # Language selection, function size
│   ├── style.md           # 80-col lines, ASCII, tree format
│   ├── architecture.md    # Minimalism, design questions
│   ├── error-handling.md  # Fail fast, return types
│   ├── security.md        # Secure defaults, memory safety
│   ├── config.md          # Hierarchy, secrets, formats
│   ├── concurrency.md     # Async first, lock-free
│   ├── testing.md         # Coverage, table-driven tests
│   ├── tooling.md         # Make, semver, dependencies
│   └── unix.md            # Streams, exit codes, signals
│
├── .claude/
│   └── commands/          # Slash commands
│       ├── checkpoint.md  # /checkpoint - save session state
│       └── resume.md      # /resume - restore from checkpoint
│
├── libs/                  # Shared libraries
│   └── ra-media/          # Media library (renderer, audio, VQA)
│       ├── Makefile
│       ├── include/ra/    # Public headers
│       └── src/           # Implementation
│
├── tools/                 # Development utilities
│   └── asset-viewer/      # Visual asset inspection tool
│       ├── Makefile
│       └── asset_viewer.mm
│
├── submodules/            # External reference repos
│   ├── CnC_Remastered_Collection/  # EA's original source
│   ├── OpenRA/                     # Reference implementation
│   └── westwood-formats/           # File formats & libwestwood
│
├── macos/                 # macOS game port
│   ├── Makefile
│   ├── data/              # Lookup tables
│   ├── include/           # Header files
│   ├── resources/         # App bundle resources
│   └── src/
│       ├── main.mm        # Entry point
│       ├── assets/        # MIX/SHP/PAL/AUD loaders
│       ├── audio/         # Audio compatibility layer
│       ├── crypto/        # Blowfish for encrypted MIX
│       ├── game/          # Game logic
│       ├── graphics/      # Renderer compatibility layer
│       ├── input/         # Event handling
│       ├── platform/      # macOS abstractions
│       ├── tests/         # Unit tests
│       ├── ui/            # Menu and game UI
│       └── video/         # VQA compatibility layer
│
└── assets/                # Game data (gitignored)
```

### Shared Libraries

| Library | Description |
|---------|-------------|
| **ra-media** | Platform-independent media layer: Metal renderer (640x400 palettized), CoreAudio mixer (16 channels), VQA video decoder. May be promoted to its own repo. |
| **libwestwood** | Westwood file format decoders (MIX, SHP, PAL, AUD, VQA, TMP). From submodules/westwood-formats. |

### Development Tools

| Tool | Description |
|------|-------------|
| **asset-viewer** | Visual inspection tool for game assets. Browse MIX archives, preview sprites, play audio, verify asset coverage. |

### Submodules

| Path | Repository | Purpose |
|------|------------|---------|
| `submodules/CnC_Remastered_Collection/` | [electronicarts/CnC_Remastered_Collection](https://github.com/electronicarts/CnC_Remastered_Collection) | Original Windows source |
| `submodules/OpenRA/` | [OpenRA/OpenRA](https://github.com/OpenRA/OpenRA) | Reference implementation |
| `submodules/westwood-formats/` | [jassoncasey/westwood-formats](https://github.com/jassoncasey/westwood-formats) | File formats, libwestwood library |

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

## Claude Code Configuration

This project includes configuration to shape Claude Code's behavior during
development. The goal is deliberate, controlled AI assistance rather than
autonomous code generation.

### Behavioral Standards (`CLAUDE.md` + `parts/`)

The `CLAUDE.md` file references 12 standards files in `parts/` that define:

| Standard | Purpose |
|----------|---------|
| **workflow.md** | Two-mode operation: *interrogative* (discuss, explore) vs *imperative* (generate artifacts). Default is discussion mode. |
| **files.md** | Claude-initiated files go to `tmp/`, not project root. User controls project structure. |
| **agent.md** | Concise responses, proper tool selection, ask when uncertain. |
| **code.md** | Prefer pure functions under 10 lines, clear naming. |
| **style.md** | 80-column lines, ASCII, tree-format for hierarchies. |
| **architecture.md** | Simplest solution first. Delete code over adding. |
| **error-handling.md** | Fail fast, explicit return types over exceptions. |
| **security.md** | Secure defaults, memory-safe languages preferred. |
| **config.md** | Standard hierarchy, secrets in files only. |
| **concurrency.md** | Async first, lock-free over locks. |
| **testing.md** | Table-driven tests, behavior over implementation. |
| **tooling.md** | Make, semver, minimal dependencies. |
| **unix.md** | Proper streams, exit codes, signal handling. |

**Key behavior:** Claude defaults to discussion mode. It will explain, analyze,
and propose—but won't generate code or modify files until explicitly told
"make it so" or given an imperative command like "add", "create", "implement".

### Slash Commands (`.claude/commands/`)

| Command | Purpose |
|---------|---------|
| `/checkpoint` | Save session state to `.claude/checkpoints/` with goal, narrative, current state, and next steps. Enables session continuity. |
| `/resume` | List available checkpoints and restore context from a previous session. |

These commands address Claude Code's session isolation—each conversation starts
fresh. Checkpoints preserve context across sessions without relying on
Claude's memory of previous work.

---

## License

GPL v3 - Original source released by Electronic Arts, May 2020.
