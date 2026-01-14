# QB64-PE Documentation Index

## Overview

This document provides a comprehensive index to all QB64-PE documentation, helping you quickly find the information you need.

## Core Documentation

### [Architecture Documentation](ARCHITECTURE.md)
**Complete system architecture, component descriptions, and data flow**
- System overview and compilation pipeline
- Core components (compiler, runtime, build system)
- Directory structure and key technologies
- Recent architectural improvements (ADRs)
- Extension points and performance considerations

### [Getting Started Guide](GETTING_STARTED.md)
**Complete setup and development guide**
- Prerequisites and environment setup
- Quick start instructions
- Development workflow and quick reference commands
- File system critical paths and common pitfalls
- Testing overview and code quality practices
- Common development tasks and debugging
- Troubleshooting and resources


## Build and Deployment

### [Build System Documentation](build-system.md)
**Complete build process documentation, Makefile parameters, and CI process**
- Build process overview
- Out-of-source builds
- CI process and versioning
- Makefile usage and parameters
- Test build system
- Release process

### [Code Formatting and Linting](CODE_FORMATTING_AND_LINTING.md)
**Automated code quality tools and practices**
- clang-format and clang-tidy setup
- Helper scripts usage
- IDE integration
- Pre-commit hooks
- CI/CD integration

## Testing

### [Testing Implementation](testing/TESTING_IMPLEMENTATION.md)
**Comprehensive testing infrastructure details**
- Testing framework overview
- Test categories and organization
- Test discovery and reporting
- Continuous testing
- Writing and running tests

### [Component Testing Strategy](testing/COMPONENT_TESTING_STRATEGY.md)
**Strategic approach to component testing**
- Testing methodology
- Component isolation techniques
- Mock and stub strategies
- Integration with CI/CD

### [Test Consolidation Summary](testing/CONSOLIDATION_SUMMARY.md)
**Test system consolidation and improvements**
- Consolidated test framework
- Eliminated duplicate testing code
- Improved test coverage
- Standardized test practices

### [Code Coverage Analysis](CODE_COVERAGE.md)
**Code coverage analysis and reporting**
- Coverage analysis tool usage
- Report generation
- Coverage metrics
- Best practices and integration

## Specialized Documentation

### [Auto-Including System](auto-including.md)
**Automatic include file handling**
- Auto-include positions and logic
- Library dependencies
- Program structure and testing
- Implementation details

### [QB64 Wiki System](QB64_WIKI_DOCUMENTATION.md)
**Integrated help system documentation**
- Wiki architecture and components
- Character encoding support
- Entity replacements and UTF-8 handling
- Data structures and color schemes

### [QB64 Include System Analysis](QB64_INCLUDE_SYSTEM_ANALYSIS.md)
**Detailed analysis of the include system**
- Include processing logic
- Dependency resolution
- Path handling and validation
- Performance considerations

## Architecture Decisions

### [Architecture Decision Records](adr/README.md)
**Historical architectural decisions and rationale**
- ADR process and lifecycle
- Complete ADR index by category
- Recent architectural changes
- Contributing guidelines

#### Individual ADRs
- [ADR-001: Out-of-Source Builds](adr/001-out-of-source-builds.md)
- [ADR-002: Comprehensive Testing Infrastructure](adr/002-comprehensive-testing-infrastructure.md)
- [ADR-003: Code Formatting and Linting Infrastructure](adr/003-code-formatting-linting-infrastructure.md)
- [ADR-004: Security Improvements](adr/004-security-improvements-defensive-programming.md)
- [ADR-005: Memory Management and Buffer Security](adr/005-memory-management-buffer-security.md)
- [ADR-006: Error Handling API Modernization](adr/006-error-handling-api-modernization.md)
- [ADR-008: Code Quality Refactoring Framework](adr/008-code-quality-refactoring-framework.md)

## Historical and Reference

### [Refactoring Log](REFACTORING_LOG.md)
**Record of significant refactoring efforts**
- Code quality improvements
- Duplicate code elimination
- Performance optimizations
- Test coverage enhancements

### [QB64 Wiki Handbook](QB64_WIKI_HANDBOOK/)
**Complete language reference**
- [01-Introduction.md](QB64_WIKI_HANDBOOK/01-Introduction.md)
- [10-QB64-Keywords.md](QB64_WIKI_HANDBOOK/10-QB64-Keywords.md)
- [README.md](QB64_WIKI_HANDBOOK/README.md)

## Archive Documentation

### [Historical Tasks](archive/historical_tasks/)
**Archive of completed development tasks**
- [CLAUDE.md](archive/historical_tasks/CLAUDE.md)
- [CODE_REVIEW.md](archive/historical_tasks/CODE_REVIEW.md)
- [FORMAT_BAS_ERROR_ANALYSIS.md](archive/historical_tasks/FORMAT_BAS_ERROR_ANALYSIS.md)
- [PHASE1_TESTING_STATUS.md](archive/historical_tasks/PHASE1_TESTING_STATUS.md)
- [STATUS.md](archive/historical_tasks/STATUS.md)
- [documentation_added_code_review.md](archive/historical_tasks/documentation_added_code_review.md)
- [documentation_update_code_review.md](archive/historical_tasks/documentation_update_code_review.md)
- [out-of-source-builds-path-handling.md](archive/historical_tasks/out-of-source-builds-path-handling.md)

### [Problems Encountered](problems_encountered/)
**Analysis of specific technical problems**
- [qb64_console_mode_investigation.md](problems_encountered/qb64_console_mode_investigation.md)
- [qb64_goto_labels_in_included_functions.md](problems_encountered/qb64_goto_labels_in_included_functions.md)
- [qb64_main_program_structure.md](problems_encountered/qb64_main_program_structure.md)
- [test_suite_fixes.md](problems_encountered/test_suite_fixes.md)
- [windows-mkdir-directory-exists.md](problems_encountered/windows-mkdir-directory-exists.md)

## Quick Reference

### By Role

**For New Contributors:**
1. Start with [Getting Started Guide](GETTING_STARTED.md)
2. Read [Architecture Documentation](ARCHITECTURE.md) for understanding
3. Follow [Code Formatting and Linting](CODE_FORMATTING_AND_LINTING.md) for code quality

**For Compiler Development:**
1. [Architecture Documentation](ARCHITECTURE.md) - System overview
2. [Auto-Including System](auto-including.md) - Include processing
3. [Build System Documentation](build-system.md) - Build integration
4. [ADR-006](adr/006-error-handling-api-modernization.md) - Error handling patterns

**For Runtime Development:**
1. [Architecture Documentation](ARCHITECTURE.md) - Runtime library section
2. [Build System Documentation](build-system.md) - Dependency management
3. [Testing Implementation](testing/TESTING_IMPLEMENTATION.md) - Runtime testing
4. [ADR-005](adr/005-memory-management-buffer-security.md) - Memory patterns

**For Testing:**
1. [Testing Implementation](testing/TESTING_IMPLEMENTATION.md) - Framework overview
2. [Component Testing Strategy](testing/COMPONENT_TESTING_STRATEGY.md) - Testing methodology
3. [Code Coverage Analysis](CODE_COVERAGE.md) - Coverage tools
4. [Test Consolidation Summary](testing/CONSOLIDATION_SUMMARY.md) - Recent improvements

**For DevOps and CI:**
1. [Build System Documentation](build-system.md) - CI process
2. [Code Formatting and Linting](CODE_FORMATTING_AND_LINTING.md) - Quality gates
3. [ADR-001](adr/001-out-of-source-builds.md) - Build architecture
4. [ADR-002](adr/002-comprehensive-testing-infrastructure.md) - Testing automation

### By Topic

**Build System:**
- [Build System Documentation](build-system.md) - Complete reference
- [ADR-001](adr/001-out-of-source-builds.md) - Out-of-source builds

**Code Quality:**
- [Code Formatting and Linting](CODE_FORMATTING_AND_LINTING.md) - Tools and practices
- [ADR-003](adr/003-code-formatting-linting-infrastructure.md) - Quality infrastructure
- [ADR-008](adr/008-code-quality-refactoring-framework.md) - Refactoring patterns
- [Refactoring Log](REFACTORING_LOG.md) - Historical improvements

**Security:**
- [ADR-004](adr/004-security-improvements-defensive-programming.md) - Defensive programming
- [ADR-005](adr/005-memory-management-buffer-security.md) - Memory security

**Testing:**
- [Testing Implementation](testing/TESTING_IMPLEMENTATION.md) - Framework
- [Component Testing Strategy](testing/COMPONENT_TESTING_STRATEGY.md) - Strategy
- [Code Coverage Analysis](CODE_COVERAGE.md) - Coverage
- [ADR-002](adr/002-comprehensive-testing-infrastructure.md) - Infrastructure

**Architecture:**
- [Architecture Documentation](ARCHITECTURE.md) - Complete system
- [ADR README](adr/README.md) - Decision records
- All individual ADRs for specific decisions

## Navigation Tips

1. **Use this index** to find the right document quickly
2. **Check related documents** links at the bottom of each page
3. **Follow the ADR process** for architectural changes
4. **Keep documentation updated** when making changes
5. **Archive old content** to maintain clean structure

## Contributing to Documentation

When updating or adding documentation:

1. **Update this index** to reflect new documents
2. **Follow the established patterns** for consistency
3. **Add related document links** at the bottom of new pages
4. **Archive outdated content** instead of deleting
5. **Test document links** to ensure they work correctly

---

*This index is maintained as part of the QB64-PE documentation system. Last updated: January 2026*
