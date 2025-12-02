# Red Alert macOS Port - Roadmap

**Goal:** Faithful recreation of original Red Alert with full 14-mission
campaigns (Allied + Soviet) playable from original game assets.

**Current State:** Core systems complete. Single missions playable. Campaign
progression requires additional work to load missions from INI files.

---

## Campaign Mode Roadmap (Priority)

The original Red Alert shipped with **14 missions per campaign** (not 22).
The 44-mission count includes Aftermath + Counter-Strike expansions.

### Phase 1: Single Campaign Playable (~20 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| CAM-1 | INI-based mission loading | 8 hrs | Pending |
| CAM-2 | Complete trigger actions | 6 hrs | Pending |
| CAM-3 | Unit/building carryover | 4 hrs | Pending |
| CAM-4 | Briefing video integration | 2 hrs | Pending |

**CAM-1: INI-Based Mission Loading**
- Replace hardcoded `AlliedMissions[]`/`SovietMissions[]` with file loading
- Load SCG01EA.INI, SCU01EA.INI, etc. from GENERAL.MIX
- Parse [Basic], [Map], [Waypoints], [Terrain], [Units], [Structures]
- Apply mission-specific overrides to type data

**CAM-2: Complete Trigger Actions**
Currently stubbed actions that need implementation:
- `ALL_HUNT` - Set all units to hunt mode (partial)
- `REVEAL_ALL` / `REVEAL_SOME` - Map reveal (partial)
- `AUTOCREATE` - AI auto-creates teams (stub)
- `FIRE_SALE` - Sell all buildings (stub)
- `REINFORCE` - Spawn reinforcements at edge (partial)

**CAM-3: Unit/Building Carryover**
- Serialize surviving units/buildings after mission win
- Deserialize at mission start for "carryover" missions
- Implement `Save_Carryover()` / `Load_Carryover()`

**CAM-4: Briefing Video Integration**
- Play intro VQA before mission briefing
- Play outro VQA after mission complete
- Wire up `VQType` enum to actual video files

### Phase 2: Full 14+14 Campaign (~15 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| CAM-5 | Mission variant branching | 4 hrs | Pending |
| CAM-6 | Score screen | 3 hrs | Pending |
| CAM-7 | All win/lose conditions | 4 hrs | Pending |
| CAM-8 | Campaign testing/fixes | 4 hrs | Pending |

**CAM-5: Mission Variant Branching**
- Implement `Choose_Variant()` to select A/B/C/D missions
- Map mission outcomes to branch selection
- Support both linear and branching campaign paths

**CAM-6: Score Screen**
- Implement `ScoreClass::Presentation()`
- Show units lost, buildings destroyed, time elapsed
- Rank/medal system (optional)

**CAM-7: All Win/Lose Conditions**
Additional conditions beyond current 4:
- Threshold-based (N buildings destroyed)
- Specific unit type survival
- Time-limited objectives

### Phase 3: Expansion Content (~25 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| CAM-9 | Aftermath missions | 10 hrs | Pending |
| CAM-10 | Counter-Strike missions | 10 hrs | Pending |
| CAM-11 | New units/buildings | 5 hrs | Pending |

---

## Trigger System Status

### Implemented (Working)

**Events:**
- TIME, ENTERED, ATTACKED, DESTROYED
- CREDITS, DISCOVERED, HOUSE_DISC, GLOBAL

**Actions:**
- CREATE_TEAM, DESTROY_TEAM
- BEGIN_PROD, WIN, LOSE, ALLOWWIN
- START_TIMER / STOP_TIMER / ADD_TIMER / SUB_TIMER
- SET_GLOBAL / CLEAR_GLOBAL
- FORCE_TRIG / DESTR_TRIG
- DESTROY_OBJ, DZ, TEXT

### Needs Implementation

**Events:**
- NOFACTORIES, ALL_BRIDGES_DESTROYED
- EVAC_CIVILIAN, FAKES_DESTROYED
- BUILDING_EXISTS, cross-line/zone triggers

**Actions (Stubbed):**
- ALL_HUNT - needs to iterate all units
- REVEAL_ALL / REVEAL_SOME - needs fog system integration
- AUTOCREATE - needs AI team creation
- FIRE_SALE - needs building sell logic
- REINFORCE - needs edge spawn + transport logic
- PLAY_MOVIE - needs video system integration

---

## Configuration Phase (Complete)

| ID | Item | Status |
|----|------|--------|
| CFG-1 | Mutable Type Data | **DONE** |
| CFG-2 | Unit/Infantry Loading | **DONE** |
| CFG-3 | Building Loading | **DONE** |
| CFG-4 | Weapon/Warhead Loading | **DONE** |
| CFG-5 | Game Constants | **DONE** |
| CFG-6 | Aircraft Direct Flight | **DONE** |

All game data now loaded from RULES.INI.

---

## Core Systems (Complete)

- Metal renderer 60 FPS
- Terrain from theater MIX files (TEMPERAT, SNOW, INTERIOR)
- Sprites with team color remapping
- A* pathfinding (ground), direct flight (aircraft)
- Combat with armor/warhead damage matrix
- Infantry scatter from explosions
- Build prerequisites / tech tree
- Team formations
- AI threat assessment and hunt mode
- Menu system
- Sidebar UI with production
- VQA video playback with audio
- Background music streaming (AUD files)
- Save/load system
- Fog of war

---

## Backlog (Post-Campaign)

### Features

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| FEAT-1 | Skirmish mode | 8 hrs | Random map, AI opponent |
| FEAT-2 | Multiplayer stub | 4 hrs | LAN lobby (no netcode) |

### Polish

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| VIS-2 | Selection box | 1 hr | Clearer unit selection |
| UI-3 | Radar zoom | 2 hrs | Zoom levels |
| UI-4 | Sidebar scroll | 1 hr | Long build lists |
| PR-3 | Power production | 2 hrs | Low power penalty |

---

## Absolute Blockers for Campaign

1. **Mission loading** - Must load from INI files, not hardcoded
2. **Trigger stubs** - Several critical actions do nothing
3. **Carryover** - Can't progress without unit persistence

---

## Effort Summary

| Phase | Hours | Description |
|-------|-------|-------------|
| Phase 1 | 20 hrs | Single campaign playable |
| Phase 2 | 15 hrs | Full 14+14 campaigns |
| Phase 3 | 25 hrs | Expansion content |
| Backlog | 18 hrs | Skirmish, polish |
| **Total** | **~78 hrs** | Complete faithful port |

---

See [CHANGELOG.md](CHANGELOG.md) for completion history.
