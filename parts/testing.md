# Testing

## Coverage (SHOULD)
- Unit tests (primary focus)
- Positive inputs (happy path)
- Negative inputs (error cases)
- Boundary conditions, edge cases

## Philosophy (SHOULD)
- Test behavior, not implementation
- Each test validates one thing
- Clear test names
- Fast, isolated, repeatable

## Style (MAY)
- Prefer table-driven/parameterized tests
- Input/output pairs in data structure
- Single test iterates over table

**Rationale:**
Comprehensive coverage catches bugs early,
table-driven tests reduce boilerplate.
