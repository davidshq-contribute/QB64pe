QB64-PE Testing Framework
=========================

QB64-PE features a comprehensive testing infrastructure with multiple test
frameworks covering compiler functionality, runtime behavior, and compatibility.
The testing system includes automatic test discovery, reporting, and continuous
testing capabilities.

> **For comprehensive testing documentation**, see [TESTING_IMPLEMENTATION.md](TESTING_IMPLEMENTATION.md).

## Test Categories

### 1. Compiler Tests (`tests/compile_tests/`)

Compiler tests verify QB64 language features, compilation behavior, and error
handling. Each test consists of a `*.bas` file with an accompanying `*.output`
or `*.err` file.

**Structure:**
- `*.bas` - Test source file
- `*.output` - Expected program output (for successful compilation tests)
- `*.err` - Expected error message (for error tests)
- `*.flags` - Optional command-line flags for the compiler
- `*.compile-from-base` - Optional marker to compile from base directory instead of test directory

**Test Output Requirements:**
- Tests should not produce lines with trailing spaces (unless necessary for the test)
- Tests **cannot** produce trailing empty lines - output must end with the last meaningful line
- If trailing spaces are required, they must also exist in the `*.output` or `*.err` files

**Compilation Context:**
By default, tests are compiled from the directory containing the `.bas` file.
To compile from the base directory instead, create a `*.compile-from-base` file
next to the test file.

**Running:**
```bash
./tests/compile_tests.sh ./qb64pe
```

The script searches the `./tests/compile_tests` directory for any `*.bas` files
and uses the provided QB64-PE compiler to test them.

### 2. Unit Tests (`tests/unit/`)

Unit tests verify individual compiler components in isolation using the unit
test framework. Tests are organized by component (type system, parser, code
generation, etc.).

**Framework Files:**
- `test_framework.bi` - Test framework interface
- `test_runner.bas` - Test runner executable
- Component-specific test utilities (`.bi` files)

**Test Organization:**
- `type_system/` - Type system tests
- `symbol_table/` - Symbol table and hashing tests
- `const_eval/` - Constant evaluation tests
- `parser/` - Parser and expression parsing tests
- `code_generation/` - Code generation and emission tests

**Running:**
```bash
# Compile and run unit test runner
./qb64pe -x tests/unit/test_runner.bas
./test_runner

# Or use test discovery
./tests/run_tests_with_discovery.sh --category unit
```

### 3. Integration Tests (`tests/integration/`)

Integration tests verify end-to-end compiler behavior, including compiler state,
error handling, and performance characteristics.

**Test Areas:**
- `compiler_state/` - Symbol resolution, dependency detection
- `error_handling/` - Error messages, error recovery
- `performance/` - Large file handling, compilation speed

**Running:**
```bash
./tests/run_tests_with_discovery.sh --category integration
```

### 4. Runtime Tests (`tests/c/`)

C++-based tests for `libqb` runtime functionality. Each test is a `.cpp` file
that compiles into an executable with its own `main()` function.

**Test Framework:**
- `test.h` - Test framework header
- `test.cpp` - Test framework implementation
- `build.mk` - Build configuration for C++ tests

**Test Coverage:**
- String operations (`qbs.cpp`)
- Memory management (`mem.cpp`, `buffer.cpp`)
- File operations (`filesystem.cpp`, `gfs.cpp`)
- Network operations (`http.cpp`)
- Threading (`threading.cpp`)
- Graphics and audio subsystems (`graphics.cpp`, `audio.cpp`)
- Utility functions (string, filepath, datetime, command, etc.)

**Building and Running:**
```bash
# Build all C++ tests
make build-tests

# Run all C++ tests
./tests/run_c_tests.sh

# Or use test discovery
./tests/run_tests_with_discovery.sh --category runtime
```

**Test Build Configuration:**
- Each test executable gets its own name and combination of source/object files
- When testing `libqb` functionality, link the proper object files into your test
- Extra compiler flags can be defined individually for each test in `build.mk`
- `run_c_tests.sh` maintains an internal list of test executable names (to avoid
  using `make` to discover them) and expects tests to be built via `make build-tests`

### 5. Format Tests (`tests/format_tests/`)

Tests for code formatting functionality, including case conversion, indentation,
and spacing.

**Running:**
```bash
./tests/format_tests.sh ./qb64pe
```

### 6. QBasic Compatibility Tests (`tests/qbasic_testcases/`)

Legacy QBasic programs that verify QB64-PE can compile QBasic code without
errors. These tests verify compilation only, not program behavior.

**Test Suites:**
- Multiple test suites (n54, pete, qb45com, thebob, etc.)

**Running:**
```bash
./tests/qbasic_tests.sh ./qb64pe

# Skip QBasic tests (set environment variable)
SKIP_QBASIC_TESTS=1 ./tests/run_tests.sh
```

### 7. Distribution Tests (`tests/dist/`)

Tests for distribution builds, including platform-specific setup verification
and resource section verification (Windows).

**Running:**
```bash
./tests/dist_tests.sh <dist_location> <platform>
```

## Test Discovery System

The test discovery system (`tests/test_discovery.sh`) automatically discovers,
categorizes, and filters tests across all test directories.

### Features

- **Automatic Discovery**: Scans all test directories and categorizes tests
- **Tag Support**: Reads `.tags` files or infers tags from directory structure
- **Flexible Filtering**: Filter by category, tag, pattern, or path
- **Multiple Output Formats**: Default, list, and JSON formats
- **Programmatic API**: Functions can be sourced for use in other scripts

### Usage

```bash
# List all tests
./tests/run_tests_with_discovery.sh --list

# Run tests by category
./tests/run_tests_with_discovery.sh --category unit
./tests/run_tests_with_discovery.sh --category integration

# Run tests by tag
./tests/run_tests_with_discovery.sh --tag error
./tests/run_tests_with_discovery.sh --tag performance

# Run tests matching a pattern
./tests/run_tests_with_discovery.sh --pattern test_

# Run tests in a specific path
./tests/run_tests_with_discovery.sh --path integration/error_handling

# Combine filters
./tests/run_tests_with_discovery.sh --category integration --tag error

# JSON output for programmatic use
source ./tests/test_discovery.sh
discover_tests --category unit --json
```

## Running Tests

### Run All Tests

```bash
# Standard test runner
./tests/run_tests.sh

# Test runner with discovery system (supports filtering)
./tests/run_tests_with_discovery.sh

# Fast-fail mode (stop on first failure)
FAST_FAIL=1 ./tests/run_tests.sh
```

### Run Specific Test Categories

```bash
# Compiler tests only
./tests/compile_tests.sh ./qb64pe

# QBasic compatibility tests
./tests/qbasic_tests.sh ./qb64pe

# Format tests
./tests/format_tests.sh ./qb64pe

# C++ runtime tests
./tests/run_c_tests.sh

# Distribution tests
./tests/dist_tests.sh <dist_location> <platform>

# Converter tests
./tests/add_prefix_test.sh ./qb64pe
```

## Continuous Testing

The continuous testing script (`tests/continuous_test.sh`) provides watch mode,
incremental testing, and parallel execution for rapid development feedback.

### Features

- **Watch Mode**: Monitors file changes and automatically reruns tests
- **Incremental Testing**: Only runs tests affected by changed files
- **Parallel Execution**: Runs multiple tests simultaneously
- **State Management**: Tracks test dependencies and timestamps

### Usage

```bash
# Watch mode - automatically rerun tests on file changes
./tests/continuous_test.sh --watch

# Watch mode with incremental testing (only run affected tests)
./tests/continuous_test.sh --watch --incremental

# Parallel execution (4 parallel jobs)
./tests/continuous_test.sh --parallel 4

# Watch mode with parallel execution
./tests/continuous_test.sh --watch --parallel 8

# Combine with filters
./tests/continuous_test.sh --watch --category unit --tag error
```

## Test Reporting

The test report generator (`tests/test_report.sh`) creates HTML and text reports
with detailed test results and coverage information.

### Features

- **HTML Reports**: Interactive HTML reports with collapsible sections
- **Text Reports**: Plain text reports for terminal viewing
- **Status Detection**: Intelligent test status detection from multiple indicators
- **Error Extraction**: Automatic extraction and display of error messages
- **Coverage Calculation**: Automatic calculation of test pass rate
- **Category Summaries**: Per-category statistics and summaries

### Usage

```bash
# Generate HTML report
./tests/test_report.sh ./tests/results report.html html

# Generate text report
./tests/test_report.sh ./tests/results report.txt text
```

## Testing Best Practices

1. **Unit Tests**: Test individual components in isolation using the unit test framework
2. **Integration Tests**: Test compiler behavior end-to-end with real QB64 programs
3. **Runtime Tests**: Test libqb functionality with C++ test framework
4. **Error Tests**: Include both positive and negative test cases
5. **Test Organization**: Use subdirectories to organize related tests
6. **Test Tagging**: Add `.tags` files or rely on automatic tag inference for better filtering
7. **Test Metadata**: Use `.output` files for expected output comparison and `.err` files for error tests
8. **Continuous Testing**: Use watch mode during development for rapid feedback
9. **Parallel Execution**: Use parallel mode for faster test execution on multi-core systems
10. **Test Reports**: Generate HTML reports for detailed analysis and sharing

## Additional Resources

For more detailed information about the testing infrastructure, see:
- **[TESTING_IMPLEMENTATION.md](TESTING_IMPLEMENTATION.md)** - Complete testing strategy, implementation, and usage (authoritative source)
- **[TEST_DISCOVERY.md](TEST_DISCOVERY.md)** - Test discovery system details
- **[CONTINUOUS_TESTING.md](CONTINUOUS_TESTING.md)** - Continuous testing features
- **[COMPONENT_TESTING_STRATEGY.md](COMPONENT_TESTING_STRATEGY.md)** - Component testing strategy
- `tests/test_discovery.sh` - Test discovery system implementation
- `tests/test_utils.sh` - Test utility functions
