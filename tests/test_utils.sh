#!/bin/bash
# Test Utilities
# Provides helper functions for test scripts

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Print test header
print_test_header() {
    local test_name=$1
    echo ""
    echo "=========================================="
    echo "Running: $test_name"
    echo "=========================================="
}

# Print test result
print_test_result() {
    local test_name=$1
    local result=$2
    
    if [ "$result" -eq 0 ]; then
        echo -e "${GREEN}PASS${NC}: $test_name"
    else
        echo -e "${RED}FAIL${NC}: $test_name"
    fi
}

# Print summary
print_test_summary() {
    local total=$1
    local passed=$2
    local failed=$3
    
    echo ""
    echo "=========================================="
    echo "Test Summary"
    echo "=========================================="
    echo "Total:  $total"
    echo -e "Passed: ${GREEN}$passed${NC}"
    echo -e "Failed: ${RED}$failed${NC}"
    echo "=========================================="
    
    if [ "$failed" -eq 0 ]; then
        echo -e "${GREEN}ALL TESTS PASSED${NC}"
        return 0
    else
        echo -e "${RED}SOME TESTS FAILED${NC}"
        return 1
    fi
}

# Check if a file exists
check_file_exists() {
    local file=$1
    if [ ! -f "$file" ]; then
        echo -e "${RED}ERROR${NC}: File not found: $file"
        return 1
    fi
    return 0
}

# Clean test results directory
clean_test_results() {
    local results_dir=$1
    if [ -d "$results_dir" ]; then
        rm -rf "$results_dir"/*
    fi
    mkdir -p "$results_dir"
}
