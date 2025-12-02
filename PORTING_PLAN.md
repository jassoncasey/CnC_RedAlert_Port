# Red Alert macOS Port - Roadmap

**Goal:** Faithful recreation of original Red Alert with full 14-mission
campaigns (Allied + Soviet) playable from original game assets.

**Current State:** Phase 1 complete. Starting UI overhaul to match
original game interface.

---

## UI Overhaul (Priority - In Progress)

The current UI uses placeholder programmatic rendering. This phase
brings it to match the original Red Alert interface.

### Original Layout Reference (320x200 base, scaled to 640x400)

```
+----------------------------------------------------------+--------+
|  CREDITS: $5000                    [Options]             | RADAR  |
+----------------------------------------------------------+        |
|                                                          |  64x64 |
|                                                          +--------+
|                                                          |Rep|Sel|Z|
|                    GAME VIEW                             +--------+
|                    (Terrain + Units)                     | [cam]  |
|                                                          | [cam]  |
|                                                          | [cam]  |
|                                                          | [cam]  |
|                                                          +--------+
|                                                          | [^][v] |
+----------------------------------------------------------+--------+
```

**Original Dimensions (320x200 mode):**
- Sidebar: X=240, Width=80, Height=123 (starts at Y=77)
- Radar: 64x64 pixels, above sidebar
- Cameo icons: 32x24 each, 2 columns, 4 visible rows
- Top buttons: Repair(32w), Sell(20w), Zoom(20w), height=9

### Phase UI-1: Sidebar Restructure (~6 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| UI-1a | Two-column cameo layout | 2 hrs | Pending |
| UI-1b | Repair/Sell/Zoom buttons | 1 hr | Pending |
| UI-1c | Scroll buttons for strips | 1 hr | Pending |
| UI-1d | Power bar (vertical) | 2 hrs | Pending |

**UI-1a: Two-Column Cameo Layout**
- Structures in left column, units in right column
- Each cameo: 32x24 pixels
- 4 visible rows (scrollable)
- Production clock overlay on cameos

**UI-1b: Repair/Sell/Zoom Buttons**
- Top row of sidebar (height 13px)
- Repair: wrench icon, toggle mode
- Sell: dollar icon, toggle mode
- Zoom: magnifier, centers on base

**UI-1c: Scroll Buttons**
- Up/down arrows per column
- Below the 4 visible cameo rows
- Smooth scrolling animation

**UI-1d: Power Bar**
- Vertical gauge on right edge of sidebar
- Shows power produced vs consumed
- Red zone when low power

### Phase UI-2: Radar Separation (~3 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| UI-2a | Move radar above sidebar | 1 hr | Pending |
| UI-2b | Radar frame/border | 1 hr | Pending |
| UI-2c | Radar activation animation | 1 hr | Pending |

**UI-2a: Move Radar Above Sidebar**
- Radar at top-right, 64x64
- Separate from build strips
- Frame border around radar

**UI-2b: Radar Frame/Border**
- Beveled frame matching original style
- Dark gray 3D effect

**UI-2c: Radar Activation Animation**
- Sweep effect when radar comes online
- Requires radar building

### Phase UI-3: Top Bar (~2 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| UI-3a | Credits display (top-left) | 0.5 hr | Pending |
| UI-3b | Options button (top-right) | 0.5 hr | Pending |
| UI-3c | Mission timer display | 0.5 hr | Pending |
| UI-3d | Top bar styling | 0.5 hr | Pending |

### Phase UI-4: Cameo Icons (~4 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| UI-4a | Load cameo SHP files | 2 hrs | Pending |
| UI-4b | Render cameos in sidebar | 1 hr | Pending |
| UI-4c | Production clock overlay | 1 hr | Pending |

**UI-4a: Load Cameo SHP Files**
- Cameos are in CONQUER.MIX
- Format: <TYPE>ICON.SHP (e.g., POWIICON.SHP for Power Plant)
- 32x24 indexed color images

---

## Campaign Mode Roadmap

### Phase 1: Single Campaign Playable - COMPLETE

| ID | Item | Effort | Status |
|----|------|--------|--------|
| CAM-1 | INI-based mission loading | 8 hrs | **DONE** |
| CAM-2 | Complete trigger actions | 6 hrs | **DONE** |
| CAM-3 | Unit/building carryover | 4 hrs | **DONE** |
| CAM-4 | Briefing video integration | 2 hrs | **DONE** |

### Phase 2: Full 14+14 Campaign (~15 hrs)

| ID | Item | Effort | Status |
|----|------|--------|--------|
| CAM-5 | Mission variant branching | 4 hrs | Pending |
| CAM-6 | Score screen | 3 hrs | Pending |
| CAM-7 | All win/lose conditions | 4 hrs | Pending |
| CAM-8 | Campaign testing/fixes | 4 hrs | Pending |

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
- CREATE_TEAM, DESTROY_TEAM, ALL_HUNT
- BEGIN_PROD, AUTOCREATE, FIRE_SALE
- WIN, LOSE, ALLOWWIN
- REINFORCE, DZ, DESTROY_OBJ
- START_TIMER / STOP_TIMER / ADD_TIMER / SUB_TIMER
- SET_GLOBAL / CLEAR_GLOBAL
- FORCE_TRIG / DESTR_TRIG
- REVEAL_ALL / REVEAL_SOME
- PLAY_MOVIE, TEXT

### Needs Implementation (Non-Critical)

**Events:**
- NOFACTORIES, ALL_BRIDGES_DESTROYED
- EVAC_CIVILIAN, FAKES_DESTROYED
- BUILDING_EXISTS, cross-line/zone triggers

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
- Sidebar UI with production (basic)
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

---

## Known Issues

| ID | Issue | Priority |
|----|-------|----------|
| FOG-1 | Fog shows gray for enemy areas | Low |
| AUD-1 | Audio distortion on some sounds | Low |

---

## Effort Summary

| Phase | Hours | Description |
|-------|-------|-------------|
| UI Overhaul | 15 hrs | Match original interface |
| Phase 2 | 15 hrs | Full 14+14 campaigns |
| Phase 3 | 25 hrs | Expansion content |
| Backlog | 10 hrs | Skirmish, polish |
| **Total** | **~65 hrs** | Complete faithful port |

---

See [CHANGELOG.md](CHANGELOG.md) for completion history.
