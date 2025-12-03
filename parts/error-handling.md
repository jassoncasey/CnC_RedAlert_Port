# Error Handling

## Fail Fast (MUST)
- Detect errors at boundaries
- Stop execution on error
- No recovery from invalid state
- No silent failures

## Return Types (SHOULD)
- Prefer return types over exceptions
- Result/Option types (Rust) or equivalent
- Error types should be informative
- Propagate errors explicitly

**Rationale:**
Explicit errors are easier to debug, fail fast
prevents corruption and hidden state issues.
