# IPX Directory Research

## Overview

**Purpose:** IPX/SPX networking protocol implementation for multiplayer

**Files:** 52 total

**Status:** OBSOLETE - IPX is a dead protocol, not supported on modern systems

---

## What is IPX?

IPX (Internetwork Packet Exchange) was Novell NetWare's network protocol, popular for LAN gaming in the 1990s:
- Low-latency, connectionless protocol
- No configuration needed (auto-discovery)
- Popular for DOS games (DOOM, Duke3D, C&C)
- Superseded by TCP/IP

---

## Directory Structure

### OK/ - Working Implementation
Contains verified, tested IPX code.

**Subdirectories (27!)** - Various protocol-specific implementations for different DOS extenders and configurations.

### OLD/ - Deprecated Code
Legacy implementations kept for reference.

---

## Key Files

| File | Purpose |
|------|---------|
| IPX.CPP | Main IPX interface |
| IPX95.CPP | Windows 95 IPX |
| IPXADDR.CPP | IPX addressing |
| IPXCONN.CPP | Connection management |
| IPXGCONN.CPP | Game connections |
| IPXMGR.CPP | IPX manager |
| IPXPROT.ASM | Protocol assembly |
| IPXREAL.ASM | Real mode wrapper |

---

## Why Not Port This

1. **IPX is dead** - No modern OS supports it
2. **TCP/IP is universal** - Works everywhere
3. **Modern alternatives exist:**
   - TCP/UDP sockets (standard)
   - Steam networking
   - EOS (Epic Online Services)
   - Custom matchmaking

---

## What to Reuse

The CODE/ directory has useful networking abstractions:
- `SESSION.CPP` - Session management
- `COMQUEUE.CPP` - Command queue (deterministic lockstep)
- `COMBUF.CPP` - Communication buffers
- `PACKET.CPP` - Packet handling

These are protocol-independent and should be ported.

---

## Modern Networking Strategy

For macOS port:
1. **Local multiplayer:** UDP broadcast for LAN discovery
2. **Internet:** TCP for reliable, UDP for low-latency
3. **Protocol:** Keep deterministic lockstep model
4. **Optional:** Integrate with game services

---

## Recommendation

**DO NOT PORT IPX**

Instead:
1. Create new TCP/UDP networking layer
2. Reuse session/queue abstractions from CODE/
3. Maintain lockstep synchronization model
4. Add modern features (NAT traversal, matchmaking)

The IPX directory is reference material only.
