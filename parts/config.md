# Configuration

## Hierarchy (SHOULD)
From least to most specific:
1. System-level config
2. User-level config
3. Local/project config
4. Command-line parameters (wins)

## Loading Order (SHOULD)
- Config path from cmdline if provided
- Otherwise search: local > user > system
- First found wins

## Secrets (MUST)
- Secrets passed through files only
- Never in command-line arguments
- Never in environment variables
- Enforce proper file permissions

## Data Formats (SHOULD)
- JSON as default structured format
- TOML for config files
- Protocol Buffers for performance-critical
- Avoid custom formats

## Serialization (SHOULD)
- Validate on input, strict parsing
- Version fields in data structures
- Schema documentation

**Rationale:**
Predictable hierarchy, secure secret handling,
standard formats enable tooling.
