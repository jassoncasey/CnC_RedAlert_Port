# Tooling

## Build System (MAY)
- Prefer Make
- Flexible on alternatives per project

## Documentation (SHOULD)
- README.md required for every project
- Overview, setup instructions, usage
- MkDocs for larger documentation needs

## Version Control (SHOULD)
- Git standard
- Clear commit messages
- Meaningful history

## Versioning (SHOULD)
- Semver: MAJOR.MINOR.PATCH
- MAJOR: breaking changes
- MINOR: additive, backwards-compatible
- PATCH: bug fixes
- Maintain compatibility within major version
- Deprecate before removing

## Dependencies (SHOULD)
- Minimize dependencies
- Prefer stdlib
- Evaluate critically: maintenance, security, transitive cost
- Every dependency is a liability

**Rationale:**
Standard tools reduce friction, semver enables
dependency management, fewer deps = less risk.
