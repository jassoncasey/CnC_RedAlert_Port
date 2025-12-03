Save a session checkpoint.

Create a checkpoint file at `.claude/checkpoints/YYYY-MM-DD-HHMMSS.md`
using the current timestamp.

Write the checkpoint using this format:

```markdown
# Session Checkpoint: YYYY-MM-DD HH:MM

## Initial Goal
[What the user set out to accomplish when session started]

## Goal Evolution
[How and why the goal changed during the conversation.
Use bullet points if multiple shifts occurred, or prose if simpler.]

## Narrative Arc
[Break into phases if the session had distinct stages, or use prose
if the session was more fluid. For each phase/section cover:]
- Aim: what we were trying to do
- Research: what was explored/read
- Decisions: what was decided
- Actions: what was done

## Current State
[Where things stand now]

## Open Questions
[Unresolved items, if any]

## Next Steps
[What to do next to continue this work]

## Key Files
[Files created or modified, with brief notes]
- [path]: [created/modified] - [brief note]
```

After writing the checkpoint, report the filename and a one-line
summary of what was captured.
