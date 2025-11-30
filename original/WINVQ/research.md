# WINVQ Directory Research

## Overview

**Purpose:** Windows 95/NT implementation of VQ video codec

**Files:** 216 total

**Relationship to VQ/:** WINVQ is the Windows port of the DOS VQ codec, using Windows APIs instead of direct hardware access.

See `/original/VQ/research.md` for detailed VQA format documentation.

---

## Directory Structure

### VQA32/ - Windows VQA Player
Core VQA playback library adapted for Windows.

**Key Files:**
- Same algorithm files as DOS VQ
- Platform-specific rendering via GDI/DirectDraw

### VQAVIEW/ - Windows VQA Viewer Application
Standalone Windows application for viewing VQA files.

**Key Files:**
| File | Purpose |
|------|---------|
| MAIN.CPP | Application entry (WinMain) |
| MAINWIND.CPP | Main window handling |
| VQ.CPP | VQA playback integration |
| MOVIES.CPP | Movie file management |
| PAL.CPP | Windows palette management |
| WM.CPP | Window message handling |
| GAMETIME.CPP | Timing system |

### VPLAY32/ - VQ Player Utility
Command-line or minimal VQ player.

### VQM32/ - Support Library
Same utilities as DOS version, Windows-adapted.

### LIB/ - Compiled Libraries
Pre-built Windows libraries.

---

## Key Differences from DOS VQ

| Aspect | DOS VQ | WINVQ |
|--------|--------|-------|
| Graphics | Direct VGA/VESA | GDI/DirectDraw |
| Audio | HMI SOS | Windows multimedia |
| File I/O | DOS interrupts | Win32 APIs |
| Memory | DPMI | Win32 heap |
| Timing | PIT timer | Windows timers |

---

## Port Relevance

WINVQ is closer to what a macOS port needs:
- Already uses windowed rendering
- Uses OS audio APIs (not direct hardware)
- Standard file I/O

However, still Windows-specific:
- DirectDraw → Metal
- Windows multimedia → CoreAudio
- Win32 window management → AppKit

---

## Recommendation

For macOS port, prefer FFmpeg's VQA decoder over porting either VQ or WINVQ. See `/original/VQ/research.md` for rationale.
