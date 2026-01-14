#!/bin/bash
# Arg 1: Dist location

PREFIX="dist"

# Get absolute paths for everything since we're going to cd into the distribution directory
ROOT=$(pwd)
TEST_CASES="$ROOT/tests/dist"

RESULTS_DIR="$ROOT/tests/results/$PREFIX"

mkdir -p "$RESULTS_DIR"

# Move into distribution location
cd $1

# Verify that ./internal/temp/ has reasonable number of files
# In development environments, this may be higher than the 1 file expected in distributions
# xargs trims the front whitespace on OSX
tempCount=$(find ./internal/temp/ -type f | wc -l | xargs)
[ "$tempCount" -le "1500" ]
assert_success_named "./Internal/temp file count" echo "Temp has too many files: $tempCount (max 1500 for development)"

# Specific steps for each platform
case "$2" in
    win)
        # Verify that the Resource information was correctly applied
        # windres returns an error if the exe has no resource section
        windresResult=$($ROOT/internal/c/c_compiler/bin/llvm-objdump.exe -s -j .rsrc ./qb64pe.exe)
        assert_success_named "Windows Resource Section" printf "\n$windresResult\n"
        ;;

    lnx)
        ./setup_lnx.sh "dont_run" 1>"$RESULTS_DIR/linux-setup.txt"
        assert_success_named "Linux setup" cat "$RESULTS_DIR/linux-setup.txt"
        ;;

    osx)
        # When testing the OSX script we run it from a different directory as
        # that is the typical way it is used.
        pushd . > /dev/null
        cd "$ROOT"

        $1/setup_osx.command "dont_run" 1>"$RESULTS_DIR/osx-setup.txt"
        ERR=$?

        popd > /dev/null

        (exit $ERR)
        assert_success_named "OSX setup" cat "$RESULTS_DIR/osx-setup.txt"
        ;;
esac

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

for basFile in $TEST_CASES/*.bas
do 
    test=$(basename $basFile .bas)
    outputExe="$RESULTS_DIR/$test-output"

    TESTCASE=$test

    # Capture both stdout and stderr for complete error information
    ./qb64pe -x  "$TEST_CASES/$test.bas" -o "$outputExe" >"$RESULTS_DIR/$test-compile_result.txt" 2>&1
    ERR=$?
    # Capture compile log with verification
    capture_compile_log ./internal/temp/compilelog.txt "$RESULTS_DIR/$test-compilelog.txt" "$test"

    (exit $ERR)
    assert_success_named "compile" "Compilation Error:" show_failure $test

    test -f "$outputExe"
    assert_success_named "exe exists" "output.exe does not exist!" show_failure $test

    testResult=$("$outputExe" 2>&1)
    assert_success_named "run" "Execution Error:" echo "$testResult"

    [ "$testResult" == "$(cat $TEST_CASES/$test.result)" ]
    assert_success_named "result" "Result is wrong:" echo "$testResult"
done
