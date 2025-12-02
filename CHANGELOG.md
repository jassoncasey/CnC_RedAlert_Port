# Red Alert macOS Port - Changelog

## December 2, 2024

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
| BUG-12/13 | Building render order | Y-sorted rendering + placement check |
| BUG-14 | Chinook transport | Full transport load/unload system |
| BUG-15 | Helicopter blocked | Aircraft skip cell occupancy checks |
| BUG-16 | Group transport load | loadTarget tracking, angle-spread pathing, retry rotation |
| BUG-17 | Jeep climbs cliffs | Fixed template ID mapping (135-172 = cliffs) |
