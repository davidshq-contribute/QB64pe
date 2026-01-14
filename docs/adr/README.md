# Architecture Decision Records (ADRs)

This directory contains Architecture Decision Records (ADRs) for QB64pe. ADRs capture important architectural decisions made during the development of QB64pe, providing context, rationale, and consequences for each decision.

## What is an ADR?

An Architecture Decision Record (ADR) is a document that captures an important architectural decision made along with its context and consequences. ADRs help teams:

- Document why decisions were made
- Provide context for future developers
- Track architectural evolution
- Enable informed decision-making
- Maintain consistency in architectural choices

## ADR Format

Each ADR follows this structure:

- **Status**: Accepted, Proposed, Deprecated, or Superseded
- **Context**: What problem was being addressed
- **Decision**: What was decided and how it was implemented
- **Consequences**: Positive, negative, and risk assessment
- **Alternatives Considered**: Other options that were evaluated
- **References**: Related commits, issues, and documentation

## ADR Index

| ADR | Title | Status | Date |
|-----|-------|---------|------|
| [ADR-001](001-out-of-source-builds.md) | Out-of-Source Builds Implementation | Accepted | 2026-01-11 |
| [ADR-002](002-comprehensive-testing-infrastructure.md) | Comprehensive Testing Infrastructure Implementation | Accepted | 2026-01-11 |
| [ADR-003](003-code-formatting-linting-infrastructure.md) | Code Formatting and Linting Infrastructure | Accepted | 2026-01-11 |
| [ADR-004](004-security-improvements-defensive-programming.md) | Security Improvements - Defensive Programming Implementation | Accepted | 2026-01-12 |
| [ADR-005](005-memory-management-buffer-security.md) | Memory Management and Buffer Security Implementation | Accepted | 2026-01-13 |
| [ADR-006](006-error-handling-api-modernization.md) | Error Handling API Modernization | Accepted | 2026-01-13 |
| [ADR-008](008-code-quality-refactoring-framework.md) | Code Quality and Refactoring Framework | Accepted | 2026-01-13 |

## ADR Categories

### Build System
- **ADR-001**: Out-of-source builds for clean separation of source and build artifacts

### Testing Infrastructure
- **ADR-002**: Comprehensive multi-tier testing system with continuous testing

### Development Tools
- **ADR-003**: Automated code formatting and linting infrastructure

### Security and Quality
- **ADR-004**: Defensive programming improvements for security and stability
- **ADR-005**: Memory management and buffer security implementation

### Architecture and Code Quality
- **ADR-006**: Error handling API modernization with 13 new API functions
- **ADR-008**: Code quality and refactoring framework with systematic patterns

## Recent Architectural Changes

### January 2026 Architecture Improvements

The following major architectural changes were implemented by Dave Mackey (davidshq) in January 2026:

1. **Out-of-Source Builds** (ADR-001)
   - Moved all C/C++ build artifacts to dedicated build directories
   - Implemented cross-platform path handling helpers
   - Enabled parallel builds with TEMP_ID support

2. **Testing Infrastructure** (ADR-002)
   - Added comprehensive multi-tier testing system
   - Implemented unit tests, integration tests, and C++ runtime tests
   - Created continuous testing and test discovery capabilities

3. **Code Quality Tools** (ADR-003)
   - Added clang-format, clang-tidy, and clangd configuration
   - Implemented cross-platform automation scripts
   - Integrated with modern IDEs and CI/CD pipelines

4. **Security Improvements** (ADR-004)
   - Replaced unsafe strcpy() calls with strncpy()
   - Added memory allocation error checking
   - Implemented defensive programming patterns

5. **Memory Management and Buffer Security** (ADR-005)
   - Implemented comprehensive memory allocation validation patterns
   - Added buffer security framework with bounds checking
   - Created dedicated memory security testing infrastructure
   - Standardized memory safety patterns across codebase

6. **Error Handling API Modernization** (ADR-006)
   - Replaced 31 direct variable references with 13 modern API functions
   - Implemented type-safe error state management
   - Added comprehensive error handling with 166 tests (100% pass rate)
   - Eliminated all deprecated error handling variables

7. **Test Infrastructure Implementation** (ADR-007)
   - Created comprehensive testing framework with 73 tests across 10 suites
   - Eliminated 12 GOTO labels across 3 files for improved testability
   - Implemented cross-platform test automation with wrapper scripts
   - Added continuous testing and test discovery capabilities

8. **Code Quality and Refactoring Framework** (ADR-008)
   - Eliminated 90+ lines of duplicate code through systematic refactoring
   - Implemented automated quality tools (clang-format, clang-tidy, clangd)
   - Created comprehensive refactoring patterns and best practices
   - Added cross-platform code quality automation scripts

## ADR Process

### Creating New ADRs

1. **Identify Need**: When a significant architectural decision is needed
2. **Draft ADR**: Create ADR following the standard format
3. **Review**: Get feedback from team members
4. **Accept**: Mark status as "Accepted" when implemented
5. **Reference**: Update related documentation and code comments

### ADR Lifecycle

- **Proposed**: Initial draft for discussion
- **Accepted**: Decision made and implemented
- **Deprecated**: Decision no longer relevant
- **Superseded**: Replaced by newer decision

## Contributing to ADRs

When contributing to QB64pe:

1. **Review Existing ADRs**: Understand current architectural decisions
2. **Propose New ADRs**: For significant architectural changes
3. **Update ADRs**: When decisions evolve or new information emerges
4. **Reference ADRs**: In code comments and documentation

## Related Documentation

- [Architecture Overview](../ARCHITECTURE.md)
- [Build System Documentation](../build-system.md)
- [Testing Documentation](../testing/)
- [Code Formatting and Linting](../CODE_FORMATTING_AND_LINTING.md)
- [Getting Started Guide](../GETTING_STARTED.md)

## Contact

For questions about ADRs or architectural decisions:

- **Repository**: [QB64pe GitHub](https://github.com/davidshq-contribute/QB64pe)
- **Issues**: Use GitHub Issues for architectural discussions
- **Discussions**: GitHub Discussions for architectural proposals

---

*ADRs are living documents that evolve with the project. They represent the collective wisdom and decision-making process of the QB64pe development team.*
