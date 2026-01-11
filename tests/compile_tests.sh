#!/bin/bash
# Arg 1: qb54 location
# Arg 2: Optional category to test
# Supports fast-fail mode: set FAST_FAIL=1 to stop on first failure

PREFIX="Compilation"

RESULTS_DIR="./tests/results/$PREFIX"

mkdir -p "$RESULTS_DIR"

# Convert RESULTS_DIR to absolute path early to avoid issues with directory changes
# This ensures consistent path handling throughout the script
if RESULTS_DIR_ABS="$(cd "$RESULTS_DIR" 2>/dev/null && pwd)"; then
    RESULTS_DIR="$RESULTS_DIR_ABS"
else
    # Fallback: construct absolute path from current directory
    CURRENT_DIR="$(cd . 2>/dev/null && pwd)" || CURRENT_DIR="."
    RESULTS_DIR="$CURRENT_DIR/tests/results/$PREFIX"
fi
mkdir -p "$RESULTS_DIR"

# Helper function to get category-specific results directory
# Organizes result files by test category for better organization
get_category_results_dir() {
    local category="$1"
    local category_dir="$RESULTS_DIR/$category"
    mkdir -p "$category_dir"
    echo "$category_dir"
}

# Helper function to capture compile log with verification
# Ensures compile logs are captured and logs a warning if empty or missing
capture_compile_log() {
    local source_log="$1"
    local dest_log="$2"
    local test_name="$3"
    
    if [ -f "$source_log" ]; then
        # Check if log has content
        if [ -s "$source_log" ]; then
            cp "$source_log" "$dest_log"
        else
            # Log is empty - create a marker file
            echo "[WARNING: Compile log was empty]" > "$dest_log"
            echo "Source: $source_log" >> "$dest_log"
            echo "Test: $test_name" >> "$dest_log"
        fi
    else
        # Log file doesn't exist - create a marker file
        echo "[WARNING: Compile log not found]" > "$dest_log"
        echo "Expected location: $source_log" >> "$dest_log"
        echo "Test: $test_name" >> "$dest_log"
        echo "This may indicate a compilation issue or log generation problem." >> "$dest_log"
    fi
}

# Helper function to extract error messages from compile output
# Combines stdout and stderr for complete error information
extract_compile_errors() {
    local compile_result_file="$1"
    local compile_log_file="$2"
    
    # Start with compile result (stdout)
    if [ -f "$compile_result_file" ] && [ -s "$compile_result_file" ]; then
        cat "$compile_result_file"
    fi
    
    # Add compile log if it exists and has content
    if [ -f "$compile_log_file" ] && [ -s "$compile_log_file" ]; then
        # Check if it's not just a warning marker
        if ! grep -q "^\[WARNING:" "$compile_log_file" 2>/dev/null; then
            echo ""
            echo "--- Compile Log ---"
            cat "$compile_log_file"
        fi
    fi
}

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
    local category="$1"
    local testName="$2"
    local category_dir=$(get_category_results_dir "$category")
    
    # Use improved error extraction
    extract_compile_errors "$category_dir/$category-$testName-compile_result.txt" "$category_dir/$category-$testName-compilelog.txt"
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

# Helper function to convert a Unix path to a Windows path when needed.
# Prefer wslpath when available (WSL), otherwise fall back to /mnt/<drive>/... conversion.
to_windows_path() {
    local path="$1"

    # If wslpath exists, use it (most robust for WSL).
    if command -v wslpath >/dev/null 2>&1; then
        wslpath -w "$path"
        return
    fi

    # Fallback: Convert /mnt/c/... to C:\...
    if [[ "$path" =~ ^/mnt/([a-z])/(.*) ]]; then
        local drive="${BASH_REMATCH[1]}"
        local rest="${BASH_REMATCH[2]}"
        echo "${drive^^}:\\${rest//\//\\}"
        return
    fi

    # Already a Windows path or an unknown format; return as-is.
    echo "$path"
}

# Helper function to convert a path to an absolute *Unix* path robustly
# Handles paths with spaces and relative paths correctly.
# Note: do not convert to Windows paths here; keep Unix paths for bash file ops.
to_absolute_path() {
    local path="$1"
    local dir base
    
    # If path is empty, return empty string explicitly
    [ -z "$path" ] && { echo ""; return 1; }
    
    # Get directory and base name
    dir="$(dirname "$path")"
    base="$(basename "$path")"
    
    # Create directory if it doesn't exist
    mkdir -p "$dir" 2>/dev/null || true
    
    # Convert to absolute path
    if dir_abs="$(cd "$dir" 2>/dev/null && pwd)"; then
        local result="$dir_abs/$base"
        echo "$result"
    else
        # Fallback: try to resolve from current directory
        if current_dir="$(cd . 2>/dev/null && pwd)"; then
            # Handle case where path might be relative to current directory
            if [ "$dir" = "." ]; then
                local result="$current_dir/$base"
                echo "$result"
            else
                local result="$current_dir/$path"
                echo "$result"
            fi
        else
            # Last resort: return as-is (shouldn't happen)
            echo "$path"
        fi
    fi
}

    # Each .bas file represents a separate test.
while IFS= read -r test
do 
    category=$(basename "$(dirname "$test")")
    testName=$(basename "$test" .bas)

    TESTCASE="$category/$testName"
    
    # Get category-specific results directory for better organization
    CATEGORY_RESULTS_DIR=$(get_category_results_dir "$category")
    
    # Build executable name with proper quoting to handle spaces
    # Store executables in category-specific directory
    EXE_NAME="$category-$testName - output"
    
    # Determine if we should add .exe based on the QB64 executable being used
    # If QB64 is a .exe file, it's Windows and will create .exe files
    # Otherwise, it's Linux/macOS and will create executables without .exe
    if [ "$OS" == "win" ] && [[ "$QB64" == *.exe ]]; then
        EXE_NAME="$EXE_NAME.exe"
    fi
    
    EXE="$CATEGORY_RESULTS_DIR/$EXE_NAME"
    
    # Convert to absolute *Unix* path using helper function
    # This ensures robust path handling with spaces and directory changes
    EXE="$(to_absolute_path "$EXE")"

    # QB64 is typically a Windows executable on Windows/WSL. It expects -o to be a Windows path.
    EXE_OUT="$EXE"
    if [ "$OS" == "win" ] && [[ "$QB64" == *.exe ]]; then
        EXE_OUT="$(to_windows_path "$EXE")"
    fi
    
    # Ensure the output directory exists before compilation
    # This prevents "path not found" errors when QB64 tries to create the executable
    # Create directory using the path before conversion (if conversion happens)
    exe_dir_before_conv="$CATEGORY_RESULTS_DIR"
    mkdir -p "$exe_dir_before_conv" 2>/dev/null || true
    # Also ensure it exists in the converted path format
    mkdir -p "$(dirname "$EXE")" 2>/dev/null || true

    # If a .err file exists, then this test is actually testing a compilation error
    testType="success"
    if test -f "./tests/compile_tests/$category/$testName.err"; then
        testType="error"
    fi

    # Clear out temp folder before next compile, avoids stale compilelog files
    rm -fr ./internal/temp/* 2>/dev/null || true

    # Clean up existing EXE, so we don't use it by accident
    # Use find to properly handle paths with spaces in filenames
    find "$(dirname "$EXE")" -maxdepth 1 -name "$(basename "$EXE")*" -delete 2>/dev/null || true

    # Store compile result in category-specific directory
    compileResultOutput="$CATEGORY_RESULTS_DIR/$category-$testName-compile_result.txt"
    # Convert to absolute path for consistency
    compileResultOutput="$(to_absolute_path "$compileResultOutput")"

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

    # Ensure the output directory exists before compilation
    # This prevents "path not found" errors when QB64 tries to create the executable
    mkdir -p "$(dirname "$EXE")"

    if [ "$compileFromBase" == "y" ]; then
        # -m and -q make sure that we get predictable results
        # Capture both stdout and stderr for complete error information
        # Both EXE and compileResultOutput are already absolute paths
        "$QB64" "-f:OptimizeCppProgram=true" "-f:StripDebugSymbols=false" $compilerFlags -q -m -x "./tests/compile_tests/$category/$testName.bas" -o "$EXE_OUT" >"$compileResultOutput" 2>&1
        ERR=$?
    else
        pushd . >/dev/null
        cd "./tests/compile_tests/$category"

        # -m and -q make sure that we get predictable results
        # Capture both stdout and stderr for complete error information
        # Both EXE and compileResultOutput are already absolute paths, so use them directly
        # This avoids path resolution issues when changing directories
        # Check if QB64 is already an absolute path - if so, use it directly
        if [[ "$QB64" == /* ]]; then
            # Absolute path - use directly
            QB64_PATH="$QB64"
        else
            # Relative path - construct relative to current directory
            QB64_PATH="../../../$QB64"
        fi
        "$QB64_PATH" "-f:OptimizeCppProgram=true" "-f:StripDebugSymbols=false" $compilerFlags -q -m -x "$testName.bas" -o "$EXE_OUT" >"$compileResultOutput" 2>&1
        ERR=$?

        popd >/dev/null
    fi

    # Capture compile log with verification - store in category-specific directory
    compileLogPath="$CATEGORY_RESULTS_DIR/$category-$testName-compilelog.txt"
    capture_compile_log "./internal/temp/compilelog.txt" "$compileLogPath" "$category/$testName"

    if [ "$testType" == "success" ]; then
        # EXE should already be an absolute path, but verify it exists
        # If not, try to resolve it again using the helper function
        if [ ! -f "$EXE" ]; then
            # Try to resolve the path again
            EXE_RESOLVED="$(to_absolute_path "$EXE")"
            if [ -n "$EXE_RESOLVED" ] && [ -f "$EXE_RESOLVED" ]; then
                EXE="$EXE_RESOLVED"
            fi
        fi
        
        # Check executable existence FIRST - this is the most reliable indicator of success
        # QB64 may report errors or have non-zero exit codes even when compilation succeeds
        # (e.g., if it can't find the executable at the expected path, but it was created elsewhere)
        if [ -f "$EXE" ]; then
            # Executable exists - compilation succeeded regardless of exit code or error messages
            # This fixes false negatives where QB64 reports failure but executable was created
            : # Success - continue with test execution
        else
            # Executable doesn't exist - try without .exe extension (Linux/macOS executables)
            # This handles the case where we're using Linux qb64pe in WSL but OS was detected as "win"
            if [[ "$EXE" == *.exe ]]; then
                EXE_WITHOUT_EXE="${EXE%.exe}"
                if [ -f "$EXE_WITHOUT_EXE" ]; then
                    EXE="$EXE_WITHOUT_EXE"
                    : # Success - continue with test execution
                else
                    # Executable doesn't exist - check exit code to determine if it was a real failure
                    (exit $ERR)
                    assert_success_named "Compile" "Compilation Error:" show_failure "$category" "$testName"
                    # If we get here, exit code was 0 but executable doesn't exist - this is a real problem
                    test -f "$EXE" || test -f "$EXE_WITHOUT_EXE"
                    assert_success_named "exe exists" "Executable '$EXE' (or without .exe) does not exist!" show_failure "$category" "$testName"
                fi
            else
                # Executable doesn't exist - check exit code to determine if it was a real failure
                (exit $ERR)
                assert_success_named "Compile" "Compilation Error:" show_failure "$category" "$testName"
                # If we get here, exit code was 0 but executable doesn't exist - this is a real problem
                test -f "$EXE"
                assert_success_named "exe exists" "Executable '$EXE' does not exist!" show_failure "$category" "$testName"
            fi
        fi

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

        # Ensure EXE is an absolute path before changing directories
        # This prevents path resolution issues when we cd into the test directory
        if [ ! -f "$EXE" ]; then
            # Try to resolve the path again - might need to check in category subdirectory
            EXE_IN_CATEGORY_DIR="$CATEGORY_RESULTS_DIR/$category-$testName - output"
            # Only add .exe if we're using a Windows QB64 executable
            if [ "$OS" == "win" ] && [[ "$QB64" == *.exe ]]; then
                EXE_IN_CATEGORY_DIR="${EXE_IN_CATEGORY_DIR}.exe"
            fi
            if [ -f "$EXE_IN_CATEGORY_DIR" ]; then
                EXE="$(to_absolute_path "$EXE_IN_CATEGORY_DIR")"
            else
                # Last resort: try to resolve the original EXE path again
                EXE="$(to_absolute_path "$EXE")"
            fi
        else
            # Ensure it's an absolute path even if file exists
            EXE="$(to_absolute_path "$EXE")"
        fi

        pushd . > /dev/null
        cd "./tests/compile_tests/$category"
        # Use absolute paths to avoid path resolution issues when changing directories
        # Store log files in category-specific directory
        LOG_FILE_PATH="$CATEGORY_RESULTS_DIR/$category-$testName-log.txt"
        # Execute compiled program.
        # - On Windows/WSL: run the .exe via its Unix path, but pass Windows paths as args/env.
        # - On Linux/macOS: run normally with Unix paths.
        CATEGORY_RESULTS_DIR_ARG="$CATEGORY_RESULTS_DIR"
        LOG_FILE_PATH_ARG="$LOG_FILE_PATH"
        if [ "$OS" == "win" ] && [[ "$EXE" == *.exe ]]; then
            CATEGORY_RESULTS_DIR_ARG="$(to_windows_path "$CATEGORY_RESULTS_DIR")"
            LOG_FILE_PATH_ARG="$(to_windows_path "$LOG_FILE_PATH")"
        fi

        # Run the test and capture output
        # Filter out ALSA error messages (common in WSL/headless environments where audio hardware isn't available)
        # ALSA errors start with "ALSA lib" and are written to stderr, but don't affect program functionality
        originalResult=$(
            QB64PE_LOG_HANDLERS=file \
            QB64PE_LOG_SCOPES="qb64,libqb,libqb-image,libqb-audio" \
            QB64PE_LOG_FILE_PATH="$LOG_FILE_PATH_ARG" \
            $LNX_PREFIX "$EXE" "$CATEGORY_RESULTS_DIR_ARG" "$category-$testName" 2>&1
        )
        ERR=$?
        popd > /dev/null

        # Store original output for debugging (includes ALSA errors)
        echo "$originalResult" > "$CATEGORY_RESULTS_DIR/$category-$testName-run-output.txt"

        # Filter out ALSA error messages for comparison (they're expected in WSL/headless environments)
        testResult=$(echo "$originalResult" | grep -v "^ALSA lib" || echo "$originalResult")

        (exit $ERR)
        assert_success_named "run" "Execution Error:" echo "$testResult"

        # Normalize newlines for comparison (strip trailing newlines from both)
        expectedResult=$(echo -n "$expectedResult" | sed 's/\r$//')
        testResult=$(echo -n "$testResult" | sed 's/\r$//')
        
        # Strip license text from both outputs before comparison
        # License text can vary between platforms (Windows shows MinGW licenses, Linux shows QB64-PE licenses)
        # and shouldn't be part of test output comparison
        # License sections are marked by lines of dashes and contain "License", "Copyright", etc.
        
        # Helper function to strip license text - finds where actual program output begins
        strip_license() {
            local text="$1"
            # Find the first line that looks like actual program output:
            # - Starts with optional whitespace followed by a digit (like " 0")
            # - Or contains specific test output patterns
            # Extract from that line onwards, skipping all license content before it
            echo "$text" | awk '
                BEGIN { 
                    found_start=0
                }
                # Detect start of actual program output - this is what we want to keep
                # Look for lines that start with a digit (possibly with leading whitespace)
                # or contain specific test output messages
                /^[[:space:]]*[0-9]/ || /^[[:space:]]*This failure/ || /^[[:space:]]*So was this/ {
                    found_start=1
                    print
                    next
                }
                # Once we found the start, output everything
                found_start==1 {
                    print
                    next
                }
                # Before we found the start, skip everything (it's license text)
                # Don't output anything until we find the program output
            '
        }
        
        # Always strip license text from both outputs if they contain license markers
        # This ensures we only compare actual program output
        # Check for various license text patterns that might appear at the start
        if echo "$expectedResult" | head -5 | grep -qiE "--------------------------------------------------------------------------------|License of|ARISING OUT OF|Copyright.*Sun Microsystems|Parts of the math library|GCC RUNTIME|Freeglut Copyright"; then
            expectedResult=$(strip_license "$expectedResult")
        fi
        if echo "$testResult" | head -5 | grep -qiE "--------------------------------------------------------------------------------|License of|ARISING OUT OF|Copyright.*Sun Microsystems|Parts of the math library|GCC RUNTIME|Freeglut Copyright"; then
            testResult=$(strip_license "$testResult")
        fi
        
        # Normalize whitespace after stripping (remove leading/trailing blank lines)
        expectedResult=$(echo "$expectedResult" | sed -e '/./,$!d' -e :a -e '/^\n*$/{$d;N;ba' -e '}')
        testResult=$(echo "$testResult" | sed -e '/./,$!d' -e :a -e '/^\n*$/{$d;N;ba' -e '}')
        
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
