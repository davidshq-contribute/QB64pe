#!/bin/bash

# Enhanced test runner with better reporting and utilities
# Defaults: Fast-fail enabled, QBasic compatibility tests skipped
# Use --no-fast-fail to disable fast-fail mode
# Use --run-qbasic to include QBasic compatibility tests

# Determine script location and project root
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
# If script is in tests/ directory, go up one level to project root
if [ "$(basename "$SCRIPT_DIR")" = "tests" ]; then
    ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
else
    ROOT_DIR="$SCRIPT_DIR"
fi

# Change to project root
cd "$ROOT_DIR"

# Source test utilities (adjust path based on where we are)
if [ -f "./tests/test_utils.sh" ]; then
    . ./tests/test_utils.sh
elif [ -f "./test_utils.sh" ]; then
    . ./test_utils.sh
else
    echo "Error: test_utils.sh not found!" >&2
    exit 1
fi

# Default values
FAST_FAIL_DEFAULT=1
SKIP_QBASIC_DEFAULT=1

# Parse command-line arguments
FAST_FAIL=$FAST_FAIL_DEFAULT
SKIP_QBASIC=$SKIP_QBASIC_DEFAULT

while [[ $# -gt 0 ]]; do
    case $1 in
        --no-fast-fail)
            FAST_FAIL=0
            shift
            ;;
        --run-qbasic)
            SKIP_QBASIC=0
            shift
            ;;
        --help|-h)
            cat << EOF
Usage: $0 [OPTIONS]

Options:
    --no-fast-fail    Disable fast-fail mode (run all tests even if one fails)
    --run-qbasic      Include QBasic compatibility tests (skipped by default)
    --help, -h        Show this help message

Defaults:
    - Fast-fail mode: ENABLED (stops on first failure)
    - QBasic tests: SKIPPED (use --run-qbasic to include them)

Examples:
    $0                    # Run with defaults (fast-fail, skip QBasic)
    $0 --no-fast-fail      # Run all tests even if one fails
    $0 --run-qbasic        # Include QBasic compatibility tests
    $0 --no-fast-fail --run-qbasic  # Both: run all tests including QBasic
EOF
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            echo "Use --help for usage information" >&2
            exit 1
            ;;
    esac
done

result=0
total_tests=0
passed_tests=0
failed_tests=0

# Detect the correct QB64 executable name
# In WSL/Linux, prioritize Linux binary; in Windows, prioritize Windows executable
if [ -n "$WSL_DISTRO_NAME" ] || [ "$OSTYPE" = "linux-gnu" ] || [ -f "/proc/version" ] && grep -q "Linux" /proc/version 2>/dev/null; then
    # Running in WSL or Linux - prioritize Linux binary
    if [ -f "$ROOT_DIR/qb64pe" ]; then
        QB64_EXE="$ROOT_DIR/qb64pe"
    elif [ -f "$ROOT_DIR/qb64pe.exe" ]; then
        QB64_EXE="$ROOT_DIR/qb64pe.exe"
    else
        echo "Error: qb64pe executable not found in $ROOT_DIR!"
        echo "Please build QB64-PE for Linux first: ./setup_lnx.sh"
        exit 1
    fi
else
    # Running in Windows - prioritize Windows executable
    if [ -f "$ROOT_DIR/qb64pe.exe" ]; then
        QB64_EXE="$ROOT_DIR/qb64pe.exe"
    elif [ -f "$ROOT_DIR/qb64pe" ]; then
        QB64_EXE="$ROOT_DIR/qb64pe"
    else
        echo "Error: qb64pe executable not found in $ROOT_DIR!"
        echo "Please run this script from the QB64 project root directory."
        exit 1
    fi
fi

# Set fast-fail mode (default: enabled)
if [ "$FAST_FAIL" = "1" ]; then
    echo "Fast-fail mode enabled - stopping on first failure"
    export FAST_FAIL=1
else
    echo "Fast-fail mode disabled - running all tests"
    export FAST_FAIL=0
fi

# Export SKIP_QBASIC_TESTS for child scripts
if [ "$SKIP_QBASIC" = "1" ]; then
    export SKIP_QBASIC_TESTS=1
else
    export SKIP_QBASIC_TESTS=0
fi

# Run compiler tests
print_test_header "Compiler Tests"
"$ROOT_DIR/tests/assert.sh" "$ROOT_DIR/tests/compile_tests.sh" "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Compiler Tests" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Run QBasic compatibility tests (default: skipped)
if [ "$SKIP_QBASIC" != "1" ]; then
    print_test_header "QBasic Compatibility Tests"
    "$ROOT_DIR/tests/assert.sh" "$ROOT_DIR/tests/qbasic_tests.sh" "$QB64_EXE"
    test_result=$?
    total_tests=$((total_tests + 1))
    if [ $test_result -eq 0 ]; then
        passed_tests=$((passed_tests + 1))
    else
        failed_tests=$((failed_tests + 1))
        result=1
    fi
    print_test_result "QBasic Compatibility Tests" $test_result
    # Exit early if fast-fail is enabled and test failed
    if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
        print_test_summary $total_tests $passed_tests $failed_tests
        exit $result
    fi
else
    echo "Skipping QBasic Compatibility Tests (use --run-qbasic to include them)"
fi

# Run format tests
print_test_header "Format Tests"
"$ROOT_DIR/tests/assert.sh" "$ROOT_DIR/tests/format_tests.sh" "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Format Tests" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Run add prefix test
print_test_header "Add Prefix Test"
"$ROOT_DIR/tests/assert.sh" "$ROOT_DIR/tests/add_prefix_test.sh" "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Add Prefix Test" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Run C++ runtime tests
print_test_header "C++ Runtime Tests"
"$ROOT_DIR/tests/assert.sh" "$ROOT_DIR/tests/run_c_tests.sh"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "C++ Runtime Tests" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Run unit tests
print_test_header "Unit Tests"
if [ -f "$ROOT_DIR/tests/unit/test_runner.bas" ]; then
    "$QB64_EXE" -x "$ROOT_DIR/tests/unit/test_runner.bas" -o "$ROOT_DIR/test_runner_output" 2>&1
    if [ -f "$ROOT_DIR/test_runner_output" ] || [ -f "$ROOT_DIR/test_runner_output.exe" ]; then
        EXE="$ROOT_DIR/test_runner_output"
        [ -f "$ROOT_DIR/test_runner_output.exe" ] && EXE="$ROOT_DIR/test_runner_output.exe"
        $EXE
        test_result=$?
        rm -f "$EXE" "$ROOT_DIR/test_runner_output" "$ROOT_DIR/test_runner_output.exe" 2>/dev/null
    else
        echo "Failed to compile unit test runner"
        test_result=1
    fi
else
    echo "Unit test runner not found at ./tests/unit/test_runner.bas"
    test_result=1
fi
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Unit Tests" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Run integration tests
print_test_header "Integration Tests"
test_result=0
integration_count=0
integration_passed=0
integration_failed=0
for test_file in $(find "$ROOT_DIR/tests/integration" -name "*.bas" -type f 2>/dev/null | sort); do
    integration_count=$((integration_count + 1))
    test_name=$(basename "$test_file" .bas)
    echo "  Running: $test_name"
    "$QB64_EXE" -x "$test_file" -o "$ROOT_DIR/integration_test_output" 2>&1
    compile_result=$?
    if [ $compile_result -ne 0 ]; then
        echo "    Failed to compile: $test_name"
        integration_failed=$((integration_failed + 1))
        test_result=1
        continue
    fi
    if [ -f "$ROOT_DIR/integration_test_output" ] || [ -f "$ROOT_DIR/integration_test_output.exe" ]; then
        EXE="$ROOT_DIR/integration_test_output"
        [ -f "$ROOT_DIR/integration_test_output.exe" ] && EXE="$ROOT_DIR/integration_test_output.exe"
        $EXE
        if [ $? -ne 0 ]; then
            integration_failed=$((integration_failed + 1))
            test_result=1
        else
            integration_passed=$((integration_passed + 1))
        fi
        rm -f "$EXE" "$ROOT_DIR/integration_test_output" "$ROOT_DIR/integration_test_output.exe" 2>/dev/null
    else
        echo "    Executable not found after compilation: $test_name"
        integration_failed=$((integration_failed + 1))
        test_result=1
    fi
done
if [ $integration_count -eq 0 ]; then
    echo "  No integration tests found"
    test_result=0
else
    echo "  Integration tests: $integration_passed passed, $integration_failed failed out of $integration_count total"
fi
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Integration Tests" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Run distribution tests
print_test_header "Distribution Tests"
"$ROOT_DIR/tests/assert.sh" "$ROOT_DIR/tests/run_dist_tests.sh" "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Distribution Tests" $test_result
# Exit early if fast-fail is enabled and test failed
if [ "$FAST_FAIL" = "1" ] && [ $test_result -ne 0 ]; then
    print_test_summary $total_tests $passed_tests $failed_tests
    exit $result
fi

# Print summary
print_test_summary $total_tests $passed_tests $failed_tests

exit $result
