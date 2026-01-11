# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

#### Documentation
- **Documentation Reorganization**
  - Added `OUTSTANDING_TASKS.md` - Consolidated task tracking from all markdown documentation
  - Added `docs/problems_encountered/documentation_added_code_review.md` - Code review documentation for documentation additions
  - Added `docs/problems_encountered/documentation_update_code_review.md` - Code review documentation for documentation updates
  - Added `docs/testing/TESTING_HISTORY.md` - Comprehensive testing history and evolution
  - Added `docs/testing/TEST_RESULTS.md` - Test results documentation
  - Added `tests/unit/TEST_RESULTS.md` - Unit test results documentation

#### Testing Infrastructure
- **Component Test Harness**
  - Added `tests/unit/test_state_manager.bi` - Test state manager for component isolation
  - Added `tests/unit/test_component_utils.bi` - Component test utilities and helpers
  - Added `tests/unit/test_compiler_context.bi` - Minimal compiler context for component testing
  - Added `docs/COMPONENT_TEST_HARNESS_IMPLEMENTATION.md` - Comprehensive documentation for component test harness

#### Documentation
- **Code Analysis Documentation**
  - Added `CODE_ANALYSIS_ISSUE_DATABASE.md` - Comprehensive database of code analysis issues
  - Added `CODE_ANALYSIS_MASTER_INVENTORY.md` - Master inventory of codebase components
  - Added `CODE_ANALYSIS_SUMMARY_REPORT.md` - Summary report of code analysis findings
  - Added `docs/general/CODE_ANALYSIS.md` - Consolidated code analysis findings with issue tracking and recommendations
  - Added `docs/general/IMPROVEMENTS.md` - Low-hanging fruit improvements with priority rankings

- **Architecture Documentation**
  - Added `docs/ARCHITECTURAL_REVIEW.md` - Detailed architectural review of the codebase
  - Added `docs/ARCHITECTURE.md` - Comprehensive architecture documentation
  - Added `docs/DEAD_CODE.md` - Documentation of dead/unused code identification
  - Added `docs/GETTING_STARTED.md` - Getting started guide for developers
  - Added `docs/IMPROVEMENTS.md` - Documented improvement opportunities

- **Testing Documentation**
  - Added `docs/COMPONENT_TESTING_STRATEGY.md` - Strategy for component-level testing
  - Added `docs/COMPREHENSIVE_TESTING_PLAN.md` - Comprehensive testing plan and methodology
  - Added `docs/TESTING_IMPLEMENTATION.md` - Testing implementation guidelines
  - Added `docs/CODE_REVIEW_COMBINED.md` - Combined test infrastructure review with status of improvements
  - Added `docs/testing/CONTINUOUS_TESTING.md` - Documentation for continuous testing features
  - Added `docs/testing/TEST_DISCOVERY.md` - Test discovery system documentation

#### Source Code
- **Include Provider Utility**
  - Added `source/utilities/include_provider.bas` - New include provider implementation
  - Added `source/utilities/include_provider.bi` - Include provider header/interface definitions

- **Three-Phase Include System Support**
  - Added `source/utilities/hash_declarations.bi` - Hash table declarations for three-phase include system
  - Added `source/utilities/hash_init.bas` - Hash table initialization code
  - Added `source/utilities/s-buffer/simplebuffer_declarations.bi` - Simple buffer declarations
  - Added `source/utilities/s-buffer/simplebuffer_init.bas` - Simple buffer initialization code
  - Added `source/utilities/type_declarations.bi` - Type system declarations
  - Added `source/utilities/type_init.bas` - Type system initialization code

#### Testing Infrastructure
- **Test Framework**
  - Added `tests/unit/test_framework.bi` - Unit testing framework header/interface
  - Added `tests/unit/test_runner.bas` - Test runner implementation
  - Added `tests/test_report.sh` - Test reporting script
  - Added `tests/test_utils.sh` - Test utility functions
  - Added `tests/unit/run_tests.sh` - Bash wrapper script for cross-platform test execution (Linux/Mac/Git Bash)
  - Added `tests/unit/run_tests.bat` - Windows batch wrapper script for test execution
  - Added `tests/unit/run_tests_wsl.sh` - WSL test runner for fully automated testing on Windows
  - Added `tests/unit/run_tests_wsl.bat` - Windows wrapper for WSL test runner
  - Added `tests/unit/README.md` - Comprehensive test documentation and usage guide
  - Added `tests/unit/WSL_SETUP.md` - WSL setup guide for automated testing on Windows

- **C Test Utilities**
  - Added `tests/c/mem.cpp` - Memory testing utilities
  - Added `tests/c/qbs.cpp` - QBS (QB64 String) testing utilities

#### Unit Tests
- **Constant Evaluation Tests**
  - Added `tests/unit/const_eval/test_const_eval.bas` - Tests for constant evaluation functionality
  - Updated to use component test harness for isolated testing

- **Symbol Table Tests**
  - Added `tests/unit/symbol_table/test_hash.bas` - Tests for hash functionality in symbol tables
  - Updated to use component test harness with proper state management

- **Type System Tests**
  - Added `tests/unit/type_system/test_type_system.bas` - Tests for type system functionality
  - Updated to use component test harness for isolated testing

#### Integration Tests
- **Compiler State Tests**
  - Added `tests/integration/compiler_state/test_dependency_detection.bas` - Tests for dependency detection
  - Added `tests/integration/compiler_state/test_dependency_detection.output` - Expected output for dependency detection tests
  - Added `tests/integration/compiler_state/test_symbol_resolution.bas` - Tests for symbol resolution
  - Added `tests/integration/compiler_state/test_symbol_resolution.output` - Expected output for symbol resolution tests

- **Error Handling Tests**
  - Added `tests/integration/error_handling/test_error_messages.bas` - Tests for error message generation
  - Added `tests/integration/error_handling/test_error_messages.err` - Expected error output
  - Added `tests/integration/error_handling/test_error_recovery.bas` - Tests for error recovery mechanisms
  - Added `tests/integration/error_handling/test_error_recovery.output` - Expected output for error recovery tests

- **Performance Tests**
  - Added `tests/integration/performance/test_large_files.bas` - Tests for handling large files
  - Added `tests/integration/performance/test_large_files.output` - Expected output for large file tests

### Changed

#### Documentation
- **Documentation Reorganization and Updates**
  - Updated `docs/general/CODE_ANALYSIS.md` - Updated file size metrics (qb64pe.bas: ~20,792 → ~20,945 lines), updated dates to 2026-01-10, updated parser size references
  - Updated `docs/general/IMPROVEMENTS.md` - Updated dates to 2026-01-10
  - Updated `docs/ARCHITECTURE.md` - Enhanced architecture documentation with updated metrics and improved organization
  - Updated `docs/GETTING_STARTED.md` - Streamlined getting started guide
  - Updated `docs/QB64_WIKI_GUIDE.md` - Updated wiki guide documentation
  - Updated `docs/testing/COMPONENT_TESTING_STRATEGY.md` - Streamlined component testing strategy documentation
  - Updated `docs/testing/TESTING_IMPLEMENTATION.md` - Enhanced testing implementation documentation with comprehensive details
  - Updated `docs/testing/testing.md` - Updated testing documentation
  - Updated `tests/unit/README.md` - Updated unit test documentation
  - Moved `REFACTORING_LOG.md` → `docs/REFACTORING_LOG.md` - Reorganized documentation structure
  - Moved `tests/unit/WSL_SETUP.md` → `docs/testing/WSL_SETUP.md` - Reorganized testing documentation
  - Moved `tests/unit/QB64_MAIN_PROGRAM_STRUCTURE_DEBUG.md` → `docs/problems_encountered/qb64_main_program_structure.md` - Reorganized problem documentation

- **C Header Documentation**
  - Enhanced `internal/c/libqb/include/buffer.h` - Added comprehensive function documentation (76 lines added)
  - Enhanced `internal/c/libqb/include/datetime.h` - Added function and type documentation (67 lines added)
  - Enhanced `internal/c/libqb/include/error_handle.h` - Added error handling documentation (71 lines added)
  - Enhanced `internal/c/libqb/include/filepath.h` - Added file path utility documentation (66 lines added)
  - Enhanced `internal/c/libqb/include/filesystem.h` - Added filesystem operation documentation (108 lines added)
  - Enhanced `internal/c/libqb/include/mem.h` - Added memory management documentation (178 lines added)
  - Enhanced `internal/c/libqb/include/qbs.h` - Added QB64 string system documentation (360 lines added)

#### Source Code
- **Main Compiler**
  - Modified `source/qb64pe.bas` - Updates to main compiler source (317 lines changed)
  - Modified `internal/c/qbx.cpp` - Fixed `func_val` forward declaration (changed from `static inline` to `extern`)

- **Include Provider Utility**
  - Updated `source/utilities/include_provider.bas` - Enhanced include provider implementation with improved functionality
  - Updated `source/utilities/include_provider.bi` - Updated interface definitions for include provider

- **Code Quality Improvements - GOTO Label Refactoring**
  - Refactored `source/utilities/hash.bas` - Eliminated 6 GOTO labels, replaced with structured DO...LOOP control flow in HashFind, HashFindRev, HashFindCont, and HashDump functions
  - Refactored `source/utilities/include_provider.bas` - Eliminated 2 error handler GOTO labels, replaced with pre-validation checks using `_FILEEXISTS`
  - Refactored `source/utilities/elements.bas` - Eliminated 4 GOTO labels, replaced with structured DO...LOOP control flow in getelement$, getelements$, getelementsafter$, and numelements functions
  - **Total GOTO labels eliminated**: 12 labels across 3 utility files, improving code maintainability and test compatibility

#### Documentation
- **Architecture Documentation**
  - Enhanced `docs/ARCHITECTURE.md` - Expanded architecture documentation with additional details and improvements (84 lines added)
  - Removed `docs/CODE_REVIEW_COMBINED.md` - Consolidated documentation removed as part of documentation reorganization

#### Testing Infrastructure
- **Test Runner**
  - Enhanced `tests/run_tests.sh` - Significant improvements to test runner script with expanded functionality and better test execution (214 lines added)
  - Modified `tests/compile_tests.sh` - Improved OS detection logic and enhanced cross-platform compatibility (70 lines modified)
  - Modified `tests/add_prefix_test.sh` - Enhanced OS detection and error handling
  - Modified `tests/assert.sh` - Added improved error reporting and fast-fail support

- **Test Coverage**
  - Enhanced `tests/test_coverage.sh` - Improved test coverage analysis with better file discovery, path normalization, and coverage calculation (79 lines modified)
  - Added support for more comprehensive coverage mapping and statistics generation

- **Test Reporting**
  - Enhanced `tests/test_report.sh` - Added comprehensive error handling with trap handlers, input validation, and JSON parsing fallback support
  - Improved error recovery and cleanup mechanisms in test reporting

- **Continuous Testing**
  - Enhanced `tests/continuous_test.sh` - Improved error handling with `set -euo pipefail` and trap handlers, better process management (101 lines modified)
  - Mitigated race conditions in parallel test execution with improved array handling and process management

- **Test Output Verification**
  - Updated `tests/unit/test_output_verification.bas` - Improved output verification logic and error handling (36 lines modified)
  - Updated `tests/unit/test_output_verification.bi` - Enhanced interface definitions for output verification

- **Unit Tests**
  - Updated multiple unit test files with improved test implementations and better integration with test framework:
    - `tests/unit/code_generation/test_code_emission.bas`
    - `tests/unit/code_generation/test_code_generation.bas`
    - `tests/unit/code_generation/test_code_structure.bas`
    - `tests/unit/const_eval/test_const_eval.bas`
    - `tests/unit/parser/test_error_handling.bas`
    - `tests/unit/parser/test_expression_parsing.bas`
    - `tests/unit/parser/test_parser.bas`
    - `tests/unit/parser/test_statement_parsing.bas`
    - `tests/unit/symbol_table/test_hash.bas`
    - `tests/unit/type_system/test_type_system.bas`
  - Updated `tests/unit/test_runner.bas` - Enhanced with three-phase include system support, enabled all 10 test suites, fixed $INCLUDE syntax
  - Updated `tests/unit/test_framework_implementations.bas` - Added file output support for test results (test_results.txt) to enable result capture on Windows console applications

### Removed

#### Documentation Cleanup
- Removed `CODE_REVIEW.md` - Consolidated into other documentation files
- Removed `tests/unit/COMPILATION_NOTES.md` - Consolidated into testing documentation
- Removed `tests/unit/FORMAT_TESTING.md` - Consolidated into testing documentation
- Removed `tests/unit/INFRASTRUCTURE_FIXES.md` - Consolidated into testing documentation
- Removed `tests/unit/PHASE1_IMPLEMENTATION_STATUS.md` - Consolidated into testing documentation
- Removed `tests/unit/TESTING_ISSUES_ANALYSIS.md` - Consolidated into testing documentation
- Removed `tests/unit/TESTING_STATUS.md` - Consolidated into testing documentation
- Removed `tests/unit/TESTING_STRATEGY_ANALYSIS.md` - Consolidated into testing documentation
- Removed `tests/unit/TESTING_SUCCESS_SUMMARY.md` - Consolidated into testing documentation
- Removed `tests/unit/TEST_ISOLATION_ANALYSIS.md` - Consolidated into testing documentation
- Removed `tests/unit/TROUBLESHOOTING_LOG.md` - Consolidated into testing documentation

#### Test Infrastructure Cleanup
- Removed `tests/unit/test_constants.bas` - Consolidated into component test harness
- Removed `tests/unit/test_constants_setup.bi` - Consolidated into component test harness
- Removed `tests/unit/test_format_standalone.bas` - Consolidated into test framework
- Removed `tests/unit/test_framework.bi` - Consolidated into component test harness
- Removed `tests/unit/test_global_state_reset.bi` - Consolidated into component test harness
- Removed `tests/unit/test_minimal.bas` - Consolidated into test framework
- Removed `tests/unit/test_runner_format_minimal.bas` - Consolidated into test framework
- Removed `tests/unit/test_runner_simple.bas` - Consolidated into test framework

### Fixed
- **Compiler Issues**
  - Fixed `func_val` forward declaration in `internal/c/qbx.cpp` - Changed from incorrect `static inline` to proper `extern` declaration

- **Test Infrastructure**
  - Fixed hash table size mismatch in test state manager - Now uses consistent default size (65536)
  - Fixed missing `SHARED` keyword in REDIM statements for const arrays
  - Fixed incomplete state restoration - Arrays are now properly REDIM'd when restoring saved state
  - Fixed race conditions in parallel test execution - Improved array rebuilding approach for more robust process management
  - Fixed error handling gaps in test reporting - Added comprehensive error trap handlers, input validation, and file write error checking
  - Improved JSON parsing robustness - Added fallback handling when `jq` is not available, with proper escaping for special characters
  - Fixed $INCLUDE syntax errors in test files - All include directives now use correct `'$INCLUDE:'filename'` syntax with leading apostrophe
  - Fixed test compilation errors - Resolved "Common label within a SUB/FUNCTION" errors by refactoring GOTO labels to structured control flow
  - Fixed parser test bugs - Corrected element separator usage in test_statement_parsing.bas (3 assertions fixed)
  - Fixed include provider test bug - Corrected path mapping expectation in test_include_provider.bas (1 assertion fixed)
  - **All tests now passing**: 73/73 tests pass with 100% assertion success rate

### Statistics
- **Total Changes**: 69 files changed (63 in this update)
- **Additions**: 10,102 insertions (+3,471 in this update)
- **Deletions**: 2,334 deletions (+4,483 in this update)
- **Net Change**: +7,768 lines (-1,012 in this update)

---

## Notes

This changelog entry represents a significant update focusing on:
1. **Comprehensive Documentation**: Extensive documentation added covering architecture, code analysis, and testing strategies
2. **Testing Infrastructure**: Complete test framework implementation with unit and integration test support
3. **Component Test Harness**: New isolated testing infrastructure allowing components to be tested independently
4. **Code Quality**: New include provider utility and improvements to main compiler
5. **Test Coverage**: Addition of unit tests for core components (const eval, symbol tables, type system) and integration tests for compiler state, error handling, and performance
6. **Test Implementation**: Replaced placeholder tests with actual implementations using the component test harness

### Recent Updates (Component Test Harness)
- **Component Isolation**: Tests can now run in isolation with minimal compiler state
- **State Management**: Proper initialization and cleanup of component state for consistent testing
- **Cross-Platform**: Improved OS detection in test scripts for better Windows/Linux/macOS support
- **Bug Fixes**: Resolved critical and medium priority issues in test infrastructure

### Recent Updates (Test Infrastructure Improvements)
- **Error Handling**: Significantly improved error handling in `test_report.sh` with comprehensive validation, trap handlers, and fallback mechanisms
- **Race Condition Mitigation**: Improved parallel test execution robustness with better array handling and process management
- **Code Quality**: Enhanced error recovery mechanisms and improved JSON parsing with proper escaping support
- **Documentation**: Added comprehensive code review documentation tracking improvements and remaining issues

### Recent Updates (2026-01-10 - GOTO Refactoring & Test Infrastructure)
- **GOTO Label Elimination**: Successfully refactored all 12 GOTO labels from utility files (hash.bas: 6, include_provider.bas: 2, elements.bas: 4) to structured control flow, enabling full test suite compilation
- **Three-Phase Include System**: Split hash.bi, simplebuffer.bi, and type.bi into declaration (.bi) and initialization (.bas) files to support QB64's three-phase include system requirements
- **Test Suite Completion**: All 10 test suites now enabled and compiling successfully (type system, symbol table, parser, code generation, file utilities, string utilities, include provider, error handling, state variables, build utilities, format)
- **Test Execution Tools**: Added wrapper scripts for seamless test execution across platforms, with WSL support for fully automated testing on Windows
- **Test Results**: Achieved 100% test pass rate (73/73 tests, 73/73 assertions) after fixing test bugs and refactoring code
- **Code Quality**: Improved maintainability by replacing GOTO labels with structured DO...LOOP control flow throughout utility files

### Recent Updates (2026-01-10 - Documentation Reorganization & C Header Documentation)
- **Documentation Reorganization**: Consolidated and reorganized documentation structure, moving files to appropriate directories and removing duplicate/outdated documentation
- **Documentation Updates**: Updated file size metrics (qb64pe.bas: ~20,792 → ~20,945 lines), updated dates to 2026-01-10, and updated parser size references throughout documentation
- **C Header Documentation**: Added comprehensive documentation to 7 C header files (buffer.h, datetime.h, error_handle.h, filepath.h, filesystem.h, mem.h, qbs.h) with function descriptions, parameter documentation, and usage notes
- **Task Tracking**: Created OUTSTANDING_TASKS.md to consolidate all outstanding tasks from markdown documentation
- **Code Reviews**: Added documentation code review files tracking documentation additions and updates
- **Test Infrastructure Cleanup**: Removed obsolete test files and consolidated functionality into component test harness
- **Net Documentation Change**: -1,012 lines (removed duplicates and consolidated documentation)
