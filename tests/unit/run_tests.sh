#!/bin/bash
# Wrapper script to run QB64 unit tests and display results
# This script handles the Windows console window issue by:
# 1. Running test_runner.exe (which opens in a new window)
# 2. Waiting for completion
# 3. Reading and displaying test_results.txt

set -e

# Colors for output (if terminal supports it)
if [ -t 1 ]; then
    RED='\033[0;31m'
    GREEN='\033[0;32m'
    YELLOW='\033[1;33m'
    BLUE='\033[0;34m'
    NC='\033[0m' # No Color
else
    RED=''
    GREEN=''
    YELLOW=''
    BLUE=''
    NC=''
fi

# Determine the root directory (two levels up from this script)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$( cd "$SCRIPT_DIR/../.." && pwd )"

# Change to root directory
cd "$ROOT_DIR"

echo -e "${BLUE}=== QB64-PE Unit Test Runner ===${NC}"
echo ""

# Check if test_runner.exe exists
if [ ! -f "test_runner.exe" ]; then
    echo -e "${YELLOW}test_runner.exe not found. Compiling tests...${NC}"

    # Compile the test runner
    if ./qb64pe -x tests/unit/test_runner.bas; then
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
echo -e "${YELLOW}WARNING: On Windows, a separate console window will open.${NC}"
echo -e "${YELLOW}The window may show error dialogs that require user interaction.${NC}"
echo -e "${YELLOW}This is a Windows QB64 limitation.${NC}"
echo ""
echo -e "${BLUE}TIP: For automated/headless testing on Windows, use WSL:${NC}"
echo -e "${BLUE}     wsl bash tests/unit/run_tests_wsl.sh${NC}"
echo ""

# Run the test executable
# On Windows with Git Bash, this will open in a new console window
# We use 'start /wait' on Windows to wait for completion
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    # Windows: use start /wait to run and wait for completion
    # This will open in a new console window but wait for it to finish
    cmd //c "start /wait test_runner.exe"
else
    # Linux/Mac: run directly
    ./test_runner.exe
fi

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
