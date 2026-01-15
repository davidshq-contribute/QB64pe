# QB64-PE Test Suite

This directory contains the comprehensive test suite for QB64-PE. The tests verify compiler functionality, runtime behavior, and compatibility with QBasic/QB64 programs.

## Table of Contents

- [Overview](#overview)
- [Running Tests](#running-tests)
- [Test Categories](#test-categories)
  - [C++ Unit Tests](#c-unit-tests)
  - [Compilation Tests](#compilation-tests)
  - [QBasic Compatibility Tests](#qbasic-compatibility-tests)
  - [Format Tests](#format-tests)
  - [Converter Tests](#converter-tests)
  - [Distribution Tests](#distribution-tests)
- [Test Structure](#test-structure)
- [Writing New Tests](#writing-new-tests)
- [Troubleshooting](#troubleshooting)

## Overview

The QB64-PE test suite is organized into several categories, each testing different aspects of the compiler and runtime:

- **C++ Unit Tests**: Test individual C++ modules in isolation
- **Compilation Tests**: Verify that QB64 programs compile correctly
- **QBasic Compatibility Tests**: Ensure compatibility with QBasic/QB64 programs
- **Format Tests**: Verify code formatting functionality
- **Converter Tests**: Test code conversion utilities
- **Regression Tests**: Prevent reintroduction of previously fixed bugs
- **Distribution Tests**: Verify distribution packaging and setup scripts

## Running Tests

### Running All Tests

To run the complete test suite:

```bash
./tests/run_tests.sh ./qb64pe
```

This will execute:
- Compilation tests
- QBasic compatibility tests
- Format tests
- Add prefix converter tests
- Regression tests

### Running Individual Test Suites

#### C++ Unit Tests

Build and run C++ unit tests:

```bash
# Build the tests (requires Makefile)
make build-tests

# Run all C++ unit tests
./tests/run_c_tests.sh

# Run a specific test
./tests/exes/cpp/color_test
./tests/exes/cpp/graphics_test
./tests/exes/cpp/screen_test
./tests/exes/cpp/fileio_test
./tests/exes/cpp/buffer_test
./tests/exes/cpp/http_test
```

#### Compilation Tests

Test that QB64 programs compile correctly:

```bash
# Run all compilation tests
./tests/assert.sh ./tests/compile_tests.sh ./qb64pe

# Run tests for a specific category
./tests/assert.sh ./tests/compile_tests.sh ./qb64pe category_name

# Run a specific test file
./tests/assert.sh ./tests/compile_tests.sh ./qb64pe category_name "test_name.bas"
```

#### QBasic Compatibility Tests

Test compatibility with QBasic/QB64 programs:

```bash
./tests/assert.sh ./tests/qbasic_tests.sh ./qb64pe
```

#### Format Tests

Test code formatting functionality:

```bash
# Run all format tests
./tests/assert.sh ./tests/format_tests.sh ./qb64pe

# Run tests for a specific category
./tests/assert.sh ./tests/format_tests.sh ./qb64pe category_name
```

#### Converter Tests

Test the AddPREFIX converter utility:

```bash
./tests/assert.sh ./tests/add_prefix_test.sh ./qb64pe
```

#### Distribution Tests

Test distribution packaging (requires a built distribution):

```bash
# Test Windows distribution
./tests/assert.sh ./tests/dist_tests.sh /path/to/dist win

# Test Linux distribution
./tests/assert.sh ./tests/dist_tests.sh /path/to/dist lnx

# Test macOS distribution
./tests/assert.sh ./tests/dist_tests.sh /path/to/dist osx
```

## Test Categories

### C++ Unit Tests

**Location**: `tests/c/`

**Purpose**: Test individual C++ modules in isolation to verify their functionality independently of the full QB64 runtime.

**Test Files**:
- `test_color.cpp` - Color manipulation functions (RGB, RGBA, HSB conversion)
- `test_fileio.cpp` - File I/O operations and file handle management
- `test_graphics.cpp` - Graphics functions (HSB/RGB color conversion)
- `test_screen.cpp` - Screen management functions (fullscreen, resize, display control)
- `test.cpp` / `test.h` - Test framework utilities
- `buffer.cpp` - Buffer management tests
- `http.cpp` - HTTP client functionality tests

**How to Run**:
```bash
make build-tests
./tests/run_c_tests.sh
```

**Expected Behavior**: All tests should pass. Tests that require runtime initialization (like display) may have limited functionality but should not crash.

**See Also**: Individual test files contain detailed documentation about what each test covers.

### Compilation Tests

**Location**: `tests/compile_tests/`

**Purpose**: Verify that QB64 programs compile correctly, produce expected output, and handle compilation errors appropriately.

**Test Structure**:
- Each `.bas` file is a test case
- `.output` files contain expected program output
- `.err` files indicate tests that should produce compilation errors
- `.flags` files contain additional compiler flags for specific tests
- `.license` files (OS-specific) verify license file generation

**How to Run**:
```bash
./tests/assert.sh ./tests/compile_tests.sh ./qb64pe
```

**Expected Behavior**: 
- Programs with `.output` files should compile and produce matching output
- Programs with `.err` files should fail to compile with the expected error message
- Test results are saved in `tests/results/Compilation/`

### QBasic Compatibility Tests

**Location**: `tests/qbasic_testcases/`

**Purpose**: Ensure QB64-PE can compile and run programs originally written for QBasic or QB64, maintaining backward compatibility.

**Test Sources**:
- `n54/` - Test cases from various sources
- `open_gl/` - OpenGL-related programs
- `pete/` - Pete's QB64 programs
- `qb45com/` - QBasic 4.5 compatibility tests
- `thebob/` - TheBOB's programs
- `misc/` - Miscellaneous test cases

**How to Run**:
```bash
./tests/assert.sh ./tests/qbasic_tests.sh ./qb64pe
```

**Expected Behavior**: All programs should compile successfully. Test results are saved in `tests/results/QBasic/`

### Format Tests

**Location**: `tests/format_tests/`

**Purpose**: Verify that the code formatter produces correct output with various formatting options.

**Test Structure**:
- Each test has a `.bas` source file
- `.flagmap` files define test variants with different compiler flags
- `.out` files contain expected formatted output for each variant

**How to Run**:
```bash
./tests/assert.sh ./tests/format_tests.sh ./qb64pe
```

**Expected Behavior**: The formatter should produce output matching the expected `.out` files for each flag combination. Test results are saved in `tests/results/Format/`

### Converter Tests

**Location**: `tests/converter_tests/`

**Purpose**: Test code conversion utilities, particularly the AddPREFIX converter.

**How to Run**:
```bash
./tests/assert.sh ./tests/add_prefix_test.sh ./qb64pe
```

**Expected Behavior**: The converter should produce output matching the expected result. Test results are saved in `tests/results/addprefix/`

### Regression Tests

**Location**: `tests/compile_tests/regression/`

**Purpose**: Prevent reintroduction of previously fixed bugs by testing common bug patterns and edge cases.

**Test Files**:
- `string_empty_handling.bas` - Tests empty string handling in various operations
- `array_bounds_check.bas` - Tests array bounds checking and edge cases
- `type_conversion_edge_cases.bas` - Tests type conversion edge cases and overflow handling
- `file_io_edge_cases.bas` - Tests file I/O operations with edge cases (empty lines, EOF, etc.)
- `string_concatenation_edge_cases.bas` - Tests string concatenation with various edge cases
- `numeric_precision.bas` - Tests numeric precision and mathematical operations

**How to Run**:
```bash
./tests/assert.sh ./tests/compile_tests.sh ./qb64pe regression
```

**Expected Behavior**: All regression tests should pass, ensuring that previously fixed bugs do not regress.

### Distribution Tests

**Location**: `tests/dist/`

**Purpose**: Verify that distribution packages are correctly built and setup scripts work properly.

**Test Structure**:
- Tests verify that distribution directories contain expected files
- Platform-specific tests verify setup scripts (Windows, Linux, macOS)
- Tests compile and run sample programs from the distribution

**How to Run**:
```bash
./tests/assert.sh ./tests/dist_tests.sh /path/to/distribution win
```

**Expected Behavior**: 
- Distribution should have correct file structure
- Setup scripts should execute without errors
- Sample programs should compile and run correctly

## Test Structure

### Test Results

Test results are stored in `tests/results/` organized by test category:
- `Compilation/` - Compilation test results
- `QBasic/` - QBasic compatibility test results
- `Format/` - Format test results
- `addprefix/` - Converter test results
- `Compilation/` - Compilation test results (includes regression tests)
- `dist/` - Distribution test results

Each test produces:
- `*-compile_result.txt` - Compilation output
- `*-compilelog.txt` - Detailed compilation log
- `*-run-output.txt` - Program execution output (if applicable)

### Test Framework

The C++ unit tests use a custom test framework defined in `tests/c/test.h` and `tests/c/test.cpp`. The framework provides:

- `test_assert(condition)` - Basic assertion
- `test_assert_with_name(name, condition)` - Named assertion
- `test_assert_ints(expected, actual)` - Integer comparison
- `test_assert_ints_with_name(name, expected, actual)` - Named integer comparison
- `test_assert_buffers(buf1, buf2, len)` - Buffer comparison
- `run_tests(mod_name, tests, count)` - Run a test suite

The shell-based tests use `tests/assert.sh` which provides:
- `assert_success_named(name, message, command...)` - Assert command succeeds
- `assert_success(message, command...)` - Assert command succeeds (unnamed)

## Writing New Tests

### Adding a C++ Unit Test

1. Create a new test file in `tests/c/` (e.g., `test_mymodule.cpp`)
2. Include `test.h` and declare test functions
3. Add test cases to the `main()` function
4. Update `tests/build.mk` to include your test:
   ```makefile
   TESTS += mymodule
   mymodule.src-y := ./tests/c/test_mymodule.cpp \
                      $(PATH_LIBQB)/src/mymodule.cpp
   ```
5. Build and run: `make build-tests && ./tests/exes/cpp/mymodule_test`

### Adding a Compilation Test

1. Create a `.bas` file in `tests/compile_tests/category/`
2. If the program should produce output, create a `.output` file with expected output
3. If the program should fail to compile, create a `.err` file with expected error
4. Optionally add a `.flags` file with compiler flags
5. Run: `./tests/assert.sh ./tests/compile_tests.sh ./qb64pe category`

### Adding a Format Test

1. Create a `.bas` source file in `tests/format_tests/category/`
2. Create a `.flagmap` file defining test variants:
   ```
   variant1.out -f:flag1=true -f:flag2=false
   variant2.out -f:flag1=false -f:flag2=true
   ```
3. Create `.out` files with expected formatted output for each variant
4. Run: `./tests/assert.sh ./tests/format_tests.sh ./qb64pe category`

## Troubleshooting

### Tests Fail to Build

- Ensure you have the required build dependencies (see main README.md)
- Check that `make build-tests` completes successfully
- Verify that all required libraries are available

### C++ Unit Tests Fail

- Check that the module being tested is properly linked
- Some tests may require runtime initialization - check test file comments
- Verify that test expectations match actual module behavior

### Compilation Tests Fail

- Check `tests/results/Compilation/*-compilelog.txt` for detailed error messages
- Verify that test programs are valid QB64 code
- Ensure expected output files match actual program behavior

### Format Tests Fail

- Verify that `.flagmap` files are correctly formatted
- Check that expected `.out` files match current formatter behavior
- Ensure line endings are consistent (tests handle `\r\n` vs `\n`)

### CI-Specific Issues

Some tests behave differently in CI environments:
- Set `CI_OS` environment variable to `win`, `lnx`, or `osx` for OS-specific tests
- Linux tests use `xvfb-run` for graphics tests (automatically handled)
- Audio tests may require PulseAudio restart between tests

## Additional Resources

- Main project README: `../README.md`
- Contributing guidelines: `../CONTRIBUTING.md`
- Build system documentation: `../docs/build-system.md`
- Testing documentation: `../docs/testing.md`
