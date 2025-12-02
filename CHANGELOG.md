# Red Alert macOS Port - Changelog

## December 2, 2024

### UI Phase 1: Sidebar Restructure Complete

The sidebar now matches the original Red Alert 1996 layout:

| ID | Item | Description |
|----|------|-------------|
| UI-1a | Two-column cameo layout | Structures left, units right |
| UI-1b | Repair/Sell/Zoom buttons | Top buttons below radar |
| UI-1c | Scroll buttons | Per-column scroll arrows |
| UI-1d | Power bar | Vertical gauge on sidebar edge |

### Campaign Phase 1 Complete

All Phase 1 campaign infrastructure is now working:

| ID | Item | Description |
|----|------|-------------|
| CAM-1 | INI-based mission loading | Loads SCG/SCU missions from GENERAL.MIX |
| CAM-2 | Complete trigger actions | All critical actions implemented |
| CAM-3 | Unit/building carryover | Save_Carryover/Load_Carryover system |
| CAM-4 | Briefing video integration | VQA playback for Brief/Win/Lose |

### Trigger Actions
| ID | Item | Notes |
|----|------|-------|
| TR-5 | AUTOCREATE | AI team auto-creation |
| TR-6 | DESTROY_TEAM | Team unit tracking + removal |
| TR-7 | FIRE_SALE | AI sells all buildings |
| TR-8 | DZ (drop zone) | Flare state + rendering API |
| TR-9 | PLAY_MOVIE | Mid-mission VQA playback |
| TR-10 | START/STOP_TIMER | Mission timer control |
| TR-11 | DESTROY_OBJ | Trigger-linked destruction |
| TR-12 | ALL_HUNT | Set all units to hunt mode |
| TR-13 | REVEAL_ALL | Full map reveal |
| TR-14 | REVEAL_SOME | Waypoint-based reveal |
| TR-15 | REINFORCE | Spawn units at waypoint |

### Trigger Events
| ID | Item | Notes |
|----|------|-------|
| EV-4 | CREDITS event | Threshold comparison |
| EV-5 | DISCOVERED event | Fog reveal tracking |
| EV-6 | HOUSE_DISC event | Enemy sighting |

### AI & Combat
| ID | Item | Notes |
|----|------|-------|
| AI-1 | Hunt mode improvements | Map-wide targeting |
| AI-2 | Team formations | Formation movement |
| AI-3 | Threat assessment | Target prioritization |
| CB-1 | Armor types | Full warhead matrix |
| CB-2 | Weapon damage mods | Per-armor multipliers |
| CB-3 | Scatter on explosion | Infantry scatter |
| PR-1 | Build prerequisites | Tech tree validation |

### UI
| ID | Item | Notes |
|----|------|-------|
| UI-2 | TEXT action | Mission text overlay |

### Bug Fixes
| ID | Item | Notes |
|----|------|-------|
| BUG-03 | P for Pause | Pause toggle working |
| BUG-04 | Briefing after video | Palette/clip reset mitigation |
| BUG-05 | Fog initialization | Fog starts black properly |
| BUG-09 | Attack cursor | Context-sensitive cursor system |
| BUG-10 | Units cross cliffs | Terrain passability check added |
| BUG-11 | Teal dots sprites | Sprite loading fixed |
| BUG-12/13 | Building render order | Y-sorted rendering + placement |
| BUG-14 | Chinook transport | Full transport load/unload system |
| BUG-15 | Helicopter blocked | Aircraft skip cell occupancy |
| BUG-16 | Group transport load | loadTarget tracking, retry rotation |
| BUG-17 | Jeep climbs cliffs | Fixed template ID mapping |

---

## November 2024

### Core Systems

| ID | Item | Notes |
|----|------|-------|
| SYS-1 | Metal renderer | 60 FPS palette rendering |
| SYS-2 | Theater loading | TEMPERAT, SNOW, INTERIOR MIX |
| SYS-3 | Team color remap | Sprite palette remapping |
| SYS-4 | A* pathfinding | Ground unit movement |
| SYS-5 | Aircraft flight | Direct flight pathing |
| SYS-6 | VQA playback | Video with audio sync |
| SYS-7 | AUD streaming | Background music |
| SYS-8 | Save/Load | Game state persistence |
| SYS-9 | Fog of war | Shroud + fog system |
| SYS-10 | Menu system | Main menu, options |

### Configuration

| ID | Item | Notes |
|----|------|-------|
| CFG-1 | Mutable type data | Runtime type modification |
| CFG-2 | Unit/Infantry loading | RULES.INI parsing |
| CFG-3 | Building loading | Structure data from INI |
| CFG-4 | Weapon/Warhead | Combat data from INI |
| CFG-5 | Game constants | Speed, costs, etc. |
| CFG-6 | Aircraft flight | Direct flight mode |
