# TOOLS Directory Research

## Overview

**Purpose:** Build tools and content pipeline utilities for Red Alert development

**Files:** 25 total (mostly compiled executables)

---

## Available Tools

### Content Creation

| Tool | Purpose |
|------|---------|
| ANIMATE.EXE | Animation compilation |
| FONTMAKE.EXE | Font file creation |
| ICONCOMP.EXE | Icon compilation |
| ICONMAP.EXE | Icon mapping |
| KEYFRAME.EXE | Keyframe animation |
| MAP2MAP.EXE | Map format conversion |

### Audio

| Tool | Purpose |
|------|---------|
| AUDIOMAK.EXE | Audio compilation |

**AUDIOMAK/ subdirectory** - Audio toolchain source

### Archive Management

| Tool | Purpose |
|------|---------|
| MIXFILE.EXE | MIX archive creation/extraction |
| WWCOMP.EXE | Westwood compression |
| WWPACK.EXE | Westwood packing |

**MIX/ subdirectory** - MIX tool source

### Utilities

| Tool | Purpose |
|------|---------|
| FILEDATA.EXE | File data extraction |
| GETREG.EXE | Registry reading |

---

## Subdirectories

### AUDIOMAK/
Audio compilation tool source code.
- Converts raw audio to AUD format
- Applies Westwood compression

### MIX/
MIX archive tool source code.
- Creates MIX files from loose files
- Extracts files from MIX archives
- Handles CRC-based file lookup

---

## Port Relevance

### Useful for Development
- **MIXFILE** - Understanding MIX format helps with asset loading
- **MIX/ source** - Reference for implementing MIX reader

### Not Needed for Runtime
All tools are content pipeline - not needed in shipping game.

### Modern Alternatives
For modding support, could create new tools:
- MIX extractor (already have reader in macOS port)
- Asset converter (SHP → PNG, etc.)
- Map editor (would be separate app)

---

## MIX File Format

MIX archives use CRC-based lookup:
```c
struct MIXHeader {
    uint16_t numFiles;
    uint32_t dataSize;
};

struct MIXEntry {
    uint32_t crc;      // CRC of filename
    uint32_t offset;   // Offset in data section
    uint32_t size;     // File size
};
```

Files are identified by CRC of their name, not the name itself.

---

## Recommendation

The macOS port already has MIX file loading implemented.

Tools are reference material for understanding asset formats. No need to port the tools themselves unless creating modding support.
