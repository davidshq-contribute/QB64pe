# Continuous Testing

This document describes the continuous testing features implemented for QB64-PE, including watch mode, incremental test running, and parallel test execution.

## Overview

The continuous testing system (`tests/continuous_test.sh`) provides three main features:

1. **Watch Mode**: Monitors file changes and automatically reruns tests
2. **Incremental Testing**: Only runs tests affected by changed files
3. **Parallel Execution**: Runs multiple tests concurrently for faster execution

## Usage

### Basic Usage

```bash
# Run all tests once
./tests/continuous_test.sh

# Enable watch mode (monitors for file changes)
./tests/continuous_test.sh --watch

# Run tests in parallel (uses all CPU cores by default)
./tests/continuous_test.sh --parallel

# Run tests in parallel with specific number of jobs
./tests/continuous_test.sh --parallel 8

# Enable incremental testing (only run affected tests)
./tests/continuous_test.sh --incremental

# Combine features: watch mode with incremental and parallel testing
./tests/continuous_test.sh --watch --incremental --parallel 4
```

### Filtering Tests

You can filter which tests to run using the same options as the test discovery system:

```bash
# Run only unit tests
./tests/continuous_test.sh --category unit

# Run only tests with a specific tag
./tests/continuous_test.sh --tag error

# Run only tests matching a pattern
./tests/continuous_test.sh --pattern test_

# Run only tests in a specific path
./tests/continuous_test.sh --path tests/unit
```

### Examples

```bash
# Watch mode for unit tests only, with parallel execution
./tests/continuous_test.sh --watch --category unit --parallel 4

# Incremental testing for compile tests
./tests/continuous_test.sh --incremental --category compile

# Watch mode with all features enabled
./tests/continuous_test.sh --watch --incremental --parallel
```

## Features

### Watch Mode

Watch mode monitors the following directories for changes:
- `source/` - QB64-PE source code
- `internal/c/` - C++ runtime code
- `tests/` - Test files

When a file change is detected, tests are automatically rerun.

**Platform Support:**
- **Linux**: Uses `inotifywait` (requires `inotify-tools` package)
- **macOS**: Uses `fswatch` (install via `brew install fswatch`)
- **Other/Windows**: Falls back to polling mode (checks every 2 seconds)

**Usage:**
```bash
./tests/continuous_test.sh --watch
```

Press `Ctrl+C` to stop watch mode.

### Incremental Testing

Incremental testing tracks which tests have been run and when, and only reruns tests that:
1. Have had their test files modified since the last run
2. Have dependencies (source files) that have changed since the last run

This significantly speeds up test runs during development by skipping tests that haven't changed.

**Usage:**
```bash
./tests/continuous_test.sh --incremental
```

Test state is stored in `tests/.test_state/timestamps.json`. You can delete this file to force all tests to run.

### Parallel Execution

Parallel execution runs multiple tests concurrently, utilizing multiple CPU cores for faster test execution.

**Default Behavior:**
- If `--parallel` is specified without a number, uses all available CPU cores
- Automatically detects CPU count using `nproc`, `/proc/cpuinfo`, or `sysctl`

**Usage:**
```bash
# Use all CPU cores
./tests/continuous_test.sh --parallel

# Use specific number of parallel jobs
./tests/continuous_test.sh --parallel 4
```

**Note**: Some tests may not be suitable for parallel execution if they:
- Modify shared resources
- Have file system conflicts
- Require exclusive access to hardware

The script handles job control automatically and waits for all tests to complete before reporting results.

## Implementation Details

### Test State Tracking

The continuous testing system maintains state in `tests/.test_state/`:
- `timestamps.json`: Records when each test was last run successfully
- `*.output`: Test output files
- `*.error`: Test error files

### File Change Detection

For incremental testing, the system:
1. Tracks modification times of test files
2. Monitors common source directories (`source/`, `internal/c/`, `tests/`)
3. Compares timestamps to determine if a test needs to run

### Job Control

Parallel execution uses:
- Background processes (`&`) for concurrent execution
- Process ID tracking to monitor job completion
- Automatic job slot management (waits for available slots when at max capacity)

## Integration with Existing Test Infrastructure

The continuous testing script integrates with:
- **Test Discovery** (`tests/test_discovery.sh`): Uses the same filtering and discovery mechanisms
- **Test Utilities** (`tests/test_utils.sh`): Uses shared utility functions for output formatting
- **Test Runners**: Calls existing test scripts (`compile_tests.sh`, `format_tests.sh`, etc.)

## Best Practices

1. **Development Workflow**: Use watch mode during active development
   ```bash
   ./tests/continuous_test.sh --watch --incremental --parallel
   ```

2. **Quick Checks**: Use incremental mode for fast feedback
   ```bash
   ./tests/continuous_test.sh --incremental --category unit
   ```

3. **Full Test Suite**: Run without incremental mode for complete coverage
   ```bash
   ./tests/continuous_test.sh --parallel
   ```

4. **CI/CD**: Continuous testing features are designed for local development. CI/CD should use the standard test runners (`run_tests.sh`) for consistent, reproducible results.

## Troubleshooting

### Watch Mode Not Working

- **Linux**: Install `inotify-tools`: `sudo apt-get install inotify-tools`
- **macOS**: Install `fswatch`: `brew install fswatch`
- **Windows/Other**: Watch mode will use polling (slower but functional)

### Tests Running When They Shouldn't (Incremental Mode)

- Delete `tests/.test_state/timestamps.json` to reset state
- Check that source file timestamps are being detected correctly

### Parallel Execution Issues

- Reduce the number of parallel jobs if tests conflict
- Some tests may need to run sequentially (file system locks, etc.)

## Future Enhancements

Potential improvements:
- More sophisticated dependency tracking (parse includes, etc.)
- Test result caching with hash-based change detection
- Integration with IDE plugins for automatic test running
- Performance metrics and timing information
