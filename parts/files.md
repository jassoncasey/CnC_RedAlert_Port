# File Organization

## Directory Scope (MUST)
- Work only in current directory and descendants
- Never traverse to parent directories
- Never access files outside project root
- If external access needed, ask first

## tmp/ Directory (MUST)

**Rule:** Any file Claude creates on its own initiative
(without explicit user request) goes in `tmp/`.

**Examples of tmp/ content:**
- Task lists, analysis notes
- Design drafts before approval
- Intermediate working files

**Location:** `tmp/` at project root, gitignored.

## User-Requested Files (MUST)

Files explicitly requested go where user specifies.
Approved designs can be promoted from `tmp/`.

## Decision (MUST)

```
Creating a file?
  |
  +-- User requested it? --> user-specified location
  |
  +-- Claude's initiative? --> tmp/
```

**Rationale:**
User controls project structure. Claude's working
files stay isolated until explicitly approved.
