# Red Alert macOS Port - Roadmap

**Goal:** Faithful recreation of original Red Alert with full 14-mission
campaigns (Allied + Soviet) playable from original game assets.

**Current State:** Phase 1 complete. UI overhaul done. Ready for full
campaign implementation.

---

## Campaign Mode Roadmap

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

## Effort Summary

| Phase | Hours | Description |
|-------|-------|-------------|
| Phase 2 | 15 hrs | Full 14+14 campaigns |
| Phase 3 | 25 hrs | Expansion content |
| Backlog | 13 hrs | Skirmish, polish |
| **Total** | **~53 hrs** | Complete faithful port |
