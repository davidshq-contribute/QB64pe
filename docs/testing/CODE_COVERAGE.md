# Code Coverage Analysis

## Overview

The QB64-PE test coverage tools provide code coverage analysis and reporting capabilities. These tools analyze which source files are covered by tests and generate detailed coverage reports.

## Features

### Coverage Analysis

The coverage analysis tool (`tests/test_coverage.sh`) performs the following:

1. **Test Discovery**: Automatically discovers all tests across test directories
2. **Source File Mapping**: Maps tests to source files they cover by analyzing:
   - `$INCLUDE` statements in QB64 tests (`.bas` files)
   - `#include` statements in C++ tests (`.cpp` files)
   - Function/subroutine names that indicate source file usage
3. **Coverage Statistics**: Calculates coverage metrics:
   - Total source files
   - Covered files
   - Uncovered files
   - Coverage percentage

### Coverage Reports

The tool generates coverage reports in multiple formats:

- **HTML Reports**: Interactive HTML reports with visual coverage bars and detailed file listings
- **Text Reports**: Plain text reports for terminal viewing
- **JSON Reports**: Machine-readable JSON format for programmatic use

## Usage

### Basic Usage

```bash
# Analyze coverage
./tests/test_coverage.sh --analyze

# Generate HTML coverage report
./tests/test_coverage.sh --report --format html

# Generate text coverage report
./tests/test_coverage.sh --report --format text

# Analyze and generate report in one command
./tests/test_coverage.sh --analyze --report
```

### Advanced Options

```bash
# Specify output file
./tests/test_coverage.sh --report --format html --output coverage.html

# Set minimum coverage threshold (exits with error if below threshold)
./tests/test_coverage.sh --analyze --min-coverage 80

# Specify custom source directories
./tests/test_coverage.sh --analyze --source-dir ./source --c-source-dir ./internal/c

# Verbose output
./tests/test_coverage.sh --analyze --verbose
```

### Integration with Test Runners

Coverage analysis can be automatically run after tests:

```bash
# Run tests with coverage analysis
COVERAGE=1 ./tests/run_tests_with_discovery.sh

# Or set environment variable
export COVERAGE=1
./tests/run_tests_with_discovery.sh
```

## Coverage Data

Coverage data is stored in `tests/coverage_data/`:

- `coverage.json`: JSON format coverage data
- `coverage_map.txt`: Mapping of tests to source files
- `coverage_report.html`: HTML coverage report (when generated)
- `coverage_report.txt`: Text coverage report (when generated)

## Coverage Analysis Details

### QB64 Source Files

The tool analyzes QB64 source files (`.bas`, `.bi`, `.bm`) by:

1. **Include Analysis**: Extracts `$INCLUDE` statements and maps them to actual source files
2. **Function Mapping**: Uses heuristics to map function/subroutine names to likely source files
3. **Path Resolution**: Searches common include paths to locate included files

### C++ Source Files

The tool analyzes C++ source files (`.cpp`, `.h`) by:

1. **Include Analysis**: Extracts `#include` statements from test files
2. **Path Resolution**: Searches C++ source directories (`internal/c/libqb`, `internal/c/parts`)
3. **Header Mapping**: Maps header files to corresponding implementation files

## Coverage Metrics

The coverage tool calculates:

- **File Coverage**: Percentage of source files covered by at least one test
- **Coverage by Category**: Coverage breakdown by test category (compile, unit, integration, runtime, etc.)
- **Test-to-File Mapping**: Which tests cover which source files

## Limitations

The current coverage analysis uses static analysis techniques:

1. **Static Analysis**: Analyzes source code statically (does not require test execution)
2. **Heuristic Mapping**: Uses naming conventions and patterns to map functions to files
3. **Include-Based**: Primarily relies on include statements to determine coverage

Future enhancements could include:

- Dynamic coverage analysis (requires instrumented builds)
- Line-level coverage (requires code instrumentation)
- Branch coverage (requires advanced instrumentation)
- Function-level coverage metrics

## Report Formats

### HTML Report

The HTML report includes:

- Summary cards with total files, covered files, uncovered files, and coverage percentage
- Visual coverage bar with color coding (green for good, yellow for medium, red for low)
- Detailed file listing with coverage status
- List of tests that cover each file

### Text Report

The text report includes:

- Summary statistics
- File-by-file coverage listing
- Test-to-file mappings

### JSON Report

The JSON report provides:

- Machine-readable coverage data
- Complete file listings with coverage status
- Test-to-file mappings
- Timestamp and metadata

## Best Practices

1. **Regular Analysis**: Run coverage analysis regularly to track coverage trends
2. **Set Thresholds**: Use `--min-coverage` to enforce minimum coverage requirements
3. **Review Reports**: Regularly review coverage reports to identify gaps
4. **Integration**: Integrate coverage analysis into CI/CD pipelines
5. **Documentation**: Document coverage goals and track progress over time

## Examples

### Basic Coverage Workflow

```bash
# 1. Run tests
./tests/run_tests_with_discovery.sh

# 2. Analyze coverage
./tests/test_coverage.sh --analyze

# 3. Generate report
./tests/test_coverage.sh --report --format html

# 4. View report
# Open tests/coverage_data/coverage_report.html in browser
```

### CI/CD Integration

```bash
# In CI/CD pipeline
./tests/run_tests_with_discovery.sh
./tests/test_coverage.sh --analyze --min-coverage 70
./tests/test_coverage.sh --report --format html --output coverage.html
# Upload coverage.html as artifact
```

### Custom Source Directories

```bash
# Analyze specific source directories
./tests/test_coverage.sh --analyze \
    --source-dir ./source/utilities \
    --c-source-dir ./internal/c/libqb
```

## Troubleshooting

### Coverage Data Not Found

If you see "Coverage data not found" error:

1. Run `--analyze` first to generate coverage data
2. Check that `tests/coverage_data/` directory exists
3. Verify that tests were discovered correctly

### Low Coverage Percentage

If coverage is lower than expected:

1. Review which files are uncovered
2. Check if tests are properly discovering source files
3. Verify include statements in test files
4. Consider adding more tests for uncovered files

### JSON Parsing Errors

If you see JSON parsing errors:

1. Install `jq` for robust JSON parsing: `sudo apt-get install jq` (Linux) or `brew install jq` (macOS)
2. The tool will fall back to basic parsing without `jq`, but functionality is limited

## Related Documentation

- [Test Discovery System](../tests/test_discovery.sh) - Test discovery and categorization
- [Test Reporting](../tests/test_report.sh) - Test report generation
- [Testing Implementation and Strategy](../testing/TESTING_IMPLEMENTATION.md) - Testing framework details
