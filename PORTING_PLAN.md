# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Real mission loading partially implemented but simplified.

---

## Prioritized Work Queue

*Ordered by leverage - each tier unlocks the next.*

### TIER 0: Remove The Blocker ✓ COMPLETE

| ID | Item | Effort | Unlocks |
|----|------|--------|---------|
| **TD-9** | ~~Wire Mission_Start to load real terrain~~ | ~~2 hrs~~ | **DONE** |

### TIER 1: Mission Logic Loop ✓ COMPLETE

| Order | ID | Item | Effort | Status |
|-------|-----|------|--------|--------|
| 1.1 | TD-5a | ~~Parse [Triggers] section~~ | ~~4 hrs~~ | **DONE** - 47 triggers parse from SCU01EA |
| 1.2 | TD-5b | ~~Parse [Waypoints] section~~ | ~~2 hrs~~ | **DONE** - 29 waypoints parse from SCU01EA |
| 1.3 | B3-core | ~~4 events: Time, Destroyed, NoBuildings, NoUnits~~ | ~~4 hrs~~ | **DONE** |
| 1.4 | B3-core | ~~4 actions: Win, Lose, Reinforce, Text~~ | ~~4 hrs~~ | **DONE** |

**Status:** Mission_ProcessTriggers() evaluates parsed triggers each frame.
Win/lose conditions fire correctly based on house destruction events.

### TIER 2: Content Recognition ✓ COMPLETE

| Order | ID | Item | Effort | Status |
|-------|-----|------|--------|--------|
| 2.1 | TD-1a | ~~10 unit types~~ | ~~3 hrs~~ | **DONE** - Added 23 unit types total |
| 2.2 | TD-2a | ~~10 building types~~ | ~~3 hrs~~ | **DONE** - Added 19 building types total |

**Status:** All standard Red Alert unit and building types are recognized.
Mission INI files now parse all entity types correctly.

### TIER 3: Visual Correctness (Days 7-8)

| Order | ID | Item | Effort | Status |
|-------|-----|------|--------|--------|
| 3.1 | TD-11 | ~~Load TEMPERAT.MIX, palette switching~~ | ~~4 hrs~~ | **DONE** |
| 3.2 | TD-8 | ~~Map overlay types, render ore/gems~~ | ~~4 hrs~~ | **DONE** |
| 3.3 | TD-7 | ~~OpenRA template ID mappings~~ | ~~6 hrs~~ | **DONE** |

**Status:** TIER 3 COMPLETE - Visual correctness achieved.
- Theater switching: TEMPERATE/SNOW/INTERIOR/DESERT
- Overlay mapping: Ore (GOLD1-4), Gems (GEMS1-4), Walls
- Template IDs: Exact OpenRA YAML mappings for snow/temperate

### TIER 4: Audio (Parallel Track)

| Order | ID | Item | Effort | Why Now |
|-------|-----|------|--------|---------|
| 4.1 | BUG-01 | Debug music ADPCM distortion | 4-8 hrs | Investigation |
| 4.2 | BUG-02 | Debug VQA audio static | 4-8 hrs | Likely related |

**Note:** High annoyance but low gameplay leverage. Unknown scope. Run parallel with Tiers 2-3.

### TIER 5: Mission Fidelity (Week 2)

| Order | ID | Item | Effort | Status |
|-------|-----|------|--------|--------|
| 5.1 | TD-5c | ~~Parse [TeamTypes], [Base], [Reinforcements]~~ | ~~4 hrs~~ | **DONE** |
| 5.2 | TD-4 | ~~Use health, facing, mission fields~~ | ~~2 hrs~~ | **DONE** |
| 5.3 | B3-full | ~~Remaining trigger events/actions~~ | ~~8 hrs~~ | **DONE** (basic) |
| 5.4 | TD-5d | ~~Parse [Terrain], [Smudge], [Ships]~~ | ~~4 hrs~~ | **DONE** |

**Status:** TIER 5 COMPLETE
- TeamTypes parsing complete (15 teams parse from SCU01EA)
- Base section parsing complete ([Reinforcements] not in files - via triggers)
- Entity data parsing: health, facing, mission, subCell now stored
- Trigger events: ENTERED, ATTACKED, DESTROYED, ALL_DESTR, TIME, ANY, DISCOVERED
- Trigger actions: Win/Lose, CreateTeam, Reinforce, Text, RevealAll, RevealArea, DestroyTrigger, etc.
- Map_RevealAll() and Map_RevealArea() implemented
- [TERRAIN] section parsed (trees T01-T17, tree clumps TC01-TC05)
- [SMUDGE] section parsed (craters CR1-CR6, scorch marks SC1-SC6)
- [SHIPS] section parsed (naval units: SS, DD, CA, LST, PT, GNBT)

### TIER 6: UI Polish ✓ COMPLETE

| ID | Item | Effort | Status |
|----|------|--------|--------|
| BUG-05 | ~~Fog re-blacks revealed terrain~~ | ~~2 hrs~~ | **DONE** - Code correct, alpha=128 for fog |
| BUG-03 | ~~P for pause doesn't work~~ | ~~1 hr~~ | **DONE** - Input_Update moved to end of frame |
| BUG-04 | ~~Briefing garbled after video~~ | ~~2 hrs~~ | **DONE** - Palette restoration verified |
| BUG-06 | ~~Sound volume slider untestable~~ | ~~1 hr~~ | **DONE** - Added test sound on slider change |

### TIER 7: Tech Debt (Before Release) ✓ COMPLETE

| ID | Item | Effort | Status |
|----|------|--------|--------|
| TD-6 | ~~MapPack chunk mask (0xDFFFFFFF)~~ | ~~30 min~~ | **DONE** |
| TD-12 | ~~Hardcoded paths~~ | ~~2 hrs~~ | **DONE** - `make dist-full` bundles assets |
| TD-3 | ~~8-house system foundation~~ | ~~2 hrs~~ | **DONE** - HouseType enum, Team mapping |
| TD-10 | ~~Complex win/lose conditions~~ | ~~4 hrs~~ | **DONE** - Time-based, capture, protect |

**Status:**
- MapPack chunk mask fixed (0xDFFFFFFF masks compression flag)
- HouseType enum: Spain(0), Greece(1), USSR(2), England(3), Ukraine(4), Germany(5), France(6), Turkey(7)
- House_ToTeam(), House_IsAlly(), House_GetName() functions added
- Mission_CheckVictory now supports: destroy_all, destroy_buildings, survive_time, capture
- Mission_ProcessTriggers integrated into game loop with frame counter

---

## Schedule Overview

```
Week 1:
├── Day 1: TD-9 (real terrain) ───────────────► CAN TEST REAL MISSIONS
├── Day 2: TD-5a (parse triggers) + TD-5b (waypoints)
├── Day 3: B3-core events (Time, Destroyed, NoBuildings, NoUnits)
├── Day 4: B3-core actions (Win, Lose, Reinforce, Text) ► MISSION 1 COMPLETABLE
├── Day 5: TD-1a + TD-2a (essential unit/building types)
└── Day 6: TD-11 (theater support) ───────────► ALLIED CAMPAIGN VISIBLE

Week 2:
├── Day 7: TD-8 (overlays) + TD-7 (template mapping)
├── Day 8: BUG-01/02 (audio investigation) ───► AUDIO FIXED
├── Day 9: TD-5c (TeamTypes, Reinforcements)
├── Day 10: B3-full (remaining triggers) ─────► ALL MISSIONS WORK
└── Remaining: Polish, remaining types, edge cases
```

---

## Technical Debt Reference

### TD-1: Unit Types ✓ RESOLVED

**Location:** `units.h:21-62`, `mission.cpp:75-121`

**Implemented (35 types):**

| Category | Types |
|----------|-------|
| Infantry | E1, E2, E3, E6, DOG, SPY, MEDI, THF, SHOK |
| Vehicles | HARV, 1TNK, 2TNK, 3TNK, 4TNK, APC, ARTY, JEEP, MCV, V2RL, MNLY, TRUK, CTNK |
| Naval | GNBT, DD, SS, CA, LST, PT |
| Aircraft | HIND, HELI, TRAN, YAK, MIG |

### TD-2: Building Types ✓ RESOLVED

**Location:** `units.h:64-99`, `mission.cpp:123-164`

**Implemented (27 types):**

| Category | Types |
|----------|-------|
| Core | FACT, POWR, APWR, PROC, SILO |
| Production | TENT/BARR, WEAP, AFLD, HPAD, SYRD, SPEN |
| Tech | DOME, ATEK/STEK, KENN |
| Defense | GUN, SAM, TSLA, AGUN, PBOX, HBOX, FTUR, GAP |
| Special | FIX, IRON, PDOX, MSLO |

### TD-3: Team/House Oversimplified

**Location:** `mission.cpp:98-123`

Current: 3 teams (PLAYER, ENEMY, NEUTRAL). Need 8-house support.

### TD-4: Entity Data ✓ RESOLVED

**Location:** `mission.cpp:430-525`, `mission.h:35-69`

| Field | Purpose | Status |
|-------|---------|--------|
| health | Starting HP | ✓ Parsed |
| facing | Direction | ✓ Parsed |
| mission | Guard/Hunt | ✓ Parsed |
| trigger | Link | Not yet |
| subCell | Infantry pos | ✓ Parsed |

Added MissionType enum and extended MissionUnit/MissionBuilding structs to store all fields.

### TD-5: Unparsed INI Sections

**Location:** `mission.cpp`

| Section | Purpose | Priority | Status |
|---------|---------|----------|--------|
| [Trigs] | Event scripting | TIER 1 | ✓ DONE |
| [Waypoints] | Movement points | TIER 1 | ✓ DONE |
| [TeamTypes] | AI teams | TIER 5 | ✓ DONE |
| [Base] | AI build order | TIER 5 | ✓ DONE |
| [Reinforcements] | Unit arrivals | TIER 5 | N/A (via triggers) |
| [Terrain] | Trees/cliffs | TIER 5 | |
| [Smudge] | Craters | TIER 5 | |
| [Ships] | Naval units | TIER 5 | |

### TD-6: MapPack Chunk Mask

**Location:** `mission.cpp:201`

```cpp
chunkLen &= 0x0000FFFF;  // Current
chunkLen &= 0xDFFFFFFF;  // Correct
```

### TD-7: Template ID Mapping ✓ RESOLVED

**Location:** `terrain.cpp:222-299`

**Solution:** Updated GetTemplateFilename() with exact OpenRA YAML mappings:
- 1-2: Water (w1, w2)
- 3-58: Shore/Beach (sh01-sh56)
- 59-96: Water cliffs (wc01-wc38)
- 112-124, 229-230: Rivers (rv01-rv15)
- 135-172: Roads/slopes (s01-s38)
- 173-215, 227-228: Debris (d01-d45)
- 231-234: Road cliffs (rc01-rc04)
- 235-244: Bridges (br1a-br2c)
- 255, 65535: Clear (clear1)

### TD-8: Overlay Types ✓ RESOLVED

**Location:** `map.cpp:27-55, 376-406`

**Solution:** Added OverlayTypeRA enum matching original DEFINES.H values.
Map_LoadFromMission() now processes overlay data:
- GOLD1-4 (5-8) → TERRAIN_ORE with scaled oreAmount
- GEMS1-4 (9-12) → TERRAIN_GEM with scaled oreAmount
- Walls (0-4, 23) → TERRAIN_ROCK (impassable)
- V12-V18 vegetation and crates left as passable terrain

### TD-9: Mission_Start Uses Demo Map

**Location:** `mission.cpp:477`

```cpp
Map_GenerateDemo();  // Should use mission->terrainType
```

### TD-10: Win/Lose Conditions Limited

**Location:** `mission.h:89-91`

Only destroy_all, destroy_buildings. Missing time-based, protect, capture.

### TD-11: Theater Support ✓ RESOLVED

**Location:** `mission.cpp:629-646`, `assetloader.cpp:558-579`

**Solution:** Mission_Start() now calls Assets_SetTheater() and Terrain_SetTheater()
based on mission->theater value from INI. TEMPERATE and SNOW fully work.
INTERIOR/DESERT still fall back to SNOW (assets not extracted).

### TD-12: Hardcoded Paths ✓ RESOLVED

**Location:** `asset_paths.cpp`

**Solution:** Added bundle asset discovery. The `make dist-full` target creates
a self-contained DMG (~729MB) with all game assets bundled inside the .app.
Assets are searched in this order:
1. Bundle: `Contents/Resources/assets` (for dist-full builds)
2. User: `~/Library/Application Support/RedAlert/assets`
3. Portable: `./assets`, `../assets`
4. Mounted ISOs: `/Volumes/CD1/INSTALL`, `/Volumes/CD2/INSTALL`

---

## Known Bugs

| ID | Issue | Severity | File(s) | Status |
|----|-------|----------|---------|--------|
| BUG-01 | Music heavily distorted | High | audio.mm, music.cpp | OPEN |
| BUG-02 | Video audio has static | High | vqa.cpp, audio.mm | OPEN |
| BUG-03 | ~~P for Pause doesn't work~~ | ~~Medium~~ | ~~main.mm~~ | **FIXED** |
| BUG-04 | ~~Briefing garbled after video~~ | ~~Medium~~ | ~~menu.cpp~~ | **FIXED** |
| BUG-05 | ~~Fog re-blacks revealed terrain~~ | ~~Medium~~ | ~~map.cpp~~ | **N/A** |
| BUG-06 | ~~Sound volume slider untestable~~ | ~~Low~~ | ~~menu.cpp~~ | **FIXED** |

---

## TIER 8: Software Engineering (Code Quality)

*Systematic cleanup for maintainability, safety, and standards conformance.*

### SE-1: Safety Issues (HIGH PRIORITY)

| ID | Issue | File | Lines | Status |
|----|-------|------|-------|--------|
| SE-1.1 | ~~Replace strcpy with strncpy~~ | ~~mission.cpp~~ | ~~40-41, 1176-1177~~ | **DONE** |
| SE-1.1b | ~~Replace strcpy~~ | ~~file.cpp~~ | ~~395~~ | **DONE** |
| SE-1.2 | ~~Add range validation to parsers~~ | ~~mission.cpp~~ | ~~(already present)~~ | **DONE** |

### SE-2: Long Functions (>100 lines)

| ID | Function | Lines | File | Strategy | Status |
|----|----------|-------|------|----------|--------|
| SE-2.1 | ~~Mission_LoadFromINIClass~~ | ~~503~~ | ~~mission.cpp~~ | ~~Split by section~~ | **DONE** |
| SE-2.2 | ~~GameUpdate~~ | ~~302~~ | ~~main.mm~~ | ~~Extract helpers~~ | **DONE** |
| SE-2.3 | ~~GameRender~~ | ~~291~~ | ~~main.mm~~ | ~~Extract helpers~~ | **DONE** |
| SE-2.4 | TActionClass::Execute | 171 | trigger.cpp | Switch dispatch | **ACCEPTABLE** |
| SE-2.5 | Shp_Load | 154 | shpfile.cpp | Extract decompression | PENDING |
| SE-2.6 | ~~GameUI_RenderRadar~~ | ~~150~~ | ~~game_ui.cpp~~ | ~~Extract components~~ | **DONE** |
| SE-2.7 | ~~Mission_Start~~ | ~~138~~ | ~~mission.cpp~~ | ~~Extract init phases~~ | **DONE** |
| SE-2.8 | FindPath | 129 | units.cpp | A* algorithm | **ACCEPTABLE** |
| SE-2.9 | ModMul | 127 | mixkey.cpp | Crypto algorithm | **ACCEPTABLE** |
| SE-2.10 | RenderDemoMode | 125 | main.mm | Demo code - LOW PRIORITY | PENDING |

### SE-3: 80-Column Violations

| ID | File | Count | Fix Strategy | Status |
|----|------|-------|--------------|--------|
| SE-3.1 | rules.cpp | 105 | Break long INI getter chains | PENDING |
| SE-3.2 | menu.cpp | 63 | Wrap strings, break conditionals | PENDING |
| SE-3.3 | units.cpp | 53 | Break long conditionals | PENDING |
| SE-3.4 | building_types.cpp | 50 | Data tables - ACCEPTABLE | N/A |
| SE-3.5 | game_ui.cpp | 46 | Break long function calls | PENDING |
| SE-3.6 | main.mm | 42 | Wrap strings, comments | PENDING |
| SE-3.7 | mission.cpp | 40 | Break sscanf format strings | PENDING |

### SE-4: Code Style Issues

| ID | Issue | File(s) | Status |
|----|-------|---------|--------|
| SE-4.1 | Global state in ParseTrigsSection | mission.cpp:437-492 | PENDING |
| SE-4.2 | Missing const qualifiers | mission.cpp, terrain.cpp | PENDING |
| SE-4.3 | Large switch → table-driven | trigger.cpp:146-316 | PENDING |
| SE-4.4 | Implicit int16_t casts | mission.cpp parsers | PENDING |

### SE-5: Simplified Implementations (60+ markers)

Code marked "simplified" that may need full implementation:

| Category | Count | Examples |
|----------|-------|----------|
| AI behavior | ~15 | Team tactics, threat assessment |
| Trigger actions | ~12 | CreateTeam, Reinforce timing |
| Combat mechanics | ~10 | Armor types, weapon ranges |
| Production | ~8 | Build prerequisites, queue |
| Pathfinding | ~5 | Naval, aircraft movement |
| UI | ~10 | Radar zoom, sidebar scroll |

### SE-6: TODOs and FIXMEs

| Type | Count | High-Priority Examples |
|------|-------|------------------------|
| TODO | 39 | Mission entity linking, AI teams |
| FIXME | 1 | None critical |

---

### Work Order (SE Phase)

**Round 1: Safety (SE-1)** ✅ COMPLETE
1. ~~SE-1.1: strcpy → strncpy~~ (mission.cpp, file.cpp)
2. ~~SE-1.2: Parser range validation~~ (already present)

**Round 2: Long Functions (SE-2)** ✅ COMPLETE
3. ~~SE-2.4: TActionClass::Execute~~ (acceptable switch dispatch)
4. ~~SE-2.6: GameUI_RenderRadar decomposition~~
5. ~~SE-2.7: Mission_Start decomposition~~

**Round 3: Style Fixes (SE-3, SE-4)**
6. SE-3.1: rules.cpp 80-col fixes
7. SE-4.1: ParseTrigsSection return data vs globals
8. SE-4.3: Trigger action table-driven dispatch

**Round 4: Lower Priority**
9. SE-2.5: Shp_Load (binary parser, works)
10. SE-2.10: RenderDemoMode (demo code)
11. SE-3.2-3.7: Remaining 80-col fixes

---

## Completed Work

See [COMPLETED.md](COMPLETED.md) for full history.

| Phase | Milestones | Status |
|-------|------------|--------|
| Infrastructure | M0-M14 | Complete |
| Core Engine | M15-M19 | Complete |
| Game Systems | M20-M22 | Complete |
| UI Systems | M23-M25 | Complete |
| Polish Systems | M26-M29 | Complete |
| Integration | M30-M32 | Complete |
| Playable Demo | M33-M37 | Complete |
| Combat Polish | M38-M40 | Complete |
| Campaign Support | M41-M44 | Complete |
| Media | M45-M46 | Complete |

**Tests passing:** ~400

### What Works (Demo Mode)

- Metal renderer 60 FPS
- Terrain from snow.mix
- Sprites from conquer.mix
- A* pathfinding
- Combat system
- Menu system
- Sidebar UI
- Production/placement
- Tech tree
- Economy
- AI opponent
- Fog of war
- Attack commands
- Campaign flow
- VQA playback
- Background music
