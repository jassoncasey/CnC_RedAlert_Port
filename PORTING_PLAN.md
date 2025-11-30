# Command & Conquer Red Alert - macOS Port Plan

## Current Status

**Completed Milestones:** 0-19 (Infrastructure + Data Tables + Object Hierarchy + INI/Rules + Map/Cell/Pathfinding + Entity Classes)
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
| Map/Terrain | Complete | game/cell.cpp, mapclass.cpp, pathfind.cpp (38 tests) |
| Units | Demo only | game/units.cpp (12 types, basic AI) |

### What's Missing (Original Has)

#### CRITICAL - Core Game Logic

| Feature | Original Files | Lines | Effort | Priority |
|---------|----------------|-------|--------|----------|
| Object Hierarchy | OBJECT/TECHNO/FOOT.CPP | ~10K | ✓ Complete | P1 |
| Infantry | INFANTRY.CPP, IDATA.CPP | ~8K | ✓ Complete | P1 |
| Vehicles | UNIT.CPP, UDATA.CPP | ~6K | ✓ Complete | P1 |
| Aircraft | AIRCRAFT.CPP, ADATA.CPP | ~8K | ✓ Complete | P1 |
| Buildings | BUILDING.CPP, BDATA.CPP | ~12K | ✓ Complete | P1 |
| House/Faction | HOUSE.CPP, HDATA.CPP | ~10K | High | P1 |
| Combat System | COMBAT.CPP, BULLET.CPP | ~5K | ✓ Complete | P1 |

#### CRITICAL - Map & Display

| Feature | Original Files | Lines | Effort | Priority |
|---------|----------------|-------|--------|----------|
| Map System | MAP.CPP, CELL.CPP | ~8K | ✓ Complete | P1 |
| Pathfinding | FINDPATH.CPP | ~3K | ✓ Complete | P1 |
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

#### Milestone 15: Data Tables & Types ✓ COMPLETE
**Priority:** P1 | **Effort:** 40-60 hours | **Status:** ✓ Done

Port pure data files (platform-independent):
- [x] infantry_types.cpp/h - Infantry definitions (E1-E7, 7 types)
- [x] unit_types.cpp/h - Unit/vehicle definitions (27 types)
- [x] weapon_types.cpp/h - Weapon definitions (38 types)
- [x] building_types.cpp/h - Building definitions (40+ types)
- [x] types.h - Core enums and type definitions

**Verification:**
```bash
make  # All data tables compile and link
```

#### Milestone 16: Object Class Hierarchy ✓ COMPLETE
**Priority:** P1 | **Effort:** 80-100 hours | **Status:** ✓ Done

Port base classes in order:
- [x] object.cpp/h - AbstractClass, ObjectClass
- [x] object.cpp/h - MissionClass (AI orders)
- [x] object.cpp/h - RadioClass (unit communication)
- [x] object.cpp/h - TechnoClass (combat entities)
- [x] object.cpp/h - FootClass (mobile units)
- [x] types.h - RTTIType, RadioMessageType, CloakType, MoveType, etc.

**Verification:**
```bash
make test_objects
# 45/45 tests pass
# - Tests all class types
# - Tests virtual method dispatch
# - Tests inheritance chain
# - Tests Distance/Direction functions
```

#### Milestone 17: INI & Rules System ✓ COMPLETE
**Priority:** P1 | **Effort:** 40-60 hours | **Status:** ✓ Done

Essential for loading game data:
- [x] ini.cpp/h - Generic INI parser (case-insensitive, section/key lookup)
- [x] rules.cpp/h - RULES.INI processing
- [x] resources/RULES.INI - Reference rules file

**Verification:**
```bash
make test_ini     # 26/26 tests pass
make test_rules   # 27/27 tests pass
# - Parses RULES.INI (2978 lines, ~150 sections)
# - Tests General settings (crate, chrono, ore, build speed)
# - Tests IQ settings
# - Tests Difficulty multipliers
# - Tests Country bonuses
```

#### Milestone 18: Map & Cell System ✓ COMPLETE
**Priority:** P1 | **Effort:** 60-80 hours | **Status:** ✓ Done

Replace demo procedural map with real system:
- [x] cell.cpp/h - CellClass with terrain, occupancy, overlapper list
- [x] mapclass.cpp/h - MapClass (128x128 grid, coordinate math)
- [x] pathfind.cpp/h - A* pathfinding with threat avoidance

**Verification:**
```bash
make test_map
# 38/38 tests pass
# - Cell coordinate math, lepton conversions
# - Cell passability queries
# - A* pathfinding (1000 paths in <100ms)
# - Threat avoidance, closest buildable cell
```

#### Milestone 19: Entity Implementation ✓ COMPLETE
**Priority:** P1 | **Effort:** 100-120 hours | **Status:** ✓ Done

Implement game objects:
- [x] infantry.cpp/h - InfantryClass (fear, prone, sub-cell spots)
- [x] unit.cpp/h - UnitClass (turrets, harvesting, MCV deploy)
- [x] building.cpp/h - BuildingClass (production, power, repair)
- [x] aircraft.cpp/h - AircraftClass (flight states, altitude, transport)
- [x] aircraft_types.cpp/h - Aircraft type data (7 types)

**Verification:**
```bash
make test_entities
# 35/35 tests pass
# - Infantry: fear, prone, sub-cell positions, animation
# - Vehicles: turret control, harvester AI, MCV deployment
# - Buildings: factory production, power, repair
# - Aircraft: flight states, helicopter vs plane, transport
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

## Completed Milestones (Phase 1 + Phase 2 Partial)

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
| 15. Data Tables | ✓ | Infantry/unit/weapon/building types | `make` |
| 16. Object Hierarchy | ✓ | Abstract→Object→Mission→Radio→Techno→Foot | `make test_objects` (45 tests) |
| 17. INI/Rules | ✓ | INI parser + RULES.INI processor | `make test_ini test_rules` (53 tests) |
