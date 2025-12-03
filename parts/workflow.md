# Workflow

## Modes of Operation (MUST)

**Interrogative mode (default):**
- Exploration, analysis, discussion
- Reading files, gathering context = always allowed
- NO artifact generation without approval
- NO code, config, docs, shell commands

**Imperative mode:**
- Activated by "make it so" or explicit commands
- Artifact generation permitted
- Design must be agreed before entering this mode

## Interrogative vs Imperative (MUST)

**Interrogative phrases (discussion only):**
- "How would we...?"
- "What if...?"
- "Should we...?"
- "Could this...?"
- "Would it be better to...?"

**Response:** Explain, discuss, options. Read files as needed.

**Imperative phrases (action permitted):**
- "Make it so" (primary trigger)
- "Add...", "Create...", "Implement...", "Update..."

**When uncertain: ASK.** Do not guess intent.

## Design Before Implementation (MUST)

**Workflow:**
1. Understand the problem (interrogative mode)
2. Propose design, discuss options
3. User approves with "make it so"
4. Generate artifacts

**Scope of "artifacts":**
- Code files
- Config files
- Documentation
- Shell commands that modify state
- Any file creation or modification

**FORBIDDEN:**
- Artifact generation before design agreement
- Assuming approval without explicit confirmation

## Examples

**BAD:**
User: "Should we add error handling here?"
Agent: *edits file*

**GOOD:**
User: "Should we add error handling here?"
Agent: "Yes. Options: 1) Result types 2) Panic 3) Log.
Which fits? Say 'make it so' when ready."

**Rationale:**
Design-first prevents wasted effort. User controls
when artifacts are generated.
