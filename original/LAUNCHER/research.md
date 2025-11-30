# LAUNCHER Directory Research

## Overview

**Purpose:** Westwood game launcher application

**Files:** 47 total

---

## What the Launcher Does

1. **Game selection** - Choose which C&C game to play
2. **Configuration** - Settings before launch
3. **Patching** - Check for and apply updates
4. **Process launching** - Start the actual game

---

## Directory Structure

### Root Files

| File | Purpose |
|------|---------|
| LAUNCH.CPP | Main entry point |
| WINBLOWS.CPP/H | Windows window management |
| PROCESS.CPP/H | Process execution |
| LOADBMP.CPP | Bitmap loading |
| CONFIGFILE.CPP/H | Config file handling |
| FINDPATCH.CPP/H | Patch detection |
| PATCH.CPP/H | Patch application |

### UTIL/ Subdirectory
Utility classes:
- Stream handling
- File operations
- Debug logging
- Message boxes
- System utilities

---

## Key Components

### Window Management (WINBLOWS.*)
Windows API wrapper for launcher UI:
- CreateWindow
- Message loop
- Dialog boxes

### Process Execution (PROCESS.*)
Launches the game executable:
- CreateProcess
- Command-line arguments
- Working directory

### Configuration (CONFIGFILE.*)
INI-style configuration:
- Read/write settings
- Game options
- Last used settings

### Patching System (FINDPATCH.*, PATCH.*)
Update mechanism:
- Check version
- Download patches
- Apply updates

---

## Port Relevance

### Not Needed for macOS

The launcher is Windows-specific and serves purposes that macOS handles differently:

| Launcher Feature | macOS Equivalent |
|------------------|------------------|
| Window management | App already has window |
| Process launching | App IS the process |
| Configuration | System Preferences / in-app settings |
| Patching | App Store updates / Sparkle framework |

### What We Already Have

The macOS port already includes:
- Main menu system
- Options screen
- Direct game launch

---

## Recommendation

**DO NOT PORT** the launcher.

The macOS app bundle model doesn't need a separate launcher:
1. Double-click RedAlert.app launches directly
2. Settings are in-app (Options menu)
3. Updates via App Store or manual download

The launcher code is reference-only for understanding how Westwood structured their Windows distribution.
