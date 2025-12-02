# Red Alert macOS Port - Roadmap

**Goal:** Full playable game with all 44 campaign missions and skirmish mode.

**Current State:** Demo mode works. Real mission loading implemented with
simplifications. Triggers parsed but many actions stubbed.

---

## Active Work Queue

*Ordered by leverage - highest impact items first.*

### TIER A: Mission Playability (HIGH LEVERAGE)

These items block campaign missions from being completable.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **TR-1** | CREATE_TEAM action (spawn AI teams) | 4 hrs | **CRITICAL** | DONE |
| **TR-2** | REINFORCE action (spawn units at edge) | 4 hrs | **CRITICAL** | DONE |
| **TR-3** | ALL_HUNT action (AI attack mode) | 2 hrs | HIGH | DONE |
| **TR-4** | BEGIN_PROD action (AI production) | 3 hrs | HIGH | STUB |
| **TR-5** | AUTOCREATE action (auto team creation) | 2 hrs | MEDIUM | STUB |

**Why critical:** Most campaign missions use CREATE_TEAM and REINFORCE to spawn
enemy waves. Without these, missions have no enemy reinforcements and are
trivially easy or broken.

### TIER B: Trigger Event Completion (HIGH LEVERAGE)

Events that control when triggers fire.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **EV-1** | ENTERED event (unit in waypoint zone) | 3 hrs | **CRITICAL** | DONE |
| **EV-2** | ATTACKED event (trigger-linked attacked) | 2 hrs | HIGH | STUB |
| **EV-3** | DESTROYED event (trigger-linked destroyed) | 2 hrs | HIGH | STUB |
| **EV-4** | CREDITS event (player credits threshold) | 1 hr | MEDIUM | STUB |
| **EV-5** | DISCOVERED event (fog reveal) | 2 hrs | MEDIUM | STUB |
| **EV-6** | HOUSE_DISC event (house discovery) | 1 hr | LOW | STUB |

**Why critical:** ENTERED triggers ambushes, mission objectives, and scripted
events. Many missions won't progress without it.

### TIER C: UI Features (MEDIUM LEVERAGE)

User-visible features that affect gameplay clarity.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **UI-1** | Mission timer display | 2 hrs | MEDIUM | MISSING |
| **UI-2** | TEXT action (mission text messages) | 3 hrs | MEDIUM | STUB |
| **UI-3** | Radar zoom levels | 2 hrs | LOW | MISSING |
| **UI-4** | Sidebar scroll | 1 hr | LOW | MISSING |

### TIER D: Audio Bugs (MEDIUM LEVERAGE)

High annoyance but don't block gameplay.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **BUG-01** | Music ADPCM distortion | 4-8 hrs | HIGH | OPEN |
| **BUG-02** | Video audio static | 4-8 hrs | HIGH | OPEN |

### TIER E: Combat/AI Refinement (LOW LEVERAGE)

Affects realism but game is playable without.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **AI-1** | Hunt mode for units | 3 hrs | MEDIUM | MISSING |
| **AI-2** | Team tactics (formations) | 4 hrs | LOW | MISSING |
| **AI-3** | Threat assessment | 4 hrs | LOW | MISSING |
| **CB-1** | Armor types (light/medium/heavy) | 3 hrs | LOW | SIMPLIFIED |
| **CB-2** | Weapon damage modifiers | 2 hrs | LOW | SIMPLIFIED |
| **CB-3** | Scatter on explosion | 1 hr | LOW | SIMPLIFIED |

### TIER F: Additional Trigger Actions (LOW LEVERAGE)

Used by specific missions, not all.

| ID | Item | Effort | Leverage | Status |
|----|------|--------|----------|--------|
| **TR-6** | DESTROY_TEAM action | 1 hr | LOW | STUB |
| **TR-7** | FIRE_SALE action (sell all buildings) | 2 hrs | LOW | STUB |
| **TR-8** | DZ action (drop zone flare) | 2 hrs | LOW | STUB |
| **TR-9** | PLAY_MOVIE action | 2 hrs | LOW | STUB |
| **TR-10** | START_TIMER/STOP_TIMER actions | 2 hrs | LOW | STUB |
| **TR-11** | DESTROY_OBJ action | 2 hrs | LOW | STUB |

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
| ATTACKED | 1461 | No attack event tracking - need to track attacks on trigger-linked objects |
| DESTROYED | 1467 | No destruction tracking - need to track death of trigger-linked objects |
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

| Commit | Description |
|--------|-------------|
| bba4184 | Complete unit/building types (E4, E7, C1-C10, civilians, fakes) |
| | Global flags for trigger inter-communication |
| | FORCE_TRIG action for trigger chaining |

---

## Known Bugs

| ID | Issue | Severity | Status |
|----|-------|----------|--------|
| BUG-01 | Music heavily distorted | High | OPEN |
| BUG-02 | Video audio has static | High | OPEN |

---

## Recommended Next Step

**TR-1: CREATE_TEAM action** - 4 hours, CRITICAL leverage

This is the highest-impact single item. Most campaign missions use triggers
like:

```
[Trigs]
atk1=USSR,Time,90,Create Team,badat,0,1,1,0
```

Without CREATE_TEAM, these do nothing. Implementing it requires:

1. Look up team definition from parsed TeamTypes
2. Spawn each member unit at the team's origin waypoint
3. Set units to team's initial mission (Guard, Hunt, etc.)

Once CREATE_TEAM works, enemy waves will spawn and missions become playable.
