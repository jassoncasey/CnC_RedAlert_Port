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

### M35: Tech Tree & Prerequisites ← **START HERE**

**Goal:** Advanced units/buildings require prerequisites.

**Original tech tree (simplified):**
```
Construction Yard
  └─ Power Plant
      └─ Barracks → Infantry (E1, E2)
      └─ Ore Refinery → Harvester
          └─ War Factory → Vehicles (1TNK, 2TNK)
              └─ Radar Dome
                  └─ Tech Center → Advanced units
```

**Tasks:**
1. [ ] Track which buildings player owns
2. [ ] Each build item has prerequisite list
3. [ ] Unavailable items greyed out (not black)
4. [ ] Tooltip shows "Requires: Barracks"
5. [ ] Building something unlocks dependent items

**Verification:**
- [ ] Game start: only Power Plant available
- [ ] Build Power Plant → Barracks/Refinery unlock
- [ ] Build Barracks → E1/E2 infantry unlock
- [ ] Build War Factory → Tanks unlock

---

### M36: Resource Economy

**Goal:** Harvesters collect ore, refineries convert to credits.

**Tasks:**
1. [ ] Ore fields on map (yellow terrain)
2. [ ] Harvester unit auto-harvests when idle
3. [ ] Harvester fills up, returns to Refinery
4. [ ] Refinery converts ore to credits over time
5. [ ] Credits display updates in real-time
6. [ ] Can't build if insufficient credits

**Verification:**
- [ ] Harvester drives to ore field
- [ ] Harvester fills (shows on unit?)
- [ ] Returns to Refinery
- [ ] Credits tick up
- [ ] Try to build expensive item with no credits → rejected

---

### M37: Basic AI Opponent

**Goal:** Enemy builds base and attacks player.

**Tasks:**
1. [ ] AI player with own credits and buildings
2. [ ] AI follows simple build order (power → refinery → barracks)
3. [ ] AI produces units when it can afford them
4. [ ] AI sends attack waves periodically
5. [ ] AI defends its base when attacked

**Verification:**
- [ ] Enemy base exists at start
- [ ] Enemy produces units over time
- [ ] Enemy units attack player base
- [ ] Can destroy enemy to win

---

## Checkpoint: Playable Skirmish

After M33-M37, we have:
- [x] Working map with terrain
- [x] Units that move and fight
- [x] **Production that works** (M33 complete)
- [x] **Building placement** (M34 complete)
- [ ] **Tech tree progression**
- [ ] **Resource economy**
- [ ] **AI opponent**

This is a playable game loop: build base → train army → destroy enemy.

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
| **Tech tree** | Missing | **HIGH** |
| **Economy** | Missing | **HIGH** |
| **AI opponent** | Missing | **HIGH** |
| Fog of war | Missing | Medium |
| Attack commands | Missing | Medium |
| Campaign missions | Missing | Low (need skirmish first) |
| Cutscenes/music | Missing | Low |

**Next step:** M35 - Tech tree with prerequisites.
