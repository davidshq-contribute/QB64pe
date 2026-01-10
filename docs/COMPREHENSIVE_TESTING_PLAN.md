# Comprehensive Testing Strategy for QB64-PE

## Executive Summary

This document outlines the comprehensive testing strategy for QB64-PE. The primary challenge addressed is that QB64's `$INCLUDE` system copies file content directly into the source stream, making unit testing difficult. This plan improves testability while maintaining backward compatibility.

**Status**: Phase 1 Complete, Phases 2-5 Partially Implemented

## Current Testing Infrastructure Analysis

### Existing Test Frameworks

1. **QB64 Compiler Tests** (`tests/compile_tests/`)
   - Structure: `.bas` files with `.output` or `.err` expected results
   - Coverage: Language features, dependencies, error handling
   - Limitations: Integration-level only, no unit testing capability
   - **Status**: ✅ Existing and functional

2. **C++ Runtime Tests** (`tests/c/`)
   - Structure: Individual `.cpp` files with `test.h` framework
   - Coverage: `libqb` runtime functions, memory management
   - Limitations: Only tests C++ runtime, not QB64 compiler code
   - **Status**: ✅ Existing, ⚠️ Partially expanded

3. **QBasic Compatibility Tests** (`tests/qbasic_testcases/`)
   - Purpose: Verify legacy code compiles
   - Limitations: Only checks compilation, not behavior
   - **Status**: ✅ Existing and functional

4. **Format Tests** (`tests/format_tests/`)
   - Purpose: Verify code formatting functionality
   - Coverage: Limited to formatting features
   - **Status**: ✅ Existing and functional

### Current Testing Gaps

1. **No Unit Testing for QB64 Compiler Code** ⚠️ **PARTIALLY ADDRESSED**
   - ✅ Include abstraction created to enable module isolation
   - ✅ Skip includes mode implemented for testing functions without dependencies
   - ✅ Unit test framework created
   - ❌ Actual unit tests for compiler components are mostly placeholders
   - ⚠️ Strategy defined for testing components with minimal state (see COMPONENT_TESTING_STRATEGY.md)
   - ❌ Test harness infrastructure not yet implemented
   - ❌ No way to test individual compiler components (parser, type system, code generator) in isolation with real implementations

2. **Limited Integration Testing** ⚠️ **PARTIALLY ADDRESSED**
   - ✅ Some new integration tests created
   - ❌ Tests verify end-to-end compilation but don't test intermediate stages
   - ❌ No testing of compiler internal state (symbol tables, type checking, etc.)

3. **No Mock/Stub Capabilities** ✅ **ADDRESSED**
   - ✅ Include provider abstraction enables mocking file I/O
   - ✅ Memory provider allows testing without file system
   - ✅ Skip includes mode allows testing functions without processing dependencies
   - ⚠️ Stub capabilities for runtime functions not yet implemented

4. **Limited Error Case Coverage** ⚠️ **PARTIALLY ADDRESSED**
   - ✅ Some error path tests added
   - ❌ Edge cases in include resolution not fully covered
   - ❌ Comprehensive error case testing still needed

## Proposed Testing Architecture

### Phase 1: Improve Include System for Testability ✅ **COMPLETE**

**Problem**: Current `$INCLUDE` system in [`source/qb64pe.bas`](source/qb64pe.bas) (lines 2824-2832) reads files directly using file handles, making it impossible to inject test content or mock includes.

**Solution**: Refactor include processing to use an abstraction layer:

1. **Create Include Abstraction Interface** ✅ **COMPLETE**
   - ✅ Added `IncludeProvider` interface/type that can be swapped
   - ✅ Default implementation: File system (current behavior)
   - ✅ Test implementation: In-memory content provider

2. **Refactor Include Processing** ✅ **COMPLETE**
   - ✅ Extracted include resolution logic from main compiler loop
   - ✅ Created `ProcessInclude()` function that accepts content source
   - ✅ Maintained backward compatibility with existing `$INCLUDE` syntax

3. **Implementation Files**: ✅ **COMPLETE**
   - ✅ `source/utilities/include_provider.bas` - Include abstraction
   - ✅ `source/utilities/include_provider.bi` - Interface definitions
   - ✅ Modified `source/qb64pe.bas` to use abstraction

**Benefits**:
- ✅ Enables unit testing of include processing
- ✅ Allows testing include resolution without file system
- ✅ Skip includes mode allows testing functions without processing dependencies
- ✅ Maintains full backward compatibility

**Status**: ✅ **COMPLETE** - All include processing refactored to use abstraction layer, skip includes mode added

### Phase 2: Compiler Component Unit Testing ⚠️ **FRAMEWORK CREATED, TESTS NEEDED**

**Goal**: Test individual compiler components in isolation.

**Components to Test**:

1. **Parser** (`source/subs_functions/subs_functions.bas`) ❌ **NOT STARTED**
   - ❌ Test statement parsing
   - ❌ Test expression parsing
   - ❌ Test error handling
   - **Status**: Framework exists, no actual tests implemented

2. **Type System** (`source/utilities/type.bas`) ⚠️ **PARTIALLY DONE**
   - ✅ Test framework created
   - ✅ Test structure defined
   - ❌ Actual test implementations are placeholders
   - ❌ Test type inference
   - ❌ Test type conversions
   - ❌ Test type checking
   - **Status**: Test file created with placeholder tests

3. **Symbol Table** (`source/utilities/hash.bas`) ⚠️ **PARTIALLY DONE**
   - ✅ Test framework created
   - ✅ Test structure defined
   - ❌ Actual test implementations are placeholders
   - ❌ Test symbol insertion/lookup
   - ❌ Test scope resolution
   - ❌ Test hash collision handling
   - **Status**: Test file created with placeholder tests

4. **Constant Evaluation** (`source/utilities/const_eval.bas`) ⚠️ **PARTIALLY DONE**
   - ✅ Test framework created
   - ✅ Test structure defined
   - ❌ Actual test implementations are placeholders
   - ❌ Test constant folding
   - ❌ Test compile-time evaluation
   - ❌ Test error cases
   - **Status**: Test file created with placeholder tests

5. **Code Generation** (`source/emit/logging.bas`) ❌ **NOT STARTED**
   - ❌ Test C++ code emission
   - ❌ Test code structure generation
   - ❌ Test optimization application
   - **Status**: No tests created

**Testing Approach**:
- ✅ Test harness created that can compile individual modules
- ✅ Include abstraction available to provide test inputs
- ❌ Verification of outputs (symbol tables, generated code, etc.) not implemented

**Test Structure**: ✅ **CREATED**
```
tests/unit/
├── parser/                    ❌ NOT CREATED
│   ├── test_statement_parsing.bas
│   ├── test_expression_parsing.bas
│   └── test_error_handling.bas
├── type_system/               ⚠️ CREATED (placeholder tests)
│   ├── test_type_inference.bas
│   └── test_type_conversion.bas
├── symbol_table/              ⚠️ CREATED (placeholder tests)
│   └── test_hash.bas
└── const_eval/                ⚠️ CREATED (placeholder tests)
    └── test_const_eval.bas
```

**Status**: ⚠️ **PARTIALLY COMPLETE** - Framework exists, but actual test implementations needed

### Phase 3: Integration Testing Enhancements ⚠️ **PARTIALLY COMPLETE**

**Current State**: Integration tests exist but could be more comprehensive.

**Improvements**:

1. **Compiler State Testing** ⚠️ **PARTIALLY DONE**
   - ✅ Basic symbol resolution test created
   - ✅ Dependency detection test created
   - ❌ Test symbol table state after compilation (not implemented)
   - ❌ Test type resolution correctness (not implemented)
   - ❌ Test dependency detection accuracy (basic test only)

2. **Multi-Stage Testing** ❌ **NOT STARTED**
   - ❌ Test preprocessing stage output
   - ❌ Test semantic analysis results
   - ❌ Test code generation intermediate states

3. **Error Path Testing** ⚠️ **PARTIALLY DONE**
   - ✅ Basic error message test created
   - ✅ Error recovery test created
   - ❌ Comprehensive error message testing (only basic example)
   - ❌ Test error recovery mechanisms (only basic example)
   - ❌ Test edge cases in error reporting (not implemented)

4. **Performance Testing** ⚠️ **PARTIALLY DONE**
   - ✅ Basic large file test created
   - ❌ Test compilation speed (not implemented)
   - ❌ Test memory usage (not implemented)
   - ❌ Comprehensive large file handling (basic test only)

**Test Structure**: ⚠️ **PARTIALLY CREATED**
```
tests/integration/
├── compiler_state/            ⚠️ CREATED (basic tests)
│   ├── test_symbol_resolution.bas
│   └── test_dependency_detection.bas
├── error_handling/             ⚠️ CREATED (basic tests)
│   ├── test_error_messages.bas
│   └── test_error_recovery.bas
└── performance/                ⚠️ CREATED (basic test)
    └── test_large_files.bas
```

**Status**: ⚠️ **PARTIALLY COMPLETE** - Basic structure created, comprehensive testing needed

### Phase 4: Runtime Testing Expansion ⚠️ **PARTIALLY COMPLETE**

**Current State**: Basic C++ runtime tests exist.

**Improvements**:

1. **Expand Coverage** ⚠️ **PARTIALLY DONE**
   - ✅ Test files created for qbs (string system)
   - ✅ Test files created for mem (memory management)
   - ❌ Test all `libqb` modules systematically (only 2 modules have tests)
   - ❌ Test platform-specific code paths (not implemented)
   - ❌ Test edge cases and error conditions (not implemented)
   - **Missing modules**: filesystem, graphics, threading, audio, network, etc.

2. **Add Property-Based Testing** ❌ **NOT STARTED**
   - ❌ Test string operations with random inputs
   - ❌ Test memory management under various loads
   - ❌ Test concurrent operations

3. **Add Fuzz Testing** ❌ **NOT STARTED**
   - ❌ Fuzz string operations
   - ❌ Fuzz file I/O operations
   - ❌ Fuzz network operations

**Test Structure**: ⚠️ **PARTIALLY CREATED**
```
tests/c/
├── qbs/          ⚠️ CREATED (basic tests)
├── mem/           ⚠️ CREATED (basic tests)
├── filesystem/    ❌ NOT CREATED
├── graphics/      ❌ NOT CREATED
└── threading/     ❌ NOT CREATED
```

**Status**: ⚠️ **PARTIALLY COMPLETE** - Only 2 modules have basic tests, comprehensive coverage needed

### Phase 5: Test Infrastructure Improvements ⚠️ **PARTIALLY COMPLETE**

**Test Framework Enhancements**:

1. **Test Discovery** ⚠️ **PARTIALLY DONE**
   - ✅ Basic test structure in place
   - ❌ Automatic test discovery (not implemented)
   - ❌ Test categorization and tagging (not implemented)
   - ❌ Test filtering capabilities (not implemented)

2. **Test Reporting** ⚠️ **PARTIALLY DONE**
   - ✅ Enhanced test output with colors
   - ✅ Test summary generation
   - ✅ Basic HTML/text report generator created
   - ❌ Detailed test reports with coverage (not implemented)
   - ❌ HTML test reports fully functional (basic structure only)
   - ❌ CI/CD integration (not enhanced)

3. **Test Utilities** ⚠️ **PARTIALLY DONE**
   - ✅ Test fixtures and helpers (`test_utils.sh`)
   - ✅ Basic assertion helpers (in test framework)
   - ❌ Mock/stub utilities (include provider exists, but not comprehensive)
   - ❌ Advanced assertion helpers (basic ones only)

4. **Continuous Testing** ❌ **NOT STARTED**
   - ❌ Watch mode for development
   - ❌ Incremental test running
   - ❌ Parallel test execution

**Implementation**:
- ✅ Enhanced `tests/compile_tests.sh` with better reporting (via `run_tests.sh`)
- ✅ Created test utilities library (`test_utils.sh`)
- ❌ Test coverage tools (not implemented)

**Status**: ⚠️ **PARTIALLY COMPLETE** - Basic infrastructure improved, advanced features needed

## Implementation Plan

### Step 1: Include System Refactoring ✅ **COMPLETE**

**Files Modified**:
- ✅ [`source/qb64pe.bas`](source/qb64pe.bas) - Main compiler file
- ✅ Created `source/utilities/include_provider.bas` - New abstraction
- ✅ Created `source/utilities/include_provider.bi` - Interface definitions

**Changes**:
- ✅ Extracted include processing logic (lines 2765-2832)
- ✅ Created include provider abstraction
- ✅ Refactored to use abstraction
- ⚠️ Tests for include processing (framework exists, actual tests needed)

**Testing**:
- ✅ Verified existing tests still pass (backward compatibility maintained)
- ⚠️ Unit tests for include resolution (framework ready, tests needed)
- ⚠️ Test include path resolution (not implemented)
- ⚠️ Test `$INCLUDEONCE` behavior (not implemented)

**Status**: ✅ **COMPLETE** - Refactoring done, testing needed

### Step 2: Unit Testing Framework ⚠️ **FRAMEWORK COMPLETE, TESTS NEEDED**

**Create Test Infrastructure**:
- ✅ Test harness for QB64 modules
- ✅ Test assertion library
- ✅ Test runner script

**Initial Unit Tests**:
- ⚠️ Type system tests (framework created, placeholder tests only)
- ⚠️ Symbol table tests (framework created, placeholder tests only)
- ⚠️ Constant evaluation tests (framework created, placeholder tests only)
- ❌ Parser tests (not started)
- ❌ Code generation tests (not started)

**Status**: ⚠️ **PARTIALLY COMPLETE** - Framework ready, actual tests need implementation

### Step 3: Integration Test Expansion ⚠️ **PARTIALLY COMPLETE**

**Expand Existing Tests**:
- ⚠️ Compiler state verification (basic tests created)
- ❌ Multi-stage testing (not implemented)
- ⚠️ Error path coverage (basic tests created)

**New Test Categories**:
- ✅ Dependency detection tests (basic test created)
- ❌ Recompile mechanism tests (not implemented)
- ❌ State variable tests (not implemented)

**Status**: ⚠️ **PARTIALLY COMPLETE** - Basic structure created, comprehensive tests needed

### Step 4: Runtime Test Expansion ⚠️ **PARTIALLY COMPLETE**

**Systematic Coverage**:
- ⚠️ Created test for qbs module (basic tests)
- ⚠️ Created test for mem module (basic tests)
- ❌ Create test for each `libqb` module (only 2 of many modules)
- ❌ Add property-based tests (not implemented)
- ❌ Add fuzz tests (not implemented)

**Status**: ⚠️ **PARTIALLY COMPLETE** - Only 2 modules have basic tests

### Step 5: Test Infrastructure Polish ⚠️ **PARTIALLY COMPLETE**

**Enhancements**:
- ✅ Better test reporting (basic improvements)
- ✅ Test utilities library (basic utilities)
- ❌ CI/CD integration improvements (not implemented)
- ❌ Test coverage tools (not implemented)
- ❌ Advanced reporting features (basic structure only)

**Status**: ⚠️ **PARTIALLY COMPLETE** - Basic improvements made, advanced features needed

## Testing Best Practices

### For QB64 Compiler Tests

1. **Test Structure**
   - ✅ Each test in its own directory
   - ✅ Clear test names
   - ✅ Comprehensive `.output` or `.err` files

2. **Test Isolation**
   - ✅ Tests should not depend on each other
   - ✅ Clean state between tests
   - ✅ No shared resources

3. **Test Coverage**
   - ⚠️ Test happy paths (some coverage)
   - ⚠️ Test error cases (basic coverage)
   - ❌ Test edge cases (limited coverage)

### For C++ Runtime Tests

1. **Test Structure**
   - ✅ One test file per module (for modules that have tests)
   - ✅ Use `test.h` framework
   - ✅ Clear test names

2. **Test Coverage**
   - ⚠️ Test all public APIs (only 2 modules covered)
   - ❌ Test error conditions (not implemented)
   - ❌ Test platform-specific behavior (not implemented)

## Success Metrics

1. **Test Coverage**
   - ❌ Unit test coverage: >80% for critical components (framework exists, actual tests needed)
   - ⚠️ Integration test coverage: All major features (basic coverage)
   - ⚠️ Runtime test coverage: All `libqb` modules (only 2 modules have tests)

2. **Test Quality**
   - ✅ All tests pass consistently (for existing tests)
   - ✅ Tests run in reasonable time
   - ✅ Tests are maintainable

3. **Testability**
   - ✅ Components can be tested in isolation (include abstraction enables this)
   - ⚠️ Mock/stub capabilities available (include provider exists, but not comprehensive)
   - ✅ Test infrastructure is easy to use (basic framework ready)

## Risks and Mitigation

### Risk 1: Breaking Changes ✅ **MITIGATED**
- ✅ Maintained backward compatibility
- ✅ Extensive testing before changes (existing tests still pass)

### Risk 2: Performance Impact ⚠️ **NEEDS MONITORING**
- ⚠️ Profile test infrastructure (not yet done)
- ⚠️ Optimize hot paths (not yet done)

### Risk 3: Maintenance Burden ⚠️ **ONGOING**
- ✅ Keep tests simple
- ✅ Document test structure
- ⚠️ Automate where possible (basic automation exists)

## Timeline Estimate

- **Phase 1** (Include System): ✅ **COMPLETE** (2-3 weeks estimated, completed)
- **Phase 2** (Unit Testing): ⚠️ **PARTIALLY COMPLETE** (3-4 weeks estimated, framework done, tests needed)
- **Phase 3** (Integration): ⚠️ **PARTIALLY COMPLETE** (2-3 weeks estimated, basic structure done)
- **Phase 4** (Runtime): ⚠️ **PARTIALLY COMPLETE** (2-3 weeks estimated, 2 modules done)
- **Phase 5** (Infrastructure): ⚠️ **PARTIALLY COMPLETE** (1-2 weeks estimated, basic improvements done)

**Total Progress**: Approximately 30-40% complete
- ✅ Phase 1: 100% complete
- ⚠️ Phase 2: 40% complete (framework done, tests needed)
- ⚠️ Phase 3: 30% complete (basic structure done)
- ⚠️ Phase 4: 10% complete (2 of many modules)
- ⚠️ Phase 5: 50% complete (basic improvements done)

## Next Steps

### Immediate Priorities

1. **Implement Component Test Harness** (High Priority)
   - Create test state manager for component isolation (see COMPONENT_TESTING_STRATEGY.md)
   - Implement test harness infrastructure
   - Start with type system tests (easiest to isolate)
   - Then symbol table and constant evaluation tests
   - This enables testing components with real implementations without major refactoring

2. **Expand Runtime Test Coverage** (High Priority)
   - Create tests for remaining `libqb` modules:
     - filesystem
     - graphics
     - threading
     - audio
     - network
     - etc.

3. **Enhance Integration Tests** (Medium Priority)
   - Add multi-stage testing
   - Add comprehensive error path testing
   - Add compiler state verification tests

4. **Complete Test Infrastructure** (Medium Priority)
   - Implement automatic test discovery
   - Add test coverage tools
   - Enhance CI/CD integration
   - Add parallel test execution

5. **Add Advanced Testing** (Low Priority)
   - Property-based testing
   - Fuzz testing
   - Performance benchmarking

### Long-term Goals

1. Achieve >80% unit test coverage for critical components
2. Comprehensive integration test coverage for all major features
3. Complete runtime test coverage for all `libqb` modules
4. Automated test discovery and reporting
5. Continuous testing in development workflow

## Files Created/Modified

### New Files Created
- ✅ `source/utilities/include_provider.bi`
- ✅ `source/utilities/include_provider.bas`
- ✅ `tests/unit/test_framework.bi`
- ✅ `tests/unit/test_runner.bas`
- ✅ `tests/unit/type_system/test_type_system.bas`
- ✅ `tests/unit/symbol_table/test_hash.bas`
- ✅ `tests/unit/const_eval/test_const_eval.bas`
- ✅ `tests/integration/compiler_state/test_symbol_resolution.bas`
- ✅ `tests/integration/compiler_state/test_dependency_detection.bas`
- ✅ `tests/integration/error_handling/test_error_messages.bas`
- ✅ `tests/integration/error_handling/test_error_recovery.bas`
- ✅ `tests/integration/performance/test_large_files.bas`
- ✅ `tests/c/qbs.cpp`
- ✅ `tests/c/mem.cpp`
- ✅ `tests/test_utils.sh`
- ✅ `tests/test_report.sh`
- ✅ `docs/TESTING_IMPLEMENTATION.md`
- ✅ `docs/COMPREHENSIVE_TESTING_PLAN.md` (this file)

### Files Modified
- ✅ `source/qb64pe.bas` (include processing refactored)
- ✅ `tests/run_tests.sh` (enhanced with better reporting)

## Conclusion

The testing infrastructure has been significantly improved with the include provider abstraction and unit testing framework. However, the actual test implementations are still needed to achieve comprehensive coverage. The foundation is in place, but substantial work remains to fill in the placeholder tests and expand coverage across all components.

**Key Achievements**:
- ✅ Include system abstraction enables testability
- ✅ Unit testing framework ready for use
- ✅ Basic integration and runtime test structure created
- ✅ Test infrastructure improvements started

**Remaining Work**:
- ❌ Implement actual unit tests (currently placeholders)
- ❌ Expand runtime test coverage (only 2 modules done)
- ❌ Comprehensive integration testing
- ❌ Advanced test infrastructure features
- ❌ Property-based and fuzz testing
