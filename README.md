# Command & Conquer: Red Alert - macOS Port

Native macOS port of the classic Command & Conquer: Red Alert (1996).

## Current Status

**Phase 1 Complete** - Infrastructure and demo gameplay working.
**Phase 2 Starting** - Porting original game logic.

### Completed (Milestones 0-14)

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

### In Progress (Milestones 15+)

See [PORTING_PLAN.md](PORTING_PLAN.md) for detailed roadmap.

| Phase | Milestones | Status |
|-------|------------|--------|
| Phase 2: Core Engine | M15-M19 | Next |
| Phase 3: Game Systems | M20-M22 | Planned |
| Phase 4: UI & Polish | M23-M27 | Planned |
| Phase 5: Media | M28-M29 | Deferred |

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
├── PORTING_PLAN.md        # Detailed milestone plan
├── archeology.md          # Original source analysis
├── LICENSE.md             # GPL v3
│
├── downloads/             # Game assets (not in repo)
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

## Game Assets

**Assets are NOT included in this repository.**

The original game was released as freeware by EA in August 2008. Download from:

- [Internet Archive](https://archive.org/details/command-and-conquer-red-alert) - ISO images
- [CnC Communications Center](https://cnc-comm.com/red-alert/downloads/the-game) - Windows installers

Required MIX files from the ISOs:
- `REDALERT.MIX` - Core game data
- `CONQUER.MIX` - Main assets
- `SOUNDS.MIX` - Sound effects
- `SCORES.MIX` - Music

---

## Gap Analysis: What's Remaining

### High Priority (P1) - Core Game

| Feature | Original | Port | Status |
|---------|----------|------|--------|
| Object Hierarchy | OBJECT/TECHNO/FOOT.CPP | game/units.cpp | Demo only |
| Infantry | INFANTRY.CPP (~8K lines) | - | Not started |
| Vehicles | UNIT.CPP (~6K lines) | - | Not started |
| Buildings | BUILDING.CPP (~12K lines) | - | Not started |
| Aircraft | AIRCRAFT.CPP (~8K lines) | - | Not started |
| Map/Cell | MAP.CPP, CELL.CPP (~8K) | game/map.cpp | Procedural only |
| INI Parser | INI.CPP, CCINI.CPP (~6K) | - | Not started |
| Pathfinding | FINDPATH.CPP (~3K) | - | Not started |
| Combat | COMBAT.CPP, BULLET.CPP | game/units.cpp | Basic only |

### Medium Priority (P2) - Systems

| Feature | Original | Status |
|---------|----------|--------|
| AI Teams | TEAM.CPP, TEAMTYPE.CPP | Not started |
| Scenarios | SCENARIO.CPP | Not started |
| Triggers | TRIGGER.CPP | Not started |
| Sidebar | SIDEBAR.CPP | Not started |
| Minimap | RADAR.CPP | Not started |
| Save/Load | SAVELOAD.CPP | Not started |

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
7. **M21: AI** - Computer opponent
8. **M22: Scenarios** - Load campaign missions

See [PORTING_PLAN.md](PORTING_PLAN.md) for complete details.

---

## Documentation

- [PORTING_PLAN.md](PORTING_PLAN.md) - Complete milestone plan with effort estimates
- [archeology.md](archeology.md) - Analysis of original source code
- [original/research.md](original/research.md) - Overview of original codebase
- `original/*/research.md` - Per-directory analysis

---

## License

Original source code is GPL v3 (see [LICENSE.md](LICENSE.md)).

Released by Electronic Arts, February 2025.
