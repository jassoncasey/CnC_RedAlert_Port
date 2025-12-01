# Red Alert macOS Port - Roadmap to Full Gameplay

**Goal:** Play the complete Allied and Soviet campaigns with all original units, buildings, and behaviors.

**Completed work:** See [COMPLETED.md](COMPLETED.md)

---

## Current State (Updated)

**What works:**
- Window opens, Metal renders at 60 FPS
- Terrain tiles load from snow.mix
- Unit sprites load from conquer.mix
- Units move with A* pathfinding around obstacles
- Combat works (units attack, take damage, die)
- Menu system functional
- Sidebar UI renders (radar, build strips, selection panel)
- Radar minimap shows terrain/units (click-to-scroll works)
- Selection panel shows unit info (health bar, state)
- **Production system functional** - click to build units, progress bar, spawns on map
- **Credits system** - starting credits, deducted on production
- **Unit spawn validation** - units can't spawn on water
- **Building placement system** - click structure in sidebar, shows footprint cursor, validates placement, places on map

**What's broken/incomplete:**
- No real tech tree (demo has all buildings unlocked)
- No harvester/economy loop
- Units spawn at fixed location (should spawn at production building)

**What's missing entirely:**
- Tech tree with real prerequisites
- Harvester/ore economy
- AI opponent
- Campaign missions
- Cutscenes and music

---

## Immediate Priority: Playable Skirmish

Before campaigns, we need a working "skirmish" where you can:
1. Build a base (construction yard → power → barracks → war factory)
2. Train units (click sidebar → unit spawns)
3. Gather resources (harvesters collect ore → credits increase)
4. Fight AI (enemy builds and attacks)

### ✓ M33: Working Production System - COMPLETE

**Goal:** Click a sidebar button, wait, unit appears on map.

**Completed:**
- [x] Sidebar renders with build items (POWR, PROC, TENT, WEAP, E1-E3, ENG, 1TNK, 2TNK)
- [x] Wire sidebar to game state with credits
- [x] Add player credits (start with $5000)
- [x] Clicking available item starts countdown timer
- [x] Show progress bar filling during production
- [x] When complete, spawn unit at valid land position
- [x] Deduct cost from credits
- [x] Unit spawn position validation (no water spawning)

**Verified:**
- [x] Click "E1" (Rifle Infantry) → progress bar fills over ~2-3 seconds
- [x] Unit spawns near player's starting position
- [x] Credits decrease by 100
- [x] Can queue another unit after previous completes

---

### ✓ M34: Construction Yard & Building Placement - COMPLETE

**Goal:** Build structures from sidebar, place them on map.

**Completed:**
- [x] Click structure in sidebar → starts construction
- [x] Progress bar fills during construction
- [x] When ready, shows "READY" and enter placement mode
- [x] Building footprint follows cursor
- [x] Green cells = valid placement, Red cells = invalid (water/rock/building/units)
- [x] Invalid cells show X pattern
- [x] Click to place building on map
- [x] Building appears on radar
- [x] Cells marked as occupied (terrain=BUILDING)
- [x] Right-click or ESC cancels placement (refunds cost)
- [x] Sidebar shows "Click map to place" hint

**Verified:**
- [x] Click "POWR" → progress fills, shows "READY"
- [x] Cursor shows 2x2 footprint following mouse
- [x] Can only place on valid ground
- [x] Building appears on map and radar

**Files:** `ui/game_ui.cpp`, `ui/game_ui.h`

---

### ✓ M35: Tech Tree & Prerequisites - COMPLETE

**Goal:** Advanced units/buildings require prerequisites.

**Original tech tree (implemented):**
```
Construction Yard
  └─ Power Plant (no prereqs)
      ├─ Barracks (power) → Infantry (E1, E2, E3, ENG)
      ├─ Ore Refinery (power)
      └─ War Factory (power + refinery) → Vehicles (1TNK, 2TNK)
          └─ Radar Dome (power + factory)
```

**Completed:**
- [x] Track which buildings player owns (bitmask flags)
- [x] Each build item has prerequisite bitmask
- [x] Unavailable items greyed out with "Need:" text
- [x] Building something unlocks dependent items
- [x] Cost shown in red when can't afford

**Verified:**
- [x] Game start: only Power Plant available
- [x] Build Power Plant → Barracks/Refinery unlock
- [x] Build Barracks → Infantry unlock
- [x] Build Refinery → War Factory unlock
- [x] Build War Factory → Tanks unlock

**Files:** `ui/game_ui.cpp`

---

### ✓ M36: Resource Economy - COMPLETE

**Goal:** Harvesters collect ore, refineries convert to credits.

**Completed:**
- [x] Ore fields on map with ore amounts (0-255 per cell)
- [x] Harvester unit auto-harvests when idle (finds nearest ore)
- [x] Harvester fills up (cargo system: 0-1000 capacity)
- [x] Harvester returns to refinery when 75%+ full
- [x] Refinery converts ore to credits (7 credits per ore unit)
- [x] Credits display updates in real-time
- [x] Can't build if insufficient credits (cost shown in red)
- [x] Harvester buildable from sidebar (requires Refinery)

**Verified:**
- [x] Harvester drives to nearest ore field
- [x] Harvester harvests ore (depletes cells)
- [x] Returns to Refinery when full
- [x] Credits increase on delivery
- [x] Depleted ore cells become clear terrain

**Files:** `game/units.cpp`, `game/units.h`, `game/map.cpp`, `game/map.h`, `ui/game_ui.cpp`

---

### ✓ M37: Basic AI Opponent - COMPLETE

**Goal:** Enemy builds base and attacks player.

**Completed:**
- [x] AI player with own credits and buildings
- [x] AI follows simple build order (power → refinery → barracks → factory → radar)
- [x] AI produces units when it can afford them (weighted random selection)
- [x] AI sends attack waves periodically (configurable timing)
- [x] AI defends its base when attacked (auto-acquire targets)
- [x] Three difficulty levels (Easy/Medium/Hard) with different timings

**Verified:**
- [x] Enemy base exists at start
- [x] Enemy produces units over time
- [x] Enemy units attack player base
- [x] Can destroy enemy to win

**Files:** `game/ai.cpp`, `game/ai.h`

---

## Checkpoint: Playable Skirmish - COMPLETE

After M33-M37, we have:
- [x] Working map with terrain
- [x] Units that move and fight
- [x] **Production that works** (M33 complete)
- [x] **Building placement** (M34 complete)
- [x] **Tech tree progression** (M35 complete)
- [x] **Resource economy** (M36 complete)
- [x] **AI opponent** (M37 complete)

**The game is now playable!** Build base → train army → destroy enemy.

---

## Phase 2: Combat Polish (M38-M40)

### M38: Fog of War
- Map starts black (shroud)
- Units reveal area around them
- Previously seen areas are dimmed
- Enemy units only visible in sight range

### M39: Attack Commands
- Attack-move (A + click)
- Force-attack (Ctrl + click)
- Guard command
- Patrol waypoints

### M40: Unit Behaviors
- Auto-acquire targets
- Return fire when attacked
- Infantry scatter from explosions
- Tanks crush infantry

---

## Phase 3: Campaign Support (M41-M44)

### M41: Scenario Loading
- Parse original .INI mission files
- Load correct map/terrain
- Spawn starting units and buildings
- Read trigger definitions

### M42: Mission Objectives
- Win/lose condition triggers
- Objective tracking UI
- Victory/defeat detection

### M43: Campaign Flow
- Campaign selection (Allied/Soviet)
- Mission progression
- Save/load campaign state

### M44: Briefings
- Pre-mission briefing screen
- Mission text and map preview

---

## Phase 4: Media (M45-M47)

### M45: VQA Cutscenes
- Play intro video
- Mission win/lose videos
- Campaign end videos

### M46: Music
- Background music streaming
- Track rotation
- Volume control

### M47: Sound Polish
- All unit acknowledgments
- Building sounds
- Combat sounds complete

---

## Summary: What Makes It "Red Alert"

| Feature | Status | Priority |
|---------|--------|----------|
| Graphics/rendering | ✓ Done | - |
| Unit movement/pathfinding | ✓ Done | - |
| Basic combat | ✓ Done | - |
| UI layout (sidebar/radar) | ✓ Done | - |
| **Production system** | ✓ Done (M33) | - |
| **Building placement** | ✓ Done (M34) | - |
| **Tech tree** | ✓ Done (M35) | - |
| **Economy** | ✓ Done (M36) | - |
| **AI opponent** | ✓ Done (M37) | - |
| Fog of war | Missing | Medium |
| Attack commands | Missing | Medium |
| Campaign missions | Missing | Medium |
| Cutscenes/music | Missing | Low |

**Next step:** M38 - Fog of War, or M41 - Scenario Loading.
