#!/bin/bash
# Continuous Testing Script
# Implements watch mode, incremental test running, and parallel test execution
#
# Usage:
#   ./tests/continuous_test.sh [options]
#
# Options:
#   --watch              Enable watch mode (monitors file changes)
#   --incremental        Only run tests affected by changed files
#   --parallel N         Run up to N tests in parallel (default: number of CPU cores)
#   --category CAT       Run only tests in category (compile, unit, integration, runtime, format, qbasic)
#   --tag TAG            Run only tests with tag
#   --pattern PAT        Run only tests matching pattern
#   --path PATH          Run only tests in path
#   --help               Show this help message
#
# Examples:
#   ./tests/continuous_test.sh --watch                    # Watch mode with all features
#   ./tests/continuous_test.sh --watch --incremental      # Watch mode with incremental testing
#   ./tests/continuous_test.sh --parallel 4               # Run tests in parallel (4 jobs)
#   ./tests/continuous_test.sh --watch --parallel 8       # Watch mode with 8 parallel jobs
#
# Edge Cases and Limitations:
#   - Parallel execution: If a test process hangs, it will block a job slot until timeout.
#     Consider using timeout(1) wrapper for long-running tests.
#   - Watch mode: Falls back to polling (2s interval) if inotifywait/fswatch unavailable.
#     Polling may miss rapid file changes (< 2s apart).
#   - Incremental mode: Uses file timestamps; may miss changes if system clock is adjusted.
#     Source change detection uses directory mtime which may be unreliable on some filesystems.
#   - Test state: Corrupted JSON in timestamps.json will cause incremental mode to fail.
#     Delete .test_state/ directory to reset state.
#   - Platform differences: macOS uses BSD stat/find, Linux uses GNU versions.
#     Some timestamp operations may behave differently across platforms.
#   - Job limits: If PARALLEL_JOBS exceeds system ulimit, some jobs may fail to start.
#     Check with 'ulimit -u' for process limits.
#   - File watching: inotifywait has limits on number of watched directories (default ~8192).
#     Large source trees may require increasing /proc/sys/fs/inotify/max_user_watches.

set -euo pipefail

. ./tests/test_utils.sh
. ./tests/test_discovery.sh

# Configuration
WATCH_MODE=0
INCREMENTAL_MODE=0
PARALLEL_JOBS=0
CATEGORY_FILTER=""
TAG_FILTER=""
PATTERN_FILTER=""
PATH_FILTER=""
TEST_STATE_DIR="./tests/.test_state"
TEST_DEPENDENCIES_FILE="$TEST_STATE_DIR/dependencies.json"
TEST_TIMESTAMPS_FILE="$TEST_STATE_DIR/timestamps.json"

# Detect the correct QB64 executable name
if [ -f "./qb64pe.exe" ]; then
    QB64_EXE="./qb64pe.exe"
elif [ -f "./qb64pe" ]; then
    QB64_EXE="./qb64pe"
else
    echo "Error: qb64pe executable not found!"
    exit 1
fi

# Detect number of CPU cores for default parallel jobs
# Uses multiple fallback methods to determine CPU count across different platforms.
# Priority: nproc (GNU coreutils) > /proc/cpuinfo (Linux) > sysctl (BSD/macOS) > default (4)
# Note: Returns logical cores (may include hyperthreading), not physical cores.
detect_cpu_cores() {
    if command -v nproc >/dev/null 2>&1; then
        # nproc is the most reliable cross-platform method (GNU coreutils)
        nproc
    elif [ -f /proc/cpuinfo ]; then
        # Linux: count processor entries in /proc/cpuinfo
        # This works even if nproc isn't installed
        grep -c ^processor /proc/cpuinfo
    elif command -v sysctl >/dev/null 2>&1; then
        # BSD/macOS: use sysctl to query hardware
        # hw.ncpu gives logical CPU count
        sysctl -n hw.ncpu
    else
        # Default fallback: assume 4 cores if detection fails
        # This is a conservative default that works on most systems
        echo 4
    fi
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --watch)
            WATCH_MODE=1
            shift
            ;;
        --incremental)
            INCREMENTAL_MODE=1
            shift
            ;;
        --parallel)
            if [ -z "${2:-}" ] || ! [[ "$2" =~ ^[0-9]+$ ]]; then
                echo "Error: --parallel requires a numeric argument" >&2
                echo "Example: --parallel 4" >&2
                exit 1
            fi
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --category)
            CATEGORY_FILTER="$2"
            shift 2
            ;;
        --tag)
            TAG_FILTER="$2"
            shift 2
            ;;
        --pattern)
            PATTERN_FILTER="$2"
            shift 2
            ;;
        --path)
            PATH_FILTER="$2"
            shift 2
            ;;
        --help)
            head -n 20 "$0" | grep "^# " | sed 's/^# //'
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Set default parallel jobs if not specified
if [ "$PARALLEL_JOBS" -eq 0 ]; then
    PARALLEL_JOBS=$(detect_cpu_cores)
fi

# Initialize test state directory
mkdir -p "$TEST_STATE_DIR"

# Track file modification times for incremental testing
# Returns Unix timestamp (seconds since epoch) of file modification time.
# Platform-specific: macOS uses BSD stat, Linux uses GNU stat with different flags.
# Returns 0 if file doesn't exist or stat fails (safe default for comparisons).
get_file_timestamp() {
    local file=$1
    if [ -f "$file" ]; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS: BSD stat uses -f flag, %m is modification time
            stat -f %m "$file" 2>/dev/null || echo 0
        else
            # Linux and others: GNU stat uses -c flag, %Y is modification time
            stat -c %Y "$file" 2>/dev/null || echo 0
        fi
    else
        echo 0  # File doesn't exist, return 0 (will trigger test run)
    fi
}

# Load test timestamps
load_test_timestamps() {
    if [ -f "$TEST_TIMESTAMPS_FILE" ]; then
        cat "$TEST_TIMESTAMPS_FILE"
    else
        echo "{}"
    fi
}

# Save test timestamp to state file
# Updates JSON file with new test timestamp, preserving existing entries.
# Uses jq for robust JSON manipulation if available, falls back to sed for basic cases.
# Edge case: If JSON is malformed or empty, sed fallback may create invalid JSON.
# The || clause handles empty JSON object case.
save_test_timestamp() {
    local test_name=$1
    local timestamp=$2
    local timestamps=$(load_test_timestamps)
    
    # Use jq for robust JSON manipulation if available
    # jq merges existing object with new key-value pair
    if command -v jq >/dev/null 2>&1; then
        # jq: merge existing timestamps with new entry
        # . + {...} creates a new object with all old keys plus the new key
        echo "$timestamps" | jq ". + {\"$test_name\": $timestamp}" > "$TEST_TIMESTAMPS_FILE"
    else
        # Fallback: sed-based JSON manipulation (fragile but works for simple cases)
        # Replaces closing brace with new entry and closing brace
        # This assumes JSON is well-formed and ends with }
        # If sed fails (e.g., malformed JSON), create new JSON object
        echo "$timestamps" | sed "s/}$/, \"$test_name\": $timestamp}/" > "$TEST_TIMESTAMPS_FILE" || {
            # If sed fails, create a new JSON object (handles empty or corrupted state)
            echo "{\"$test_name\": $timestamp}" > "$TEST_TIMESTAMPS_FILE"
        }
    fi
}

# Check if test needs to run (incremental mode)
# Determines if a test should run based on:
# 1. Test file modification time vs last run timestamp
# 2. Source directory changes (heuristic: any file in source dirs newer than last run)
#
# Edge cases:
# - Clock adjustments: If system clock moves backward, timestamps may be invalid
# - Filesystem quirks: Some filesystems don't update mtime reliably
# - Source detection: Uses directory-wide checks; may trigger on unrelated changes
# - Missing jq: Falls back to grep-based JSON parsing (fragile but works for simple cases)
test_needs_run() {
    local test_path=$1
    local test_name=$2
    
    if [ "$INCREMENTAL_MODE" -eq 0 ]; then
        return 0  # Always run if not in incremental mode
    fi
    
    # Get current timestamp of test file itself
    # If test file changed, we definitely need to rerun
    local current_ts=$(get_file_timestamp "$test_path")
    
    # Get last run timestamp from state file
    # This tracks when the test was last successfully executed
    local timestamps=$(load_test_timestamps)
    local last_ts=0
    if command -v jq >/dev/null 2>&1; then
        # jq provides robust JSON parsing
        # // 0 is the default value if key doesn't exist
        last_ts=$(echo "$timestamps" | jq -r ".[\"$test_name\"] // 0")
    else
        # Fallback: simple grep-based extraction (fragile but works for our JSON format)
        # This regex extracts the timestamp value for the test name
        # May fail on malformed JSON or special characters in test names
        last_ts=$(echo "$timestamps" | grep -o "\"$test_name\": [0-9]*" | grep -o "[0-9]*" || echo 0)
    fi
    
    # Check if source files changed (heuristic approach)
    # This is a simple heuristic: if ANY file in source directories is newer
    # than the last test run, we rerun the test. This may cause false positives
    # (e.g., unrelated source file changes), but ensures we don't miss real changes.
    local source_dirs=("source" "internal/c" "tests")
    for dir in "${source_dirs[@]}"; do
        if [ -d "$dir" ]; then
            # Platform-specific: macOS and Linux have different find syntax for time comparisons
            if [[ "$OSTYPE" == "darwin"* ]]; then
                # macOS: BSD find doesn't support -newermt, so we create a reference file
                # with the timestamp and use -newer (compares against file mtime)
                # This is a workaround for BSD find limitations
                local ref_file="$TEST_STATE_DIR/.timestamp_ref_$$"
                # Convert Unix timestamp to touch-compatible format
                # Try date -r first (modern macOS), fall back to date -j (older macOS)
                # If both fail, use epoch (1970-01-01) as safe default
                touch -t "$(date -r "$last_ts" +%Y%m%d%H%M.%S 2>/dev/null || date -j -f %s "$last_ts" +%Y%m%d%H%M.%S 2>/dev/null || echo 197001010000)" "$ref_file" 2>/dev/null
                if [ -f "$ref_file" ] && find "$dir" -type f -newer "$ref_file" 2>/dev/null | grep -q .; then
                    rm -f "$ref_file"
                    return 0  # Source changed, need to run test
                fi
                rm -f "$ref_file"
            else
                # Linux/GNU: use -newermt with @timestamp syntax (direct timestamp comparison)
                # This is more efficient than creating reference files
                # @timestamp is Unix epoch time format for GNU find
                if find "$dir" -type f -newermt "@$last_ts" 2>/dev/null | grep -q .; then
                    return 0  # Source changed, need to run test
                fi
            fi
        fi
    done
    
    # Test file itself changed (more reliable than source directory check)
    # This catches direct modifications to test files
    if [ "$current_ts" -gt "$last_ts" ]; then
        return 0  # Test file is newer than last run
    fi
    
    return 1  # Test doesn't need to run (no changes detected)
}

# Run a single test with proper output handling
# Executes a test based on its category, capturing stdout and stderr separately.
# Each test category has different execution requirements (compile, run, etc.)
# Output files are stored in TEST_STATE_DIR for later inspection if needed.
run_single_test() {
    local category=$1
    local test_path=$2
    local test_name=$3
    
    # Check if test needs to run (incremental mode)
    # If incremental mode is enabled and no changes detected, skip the test
    if ! test_needs_run "$test_path" "$test_name"; then
        echo "[SKIP] $test_name (no changes detected)"
        return 0  # Return success for skipped tests (they didn't fail)
    fi
    
    local result=0  # Exit code: 0 = pass, non-zero = fail
    # Store output separately for debugging (stdout and stderr)
    local output_file="$TEST_STATE_DIR/${test_name}.output"
    local error_file="$TEST_STATE_DIR/${test_name}.error"
    
    # Execute test based on category
    # Each category has different execution requirements
    case "$category" in
        compile)
            # Compile tests: compile_tests.sh handles the actual compilation
            # Note: compile_tests.sh runs all tests in a category, so we pass
            # the category name and specific test name for filtering
            local category=$(dirname "$test_path" | xargs basename)
            ./tests/compile_tests.sh "$QB64_EXE" "$category" "$(basename "$test_path" .bas)" > "$output_file" 2> "$error_file" || result=1
            ;;
        unit)
            # Unit tests: run via test_runner.bas which discovers and runs all unit tests
            # Two-step process: compile test_runner.bas, then execute the compiled binary
            if [ -f "./tests/unit/test_runner.bas" ]; then
                # Step 1: Compile test runner
                "$QB64_EXE" -x ./tests/unit/test_runner.bas -o ./test_runner_output > "$output_file" 2> "$error_file" || result=1
                # Step 2: Run compiled test runner (if compilation succeeded)
                if [ -f "./test_runner_output" ] || [ -f "./test_runner_output.exe" ]; then
                    EXE="./test_runner_output"
                    [ -f "./test_runner_output.exe" ] && EXE="./test_runner_output.exe"
                    # Append to output files (>>) since compilation output is already there
                    $EXE >> "$output_file" 2>> "$error_file" || result=1
                    rm -f "$EXE"  # Clean up compiled binary
                fi
            fi
            ;;
        integration)
            # Integration tests: compile and run individual test files
            # Similar to unit tests but each test is a separate file
            "$QB64_EXE" -x "$test_path" -o "./integration_test_output" > "$output_file" 2> "$error_file" || result=1
            # Only run if compilation succeeded (result=0)
            if [ $result -eq 0 ] && ([ -f "./integration_test_output" ] || [ -f "./integration_test_output.exe" ]); then
                EXE="./integration_test_output"
                [ -f "./integration_test_output.exe" ] && EXE="./integration_test_output.exe"
                $EXE >> "$output_file" 2>> "$error_file" || result=1
                rm -f "$EXE"  # Clean up compiled binary
            fi
            ;;
        runtime)
            # C++ runtime tests: pre-compiled executables (no QB64 compilation needed)
            # These test the C++ runtime library directly
            local test_exe="./tests/exes/cpp/${test_name}_test"
            # Handle Windows .exe extension
            if [[ "$QB64_EXE" == *.exe ]]; then
                [ -f "${test_exe}.exe" ] && test_exe="${test_exe}.exe"
            fi
            if [ -f "$test_exe" ]; then
                $test_exe > "$output_file" 2> "$error_file" || result=1
            else
                # Test executable missing - this is a failure
                echo "Test executable not found: $test_exe" > "$error_file"
                result=1
            fi
            ;;
        format)
            # Format tests: run format_tests.sh which tests code formatting
            ./tests/format_tests.sh "$QB64_EXE" > "$output_file" 2> "$error_file" || result=1
            ;;
        qbasic)
            # QBasic compatibility tests: run qbasic_tests.sh
            ./tests/qbasic_tests.sh "$QB64_EXE" > "$output_file" 2> "$error_file" || result=1
            ;;
    esac
    
    # Update timestamp only if test passed (incremental mode tracking)
    # Failed tests don't update timestamp so they'll be rerun next time
    if [ $result -eq 0 ]; then
        save_test_timestamp "$test_name" "$(date +%s)"
    fi
    
    return $result
}

# Run tests in parallel with job control
# This function implements a job pool pattern to limit concurrent test execution.
# It maintains arrays of PIDs and test names, and uses kill -0 to check if processes
# are still running. When max_jobs is reached, it polls for completed jobs before
# starting new ones.
#
# Edge cases handled:
# - Zombie processes: wait() reaps them and prevents accumulation
# - Hanging tests: kill -0 detects if process exists but doesn't detect hangs
#   (consider adding timeout wrapper for production use)
# - Race conditions: sleep 0.1 prevents tight polling loops
run_tests_parallel() {
    local tests=$1
    local max_jobs=$2
    
    local total_tests=0
    local passed_tests=0
    local failed_tests=0
    local running_jobs=0
    local pids=()          # Array of background process IDs
    local test_names=()    # Parallel array tracking which test each PID belongs to
    
    # Function to wait for a job to complete
    # Uses wait() to block until process exits and capture exit code.
    # Note: If process already exited, wait() returns immediately with stored exit code.
    # This function updates counters but doesn't return a value (it's a helper function).
    wait_for_job() {
        local pid=$1
        local test_name=$2
        wait "$pid"        # Blocks until process exits, returns exit code
        local result=$?
        
        if [ $result -eq 0 ]; then
            echo -e "${GREEN}[PASS]${NC} $test_name"
            passed_tests=$((passed_tests + 1))
        else
            echo -e "${RED}[FAIL]${NC} $test_name"
            failed_tests=$((failed_tests + 1))
        fi
        
        running_jobs=$((running_jobs - 1))
        # Note: This function doesn't return a value; it updates parent scope variables
    }
    
    # Process tests line by line (format: category|path|name|tags)
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        
        # Apply filters (skip tests that don't match criteria)
        [ -n "$CATEGORY_FILTER" ] && [ "$category" != "$CATEGORY_FILTER" ] && continue
        [ -n "$TAG_FILTER" ] && [[ ! "$tags" =~ $TAG_FILTER ]] && continue
        [ -n "$PATTERN_FILTER" ] && [[ ! "$name" =~ $PATTERN_FILTER ]] && continue
        [ -n "$PATH_FILTER" ] && [[ ! "$path" =~ $PATH_FILTER ]] && continue
        
        total_tests=$((total_tests + 1))
        
        # Wait for a slot if we're at max jobs
        # This loop implements the job pool: we can't start new jobs until one completes.
        # We poll the PID array, removing finished processes and waiting for them.
        while [ $running_jobs -ge $max_jobs ]; do
            # Rebuild arrays: keep running jobs, wait for finished ones
            # This approach handles the case where multiple jobs finish simultaneously.
            local new_pids=()
            local new_test_names=()
            for i in "${!pids[@]}"; do
                # kill -0 checks if process exists (doesn't send signal, just checks)
                # Returns 0 if process exists, non-zero if it doesn't
                if kill -0 "${pids[$i]}" 2>/dev/null; then
                    # Process still running, keep it in the array
                    new_pids+=("${pids[$i]}")
                    new_test_names+=("${test_names[$i]}")
                else
                    # Process finished (or never existed), wait for it to get exit code
                    # This also reaps zombie processes
                    wait_for_job "${pids[$i]}" "${test_names[$i]}"
                fi
            done
            # Update arrays with only running processes
            pids=("${new_pids[@]}")
            test_names=("${new_test_names[@]}")
            running_jobs=${#pids[@]}  # Update count from array length
            sleep 0.1  # Brief pause to avoid tight polling loop (reduces CPU usage)
        done
        
        # Start new job in background subshell
        # The subshell isolates the test execution environment
        (
            run_single_test "$category" "$path" "$name"
        ) &
        local pid=$!              # Capture background process PID
        pids+=($pid)              # Add to tracking array
        test_names+=("$name")     # Track which test this PID represents
        running_jobs=$((running_jobs + 1))
    done <<< "$tests"
    
    # Wait for all remaining jobs to complete
    # After processing all tests, we must wait for the final batch of jobs.
    for i in "${!pids[@]}"; do
        wait_for_job "${pids[$i]}" "${test_names[$i]}"
    done
    
    # Print summary
    echo ""
    print_test_summary $total_tests $passed_tests $failed_tests
    
    return $([ $failed_tests -eq 0 ] && echo 0 || echo 1)
}

# Run tests sequentially
run_tests_sequential() {
    local tests=$1
    
    local total_tests=0
    local passed_tests=0
    local failed_tests=0
    
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        
        # Apply filters
        [ -n "$CATEGORY_FILTER" ] && [ "$category" != "$CATEGORY_FILTER" ] && continue
        [ -n "$TAG_FILTER" ] && [[ ! "$tags" =~ $TAG_FILTER ]] && continue
        [ -n "$PATTERN_FILTER" ] && [[ ! "$name" =~ $PATTERN_FILTER ]] && continue
        [ -n "$PATH_FILTER" ] && [[ ! "$path" =~ $PATH_FILTER ]] && continue
        
        total_tests=$((total_tests + 1))
        
        if run_single_test "$category" "$path" "$name"; then
            echo -e "${GREEN}[PASS]${NC} $name"
            passed_tests=$((passed_tests + 1))
        else
            echo -e "${RED}[FAIL]${NC} $name"
            failed_tests=$((failed_tests + 1))
        fi
    done <<< "$tests"
    
    # Print summary
    echo ""
    print_test_summary $total_tests $passed_tests $failed_tests
    
    return $([ $failed_tests -eq 0 ] && echo 0 || echo 1)
}

# Watch for file changes and rerun tests
# Implements three file watching strategies:
# 1. inotifywait (Linux): Uses kernel inotify API for efficient event-driven watching
# 2. fswatch (macOS): Uses FSEvents API for efficient watching on macOS
# 3. Polling (fallback): Checks directory mtimes every 2 seconds (less efficient but universal)
#
# Edge cases:
# - Rapid file changes: inotifywait/fswatch may queue events; we use head -1 to get first event
# - Missing directories: Watch paths are checked before adding to watch list
# - Permission errors: 2>/dev/null suppresses errors but may hide real issues
# - Polling limitations: May miss changes if files are modified and reverted within 2s window
watch_mode() {
    echo "Watch mode enabled. Monitoring for file changes..."
    echo "Press Ctrl+C to stop."
    echo ""
    
    # Discover tests once (test list doesn't change during watch session)
    # This avoids re-discovering on every file change, improving performance
    _DISCOVERY_CATEGORY="$CATEGORY_FILTER"
    _DISCOVERY_TAG="$TAG_FILTER"
    _DISCOVERY_PATTERN="$PATTERN_FILTER"
    _DISCOVERY_PATH="$PATH_FILTER"
    local tests=$(get_raw_test_list)
    
    if [ -z "$tests" ]; then
        echo "No tests found matching criteria"
        return 0
    fi
    
    # Determine watch command based on platform and available tools
    # Priority: inotifywait (Linux) > fswatch (macOS) > polling (fallback)
    local watch_cmd=""
    local watch_paths=("source" "internal/c" "tests")  # Directories to monitor
    
    if command -v inotifywait >/dev/null 2>&1; then
        # Linux with inotify (most efficient, event-driven)
        # inotifywait uses kernel inotify API, very low overhead
        watch_cmd="inotifywait"
    elif command -v fswatch >/dev/null 2>&1; then
        # macOS with fswatch (efficient, event-driven)
        # fswatch uses FSEvents API, native macOS file system notifications
        watch_cmd="fswatch"
    else
        # Fallback: polling mode (works everywhere but less efficient)
        # Checks directory modification times every 2 seconds
        echo "Warning: No file watching tool found (inotifywait/fswatch). Using polling mode."
        watch_cmd="poll"
    fi
    
    # Setup cleanup handler for Ctrl+C (SIGINT) and SIGTERM
    # Ensures graceful shutdown when user presses Ctrl+C
    # Note: Signal handlers must use exit, not return
    trap 'echo ""; echo "Stopping watch mode..."; exit 0' INT TERM
    
    # Run tests initially (before waiting for changes)
    # This ensures tests run immediately when watch mode starts
    echo "Running initial test suite..."
    local test_result=0
    if [ "$PARALLEL_JOBS" -gt 1 ]; then
        run_tests_parallel "$tests" "$PARALLEL_JOBS" || test_result=$?
    else
        run_tests_sequential "$tests" || test_result=$?
    fi
    
    # Main watch loop: continuously monitor for file changes
    while true; do
        if [ "$watch_cmd" = "poll" ]; then
            # Polling mode: check directory modification times every 2 seconds
            # This is less efficient but works on all systems without additional tools
            sleep 2
            local changed=0
            for path in "${watch_paths[@]}"; do
                if [ -d "$path" ]; then
                    # Compare current directory mtime with last known mtime
                    # Note: Directory mtime updates when files are added/removed/modified
                    # This is a heuristic - it may not catch all changes (e.g., if mtime
                    # doesn't update due to filesystem quirks)
                    local current_mtime=$(get_file_timestamp "$path")
                    local last_mtime_file="$TEST_STATE_DIR/last_mtime_$(basename "$path")"
                    local last_mtime=0
                    [ -f "$last_mtime_file" ] && last_mtime=$(cat "$last_mtime_file")
                    
                    if [ "$current_mtime" -gt "$last_mtime" ]; then
                        changed=1
                        echo "$current_mtime" > "$last_mtime_file"  # Save new mtime
                    fi
                fi
            done
            
            # Skip test run if no changes detected
            if [ $changed -eq 0 ]; then
                continue
            fi
        else
            # Event-driven mode: block until file system event occurs
            if [ "$watch_cmd" = "inotifywait" ]; then
                # Linux: inotifywait blocks until a file event occurs
                # -r: recursive watching
                # -m: monitor mode (continuous, not one-shot)
                # -e: events to watch (modify, create, delete, move)
                # --format: output format (we discard it, just need to know something changed)
                # --quiet: suppress normal output
                # head -1: take first event then exit (we'll restart the command in next loop)
                local inotify_paths=()
                for path in "${watch_paths[@]}"; do
                    [ -d "$path" ] && inotify_paths+=("$path")
                done
                if [ ${#inotify_paths[@]} -gt 0 ]; then
                    # Note: This command blocks until an event occurs
                    # If multiple events happen quickly, we only process the first one
                    # The next loop iteration will catch subsequent events
                    inotifywait -r -m -e modify,create,delete,move "${inotify_paths[@]}" --format '%w%f' --quiet 2>/dev/null | head -1 > /dev/null
                fi
            elif [ "$watch_cmd" = "fswatch" ]; then
                # macOS: fswatch blocks until a file event occurs
                # -r: recursive watching
                # -1: one-shot mode (exit after first event, we'll restart in next loop)
                local fswatch_paths=()
                for path in "${watch_paths[@]}"; do
                    [ -d "$path" ] && fswatch_paths+=("$path")
                done
                if [ ${#fswatch_paths[@]} -gt 0 ]; then
                    # Note: Similar to inotifywait, this blocks until an event
                    # -1 ensures we exit after first event to allow test execution
                    fswatch -r -1 "${fswatch_paths[@]}" > /dev/null 2>&1
                fi
            fi
        fi
        
        # File change detected - rerun tests
        echo ""
        echo "File change detected. Rerunning tests..."
        echo "=========================================="
        
        # Rerun tests (using same parallel/sequential logic as initial run)
        # Note: We don't capture exit codes here as watch mode continues regardless
        if [ "$PARALLEL_JOBS" -gt 1 ]; then
            run_tests_parallel "$tests" "$PARALLEL_JOBS" || true
        else
            run_tests_sequential "$tests" || true
        fi
        
        echo ""
        echo "Watching for changes... (Press Ctrl+C to stop)"
    done
}

# Main execution
main() {
    # Discover tests
    echo "Discovering tests..."
    _DISCOVERY_CATEGORY="$CATEGORY_FILTER"
    _DISCOVERY_TAG="$TAG_FILTER"
    _DISCOVERY_PATTERN="$PATTERN_FILTER"
    _DISCOVERY_PATH="$PATH_FILTER"
    local tests=$(get_raw_test_list)
    
    if [ -z "$tests" ]; then
        echo "No tests found matching criteria"
        exit 0
    fi
    
    # Count tests
    local test_count=$(echo "$tests" | grep -c . || echo 0)
    echo "Found $test_count tests"
    
    if [ "$INCREMENTAL_MODE" -eq 1 ]; then
        echo "Incremental mode: Only running tests affected by changes"
    fi
    
    if [ "$PARALLEL_JOBS" -gt 1 ]; then
        echo "Parallel mode: Running up to $PARALLEL_JOBS tests concurrently"
    fi
    
    echo ""
    
    # Run tests
    local final_result=0
    if [ "$WATCH_MODE" -eq 1 ]; then
        watch_mode || final_result=$?
    else
        if [ "$PARALLEL_JOBS" -gt 1 ]; then
            run_tests_parallel "$tests" "$PARALLEL_JOBS" || final_result=$?
        else
            run_tests_sequential "$tests" || final_result=$?
        fi
    fi
    
    # Return appropriate exit code
    return $final_result
}

# Run main function and exit with its return code
main "$@"
exit $?