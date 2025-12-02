# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Core systems complete. Most trigger
actions/events implemented. Audio and video fully functional.

---

## Active Bugs (Fix First)

*All bugs fixed - see CHANGELOG.md for history*

---

## Backlog

### Gameplay Bugs (BUG-xx)

*None pending - all moved to Active or Completed*

### Features (FEAT-xx)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| FEAT-1 | Skirmish mode | 8 hrs | Random map, AI opponent selection |
| FEAT-2 | Multiplayer stub | 4 hrs | LAN discovery, lobby UI (no netcode) |

### Polish (UI-xx, VIS-xx)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| VIS-2 | Selection box visibility | 1 hr | Clearer unit selection |
| UI-3 | Radar zoom | 2 hrs | Zoom levels for minimap |
| UI-4 | Sidebar scroll | 1 hr | Scroll long build lists |
| PR-3 | Power affects production | 2 hrs | Low power slows building |

### Code Quality (SE-xx)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| SE-3.1 | rules.cpp 80-col | 1 hr | Line length fixes |
| SE-3.2 | menu.cpp 80-col | 1 hr | Line length fixes |
| SE-3.3 | units.cpp 80-col | 1 hr | Line length fixes |

---

## File-Based Configuration (CFG-xx)

**Goal:** Move hardcoded values to RULES.INI for moddability and
accuracy to original game.

### Current State

**Complete (CFG-1 through CFG-5):**
- Mutable runtime type data arrays (no longer const)
- Infantry stats loaded from [E1], [E2], etc. sections
- Unit/vehicle stats loaded from [1TNK], [2TNK], etc. sections
- Building stats loaded from [FACT], [POWR], etc. sections
- Weapon stats loaded from [Colt45], [Dragon], etc. sections
- Warhead damage matrices loaded from [SA], [HE], etc. sections
- Game constants (GoldValue, GemValue) loaded from [General]

**Remaining:**
- `ProcessProjectiles()` - projectile data not loaded from INI

### Hardcoded Values Inventory

**Game Limits (units.h, mission.h):**

| Constant | Value | Original | INI Key |
|----------|-------|----------|---------|
| MAX_UNITS | 256 | 500 | [General] MaxUnit |
| MAX_BUILDINGS | 128 | 500 | [General] MaxBuilding |
| MAX_PASSENGERS | 5 | per-type | [APC] Passengers |
| HARVESTER_MAX_CARGO | 1000 | 28 | [Harvester] Storage |
| HARVESTER_LOAD_RATE | 50 | - | [General] OreTruckRate |
| ORE_VALUE | 7 | 25/50 | [General] GoldValue |
| MAX_PATH_WAYPOINTS | 32 | 24 | hardcoded |

**Unit Stats (units.cpp UnitTypeDef array):**
- Health (Strength in INI)
- Speed (Speed in INI)
- AttackRange (Range in weapon section)
- AttackDamage (Damage in weapon section)
- AttackRate (ROF in weapon section)
- SightRange (Sight in INI)
- isInfantry/isNaval/isAircraft (inferred from SpeedType)

**Timing (gameloop.h):**

| Constant | Value | Notes |
|----------|-------|-------|
| DEFAULT_GAME_FPS | 15 | Original game tick rate |
| TICKS_PER_SECOND | 60 | Frame subdivision |

### Original Game Architecture (Reference)

**Aircraft Movement:**
- Aircraft do NOT use A* pathfinding
- Use FlyClass for direct point-to-point flight
- SpeedType::WINGED makes all terrain passable
- Only check map bounds, no terrain collision

**Type Data Loading:**
```cpp
// Original: TechnoTypeClass::Read_INI()
Strength = ini.Get_Int(section, "Strength", Strength);
Speed = ini.Get_Int(section, "Speed", Speed);
Sight = ini.Get_Int(section, "Sight", Sight);
```
Hardcoded defaults in UDATA.CPP, IDATA.CPP, BDATA.CPP as fallbacks.

### Migration Phases

| ID | Item | Effort | Status |
|----|------|--------|--------|
| CFG-1 | Mutable Type Data | 4 hrs | **DONE** |
| CFG-2 | Unit/Infantry Loading | 6 hrs | **DONE** |
| CFG-3 | Building Loading | 4 hrs | **DONE** |
| CFG-4 | Weapon/Warhead Loading | 6 hrs | **DONE** |
| CFG-5 | Game Constants | 2 hrs | **DONE** |
| CFG-6 | Aircraft Direct Flight | 4 hrs | Pending |

### Remaining Work

1. CFG-6 (aircraft) - bypass A* for aircraft, use direct flight

---

## What's Complete

### Trigger Actions (Implemented)

- CREATE_TEAM, REINFORCE, DESTROY_TEAM
- ALL_HUNT, BEGIN_PROD, FIRE_SALE
- REVEAL_ALL / REVEAL_SOME / REVEAL_ZONE
- AUTOCREATE
- START_TIMER / STOP_TIMER / ADD_TIMER / SUB_TIMER
- SET_GLOBAL / CLEAR_GLOBAL
- FORCE_TRIG / DESTR_TRIG
- DESTROY_OBJ
- WIN / LOSE / ALLOWWIN
- DZ, PLAY_MOVIE, TEXT

### Trigger Events (Implemented)

- TIME, ENTERED, ATTACKED, DESTROYED
- CREDITS, DISCOVERED, HOUSE_DISC, GLOBAL

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

See [CHANGELOG.md](CHANGELOG.md) for detailed completion history.
