# Game Asset Setup

This document explains how to obtain and set up game assets for the Red Alert macOS port.

## Overview

The game binary and assets are distributed separately:
- **Game Binary:** This repository (GPL v3 open source)
- **Game Assets:** Freeware release from EA (August 2008)

## Legal Status

Command & Conquer: Red Alert was released as **freeware** by Electronic Arts in August 2008. The game assets are freely redistributable. They are not included in this repository to keep the download size small.

---

## Quick Setup

### Option 1: Download from Internet Archive (Recommended)

1. Download the ISO images (~1.3 GB total):
   - [CD1 Allied Disc](https://archive.org/download/command-and-conquer-red-alert/CD1_ALLIED_DISC.ISO) (624 MB)
   - [CD2 Soviet Disc](https://archive.org/download/command-and-conquer-red-alert/CD2_SOVIET_DISC.ISO) (646 MB)

2. Mount CD1 (double-click the ISO on macOS)

3. Copy the required files:
   ```bash
   # Create asset directory
   mkdir -p ~/Library/Application\ Support/RedAlert/assets

   # Copy from mounted CD1
   cp /Volumes/CD1/INSTALL/REDALERT.MIX ~/Library/Application\ Support/RedAlert/assets/
   cp /Volumes/CD1/MAIN.MIX ~/Library/Application\ Support/RedAlert/assets/MAIN_ALLIED.MIX
   cp /Volumes/CD1/SETUP/AUD.MIX ~/Library/Application\ Support/RedAlert/assets/
   cp /Volumes/CD1/SETUP/SETUP.MIX ~/Library/Application\ Support/RedAlert/assets/
   ```

4. Mount CD2 and copy Soviet campaign:
   ```bash
   cp /Volumes/CD2/MAIN.MIX ~/Library/Application\ Support/RedAlert/assets/MAIN_SOVIET.MIX
   ```

5. Unmount the ISOs and run the game

### Option 2: Use Mounted ISOs Directly

The game can read assets directly from mounted ISOs:

1. Mount CD1_ALLIED_DISC.ISO
2. Run the game - it will detect `/Volumes/CD1/`
3. For Soviet campaign, also mount CD2

### Option 3: Development Setup

For development, place assets in the project directory:

```bash
mkdir -p assets
# Copy MIX files to ./assets/
```

---

## Asset Search Paths

The game searches for assets in this order:

| Priority | Path | Purpose |
|----------|------|---------|
| 1 | `~/Library/Application Support/RedAlert/assets/` | User installation |
| 2 | `./assets/` | Portable/adjacent to app |
| 3 | `../assets/` | Development builds |
| 4 | `/Volumes/CD1/INSTALL/` | Mounted Allied ISO |
| 5 | `/Volumes/CD2/INSTALL/` | Mounted Soviet ISO |

---

## Required Files

### Core Game Data

| File | Size | Source | Description |
|------|------|--------|-------------|
| REDALERT.MIX | 24 MB | CD1/INSTALL/ | Core rules, palettes, UI |
| AUD.MIX | 1.4 MB | CD1/SETUP/ | Menu audio |
| SETUP.MIX | 12 MB | CD1/SETUP/ | Setup graphics |

### Campaign Data

| File | Size | Source | Description |
|------|------|--------|-------------|
| MAIN_ALLIED.MIX | 434 MB | CD1/MAIN.MIX | Allied campaign |
| MAIN_SOVIET.MIX | 500 MB | CD2/MAIN.MIX | Soviet campaign |

### Optional Files

| File | Size | Source | Description |
|------|------|--------|-------------|
| SCORES.MIX | varies | CD1/ | Music tracks |
| MOVIES1.MIX | varies | CD1/ | Allied cutscenes |
| MOVIES2.MIX | varies | CD2/ | Soviet cutscenes |

---

## File Format Notes

### MIX Files

MIX files are Westwood's archive format containing multiple sub-files.

**Encrypted vs Unencrypted:**
- `REDALERT.MIX`, `MAIN_*.MIX` - Encrypted headers (RSA)
- `AUD.MIX`, `SETUP.MIX` - Unencrypted (old format)

The game handles both formats automatically.

### Contents of REDALERT.MIX

- `RULES.INI` - Game balance and unit definitions
- `*.SHP` - Sprite graphics
- `*.PAL` - Color palettes
- `CONQUER.MIX` - Nested archive with more assets
- `LOCAL.MIX` - Localized text strings
- `HIRES.MIX` / `LORES.MIX` - Resolution-specific graphics

---

## Troubleshooting

### "Assets not found" Error

1. Check that MIX files are in one of the search paths
2. Verify file permissions: `ls -la ~/Library/Application\ Support/RedAlert/assets/`
3. Try mounting the ISO directly

### "Failed to read MIX file" Error

1. The file may be corrupted - re-download the ISO
2. Check file sizes match expected values above

### ISO Won't Mount

On macOS, double-click the ISO file or use:
```bash
hdiutil attach CD1_ALLIED_DISC.ISO
```

To unmount:
```bash
hdiutil detach /Volumes/CD1
```

---

## Verification

After setup, verify your installation:

```bash
cd macos
make test_assets
```

Expected output:
```
=== MIX File Reader Test ===
Opening: ../../assets/AUD.MIX
SUCCESS: File opened
File count: 47
MIX file reader is working correctly!
=== Test Passed ===
```

---

## Alternative Sources

If Internet Archive is unavailable:

1. **CnC Communications Center:** https://cnc-comm.com/red-alert/downloads/the-game
2. **MyAbandonware:** https://www.myabandonware.com/game/command-conquer-red-alert-2xk
3. **Original CDs:** If you own the original game

All sources provide the same freeware release.

---

## Directory Structure Example

After complete setup:

```
~/Library/Application Support/RedAlert/
└── assets/
    ├── REDALERT.MIX      # 24 MB - Core data
    ├── MAIN_ALLIED.MIX   # 434 MB - Allied campaign
    ├── MAIN_SOVIET.MIX   # 500 MB - Soviet campaign
    ├── AUD.MIX           # 1.4 MB - Audio
    └── SETUP.MIX         # 12 MB - Graphics
```

Total: ~970 MB
