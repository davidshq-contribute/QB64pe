#!/bin/bash
# Arg 1: qb54 location

PREFIX="QBasic"

RESULTS_DIR="./tests/results/$PREFIX"
EXES_DIR="./tests/exes/$PREFIX"

mkdir -p $RESULTS_DIR
mkdir -p $EXES_DIR

QB64=$1

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
    
    if [ -f "$compile_log_file" ] && [ -s "$compile_log_file" ]; then
        if ! grep -q "^\[WARNING:" "$compile_log_file" 2>/dev/null; then
            echo ""
            echo "--- Compile Log ---"
            cat "$compile_log_file"
        fi
    fi
}

show_failure()
{
    extract_compile_errors "$RESULTS_DIR/$1-compile_result.txt" "$RESULTS_DIR/$1-compilelog.txt"
}

for sourceFile in $(find ./tests/qbasic_testcases/n54/ -name '*.bas') \
                  ./tests/qbasic_testcases/open_gl/*.bas \
                  $(find ./tests/qbasic_testcases/pete -name '*.bas') \
                  $(find ./tests/qbasic_testcases/qb45com -name '*.bas') \
                  $(find ./tests/qbasic_testcases/thebob -name '*.bas') \
                  ./tests/qbasic_testcases/misc/*.bas
do 
    test=$(basename $sourceFile .bas)

    TESTCASE=$test

    # Clear out temp folder before next compile, avoids stale compilelog files
    rm -fr ./internal/temp/*

    # Capture both stdout and stderr for complete error information
    "$QB64" -x  "$sourceFile" -o "./$EXES_DIR/$test-output.exe" >"$RESULTS_DIR/$test-compile_result.txt" 2>&1
    ERR=$?
    # Capture compile log with verification
    capture_compile_log ./internal/temp/compilelog.txt "$RESULTS_DIR/$test-compilelog.txt" "$test"

    (exit $ERR)
    assert_success_named "Compile" "Compilation Error:" show_failure $test
done
