# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Real mission loading implemented with
simplifications. Core triggers working. Unit types complete. Map centering fixed.

---

## Active Work Queue

*Ordered by leverage - highest impact items first.*

### TIER A: Visual Clarity (HIGH LEVERAGE)

Items that affect ability to distinguish units and play effectively.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **VIS-1** | Sprite color remapping (team colors) | 4 hrs | **HIGH** | DONE |
| **VIS-2** | Selection box visibility | 1 hr | MEDIUM | PARTIAL |

**Implementation notes:** VIS-1 complete. Palette indices 80-95 are remapped to
team-specific color gradients (blue for Allies, red for Soviets, etc.) during
sprite rendering via `Renderer_BlitRemapped()` and `Sprites_GetRemapTable()`.

### TIER B: Remaining Trigger Actions (MEDIUM LEVERAGE)

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **TR-4** | BEGIN_PROD action (AI production) | 3 hrs | MEDIUM | STUB |
| **TR-5** | AUTOCREATE action (auto team creation) | 2 hrs | LOW | STUB |
| **TR-6** | DESTROY_TEAM action | 1 hr | LOW | STUB |
| **TR-7** | FIRE_SALE action (sell all buildings) | 2 hrs | LOW | STUB |
| **TR-8** | DZ action (drop zone flare) | 2 hrs | LOW | STUB |
| **TR-9** | PLAY_MOVIE action | 2 hrs | LOW | STUB |
| **TR-10** | START_TIMER/STOP_TIMER actions | 2 hrs | LOW | STUB |
| **TR-11** | DESTROY_OBJ action | 2 hrs | LOW | STUB |

### TIER C: Remaining Trigger Events

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **EV-4** | CREDITS event (player credits threshold) | 1 hr | MEDIUM | STUB |
| **EV-5** | DISCOVERED event (fog reveal) | 2 hrs | LOW | STUB |
| **EV-6** | HOUSE_DISC event (house discovery) | 1 hr | LOW | STUB |

### TIER D: UI Features (MEDIUM LEVERAGE)

User-visible features that affect gameplay clarity.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **UI-1** | Mission timer display | 2 hrs | MEDIUM | MISSING |
| **UI-2** | TEXT action (mission text messages) | 3 hrs | MEDIUM | STUB |
| **UI-3** | Radar zoom levels | 2 hrs | LOW | MISSING |
| **UI-4** | Sidebar scroll | 1 hr | LOW | MISSING |

### TIER E: Audio Bugs (MEDIUM LEVERAGE)

High annoyance but don't block gameplay.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **BUG-01** | Music ADPCM distortion | 4-8 hrs | HIGH | OPEN |
| **BUG-02** | Video audio static | 4-8 hrs | HIGH | OPEN |

### TIER F: Combat/AI Refinement (LOW LEVERAGE)

Affects realism but game is playable without.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **AI-1** | Hunt mode for units | 3 hrs | MEDIUM | MISSING |
| **AI-2** | Team tactics (formations) | 4 hrs | LOW | MISSING |
| **AI-3** | Threat assessment | 4 hrs | LOW | MISSING |
| **CB-1** | Armor types (light/medium/heavy) | 3 hrs | LOW | SIMPLIFIED |
| **CB-2** | Weapon damage modifiers | 2 hrs | LOW | SIMPLIFIED |
| **CB-3** | Scatter on explosion | 1 hr | LOW | SIMPLIFIED |

### TIER G: Production/Economy (LOW LEVERAGE)

Demo mode already has basic production working.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **PR-1** | Build prerequisites validation | 3 hrs | LOW | SIMPLIFIED |
| **PR-2** | Production queue | 2 hrs | LOW | SIMPLIFIED |
| **PR-3** | Power affects production | 2 hrs | LOW | MISSING |

### TIER H: Movement/Pathfinding (LOW LEVERAGE)

Ground pathfinding works, specialized movement missing.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **MV-1** | Naval pathfinding | 4 hrs | LOW | MISSING |
| **MV-2** | Aircraft movement | 4 hrs | LOW | MISSING |
| **MV-3** | Transport loading/unloading | 3 hrs | LOW | MISSING |

### TIER Z: Code Quality (NO GAMEPLAY LEVERAGE)

Style and maintainability. Do when convenient.

| ID | Item | Effort | Status |
|----|------|--------|--------|
| SE-3.1 | rules.cpp 80-col fixes | 1 hr | PENDING |
| SE-3.2 | menu.cpp 80-col fixes | 1 hr | PENDING |
| SE-3.3 | units.cpp 80-col fixes | 1 hr | PENDING |
| SE-4.1 | ParseTrigsSection globals | 1 hr | PENDING |
| SE-4.3 | Trigger table-driven dispatch | 2 hrs | PENDING |

---

## Detailed Stub Documentation

### Stubbed Trigger Events (mission.cpp)

| Event | Line | What's Missing |
|-------|------|----------------|
| ENTERED | - | DONE - Waypoint and cell-based detection implemented |
| ATTACKED | - | DONE - Objects with triggers fire event when damaged |
| DESTROYED | - | DONE - Objects with triggers fire event when killed |
| CREDITS | 1506 | No credit threshold check - need to compare player credits to param2 |
| DISCOVERED | 1514 | No fog reveal tracking - need to track when cells are revealed |
| HOUSE_DISC | 1519 | No house discovery tracking - need to track when enemy units first seen |

### Stubbed Trigger Actions (mission.cpp)

| Action | Line | What's Missing |
|--------|------|----------------|
| BEGIN_PROD | 1571 | AI production not started - need to enable AI build queue |
| DESTROY_TEAM | 1589 | Team removal not implemented |
| DZ | 1610 | Drop zone flare not implemented |
| FIRE_SALE | 1616 | AI sell-all not implemented |
| TEXT | 1627 | Mission text not displayed - need UI overlay |
| AUTOCREATE | 1641 | Auto team creation not enabled |
| START_TIMER | 1680 | Timer UI not implemented |
| STOP_TIMER | 1695 | Timer UI not implemented |
| DESTROY_OBJ | 1706 | Trigger-object link not tracked |

### Simplified Implementations

| Area | File | What's Simplified |
|------|------|-------------------|
| Infantry scatter | units.cpp | No scatter on nearby explosions |
| Armor types | combat.cpp | All units use same armor class |
| Weapon ranges | weapon_types.cpp | Simplified range calculations |
| Team formations | ai.cpp | No formation movement |
| Threat assessment | ai.cpp | Basic nearest-enemy targeting |
| Build prerequisites | building_types.cpp | No tech tree validation |
| Naval movement | units.cpp | Uses ground pathfinding |
| Aircraft movement | units.cpp | Uses ground pathfinding |

---

## Completed Tiers (Reference)

### TIER 0-7: ✓ COMPLETE

| Tier | Description | Status |
|------|-------------|--------|
| 0 | Real terrain loading | ✓ DONE |
| 1 | Mission logic loop | ✓ DONE |
| 2 | Content recognition | ✓ DONE |
| 3 | Visual correctness | ✓ DONE |
| 4 | Audio (bugs remain) | PARTIAL |
| 5 | Mission fidelity (stubs remain) | PARTIAL |
| 6 | UI polish | ✓ DONE |
| 7 | Tech debt | ✓ DONE |

### What Works

- Metal renderer 60 FPS
- Terrain from theater MIX files
- Sprites from conquer.mix
- A* pathfinding (ground units)
- Combat system (basic)
- Menu system
- Sidebar UI
- Production/placement
- Tech tree (simplified)
- Economy
- AI opponent (basic)
- Fog of war
- Attack commands
- Campaign flow
- VQA playback
- Background music (distorted)
- Trigger parsing and basic evaluation
- Global flags (SET_GLOBAL, CLEAR_GLOBAL, GLOBAL_SET, GLOBAL_CLR)
- FORCE_TRIG action

### Recently Completed

| Date | Description |
|------|-------------|
| Dec 2 | VIS-1: Sprite color remapping (team colors) |
| Dec 2 | BUG-07: Map centering on player start (viewport fix) |
| Dec 2 | BUG-08: Units not moving (unit types array incomplete) |
| Dec 2 | Complete g_unitTypes array (50 types with proper speeds) |
| bba4184 | Complete unit/building types (E4, E7, C1-C10, civilians, fakes) |
| | Global flags for trigger inter-communication |
| | FORCE_TRIG action for trigger chaining |
| | TR-1: CREATE_TEAM, TR-2: REINFORCE, TR-3: ALL_HUNT |
| | EV-1: ENTERED, EV-2: ATTACKED, EV-3: DESTROYED |

---

## Known Bugs

| ID | Issue | Severity | Status |
|----|-------|----------|--------|
| BUG-01 | Music heavily distorted | High | OPEN |
| BUG-02 | Video audio has static | High | OPEN |
| BUG-07 | Map not centered on player start | High | FIXED |
| BUG-08 | Some units won't move | High | FIXED |
| BUG-09 | No target cursor on enemy hover | Medium | OPEN |

**BUG-09 details:** In the original game, hovering over enemy units with
friendly units selected shows a target/attack cursor. Currently the cursor
doesn't change based on what's under it. Needs cursor state management based
on hover target and selection context.

---

## Recommended Next Step

**VIS-2: Selection Box Visibility** - 1 hour, MEDIUM leverage

Make unit selection boxes more visible. Currently selection is indicated but
could be clearer, especially in busy combat situations.

Alternatively, work on **TR-4: BEGIN_PROD** to enable AI production, which
would make missions more challenging and dynamic.
