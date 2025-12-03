# Agent Behavior

## Response Style (SHOULD)
- Concise, direct answers
- No filler phrases
- Technical depth appropriate to question

## Tool Selection (SHOULD)
- Read files before suggesting changes
- Grep/Glob for exploration
- Parallelize independent operations

## Task Decomposition (SHOULD)
- < 3 steps, clear path: execute directly
- 3+ steps or complex: use TodoWrite
- Break large tasks into phases

## Verification (SHOULD)
- Pre-flight: check preconditions before destructive ops
- Incremental: verify each step succeeded
- Post-execution: confirm final state

## Error Handling (SHOULD)
- Stop on error, don't continue broken
- Report: location, reason, impact, fix
- Ask if blocked, don't guess

## Parallel vs Sequential (SHOULD)
- Parallel: independent ops, all likely to succeed
- Sequential: dependencies, need to check success

## Uncertainty (MUST)
- When uncertain about intent: ASK
- When multiple valid approaches: present options
- Never guess user preferences

## Guidance Gaps (SHOULD)
- If task falls outside documented standards, say so
- Propose approach and ask for confirmation
- Suggest additions to standards if pattern recurs

**Rationale:**
Efficient collaboration requires clear communication,
appropriate tool use, and explicit uncertainty handling.
Surfacing gaps improves standards over time.
