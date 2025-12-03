# Security

## Secure by Design (MUST)
- Security from initial design
- Threat modeling in architecture phase
- Not bolted on after implementation

## Secure Defaults (MUST)
- Default configuration must be secure
- Opt-in for less secure options
- Fail closed, not open
- Least privilege by default

## Language Selection (SHOULD)
- Prefer memory-safe languages: Rust, Go
- Rust for systems programming
- Go for services and CLIs

**Rationale:**
Safe languages eliminate vulnerability classes,
secure defaults prevent misconfiguration.
