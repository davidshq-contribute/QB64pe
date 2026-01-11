#!/bin/bash
# Shell script to lint C/C++ code using clang-tidy
# Usage: ./scripts/lint-code.sh [--fix] [--files file1 file2 ...]

set -e

FIX_MODE=false
FILES=()

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --fix)
            FIX_MODE=true
            shift
            ;;
        --files)
            shift
            while [[ $# -gt 0 ]] && [[ ! "$1" =~ ^-- ]]; do
                FILES+=("$1")
                shift
            done
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Check if clang-tidy is available
if ! command -v clang-tidy &> /dev/null; then
    echo "Error: clang-tidy not found. Please install LLVM/Clang tools."
    exit 1
fi

# Get project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Define source directories
SOURCE_DIRS=(
    "$PROJECT_ROOT/internal/c"
    "$PROJECT_ROOT/tests/c"
)

# Get files to lint
if [ ${#FILES[@]} -gt 0 ]; then
    FILES_TO_LINT=()
    for file in "${FILES[@]}"; do
        if [ -f "$file" ]; then
            FILES_TO_LINT+=("$file")
        fi
    done
else
    # Find all C/C++ source files (exclude third-party code)
    FILES_TO_LINT=()
    for dir in "${SOURCE_DIRS[@]}"; do
        if [ -d "$dir" ]; then
            while IFS= read -r -d '' file; do
                # Exclude third-party libraries
                if [[ ! "$file" =~ freetype ]] && \
                   [[ ! "$file" =~ stb ]] && \
                   [[ ! "$file" =~ nanosvg ]] && \
                   [[ ! "$file" =~ qoi ]] && \
                   [[ ! "$file" =~ jo_gif ]] && \
                   [[ ! "$file" =~ modp_b64 ]] && \
                   [[ ! "$file" =~ libmidi ]] && \
                   [[ ! "$file" =~ radv2 ]] && \
                   [[ ! "$file" =~ primesynth ]] && \
                   [[ ! "$file" =~ ymfmidi ]] && \
                   [[ ! "$file" =~ hqx ]] && \
                   [[ ! "$file" =~ mmpx ]] && \
                   [[ ! "$file" =~ sxbr ]]; then
                    FILES_TO_LINT+=("$file")
                fi
            done < <(find "$dir" -type f \( -name "*.c" -o -name "*.cpp" \) -print0)
        fi
    done
fi

if [ ${#FILES_TO_LINT[@]} -eq 0 ]; then
    echo "Warning: No C/C++ files found to lint."
    exit 0
fi

TOTAL_FILES=${#FILES_TO_LINT[@]}
ERRORS=()
WARNINGS=()

echo "Linting $TOTAL_FILES file(s)..."

# Check for compile_commands.json
if [ ! -f "$PROJECT_ROOT/compile_commands.json" ]; then
    echo "Note: compile_commands.json not found. Some checks may be limited."
    echo "      Consider generating it with: bear -- make ..."
fi

for file in "${FILES_TO_LINT[@]}"; do
    RELATIVE_PATH="${file#$PROJECT_ROOT/}"
    echo ""
    echo "Checking: $RELATIVE_PATH"
    
    FIX_FLAG=""
    if [ "$FIX_MODE" = true ]; then
        FIX_FLAG="-fix"
    fi
    
    # Run clang-tidy and capture output
    # Note: clang-tidy may return non-zero on warnings, so we capture output separately
    # Use a temporary file to capture both stdout and stderr while preserving exit code
    TEMP_OUTPUT=$(mktemp)
    set +e  # Temporarily disable exit on error to capture exit code
    clang-tidy $FIX_FLAG "$file" > "$TEMP_OUTPUT" 2>&1
    EXIT_CODE=$?
    set -e  # Re-enable exit on error
    OUTPUT=$(cat "$TEMP_OUTPUT")
    rm -f "$TEMP_OUTPUT"
    
    # Parse output for errors and warnings
    FILE_ERRORS=$(echo "$OUTPUT" | grep "error:" || true)
    FILE_WARNINGS=$(echo "$OUTPUT" | grep "warning:" || true)
    
    if [ -n "$FILE_ERRORS" ]; then
        while IFS= read -r line; do
            ERRORS+=("$line")
            echo "  $line" | sed 's/error:/[ERROR]/' | sed 's/^/  /'
        done <<< "$FILE_ERRORS"
    fi
    
    if [ -n "$FILE_WARNINGS" ]; then
        while IFS= read -r line; do
            WARNINGS+=("$line")
            echo "  $line" | sed 's/warning:/[WARNING]/' | sed 's/^/  /'
        done <<< "$FILE_WARNINGS"
    fi
    
    # Check for actual clang-tidy failures (not just warnings)
    if [ $EXIT_CODE -ne 0 ] && [ -z "$FILE_ERRORS" ] && [ -z "$FILE_WARNINGS" ]; then
        ERRORS+=("clang-tidy failed for $RELATIVE_PATH (exit code: $EXIT_CODE)")
        echo "  [ERROR] clang-tidy failed (exit code: $EXIT_CODE)"
        if [ -n "$OUTPUT" ]; then
            echo "$OUTPUT" | head -5
        fi
    elif [ -z "$FILE_ERRORS" ] && [ -z "$FILE_WARNINGS" ]; then
        echo "  [OK] No issues found"
    fi
done

echo ""
echo "============================================================"
echo "Linting Summary:"
echo "  Files checked: $TOTAL_FILES"
echo "  Errors: ${#ERRORS[@]}"
echo "  Warnings: ${#WARNINGS[@]}"

if [ ${#ERRORS[@]} -gt 0 ]; then
    echo ""
    echo "Linting failed with errors."
    exit 1
elif [ ${#WARNINGS[@]} -gt 0 ]; then
    echo ""
    echo "Linting completed with warnings."
    exit 0
else
    echo ""
    echo "Linting passed!"
    exit 0
fi
