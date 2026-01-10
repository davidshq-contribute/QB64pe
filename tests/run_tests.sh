#!/bin/bash

# Enhanced test runner with better reporting and utilities

. ./tests/test_utils.sh

result=0
total_tests=0
passed_tests=0
failed_tests=0

# Run compiler tests
print_test_header "Compiler Tests"
./tests/assert.sh ./tests/compile_tests.sh ./qb64pe
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "Compiler Tests" $test_result

# Run QBasic compatibility tests
print_test_header "QBasic Compatibility Tests"
./tests/assert.sh ./tests/qbasic_tests.sh ./qb64pe
test_result=$?
total_tests=$((total_tests + 1))
if [ $test_result -eq 0 ]; then
    passed_tests=$((passed_tests + 1))
else
    failed_tests=$((failed_tests + 1))
    result=1
fi
print_test_result "QBasic Compatibility Tests" $test_result

# Run format tests
print_test_header "Format Tests"
./tests/assert.sh ./tests/format_tests.sh ./qb64pe
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
./tests/assert.sh ./tests/add_prefix_test.sh ./qb64pe
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
