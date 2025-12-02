# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Core systems complete. Most trigger
actions/events implemented. Audio and video fully functional.

---

## Recently Completed

| ID | Item | Date | Notes |
|----|------|------|-------|
| **TR-6** | DESTROY_TEAM | Dec 2 | Team unit tracking + removal |
| **TR-7** | FIRE_SALE | Dec 2 | AI sells all buildings |
| **TR-8** | DZ (drop zone) | Dec 2 | Flare state + rendering API |
| **TR-9** | PLAY_MOVIE | Dec 2 | Mid-mission VQA playback |
| **UI-2** | TEXT action | Dec 2 | Mission text overlay |
| **EV-4** | CREDITS event | Dec 2 | Threshold comparison |
| **EV-5** | DISCOVERED event | Dec 2 | Fog reveal tracking |
| **EV-6** | HOUSE_DISC event | Dec 2 | Enemy sighting |
| **AI-1** | Hunt mode improvements | Dec 2 | Map-wide targeting |
| **AI-3** | Threat assessment | Dec 2 | Target prioritization |
| **CB-1** | Armor types | Dec 2 | Full warhead matrix |
| **CB-2** | Weapon damage mods | Dec 2 | Per-armor multipliers |
| **CB-3** | Scatter on explosion | Dec 2 | Infantry scatter |
| **AI-2** | Team formations | Dec 2 | Formation movement |
| **PR-1** | Build prerequisites | Dec 2 | Tech tree validation |
| **TR-5** | AUTOCREATE | Dec 2 | AI team auto-creation |
| **TR-10** | START/STOP_TIMER | Dec 2 | Mission timer control |
| **TR-11** | DESTROY_OBJ | Dec 2 | Trigger-linked destruction |

---

## Active Work Queue

*Sorted by priority. Work top-to-bottom.*

### TIER 1: BUGS (User-visible issues)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| **BUG-03** | P for Pause doesn't work | 1 hr | Verify pause toggle |
| **BUG-04** | Briefing garbled after video | 2 hrs | Text corruption |
| **BUG-05** | Fog re-blacks revealed terrain | 2 hrs | Fog persistence |

### TIER 2: POLISH (UI/UX improvements)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| **VIS-2** | Selection box visibility | 1 hr | Clearer unit selection |
| **BUG-09** | Target cursor on hover | 2 hrs | Attack cursor on enemies |
| **UI-3** | Radar zoom | 2 hrs | Zoom levels for minimap |
| **UI-4** | Sidebar scroll | 1 hr | Scroll long build lists |
| **PR-3** | Power affects production | 2 hrs | Low power slows building |

---

## Code Quality (No gameplay impact)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| SE-3.1 | rules.cpp 80-col | 1 hr | Line length fixes |
| SE-3.2 | menu.cpp 80-col | 1 hr | Line length fixes |
| SE-3.3 | units.cpp 80-col | 1 hr | Line length fixes |

---

## What's Complete

### Trigger Actions (Implemented)

- CREATE_TEAM - Creates team from template (with unit tracking)
- REINFORCE - Spawns reinforcement team (with unit tracking)
- DESTROY_TEAM - Removes team and all its units
- ALL_HUNT - All units attack-move to enemies
- BEGIN_PROD - AI starts production
- FIRE_SALE - AI sells all buildings for credits
- REVEAL_ALL / REVEAL_SOME / REVEAL_ZONE - Map reveal
- AUTOCREATE - Enable AI team auto-creation
- START_TIMER / STOP_TIMER / ADD_TIMER / SUB_TIMER - Timer control
- SET_GLOBAL / CLEAR_GLOBAL - Global flag management
- FORCE_TRIG / DESTR_TRIG - Trigger manipulation
- DESTROY_OBJ - Destroy trigger-linked objects
- WIN / LOSE / ALLOWWIN - Victory/defeat handling
- DZ - Drop zone flare rendering
- PLAY_MOVIE - Mid-mission VQA playback
- TEXT - Mission text overlay display

### Trigger Events (Implemented)

- TIME - Frame count threshold
- ENTERED - Unit enters zone
- ATTACKED / DESTROYED - Object damage/death
- CREDITS - Credit threshold
- DISCOVERED - Fog reveal
- HOUSE_DISC - Enemy sighting
- GLOBAL - Global flag check

### Game Systems (Implemented)

- Metal renderer 60 FPS
- Terrain from theater MIX files
- Sprites with team color remapping
- A* pathfinding (ground units)
- Combat with armor/warhead damage matrix
- Infantry scatter from explosions
- Build prerequisites / tech tree
- Team formations
- AI threat assessment and hunt mode
- Menu system
- Sidebar UI with production
- VQA video playback
- Background music streaming
- Save/load system
- Campaign flow
- Fog of war

---

## Recommended Next Steps

1. **BUG-03: P for Pause** (1 hr) - Verify or fix
2. **BUG-04: Briefing text** (2 hrs) - Text corruption
3. **BUG-05: Fog persistence** (2 hrs) - Prevent re-blackening
4. **VIS-2: Selection box** (1 hr) - Clearer unit selection
5. **UI-3: Radar zoom** (2 hrs) - Minimap zoom levels
