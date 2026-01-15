# Low Hanging Fruit - Quick Wins for QB64-PE

This document identifies easy improvements, simple fixes, and quick wins that can be tackled by contributors of all skill levels. These tasks require minimal context and provide immediate value to the project.

## Table of Contents

2. [Documentation Improvements](#documentation-improvements)
3. [Code Style & Consistency](#code-style--consistency)
4. [Testing & Quality Assurance](#testing--quality-assurance)
5. [Deprecated Code Removal](#deprecated-code-removal)
6. [Simple Refactoring](#simple-refactoring)
7. [Build & Development Tools](#build--development-tools)


---

## Code Style & Consistency

### 7. Standardize Comment Style

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
**Estimated Time**: 2-4 hours (can be done incrementally)

---

### 8. Fix Inconsistent Variable Naming

**Location**: Throughout codebase

**Issue**: Some variables may not follow the naming conventions outlined in `CONTRIBUTING.md`.

**Action**: Identify and fix naming inconsistencies:
- C++: `snake_case` for functions/variables, `PascalCase` for classes
- BASIC: Follow QB64 conventions

**Difficulty**: Easy  
**Estimated Time**: 1-2 hours (can be done incrementally)

---

## Deprecated Code Removal

### 13. Remove Deprecated `$NOPREFIX` Feature

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
**Estimated Time**: 2-3 hours

---

### 14. Remove Deprecated `$MIDISOUNDFONT` Keyword

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
**Estimated Time**: 1-2 hours

---

## Simple Refactoring

### 15. Extract Magic Numbers to Constants

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
**Estimated Time**: 30 minutes - 1 hour per file


---

### 17. Simplify Complex Conditionals

**Location**: Throughout codebase, especially in `source/qb64pe.bas`

**Issue**: Some conditionals are deeply nested or overly complex.

**Action**: Refactor complex conditionals into clearer logic:
- Extract conditions into well-named boolean variables
- Break into smaller functions
- Use early returns where appropriate

**Difficulty**: Easy to Medium  
**Estimated Time**: 30 minutes - 1 hour per function

---

## Build & Development Tools

### 18. Add Pre-commit Hooks

**Location**: `.git/hooks/` or use a tool like `pre-commit`

**Issue**: No automated checks before commits (formatting, linting, etc.).

**Action**: Set up pre-commit hooks to:
- Check code formatting
- Run basic linting
- Verify no debug code is committed
- Check for common mistakes

**Difficulty**: Medium  
**Estimated Time**: 2-3 hours

---

### 20. Add Code Formatting Check to CI

**Location**: CI configuration files

**Issue**: Code formatting may not be automatically checked in CI.

**Action**: Add a CI step that:
- Checks code formatting
- Fails the build if formatting is incorrect
- Provides instructions for fixing

**Difficulty**: Easy  
**Estimated Time**: 1 hour

---

## Code Organization

### 24. Organize Utility Functions

**Location**: `source/utilities/`

**Issue**: Utility functions may be scattered or poorly organized.

**Action**: Review and organize utility functions into logical groups with clear documentation.

**Difficulty**: Easy  
**Estimated Time**: 1-2 hours

---

## Summary

These tasks are organized by category and difficulty. Start with "Very Easy" and "Easy" tasks to get familiar with the codebase, then move on to more substantial improvements.

**Recommended Starting Points**:
1. Fix FIXME comment (#1)
2. Remove commented-out debug code (#2)
3. Add file header comments (#25)
4. Improve test documentation (#12)
5. Add function documentation (#4)

**Total Estimated Time for All Tasks**: ~50-80 hours (can be done incrementally by multiple contributors)

---

*Last Updated: [Current Date]*  
*Contributors: Add your name when you complete a task!*
