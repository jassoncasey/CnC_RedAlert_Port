# Command & Conquer Red Alert - macOS Port Plan

## Current Status

**Completed Milestones:** 0-14 (Infrastructure complete)
**Current Phase:** Phase 2 - Game Content Integration

---

## Gap Analysis: Original vs Port

### What We Have (Port)

| Component | Status | Files |
|-----------|--------|-------|
| Platform Layer | Complete | main.mm, platform/*.cpp |
| Metal Renderer | Complete | graphics/metal/renderer.mm |
| CoreAudio | Complete | audio/audio.mm |
| Input System | Complete | input/input.mm |
| Game Loop | Complete | game/gameloop.cpp |
| Menu System | Complete | ui/menu.cpp |
| Asset Loaders | Complete | assets/*.cpp (MIX, SHP, PAL, AUD) |
| Map/Terrain | Demo only | game/map.cpp (procedural 64x64) |
| Units | Demo only | game/units.cpp (12 types, basic AI) |

### What's Missing (Original Has)

#### CRITICAL - Core Game Logic

| Feature | Original Files | Lines | Effort | Priority |
|---------|----------------|-------|--------|----------|
| Object Hierarchy | OBJECT/TECHNO/FOOT.CPP | ~10K | High | P1 |
| Infantry | INFANTRY.CPP, IDATA.CPP | ~8K | High | P1 |
| Vehicles | UNIT.CPP, UDATA.CPP | ~6K | High | P1 |
| Aircraft | AIRCRAFT.CPP, ADATA.CPP | ~8K | High | P1 |
| Buildings | BUILDING.CPP, BDATA.CPP | ~12K | High | P1 |
| House/Faction | HOUSE.CPP, HDATA.CPP | ~10K | High | P1 |
| Combat System | COMBAT.CPP, BULLET.CPP | ~5K | High | P1 |

#### CRITICAL - Map & Display

| Feature | Original Files | Lines | Effort | Priority |
|---------|----------------|-------|--------|----------|
| Map System | MAP.CPP, CELL.CPP | ~8K | High | P1 |
| Pathfinding | FINDPATH.CPP | ~3K | Medium | P1 |
| Sidebar | SIDEBAR.CPP | ~4K | High | P2 |
| Radar/Minimap | RADAR.CPP | ~4K | Medium | P2 |
| Terrain | TERRAIN.CPP, TDATA.CPP | ~4K | Medium | P1 |
| Overlays | OVERLAY.CPP, ODATA.CPP | ~3K | Medium | P2 |

#### CRITICAL - Game Systems

| Feature | Original Files | Lines | Effort | Priority |
|---------|----------------|-------|--------|----------|
| AI System | TEAM.CPP, TEAMTYPE.CPP | ~7K | High | P2 |
| Triggers | TRIGGER.CPP | ~2K | Medium | P2 |
| Scenarios | SCENARIO.CPP | ~5K | High | P2 |
| INI Parser | INI.CPP, CCINI.CPP | ~6K | Medium | P1 |
| Rules | RULES.CPP | ~3K | Medium | P1 |
| Animations | ANIM.CPP | ~3K | Medium | P2 |

#### IMPORTANT - UI & Effects

| Feature | Original Files | Lines | Effort | Priority |
|---------|----------------|-------|--------|----------|
| Cursors | MOUSE.CPP | ~2K | Low | P3 |
| Dialog System | DIALOG.CPP, GADGET.CPP | ~5K | Medium | P3 |
| Score Screen | SCORE.CPP | ~3K | Low | P3 |
| Save/Load | SAVELOAD.CPP | ~4K | Medium | P2 |
| Options | OPTIONS.CPP | ~2K | Low | P3 |

#### DEFERRED - Not Porting Now

| Feature | Reason | Files |
|---------|--------|-------|
| Networking | Deferred per request | IPX.CPP, TCPIP.CPP, SESSION.CPP |
| Westwood Online | Defunct service | WOL_*.CPP |
| Video Playback | Use FFmpeg later | VQ*.CPP |
| MIDI Music | CoreMIDI later | |
| Modem/Serial | Obsolete | |

---

## Porting Roadmap

### Phase 2: Core Game Engine (Current)

**Goal:** Port the fundamental game logic from original CODE/ directory

#### Milestone 15: Data Tables & Types
**Priority:** P1 | **Effort:** 40-60 hours

Port pure data files (platform-independent):
- [ ] BDATA.CPP - Building definitions
- [ ] UDATA.CPP - Unit/vehicle definitions
- [ ] IDATA.CPP - Infantry definitions
- [ ] ADATA.CPP - Aircraft definitions
- [ ] HDATA.CPP - House/faction definitions
- [ ] TDATA.CPP - Terrain definitions
- [ ] ODATA.CPP - Overlay definitions
- [ ] BBDATA.CPP - Bullet/projectile definitions

These are mostly static arrays - minimal adaptation needed.

#### Milestone 16: Object Class Hierarchy
**Priority:** P1 | **Effort:** 80-100 hours

Port base classes in order:
- [ ] OBJECT.CPP/H - Base ObjectClass
- [ ] MISSION.CPP/H - MissionClass (AI orders)
- [ ] RADIO.CPP/H - RadioClass (unit communication)
- [ ] TECHNO.CPP/H - TechnoClass (combat entities)
- [ ] FOOT.CPP/H - FootClass (mobile units)
- [ ] DRIVE.CPP/H - DriveClass (wheeled/tracked)

Each builds on previous. Test incrementally.

#### Milestone 17: INI & Rules System
**Priority:** P1 | **Effort:** 40-60 hours

Essential for loading game data:
- [ ] INI.CPP/H - Generic INI parser
- [ ] CCINI.CPP/H - C&C-specific INI handling
- [ ] RULES.CPP/H - RULES.INI processing
- [ ] Integrate with type definitions

#### Milestone 18: Map & Cell System
**Priority:** P1 | **Effort:** 60-80 hours

Replace demo procedural map with real system:
- [ ] CELL.CPP/H - Map cell representation
- [ ] MAP.CPP/H - Map data management
- [ ] TERRAIN.CPP/H - Terrain types
- [ ] FINDPATH.CPP/H - A* pathfinding

#### Milestone 19: Entity Implementation
**Priority:** P1 | **Effort:** 100-120 hours

Implement game objects:
- [ ] INFANTRY.CPP/H - Infantry units
- [ ] UNIT.CPP/H - Vehicles
- [ ] BUILDING.CPP/H - Structures
- [ ] AIRCRAFT.CPP/H - Air units
- [ ] VESSEL.CPP/H - Naval units

---

### Phase 3: Game Systems

#### Milestone 20: Combat & Weapons
**Priority:** P1 | **Effort:** 60-80 hours

- [ ] COMBAT.CPP - Combat resolution
- [ ] BULLET.CPP - Projectiles
- [ ] WEAPON.CPP - Weapon types
- [ ] WARHEAD.CPP - Damage types
- [ ] Integrate with units

#### Milestone 21: AI & Teams
**Priority:** P2 | **Effort:** 80-100 hours

- [ ] HOUSE.CPP - Faction management
- [ ] TEAM.CPP - AI team control
- [ ] TEAMTYPE.CPP - Team templates
- [ ] Basic computer opponent

#### Milestone 22: Scenarios & Triggers
**Priority:** P2 | **Effort:** 60-80 hours

- [ ] SCENARIO.CPP - Mission loading
- [ ] TRIGGER.CPP - Event triggers
- [ ] Load original campaign missions
- [ ] Victory/defeat conditions

---

### Phase 4: UI & Polish

#### Milestone 23: Sidebar & Build Menu
**Priority:** P2 | **Effort:** 60-80 hours

- [ ] SIDEBAR.CPP - Build interface
- [ ] Production queue
- [ ] Power system
- [ ] Credits/money

#### Milestone 24: Minimap & Radar
**Priority:** P2 | **Effort:** 40-60 hours

- [ ] RADAR.CPP - Minimap display
- [ ] Fog of war
- [ ] Shroud system

#### Milestone 25: Save/Load
**Priority:** P2 | **Effort:** 40-60 hours

- [ ] SAVELOAD.CPP - Game state serialization
- [ ] Save file format
- [ ] Load game menu

#### Milestone 26: Animations & Effects
**Priority:** P3 | **Effort:** 40-60 hours

- [ ] ANIM.CPP - Animation system
- [ ] Explosions
- [ ] Death animations
- [ ] Special effects

#### Milestone 27: Campaigns
**Priority:** P3 | **Effort:** 60-80 hours

- [ ] Allied campaign (14 missions)
- [ ] Soviet campaign (14 missions)
- [ ] Mission briefings
- [ ] Story progression

---

### Phase 5: Media (Deferred)

#### Milestone 28: Video Playback
- [ ] FFmpeg integration for VQA
- [ ] Cutscenes between missions

#### Milestone 29: Music
- [ ] CoreMIDI for MIDI playback
- [ ] Music tracks from SCORES.MIX

---

## Recommended Order

| Order | Milestone | Why First |
|-------|-----------|-----------|
| 1 | M15: Data Tables | Foundation - no dependencies |
| 2 | M17: INI/Rules | Needed to load game data |
| 3 | M16: Object Hierarchy | Core abstractions for everything |
| 4 | M18: Map/Cell | Needed before entities |
| 5 | M19: Entities | Main game objects |
| 6 | M20: Combat | Makes gameplay functional |
| 7 | M21: AI | Computer opponent |
| 8 | M22: Scenarios | Load real missions |
| 9 | M23: Sidebar | Build units/structures |
| 10 | M24: Minimap | Navigation aid |
| 11 | M25: Save/Load | Game persistence |
| 12 | M26: Animations | Visual polish |
| 13 | M27: Campaigns | Full game content |

---

## Effort Estimate

| Phase | Milestones | Hours |
|-------|------------|-------|
| Phase 1 (Done) | M0-M14 | ~600 |
| Phase 2 | M15-M19 | 320-420 |
| Phase 3 | M20-M22 | 200-260 |
| Phase 4 | M23-M27 | 240-340 |
| Phase 5 | M28-M29 | 80-120 |
| **Total Remaining** | | **840-1140** |

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
├── README.md                 # Project overview
├── PORTING_PLAN.md          # This document
├── archeology.md            # Original source analysis
├── LICENSE.md               # GPL v3
│
├── downloads/               # Game assets (gitignored)
│   ├── CD1_ALLIED_DISC.ISO
│   └── CD2_SOVIET_DISC.ISO
│
├── original/                # Original source (read-only)
│   ├── CODE/               # Main game logic
│   ├── WIN32LIB/           # Windows library
│   ├── WWFLAT32/           # Optimized library
│   └── */research.md       # Analysis docs
│
└── macos/                   # macOS port
    ├── Makefile
    ├── src/
    │   ├── main.mm
    │   ├── platform/
    │   ├── graphics/metal/
    │   ├── audio/
    │   ├── input/
    │   ├── assets/
    │   ├── game/
    │   └── ui/
    └── resources/
```

---

## Asset Strategy

**Game assets are NOT in this repository.**

The ISOs from the 2008 freeware release contain:
- `REDALERT.MIX` - Core game data
- `CONQUER.MIX` - Main assets
- `LOCAL.MIX` - Localized strings
- `GENERAL.MIX` - General assets
- `INTERIOR.MIX` - Building interiors
- `SCORES.MIX` - Music
- `SOUNDS.MIX` - Sound effects
- `MOVIES.MIX` - Video cutscenes

---

## Completed Milestones (Phase 1)

| Milestone | Status | Description |
|-----------|--------|-------------|
| 0. Repo Setup | ✓ | Directory structure |
| 1. Minimal Build | ✓ | AppKit window, Metal view |
| 2. Compat Layer | ✓ | Windows API stubs |
| 3. File I/O | ✓ | POSIX abstraction |
| 4. Timing | ✓ | Game timer |
| 5. Stub Assets | ✓ | Placeholder graphics/audio |
| 6. Graphics | ✓ | Metal renderer |
| 7. Input | ✓ | Keyboard/mouse |
| 8. Game Loop | ✓ | Core loop |
| 9. Rendering | ✓ | Drawing primitives |
| 10. Audio | ✓ | CoreAudio |
| 11. Menus | ✓ | Navigation system |
| 12. Real Assets | ✓ | MIX/SHP/PAL/AUD loaders |
| 13. Gameplay | ✓ | Demo mission |
| 14. Polish | ✓ | App bundle, icon, fullscreen |
