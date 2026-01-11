#!/bin/bash
# WSL-based test runner for QB64 unit tests
# Runs tests in Linux environment to avoid Windows GUI dialogs
#
# Usage from Windows Git Bash or Command Prompt:
#   wsl bash tests/unit/run_tests_wsl.sh

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Get the Windows path and convert to WSL path
# This script should be run from the QB64 root directory
if [ -f "qb64pe" ]; then
    ROOT_DIR="$(pwd)"
else
    # We're probably in tests/unit, go up two levels
    ROOT_DIR="$(cd ../.. && pwd)"
fi

cd "$ROOT_DIR"

echo -e "${BLUE}=== QB64-PE Unit Test Runner (WSL) ===${NC}"
echo ""

# Check if qb64pe exists (Linux binary)
if [ ! -f "qb64pe" ]; then
    echo -e "${RED}Error: qb64pe not found${NC}"
    echo "Please build QB64-PE for Linux first:"
    echo "  ./setup_lnx.sh"
    exit 1
fi

# Check if test_runner exists, compile if needed
if [ ! -f "test_runner" ]; then
    echo -e "${YELLOW}test_runner not found. Compiling tests...${NC}"
    echo ""

    # Compile the test runner
    if ./qb64pe -x tests/unit/test_runner.bas -o test_runner; then
        echo -e "${GREEN}✓ Tests compiled successfully${NC}"
        echo ""
    else
        echo -e "${RED}✗ Failed to compile tests${NC}"
        exit 1
    fi
fi

# Remove old test results if they exist
rm -f test_results.txt

echo -e "${BLUE}Running tests...${NC}"
echo ""

# Run the test executable (Linux - no separate window!)
./test_runner

# Wait a moment for the file to be written
sleep 1

# Check if test results file was created
if [ ! -f "test_results.txt" ]; then
    echo -e "${RED}✗ Error: test_results.txt not found${NC}"
    echo "Tests may have crashed or failed to run."
    exit 1
fi

echo -e "${BLUE}=== Test Results ===${NC}"
echo ""

# Read and display the test results
RESULTS=$(cat test_results.txt)
echo "$RESULTS"
echo ""

# Parse results to determine exit code
if echo "$RESULTS" | grep -q "ALL TESTS PASSED"; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    EXIT_CODE=0
elif echo "$RESULTS" | grep -q "SOME TESTS FAILED"; then
    echo -e "${RED}✗ Some tests failed${NC}"
    EXIT_CODE=1
elif echo "$RESULTS" | grep -q "Failed: 0"; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    EXIT_CODE=0
else
    # Check for any failures
    FAILED_COUNT=$(echo "$RESULTS" | grep "^Failed:" | awk '{print $2}' || echo "unknown")
    if [ "$FAILED_COUNT" = "0" ]; then
        echo -e "${GREEN}✓ All tests passed!${NC}"
        EXIT_CODE=0
    else
        echo -e "${RED}✗ Tests completed with failures${NC}"
        EXIT_CODE=1
    fi
fi

exit $EXIT_CODE
