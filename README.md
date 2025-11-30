# Command & Conquer: Red Alert - macOS Port

Native macOS port of the classic Command & Conquer: Red Alert (1996).

## Current Status

**Phases 1-5 Complete** - Infrastructure, game systems, and media working.

### Completed (Milestones 0-29)

| Component | Status |
|-----------|--------|
| Platform Layer (AppKit/Metal) | ✓ |
| Rendering (Metal) | ✓ |
| Audio (CoreAudio) | ✓ |
| Input (Keyboard/Mouse) | ✓ |
| Asset Loaders (MIX/SHP/PAL/AUD) | ✓ |
| Menu System | ✓ |
| Demo Gameplay | ✓ |
| App Bundle | ✓ |
| Data Tables (Infantry/Unit/Weapon/Building/Aircraft) | ✓ |
| Object Class Hierarchy (45 tests) | ✓ |
| INI/Rules Parser (53 tests) | ✓ |
| Map/Cell System (38 tests) | ✓ |
| A* Pathfinding | ✓ |
| Entity Classes (35 tests) | ✓ |
| Combat System (28 tests) | ✓ |
| AI & Teams (28 tests) | ✓ |
| Scenarios & Triggers (26 tests) | ✓ |
| Sidebar & Production (22 tests) | ✓ |
| Radar/Minimap (21 tests) | ✓ |
| Save/Load System (18 tests) | ✓ |
| Animation System (21 tests) | ✓ |
| Campaign System (35 tests) | ✓ |
| VQA Video Playback (12 tests) | ✓ |
| Music Streaming (24 tests) | ✓ |

### Progress Summary

See [PORTING_PLAN.md](PORTING_PLAN.md) for detailed roadmap.

| Phase | Milestones | Status |
|-------|------------|--------|
| Phase 1: Infrastructure | M0-M14 | ✓ Complete |
| Phase 2: Core Engine | M15-M22 | ✓ Complete |
| Phase 3: Game Systems | M23 | ✓ Complete |
| Phase 4: UI & Polish | M24-M27 | ✓ Complete |
| Phase 5: Media | M28-M29 | ✓ Complete |

---

## Quick Start

### Building

```bash
cd macos
make
./RedAlert.app/Contents/MacOS/RedAlert
```

### Distribution

```bash
make dist   # Creates signed zip
make dmg    # Creates DMG image
```

---

## Game Assets

**This repository contains only the game engine - no game assets are included.**

The game and assets are distributed separately:
- **Game Binary:** This repository (GPL v3, ~5 MB)
- **Game Assets:** User must obtain separately (~950 MB)

### Asset Setup

See **[ASSETS.md](ASSETS.md)** for detailed setup instructions.

**Quick version:**

1. Download the freeware ISOs from [Internet Archive](https://archive.org/details/command-and-conquer-red-alert)
2. Extract MIX files to one of these locations:
   - `~/Library/Application Support/RedAlert/assets/`
   - `./assets/` (next to the app bundle)
3. Run the game

### Asset Search Paths

The game searches for assets in this order:
1. `~/Library/Application Support/RedAlert/assets/`
2. `./assets/` (relative to app bundle)
3. `../assets/` (for development)
4. `/Volumes/CD1/INSTALL/` (mounted ISO)

### Required Files

| File | Size | Source | Notes |
|------|------|--------|-------|
| REDALERT.MIX | 24 MB | CD1/INSTALL/ | Core game data (encrypted) |
| MAIN.MIX | 434-500 MB | CD1/ or CD2/ | Campaign data (encrypted) |
| AUD.MIX | 1.4 MB | CD1/SETUP/ | Audio (unencrypted) |
| SETUP.MIX | 12 MB | CD1/SETUP/ | Graphics (unencrypted) |

### Legal Status

- **Source Code:** GPL v3 - Released by EA, February 2025
- **Game Assets:** Freeware - Released by EA, August 2008
- Assets are freely redistributable but not included here to keep repository size small

---

## Target Platform

- **OS:** macOS 14+ (Sonoma)
- **Architecture:** ARM64 (Apple Silicon)
- **Graphics:** Metal
- **Audio:** CoreAudio

---

## Repository Structure

```
CnC_Red_Alert/
├── README.md              # This file
├── ASSETS.md              # Asset setup instructions
├── PORTING_PLAN.md        # Detailed milestone plan
├── archeology.md          # Original source analysis
├── LICENSE.md             # GPL v3
│
├── assets/                # Game assets (not in git, ~950 MB)
├── downloads/             # ISO downloads (not in git, ~1.3 GB)
│
├── original/              # Original Windows source (read-only)
│   ├── CODE/             # Main game logic (~520 files)
│   ├── WIN32LIB/         # Windows library
│   ├── WWFLAT32/         # Optimized ASM library
│   └── */research.md     # Analysis documents
│
└── macos/                 # macOS port
    ├── Makefile
    ├── src/
    │   ├── main.mm       # Entry point
    │   ├── platform/     # Platform abstractions
    │   ├── graphics/     # Metal renderer
    │   ├── audio/        # CoreAudio backend
    │   ├── input/        # Keyboard/mouse
    │   ├── assets/       # Asset loaders
    │   ├── game/         # Game logic
    │   └── ui/           # Menu system
    └── resources/        # App bundle resources
```

---

## Gap Analysis: What's Remaining

### High Priority (P1) - Core Game

| Feature | Original | Port | Status |
|---------|----------|------|--------|
| Object Hierarchy | OBJECT/TECHNO/FOOT.CPP | game/object.cpp | ✓ Complete (45 tests) |
| INI Parser | INI.CPP, CCINI.CPP (~6K) | game/ini.cpp | ✓ Complete (26 tests) |
| Rules System | RULES.CPP (~3K) | game/rules.cpp | ✓ Complete (27 tests) |
| Data Tables | *DATA.CPP | game/*_types.cpp | ✓ Complete |
| Infantry | INFANTRY.CPP (~8K lines) | - | Not started |
| Vehicles | UNIT.CPP (~6K lines) | - | Not started |
| Buildings | BUILDING.CPP (~12K lines) | - | Not started |
| Aircraft | AIRCRAFT.CPP (~8K lines) | - | Not started |
| Map/Cell | MAP.CPP, CELL.CPP (~8K) | game/cell.cpp, mapclass.cpp | ✓ Complete (38 tests) |
| Pathfinding | FINDPATH.CPP (~3K) | game/pathfind.cpp | ✓ Complete |
| Combat | COMBAT.CPP, BULLET.CPP | game/combat.cpp, bullet.cpp | ✓ Complete (28 tests) |

### Medium Priority (P2) - Systems

| Feature | Original | Status |
|---------|----------|--------|
| AI & Houses | HOUSE.CPP, TEAM.CPP | ✓ Complete (28 tests) |
| Scenarios | SCENARIO.CPP | ✓ Complete (26 tests) |
| Triggers | TRIGGER.CPP | ✓ Complete |
| Sidebar | SIDEBAR.CPP, FACTORY.CPP | ✓ Complete (22 tests) |
| Minimap | RADAR.CPP | ✓ Complete (21 tests) |
| Save/Load | SAVELOAD.CPP | ✓ Complete (18 tests) |

### Deferred

| Feature | Reason |
|---------|--------|
| Networking | Not interested for now |
| Video Playback | FFmpeg integration later |
| MIDI Music | CoreMIDI later |

---

## Recommended Porting Order

1. **M15: Data Tables** - Port *DATA.CPP files (pure data, easy)
2. **M17: INI/Rules** - Load RULES.INI for game configuration
3. **M16: Object Hierarchy** - Port ObjectClass → TechnoClass → FootClass
4. **M18: Map/Cell** - Real map loading from scenarios
5. **M19: Entities** - Infantry, Vehicles, Buildings, Aircraft
6. **M20: Combat** - Weapons, bullets, damage
7. **M21: AI** - Computer opponent (✓ Complete)
8. **M22: Scenarios** - Load campaign missions

See [PORTING_PLAN.md](PORTING_PLAN.md) for complete details.

---

## Documentation

- [ASSETS.md](ASSETS.md) - Game asset setup instructions
- [PORTING_PLAN.md](PORTING_PLAN.md) - Complete milestone plan with effort estimates
- [archeology.md](archeology.md) - Analysis of original source code
- [original/research.md](original/research.md) - Overview of original codebase
- `original/*/research.md` - Per-directory analysis

---

## Related Projects

### OpenRA - Open Source RTS Engine

**[OpenRA](https://github.com/OpenRA/OpenRA)** is a mature, actively maintained open-source game engine that recreates the classic C&C games (Red Alert, Tiberian Dawn, Dune 2000).

Use OpenRA as a reference for:
- **Alternative implementations** - When the original source is unclear or poorly documented
- **Cross-referencing behavior** - Verify game mechanics and edge cases
- **Ideas for modern approaches** - See how they solved similar problems
- **File format documentation** - Their codebase has excellent asset loader implementations

Key differences from this port:
| Aspect | This Port | OpenRA |
|--------|-----------|--------|
| Language | C++17 | C# |
| Goal | Faithful to original source | Modern reimplementation |
| Platform | macOS native (Metal) | Cross-platform (OpenGL/SDL) |
| Assets | Uses original formats | Converts to modern formats |
| Multiplayer | Not planned | Fully featured |

OpenRA is especially useful when:
- Original Windows code is x86 assembly or Win32-specific
- Behavior isn't clear from reading original source
- Looking for inspiration on how to structure modern game systems
- Debugging file format parsing issues

---

## License

Original source code is GPL v3 (see [LICENSE.md](LICENSE.md)).

Released by Electronic Arts, February 2025.
