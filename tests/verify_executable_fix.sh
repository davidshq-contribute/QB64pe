#!/bin/bash
# Quick verification script for executable creation fix
# Tests the specific cases that were failing with "Executable Not Found" errors

set -euo pipefail

. ./tests/colors.sh

# Detect the correct QB64 executable name
if [ -f "./qb64pe.exe" ]; then
    QB64_EXE="./qb64pe.exe"
elif [ -f "./qb64pe" ]; then
    QB64_EXE="./qb64pe"
else
    echo "Error: qb64pe executable not found!"
    exit 1
fi

# Tests to verify (category and test name pairs)
declare -a TEST_CASES=(
    "audio_mem_test:newsound_test"
    "audio_mem_test:sndopen_mem_test"
    "audio_sound_test:sound_test"
    "auto_include:am-trigger"
    "auto_include:color0"
    "auto_include:color32"
    "auto_include:debug"
    "auto_include:general"
    "auto_include:no-debug"
)

RESULTS_DIR="./tests/results/Compilation"
PASSED=0
FAILED=0
TOTAL=${#TEST_CASES[@]}

echo "=========================================="
echo "Verifying Executable Creation Fix"
echo "=========================================="
echo "Testing $TOTAL specific test cases"
echo ""

for test_case in "${TEST_CASES[@]}"; do
    IFS=':' read -r category test_name <<< "$test_case"
    
    echo -n "Testing $category/$test_name... "
    
    # Run the specific test
    if ./tests/compile_tests.sh "$QB64_EXE" "$category" "${test_name}.bas" > /dev/null 2>&1; then
        # Check if executable was created
        EXE_PATH="$RESULTS_DIR/$category/$category-$test_name - output"
        if [[ "$QB64_EXE" == *.exe ]]; then
            EXE_PATH="${EXE_PATH}.exe"
        fi
        
        if [ -f "$EXE_PATH" ]; then
            echo "${GREEN}PASS${RESET} - Executable created successfully"
            PASSED=$((PASSED + 1))
        else
            echo "${RED}FAIL${RESET} - Compilation succeeded but executable not found at: $EXE_PATH"
            FAILED=$((FAILED + 1))
        fi
    else
        # Check compile result to see what failed
        COMPILE_RESULT="$RESULTS_DIR/$category/$category-$test_name-compile_result.txt"
        if [ -f "$COMPILE_RESULT" ]; then
            if grep -q "path not found" "$COMPILE_RESULT" 2>/dev/null; then
                echo "${RED}FAIL${RESET} - Still getting 'path not found' error"
            else
                echo "${YELLOW}FAIL${RESET} - Compilation failed (check compile_result.txt for details)"
            fi
        else
            echo "${RED}FAIL${RESET} - Test execution failed"
        fi
        FAILED=$((FAILED + 1))
    fi
done

echo ""
echo "=========================================="
echo "Results: $PASSED passed, $FAILED failed out of $TOTAL tests"
echo "=========================================="

if [ $FAILED -eq 0 ]; then
    echo "${GREEN}All tests passed! Fix is working correctly.${RESET}"
    exit 0
else
    echo "${RED}Some tests failed. Fix may need additional work.${RESET}"
    exit 1
fi
