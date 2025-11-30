# CODE Directory Research

## Overview

**Purpose:** Main game engine source code for Command & Conquer Red Alert (1996)

**Statistics:**
- Total source files: ~520 (.CPP, .H, .ASM)
- C++ source: ~290 files
- Headers: ~242 files
- Assembly: 12 files

**Organization:** Flat directory structure with subdirectories for language support (ENG/) and Westwood Online API (WOLAPI/).

---

## Subdirectories

### ENG/ - Language Resources
- `CONQUER.TXT` - Main game text strings
- `CREDITS.TXT` - Credits text
- `DEBUG.TXT` - Debug mode text
- Subdirs: ENG/, GER/, FRE/ for localization

### WOLAPI/ - Westwood Online API
- `WOLAPI.H` (111K) - Main API header
- `CHATDEFS.H` - Chat system
- Defunct service - not needed for port

---

## Core Files by Category

### Main Entry & Game Loop

| File | Size | Description | Port Priority |
|------|------|-------------|---------------|
| CONQUER.CPP | 178K | Main game entry, game loop | CRITICAL |
| INIT.CPP | 119K | Game initialization | CRITICAL |
| STARTUP.CPP | 32K | Startup sequence | CRITICAL |

**Key Functions:**
- `Main_Game()` - Game initialization and main loop
- `Main_Loop()` - Single frame iteration
- `Call_Back()` - Maintenance callback
- `Play_Movie()` - VQ playback

### Object Class Hierarchy

```
ObjectClass (OBJECT.CPP - 131K)
  └── MissionClass (MISSION.CPP - 28K)
      └── RadioClass (RADIO.CPP)
          └── TechnoClass (TECHNO.CPP - 286K)
              ├── FootClass (FOOT.CPP - 122K)
              │   ├── InfantryClass (INFANTRY.CPP - 167K)
              │   ├── DriveClass (DRIVE.CPP - 91K)
              │   │   ├── UnitClass (UNIT.CPP - 154K)
              │   │   └── VesselClass (VESSEL.CPP)
              │   └── AircraftClass (AIRCRAFT.CPP - 179K)
              └── BuildingClass (BUILDING.CPP - 235K)
```

All object classes are **CRITICAL** for port.

### Map & Display System

| File | Size | Description | Port Priority |
|------|------|-------------|---------------|
| MAP.CPP | 101K | Map data management | CRITICAL |
| CELL.CPP | 128K | Cell/tile implementation | CRITICAL |
| DISPLAY.CPP | 191K | Rendering system | CRITICAL (needs Metal) |
| SIDEBAR.CPP | 109K | Build sidebar UI | CRITICAL |
| RADAR.CPP | 106K | Minimap display | CRITICAL |
| FINDPATH.CPP | 45K | A* pathfinding | CRITICAL |

### AI & Faction System

| File | Size | Description | Port Priority |
|------|------|-------------|---------------|
| HOUSE.CPP | 319K | Player/AI faction | CRITICAL |
| TEAM.CPP | 125K | AI team management | CRITICAL |
| TEAMTYPE.CPP | 74K | Team templates | CRITICAL |
| TRIGGER.CPP | 17K | Event triggers | CRITICAL |
| SCENARIO.CPP | 116K | Mission scenarios | CRITICAL |
| RULES.CPP | 51K | Game rules (RULES.INI) | CRITICAL |

### Networking

| File | Size | Description | Port Priority |
|------|------|-------------|---------------|
| IPX.CPP | 53K | IPX networking | NOT NEEDED (obsolete) |
| TCPIP.CPP | 35K | TCP/IP networking | REFERENCE |
| SESSION.CPP | 70K | Session management | CRITICAL |
| COMQUEUE.CPP | 40K | Command queue (lockstep) | CRITICAL |
| NETDLG.CPP | 330K | Network dialogs | NEEDS REPLACEMENT |
| WOL_*.CPP | ~400K | Westwood Online | NOT NEEDED (defunct) |

### File I/O & Assets

| File | Size | Description | Port Priority |
|------|------|-------------|---------------|
| MIXFILE.CPP | 27K | MIX archive reader | CRITICAL |
| CCFILE.CPP | 32K | C&C file wrapper | CRITICAL |
| RAWFILE.CPP | 54K | Raw file I/O | CRITICAL |
| INI.CPP | 72K | INI parsing | CRITICAL |
| CCINI.CPP | 100K | C&C INI handling | CRITICAL |

### Compression & Crypto

| File | Description | Port Priority |
|------|-------------|---------------|
| LCW*.CPP | LCW compression | CRITICAL |
| LZO*.CPP | LZO compression | USEFUL |
| LZW*.CPP | LZW compression | USEFUL |
| BLOWFISH.CPP | Encryption | USEFUL |
| SHA.CPP | SHA hash | USEFUL |
| CRC.CPP | CRC checksum | USEFUL |

### Data Tables (*DATA.CPP)

| File | Size | Content |
|------|------|---------|
| BDATA.CPP | 196K | Building definitions (largest) |
| ADATA.CPP | 100K | Aircraft data |
| CDATA.CPP | 99K | Infantry/character data |
| UDATA.CPP | 68K | Unit/vehicle data |
| IDATA.CPP | 61K | Infantry type data |
| ODATA.CPP | 52K | Overlay data |
| TDATA.CPP | 42K | Template/terrain data |
| VDATA.CPP | 40K | Vessel data |
| AADATA.CPP | 37K | Aircraft animation |
| SDATA.CPP | 32K | Smudge data |
| HDATA.CPP | 26K | House data |
| BBDATA.CPP | 18K | Bullet data |

All data files are **CRITICAL** - pure data, platform-independent.

### UI Elements

| File | Description | Port Priority |
|------|-------------|---------------|
| GADGET.CPP | Base UI widget | REFERENCE |
| DIALOG.CPP | Dialog system | REFERENCE |
| CHECKBOX/EDIT/GAUGE/LIST/SLIDER.CPP | Widgets | REFERENCE |

Windows UI - needs macOS replacement.

### Graphics & Effects

| File | Description | Port Priority |
|------|-------------|---------------|
| ANIM.CPP | Animation system | CRITICAL |
| SPRITE.CPP | Sprite rendering | CRITICAL |
| EXPAND.CPP | Shape decompression | CRITICAL |
| PALETTE.CPP | Palette management | CRITICAL |
| VORTEX.CPP | Chronosphere effect | USEFUL |
| BULLET.CPP | Projectiles | CRITICAL |

### Input

| File | Description | Port Priority |
|------|-------------|---------------|
| MOUSE.CPP | Mouse handling | NEEDS REPLACEMENT |
| KEYBOARD.CPP | Keyboard handling | NEEDS REPLACEMENT |
| KEY.CPP | Key mapping | USEFUL |

Assembly files (KEYFBUFF.ASM, 2KEYFBUF.ASM) need C++ replacement.

---

## Assembly Files (12 total)

| File | Size | Description |
|------|------|-------------|
| KEYFBUFF.ASM | 77K | Keyboard buffer |
| 2KEYFBUF.ASM | 118K | Extended keyboard |
| LCWCOMP.ASM | 8K | LCW compression |
| COORDA.ASM | 7K | Coordinate math |
| SUPPORT.ASM | 9K | Support routines |
| 2SUPPORT.ASM | 9K | More support |
| TXTPRNT.ASM | 10K | Text printing |
| 2TXTPRNT.ASM | 10K | Extended text |
| WINASM.ASM | 16K | Windows assembly |
| IPXREAL.ASM | 13K | IPX real mode |
| IPXPROT.ASM | 5K | IPX protected mode |
| CPUID.ASM | 6K | CPU detection |

**Strategy:** Rewrite in C++ or use modern libraries.

---

## Port Analysis

### Platform-Independent (~60%)
- Game logic (objects, AI, combat)
- Data structures and tables
- Pathfinding algorithms
- Scenario/rules parsing
- Compression algorithms
- Networking protocols (concepts)

### Needs Replacement (~30%)
- Graphics rendering → Metal
- Audio system → CoreAudio
- Input handling → AppKit
- File I/O → POSIX
- UI widgets → native macOS

### Obsolete (~10%)
- IPX networking
- Westwood Online
- CD-ROM code
- DOS/DPMI code
- Modem/serial

---

## Build System

**Original:** Watcom C++ (wmake)
- `Makefile` (756 lines)
- `REDALERT.IDE` - Watcom IDE project
- `RA95.PJT` - Win95 project

**Libraries:**
- win32lib.lib, wwflat32.lib
- vqa32wp.lib, vqm32wp.lib (video)
- ddraw.lib, dsound.lib (DirectX)
- wwipx32.lib (networking)

---

## Key Insights

1. **Deep OOP hierarchy** - Objects inherit 5-7 levels deep
2. **Type/Instance separation** - Static data in *TypeClass, runtime in *Class
3. **Deterministic lockstep** - Critical for multiplayer sync
4. **Layer-based rendering** - Multiple draw layers for proper overlap
5. **Event queue system** - All actions go through central queue

---

## Recommended Port Order

1. Data tables and type definitions
2. Base object classes (Object → Techno → Foot)
3. Map and cell system
4. Specific object types (Building, Unit, Infantry, Aircraft)
5. AI and mission system
6. Combat and weapons
7. Scenario loading
8. UI and display
9. Multiplayer (last)
