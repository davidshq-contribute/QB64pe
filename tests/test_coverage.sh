#!/bin/bash
# Test Coverage Analysis Tool
# Analyzes code coverage by mapping tests to source files and generating coverage reports
#
# Requirements:
#   - jq: JSON processor (required for HTML and text reports)
#     * On Linux/macOS: Install via package manager (apt, brew, etc.)
#     * On Windows: WSL is recommended for best compatibility
#     * On Git Bash: Ensure jq is in PATH or use WSL
#     * Install: sudo apt install jq (Ubuntu/Debian) or brew install jq (macOS)
#
# Usage:
#   ./tests/test_coverage.sh [options]
#
# Options:
#   --analyze           Analyze coverage (default action)
#   --report            Generate coverage report
#   --format FORMAT     Report format: html, text, json (default: html)
#   --output FILE       Output file path
#   --min-coverage NUM  Minimum coverage threshold (percentage)
#   --source-dir DIR    Source directory to analyze (default: source/)
#   --c-source-dir DIR  C++ source directory (default: internal/c/)
#   --verbose           Verbose output
#   --help              Show help

# Error handling: exit on error, undefined variables, and pipe failures
set -euo pipefail

# Default values
COVERAGE_ACTION="analyze"
REPORT_FORMAT="html"
OUTPUT_FILE=""
MIN_COVERAGE=""
SOURCE_DIR="./source"
C_SOURCE_DIR="./internal/c"
VERBOSE=0

# Coverage data storage
COVERAGE_DATA_DIR="./tests/coverage_data"
COVERAGE_JSON="$COVERAGE_DATA_DIR/coverage.json"
COVERAGE_MAP="$COVERAGE_DATA_DIR/coverage_map.txt"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --analyze)
            COVERAGE_ACTION="analyze"
            shift
            ;;
        --report)
            COVERAGE_ACTION="report"
            shift
            ;;
        --format)
            REPORT_FORMAT="$2"
            shift 2
            ;;
        --output)
            OUTPUT_FILE="$2"
            shift 2
            ;;
        --min-coverage)
            MIN_COVERAGE="$2"
            shift 2
            ;;
        --source-dir)
            SOURCE_DIR="$2"
            shift 2
            ;;
        --c-source-dir)
            C_SOURCE_DIR="$2"
            shift 2
            ;;
        --verbose)
            VERBOSE=1
            shift
            ;;
        --help)
            cat << EOF
Test Coverage Analysis Tool

Usage: $0 [options]

Options:
  --analyze              Analyze coverage (default action)
  --report               Generate coverage report
  --format FORMAT        Report format: html, text, json (default: html)
  --output FILE          Output file path
  --min-coverage NUM     Minimum coverage threshold (percentage)
  --source-dir DIR       Source directory to analyze (default: source/)
  --c-source-dir DIR     C++ source directory (default: internal/c/)
  --verbose              Verbose output
  --help                 Show this help

Examples:
  $0 --analyze                    # Analyze coverage
  $0 --report --format html      # Generate HTML coverage report
  $0 --report --format text      # Generate text coverage report
  $0 --analyze --report          # Analyze and generate report
EOF
            exit 0
            ;;
        *)
            echo "Unknown option: $1" >&2
            echo "Use --help for usage information" >&2
            exit 1
            ;;
    esac
done

# Create coverage data directory
if ! mkdir -p "$COVERAGE_DATA_DIR"; then
    echo "Error: Cannot create coverage data directory: $COVERAGE_DATA_DIR" >&2
    exit 1
fi

# Verbose output helper
verbose_echo() {
    if [ "$VERBOSE" -eq 1 ]; then
        echo "$@" >&2
    fi
}

# Helper function to escape JSON strings properly
# Uses jq if available, otherwise uses sed-based escaping
# Returns a properly quoted and escaped JSON string
escape_json_string() {
    local str="$1"
    if command -v jq >/dev/null 2>&1; then
        # jq handles all JSON escaping automatically and returns quoted string
        echo "$str" | jq -Rs .
    else
        # Fallback: manual escaping (handles quotes, backslashes, newlines, tabs, carriage returns)
        # Must escape backslashes first
        str=$(printf '%s' "$str" | sed 's/\\/\\\\/g')
        # Escape quotes
        str=$(printf '%s' "$str" | sed 's/"/\\"/g')
        # Escape newlines (handle multiline strings)
        str=$(printf '%s' "$str" | tr '\n' '\001' | sed 's/\001/\\n/g')
        # Escape tabs
        str=$(printf '%s' "$str" | sed 's/\t/\\t/g')
        # Escape carriage returns
        str=$(printf '%s' "$str" | sed 's/\r/\\r/g')
        # Return quoted string
        echo "\"$str\""
    fi
}

# Helper function to normalize paths robustly
# Handles Windows paths, spaces, and relative paths
normalize_path() {
    local path="$1"
    
    # If path is empty, return empty string
    [ -z "$path" ] && { echo ""; return 1; }
    
    # Try to convert to absolute path
    if dir_abs="$(cd "$(dirname "$path")" 2>/dev/null && pwd)"; then
        local result="$dir_abs/$(basename "$path")"
        echo "$result"
    else
        # Fallback: return as-is
        echo "$path"
    fi
}

# Find all source files to track
find_source_files() {
    local base_dir="$1"
    local extension="$2"
    
    if [ ! -d "$base_dir" ]; then
        return
    fi
    
    find "$base_dir" -name "*.$extension" -type f 2>/dev/null | sort
}

# Analyze which source files are referenced by a test
analyze_test_coverage() {
    local test_file="$1"
    local test_category="$2"
    local test_name="$3"
    
    verbose_echo "Analyzing test: $test_name ($test_category)"
    
    local covered_files=()
    
    # For QB64 tests (.bas files), check for $INCLUDE statements and function calls
    if [[ "$test_file" == *.bas ]]; then
        # Get test file directory for resolving relative paths
        local test_dir=$(dirname "$test_file")
        
        # Extract $INCLUDE statements
        while IFS= read -r line; do
            # Match $INCLUDE patterns: '$INCLUDE: 'file.bi' or $INCLUDE 'file.bi' or '$INCLUDE 'file.bi'
            # Pattern allows optional leading quote, then $INCLUDE, optional colon, then quoted filename
            if [[ "$line" =~ [\'\"]?\$INCLUDE[[:space:]:]*[\'\"]([^\'\"]+)[\'\"] ]]; then
                local included_file="${BASH_REMATCH[1]}"
                # Try to find the actual file
                local found_file=""
                
                # First, try relative to test file directory
                if [[ "$included_file" == ../* ]] || [[ "$included_file" == ./* ]]; then
                    local resolved_path=""
                    # Try realpath first (Linux, modern systems)
                    if command -v realpath >/dev/null 2>&1; then
                        resolved_path=$(cd "$test_dir" 2>/dev/null && realpath "$included_file" 2>/dev/null)
                    # Fallback: try readlink -f (Linux alternative)
                    elif command -v readlink >/dev/null 2>&1 && readlink -f / >/dev/null 2>&1; then
                        resolved_path=$(cd "$test_dir" 2>/dev/null && readlink -f "$included_file" 2>/dev/null)
                    # Fallback: construct path manually and check if it exists
                    else
                        # Try to resolve by changing to the directory and checking
                        local check_path=$(cd "$test_dir" 2>/dev/null && cd "$(dirname "$included_file")" 2>/dev/null && echo "$(pwd)/$(basename "$included_file")")
                        if [ -n "$check_path" ] && [ -f "$check_path" ]; then
                            resolved_path="$check_path"
                        fi
                    fi
                    if [ -n "$resolved_path" ] && [ -f "$resolved_path" ]; then
                        found_file="$resolved_path"
                    fi
                fi
                
                # If not found, check common include paths
                if [ -z "$found_file" ]; then
                    for search_dir in "$SOURCE_DIR" "$SOURCE_DIR/utilities" "$SOURCE_DIR/ide" "$SOURCE_DIR/global" "$test_dir" "$(dirname "$test_dir")"; do
                        if [ -f "$search_dir/$included_file" ]; then
                            found_file="$search_dir/$included_file"
                            break
                        fi
                    done
                fi
                
                if [ -n "$found_file" ]; then
                    # Normalize the path
                    found_file=$(normalize_path "$found_file")
                    covered_files+=("$found_file")
                fi
            fi
        done < "$test_file"
        
        # Check for function/sub names that might indicate source file usage
        # This is a heuristic - we look for common QB64 function patterns
        # and map them to likely source files based on naming conventions
        while IFS= read -r line; do
            # Look for function calls that might indicate coverage
            # This is simplified - a full implementation would parse the AST
            if [[ "$line" =~ (SUB|FUNCTION|TYPE)\s+([A-Za-z_][A-Za-z0-9_]*) ]]; then
                local func_name="${BASH_REMATCH[2]}"
                # Map function names to likely source files (heuristic)
                case "$func_name" in
                    *hash*|*Hash*)
                        if [ -f "$SOURCE_DIR/utilities/hash.bas" ]; then
                            covered_files+=("$SOURCE_DIR/utilities/hash.bas")
                        fi
                        ;;
                    *type*|*Type*)
                        if [ -f "$SOURCE_DIR/utilities/type.bas" ]; then
                            covered_files+=("$SOURCE_DIR/utilities/type.bas")
                        fi
                        ;;
                    *const*|*Const*)
                        if [ -f "$SOURCE_DIR/utilities/const_eval.bas" ]; then
                            covered_files+=("$SOURCE_DIR/utilities/const_eval.bas")
                        fi
                        ;;
                    *include*|*Include*)
                        if [ -f "$SOURCE_DIR/utilities/include_provider.bas" ]; then
                            covered_files+=("$SOURCE_DIR/utilities/include_provider.bas")
                        fi
                        ;;
                esac
            fi
        done < "$test_file"
    fi
    
    # For C++ tests (.cpp files), check for #include statements
    if [[ "$test_file" == *.cpp ]]; then
        while IFS= read -r line; do
            # Match #include patterns
            if [[ "$line" =~ ^#include[[:space:]]+[\"\'\<]([^\>\"\']+)[\"\'\>] ]]; then
                local included_file="${BASH_REMATCH[1]}"
                local found_file=""
                
                # Check C++ source directories
                for search_dir in "$C_SOURCE_DIR" "$C_SOURCE_DIR/libqb" "$C_SOURCE_DIR/parts"; do
                    if [ -f "$search_dir/$included_file" ]; then
                        found_file="$search_dir/$included_file"
                        break
                    fi
                    # Also check without extension
                    if [ -f "$search_dir/${included_file%.h}.cpp" ]; then
                        found_file="$search_dir/${included_file%.h}.cpp"
                        break
                    fi
                done
                
                if [ -n "$found_file" ]; then
                    covered_files+=("$found_file")
                fi
            fi
        done < "$test_file"
    fi
    
    # Remove duplicates and return
    printf '%s\n' "${covered_files[@]}" | sort -u
}

# Build coverage map from all tests
build_coverage_map() {
    verbose_echo "Building coverage map..."
    
    # Source test discovery if available
    if [ -f "./tests/test_discovery.sh" ]; then
        source "./tests/test_discovery.sh"
    fi
    
    # Initialize coverage map
    > "$COVERAGE_MAP"
    
    # Get all tests using manual discovery (discover_tests formats output, so we use manual search)
    local all_tests=""
    
    # Manual discovery - find all test files
    for test_dir in "./tests/compile_tests" "./tests/unit" "./tests/integration" "./tests/format_tests"; do
        if [ -d "$test_dir" ]; then
            while IFS= read -r test_file; do
                local category=$(basename "$(dirname "$test_file")")
                local name=$(basename "$test_file" .bas)
                all_tests="${all_tests}${test_file}|${category}|${name}"$'\n'
            done < <(find "$test_dir" -name "*.bas" -type f 2>/dev/null)
        fi
    done
    
    # Add C++ tests
    if [ -d "./tests/c" ]; then
        while IFS= read -r test_file; do
            local name=$(basename "$test_file" .cpp)
            all_tests="${all_tests}${test_file}|runtime|${name}"$'\n'
        done < <(find "./tests/c" -name "*.cpp" -type f ! -name "test.cpp" 2>/dev/null)
    fi
    
    # Process each test
    local total_tests=0
    while IFS='|' read -r test_file test_category test_name test_tags; do
        [ -z "$test_file" ] && continue
        [ ! -f "$test_file" ] && continue
        
        total_tests=$((total_tests + 1))
        verbose_echo "Processing test $total_tests: $test_name"
        
        # Analyze coverage for this test
        local covered_files
        covered_files=$(analyze_test_coverage "$test_file" "$test_category" "$test_name")
        
        # Store in coverage map
        if [ -n "$covered_files" ]; then
            while IFS= read -r covered_file; do
                [ -z "$covered_file" ] && continue
                echo "$test_file|$test_category|$test_name|$covered_file" >> "$COVERAGE_MAP"
            done <<< "$covered_files"
        fi
    done <<< "$all_tests"
    
    verbose_echo "Processed $total_tests tests"
}

# Calculate coverage statistics
calculate_coverage_stats() {
    verbose_echo "Calculating coverage statistics..."
    
    # Find all source files
    local all_source_files=()
    while IFS= read -r file; do
        all_source_files+=("$file")
    done < <(find_source_files "$SOURCE_DIR" "bas")
    while IFS= read -r file; do
        all_source_files+=("$file")
    done < <(find_source_files "$SOURCE_DIR" "bi")
    while IFS= read -r file; do
        all_source_files+=("$file")
    done < <(find_source_files "$C_SOURCE_DIR/libqb" "cpp")
    while IFS= read -r file; do
        all_source_files+=("$file")
    done < <(find_source_files "$C_SOURCE_DIR/libqb" "h")
    
    # Count total files
    local total_files=${#all_source_files[@]}
    
    # Find covered files from coverage map
    local covered_files_set=()
    if [ -f "$COVERAGE_MAP" ]; then
        while IFS='|' read -r test_file test_category test_name covered_file; do
            # Validate all fields are present
            [ -z "$test_file" ] || [ -z "$test_category" ] || [ -z "$test_name" ] || [ -z "$covered_file" ] && continue
            # Normalize path
            covered_file=$(normalize_path "$covered_file")
            covered_files_set+=("$covered_file")
        done < "$COVERAGE_MAP"
    fi
    
    # Remove duplicates
    local unique_covered_files
    unique_covered_files=$(printf '%s\n' "${covered_files_set[@]}" | sort -u)
    local covered_count=0
    if [ -n "$unique_covered_files" ]; then
        covered_count=$(echo "$unique_covered_files" | grep -c . || echo "0")
    fi
    
    # Calculate coverage percentage
    local coverage_percent=0
    if [ "$total_files" -gt 0 ]; then
        coverage_percent=$((covered_count * 100 / total_files))
    fi
    
    # Generate JSON coverage data
    {
        echo "{"
        echo "  \"timestamp\": \"$(date -u +"%Y-%m-%dT%H:%M:%SZ")\","
        echo "  \"summary\": {"
        echo "    \"total_files\": $total_files,"
        echo "    \"covered_files\": $covered_count,"
        echo "    \"uncovered_files\": $((total_files - covered_count)),"
        echo "    \"coverage_percent\": $coverage_percent"
        echo "  },"
        echo "  \"files\": ["
        
        local first=1
        for source_file in "${all_source_files[@]}"; do
            # Normalize path for comparison
            local normalized_source
            normalized_source=$(normalize_path "$source_file")
            
            # Check if covered
            local is_covered=0
            if echo "$unique_covered_files" | grep -Fxq "$normalized_source" 2>/dev/null; then
                is_covered=1
            fi
            
            # Get tests that cover this file
            local covering_tests=()
            if [ -f "$COVERAGE_MAP" ]; then
                while IFS='|' read -r test_file test_category test_name covered_file; do
                    # Validate all fields are present
                    [ -z "$test_file" ] || [ -z "$test_category" ] || [ -z "$test_name" ] || [ -z "$covered_file" ] && continue
                    local normalized_covered
                    normalized_covered=$(normalize_path "$covered_file")
                    if [ "$normalized_covered" = "$normalized_source" ]; then
                        covering_tests+=("$test_category:$test_name")
                    fi
                done < "$COVERAGE_MAP"
            fi
            
            if [ "$first" -eq 0 ]; then
                echo ","
            fi
            first=0
            
            # Escape file path for JSON
            local escaped_file
            escaped_file=$(escape_json_string "$source_file")
            
            echo "    {"
            echo "      \"file\": $escaped_file,"
            echo "      \"covered\": $is_covered,"
            echo "      \"covering_tests\": ["
            local first_test=1
            for test in "${covering_tests[@]}"; do
                if [ "$first_test" -eq 0 ]; then
                    echo ","
                fi
                first_test=0
                # Escape test name for JSON
                local escaped_test
                escaped_test=$(escape_json_string "$test")
                echo "        $escaped_test"
            done
            echo "      ]"
            echo -n "    }"
        done
        
        echo ""
        echo "  ]"
        echo "}"
    } > "$COVERAGE_JSON"
    
    echo "$coverage_percent|$total_files|$covered_count"
}

# Helper function to check if jq is available
# Sets jq_available (0 or 1) and jq_cmd (command to use) as output variables
# 
# Requirements:
#   - jq must be installed and available in PATH
#   - On Windows, WSL is recommended for best compatibility
#   - On Git Bash, ensure jq is in PATH or use WSL
#
# Usage:
#   local jq_available jq_cmd
#   check_jq_available jq_available jq_cmd
check_jq_available() {
    local __jq_available_var=$1
    local __jq_cmd_var=$2
    local detected_available=0
    local detected_cmd="jq"
    
    # Check if jq is available via command -v (standard method)
    if command -v jq >/dev/null 2>&1; then
        detected_available=1
        detected_cmd="jq"
    # Fallback: try running jq directly (handles cases where command -v fails)
    elif jq --version >/dev/null 2>&1; then
        detected_available=1
        detected_cmd="jq"
    fi
    
    # Return values via eval (set the variables in the caller's scope)
    eval "$__jq_available_var=$detected_available"
    eval "$__jq_cmd_var=\"$detected_cmd\""
}

# Generate HTML coverage report
generate_html_coverage_report() {
    local output_file="$1"
    
    verbose_echo "Generating HTML coverage report..."
    
    if [ ! -f "$COVERAGE_JSON" ]; then
        echo "Error: Coverage data not found. Run --analyze first." >&2
        return 1
    fi
    
    # Check if jq is available using shared helper
    local jq_available=0
    local jq_cmd="jq"
    check_jq_available jq_available jq_cmd
    # Ensure variables are set (printf might fail in some shells)
    jq_available=${jq_available:-0}
    jq_cmd=${jq_cmd:-jq}
    
    {
        cat << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>QB64-PE Code Coverage Report</title>
    <meta charset="UTF-8">
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif; 
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1400px;
            margin: 0 auto;
            background: white;
            padding: 30px;
            border-radius: 8px;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        h1 { 
            color: #333; 
            margin-top: 0;
            border-bottom: 3px solid #4CAF50;
            padding-bottom: 10px;
        }
        h2 {
            color: #555;
            margin-top: 30px;
        }
        .summary {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 20px;
            margin: 20px 0;
        }
        .summary-card {
            padding: 20px;
            border-radius: 6px;
            text-align: center;
        }
        .summary-card.total { background-color: #e3f2fd; }
        .summary-card.covered { background-color: #e8f5e9; }
        .summary-card.uncovered { background-color: #ffebee; }
        .summary-card.coverage { background-color: #fff3e0; }
        .summary-card h3 {
            margin: 0 0 10px 0;
            font-size: 14px;
            color: #666;
            text-transform: uppercase;
        }
        .summary-card .value {
            font-size: 36px;
            font-weight: bold;
            color: #333;
        }
        .coverage-bar {
            height: 30px;
            background-color: #e0e0e0;
            border-radius: 15px;
            overflow: hidden;
            margin: 20px 0;
            position: relative;
        }
        .coverage-fill {
            height: 100%;
            background: linear-gradient(90deg, #4CAF50 0%, #8BC34A 100%);
            transition: width 0.5s;
            display: flex;
            align-items: center;
            justify-content: center;
            color: white;
            font-weight: bold;
        }
        .coverage-fill.low {
            background: linear-gradient(90deg, #f44336 0%, #ff9800 100%);
        }
        .coverage-fill.medium {
            background: linear-gradient(90deg, #ff9800 0%, #ffc107 100%);
        }
        table { 
            border-collapse: collapse; 
            width: 100%; 
            margin-top: 10px;
        }
        th, td { 
            border: 1px solid #ddd; 
            padding: 12px; 
            text-align: left; 
        }
        th { 
            background-color: #f2f2f2;
            font-weight: 600;
        }
        tr:hover {
            background-color: #f9f9f9;
        }
        .file-covered { color: #4CAF50; font-weight: bold; }
        .file-uncovered { color: #f44336; font-weight: bold; }
        .test-list {
            font-size: 12px;
            color: #666;
            margin-top: 5px;
        }
        .timestamp {
            color: #666;
            font-size: 14px;
            margin-bottom: 20px;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>QB64-PE Code Coverage Report</h1>
        <div class="timestamp">Generated: 
EOF
        date
        cat << 'EOF'
        </div>
        
        <h2>Summary</h2>
        <div class="summary">
            <div class="summary-card total">
                <h3>Total Files</h3>
                <div class="value" id="total-files">0</div>
            </div>
            <div class="summary-card covered">
                <h3>Covered Files</h3>
                <div class="value" id="covered-files">0</div>
            </div>
            <div class="summary-card uncovered">
                <h3>Uncovered Files</h3>
                <div class="value" id="uncovered-files">0</div>
            </div>
            <div class="summary-card coverage">
                <h3>Coverage</h3>
                <div class="value" id="coverage-percent">0%</div>
            </div>
        </div>
        
        <div class="coverage-bar">
            <div class="coverage-fill" id="coverage-fill" style="width: 0%">0%</div>
        </div>
        
        <h2>File Coverage Details</h2>
        <table>
            <tr>
                <th>File</th>
                <th>Status</th>
                <th>Covering Tests</th>
            </tr>
EOF
        
        # Extract and display file coverage data
        if [ "$jq_available" -eq 1 ]; then
            # Use jq for robust JSON parsing
            local total_files covered_files uncovered_files coverage_percent
            total_files=$("$jq_cmd" -r '.summary.total_files' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            covered_files=$("$jq_cmd" -r '.summary.covered_files' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            uncovered_files=$("$jq_cmd" -r '.summary.uncovered_files' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            coverage_percent=$("$jq_cmd" -r '.summary.coverage_percent' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            
            cat << EOF
        <script>
            document.getElementById('total-files').textContent = '$total_files';
            document.getElementById('covered-files').textContent = '$covered_files';
            document.getElementById('uncovered-files').textContent = '$uncovered_files';
            document.getElementById('coverage-percent').textContent = '$coverage_percent%';
            
            var coveragePercent = $coverage_percent;
            var fillElement = document.getElementById('coverage-fill');
            fillElement.style.width = coveragePercent + '%';
            fillElement.textContent = coveragePercent + '%';
            
            if (coveragePercent < 50) {
                fillElement.classList.add('low');
            } else if (coveragePercent < 80) {
                fillElement.classList.add('medium');
            }
        </script>
EOF
            
            # Generate file rows
            "$jq_cmd" -r '.files[] | "\(.file)|\(.covered)|\(.covering_tests | join(","))"' "$COVERAGE_JSON" 2>/dev/null | while IFS='|' read -r file covered tests; do
                local status_class="file-uncovered"
                local status_text="Not Covered"
                if [ "$covered" = "1" ]; then
                    status_class="file-covered"
                    status_text="Covered"
                fi
                
                echo "            <tr>"
                echo "                <td>$file</td>"
                echo "                <td class=\"$status_class\">$status_text</td>"
                echo "                <td>"
                if [ -n "$tests" ] && [ "$tests" != "null" ]; then
                    IFS=',' read -ra test_array <<< "$tests"
                    for test in "${test_array[@]}"; do
                        echo "                    <div class=\"test-list\">• $test</div>"
                    done
                else
                    echo "                    <span class=\"test-list\">None</span>"
                fi
                echo "                </td>"
                echo "            </tr>"
            done
        else
            # Fallback: basic parsing without jq
            echo "            <tr><td colspan=\"3\">JSON parsing requires jq. Install jq for full report.</td></tr>"
        fi
        
        cat << 'EOF'
        </table>
    </div>
</body>
</html>
EOF
    } > "$output_file"
    
    echo "HTML coverage report generated: $output_file"
}

# Generate text coverage report
generate_text_coverage_report() {
    local output_file="$1"
    
    verbose_echo "Generating text coverage report..."
    
    if [ ! -f "$COVERAGE_JSON" ]; then
        echo "Error: Coverage data not found. Run --analyze first." >&2
        return 1
    fi
    
    # Check if jq is available using shared helper
    local jq_available=0
    local jq_cmd="jq"
    check_jq_available jq_available jq_cmd
    # Ensure variables are set (printf might fail in some shells)
    jq_available=${jq_available:-0}
    jq_cmd=${jq_cmd:-jq}
    
    {
        echo "QB64-PE Code Coverage Report"
        echo "==========================="
        echo "Generated: $(date)"
        echo ""
        echo "Summary:"
        echo "--------"
        
        if [ "$jq_available" -eq 1 ]; then
            local total_files covered_files uncovered_files coverage_percent
            total_files=$("$jq_cmd" -r '.summary.total_files' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            covered_files=$("$jq_cmd" -r '.summary.covered_files' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            uncovered_files=$("$jq_cmd" -r '.summary.uncovered_files' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            coverage_percent=$("$jq_cmd" -r '.summary.coverage_percent' "$COVERAGE_JSON" 2>/dev/null || echo "0")
            
            echo "Total Files:     $total_files"
            echo "Covered Files:  $covered_files"
            echo "Uncovered Files: $uncovered_files"
            echo "Coverage:       ${coverage_percent}%"
            echo ""
            echo "File Coverage Details:"
            echo "---------------------"
            
            "$jq_cmd" -r '.files[] | "\(.file)|\(.covered)|\(.covering_tests | join(","))"' "$COVERAGE_JSON" 2>/dev/null | while IFS='|' read -r file covered tests; do
                local status="[NOT COVERED]"
                if [ "$covered" = "1" ]; then
                    status="[COVERED]"
                fi
                
                printf "%-60s %s\n" "$file" "$status"
                if [ -n "$tests" ] && [ "$tests" != "null" ]; then
                    IFS=',' read -ra test_array <<< "$tests"
                    for test in "${test_array[@]}"; do
                        echo "    - $test"
                    done
                fi
            done
        else
            echo "JSON parsing requires jq. Install jq for full report."
        fi
    } > "$output_file"
    
    echo "Text coverage report generated: $output_file"
}

# Main execution
main() {
    case "$COVERAGE_ACTION" in
        analyze)
            echo "Analyzing test coverage..."
            build_coverage_map
            local stats
            stats=$(calculate_coverage_stats)
            local coverage_percent=$(echo "$stats" | cut -d'|' -f1)
            local total_files=$(echo "$stats" | cut -d'|' -f2)
            local covered_files=$(echo "$stats" | cut -d'|' -f3)
            
            echo ""
            echo "Coverage Analysis Complete"
            echo "=========================="
            echo "Total Files:     $total_files"
            echo "Covered Files:   $covered_files"
            echo "Coverage:        ${coverage_percent}%"
            echo ""
            echo "Coverage data saved to: $COVERAGE_JSON"
            
            # Check minimum coverage threshold if specified
            if [ -n "$MIN_COVERAGE" ]; then
                if [ "$coverage_percent" -lt "$MIN_COVERAGE" ]; then
                    echo "WARNING: Coverage ${coverage_percent}% is below minimum threshold of ${MIN_COVERAGE}%" >&2
                    return 1
                fi
            fi
            ;;
        report)
            if [ -z "$OUTPUT_FILE" ]; then
                # Default output file based on format
                case "$REPORT_FORMAT" in
                    html)
                        OUTPUT_FILE="./tests/coverage_data/coverage_report.html"
                        ;;
                    text)
                        OUTPUT_FILE="./tests/coverage_data/coverage_report.txt"
                        ;;
                    json)
                        OUTPUT_FILE="./tests/coverage_data/coverage_report.json"
                        ;;
                esac
            fi
            
            case "$REPORT_FORMAT" in
                html)
                    generate_html_coverage_report "$OUTPUT_FILE"
                    ;;
                text)
                    generate_text_coverage_report "$OUTPUT_FILE"
                    ;;
                json)
                    if [ -f "$COVERAGE_JSON" ]; then
                        cp "$COVERAGE_JSON" "$OUTPUT_FILE"
                        echo "JSON coverage report generated: $OUTPUT_FILE"
                    else
                        echo "Error: Coverage data not found. Run --analyze first." >&2
                        return 1
                    fi
                    ;;
                *)
                    echo "Error: Invalid format '$REPORT_FORMAT'. Must be html, text, or json." >&2
                    return 1
                    ;;
            esac
            ;;
        *)
            echo "Error: Unknown action '$COVERAGE_ACTION'" >&2
            return 1
            ;;
    esac
}

# Run main function
main "$@"
