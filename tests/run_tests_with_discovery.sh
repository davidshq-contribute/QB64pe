#!/bin/bash
# Enhanced test runner using test discovery system
# Supports filtering by category, tag, pattern, and path

. ./tests/test_utils.sh
. ./tests/test_discovery.sh

result=0
total_tests=0
passed_tests=0
failed_tests=0

# Detect the correct QB64 executable name
if [ -f "./qb64pe.exe" ]; then
    QB64_EXE="./qb64pe.exe"
elif [ -f "./qb64pe" ]; then
    QB64_EXE="./qb64pe"
else
    echo "Error: qb64pe executable not found!"
    exit 1
fi

# Parse command line arguments
CATEGORY_FILTER=""
TAG_FILTER=""
PATTERN_FILTER=""
PATH_FILTER=""
LIST_ONLY=0
VERBOSE=0

while [[ $# -gt 0 ]]; do
    case $1 in
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
        --list)
            LIST_ONLY=1
            shift
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --help)
            echo "Usage: $0 [options]"
            echo ""
            echo "Options:"
            echo "  --category CAT    Run only tests in category (compile, unit, integration, runtime, format, qbasic)"
            echo "  --tag TAG         Run only tests with tag"
            echo "  --pattern PAT     Run only tests matching pattern"
            echo "  --path PATH       Run only tests in path"
            echo "  --list            List tests without running"
            echo "  --verbose         Verbose output"
            echo "  --help            Show this help"
            echo ""
            echo "Examples:"
            echo "  $0 --category unit              # Run only unit tests"
            echo "  $0 --tag error                  # Run only error tests"
            echo "  $0 --pattern test_              # Run tests matching pattern"
            echo "  $0 --list --category compile    # List compile tests"
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Build discovery options array (safer than string concatenation)
DISCOVERY_OPTS_ARRAY=()
[ -n "$CATEGORY_FILTER" ] && DISCOVERY_OPTS_ARRAY+=(--category "$CATEGORY_FILTER")
[ -n "$TAG_FILTER" ] && DISCOVERY_OPTS_ARRAY+=(--tag "$TAG_FILTER")
[ -n "$PATTERN_FILTER" ] && DISCOVERY_OPTS_ARRAY+=(--pattern "$PATTERN_FILTER")
[ -n "$PATH_FILTER" ] && DISCOVERY_OPTS_ARRAY+=(--path "$PATH_FILTER")
[ "$LIST_ONLY" -eq 1 ] && DISCOVERY_OPTS_ARRAY+=(--list)
[ "$VERBOSE" -eq 1 ] && DISCOVERY_OPTS_ARRAY+=(--verbose)

# If list only, just show tests and exit
if [ "$LIST_ONLY" -eq 1 ]; then
    echo "Test Discovery Results"
    echo "======================"
    echo ""
    discover_tests "${DISCOVERY_OPTS_ARRAY[@]}"
    exit 0
fi

# Enable fast-fail if requested
if [ "${FAST_FAIL:-0}" = "1" ]; then
    echo "Fast-fail mode enabled - stopping on first failure"
    export FAST_FAIL=1
fi

# Discover tests
echo "Discovering tests..."
# Set filter variables for get_raw_test_list
_DISCOVERY_CATEGORY="$CATEGORY_FILTER"
_DISCOVERY_TAG="$TAG_FILTER"
_DISCOVERY_PATTERN="$PATTERN_FILTER"
_DISCOVERY_PATH="$PATH_FILTER"
DISCOVERED_TESTS=$(get_raw_test_list)

if [ -z "$DISCOVERED_TESTS" ]; then
    echo "No tests found matching criteria"
    exit 0
fi

# Count tests by category
declare -A category_counts
while IFS='|' read -r category path name tags; do
    [ -z "$category" ] && continue
    category_counts["$category"]=$((${category_counts["$category"]:-0} + 1))
done <<< "$DISCOVERED_TESTS"

# Run tests by category
for category in compile unit integration runtime format qbasic; do
    count=${category_counts["$category"]:-0}
    if [ "$count" -eq 0 ]; then
        continue
    fi
    
    print_test_header "$category Tests ($count tests)"
    
    case "$category" in
        compile)
            if [ -z "$CATEGORY_FILTER" ] || [ "$CATEGORY_FILTER" = "compile" ]; then
                ./tests/assert.sh ./tests/compile_tests.sh "$QB64_EXE"
                test_result=$?
            else
                # Run specific compile tests based on filters
                # LIMITATION: compile_tests.sh doesn't support individual test filtering yet.
                # When compile tests are filtered by tag, pattern, or path, we run all compile tests
                # because compile_tests.sh processes all tests in the compile_tests directory.
                # Individual compile test filtering requires enhancements to compile_tests.sh.
                if [ "$VERBOSE" -eq 1 ]; then
                    echo "Warning: Compile test filtering by tag/pattern/path is not yet supported."
                    echo "Running all compile tests instead."
                fi
                ./tests/assert.sh ./tests/compile_tests.sh "$QB64_EXE"
                test_result=$?
            fi
            ;;
        unit)
            # Unit tests are run via test_runner.bas
            if [ -f "./tests/unit/test_runner.bas" ]; then
                "$QB64_EXE" -x ./tests/unit/test_runner.bas -o ./test_runner_output 2>&1
                if [ -f "./test_runner_output" ] || [ -f "./test_runner_output.exe" ]; then
                    EXE="./test_runner_output"
                    [ -f "./test_runner_output.exe" ] && EXE="./test_runner_output.exe"
                    $EXE
                    test_result=$?
                    rm -f "$EXE"
                else
                    echo "Failed to compile unit test runner"
                    test_result=1
                fi
            else
                echo "Unit test runner not found"
                test_result=1
            fi
            ;;
        integration)
            # Integration tests - run each discovered test
            test_result=0
            while IFS='|' read -r cat path name tags; do
                [ "$cat" != "integration" ] && continue
                echo "Running: $name"
                "$QB64_EXE" -x "$path" -o "./integration_test_output" 2>&1
                compile_result=$?
                if [ $compile_result -ne 0 ]; then
                    echo "  Failed to compile: $name"
                    test_result=1
                    continue
                fi
                if [ -f "./integration_test_output" ] || [ -f "./integration_test_output.exe" ]; then
                    EXE="./integration_test_output"
                    [ -f "./integration_test_output.exe" ] && EXE="./integration_test_output.exe"
                    $EXE
                    if [ $? -ne 0 ]; then
                        test_result=1
                    fi
                    rm -f "$EXE"
                else
                    echo "  Executable not found after compilation: $name"
                    test_result=1
                fi
            done <<< "$DISCOVERED_TESTS"
            ;;
        runtime)
            # C++ runtime tests
            if [ -z "$CATEGORY_FILTER" ] || [ "$CATEGORY_FILTER" = "runtime" ]; then
                ./tests/assert.sh ./tests/run_c_tests.sh
                test_result=$?
            else
                # Run specific runtime tests
                test_result=0
                # Detect OS for executable extension handling
                # Check environment variable and other indicators before setting local variable
                local OS_ENV="${OS:-}"  # Save environment variable before local declaration
                local OS="unix"
                if [[ "$QB64_EXE" == *.exe ]] || \
                   [[ "$OS_ENV" == "Windows_NT" ]] || \
                   [[ -n "${WINDIR:-}" ]] || \
                   [[ -n "${MSYSTEM:-}" ]] || \
                   [[ "$(uname -s 2>/dev/null)" =~ ^(MINGW|MSYS|CYGWIN) ]]; then
                    OS="win"
                fi
                
                while IFS='|' read -r cat path name tags; do
                    [ "$cat" != "runtime" ] && continue
                    # Extract test name and construct executable path
                    test_exe="./tests/exes/cpp/${name}_test"
                    # Check for .exe extension on Windows
                    if [ "$OS" == "win" ]; then
                        [ -f "${test_exe}.exe" ] && test_exe="${test_exe}.exe"
                    fi
                    # Verify executable exists
                    if [ ! -f "$test_exe" ]; then
                        echo "  Test executable not found: $test_exe"
                        echo "  (Test may need to be built - run 'make' in tests/c directory)"
                        test_result=1
                        continue
                    fi
                    echo "Running: $name"
                    $test_exe || test_result=1
                done <<< "$DISCOVERED_TESTS"
            fi
            ;;
        format)
            if [ -z "$CATEGORY_FILTER" ] || [ "$CATEGORY_FILTER" = "format" ]; then
                ./tests/assert.sh ./tests/format_tests.sh "$QB64_EXE"
                test_result=$?
            else
                test_result=0
            fi
            ;;
        qbasic)
            if [ -z "$CATEGORY_FILTER" ] || [ "$CATEGORY_FILTER" = "qbasic" ]; then
                if [ "${SKIP_QBASIC_TESTS:-0}" != "1" ]; then
                    ./tests/assert.sh ./tests/qbasic_tests.sh "$QB64_EXE"
                    test_result=$?
                else
                    echo "Skipping QBasic Compatibility Tests (SKIP_QBASIC_TESTS=1)"
                    test_result=0
                fi
            else
                test_result=0
            fi
            ;;
        *)
            test_result=0
            ;;
    esac
    
    total_tests=$((total_tests + 1))
    if [ $test_result -eq 0 ]; then
        passed_tests=$((passed_tests + 1))
    else
        failed_tests=$((failed_tests + 1))
        result=1
    fi
    print_test_result "$category Tests" $test_result
done

# Print summary
print_test_summary $total_tests $passed_tests $failed_tests

# Optionally run coverage analysis if COVERAGE=1 is set
if [ "${COVERAGE:-0}" = "1" ]; then
    echo ""
    echo "Running coverage analysis..."
    if [ -f "./tests/test_coverage.sh" ]; then
        if ./tests/test_coverage.sh --analyze; then
            if ! ./tests/test_coverage.sh --report --format html; then
                echo "Warning: Coverage report generation failed" >&2
            fi
        else
            echo "Warning: Coverage analysis failed" >&2
        fi
    else
        echo "Warning: Coverage tool not found at ./tests/test_coverage.sh" >&2
    fi
fi

exit $result
