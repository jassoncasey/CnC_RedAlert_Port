# Command & Conquer Red Alert - macOS Port Plan

## Current Status

**Completed Milestones:** 0-29 (All systems built and tested)
**Current Phase:** Phase 6 - Integration (CRITICAL)
**Blocking Issue:** MIX file decryption needed for real assets

---

## CRITICAL: Integration Gap

### The Problem

We have built ~500 tests passing across all game systems, but **most code is NOT integrated into the actual game**. The game currently runs a hardcoded demo with procedural maps and stub graphics.

### What's Actually Used in main.mm

| System | Integrated? | Notes |
|--------|-------------|-------|
| Metal Renderer | ✓ | Working |
| CoreAudio | ✓ | Working |
| Input | ✓ | Working |
| Game Loop | ✓ | Working |
| Menu System | ✓ | Working |
| Map (Demo) | ✓ | **Procedural, not real MapClass** |
| Units (Demo) | ✓ | **Simplified, not real entity classes** |

### What's Built but NOT Integrated (Tests Only)

| System | Tests | Status |
|--------|-------|--------|
| Object Hierarchy | 45 pass | Sitting in tests |
| INI Parser | 26 pass | Sitting in tests |
| Rules System | 27 pass | Sitting in tests |
| Cell/Map System | 38 pass | Sitting in tests |
| Pathfinding | (in map) | Sitting in tests |
| InfantryClass | 35 pass | Sitting in tests |
| UnitClass | (in entity) | Sitting in tests |
| BuildingClass | (in entity) | Sitting in tests |
| AircraftClass | (in entity) | Sitting in tests |
| Combat/Bullets | 28 pass | Sitting in tests |
| AI/Houses | 28 pass | Sitting in tests |
| Scenarios | 26 pass | Sitting in tests |
| Sidebar | 22 pass | Sitting in tests |
| Radar | 21 pass | Sitting in tests |
| Save/Load | 18 pass | Sitting in tests |
| Animations | 21 pass | Sitting in tests |
| Campaigns | 35 pass | Sitting in tests |
| VQA Video | 12 pass | Sitting in tests |
| Music | 24 pass | Sitting in tests |

**Total: ~500 tests passing, ~0% integrated into game**

---

## Porting Roadmap (Updated)

### Phase 6: Integration (NEXT - CRITICAL)

**Goal:** Wire all the tested systems into the actual game

#### Milestone 30: MIX Decryption ← **CURRENT BLOCKER**
**Priority:** P0 | **Status:** In Progress

Without this, we cannot load real game assets from REDALERT.MIX.

- [ ] Port PKStraw from `original/CODE/PKSTRAW.CPP`
- [ ] Port PKey RSA from `original/CODE/PKEY.CPP`
- [ ] Port Blowfish from `original/CODE/BLOWFISH.CPP` (for body decryption)
- [ ] Decrypt REDALERT.MIX header
- [ ] Extract RULES.INI to verify

**Original files:**
- `PKSTRAW.CPP/H` - Public key stream class
- `PKEY.CPP/H` - Public key implementation
- `BLOWFISH.CPP/H` - Blowfish cipher
- `CONST.CPP` - Contains the public key

#### Milestone 31: Asset Pipeline
**Priority:** P0 | **Depends on:** M30

- [ ] Load palettes from MIX (temperat.pal, etc.)
- [ ] Load SHP sprites (infantry, vehicles, buildings)
- [ ] Render real sprites instead of colored rectangles
- [ ] Load sounds from AUD.MIX

#### Milestone 32: System Integration
**Priority:** P0 | **Depends on:** M31

Replace demo systems with real implementations:
- [ ] Replace demo map.cpp with MapClass/CellClass
- [ ] Replace demo units.cpp with InfantryClass/UnitClass/BuildingClass/AircraftClass
- [ ] Integrate ScenarioClass for mission loading
- [ ] Integrate HouseClass for AI
- [ ] Integrate CampaignClass for mission progression

#### Milestone 33: UI Integration
**Priority:** P1 | **Depends on:** M32

- [ ] Integrate SidebarClass (build menu)
- [ ] Integrate RadarClass (minimap)
- [ ] Integrate AnimClass (explosions, effects)
- [ ] Integrate SaveLoad (game persistence)

#### Milestone 34: Media Integration
**Priority:** P2 | **Depends on:** M33

- [ ] Integrate VQA player for cutscenes
- [ ] Integrate Music system for soundtrack
- [ ] Play intro.vqa on startup
- [ ] Play mission briefings

---

### Previously Completed Phases

#### Phase 1: Infrastructure (M0-M14) ✓
Platform layer, Metal, CoreAudio, input, game loop, menus, asset loaders.

#### Phase 2: Core Engine (M15-M19) ✓
Data tables, object hierarchy, INI/rules, map/cell, entity classes.

#### Phase 3: Game Systems (M20-M22) ✓
Combat, AI/teams, scenarios/triggers.

#### Phase 4: UI & Polish (M23-M27) ✓
Sidebar, radar, save/load, animations, campaigns.

#### Phase 5: Media (M28-M29) ✓
VQA video player, music streaming.

---

## Gap Analysis: Original vs Port

### What We Have (Tested)

| Component | Status | Tests | Files |
|-----------|--------|-------|-------|
| Platform Layer | ✓ Complete | - | main.mm, platform/*.cpp |
| Metal Renderer | ✓ Complete | - | graphics/metal/renderer.mm |
| CoreAudio | ✓ Complete | - | audio/audio.mm |
| Input System | ✓ Complete | - | input/input.mm |
| Game Loop | ✓ Complete | - | game/gameloop.cpp |
| Menu System | ✓ Complete | - | ui/menu.cpp |
| Asset Loaders | ✓ Complete | - | assets/*.cpp (MIX, SHP, PAL, AUD) |
| Object Hierarchy | ✓ Complete | 45 | game/object.cpp |
| INI Parser | ✓ Complete | 26 | game/ini.cpp |
| Rules System | ✓ Complete | 27 | game/rules.cpp |
| Map/Cell | ✓ Complete | 38 | game/cell.cpp, mapclass.cpp |
| Pathfinding | ✓ Complete | - | game/pathfind.cpp |
| Infantry | ✓ Complete | 35 | game/infantry.cpp |
| Vehicles | ✓ Complete | - | game/unit.cpp |
| Buildings | ✓ Complete | - | game/building.cpp |
| Aircraft | ✓ Complete | - | game/aircraft.cpp |
| Combat | ✓ Complete | 28 | game/combat.cpp, bullet.cpp |
| AI/Houses | ✓ Complete | 28 | game/house.cpp, team.cpp |
| Scenarios | ✓ Complete | 26 | game/scenario.cpp, trigger.cpp |
| Sidebar | ✓ Complete | 22 | game/sidebar.cpp, factory.cpp |
| Radar | ✓ Complete | 21 | game/radar.cpp |
| Save/Load | ✓ Complete | 18 | game/saveload.cpp |
| Animations | ✓ Complete | 21 | game/anim.cpp |
| Campaigns | ✓ Complete | 35 | game/campaign.cpp |
| VQA Video | ✓ Complete | 12 | video/vqa.cpp |
| Music | ✓ Complete | 24 | video/music.cpp |

### What's Blocking (MIX Decryption)

| File | Size | Encrypted? | Contents |
|------|------|------------|----------|
| REDALERT.MIX | 24 MB | **YES** | Core data, RULES.INI, palettes |
| MAIN.MIX | 434-500 MB | **YES** | Campaign data, movies |
| AUD.MIX | 1.4 MB | No | Setup audio ✓ |
| SETUP.MIX | 12 MB | No | Setup graphics ✓ |

The unencrypted MIX files work. The encrypted ones need RSA decryption.

---

## MIX Encryption Details

The encrypted MIX files use a two-layer encryption:

1. **Header encryption:** RSA with Westwood's public key
2. **Body encryption:** Blowfish with key derived from header

**Original source files:**
```
original/CODE/PKSTRAW.CPP  - PKStraw class (RSA stream)
original/CODE/PKEY.CPP     - PKey class (bignum RSA)
original/CODE/BLOWFISH.CPP - Blowfish cipher
original/CODE/CONST.CPP    - Public key data
original/CODE/MIXFILE.CPP  - MIX file format
```

**Public key location:** `original/CODE/CONST.CPP` line ~230

---

## Completed Milestones Summary

| # | Milestone | Tests | Status |
|---|-----------|-------|--------|
| 0-14 | Infrastructure | - | ✓ Complete |
| 15 | Data Tables | - | ✓ Complete |
| 16 | Object Hierarchy | 45 | ✓ Complete |
| 17 | INI/Rules | 53 | ✓ Complete |
| 18 | Map/Cell | 38 | ✓ Complete |
| 19 | Entities | 35 | ✓ Complete |
| 20 | Combat | 28 | ✓ Complete |
| 21 | AI/Teams | 28 | ✓ Complete |
| 22 | Scenarios | 26 | ✓ Complete |
| 23 | Sidebar | 22 | ✓ Complete |
| 24 | Radar | 21 | ✓ Complete |
| 25 | Save/Load | 18 | ✓ Complete |
| 26 | Animations | 21 | ✓ Complete |
| 27 | Campaigns | 35 | ✓ Complete |
| 28 | VQA Video | 12 | ✓ Complete |
| 29 | Music | 24 | ✓ Complete |
| **Total** | | **~500** | **Tests passing** |

---

## Next Steps (Priority Order)

1. **M30: MIX Decryption** - Unblock asset loading
2. **M31: Asset Pipeline** - Real graphics/sounds
3. **M32: System Integration** - Wire up all tested systems
4. **M33: UI Integration** - Sidebar, radar, effects
5. **M34: Media Integration** - Cutscenes, music

---

## Target Platform

- **OS:** macOS 14+ (Sonoma)
- **Architecture:** ARM64 (Apple Silicon)
- **Graphics:** Metal
- **Audio:** CoreAudio

---

## Related Projects

**[OpenRA](https://github.com/OpenRA/OpenRA)** - Open source RTS engine that recreates C&C games. Useful for:
- Cross-referencing behavior when original source is unclear
- Alternative implementations of complex systems
- File format documentation

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
    │   ├── main.mm          # Entry point (needs integration work)
    │   ├── platform/
    │   ├── graphics/metal/
    │   ├── audio/
    │   ├── input/
    │   ├── assets/          # MIX/SHP/PAL/AUD loaders
    │   ├── game/            # All game logic (tested but not integrated)
    │   ├── video/           # VQA/Music (tested but not integrated)
    │   ├── tests/           # 500+ passing tests
    │   └── ui/
    └── resources/
```
