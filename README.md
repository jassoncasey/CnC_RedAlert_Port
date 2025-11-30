# Command & Conquer: Red Alert - macOS Port

Native macOS port of the classic Command & Conquer: Red Alert (1996).

## Current Status: Milestone 6 Complete

| Milestone | Status | Description |
|-----------|--------|-------------|
| 0. Repo Setup | Done | Directory structure, original code isolated |
| 1. Minimal Build | Done | AppKit window, Metal view, Cmd+Q quit |
| 2. Compat Layer | Done | Windows API stubs for compilation |
| 3. File I/O | Done | POSIX file abstraction |
| 4. Timing | Done | Game timer implementation |
| 5. Stub Assets | Done | Placeholder graphics/audio |
| 6. Graphics | Done | Metal renderer |
| 7. Input | Pending | Keyboard/mouse handling |
| 8. Game Loop | Pending | Core loop integration |
| 9. Rendering | Pending | Sprite/shape drawing |
| 10. Audio | Pending | CoreAudio sound effects |
| 11. Menus | Pending | Menu navigation |
| 12. Real Assets | Pending | MIX/SHP/AUD loading |
| 13. Gameplay | Pending | Playable mission |
| 14. Polish | Pending | App bundle, fullscreen |

## Target Platform

- macOS 14+ (Sonoma)
- Apple Silicon (ARM64)
- Metal graphics
- CoreAudio sound

## Building

```bash
cd macos
make
./RedAlert.app/Contents/MacOS/RedAlert
```

## Repository Structure

```
CnC_Red_Alert/
├── original/          # Original Windows source (read-only reference)
├── macos/             # macOS port
│   ├── src/           # Source code
│   ├── include/compat # Windows API compatibility headers
│   └── resources/     # App bundle resources
├── PORTING_PLAN.md    # Detailed milestone plan
└── LICENSE.md         # GPL v3
```

## Game Assets

Game assets are **not included** in this repository. The original game was released as freeware by EA in 2008. For development, we use stub assets. For gameplay, obtain assets from the freeware release.

## Deferred Features

- Networking (TCP/IP, IPX) - stubbed for later
- MIDI music - stubbed for later
- Intel x86_64 - ARM64 only for now

## License

Original source code is GPL v3 (see LICENSE.md).
