# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Real mission loading with simplifications.
Core triggers working. Unit types complete. Team colors working.

---

## Active Work Queue

*Sorted by leverage (impact/effort ratio). Work top-to-bottom.*

### HIGH LEVERAGE

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| **BUG-01** | Music ADPCM distortion | 4-8 hrs | Background music heavily distorted |
| **BUG-02** | Video audio static | 4-8 hrs | VQA audio has static/noise |
| **TR-4** | BEGIN_PROD action | 3 hrs | Enable AI production queue |
| **UI-2** | TEXT action | 3 hrs | Display mission briefing text |

### MEDIUM LEVERAGE

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| **VIS-2** | Selection box visibility | 1 hr | Clearer unit selection indicators |
| **BUG-09** | Target cursor on hover | 2 hrs | Attack cursor when hovering enemies |
| **UI-1** | Mission timer display | 2 hrs | Show countdown timer in HUD |
| **EV-4** | CREDITS event | 1 hr | Trigger on credit threshold |
| **TR-5** | AUTOCREATE action | 2 hrs | Auto team creation for AI |
| **AI-1** | Hunt mode | 3 hrs | Units seek and destroy enemies |

### LOW LEVERAGE

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| **TR-6** | DESTROY_TEAM | 1 hr | Remove team from game |
| **TR-7** | FIRE_SALE | 2 hrs | AI sells all buildings |
| **TR-8** | DZ (drop zone) | 2 hrs | Drop zone flare effect |
| **TR-9** | PLAY_MOVIE | 2 hrs | Trigger mid-mission movies |
| **TR-10** | START/STOP_TIMER | 2 hrs | Mission timer control |
| **TR-11** | DESTROY_OBJ | 2 hrs | Destroy trigger-linked object |
| **EV-5** | DISCOVERED event | 2 hrs | Trigger on fog reveal |
| **EV-6** | HOUSE_DISC event | 1 hr | Trigger on enemy sighting |
| **UI-3** | Radar zoom | 2 hrs | Zoom levels for minimap |
| **UI-4** | Sidebar scroll | 1 hr | Scroll long build lists |
| **AI-2** | Team formations | 4 hrs | Units move in formation |
| **AI-3** | Threat assessment | 4 hrs | Smarter target selection |
| **MV-1** | Naval pathfinding | 4 hrs | Water-only movement |
| **MV-2** | Aircraft movement | 4 hrs | Flying unit pathfinding |
| **MV-3** | Transport load/unload | 3 hrs | APCs, transports |
| **PR-3** | Power affects production | 2 hrs | Low power slows building |

### SIMPLIFIED (Working but not authentic)

| ID | Item | Effort | What's Simplified |
|----|------|--------|-------------------|
| **CB-1** | Armor types | 3 hrs | All units same armor class |
| **CB-2** | Weapon damage mods | 2 hrs | No armor vs weapon matrix |
| **CB-3** | Scatter on explosion | 1 hr | Infantry don't scatter |
| **PR-1** | Build prerequisites | 3 hrs | No tech tree validation |
| **PR-2** | Production queue | 2 hrs | Basic queue only |

### CODE QUALITY (No gameplay impact)

| ID | Item | Effort | Description |
|----|------|--------|-------------|
| SE-3.1 | rules.cpp 80-col | 1 hr | Line length fixes |
| SE-3.2 | menu.cpp 80-col | 1 hr | Line length fixes |
| SE-3.3 | units.cpp 80-col | 1 hr | Line length fixes |
| SE-4.1 | ParseTrigs globals | 1 hr | Reduce global state |
| SE-4.3 | Trigger dispatch | 2 hrs | Table-driven triggers |

---

## Stub Details

### Stubbed Trigger Events (mission.cpp)

| Event | What's Missing |
|-------|----------------|
| CREDITS | Compare player credits to threshold param |
| DISCOVERED | Track when fog cells are revealed |
| HOUSE_DISC | Track when enemy units first seen |

### Stubbed Trigger Actions (mission.cpp)

| Action | What's Missing |
|--------|----------------|
| BEGIN_PROD | Enable AI build queue |
| DESTROY_TEAM | Remove team and its units |
| DZ | Render drop zone flare sprite |
| FIRE_SALE | AI sells all buildings for credits |
| TEXT | Display text overlay in HUD |
| AUTOCREATE | Enable automatic team creation |
| START_TIMER | Show and start countdown timer |
| STOP_TIMER | Stop and hide countdown timer |
| DESTROY_OBJ | Find and destroy trigger-linked object |

### Simplified Systems

| System | File | Current Implementation |
|--------|------|------------------------|
| Infantry scatter | units.cpp | No scatter on nearby explosions |
| Armor types | combat.cpp | Single armor class for all units |
| Weapon ranges | weapon_types.cpp | Simplified range calculations |
| Team formations | ai.cpp | No formation movement |
| Threat assessment | ai.cpp | Basic nearest-enemy targeting |
| Build prerequisites | building_types.cpp | No tech tree validation |
| Naval movement | units.cpp | Uses ground pathfinding |
| Aircraft movement | units.cpp | Uses ground pathfinding |

---

## Completed Work

### What Works

- Metal renderer 60 FPS
- Terrain from theater MIX files (TEMPERAT, SNOW, INTERIOR)
- Sprites from conquer.mix with team color remapping
- A* pathfinding (ground units)
- Combat system (damage, death, projectiles)
- Menu system (main menu, options, credits)
- Sidebar UI (build buttons, power bar)
- Production/placement
- Economy (credits, ore harvesting)
- AI opponent (basic attack behavior)
- Fog of war
- Attack commands
- Campaign mission flow
- VQA video playback
- Background music (distorted)
- Trigger system (parsing, evaluation, chaining)

### Completed Features

| ID | Item | Date |
|----|------|------|
| VIS-1 | Sprite color remapping (team colors) | Dec 2 |
| BUG-07 | Map centering on player start | Dec 2 |
| BUG-08 | Units not moving (unit types fix) | Dec 2 |
| TR-1 | CREATE_TEAM action | Nov |
| TR-2 | REINFORCE action | Nov |
| TR-3 | ALL_HUNT action | Nov |
| EV-1 | ENTERED event | Nov |
| EV-2 | ATTACKED event | Nov |
| EV-3 | DESTROYED event | Nov |
| - | Global flags (SET/CLEAR_GLOBAL) | Nov |
| - | FORCE_TRIG action | Nov |
| - | Complete unit types (50 types) | Dec 2 |
| - | Complete building types | Nov |

### Fixed Bugs

| ID | Issue | Resolution |
|----|-------|------------|
| BUG-07 | Map not centered on player start | Fixed viewport calculation |
| BUG-08 | Some units won't move | Completed g_unitTypes array |

---

## Next Step

**BUG-01: Music ADPCM distortion** - HIGH leverage, 4-8 hrs

The background music plays but is heavily distorted. This is the most
noticeable quality issue affecting the game experience. The ADPCM decoder
in audfile.cpp may have issues with the IMA ADPCM format used by Red Alert.

Alternative: **TR-4: BEGIN_PROD** (3 hrs) - Enable AI production for more
dynamic missions. Currently AI doesn't build new units during gameplay.
