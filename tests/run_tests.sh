#!/bin/bash

# Enhanced test runner with better reporting and utilities
# Supports fast-fail mode: set FAST_FAIL=1 to stop on first failure

. ./tests/test_utils.sh

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

# Enable fast-fail if requested
if [ "${FAST_FAIL:-0}" = "1" ]; then
    echo "Fast-fail mode enabled - stopping on first failure"
    export FAST_FAIL=1
fi

# Run compiler tests
print_test_header "Compiler Tests"
./tests/assert.sh ./tests/compile_tests.sh "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Compiler Tests" $test_result

# Run QBasic compatibility tests (optional - skip if SKIP_QBASIC_TESTS=1)
if [ "${SKIP_QBASIC_TESTS:-0}" != "1" ]; then
    print_test_header "QBasic Compatibility Tests"
    ./tests/assert.sh ./tests/qbasic_tests.sh "$QB64_EXE"
    test_result=$?
    total_tests=$((total_tests + 1))
    if [ $test_result -eq 0 ]; then
        passed_tests=$((passed_tests + 1))
    else
        failed_tests=$((failed_tests + 1))
        result=1
    fi
    print_test_result "QBasic Compatibility Tests" $test_result
else
    echo "Skipping QBasic Compatibility Tests (SKIP_QBASIC_TESTS=1)"
fi

# Run format tests
print_test_header "Format Tests"
./tests/assert.sh ./tests/format_tests.sh "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Format Tests" $test_result

# Run add prefix test
print_test_header "Add Prefix Test"
./tests/assert.sh ./tests/add_prefix_test.sh "$QB64_EXE"
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Add Prefix Test" $test_result

# Print summary
print_test_summary $total_tests $passed_tests $failed_tests

exit $result
