# Outstanding Tasks - QB64-PE

This document consolidates all outstanding tasks identified from markdown documentation in the QB64-PE repository.

**Generated**: 2026-01-10  
**Last Updated**: 2026-01-10  
**Source**: Review of all `.md` files in the repository

---

## Critical Priority (P0) - Immediate Action Required

### Security & Safety Issues

1. **BUG-001 / SEC-001: Unsafe sprintf Usage**
   - **Files**: `internal/c/libqb/src/qbs_str.cpp`, `qbs_val.cpp`, `qbs__tostr.cpp`, `logging/stacktrace.cpp`, `logging/logging.cpp`, `error_handle.cpp`
   - **Issue**: Multiple uses of `sprintf()` without bounds checking (13+ instances confirmed in `qbs_str.cpp` alone)
   - **Impact**: Potential buffer overflow leading to memory corruption, crashes, or security vulnerabilities
   - **Solution**: Replace all `sprintf()` calls with `snprintf()` with explicit buffer size limits
   - **Effort**: Medium
   - **Status**: ✅ **Completed** - Fixed unsafe `__mingw_sprintf` in libqb.cpp and replaced `sscanf` in qbs_val.cpp with safer `strtold`. Other files were already using safe `snprintf` or `sprintf_safe` wrappers.
   - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

---

## High Priority (P1) - Address Soon

### Bugs & Functional Issues

2. **BUG-003: Memory Allocation Without Error Checking**
   - **Files**: `internal/c/libqb/src/mem.cpp`
   - **Issue**: Several `malloc()` calls without checking for NULL return; memory leak at line 26
   - **Impact**: Crashes if memory allocation fails
   - **Solution**: Add NULL checks after all malloc/realloc calls; fix memory leak
   - **Effort**: Low
   - **Status**: ✅ **Completed** - Added proper NULL checks for all malloc/calloc calls and implemented proper initialization system for global memory structures
   - **Source**: `docs/general/CODE_ANALYSIS.md`

3. **BUG-004: Memory Leak Risk in qbs_new_descriptor**
   - **Files**: `internal/c/libqb/src/qbs.cpp` (lines 36-39)
   - **Issue**: Old buffer not freed when new buffer allocated
   - **Impact**: Memory leak; potential use-after-free bugs
   - **Solution**: Fix memory management to prevent leaks
   - **Effort**: Medium
   - **Source**: `docs/general/CODE_ANALYSIS.md`

4. **BUG-005: Potential Integer Overflow**
   - **Files**: `internal/c/libqb/src/qbs.cpp` (line 175)
   - **Issue**: Buffer size calculation could overflow
   - **Impact**: Incorrect buffer size calculation
   - **Solution**: Add overflow checks
   - **Effort**: Low
   - **Source**: `docs/general/CODE_ANALYSIS.md`

5. **BUG-006: Const Evaluation Operator Precedence Bug**
   - **Files**: `source/utilities/const_eval.bas` (line 1104)
   - **Issue**: FIXME indicates `NOT` operator precedence not handled correctly with exponentiation
   - **Impact**: Incorrect constant evaluation
   - **Solution**: Fix `PreParse` function to correctly handle `NOT` operator precedence
   - **Effort**: Medium
   - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

### Code Quality & Maintainability

6. **QUAL-001: Repetitive Error Handling Code**
   - **Files**: `internal/c/libqb/src/error_handle.cpp` (lines 228-300)
   - **Issue**: 18 nearly identical blocks for out-of-memory errors (error codes 257, 502-518)
   - **Impact**: Code bloat, maintenance burden
   - **Solution**: Refactor to use switch statement or array lookup (reduces ~50 lines to ~10)
   - **Effort**: Low
   - **Status**: ⚠️ **Open**
   - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

7. **QUAL-002: Duplicate File Path Operations**
   - **Files**: `source/utilities/file.bas`
   - **Issue**: Four functions duplicate path separator finding logic (~40 lines of duplication)
   - **Impact**: Code duplication, inconsistent logic
   - **Solution**: Create shared helper function `FindLastPathSeparator(f$, start_pos)`
   - **Effort**: Low
   - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

8. **MAINT-001: Deprecated Error Handling Variables**
   - **Files**: `internal/c/libqb/include/error_handle.h` (7 variables marked for removal)
   - **Issue**: Direct access to deprecated variables instead of API functions (31 references across 6 files)
   - **Impact**: Technical debt, harder to maintain
   - **Solution**: Audit all usages, replace with API functions, remove extern declarations
   - **Effort**: Medium
   - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

### Security Issues

9. **SEC-002: Potential Race Conditions**
   - **Files**: `internal/c/libqb/src/threading*.cpp`, `datetime.cpp`
   - **Issue**: Static variables in datetime.cpp could have race conditions; threading code needs audit
   - **Impact**: Crashes or incorrect behavior in multi-threaded programs
   - **Solution**: Audit and fix thread safety issues; protect static variables
   - **Effort**: Medium
   - **Source**: `docs/general/CODE_ANALYSIS.md`

10. **SEC-003: Potential Path Traversal**
    - **Files**: `source/utilities/file.bas`, `internal/c/libqb/src/filepath.cpp`
    - **Issue**: File path operations may not properly validate paths
    - **Impact**: Potential security vulnerability if malicious paths are processed
    - **Solution**: Add path validation to prevent `..` sequences and other path traversal attempts
    - **Effort**: Medium
    - **Source**: `docs/general/CODE_ANALYSIS.md`

---

## Medium Priority (P2) - Plan for Next Release

### Code Quality

11. **QUAL-003: Inefficient String Operations**
    - **Files**: `source/utilities/strings.bas`
    - **Issue**: `StrRemove$()` and `StrReplace$()` use inefficient string concatenation
    - **Solution**: Optimize to use more efficient string building techniques
    - **Effort**: Medium
    - **Source**: `docs/general/CODE_ANALYSIS.md`

12. **QUAL-004: Debug Code Always Compiled**
    - **Files**: Multiple (`source/qb64pe.bas`, `source/utilities/const_eval.bas`, etc.)
    - **Issue**: 47+ instances of `IF Debug THEN` and `IF CONST_EVAL_DEBUG THEN` always compiled into binary
    - **Solution**: Use conditional compilation (`#IFDEF` or build-time flags) to exclude debug code
    - **Effort**: Low-Medium
    - **Note**: QB64 may not support `#IFDEF` directly; alternative: build-time flag that sets `CONST Debug = 0`
    - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

13. **QUAL-006: Magic Numbers**
    - **Files**: Multiple
    - **Issue**: Magic numbers without named constants (e.g., `65536`, `1048576`, ASCII codes `47`, `92`)
    - **Solution**: Define named constants for magic numbers
    - **Effort**: Low-Medium
    - **Source**: `docs/general/CODE_ANALYSIS.md`

### Bugs

14. **BUG-002: Unsafe strcpy Usage**
    - **Files**: `internal/c/libqb/src/filesystem.cpp` (lines 614, 628)
    - **Issue**: `strcpy()` used without bounds checking (defensive programming improvement)
    - **Solution**: Replace with `strncpy()` with explicit size limits
    - **Effort**: Low
    - **Source**: `docs/general/CODE_ANALYSIS.md`

15. **BUG-007: Missing Error Handling in CopyFile**
    - **Files**: `source/utilities/file.bas` (lines 6-14)
    - **Issue**: Minimal error handling in `CopyFile&` function
    - **Solution**: Improve error handling to provide more specific error information
    - **Effort**: Low
    - **Source**: `docs/general/CODE_ANALYSIS.md`

16. **BUG-008: Potential Division by Zero in Hash Functions**
    - **Files**: `source/utilities/hash.bas`
    - **Issue**: Need to verify all edge cases are handled
    - **Solution**: Add explicit checks for edge cases (empty strings, very long strings)
    - **Effort**: Low
    - **Source**: `docs/general/CODE_ANALYSIS.md`

17. **BUG-009: Potential Array Bounds Overflow in REDIM Operations**
    - **Files**: `source/qb64pe.bas`, `source/utilities/type.bas`
    - **Issue**: Array resizing could theoretically overflow
    - **Solution**: Add overflow checks or use safer growth strategy
    - **Effort**: Low
    - **Source**: `docs/general/CODE_ANALYSIS.md`

18. **BUG-010: Missing NULL Check in file_log_handler Constructor**
    - **Files**: `internal/c/libqb/src/logging/handlers/fp_handler.cpp` (lines 50-56)
    - **Issue**: Missing NULL checks for `filepath` and `fopen()` return value
    - **Solution**: Add NULL checks and improve error handling
    - **Effort**: Low
    - **Source**: `docs/general/CODE_ANALYSIS.md`

### Maintainability

19. **MAINT-002: FIXME Comments Indicating Incomplete Work**
    - **Files**: Multiple (`qbs_cmem.cpp:11`, `console-only-main-thread.cpp:14`, `error_handle.cpp:217`, `miniz.c:4283`)
    - **Issue**: Multiple FIXME comments indicating incomplete work
    - **Solution**: Systematically address FIXME comments
    - **Effort**: Medium
    - **Source**: `docs/general/CODE_ANALYSIS.md`

### Architecture

20. **ARCH-001: Tight Coupling Between Components**
    - **Files**: Multiple
    - **Issue**: Tight coupling between compiler, runtime, and IDE components
    - **Solution**: Consider better separation of concerns and dependency injection
    - **Effort**: High
    - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/ARCHITECTURE.md`

21. **ARCH-002: Circular Dependencies**
    - **Files**: Multiple
    - **Issue**: Potential circular dependencies between header files and modules
    - **Solution**: Audit include dependencies and break circular references
    - **Effort**: Medium
    - **Source**: `docs/general/CODE_ANALYSIS.md`

---

## Low Priority (P3) - As Time Permits

23. **QUAL-005: Commented-Out Code**
    - **Files**: Multiple (`internal/c/libqb.cpp`, `internal/c/parts/video/font/font.cpp`)
    - **Issue**: Hundreds of lines of commented-out debug code
    - **Solution**: Remove debug-only commented code
    - **Effort**: Low
    - **Source**: `docs/general/CODE_ANALYSIS.md`, `docs/general/IMPROVEMENTS.md`

24. **QUAL-007: Inconsistent Naming Conventions**
    - **Files**: Multiple
    - **Issue**: Mix of naming conventions across codebase
    - **Solution**: Document and enforce consistent naming conventions
    - **Effort**: High
    - **Source**: `docs/general/CODE_ANALYSIS.md`

26. **PERF-003: Potential Memory Fragmentation**
    - **Files**: `internal/c/libqb/src/qbs.cpp`
    - **Issue**: Multiple allocation strategies could lead to fragmentation
    - **Solution**: Consider memory pool strategies or defragmentation routines
    - **Effort**: High
    - **Source**: `docs/general/CODE_ANALYSIS.md`

---

## Testing-Specific Tasks

### Test Coverage Expansion

27. **Priority 4: Additional Test Coverage**
    - **Source**: `docs/testing/TEST_RESULTS.md`, `docs/REFACTORING_LOG.md`
    - **Action**: Run tests individually and verify all assertions pass with valid test data, expand coverage as needed for:
      - Type system tests (validate against actual type.bi behavior)
      - Symbol table tests (expand hash test coverage)
      - Code generation tests (add more code generation scenarios)
      - String utility tests (add edge case coverage)
      - Error handling tests (add more error scenarios)
      - State variable tests (add state transition tests)
      - Build utility tests (add build scenario tests)

### Integration Testing Enhancements

28. **Phase 3: Integration Testing Enhancements (30% complete)**
    - **Source**: `docs/testing/TESTING_IMPLEMENTATION.md`
    - **Remaining Work**:
      - **Compiler State Testing**: Test symbol table state after compilation, type resolution correctness, dependency detection accuracy
      - **Multi-Stage Testing**: Test preprocessing stage output, semantic analysis results, code generation intermediate states
      - **Error Path Testing**: Comprehensive error message testing, error recovery mechanisms, edge cases in error reporting
      - **Performance Testing**: Test compilation speed, memory usage, comprehensive large file handling

### Runtime Testing Expansion

29. **Phase 4: Runtime Testing Expansion**
    - **Source**: `docs/testing/TESTING_IMPLEMENTATION.md`
    - **Remaining Work**:
      - Add Property-Based Testing for string operations, memory management, concurrent operations
      - Add Fuzz Testing for string operations, file I/O operations, network operations

### Test Infrastructure Improvements

30. **Phase 5: Test Infrastructure Improvements**
    - **Source**: `docs/testing/TESTING_IMPLEMENTATION.md`
    - **Remaining Work**:
      - Profile test infrastructure
      - Optimize hot paths
      - Enhanced automation where possible

### Known Issues

31. **VAL Function Overload Issue**
    - **Files**: `source/utilities/elements.bas` (lines 519, 522)
    - **Issue**: Compiler may not recognize `VAL` overload in test compilation context
    - **Potential Solutions**: May resolve with full compiler, compiler version update, or temporarily skip `const_eval` tests
    - **Source**: `docs/testing/TESTING_IMPLEMENTATION.md`

### Future Work (To Enable Additional Test Suites)

32. **Enable Additional Test Suites**
    - **Source**: `docs/testing/TESTING_HISTORY.md`
    - **Options**:
      - **Option A (Recommended)**: Uncomment one test suite at a time, fix compilation errors, document each step
      - **Option B**: Split `hash.bi`, `simplebuffer.bi`, `type.bi` into declaration/initialization pairs, include declarations in Phase 1, initialization in Phase 2, then uncomment all test suites
      - **Option C**: Create separate test runners for individual components, include only needed dependencies, test components in isolation

### Test Discovery Enhancements

33. **Future Enhancements for Test Discovery**
    - **Source**: `docs/testing/TEST_DISCOVERY.md`
    - **Enhancements**:
      - Test dependency tracking
      - Test execution time tracking
      - Test result caching
      - Parallel test execution based on discovery
      - Test coverage integration
      - Test metadata in test files (comments)

### Continuous Testing Enhancements

34. **Future Enhancements for Continuous Testing**
    - **Source**: `docs/testing/CONTINUOUS_TESTING.md`
    - **Enhancements**:
      - More sophisticated dependency tracking (parse includes)
      - Test result caching with hash-based change detection
      - Integration with IDE plugins
      - Performance metrics and timing information

### Component Testing Enhancements

35. **Future Enhancements for Component Testing**
    - **Source**: `docs/testing/COMPONENT_TESTING_STRATEGY.md`
    - **Enhancements**:
      - Expand test coverage (more edge cases, error conditions, performance/regression tests)
      - Integration testing (component interactions, full compilation workflows, error propagation)
      - Test maintenance and documentation

---

## Documentation-Specific Tasks

36. **Priority 5: Documentation Updates**
    - **Source**: `docs/REFACTORING_LOG.md`
    - **Files to Update**:
      1. `CLAUDE.md` - Add refactoring notes
      2. `docs/testing/TESTING_IMPLEMENTATION.md` - Document the GOTO label refactoring (see "Compilation Issues and Solutions" section)
      3. `docs/problems_encountered/qb64_main_program_structure.md` - Add notes about GOTO restrictions
      4. Create `tests/unit/TEST_RESULTS.md` - Document current test status

---

## Long-Term Architectural Improvements

37. **Future Architecture Considerations**
    - **Source**: `docs/ARCHITECTURE.md`
    - **Potential Improvements**:
      - **Abstract Syntax Tree (AST) implementation**: Very High effort, High risk
      - **LLVM Backend**: Significant refactoring required
      - **JIT Compilation**: New runtime infrastructure needed
      - **Language Server Protocol (LSP) implementation**: High effort, Medium risk
      - **Package Manager**: New infrastructure needed
      - **Parser Modularization**: High effort, Medium risk (split ~3,865-line file into logical modules)

38. **Compiler Enhancement (Long-term)**
    - **Source**: `docs/problems_encountered/qb64_main_program_structure.md`
    - **Option C**: Propose enhancement to QB64 compiler:
      - Add `$MAIN:` metacommand to explicitly mark main program section
      - Or modify implicit `END` logic to only trigger on SUB/FUNCTION in main file, not includes
      - Or add two-phase include processing (declarations first, then implementations)
    - **Note**: This is a future consideration, not an immediate task

---

## Summary Statistics

- **Total Tasks Identified**: 38
- **Completed**: 3 (MAINT-003: Missing Documentation, BUG-001/SEC-001: Unsafe sprintf Usage, BUG-003: Memory Allocation Without Error Checking)
- **Critical Priority (P0)**: 0 (1 completed)
- **High Priority (P1)**: 8
- **Medium Priority (P2)**: 12
- **Low Priority (P3)**: 3 (1 completed)
- **Testing-Specific**: 9
- **Documentation**: 1
- **Long-Term Architectural**: 2

---

## Recommended Implementation Phases

### Phase 1: Critical Fixes ✅ **COMPLETED**
1. ✅ Fix sprintf buffer overflows (P0)
2. Add memory allocation error checks (P1)
3. Fix integer overflow (P1)

### Phase 2: Quick Wins (1 week)
1. Refactor repetitive error handling (P1)
2. Consolidate file path operations (P1)
3. Fix unsafe strcpy usage (P2 - defensive programming)
4. Improve CopyFile error handling (P2)

### Phase 3: Technical Debt (2-3 weeks)
1. Migrate deprecated error handling variables (P1)
2. Fix const evaluation operator precedence (P1)
3. Address FIXME comments (P2)
4. Update documentation (P2)

### Phase 4: Code Quality (Ongoing)
1. Optimize string operations (P2)
2. Optimize debug code compilation (P2)
3. Replace magic numbers with constants (P2)
4. Remove commented-out code (P3)

### Phase 5: Testing Infrastructure (Ongoing)
1. Expand test coverage
2. Enhance integration tests
3. Add property-based and fuzz testing
4. Improve test infrastructure

### Phase 6: Architecture & Performance (Future)
1. Reduce component coupling (P2)
2. Break circular dependencies (P2)
3. Address memory fragmentation (P3)
4. Consider long-term architectural improvements

---

**Note**: This document consolidates tasks from multiple sources. For detailed information about each task, refer to the source documents listed in each entry.
