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

### Phase 1.5: Asset Extraction (PREREQUISITE)

**Goal:** Extract and verify game assets from freeware ISOs

#### Milestone 14.5: Asset Extraction & Verification
**Priority:** P0 | **Effort:** 8-16 hours | **Status:** IN PROGRESS

**Download ISOs:**
- [x] CD1_ALLIED_DISC.ISO (624 MB) from Internet Archive
- [x] CD2_SOVIET_DISC.ISO (646 MB) from Internet Archive

**Extract MIX files to `assets/` directory:**
- [x] REDALERT.MIX (24 MB) - Core game data, palettes, rules (ENCRYPTED)
- [x] MAIN_ALLIED.MIX (434 MB) - Allied campaign movies/sounds (ENCRYPTED)
- [x] MAIN_SOVIET.MIX (477 MB) - Soviet campaign movies/sounds (ENCRYPTED)
- [x] AUD.MIX (1.4 MB) - Setup audio files (unencrypted)
- [x] SETUP.MIX (12 MB) - Setup graphics (unencrypted)

**Verify asset loading:**
- [x] Test MIX file reader with unencrypted files (AUD.MIX - 47 files)
- [ ] Port PKStraw/PKey RSA decryption for encrypted MIX headers
- [ ] Extract and verify RULES.INI from REDALERT.MIX
- [ ] Extract and display a test sprite (e.g., infantry SHP)
- [ ] Extract and play a test sound (e.g., rifle AUD)

**Encryption Note:**
REDALERT.MIX, MAIN_ALLIED.MIX, and MAIN_SOVIET.MIX use encrypted headers.
The encryption uses RSA public key from `original/CODE/CONST.CPP`.
Must port `PKStraw` from `original/CODE/PKSTRAW.CPP` to decrypt.

**Asset directory structure:**
```
assets/                    # Not in git (~950 MB)
├── REDALERT.MIX          # Core data (encrypted header)
├── MAIN_ALLIED.MIX       # Allied campaign (encrypted)
├── MAIN_SOVIET.MIX       # Soviet campaign (encrypted)
├── AUD.MIX               # Setup audio (unencrypted) ✓
└── SETUP.MIX             # Setup graphics (unencrypted) ✓
```

**Verification:**
```bash
cd macos && make test_assets
# Expected: "File count: 47" for AUD.MIX
```

---

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

**Verification:**
```bash
make test_data_tables
# Creates test that:
# - Compiles all data tables
# - Verifies array sizes match original
# - Prints sample values (e.g., "Rifle Infantry: Cost=100, Speed=4")
```

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

**Verification:**
```bash
make test_objects
# Creates test that:
# - Instantiates each class type
# - Tests virtual method dispatch
# - Verifies inheritance chain
# - Tests object pool allocation
```

#### Milestone 17: INI & Rules System
**Priority:** P1 | **Effort:** 40-60 hours

Essential for loading game data:
- [ ] INI.CPP/H - Generic INI parser
- [ ] CCINI.CPP/H - C&C-specific INI handling
- [ ] RULES.CPP/H - RULES.INI processing
- [ ] Integrate with type definitions

**Verification:**
```bash
make test_ini
# Creates test that:
# - Parses RULES.INI from REDALERT.MIX
# - Prints unit costs, build times
# - Verifies section count matches original (~150 sections)
# - Tests Get/Put for various data types
```

#### Milestone 18: Map & Cell System
**Priority:** P1 | **Effort:** 60-80 hours

Replace demo procedural map with real system:
- [ ] CELL.CPP/H - Map cell representation
- [ ] MAP.CPP/H - Map data management
- [ ] TERRAIN.CPP/H - Terrain types
- [ ] FINDPATH.CPP/H - A* pathfinding

**Verification:**
```bash
make test_map
# Creates test that:
# - Loads a scenario map (e.g., SCG01EA.INI)
# - Verifies map dimensions
# - Tests cell passability queries
# - Runs pathfinding benchmark (1000 paths in <1 second)
```

#### Milestone 19: Entity Implementation
**Priority:** P1 | **Effort:** 100-120 hours

Implement game objects:
- [ ] INFANTRY.CPP/H - Infantry units
- [ ] UNIT.CPP/H - Vehicles
- [ ] BUILDING.CPP/H - Structures
- [ ] AIRCRAFT.CPP/H - Air units
- [ ] VESSEL.CPP/H - Naval units

**Verification:**
```bash
make test_entities
# Creates test that:
# - Creates one of each entity type
# - Tests rendering (draws sprite at correct position)
# - Tests basic movement
# - Tests state machine transitions
```

**Phase 2 Integration Test:**
```bash
make test_phase2
# Runs all Phase 2 tests in sequence
# Final test: Load SCG01EA, spawn units, verify they render
```

---

### Phase 3: Game Systems

#### Milestone 20: Combat & Weapons
**Priority:** P1 | **Effort:** 60-80 hours

- [ ] COMBAT.CPP - Combat resolution
- [ ] BULLET.CPP - Projectiles
- [ ] WEAPON.CPP - Weapon types
- [ ] WARHEAD.CPP - Damage types
- [ ] Integrate with units

**Verification:**
```bash
make test_combat
# Creates test that:
# - Spawns two opposing units
# - Commands attack
# - Verifies damage calculation
# - Verifies unit death when HP reaches 0
# - Tests weapon range and cooldown
```

#### Milestone 21: AI & Teams
**Priority:** P2 | **Effort:** 80-100 hours

- [ ] HOUSE.CPP - Faction management
- [ ] TEAM.CPP - AI team control
- [ ] TEAMTYPE.CPP - Team templates
- [ ] Basic computer opponent

**Verification:**
```bash
make test_ai
# Creates test that:
# - Creates AI house with starting units
# - Runs 100 game ticks
# - Verifies AI issues orders (move, attack, build)
# - Verifies team formation
```

#### Milestone 22: Scenarios & Triggers
**Priority:** P2 | **Effort:** 60-80 hours

- [ ] SCENARIO.CPP - Mission loading
- [ ] TRIGGER.CPP - Event triggers
- [ ] Load original campaign missions
- [ ] Victory/defeat conditions

**Verification:**
```bash
make test_scenarios
# Creates test that:
# - Loads SCG01EA (first Allied mission)
# - Verifies starting units match original
# - Verifies trigger count
# - Tests victory condition detection
```

**Phase 3 Integration Test:**
```bash
make test_phase3
# Full combat test:
# - Load scenario
# - AI controls enemy
# - Player units auto-attack
# - Victory/defeat triggers fire
```

---

### Phase 4: UI & Polish

#### Milestone 23: Sidebar & Build Menu
**Priority:** P2 | **Effort:** 60-80 hours

- [ ] SIDEBAR.CPP - Build interface
- [ ] Production queue
- [ ] Power system
- [ ] Credits/money

**Verification:**
```bash
make test_sidebar
# Creates test that:
# - Renders sidebar
# - Tests build button clicks
# - Verifies production queue timing
# - Tests power/credit display
```

#### Milestone 24: Minimap & Radar
**Priority:** P2 | **Effort:** 40-60 hours

- [ ] RADAR.CPP - Minimap display
- [ ] Fog of war
- [ ] Shroud system

**Verification:**
```bash
make test_radar
# Creates test that:
# - Renders minimap at 160x160
# - Verifies unit dots appear
# - Tests shroud reveal
# - Tests click-to-scroll
```

#### Milestone 25: Save/Load
**Priority:** P2 | **Effort:** 40-60 hours

- [ ] SAVELOAD.CPP - Game state serialization
- [ ] Save file format
- [ ] Load game menu

**Verification:**
```bash
make test_saveload
# Creates test that:
# - Saves game state to file
# - Loads game state
# - Verifies unit positions match
# - Verifies credits/power match
```

#### Milestone 26: Animations & Effects
**Priority:** P3 | **Effort:** 40-60 hours

- [ ] ANIM.CPP - Animation system
- [ ] Explosions
- [ ] Death animations
- [ ] Special effects

**Verification:**
```bash
make test_animations
# Creates test that:
# - Plays explosion animation
# - Verifies frame count and timing
# - Tests animation pooling
```

#### Milestone 27: Campaigns
**Priority:** P3 | **Effort:** 60-80 hours

- [ ] Allied campaign (14 missions)
- [ ] Soviet campaign (14 missions)
- [ ] Mission briefings
- [ ] Story progression

**Verification:**
```bash
make test_campaigns
# Creates test that:
# - Loads each campaign mission
# - Verifies mission briefing text
# - Verifies starting conditions
# - Tests mission completion flow
```

**Phase 4 Integration Test:**
```bash
make test_phase4
# Full game test:
# - Start new game
# - Complete first mission
# - Save/load mid-mission
# - Verify sidebar and minimap
```

---

### Phase 5: Media (Deferred)

#### Milestone 28: Video Playback
- [ ] FFmpeg integration for VQA
- [ ] Cutscenes between missions

**Verification:**
```bash
make test_video
# Plays intro.vqa, verifies frame rate
```

#### Milestone 29: Music
- [ ] CoreMIDI for MIDI playback
- [ ] Music tracks from SCORES.MIX

**Verification:**
```bash
make test_music
# Plays HELLMARCH.AUD, verifies audio output
```

---

## Recommended Order

| Order | Milestone | Why First | Verification |
|-------|-----------|-----------|--------------|
| 0 | M14.5: Asset Verification | Verify MIX reader works with real assets | `make test_assets` |
| 1 | M15: Data Tables | Foundation - no dependencies | `make test_data_tables` |
| 2 | M17: INI/Rules | Needed to load game data | `make test_ini` |
| 3 | M16: Object Hierarchy | Core abstractions for everything | `make test_objects` |
| 4 | M18: Map/Cell | Needed before entities | `make test_map` |
| 5 | M19: Entities | Main game objects | `make test_entities` |
| 6 | M20: Combat | Makes gameplay functional | `make test_combat` |
| 7 | M21: AI | Computer opponent | `make test_ai` |
| 8 | M22: Scenarios | Load real missions | `make test_scenarios` |
| 9 | M23: Sidebar | Build units/structures | `make test_sidebar` |
| 10 | M24: Minimap | Navigation aid | `make test_radar` |
| 11 | M25: Save/Load | Game persistence | `make test_saveload` |
| 12 | M26: Animations | Visual polish | `make test_animations` |
| 13 | M27: Campaigns | Full game content | `make test_campaigns` |

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
├── ASSETS.md                 # Asset setup instructions
├── PORTING_PLAN.md          # This document
├── archeology.md            # Original source analysis
├── LICENSE.md               # GPL v3
│
├── assets/                  # Game assets (gitignored, ~950 MB)
├── downloads/               # ISO downloads (gitignored, ~1.3 GB)
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
    │   ├── tests/          # Verification tests
    │   └── ui/
    └── resources/
```

---

## Asset Strategy

**Game assets are NOT in this repository.**

See [ASSETS.md](ASSETS.md) for complete setup instructions.

The game searches for assets in multiple locations:
1. `~/Library/Application Support/RedAlert/assets/`
2. `./assets/` (relative to app bundle)
3. `../assets/` (for development)
4. `/Volumes/CD1/INSTALL/` (mounted ISO)

---

## Completed Milestones (Phase 1)

| Milestone | Status | Description | Verification |
|-----------|--------|-------------|--------------|
| 0. Repo Setup | ✓ | Directory structure | Files exist |
| 1. Minimal Build | ✓ | AppKit window, Metal view | Window opens |
| 2. Compat Layer | ✓ | Windows API stubs | Compiles |
| 3. File I/O | ✓ | POSIX abstraction | `make test_file` |
| 4. Timing | ✓ | Game timer | `make test_timing` |
| 5. Stub Assets | ✓ | Placeholder graphics/audio | Demo renders |
| 6. Graphics | ✓ | Metal renderer | Primitives draw |
| 7. Input | ✓ | Keyboard/mouse | Input responds |
| 8. Game Loop | ✓ | Core loop | 60 FPS |
| 9. Rendering | ✓ | Drawing primitives | All shapes render |
| 10. Audio | ✓ | CoreAudio | Tones play |
| 11. Menus | ✓ | Navigation system | Menu navigates |
| 12. Real Assets | ✓ | MIX/SHP/PAL/AUD loaders | `make test_assets` |
| 13. Gameplay | ✓ | Demo mission | Units move/fight |
| 14. Polish | ✓ | App bundle, icon, fullscreen | App launches |
