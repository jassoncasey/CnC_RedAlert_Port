# Code

## Language Selection (SHOULD)
- Rust: Systems programming, safety-critical
- Go: Services, CLIs, networked applications
- Python: Scripting, prototyping, data work
- C/C++: Only when required (legacy, hardware)

## Function Size (SHOULD)
- Prefer functions under 10 lines
- Decomposition forces composability
- Exceptions allowed when simpler than splitting

## Purity (SHOULD)
- Prefer pure functions
- Constrain side effects to explicit boundaries
- Enables testability and reasoning

## Comments (SHOULD)
- Comments explain why, not what
- Code should be self-documenting
- Comment: non-obvious logic, business rules, workarounds
- Delete outdated comments, fix bad names instead

## Naming (SHOULD)
- Follow language idioms:
  - snake_case: Python, Ruby, Rust, C
  - camelCase: JavaScript, Java
- Descriptive over clever
- Avoid abbreviations unless standard
- Consistent within codebase

**Rationale:**
Short pure functions with clear names reduce need for
comments and enable confident refactoring.
