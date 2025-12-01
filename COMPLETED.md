# Completed Milestones

This document archives completed work. See [PORTING_PLAN.md](PORTING_PLAN.md) for current roadmap.

---

## Phase 1: Infrastructure (M0-M14) - COMPLETE

### M0-M1: Build System
- Makefile for clang/ARM64/macOS 14+
- App bundle generation with Info.plist
- Distribution targets (make dist, make dmg)

### M2-M3: Platform Layer
- main.mm with AppKit window
- NSApplication delegate
- Event loop integration

### M4-M5: Metal Renderer
- Metal device/queue initialization
- 640x400 framebuffer with scaling
- Palette-based rendering (8-bit indexed → RGBA)
- Sprite blitting with transparency
- Primitive drawing (lines, rectangles, circles)
- Text rendering (8x8 bitmap font)

### M6-M7: CoreAudio
- AudioUnit-based mixer
- 16 simultaneous channels
- Sample rate conversion (any → 44100 Hz)
- 8-bit and 16-bit PCM support
- Per-channel volume and pan

### M8-M9: Input System
- Keyboard event handling
- Mouse tracking and clicks
- Box selection support

### M10-M11: Asset Loaders
- MIX archive reader (encrypted and unencrypted)
- Blowfish decryption for TD-era MIX
- SHP sprite loader with LCW decompression
- PAL palette loader (6→8 bit scaling)
- AUD audio loader with IMA ADPCM
- TMP terrain tile loader

### M12-M14: Game Loop & Menus
- Fixed timestep game loop
- Menu system (main, options, credits)
- Keyboard/mouse navigation
- Sound effects for UI

---

## Phase 2: Core Engine (M15-M19) - COMPLETE

### M15: Data Tables
- InfantryTypeClass data (~20 infantry types)
- UnitTypeClass data (~30 vehicle types)
- BuildingTypeClass data (~50 building types)
- AircraftTypeClass data (~10 aircraft types)
- WeaponTypeClass data (~40 weapons)
- WarheadTypeClass data
- BulletTypeClass data

### M16: Object Hierarchy (45 tests)
- AbstractClass base
- ObjectClass (game objects)
- MissionClass (mission queue)
- RadioClass (unit communication)
- TechnoClass (player-controlled objects)
- FootClass (mobile units)

### M17: INI/Rules System (53 tests)
- INI parser (section/key/value)
- RULES.INI loader
- Type data overrides
- Game constants

### M18: Map/Cell System (38 tests)
- CellClass (terrain, occupancy, overlays)
- MapClass (cell grid, queries)
- Coordinate systems (cell, lepton, pixel)
- Terrain passability

### M19: Entity Classes (35 tests)
- InfantryClass
- UnitClass (vehicles)
- BuildingClass
- AircraftClass

---

## Phase 3: Game Systems (M20-M22) - COMPLETE

### M20: Combat System (28 tests)
- Weapon firing logic
- Bullet/projectile tracking
- Damage application
- Warhead effects
- Death handling

### M21: AI & Teams (28 tests)
- HouseClass (player/AI state)
- TeamClass (unit groups)
- TeamTypeClass (team templates)
- AI decision making

### M22: Scenarios & Triggers (26 tests)
- ScenarioClass (mission state)
- TriggerClass (event→action)
- TriggerTypeClass (trigger templates)
- Mission objectives

---

## Phase 4: UI Systems (M23-M25) - COMPLETE

### M23: Sidebar (22 tests)
- SidebarClass (build menu manager)
- StripClass (build columns)
- Build button management
- Scroll support
- Production linking

### M24: Radar (21 tests)
- RadarClass (minimap)
- Terrain coloring
- Unit/building display
- Viewport indicator
- Click-to-scroll

### M25: Save/Load (18 tests)
- Game state serialization
- Save file format
- Load/restore logic
- Checksum validation

---

## Phase 5: Polish Systems (M26-M29) - COMPLETE

### M26: Animations (21 tests)
- AnimClass (visual effects)
- AnimTypeClass (animation data)
- Explosion effects
- Muzzle flashes
- Building animations

### M27: Campaigns (35 tests)
- CampaignClass (mission progression)
- Mission unlocking
- Briefing support
- Victory/defeat handling

### M28: VQA Video (12 tests)
- VQA format parser
- Frame decompression
- Audio track extraction
- Playback timing

### M29: Music System (24 tests)
- AUD streaming
- Track management
- Crossfading
- Volume control

---

## Phase 6: Integration (M30-M32) - PARTIAL

### M30: Asset Pipeline - COMPLETE
- MIX CRC calculation fixed (signed comparison)
- SHP sprite loading from conquer.mix
- PAL palette loading from snow.mix
- TMP terrain tiles from snow.mix
- 20 sprites, 182 terrain tiles loaded

### M31: Demo Gameplay - COMPLETE
- Procedural map generation (realistic terrain)
- A* pathfinding for unit movement
- Combat system integration
- Unit spawning and control
- Basic selection and commands

### M32: System Integration - COMPLETE
- Demo map.cpp functional
- Demo units.cpp functional
- Sidebar/Radar rendering functional
- Full entity classes built (~400 tests pass)

---

## Phase 7: Playable Demo (M33-M37) - COMPLETE

### M33: Production System - COMPLETE
- Click sidebar to start production
- Progress bar during build
- Unit spawns at production building when complete
- Credits deducted on purchase

### M34: Building Placement - COMPLETE
- Click structure in sidebar → construction starts
- "READY" state enters placement mode
- Footprint follows cursor (green=valid, red=invalid)
- Click to place, ESC/right-click cancels (refunds)
- Building appears on radar

### M35: Tech Tree - COMPLETE
- Prerequisites tracked via bitmask
- Unavailable items greyed out
- Building unlocks dependent items
- Cost shown in red when can't afford

### M36: Resource Economy - COMPLETE
- Ore fields on map
- Harvester auto-harvests when idle
- Returns to refinery when 75%+ full
- Credits increase on ore delivery
- Depleted cells become clear terrain

### M37: AI Opponent - COMPLETE
- AI player with own credits/buildings
- Follows build order (power → refinery → barracks → factory)
- Produces units when affordable
- Sends periodic attack waves
- Three difficulty levels

---

## Phase 8: Combat Polish (M38-M40) - COMPLETE

### M38: Fog of War - COMPLETE
- Map starts black (shroud)
- Units/buildings reveal area (sight range)
- Previously seen areas dimmed
- Enemy units hidden outside sight range

### M39: Attack Commands - COMPLETE
- Attack-move (A + right-click)
- Force-attack (Ctrl + right-click)
- Guard (G key)
- Stop (S key)

### M40: Unit Behaviors - COMPLETE
- Auto-acquire targets
- Return fire when attacked
- Infantry scatter from explosions
- Tanks crush infantry

---

## Phase 9: Campaign Support (M41-M44) - COMPLETE

### M41: Scenario Loading - COMPLETE
- MissionData structure
- INI parser for mission files
- Unit/building type string parsing
- Mission_Start() spawns entities
- Demo mission uses mission system

### M42: Mission Objectives - COMPLETE
- Victory/defeat detection
- Result overlay screens
- Auto-return to menu on dismiss
- HUD shows mission name

### M43: Campaign Flow - COMPLETE
- Campaign selection (Allied/Soviet)
- Difficulty selection
- Skirmish mode option
- Campaign infrastructure ready

### M44: Briefings - COMPLETE
- Briefing screen with mission name
- Word-wrapped text
- COMMENCE button
- Integrated into campaign flow

---

## Phase 10: Media (M45-M46) - COMPLETE

### M45: VQA Cutscenes - COMPLETE
- VQA player with full decoder
- Video playback screen
- Skippable videos
- Automatic intro on startup

### M46: Background Music - COMPLETE
- SCORES.MIX loading (21 tracks)
- AUD streaming (IMA ADPCM)
- Volume control, fade in/out
- Shuffle and playlist support

---

## Test Summary

| System | Tests Passing |
|--------|---------------|
| Object Hierarchy | 45 |
| INI/Rules | 53 |
| Map/Cell | 38 |
| Entity Classes | 35 |
| Combat | 28 |
| AI/Teams | 28 |
| Scenarios | 26 |
| Sidebar | 22 |
| Radar | 21 |
| Save/Load | 18 |
| Animations | 21 |
| Campaigns | 35 |
| VQA Video | 12 |
| Music | 24 |
| **Total** | **~400** |

---

## Files Created

```
macos/
├── src/
│   ├── main.mm
│   ├── platform/
│   │   ├── file.cpp
│   │   ├── timing.cpp
│   │   ├── assets.cpp
│   │   └── asset_paths.cpp
│   ├── graphics/metal/
│   │   └── renderer.mm
│   ├── audio/
│   │   └── audio.mm
│   ├── input/
│   │   └── input.mm
│   ├── assets/
│   │   ├── mixfile.cpp
│   │   ├── shpfile.cpp
│   │   ├── palfile.cpp
│   │   ├── audfile.cpp
│   │   ├── tmpfile.cpp
│   │   └── assetloader.cpp
│   ├── crypto/
│   │   ├── blowfish.cpp
│   │   └── mixkey.cpp
│   ├── game/
│   │   ├── gameloop.cpp
│   │   ├── map.cpp
│   │   ├── units.cpp
│   │   ├── sprites.cpp
│   │   ├── sounds.cpp
│   │   ├── terrain.cpp
│   │   ├── object.cpp
│   │   ├── cell.cpp
│   │   ├── mapclass.cpp
│   │   ├── pathfind.cpp
│   │   ├── infantry.cpp
│   │   ├── unit.cpp
│   │   ├── building.cpp
│   │   ├── aircraft.cpp
│   │   ├── bullet.cpp
│   │   ├── combat.cpp
│   │   ├── house.cpp
│   │   ├── team.cpp
│   │   ├── scenario.cpp
│   │   ├── trigger.cpp
│   │   ├── factory.cpp
│   │   ├── sidebar.cpp
│   │   ├── radar.cpp
│   │   ├── saveload.cpp
│   │   ├── anim.cpp
│   │   ├── campaign.cpp
│   │   ├── ini.cpp
│   │   ├── rules.cpp
│   │   └── *_types.cpp (5 files)
│   ├── video/
│   │   ├── vqa.cpp
│   │   └── music.cpp
│   └── ui/
│       └── menu.cpp
└── include/
    └── (corresponding headers)
```
