#!/bin/bash
# Run QB64 BASIC unit tests
# Compiles and runs all test_*.bas files in tests/unit/
#
# IMPORTANT: Unit tests use $CONSOLE:ONLY mode which requires different
# compiled .o files than the GUI version of QB64. This script cleans the
# libqb .o files before running tests.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
QB64PE="$PROJECT_DIR/qb64pe"
LIBQB_SRC="$PROJECT_DIR/internal/c/libqb/src"

if [ ! -f "$QB64PE" ]; then
    echo "Error: qb64pe not found at $QB64PE"
    echo "Please build QB64-PE first"
    exit 1
fi

echo "Running QB64 BASIC Unit Tests"
echo "=============================="
echo

# Clean libqb .o files to allow console-only compilation
# (Console-only and GUI builds use different compiled code)
echo "Cleaning libqb object files for console-only mode..."
rm -f "$LIBQB_SRC"/*.o
echo

TOTAL_TESTS=0
FAILED_TESTS=0

# Find all test files
for testfile in "$SCRIPT_DIR"/unit/compiler/test_*.bas "$SCRIPT_DIR"/unit/ide/test_*.bas; do
    if [ -f "$testfile" ]; then
        testname=$(basename "$testfile" .bas)
        echo "Running: $testname"

        # Compile the test
        outfile="$SCRIPT_DIR/unit/$testname"
        if "$QB64PE" -x "$testfile" -o "$outfile" 2>/dev/null; then
            # Run the test
            if "$outfile"; then
                echo "  PASSED"
            else
                echo "  FAILED (runtime error)"
                FAILED_TESTS=$((FAILED_TESTS + 1))
            fi
            rm -f "$outfile"
        else
            echo "  FAILED (compilation error)"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi

        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        echo
    fi
done

echo "=============================="
echo "Total: $TOTAL_TESTS tests"
echo "Failed: $FAILED_TESTS tests"
echo
echo "NOTE: libqb .o files were rebuilt for console-only mode."
echo "Run 'rm -f internal/c/libqb/src/*.o' and recompile QB64PE"
echo "if you need to build GUI programs."

if [ $FAILED_TESTS -gt 0 ]; then
    exit 1
fi
