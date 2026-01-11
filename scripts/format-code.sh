#!/bin/bash
# Shell script to format C/C++ code using clang-format
# Usage: ./scripts/format-code.sh [--check] [--files file1 file2 ...]

set -e

CHECK_MODE=false
FILES=()

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --check)
            CHECK_MODE=true
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

# Check if clang-format is available
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format not found. Please install LLVM/Clang tools."
    exit 1
fi

# Get project root (parent of scripts directory)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# Define source directories
SOURCE_DIRS=(
    "$PROJECT_ROOT/internal/c"
    "$PROJECT_ROOT/tests/c"
)

# Get files to format
if [ ${#FILES[@]} -gt 0 ]; then
    FILES_TO_FORMAT=()
    for file in "${FILES[@]}"; do
        if [ -f "$file" ]; then
            FILES_TO_FORMAT+=("$file")
        fi
    done
else
    # Find all C/C++ source files (exclude third-party libraries)
    FILES_TO_FORMAT=()
    for dir in "${SOURCE_DIRS[@]}"; do
        if [ -d "$dir" ]; then
            while IFS= read -r -d '' file; do
                # Exclude third-party code
                if [[ ! "$file" =~ freetype ]] && \
                   [[ ! "$file" =~ stb ]] && \
                   [[ ! "$file" =~ nanosvg ]] && \
                   [[ ! "$file" =~ qoi ]] && \
                   [[ ! "$file" =~ jo_gif ]]; then
                    FILES_TO_FORMAT+=("$file")
                fi
            done < <(find "$dir" -type f \( -name "*.c" -o -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -print0)
        fi
    done
fi

if [ ${#FILES_TO_FORMAT[@]} -eq 0 ]; then
    echo "Warning: No C/C++ files found to format."
    exit 0
fi

TOTAL_FILES=${#FILES_TO_FORMAT[@]}
MODIFIED_FILES=()
ERRORS=()

echo "Formatting $TOTAL_FILES file(s)..."

for file in "${FILES_TO_FORMAT[@]}"; do
    RELATIVE_PATH="${file#$PROJECT_ROOT/}"
    
    if [ "$CHECK_MODE" = true ]; then
        # Check if file is formatted correctly
        FORMATTED=$(clang-format "$file" 2>/dev/null)
        if [ $? -ne 0 ]; then
            ERRORS+=("$RELATIVE_PATH")
            echo "  [ERROR] Failed to format check $RELATIVE_PATH"
        elif ! echo "$FORMATTED" | diff -q "$file" - > /dev/null 2>&1; then
            MODIFIED_FILES+=("$RELATIVE_PATH")
            echo "  [NEEDS FORMAT] $RELATIVE_PATH"
        else
            echo "  [OK] $RELATIVE_PATH"
        fi
    else
        # Format the file
        if clang-format -i "$file"; then
            echo "  [FORMATTED] $RELATIVE_PATH"
        else
            ERRORS+=("$RELATIVE_PATH")
            echo "  [ERROR] $RELATIVE_PATH"
        fi
    fi
done

if [ "$CHECK_MODE" = true ]; then
    if [ ${#MODIFIED_FILES[@]} -gt 0 ]; then
        echo ""
        echo "${#MODIFIED_FILES[@]} file(s) need formatting:"
        for file in "${MODIFIED_FILES[@]}"; do
            echo "  - $file"
        done
        echo ""
        echo "Run without --check to format these files."
        exit 1
    else
        echo ""
        echo "All files are properly formatted!"
        exit 0
    fi
else
    if [ ${#ERRORS[@]} -gt 0 ]; then
        echo ""
        echo "Errors occurred:"
        for file in "${ERRORS[@]}"; do
            echo "  - $file"
        done
        exit 1
    else
        echo ""
        echo "Formatting complete! Formatted $TOTAL_FILES file(s)."
        exit 0
    fi
fi
