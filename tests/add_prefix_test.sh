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

# Helper function to capture compile log with verification
capture_compile_log() {
    local source_log="$1"
    local dest_log="$2"
    local test_name="$3"
    
    if [ -f "$source_log" ]; then
        if [ -s "$source_log" ]; then
            cp "$source_log" "$dest_log"
        else
            echo "[WARNING: Compile log was empty]" > "$dest_log"
            echo "Source: $source_log" >> "$dest_log"
            echo "Test: $test_name" >> "$dest_log"
        fi
    else
        echo "[WARNING: Compile log not found]" > "$dest_log"
        echo "Expected location: $source_log" >> "$dest_log"
        echo "Test: $test_name" >> "$dest_log"
        echo "This may indicate a compilation issue or log generation problem." >> "$dest_log"
    fi
}

# Helper function to extract error messages from compile output
extract_compile_errors() {
    local compile_result_file="$1"
    local compile_log_file="$2"
    
    if [ -f "$compile_result_file" ] && [ -s "$compile_result_file" ]; then
        cat "$compile_result_file"
    fi
    
    if [ -n "$compile_log_file" ] && [ -f "$compile_log_file" ] && [ -s "$compile_log_file" ]; then
        if ! grep -q "^\[WARNING:" "$compile_log_file" 2>/dev/null; then
            echo ""
            echo "--- Compile Log ---"
            cat "$compile_log_file"
        fi
    fi
}

show_failure()
{
    local result_type="$1"
    extract_compile_errors "$RESULTS_DIR/addprefix-${result_type}_result.txt" "$RESULTS_DIR/addprefix-compilelog.txt"
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
# Capture both stdout and stderr for complete error information
"$QB64" -x internal/support/converter/AddPREFIX.bas -o "${EXE}" >"$compileResultOutput" 2>&1
ERR=$?
# Capture compile log with verification
capture_compile_log ./internal/temp/compilelog.txt "$RESULTS_DIR/addprefix-compilelog.txt" "addprefix"
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
