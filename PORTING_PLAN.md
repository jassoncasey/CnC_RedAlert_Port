# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions (28 Allied + 16 Soviet) and skirmish mode against AI.

**Completed work:** See [COMPLETED.md](COMPLETED.md)

---

## Current State

### What Works (Demo Mode)
- Metal renderer at 60 FPS (640x400, palette-based)
- Terrain tiles from snow.mix
- Unit sprites from conquer.mix
- A* pathfinding, unit movement
- Combat (attack, damage, death)
- Menu system (main, options, credits, campaign select)
- Sidebar UI (radar, build strips, selection panel)
- Production system (click to build, progress bar, spawns at building)
- Building placement (footprint cursor, validation, placement)
- Tech tree (prerequisites unlock items)
- Economy (harvesters gather ore, refineries convert to credits)
- AI opponent (builds base, produces units, attacks)
- Fog of war (units reveal area)
- Attack commands (A-move, guard, force-attack, stop)
- Campaign flow (Allied/Soviet select, difficulty, briefings)
- VQA video playback (intro, briefings)
- Background music (streaming from SCORES.MIX)

### What's Missing (Demo → Real Game)
- Real mission loading (currently procedural demo map)
- Multiple terrain theaters (only snow implemented)
- Full unit roster (missing many unit types)
- Mission triggers (win/lose beyond "destroy all")
- Naval units
- Aircraft
- Superweapons

---

## Known Issues

| ID | Issue | Description | Severity | Status |
|----|-------|-------------|----------|--------|
| BUG-01 | Music Distortion | Background music heavily distorted, likely ADPCM decode or sample rate issue | High | Open |
| BUG-02 | Video Audio Static | VQA playback has audio noise/static | High | Open |
| BUG-03 | P for Pause | Units keep moving when P pressed, pause flag not respected | Medium | Open |
| BUG-04 | Briefing Garbled | Mission briefing shows artifacts after video playback | Medium | Open |
| BUG-05 | Fog Re-blacks | Revealed terrain goes black when units leave (should stay dimmed) | Medium | Open |
| BUG-06 | Sound Volume | Can't test sound slider without sample playback button | Low | Open |

---

## Phase A: Stabilization

*Fix critical bugs before adding features. These impact testing and user experience.*

### A1: Audio Quality - NOT STARTED
**Goal:** Clean audio playback for music and video.

**Tasks:**
- [ ] Debug music distortion (check ADPCM decoder, sample rate conversion)
- [ ] Debug video audio static (check VQA audio stream decoding)
- [ ] Verify audio callback timing and buffer handling
- [ ] Test with multiple tracks to isolate issue

**Files:** `audio/audio.mm`, `video/music.cpp`, `video/vqa.cpp`, `assets/audfile.cpp`

### A2: UI Polish - NOT STARTED
**Goal:** Fix rendering and input bugs.

**Tasks:**
- [ ] Fix briefing screen rendering after video (palette/framebuffer restore)
- [ ] Fix P for pause (trace key detection → pause flag → Units_Update gate)
- [ ] Fix fog of war to keep revealed terrain visible (only hide enemy units)
- [ ] Add test tone button to Options for sound volume testing

**Files:** `ui/menu.cpp`, `main.mm`, `game/map.cpp`, `game/units.cpp`

---

## Phase B: Real Missions

*The critical path from demo to real game.*

### B1: Mission File Loading - NOT STARTED
**Goal:** Load actual Red Alert missions from INI files.

**Tasks:**
- [ ] Extract mission INIs from REDALERT.MIX (SCU01EA.INI, SCG01EA.INI, etc.)
- [ ] Parse [Basic] section (name, theater, players)
- [ ] Parse [Map] section (dimensions, position)
- [ ] Parse [MapPack] section (terrain data - base64 encoded)
- [ ] Parse [OverlayPack] section (ore, walls, etc.)
- [ ] Parse [TERRAIN] section (trees, rocks)
- [ ] Parse [Units], [Infantry], [Structures] sections
- [ ] Parse [Waypoints] section (spawn points, objectives)
- [ ] Parse [CellTriggers] section

**Mission file format reference:**
```ini
[Basic]
Name=SCU01EA
Theater=TEMPERATE
Player=Greece

[Map]
X=0
Y=0
Width=64
Height=64

[Units]
0=Greece,1TNK,256,64,128,Guard
1=USSR,3TNK,512,192,64,Area Guard

[Structures]
0=Greece,FACT,256,320
1=Greece,POWR,288,320
```

**Files:** `game/mission.cpp`, `game/ini.cpp`, `assets/assetloader.cpp`

### B2: Theater Support - NOT STARTED
**Goal:** Support all three terrain types.

**Theaters:**
| Theater | MIX File | Palette | Used In |
|---------|----------|---------|---------|
| Temperate | TEMPERAT.MIX | TEMPERAT.PAL | Most Allied missions |
| Snow | SNOW.MIX | SNOW.PAL | Soviet missions, current demo |
| Desert | DESERT.MIX | DESERT.PAL | Counterstrike expansion |

**Tasks:**
- [ ] Load theater from mission [Basic] section
- [ ] Load correct terrain MIX file based on theater
- [ ] Load correct palette based on theater
- [ ] Handle theater-specific terrain tiles (TMP files differ per theater)

**Files:** `assets/assetloader.cpp`, `game/terrain.cpp`, `game/map.cpp`

### B3: Map Rendering - NOT STARTED
**Goal:** Render maps from mission data instead of procedural generation.

**Tasks:**
- [ ] Decode [MapPack] base64 data to terrain template indices
- [ ] Decode [OverlayPack] base64 data to overlay types
- [ ] Map template indices to TMP files
- [ ] Render terrain using correct tiles
- [ ] Render overlays (ore, gems, walls, crates)
- [ ] Place [TERRAIN] objects (trees, rocks)

**Files:** `game/map.cpp`, `game/terrain.cpp`

### B4: Entity Spawning - NOT STARTED
**Goal:** Spawn units/buildings from mission INI.

**Tasks:**
- [ ] Parse unit entries: `ID=House,Type,Cell,Facing,Mission`
- [ ] Parse infantry entries: `ID=House,Type,Cell,SubCell,Facing,Mission`
- [ ] Parse structure entries: `ID=House,Type,Cell,Facing,Tag`
- [ ] Map type strings to UnitType/BuildingType enums
- [ ] Spawn at correct cell positions
- [ ] Set initial mission state (Guard, Area Guard, Hunt, etc.)
- [ ] Handle team assignments

**Files:** `game/mission.cpp`, `game/units.cpp`

### B5: Unit Type Expansion - NOT STARTED
**Goal:** Add all unit types from original game.

**Infantry (expand from current 4):**
| Type | Name | Prereq |
|------|------|--------|
| E1 | Rifle Infantry | Barracks |
| E2 | Grenadier | Barracks |
| E3 | Rocket Soldier | Barracks |
| E4 | Flamethrower | Barracks |
| E6 | Engineer | Barracks |
| SPY | Spy | Barracks + Tech |
| THF | Thief | Barracks |
| MEDI | Medic | Barracks |
| DOG | Attack Dog | Kennel |
| SHOK | Shock Trooper | Tesla Coil |

**Vehicles (expand from current 4):**
| Type | Name | Prereq |
|------|------|--------|
| HARV | Harvester | Refinery |
| MCV | Mobile Construction Vehicle | Factory |
| 1TNK | Light Tank | Factory |
| 2TNK | Medium Tank | Factory |
| 3TNK | Heavy Tank | Factory |
| 4TNK | Mammoth Tank | Factory + Tech |
| APC | APC | Factory |
| ARTY | Artillery | Factory |
| V2RL | V2 Rocket | Factory + Radar |
| MNLY | Minelayer | Factory |
| JEEP | Ranger | Factory |
| TRUK | Supply Truck | Factory |

**Naval (new):**
| Type | Name | Prereq |
|------|------|--------|
| SS | Submarine | Sub Pen |
| DD | Destroyer | Naval Yard |
| CA | Cruiser | Naval Yard + Tech |
| PT | Gunboat | Naval Yard |
| LST | Transport | Naval Yard |

**Tasks:**
- [ ] Add infantry types to unit_types.cpp
- [ ] Add vehicle types to unit_types.cpp
- [ ] Add naval types (new water movement)
- [ ] Add sprites for each type
- [ ] Add unit stats (health, speed, weapon, cost)
- [ ] Update sidebar with new units (conditional on prereqs)

**Files:** `game/unit_types.cpp`, `game/units.cpp`, `ui/game_ui.cpp`

### B6: Building Type Expansion - NOT STARTED
**Goal:** Add all building types.

**Structures (expand from current 6):**
| Type | Name | Size | Prereq |
|------|------|------|--------|
| FACT | Construction Yard | 3x3 | - |
| POWR | Power Plant | 2x2 | - |
| APWR | Advanced Power | 2x2 | Power + Tech |
| PROC | Ore Refinery | 3x3 | Power |
| SILO | Ore Silo | 1x1 | Refinery |
| BARR | Barracks | 2x2 | Power |
| TENT | Barracks (Allied) | 2x2 | Power |
| KENN | Kennel | 1x1 | Barracks |
| WEAP | War Factory | 3x3 | Power + Refinery |
| DOME | Radar Dome | 2x2 | Power + Factory |
| FIX | Service Depot | 3x3 | Factory |
| HPAD | Helipad | 2x2 | Radar |
| AFLD | Airfield | 2x3 | Radar |
| SYRD | Naval Yard | 3x3 | Power |
| SPEN | Sub Pen | 3x3 | Power |
| TECH | Tech Center | 2x2 | Radar |
| GUN | Turret | 1x1 | Barracks |
| AGUN | AA Gun | 1x1 | Radar |
| SAM | SAM Site | 2x1 | Radar |
| TSLA | Tesla Coil | 1x2 | Tech |
| GAP | Gap Generator | 1x1 | Tech |
| IRON | Iron Curtain | 2x2 | Tech |
| PDOX | Chronosphere | 2x2 | Tech |
| MSLO | Missile Silo | 2x1 | Tech |

**Tasks:**
- [ ] Add building types to building_types.cpp
- [ ] Add sprites for each type
- [ ] Add building stats (health, power, cost, size)
- [ ] Add special building behaviors (power generation, gap generator, etc.)
- [ ] Update sidebar with new buildings

**Files:** `game/building_types.cpp`, `game/units.cpp`, `ui/game_ui.cpp`

### B7: Mission Triggers - NOT STARTED
**Goal:** Event-driven mission logic.

**Trigger format:** `Name=Event,Action,Repeating,House`

**Events to implement:**
| Event | Description |
|-------|-------------|
| None | No trigger (always active) |
| Time | After N ticks |
| Discovered | Player sees location |
| Destroyed | All of type destroyed |
| Entered | Unit enters cell |
| NoBuildings | House has no buildings |
| NoUnits | House has no units |
| CivEvac | Civilians evacuated |

**Actions to implement:**
| Action | Description |
|--------|-------------|
| Win | Player wins mission |
| Lose | Player loses mission |
| Reinforcements | Spawn reinforcement team |
| CreateTeam | Create AI team |
| AllHunt | All enemy units hunt |
| Text | Display message |
| Reveal | Reveal map area |

**Tasks:**
- [ ] Parse [Triggers] section
- [ ] Parse [TeamTypes] section (for reinforcements)
- [ ] Implement event detection
- [ ] Implement action execution
- [ ] Wire triggers to game loop
- [ ] Test with Allied Mission 1

**Files:** `game/trigger.cpp`, `game/mission.cpp`, `game/scenario.cpp`

---

## Phase C: Campaign Progression

*Full campaign experience with mission sequencing and cutscenes.*

### C1: Mission Sequencing - NOT STARTED
**Goal:** Progress through campaign missions in order.

**Allied Campaign (28 missions):**
```
SCU01EA → SCU02EA → SCU03EA → ... → SCU14EA
         ↘ SCU03EB (branch)
```

**Soviet Campaign (16 missions):**
```
SCG01EA → SCG02EA → SCG03EA → ... → SCG14EA
```

**Tasks:**
- [ ] Define mission sequence tables
- [ ] Handle mission branching (A/B paths)
- [ ] Load next mission on victory
- [ ] Handle defeat (retry or menu)
- [ ] Track campaign progress (save/load)

**Files:** `game/campaign.cpp`, `ui/menu.cpp`

### C2: Cutscenes & Briefings - NOT STARTED
**Goal:** Play correct videos between missions.

**Video mapping:**
| Mission | Briefing Video | Victory Video |
|---------|----------------|---------------|
| Allied 1 | ALLY1.VQA | - |
| Allied 14 | ALLY14.VQA | ALLYEND.VQA |
| Soviet 1 | SOV1.VQA | - |
| Soviet 14 | SOV14.VQA | SOVEND.VQA |

**Tasks:**
- [ ] Map missions to briefing videos
- [ ] Map missions to victory videos
- [ ] Play briefing before mission
- [ ] Play victory video after mission complete
- [ ] Handle missing videos gracefully

**Files:** `ui/menu.cpp`, `video/vqa.cpp`

### C3: Difficulty & Scoring - NOT STARTED
**Goal:** Proper difficulty scaling and end-mission scoring.

**Difficulty effects:**
| Setting | Player Credits | Enemy Credits | AI Aggression |
|---------|----------------|---------------|---------------|
| Easy | 150% | 75% | Low |
| Normal | 100% | 100% | Normal |
| Hard | 75% | 150% | High |

**Tasks:**
- [ ] Scale starting credits by difficulty
- [ ] Scale AI parameters by difficulty
- [ ] Calculate end-mission score (time, units lost, buildings)
- [ ] Display score screen
- [ ] Track statistics

**Files:** `game/mission.cpp`, `game/ai.cpp`, `ui/menu.cpp`

---

## Phase D: Skirmish Mode

*Standalone battles against AI on random maps.*

### D1: Skirmish Setup - NOT STARTED
**Goal:** Configure and start skirmish battles.

**Options:**
- Map selection (or random)
- Number of AI players (1-7)
- AI difficulty per player
- Starting credits
- Superweapons on/off
- Crates on/off

**Tasks:**
- [ ] Skirmish setup menu
- [ ] Map browser (list available maps)
- [ ] AI player configuration
- [ ] Game rules configuration
- [ ] Start game with settings

**Files:** `ui/menu.cpp`, `game/scenario.cpp`

### D2: Random Map Generation - NOT STARTED
**Goal:** Generate playable random maps.

**Map features:**
- Balanced starting positions
- Ore fields near each player
- Varied terrain (water, cliffs, trees)
- Multiple terrain theaters

**Tasks:**
- [ ] Generate terrain heightmap
- [ ] Place ore fields fairly
- [ ] Place starting positions
- [ ] Ensure path connectivity
- [ ] Add decorative terrain (trees, rocks)

**Files:** `game/map.cpp`

### D3: Enhanced AI - NOT STARTED
**Goal:** Smarter AI for longer skirmish games.

**Improvements:**
- [ ] Multiple AI personalities (aggressive, defensive, balanced)
- [ ] Better base building (power management, expansion)
- [ ] Scouting behavior
- [ ] Tech rushing
- [ ] Counter-unit production
- [ ] Multi-front attacks

**Files:** `game/ai.cpp`

---

## Phase E: Polish

*Nice-to-have features for complete experience.*

### E1: Audio Polish - NOT STARTED
- [ ] Unit voice acknowledgments ("Yes sir!", "Acknowledged", etc.)
- [ ] Building construction sounds
- [ ] Complete combat sound effects
- [ ] Ambient sounds
- [ ] EVA voice ("Unit ready", "Building", etc.)

### E2: Visual Polish - NOT STARTED
- [ ] Explosion animations
- [ ] Muzzle flashes
- [ ] Building damage states
- [ ] Unit shadow sprites
- [ ] Water animation

### E3: Aircraft - NOT STARTED
- [ ] Helicopter movement
- [ ] Fixed-wing aircraft
- [ ] Air-to-ground attacks
- [ ] Anti-air defense
- [ ] Helipad/Airfield landing

### E4: Superweapons - NOT STARTED
- [ ] Chronosphere (teleport units)
- [ ] Iron Curtain (invulnerability)
- [ ] Nuclear Missile
- [ ] GPS Satellite
- [ ] Parabombs

---

## Summary

| Phase | Description | Priority | Status |
|-------|-------------|----------|--------|
| **A** | Stabilization (fix bugs) | High | Not Started |
| **B** | Real Missions (demo → game) | High | Not Started |
| **C** | Campaign Progression | Medium | Not Started |
| **D** | Skirmish Mode | Medium | Not Started |
| **E** | Polish | Low | Not Started |

**Minimum viable "real game":** Complete Phase A + B1-B4 (can load and play Allied Mission 1)

**Full campaign:** Complete through Phase C

**Complete game:** All phases including E
