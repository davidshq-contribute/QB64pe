#!/bin/bash

PREFIX="addprefix"
RESULTS_DIR="./tests/results/$PREFIX"
mkdir -p "$RESULTS_DIR"
QB64="$1"
OS=$CI_OS

# Auto-detect OS if not set (same logic as compile_tests.sh)
if [ -z "$OS" ]; then
    case "$(uname -s)" in
        Linux*)
            # Check if we're on WSL or Git Bash on Windows
            if [ -f /proc/version ] && grep -qi microsoft /proc/version 2>/dev/null; then
                OS="win"  # WSL
            elif [ -n "$MSYSTEM" ] || [ -n "$WSL_DISTRO_NAME" ]; then
                OS="win"  # Git Bash or WSL
            else
                OS="lnx"
            fi
            ;;
        Darwin*)  OS="osx" ;;
        MINGW*|MSYS*|CYGWIN*) OS="win" ;;
        *)
            # Default: check for .exe files or Windows paths
            if command -v cmd.exe >/dev/null 2>&1 || [ -d "/c/Windows" ] || [ -d "/mnt/c/Windows" ]; then
                OS="win"
            else
                OS="lnx"  # Default to Linux for unknown
            fi
            ;;
    esac
fi

show_failure()
{
    cat "$RESULTS_DIR/addprefix-$1_result.txt"
}

show_incorrect_result()
{
    diff -u <(echo -n "$1") <(echo -n "$2")
}


EXE="$RESULTS_DIR/AddPREFIX"
if [[ "$OS" == "win" ]]; then
    EXE="$EXE.exe"
fi

# First attempt to compile converter
rm -fr internal/temp/*
rm -f "$EXE*"
compileResultOutput="$RESULTS_DIR/addprefix-compile_result.txt"
"$QB64" -x internal/support/converter/AddPREFIX.bas -o "${EXE}" 1>"$compileResultOutput"
ERR=$?
cp_if_exists ./internal/temp/compilelog.txt "$RESULTS_DIR/addprefix-compilelog.txt"
(exit $ERR)
assert_success_named "Compile" "Compilation Error:" show_failure "compile"

# Check if executable exists (try both with and without .exe on Windows)
EXE_EXISTS=0
if [ -f "$EXE" ]; then
    EXE_EXISTS=1
elif [ "$OS" == "win" ] && [ -f "${EXE%.exe}" ]; then
    # Try without .exe extension
    EXE="${EXE%.exe}"
    EXE_EXISTS=1
elif [ "$OS" == "win" ] && [ -f "${EXE}.exe" ]; then
    # Try with .exe extension if not already there
    EXE="${EXE}.exe"
    EXE_EXISTS=1
fi

if [ $EXE_EXISTS -eq 1 ]; then
    (exit 0)
else
    (exit 1)
fi
assert_success_named "exe exists" "Executable '$EXE' does not exist!" show_failure "compile"

# Copy test case into place so converted result ends up in the results directory
cp tests/converter_tests/addprefix.bas "$RESULTS_DIR/addprefix.bas"

# Do conversion
conversionResultOutput="$RESULTS_DIR/addprefix-convert_result.txt"
"$EXE" "$RESULTS_DIR/addprefix.bas" 1> "$conversionResultOutput"
ERR=$?
(exit $ERR)
assert_success_named "Convert" "Conversion Error:" show_failure "convert"

# Confirm result is as expected
expectedResult="$(cat "tests/converter_tests/addprefix.output")"
actualResult="$(cat "$RESULTS_DIR/addprefix.bas")"
[[ "$expectedResult" == "$actualResult" ]]
assert_success_named "result" "Result is wrong:" show_incorrect_result "$expectedResult" "$actualResult"
