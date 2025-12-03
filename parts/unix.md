# Unix Philosophy

## Standard Streams (MUST)
- Read input from stdin
- Write output to stdout
- Write errors and logs to stderr
- Never mix output and error streams

## Exit Codes (MUST)
- 0 for success
- Non-zero for failure
- Document exit code meanings

## Signal Handling (SHOULD)
- Handle SIGTERM, SIGINT gracefully
- Clean up resources on signal
- Propagate signals to child processes

## Composability (SHOULD)
- Small, single-purpose tools
- Output suitable for piping
- Machine-readable output formats

## Logging (SHOULD)
- All logs to stderr
- Finite size buffers, rotating logs
- Log: state transitions, errors, security events
- No unbounded log growth

**Rationale:**
Do one thing well, integrate with ecosystem,
stderr separation enables composability.
