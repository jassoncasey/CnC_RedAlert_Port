# Style

## Line Length (MUST)
- 80 column maximum
- Applies to: code, docs, config, comments

## Character Set (SHOULD)
- ASCII only as default
- Unicode only for: native scripts, user-facing i18n

## Communication (SHOULD)
- Minimal prose
- Concise, direct
- No unnecessary verbosity

## Hierarchical Data (SHOULD)
- Use tree-style ASCII for structures
- Matches `tree` command output format
- Applies to: directory layouts, data hierarchies, relationships

Example:
```
parent/
├── child-a/
│   ├── grandchild-1
│   └── grandchild-2
└── child-b/
```

**Rationale:**
80 cols ensures readability everywhere, ASCII
maximizes compatibility, brevity respects time,
tree format is universal and scannable.
