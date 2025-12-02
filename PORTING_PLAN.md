# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Real mission loading partially implemented but simplified.

---

## Prioritized Work Queue

*Ordered by leverage - each tier unlocks the next.*

### TIER 0: Remove The Blocker (Day 1)

| ID | Item | Effort | Unlocks |
|----|------|--------|---------|
| **TD-9** | Wire Mission_Start to load real terrain | 2 hrs | Everything else |

**Why first:** Until `Mission_Start()` uses `mission->terrainType/terrainIcon` instead of `Map_GenerateDemo()`, we can't test anything. Every hour on other work is speculative.

### TIER 1: Mission Logic Loop (Days 2-4)

| Order | ID | Item | Effort | Why Now |
|-------|-----|------|--------|---------|
| 1.1 | TD-5a | Parse [Triggers] section | 4 hrs | Win/lose detection |
| 1.2 | TD-5b | Parse [Waypoints] section | 2 hrs | Spawn points, paths |
| 1.3 | B3-core | 4 events: Time, Destroyed, NoBuildings, NoUnits | 4 hrs | Basic mission flow |
| 1.4 | B3-core | 4 actions: Win, Lose, Reinforce, Text | 4 hrs | Complete the loop |

**After Tier 1:** Allied Mission 1 completable (destroy all enemies = win).

### TIER 2: Content Recognition (Days 5-6)

| Order | ID | Item | Effort | Why Now |
|-------|-----|------|--------|---------|
| 2.1 | TD-1a | 10 unit types: JEEP, DOG, MCV, 4TNK, V2RL, SPY, MEDI, TRAN, HIND, SS | 3 hrs | Missions spawn units |
| 2.2 | TD-2a | 10 building types: TSLA, KENN, SILO, AGUN, HPAD, AFLD, SYRD, SPEN, FIX, APWR | 3 hrs | Missions spawn buildings |

**After Tier 2:** Most campaign missions parse correctly.

### TIER 3: Visual Correctness (Days 7-8)

| Order | ID | Item | Effort | Why Now |
|-------|-----|------|--------|---------|
| 3.1 | TD-11 | Load TEMPERAT.MIX, palette switching | 4 hrs | Allied missions green |
| 3.2 | TD-8 | Map overlay types, render ore/gems | 4 hrs | Economy works |
| 3.3 | TD-7 | OpenRA template ID mappings | 6 hrs | Tiles render correctly |

**After Tier 3:** Missions look right and economy functions.

### TIER 4: Audio (Parallel Track)

| Order | ID | Item | Effort | Why Now |
|-------|-----|------|--------|---------|
| 4.1 | BUG-01 | Debug music ADPCM distortion | 4-8 hrs | Investigation |
| 4.2 | BUG-02 | Debug VQA audio static | 4-8 hrs | Likely related |

**Note:** High annoyance but low gameplay leverage. Unknown scope. Run parallel with Tiers 2-3.

### TIER 5: Mission Fidelity (Week 2)

| Order | ID | Item | Effort | Why Now |
|-------|-----|------|--------|---------|
| 5.1 | TD-5c | Parse [TeamTypes], [Base], [Reinforcements] | 4 hrs | AI teams |
| 5.2 | TD-4 | Use health, facing, mission fields | 2 hrs | Authentic spawn |
| 5.3 | B3-full | Remaining trigger events/actions | 8 hrs | Complex missions |
| 5.4 | TD-5d | Parse [Terrain], [Smudge], [Ships] | 4 hrs | Scenery, naval |

### TIER 6: UI Polish

| ID | Item | Effort | Notes |
|----|------|--------|-------|
| BUG-05 | Fog re-blacks revealed terrain | 2 hrs | Gameplay impact |
| BUG-03 | P for pause doesn't work | 1 hr | Debug convenience |
| BUG-04 | Briefing garbled after video | 2 hrs | Campaign flow |
| BUG-06 | Sound volume slider untestable | 1 hr | Low priority |

### TIER 7: Tech Debt (Before Release)

| ID | Item | Effort | Notes |
|----|------|--------|-------|
| TD-6 | MapPack chunk mask (0xDFFFFFFF) | 30 min | Quick fix |
| TD-12 | Hardcoded paths | 2 hrs | Distribution |
| TD-3 | Full 8-house system | 8 hrs | Only if needed |
| TD-10 | Complex win/lose conditions | 4 hrs | After triggers |

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

### TD-1: Unit Types Missing (~30 types)

**Location:** `mission.cpp:57-79`

**Implemented (12):** E1, E2, E3, E6, HARV, 1TNK, 2TNK, 3TNK, APC, ARTY, GNBT, DD

**Missing:**

| Category | Types |
|----------|-------|
| Infantry | E4, E5/SPY, THF, MEDI, DOG, SHOK, E7, C1-C10 |
| Vehicles | JEEP, MCV, 4TNK, V2RL, MNLY, TRUK, CTNK |
| Naval | SS, CA, LST, PT |
| Aircraft | HIND, HELI, TRAN, YAK, MIG |

### TD-2: Building Types Missing (~20 types)

**Location:** `mission.cpp:82-95`

**Implemented (8):** FACT, POWR, PROC, BARR, WEAP, DOME, GUN, SAM

**Missing:** TSLA, ATEK/STEK, KENN, SILO, AFLD, HPAD, PBOX, GAP, IRON, PDOX, MSLO, AGUN, SYRD, SPEN, FIX, APWR

### TD-3: Team/House Oversimplified

**Location:** `mission.cpp:98-123`

Current: 3 teams (PLAYER, ENEMY, NEUTRAL). Need 8-house support.

### TD-4: Entity Data Discarded

**Location:** `mission.cpp:316-395`

| Field | Purpose | Status |
|-------|---------|--------|
| health | Starting HP | Ignored |
| facing | Direction | Ignored |
| mission | Guard/Hunt | Ignored |
| trigger | Link | Ignored |
| subCell | Infantry pos | Ignored |

### TD-5: Unparsed INI Sections

**Location:** `mission.cpp`

| Section | Purpose | Priority |
|---------|---------|----------|
| [Triggers] | Event scripting | TIER 1 |
| [Waypoints] | Movement points | TIER 1 |
| [TeamTypes] | AI teams | TIER 5 |
| [Base] | AI build order | TIER 5 |
| [Reinforcements] | Unit arrivals | TIER 5 |
| [Terrain] | Trees/cliffs | TIER 5 |
| [Smudge] | Craters | TIER 5 |
| [Ships] | Naval units | TIER 5 |

### TD-6: MapPack Chunk Mask

**Location:** `mission.cpp:201`

```cpp
chunkLen &= 0x0000FFFF;  // Current
chunkLen &= 0xDFFFFFFF;  // Correct
```

### TD-7: Template ID Mapping

**Location:** `terrain.cpp:224-278`

Pattern-based guessing. Need exact OpenRA YAML mappings.

### TD-8: Overlay Types Not Mapped

**Location:** `mission.cpp:425-443`

Raw bytes stored. Missing: ORE, GEMS, walls, crates.

### TD-9: Mission_Start Uses Demo Map

**Location:** `mission.cpp:477`

```cpp
Map_GenerateDemo();  // Should use mission->terrainType
```

### TD-10: Win/Lose Conditions Limited

**Location:** `mission.h:89-91`

Only destroy_all, destroy_buildings. Missing time-based, protect, capture.

### TD-11: Theater Support Incomplete

**Location:** `assetloader.cpp:548-556`

INTERIOR/DESERT return nullptr (fall back to SNOW).

### TD-12: Hardcoded Paths

**Location:** `assetloader.cpp:48-117`

Absolute paths need bundle/library discovery.

---

## Known Bugs

| ID | Issue | Severity | File(s) | Tier |
|----|-------|----------|---------|------|
| BUG-01 | Music heavily distorted | High | audio.mm, music.cpp | 4 |
| BUG-02 | Video audio has static | High | vqa.cpp, audio.mm | 4 |
| BUG-03 | P for Pause doesn't work | Medium | main.mm, units.cpp | 6 |
| BUG-04 | Briefing garbled after video | Medium | menu.cpp, renderer.mm | 6 |
| BUG-05 | Fog re-blacks revealed terrain | Medium | map.cpp | 6 |
| BUG-06 | Sound volume slider untestable | Low | menu.cpp | 6 |

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
