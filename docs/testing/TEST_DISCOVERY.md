# Test Discovery System

## Overview

The test discovery system provides automatic test discovery, categorization, and filtering capabilities for QB64-PE tests. This system eliminates the need to manually maintain test lists and enables flexible test execution.

## Features

### 1. Automatic Test Discovery

The system automatically discovers tests across all test directories:

- **Compile Tests**: `tests/compile_tests/**/*.bas`
- **Unit Tests**: `tests/unit/**/*.bas` (excluding framework files)
- **Integration Tests**: `tests/integration/**/*.bas`
- **Format Tests**: `tests/format_tests/**/*.bas`
- **QBasic Tests**: `tests/qbasic_testcases/**/*.bas`
- **Runtime Tests**: `tests/c/*.cpp` (excluding framework files)

**Excluded Files** (not discovered as tests):
- Unit tests: `test_runner.bas`, `test_framework.bi`, `test_*.bi`
- Runtime tests: `test.h`, `test.cpp`

### 2. Test Categorization

Tests are automatically categorized into one of six categories:

- `compile` - Compiler functionality tests
- `unit` - Unit tests for compiler components
- `integration` - Integration tests
- `runtime` - C++ runtime tests
- `format` - Code formatting tests
- `qbasic` - QBasic compatibility tests

### 3. Test Tagging

Tests can be tagged in multiple ways:

#### Metadata Files

Create a `.tags` file next to your test file (e.g., `test_example.bas.tags`):

```
error
error_handling
integration
compiler
```

Tags are space or comma-separated.

#### Automatic Tag Inference

The system automatically infers tags from:

- **Category**: All tests automatically get their category as a tag (compile, unit, integration, etc.)

**Compile Tests** (`tests/compile_tests/`):
- Files with "error" in the name get the `error` tag
- Tests with a corresponding `.err` file get the `error` tag
- Tests in directories containing "graphics" get the `graphics` tag
- Tests in directories containing "audio" get the `audio` tag

**Integration Tests** (`tests/integration/`):
- Tests in directories containing "error_handling" get the `error` tag
- Tests in directories containing "performance" get the `performance` tag
- Tests in directories containing "compiler_state" get the `compiler_state` tag

**Unit Tests** (`tests/unit/`):
- Tests automatically get their immediate subdirectory name as a tag
- For example: `tests/unit/type_system/test.bas` gets the `type_system` tag

**All Test Types**:
- Directory structure: Subdirectory names can contribute to tags
- File naming: Keywords in filenames (like "error") are detected

### 4. Test Filtering

Filter tests using multiple criteria:

- **By Category**: `--category compile`
- **By Tag**: `--tag error`
- **By Pattern**: `--pattern test_`
- **By Path**: `--path integration/error_handling`

Filters can be combined for precise test selection.

## Usage

### Basic Usage

```bash
# Source the discovery script
. ./tests/test_discovery.sh

# List all tests
discover_tests --list

# List tests in JSON format
discover_tests --json

# List tests with verbose output
discover_tests --verbose
```

### Filtering Examples

```bash
# List only unit tests
discover_tests --list --category unit

# List only error tests
discover_tests --list --tag error

# List tests matching a pattern
discover_tests --list --pattern test_error

# List tests in a specific path
discover_tests --list --path integration/error_handling

# Combine filters
discover_tests --list --category integration --tag error
```

### Running Tests with Discovery

Use the enhanced test runner:

```bash
# Run all tests
./tests/run_tests_with_discovery.sh

# Run only unit tests
./tests/run_tests_with_discovery.sh --category unit

# Run only error tests
./tests/run_tests_with_discovery.sh --tag error

# List tests without running
./tests/run_tests_with_discovery.sh --list --category compile

# Get help
./tests/run_tests_with_discovery.sh --help
```

### Environment Variables

The test runner supports the following environment variables:

- **`FAST_FAIL=1`**: Stop execution on the first test failure
- **`SKIP_QBASIC_TESTS=1`**: Skip QBasic compatibility tests (useful for faster test runs)

Example:
```bash
FAST_FAIL=1 ./tests/run_tests_with_discovery.sh --category unit
```

### Programmatic Usage

In shell scripts, you can use the discovery functions:

```bash
. ./tests/test_discovery.sh

# Get all tests (raw format: category|path|name|tags)
all_tests=$(discover_all_tests)

# Get filtered tests (raw format, no formatting)
filtered_tests=$(get_raw_test_list)

# Get tests by category
unit_tests=$(get_tests_by_category unit)

# Get tests by tag
error_tests=$(get_tests_by_tag error)

# Count tests
count=$(count_tests "$all_tests")

# Set filters before calling get_raw_test_list
_DISCOVERY_CATEGORY="compile"
_DISCOVERY_TAG="error"
_DISCOVERY_PATTERN="test_"
_DISCOVERY_PATH="error_handling"
filtered=$(get_raw_test_list)
```

**Note**: For programmatic use, prefer `get_raw_test_list()` over `discover_tests()` because:
- `discover_tests()` outputs formatted results for human consumption
- `get_raw_test_list()` returns raw pipe-delimited data that's easier to parse
- You can set filter variables (`_DISCOVERY_*`) before calling `get_raw_test_list()`

## Output Formats

### Default Format

```
compile: test_example (./tests/compile_tests/category/test_example.bas)
unit: test_type_system (./tests/unit/type_system/test_type_system.bas)
```

### List Format

```
CATEGORY        PATH                                                  NAME                            TAGS
--------------- -------------------------------------------------- ------------------------------ ------------------------------
compile         ./tests/compile_tests/category/test_example.bas      test_example                    compile,error
unit            ./tests/unit/type_system/test_type_system.bas        test_type_system                unit,type_system

Total: 2 tests
```

### JSON Format

```json
{
  "tests": [
    {
      "category": "compile",
      "path": "./tests/compile_tests/category/test_example.bas",
      "name": "test_example",
      "tags": ["compile", "error"]
    }
  ],
  "count": 1
}
```

## Integration with Existing Test Infrastructure

The discovery system integrates seamlessly with existing test scripts:

- `tests/compile_tests.sh` - Compile tests
- `tests/run_c_tests.sh` - Runtime tests
- `tests/format_tests.sh` - Format tests
- `tests/qbasic_tests.sh` - QBasic tests
- `tests/unit/test_runner.bas` - Unit tests

The enhanced runner (`run_tests_with_discovery.sh`) uses the discovery system to filter and run tests, while still leveraging the existing test execution infrastructure.

### Known Limitations

This section documents current limitations and edge cases in the test discovery system. Understanding these limitations helps set appropriate expectations and guides future improvements.

#### Test Execution Filtering Limitations

**Compile Test Filtering**: 
- **Category-based filtering works**: Filtering by `--category compile` correctly runs only compile tests.
- **Tag/Pattern/Path filtering not fully implemented**: When filtering compile tests by tag (`--tag`), pattern (`--pattern`), or path (`--path`), the system currently runs **all** compile tests instead of just the filtered subset. This is because `compile_tests.sh` processes all tests in the `compile_tests` directory and doesn't yet support individual test filtering. The discovery system correctly identifies which tests match the filter, but the underlying test runner executes all compile tests.
- **Workaround**: Use category-based filtering when you need to run only compile tests. For more granular control, you may need to manually run specific test files or enhance `compile_tests.sh` to support individual test filtering.

**Format Test Filtering**:
- Similar to compile tests, format tests only support category-based filtering (`--category format`).
- Filtering by tag, pattern, or path is not yet supported. When using these filters, all format tests will be executed.
- The underlying `format_tests.sh` script processes all tests in the format_tests directory.

**QBasic Test Filtering**:
- QBasic tests only support category-based filtering (`--category qbasic`).
- Filtering by tag, pattern, or path is not yet supported. When using these filters, all QBasic tests will be executed (unless `SKIP_QBASIC_TESTS=1` is set).
- The underlying `qbasic_tests.sh` script processes all tests in the qbasic_testcases directory.

**Unit Test Filtering**:
- Unit tests are executed via `test_runner.bas`, which runs all discovered unit tests.
- While the discovery system can filter unit tests by tag, pattern, or path for listing purposes, the actual execution runs all unit tests through the test runner.
- Individual unit test filtering at execution time is not yet supported.

**Runtime Test Filtering**:
- Runtime tests (C++ tests) support filtering, but have additional requirements:
  - Tests must be **pre-built** before execution. If a test executable is not found, the runner will report an error and suggest running `make` in the `tests/c` directory.
  - When filtering by tag, pattern, or path, the system attempts to run only the filtered tests, but all filtered tests must have been built previously.
  - Category-based filtering (`--category runtime`) runs all runtime tests via `run_c_tests.sh`.

**Integration Test Filtering**:
- Integration tests **fully support** all filtering methods (category, tag, pattern, path).
- Each discovered integration test is compiled and executed individually, allowing precise filtering.

#### Tag Inference Limitations

**Automatic Tag Detection**:
- Tag inference is based on **heuristic patterns** and may not catch all relevant tags:
  - Compile tests: Only detects "error" in filenames, `.err` file presence, and "graphics"/"audio" in directory paths.
  - Integration tests: Only detects specific directory patterns ("error_handling", "performance", "compiler_state").
  - Unit tests: Only uses immediate subdirectory name as a tag.
- **Manual tagging recommended**: For comprehensive tagging, use `.tags` metadata files rather than relying solely on automatic inference.
- **Case sensitivity**: Tag matching is case-sensitive. Tags "Error" and "error" are treated as different tags.

#### JSON Output Limitations

**JSON Parsing Dependencies**:
- The test discovery system outputs valid JSON, but parsing in scripts may have limitations:
- **Recommended**: Use `jq` for robust JSON parsing in scripts (most CI environments include it).
- **Fallback**: If `jq` is not available, some scripts use `grep`/`sed` parsing which can be fragile with:
  - Test names containing special characters (quotes, backslashes, newlines)
  - Error messages containing quotes or newlines
  - Unusual JSON formatting

#### Performance Considerations

**Large Test Suites**:
- Test discovery uses `find` commands which scan the entire test directory tree.
- For very large test suites (thousands of tests), discovery may take several seconds.
- Discovery is performed once per test run, so the impact is typically minimal.

**File System Dependencies**:
- Discovery relies on standard Unix file system operations (`find`, file existence checks).
- Performance may vary on network-mounted file systems or systems with slow I/O.
- Symbolic links are followed, which may include tests from linked directories.

#### Edge Cases and Special Characters

**Test Names with Special Characters**:
- Test names containing special shell characters (spaces, quotes, etc.) should work correctly in discovery.
- However, some edge cases may exist when these characters appear in:
  - Filenames (spaces are handled, but unusual characters may cause issues)
  - Tag names (commas in tag names are used as separators)
  - Path filters (special regex characters in `--path` may need escaping)

**Empty or Missing Files**:
- Discovery skips non-existent directories gracefully.
- Empty test files (`.bas` files with no content) are still discovered and may cause compilation errors when executed.
- Missing `.tags` files are handled gracefully (automatic inference is used instead).

#### Platform-Specific Considerations

**Windows Compatibility**:
- The discovery system works on Windows with Git Bash, MSYS2, or WSL.
- Executable detection handles both `.exe` and non-`.exe` extensions appropriately.
- Path separators are normalized for cross-platform compatibility.

**File Permissions**:
- Discovery requires read permissions on test directories.
- Tests that cannot be read are silently skipped (no error reported).

#### Future Improvements

These limitations are documented to guide future enhancements:

1. **Individual Test Filtering**: Enhance `compile_tests.sh`, `format_tests.sh`, and `qbasic_tests.sh` to support running individual tests based on discovery filters.
2. **Enhanced Tag Inference**: Expand automatic tag detection to cover more patterns and use cases.
3. **Robust JSON Parsing**: Improve fallback JSON parsing or make `jq` a required dependency.
4. **Performance Optimization**: Add caching for test discovery results to speed up repeated runs.
5. **Better Error Reporting**: Improve error messages when filtered tests cannot be executed.

## Adding Tags to Tests

### Method 1: Metadata File

Create a `.tags` file next to your test:

```bash
# For test: tests/integration/error_handling/test_error.bas
# Create: tests/integration/error_handling/test_error.bas.tags

echo "error
error_handling
integration
compiler" > tests/integration/error_handling/test_error.bas.tags
```

### Method 2: Naming Convention

The system automatically infers tags from:
- Directory names (e.g., `error_handling/` → `error` tag)
- File names (e.g., `test_error.bas` → `error` tag)
- Test type (e.g., presence of `.err` file → `error` tag)

### Method 3: Directory Structure

Tests inherit tags from their directory structure:
- `tests/integration/error_handling/` → `error`, `error_handling`, `integration`
- `tests/unit/type_system/` → `type_system`, `unit`

## Best Practices

1. **Use descriptive tags**: Tag tests with meaningful labels that help identify test purpose
2. **Combine automatic and manual tags**: Let the system infer common tags, add specific ones manually
3. **Tag by feature**: Tag tests by the feature or component they test
4. **Tag by test type**: Use tags like `error`, `performance`, `integration` to categorize test types
5. **Use consistent naming**: Consistent file and directory naming helps automatic tag inference

## Examples

### Example 1: Running All Error Tests

```bash
./tests/run_tests_with_discovery.sh --tag error
```

### Example 2: Running Integration Tests Only

```bash
./tests/run_tests_with_discovery.sh --category integration
```

### Example 3: Running Tests Matching a Pattern

```bash
./tests/run_tests_with_discovery.sh --pattern test_error
```

### Example 4: Listing Tests in JSON for CI/CD

```bash
discover_tests --json --category unit > test_list.json
```

### Example 5: Programmatic Test Filtering

```bash
. ./tests/test_discovery.sh

# Set filters
_DISCOVERY_CATEGORY="integration"
_DISCOVERY_TAG="error"

# Get filtered tests
tests=$(get_raw_test_list)

# Process each test
while IFS='|' read -r category path name tags; do
    [ -z "$category" ] && continue
    echo "Processing: $name ($category)"
    # ... run test or process ...
done <<< "$tests"
```

### Example 6: Counting Tests by Category

```bash
. ./tests/test_discovery.sh

for category in compile unit integration runtime format qbasic; do
    tests=$(get_tests_by_category "$category")
    count=$(count_tests "$tests")
    echo "$category: $count tests"
done
```

## Implementation Details

The discovery system consists of:

- `tests/test_discovery.sh` - Core discovery functions
- `tests/run_tests_with_discovery.sh` - Enhanced test runner using discovery
- `tests/test_utils.sh` - Utility functions for test output formatting
- Tag metadata files (`.tags` files) - Optional manual tagging

### Exported Functions

The following functions are exported for use in other scripts:

- `discover_tests` - Main discovery function with formatting
- `discover_all_tests` - Discover all tests (raw format)
- `get_raw_test_list` - Get filtered tests (raw format, programmatic use)
- `get_tests_by_category` - Get tests by category (helper)
- `get_tests_by_tag` - Get tests by tag (helper)
- `filter_tests` - Filter tests based on criteria
- `count_tests` - Count tests in a test list

### Data Format

Tests are represented in a pipe-delimited format:
```
category|path|name|tags
```

Where:
- `category`: One of compile, unit, integration, runtime, format, qbasic
- `path`: Full path to the test file
- `name`: Test name (filename without extension)
- `tags`: Comma-separated list of tags

The system is designed to be:
- **Fast**: Uses efficient file system operations
- **Flexible**: Supports multiple filtering criteria
- **Extensible**: Easy to add new categories or tag sources
- **Compatible**: Works with existing test infrastructure

## Future Enhancements

Potential future improvements:

- Test dependency tracking
- Test execution time tracking
- Test result caching
- Parallel test execution based on discovery
- Test coverage integration
- Test metadata in test files (comments)
