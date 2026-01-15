# Low Hanging Fruit - Quick Wins for QB64-PE

This document identifies easy improvements, simple fixes, and quick wins that can be tackled by contributors of all skill levels. These tasks require minimal context and provide immediate value to the project.

## Quick Status Summary

| Category | Tasks | Completed | In Progress |
|----------|-------|-----------|-------------|
| Code Style | 2 | 0 | 0 |
| Deprecated Removal | 2 | 0 | 0 |
| Simple Refactoring | 2 | 0 | 0 |
| Build Tools | 2 | 0 | 0 |
| Code Organization | 1 | 0 | 0 |
| **Total** | **9** | **0** | **0** |

## Task Status Tracking

| # | Task | Status | Priority | Effort |
|---|------|--------|----------|--------|
| 7 | Standardize Comment Style | NOT STARTED | Medium | 2-4 hours |
| 8 | Fix Inconsistent Variable Naming | NOT STARTED | Medium | 1-2 hours |
| 13 | Remove Deprecated `$NOPREFIX` Feature | NOT STARTED | High | 2-3 hours |
| 14 | Remove Deprecated `$MIDISOUNDFONT` Keyword | NOT STARTED | High | 1-2 hours |
| 15 | Extract Magic Numbers to Constants | NOT STARTED | Low | 30 min - 1 hour |
| 17 | Simplify Complex Conditionals | NOT STARTED | Medium | 30 min - 1 hour |
| 18 | Add Pre-commit Hooks | NOT STARTED | Medium | 2-3 hours |
| 20 | Add Code Formatting Check to CI | NOT STARTED | High | 1-2 hours |
| 24 | Organize Utility Functions | NOT STARTED | Low | 1-2 hours |

**Status Legend:**
- **NOT STARTED**: Task has not been begun
- **IN PROGRESS**: Task is currently being worked on
- **COMPLETED**: Task is finished and verified
- **BLOCKED**: Task cannot proceed due to dependencies

## Table of Contents

1. [Code Style & Consistency](#code-style--consistency)
2. [Deprecated Code Removal](#deprecated-code-removal)
3. [Simple Refactoring](#simple-refactoring)
4. [Build & Development Tools](#build--development-tools)
5. [Code Organization](#code-organization)


---

## Code Style & Consistency

### 7. Standardize Comment Style

**Status:** NOT STARTED

**Location**: Throughout codebase

**Issue**: Mixed comment styles:
- `'note:` comments in BASIC
- `//` comments in C++
- Inconsistent capitalization and formatting

**Action**: Create a style guide for comments and apply consistently. Prefer:
- `' Note:` (capitalized) for BASIC
- Doxygen-style `/** */` for C++ function documentation
- `//` for inline C++ comments

**Difficulty**: Easy
**Effort**: 2-4 hours (can be done incrementally)

---

### 8. Fix Inconsistent Variable Naming

**Status:** NOT STARTED

**Location**: Throughout codebase

**Issue**: Some variables may not follow the naming conventions outlined in `CONTRIBUTING.md`.

**Action**: Identify and fix naming inconsistencies:
- C++: `snake_case` for functions/variables, `PascalCase` for classes
- BASIC: Follow QB64 conventions

**Difficulty**: Easy
**Effort**: 1-2 hours (can be done incrementally)

---

## Deprecated Code Removal

### 13. Remove Deprecated `$NOPREFIX` Feature

**Status:** NOT STARTED (Recommended for removal)

**Location**: `source/qb64pe.bas`

**Issue**: `$NOPREFIX` is marked as deprecated with warnings:
```1020:1020:source/qb64pe.bas
    a$ = "$NOPREFIX is a deprecated feature, QB64(PE) specific keywords MUST have the underscore"
```

**Action**:
1. Verify no tests or samples use this feature
2. Remove the feature code
3. Update documentation

**Difficulty**: Medium
**Effort**: 2-3 hours

---

### 14. Remove Deprecated `$MIDISOUNDFONT` Keyword

**Status:** NOT STARTED (Recommended for removal)

**Location**: `source/qb64pe.bas:1900`

**Issue**: `$MIDISOUNDFONT` is deprecated in favor of `_MIDISOUNDBANK`:
```1900:1900:source/qb64pe.bas
            a$ = "$MIDISOUNDFONT is a deprecated keyword, use _MIDISOUNDBANK instead"
```

**Action**:
1. Check for usage in codebase
2. Remove deprecated keyword handling
3. Update any remaining references

**Difficulty**: Easy
**Effort**: 1-2 hours

---

## Simple Refactoring

### 15. Extract Magic Numbers to Constants

**Status:** NOT STARTED

**Location**: Throughout codebase

**Issue**: Magic numbers scattered throughout code make it harder to understand and maintain.

**Action**: Identify magic numbers and replace with named constants:
```cpp
// Before
if (value > 1024) { ... }

// After
const int32_t MAX_BUFFER_SIZE = 1024;
if (value > MAX_BUFFER_SIZE) { ... }
```

**Difficulty**: Easy
**Effort**: 30 minutes - 1 hour per file

---

### 17. Simplify Complex Conditionals

**Status:** NOT STARTED

**Location**: Throughout codebase, especially in `source/qb64pe.bas`

**Issue**: Some conditionals are deeply nested or overly complex.

**Action**: Refactor complex conditionals into clearer logic:
- Extract conditions into well-named boolean variables
- Break into smaller functions
- Use early returns where appropriate

**Difficulty**: Easy to Medium
**Effort**: 30 minutes - 1 hour per function

---

## Build & Development Tools

### 18. Add Pre-commit Hooks

**Status:** NOT STARTED

**Location**: `.git/hooks/` or use a tool like `pre-commit`

**Issue**: No automated checks before commits (formatting, linting, etc.).

**Action**: Set up pre-commit hooks to:
- Check code formatting
- Run basic linting
- Verify no debug code is committed
- Check for common mistakes

**Difficulty**: Medium
**Effort**: 2-3 hours

---

### 20. Add Code Formatting Check to CI

**Status:** NOT STARTED (Recommended - .clang-format exists but not enforced)

**Location**: CI configuration files

**Issue**: `.clang-format` exists but code formatting is not automatically checked in CI.

**Action**: Add a CI step that:
- Runs `clang-format --dry-run --Werror` on changed files
- Fails the build if formatting is incorrect
- Provides instructions for fixing

**Difficulty**: Easy
**Effort**: 1-2 hours

---

## Code Organization

### 24. Organize Utility Functions

**Status:** NOT STARTED

**Location**: `source/utilities/`

**Issue**: Utility functions may be scattered or poorly organized.

**Action**: Review and organize utility functions into logical groups with clear documentation.

**Difficulty**: Easy
**Effort**: 1-2 hours

---

## Summary

These tasks are organized by category and difficulty. Start with "Easy" tasks to get familiar with the codebase, then move on to more substantial improvements.

**Highest ROI Starting Points** (see `modernization-recommendations.md` for full analysis):
1. Add CI formatting check (#20) - Quick win, prevents style drift
2. Remove deprecated `$MIDISOUNDFONT` (#14) - Easy cleanup
3. Remove deprecated `$NOPREFIX` (#13) - Reduces maintenance burden
4. Add pre-commit hooks (#18) - Improves developer experience

**Total Estimated Effort for All Tasks**: ~15-25 hours (can be done incrementally by multiple contributors)

---

*Last Updated: January 2026*
*Contributors: Add your name when you complete a task!*
