#!/bin/bash
# Arg 1: qb54 location
# Arg 2: Optional category to test
# Supports fast-fail mode: set FAST_FAIL=1 to stop on first failure

PREFIX="Compilation"

RESULTS_DIR="./tests/results/$PREFIX"

mkdir -p $RESULTS_DIR

QB64=$1

if [ "$#" -ge 2 ]; then
    CATEGORY="/$2"
fi

if [ "$#" -eq 3 ]; then
    TESTS_TO_RUN="$3"
else
    TESTS_TO_RUN='*.bas'
fi

show_failure()
{
    cat "$RESULTS_DIR/$1-$2-compile_result.txt"
    cat "$RESULTS_DIR/$1-$2-compilelog.txt"
}

show_incorrect_result()
{
    printf "EXPECTED: '%s'\n" "$1"
    printf "GOT:      '%s'\n" "$2"
}

# This env variable exists when running in CI. It can also be defined locally
# to enable the small OS-dependent testing
#
# This is either win, lnx, or osx
OS=$CI_OS

# Auto-detect OS if not set
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

# On Linux, we make use of xvfb-run to provide each test with a framebuffer
# based X server, which allows graphics to work.
if [ "$OS" == "lnx" ]; then
    LNX_PREFIX=xvfb-run
fi

# Each .bas file represents a separate test.
while IFS= read -r test
do 
    category=$(basename "$(dirname "$test")")
    testName=$(basename "$test" .bas)

    TESTCASE="$category/$testName"
    EXE="$RESULTS_DIR/$category-$testName - output"

    if [ "$OS" == "win" ]; then
        EXE="$EXE.exe"
    fi

    # If a .err file exists, then this test is actually testing a compilation error
    testType="success"
    if test -f "./tests/compile_tests/$category/$testName.err"; then
        testType="error"
    fi

    # Clear out temp folder before next compile, avoids stale compilelog files
    rm -fr ./internal/temp/*

    # Clean up existing EXE, so we don't use it by accident
    rm -f "$EXE*"

    compileResultOutput="$RESULTS_DIR/$category-$testName-compile_result.txt"

    # A .flags file contains any extra compiler flags to provide to QB64 for this test
    compilerFlags=
    if test -f "./tests/compile_tests/$category/$testName.flags"; then
        compilerFlags=$(cat "./tests/compile_tests/$category/$testName.flags")
    fi

    # If a license file for this OS exists, then we also check the generated license is correct
    checkLicense=
    if [ ! -z "$OS" ] && test -f "./tests/compile_tests/$category/$testName.$OS.license"; then
        compilerFlags="$compilerFlags -f:GenerateLicenseFile=true"
        checkLicense=y
    fi

    # If the "compile-from-base" file exists, then this test should be compiled
    # from the ./qb64pe directory instead of the test directory
    compileFromBase=
    if test -f "./tests/compile_tests/$category/$testName.compile-from-base"; then
        compileFromBase=y
    fi

    if [ "$compileFromBase" == "y" ]; then
        # -m and -q make sure that we get predictable results
        "$QB64" "-f:OptimizeCppProgram=true" "-f:StripDebugSymbols=false" $compilerFlags -q -m -x "./tests/compile_tests/$category/$testName.bas" -o "$EXE" 1>"$compileResultOutput"
        ERR=$?
    else
        pushd . >/dev/null
        cd "./tests/compile_tests/$category"

        # -m and -q make sure that we get predictable results
        "../../../$QB64" "-f:OptimizeCppProgram=true" "-f:StripDebugSymbols=false" $compilerFlags -q -m -x "$testName.bas" -o "../../../$EXE" 1>"../../../$compileResultOutput"
        ERR=$?

        popd >/dev/null
    fi

    cp_if_exists ./internal/temp/compilelog.txt "$RESULTS_DIR/$category-$testName-compilelog.txt"

    if [ "$testType" == "success" ]; then
        (exit $ERR)
        assert_success_named "Compile" "Compilation Error:" show_failure "$category" "$testName"

        # Use absolute path to avoid issues with working directory changes
        if [ ! -f "$EXE" ]; then
            # Try with absolute path
            ABS_EXE="$(cd "$(dirname "$EXE")" && pwd)/$(basename "$EXE")"
            if [ -f "$ABS_EXE" ]; then
                EXE="$ABS_EXE"
            fi
        fi
        test -f "$EXE"
        assert_success_named "exe exists" "Executable '$EXE' does not exist!" show_failure "$category" "$testName"

        if [ "$checkLicense" == "y" ]; then
            expectedResult="$(cat "./tests/compile_tests/$category/$testName.$OS.license")"
            testResult="$(cat "$EXE.license.txt")"

            [ "$testResult" == "$expectedResult" ]
            assert_success_named "license" "License file is wrong:" show_incorrect_result "$expectedResult" "$testResult"
        fi

        # Some tests do not have an output or err file because they should
        # compile successfully but cannot be run on the build agents
        if [ ! -f "./tests/compile_tests/$category/$testName.output" ]; then
            continue
        fi

        expectedResult="$(cat "./tests/compile_tests/$category/$testName.output")"

        pushd . > /dev/null
        cd "./tests/compile_tests/$category"
        testResult=$(\
            QB64PE_LOG_HANDLERS=file \
            QB64PE_LOG_SCOPES="qb64,libqb,libqb-image,libqb-audio" \
            QB64PE_LOG_FILE_PATH="../../../$RESULTS_DIR/$category-$testName-log.txt" \
            $LNX_PREFIX "../../../$EXE" "../../../$RESULTS_DIR" "$category-$testName" 2>&1)
        ERR=$?
        popd > /dev/null

        cat >"$RESULTS_DIR/$category-$testName-run-output.txt" <<<"$testResult"

        (exit $ERR)
        assert_success_named "run" "Execution Error:" echo "$testResult"

        # Normalize newlines for comparison (strip trailing newlines from both)
        expectedResult=$(echo -n "$expectedResult" | sed 's/\r$//')
        testResult=$(echo -n "$testResult" | sed 's/\r$//')
        
        [ "$testResult" == "$expectedResult" ]
        assert_success_named "result" "Result is wrong:" show_incorrect_result "$expectedResult" "$testResult"

        # Restart pulseaudio between each test to make sound tests work on Linux
        if [ "$CI_TESTING" == "y" ] && command -v pulseaudio > /dev/null
        then
            pulseaudio -k
            sleep .5
            pulseaudio -D
        fi
    else
        ! (exit $ERR)
        assert_success_named "Compile" "Compilation Success, was expecting error:" show_failure "$category" "$testName"

        ! test -f "$EXE"
        assert_success_named "Exe exists" "'$category-$testName - output' exists, it should not!" show_failure "$category" "$testName"

        expectedErr="$(cat "./tests/compile_tests/$category/$testName.err")"

        diffResult=$(diff -y "./tests/compile_tests/$category/$testName.err" "$compileResultOutput")
        assert_success_named "Error result" "Error reporting is wrong:" echo "$diffResult"
    fi
done < <(find "./tests/compile_tests$CATEGORY" -name "$TESTS_TO_RUN" -print)
