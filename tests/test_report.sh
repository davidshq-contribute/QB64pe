#!/bin/bash
# Test Report Generator
# Generates HTML and text reports from test results with detailed coverage

# Error handling: exit on error, undefined variables, and pipe failures
set -euo pipefail

# Trap to handle errors and cleanup
cleanup_on_error() {
    local exit_code=$?
    if [ $exit_code -ne 0 ]; then
        echo "Error: Test report generation failed with exit code $exit_code" >&2
    fi
    exit $exit_code
}
trap cleanup_on_error ERR EXIT

# Check if jq is available for robust JSON parsing
JQ_AVAILABLE=0
if command -v jq >/dev/null 2>&1; then
    JQ_AVAILABLE=1
fi

# Helper function to escape JSON strings properly
# Uses jq if available, otherwise uses sed-based escaping
# Returns a properly quoted and escaped JSON string
escape_json_string() {
    local str="$1"
    if [ "$JQ_AVAILABLE" -eq 1 ]; then
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

# Helper function to extract JSON values using jq with fallback
# Usage: extract_json_value <json_file> <jq_path> [fallback_value]
extract_json_value() {
    local json_file="$1"
    local jq_path="$2"
    local fallback="${3:-0}"
    
    if [ ! -f "$json_file" ]; then
        echo "$fallback"
        return
    fi
    
    if [ "$JQ_AVAILABLE" -eq 1 ]; then
        # Use jq for robust JSON parsing
        jq -r "$jq_path // $fallback" "$json_file" 2>/dev/null || echo "$fallback"
    else
        # Fallback: grep-based extraction (fragile but works for simple cases)
        case "$jq_path" in
            ".summary.total")
                grep -o '"total": [0-9]*' "$json_file" | tail -1 | grep -o '[0-9]*' || echo "$fallback"
                ;;
            ".summary.passed")
                grep -o '"passed": [0-9]*' "$json_file" | tail -1 | grep -o '[0-9]*' || echo "$fallback"
                ;;
            ".summary.failed")
                grep -o '"failed": [0-9]*' "$json_file" | tail -1 | grep -o '[0-9]*' || echo "$fallback"
                ;;
            ".summary.skipped")
                grep -o '"skipped": [0-9]*' "$json_file" | tail -1 | grep -o '[0-9]*' || echo "$fallback"
                ;;
            *)
                # For category-specific paths, use grep with pattern matching
                if echo "$jq_path" | grep -q '\.categories\.".*"\.summary\.'; then
                    local category=$(echo "$jq_path" | sed 's/.*\.categories\."\([^"]*\)".*/\1/')
                    local field=$(echo "$jq_path" | sed 's/.*\.summary\.\(.*\)/\1/')
                    grep -A 10 "\"$category\"" "$json_file" | grep -o "\"$field\": [0-9]*" | head -1 | grep -o '[0-9]*' || echo "$fallback"
                else
                    echo "$fallback"
                fi
                ;;
        esac
    fi
}

# Improved test status detection
# Determines test status by checking multiple indicators:
# 1. Explicit PASS/FAIL markers in result files
# 2. Error test files (.err) - tests that should fail compilation
# 3. Expected output files (.output) - compare run-output with expected
# 4. Compilation result files
# 5. Run output files
# 6. Error patterns in output
determine_test_status() {
    local result_file="$1"
    local test_status="unknown"
    local test_error=""
    
    # Get base filename without extension
    local base_file="${result_file%-compile_result.txt}"
    base_file="${base_file%-run-output.txt}"
    local compile_result="${base_file}-compile_result.txt"
    local run_output="${base_file}-run-output.txt"
    
    # Extract category and test name from result file path
    # Result file path format: tests/results/Compilation/$category/$category-$testName-*.txt
    # Or: tests/results/Compilation/$category/$subcategory/$category-$testName-*.txt
    local result_dir=$(dirname "$result_file")
    local filename=$(basename "$result_file")
    
    # Try to extract category and test name from filename
    # Format: $category-$testName-compile_result.txt or $category-$testName-run-output.txt
    local category=""
    local test_name=""
    
    # Extract from filename pattern: category-testname-suffix.txt
    if [[ "$filename" =~ ^([^-]+)-(.+)-(compile_result|run-output)\.txt$ ]]; then
        category="${BASH_REMATCH[1]}"
        test_name="${BASH_REMATCH[2]}"
    fi
    
    # If extraction failed, try to get category from directory structure
    if [ -z "$category" ] || [ -z "$test_name" ]; then
        # Result files are in: tests/results/Compilation/$category/...
        # Try to extract category from path
        if [[ "$result_dir" =~ /Compilation/([^/]+) ]]; then
            category="${BASH_REMATCH[1]}"
        fi
        # Try to extract test name from base_file
        if [ -n "$base_file" ]; then
            local base_filename=$(basename "$base_file")
            if [[ "$base_filename" =~ ^[^-]+-(.+)$ ]]; then
                test_name="${BASH_REMATCH[1]}"
            fi
        fi
    fi
    
    # Construct paths to expected test files
    local test_dir=""
    local expected_output=""
    local expected_error=""
    
    if [ -n "$category" ] && [ -n "$test_name" ]; then
        # Try to find test directory (could be in subdirectory)
        # Extract relative path from Compilation directory to find nested structure
        # Result files are in: tests/results/Compilation/$category[/$subcategory/...]
        # Test files are in: tests/compile_tests/$category[/$subcategory/...]
        if [[ "$result_dir" =~ /Compilation/(.+)$ ]]; then
            local relative_path="${BASH_REMATCH[1]}"
            # Remove any trailing filename components to get directory path
            # If result_dir ends with a category name, use it directly
            test_dir="./tests/compile_tests/$relative_path"
            
            # If that doesn't exist, try just the category (for flat structure)
            if [ ! -d "$test_dir" ]; then
                test_dir="./tests/compile_tests/$category"
            fi
        else
            # Fallback: try direct category path
            test_dir="./tests/compile_tests/$category"
        fi
        
        if [ -d "$test_dir" ]; then
            expected_output="$test_dir/$test_name.output"
            expected_error="$test_dir/$test_name.err"
        fi
    fi
    
    # Check for explicit PASS marker (highest priority)
    # Match lines that are exactly "PASS" or start with "PASSED"
    if grep -qE "^PASS$|^PASSED" "$result_file" 2>/dev/null; then
        test_status="passed"
    # Check for explicit FAIL marker
    # Match lines that are exactly "FAIL" or start with "FAILED"
    elif grep -qE "^FAIL$|^FAILED" "$result_file" 2>/dev/null; then
        test_status="failed"
        test_error=$(head -20 "$result_file" | head -c 500)
    # Check if this is an error test (has .err file)
    elif [ -n "$expected_error" ] && [ -f "$expected_error" ]; then
        # This is an error test - should fail compilation
        if [ -f "$compile_result" ]; then
            # Check if compilation actually failed (error test should fail)
            # Error tests should have compilation errors in compile_result
            if grep -qiE "error|failed|failure" "$compile_result" 2>/dev/null; then
                # Check if the error matches expected error (optional, but good to verify)
                # For now, if compilation failed, consider it passed (error test worked)
                test_status="passed"
            else
                # Compilation succeeded but should have failed - test failed
                test_status="failed"
                test_error="Expected compilation error but compilation succeeded"
            fi
        else
            # No compile result - can't determine status
            test_status="unknown"
        fi
    # Check if there's an expected output file (success test with output comparison)
    elif [ -n "$expected_output" ] && [ -f "$expected_output" ]; then
        # This test has expected output - compare run-output with expected
        if [ -f "$run_output" ]; then
            # Normalize newlines for comparison (matches compile_tests.sh approach)
            # Read files and normalize carriage returns, then compare
            local expected_content=$(cat "$expected_output")
            local actual_content=$(cat "$run_output")
            
            # Normalize: remove trailing carriage returns (matches compile_tests.sh line 361-362)
            expected_content=$(echo -n "$expected_content" | sed 's/\r$//')
            actual_content=$(echo -n "$actual_content" | sed 's/\r$//')
            
            # Compare (matches compile_tests.sh line 364)
            if [ "$actual_content" == "$expected_content" ]; then
                test_status="passed"
            else
                test_status="failed"
                # Truncate error message to avoid overly long output
                local expected_preview=$(printf '%s' "$expected_content" | head -c 200)
                local actual_preview=$(printf '%s' "$actual_content" | head -c 200)
                test_error="Output mismatch. Expected: ${expected_preview}... Got: ${actual_preview}..."
            fi
        elif [ -f "$compile_result" ]; then
            # Has expected output but no run output - check compilation
            if grep -qiE "error|failed|failure|cannot|unable|missing" "$compile_result" 2>/dev/null; then
                test_status="failed"
                test_error=$(head -20 "$compile_result" | head -c 500)
            else
                # Compilation succeeded but no run output - might be execution issue
                test_status="failed"
                test_error="Compilation succeeded but test did not produce output"
            fi
        else
            # No compile result or run output
            test_status="unknown"
        fi
    # Check compilation result if it exists (for compilation-only tests)
    elif [ -f "$compile_result" ]; then
        # Check for compilation errors
        if grep -qiE "error|failed|failure|cannot|unable|missing" "$compile_result" 2>/dev/null; then
            test_status="failed"
            test_error=$(head -20 "$compile_result" | head -c 500)
        # Check if compilation succeeded and there's a run output
        elif [ -f "$run_output" ]; then
            # Check run output for errors
            if grep -qiE "error|failed|failure|exception|abort|segmentation" "$run_output" 2>/dev/null; then
                test_status="failed"
                test_error=$(head -20 "$run_output" | head -c 500)
            else
                # No expected output file, but run output exists and has no errors
                # This might be a test without expected output - assume passed
                test_status="passed"
            fi
        else
            # Compilation succeeded but no run output - assume passed (compilation test)
            test_status="passed"
        fi
    # Check if there's a run output file
    elif [ -f "$run_output" ]; then
        # Check run output for errors
        if grep -qiE "error|failed|failure|exception|abort|segmentation" "$run_output" 2>/dev/null; then
            test_status="failed"
            test_error=$(head -20 "$run_output" | head -c 500)
        else
            # No expected output to compare, but run output exists and has no errors
            test_status="passed"
        fi
    # Check current file for error patterns
    elif grep -qiE "error|failed|failure|cannot|unable|missing" "$result_file" 2>/dev/null; then
        test_status="failed"
        test_error=$(head -20 "$result_file" | head -c 500)
    # Default to skipped if no indicators found
    else
        test_status="skipped"
    fi
    
    # Return status and error (pipe-separated for parsing)
    # Ensure test_error is not null/empty for proper parsing
    if [ -z "$test_error" ]; then
        test_error=""
    fi
    echo "$test_status|$test_error"
}

# Collect test results from all test categories
collect_test_results() {
    local results_dir=$1
    
    # Validate input
    if [ ! -d "$results_dir" ]; then
        echo "Error: Results directory does not exist: $results_dir" >&2
        return 1
    fi
    
    local test_results_file="$results_dir/test_results.json"
    local summaries_file="$results_dir/category_summaries.sh"
    
    # Initialize JSON structure with error checking
    if ! echo "{" > "$test_results_file" 2>/dev/null; then
        echo "Error: Cannot write to test results file: $test_results_file" >&2
        return 1
    fi
    echo "  \"timestamp\": \"$(date -u +"%Y-%m-%dT%H:%M:%SZ")\"," >> "$test_results_file"
    echo "  \"categories\": {" >> "$test_results_file"
    
    # Initialize category summaries file
    echo "# Category summaries stored during collection" > "$summaries_file"
    echo "declare -A category_summaries" >> "$summaries_file"
    
    local first_category=1
    local total_tests=0
    local total_passed=0
    local total_failed=0
    local total_skipped=0
    
    # Process each test category directory (handles nested subdirectories for better organization)
    for category_dir in "$results_dir"/*/; do
        if [ ! -d "$category_dir" ]; then
            continue
        fi
        
        local category=$(basename "$category_dir")
        local category_tests=0
        local category_passed=0
        local category_failed=0
        local category_skipped=0
        
        if [ "$first_category" -eq 0 ]; then
            echo "," >> "$test_results_file"
        fi
        first_category=0
        
        echo "    \"$category\": {" >> "$test_results_file"
        echo "      \"tests\": [" >> "$test_results_file"
        
        local first_test=1
        # Find all test result files (recursively search subdirectories for nested organization)
        # This handles cases like Compilation/audio/, Compilation/graphics/, etc.
        while IFS= read -r result_file; do
            if [ ! -f "$result_file" ]; then
                continue
            fi
            
            local test_name=$(basename "$result_file" | sed 's/-compile_result\.txt$//; s/-run-output\.txt$//')
            
            # Use improved test status detection
            local status_result
            status_result=$(determine_test_status "$result_file")
            local test_status=$(echo "$status_result" | cut -d'|' -f1)
            local test_error=$(echo "$status_result" | cut -d'|' -f2-)
            
            category_tests=$((category_tests + 1))
            case "$test_status" in
                passed) category_passed=$((category_passed + 1)) ;;
                failed) category_failed=$((category_failed + 1)) ;;
                skipped) category_skipped=$((category_skipped + 1)) ;;
            esac
            
            if [ "$first_test" -eq 0 ]; then
                echo "," >> "$test_results_file"
            fi
            first_test=0
            
            # Extract subcategory from path if nested (e.g., Compilation/audio/test -> audio/test)
            local relative_path="${result_file#$category_dir}"
            local subcategory=""
            local dir_part=$(dirname "$relative_path")
            if [ "$dir_part" != "." ]; then
                subcategory="$dir_part/"
            fi
            
            # Escape test name and subcategory for JSON (handles special characters)
            local full_name="${subcategory}${test_name}"
            local escaped_name
            escaped_name=$(escape_json_string "$full_name")
            # escape_json_string returns a quoted string, use it directly as JSON value
            
            echo "        {" >> "$test_results_file"
            echo "          \"name\": $escaped_name," >> "$test_results_file"
            echo "          \"status\": \"$test_status\"," >> "$test_results_file"
            if [ -n "$test_error" ]; then
                # Escape JSON special characters using helper function
                # Function returns a properly quoted and escaped JSON string
                local escaped_error
                escaped_error=$(escape_json_string "$test_error")
                echo "          \"error\": $escaped_error," >> "$test_results_file"
            fi
            # Escape file name for JSON (handles special characters in filenames)
            local file_name=$(basename "$result_file")
            local escaped_file
            escaped_file=$(escape_json_string "$file_name")
            echo "          \"file\": $escaped_file" >> "$test_results_file"
            echo -n "        }" >> "$test_results_file"
        done < <(find "$category_dir" -type f \( -name "*-compile_result.txt" -o -name "*-run-output.txt" \) 2>/dev/null | sort)
        
        echo "" >> "$test_results_file"
        echo "      ]," >> "$test_results_file"
        echo "      \"summary\": {" >> "$test_results_file"
        echo "        \"total\": $category_tests," >> "$test_results_file"
        echo "        \"passed\": $category_passed," >> "$test_results_file"
        echo "        \"failed\": $category_failed," >> "$test_results_file"
        echo "        \"skipped\": $category_skipped" >> "$test_results_file"
        echo "      }" >> "$test_results_file"
        echo -n "    }" >> "$test_results_file"
        
        # Store category summary in associative array format for later use
        # Escape category name to handle special characters safely
        local escaped_category=$(printf '%q' "$category")
        echo "category_summaries[$escaped_category]=\"$category_tests|$category_passed|$category_failed|$category_skipped\"" >> "$summaries_file"
        
        total_tests=$((total_tests + category_tests))
        total_passed=$((total_passed + category_passed))
        total_failed=$((total_failed + category_failed))
        total_skipped=$((total_skipped + category_skipped))
    done
    
    echo "" >> "$test_results_file"
    echo "  }," >> "$test_results_file"
    echo "  \"summary\": {" >> "$test_results_file"
    echo "    \"total\": $total_tests," >> "$test_results_file"
    echo "    \"passed\": $total_passed," >> "$test_results_file"
    echo "    \"failed\": $total_failed," >> "$test_results_file"
    echo "    \"skipped\": $total_skipped" >> "$test_results_file"
    echo "  }" >> "$test_results_file"
    echo "}" >> "$test_results_file"
    
    echo "$test_results_file"
}

generate_html_report() {
    local results_dir=$1
    local output_file=$2
    
    # Validate inputs
    if [ ! -d "$results_dir" ]; then
        echo "Error: Results directory does not exist: $results_dir" >&2
        return 1
    fi
    
    if [ -z "$output_file" ]; then
        echo "Error: Output file not specified" >&2
        return 1
    fi
    
    # Collect test results
    local test_results_file
    test_results_file=$(collect_test_results "$results_dir")
    
    if [ $? -ne 0 ] || [ ! -f "$test_results_file" ]; then
        echo "Error: Failed to collect test results" >&2
        return 1
    fi
    
    # Load category summaries from the file created during collection
    local summaries_file="$results_dir/category_summaries.sh"
    if [ -f "$summaries_file" ]; then
        # Source the summaries file to populate the associative array
        source "$summaries_file"
    fi
    
    # Generate HTML report
    {
        cat << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>QB64-PE Test Report</title>
    <meta charset="UTF-8">
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif; 
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1200px;
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
        .summary-card.passed { background-color: #e8f5e9; }
        .summary-card.failed { background-color: #ffebee; }
        .summary-card.skipped { background-color: #fff3e0; }
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
        .test-section { 
            margin: 30px 0; 
            border: 1px solid #ddd;
            border-radius: 6px;
            overflow: hidden;
        }
        .test-section-header {
            background-color: #f8f9fa;
            padding: 15px 20px;
            cursor: pointer;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }
        .test-section-header:hover {
            background-color: #e9ecef;
        }
        .test-section-header h3 {
            margin: 0;
            color: #333;
        }
        .test-section-content {
            display: none;
            padding: 20px;
        }
        .test-section-content.expanded {
            display: block;
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
        .test-pass { color: #4CAF50; font-weight: bold; }
        .test-fail { color: #f44336; font-weight: bold; }
        .test-skip { color: #ff9800; font-weight: bold; }
        .test-error {
            background-color: #fff5f5;
            padding: 10px;
            border-left: 4px solid #f44336;
            margin-top: 5px;
            font-family: monospace;
            font-size: 12px;
            white-space: pre-wrap;
            word-break: break-all;
        }
        .coverage-bar {
            height: 20px;
            background-color: #e0e0e0;
            border-radius: 10px;
            overflow: hidden;
            margin: 10px 0;
        }
        .coverage-fill {
            height: 100%;
            background-color: #4CAF50;
            transition: width 0.3s;
        }
        .timestamp {
            color: #666;
            font-size: 14px;
            margin-bottom: 20px;
        }
    </style>
    <script>
        function toggleSection(element) {
            const content = element.nextElementSibling;
            content.classList.toggle('expanded');
        }
    </script>
</head>
<body>
    <div class="container">
        <h1>QB64-PE Test Report</h1>
        <div class="timestamp">Generated: 
EOF
        date
        cat << 'EOF'
        </div>
        
        <h2>Summary</h2>
        <div class="summary">
            <div class="summary-card total">
                <h3>Total Tests</h3>
                <div class="value" id="total-tests">0</div>
            </div>
            <div class="summary-card passed">
                <h3>Passed</h3>
                <div class="value" id="total-passed">0</div>
            </div>
            <div class="summary-card failed">
                <h3>Failed</h3>
                <div class="value" id="total-failed">0</div>
            </div>
            <div class="summary-card skipped">
                <h3>Skipped</h3>
                <div class="value" id="total-skipped">0</div>
            </div>
        </div>
        
        <div class="coverage-bar">
            <div class="coverage-fill" id="coverage-fill" style="width: 0%"></div>
        </div>
        
        <h2>Test Results by Category</h2>
EOF
        
        # Parse JSON and generate HTML
        if [ -f "$test_results_file" ]; then
            # Extract summary using jq with fallback
            local total
            total=$(extract_json_value "$test_results_file" ".summary.total" "0")
            local passed
            passed=$(extract_json_value "$test_results_file" ".summary.passed" "0")
            local failed
            failed=$(extract_json_value "$test_results_file" ".summary.failed" "0")
            local skipped
            skipped=$(extract_json_value "$test_results_file" ".summary.skipped" "0")
            
            local coverage=0
            if [ "$total" -gt 0 ] && [ -n "$total" ] && [ "$total" != "null" ]; then
                coverage=$((passed * 100 / total))
            fi
            
            cat << EOF
        <script>
            document.getElementById('total-tests').textContent = '$total';
            document.getElementById('total-passed').textContent = '$passed';
            document.getElementById('total-failed').textContent = '$failed';
            document.getElementById('total-skipped').textContent = '$skipped';
            document.getElementById('coverage-fill').style.width = '$coverage%';
        </script>
EOF
            
            # Generate category sections using jq for robust JSON parsing
            if [ "$JQ_AVAILABLE" -eq 1 ]; then
                # Use jq to iterate through categories and tests
                local categories
                categories=$(jq -r '.categories | keys[]' "$test_results_file" 2>/dev/null || echo "")
                
                while IFS= read -r current_category; do
                    [ -z "$current_category" ] && continue
                    
                    echo "        <div class=\"test-section\">"
                    echo "            <div class=\"test-section-header\" onclick=\"toggleSection(this)\">"
                    echo "                <h3>$current_category</h3>"
                    
                    # Extract category summary using jq
                    local cat_total cat_passed cat_failed cat_skipped
                    cat_total=$(jq -r ".categories.\"$current_category\".summary.total // 0" "$test_results_file" 2>/dev/null || echo "0")
                    cat_passed=$(jq -r ".categories.\"$current_category\".summary.passed // 0" "$test_results_file" 2>/dev/null || echo "0")
                    cat_failed=$(jq -r ".categories.\"$current_category\".summary.failed // 0" "$test_results_file" 2>/dev/null || echo "0")
                    cat_skipped=$(jq -r ".categories.\"$current_category\".summary.skipped // 0" "$test_results_file" 2>/dev/null || echo "0")
                    
                    echo "                <span>Total: $cat_total, Passed: $cat_passed, Failed: $cat_failed, Skipped: $cat_skipped</span>"
                    echo "            </div>"
                    echo "            <div class=\"test-section-content\">"
                    echo "                <table>"
                    echo "                    <tr><th>Test Name</th><th>Status</th><th>Details</th></tr>"
                    
                    # Extract tests for this category
                    jq -r ".categories.\"$current_category\".tests[] | \"\(.name)|\(.status)|\(.error // \"\")\"" "$test_results_file" 2>/dev/null | while IFS='|' read -r test_name test_status test_error; do
                        local status_class="test-$test_status"
                        local status_display=$(echo "$test_status" | sed 's/^./\U&/')
                        echo "                    <tr>"
                        echo "                        <td>$test_name</td>"
                        echo "                        <td class=\"$status_class\">$status_display</td>"
                        echo "                        <td>"
                        if [ -n "$test_error" ] && [ "$test_error" != "null" ] && [ "$test_error" != "" ]; then
                            # Unescape JSON error message
                            local error_display
                            error_display=$(echo "$test_error" | sed 's/\\n/\n/g; s/\\"/"/g; s/\\\\/\\/g')
                            echo "                            <div class=\"test-error\">$error_display</div>"
                        fi
                        echo "                        </td>"
                        echo "                    </tr>"
                    done
                    
                    echo "                </table>"
                    echo "                <p><strong>Category Summary:</strong> Total: $cat_total, Passed: $cat_passed, Failed: $cat_failed, Skipped: $cat_skipped</p>"
                    echo "            </div>"
                    echo "        </div>"
                done <<< "$categories"
            else
                # Fallback: line-by-line parsing (less robust but works without jq)
                local in_category=0
                local current_category=""
                
                while IFS= read -r line; do
                    if echo "$line" | grep -q '"categories": {'; then
                        continue
                    elif echo "$line" | grep -q '"[^"]*": {'; then
                        if [ "$in_category" -eq 1 ] && [ -n "$current_category" ]; then
                            echo "        </table>"
                            echo "        </div>"
                            echo "    </div>"
                        fi
                        current_category=$(echo "$line" | sed 's/.*"\([^"]*\)": {/\1/')
                        in_category=1
                        echo "        <div class=\"test-section\">"
                        echo "            <div class=\"test-section-header\" onclick=\"toggleSection(this)\">"
                        echo "                <h3>$current_category</h3>"
                        echo "                <span id=\"${current_category}-summary\"></span>"
                        echo "            </div>"
                        echo "            <div class=\"test-section-content\">"
                        echo "                <table>"
                        echo "                    <tr><th>Test Name</th><th>Status</th><th>Details</th></tr>"
                    elif echo "$line" | grep -q '"name":'; then
                        local test_name=$(echo "$line" | sed 's/.*"name": "\([^"]*\)".*/\1/')
                    elif echo "$line" | grep -q '"status":'; then
                        local status=$(echo "$line" | sed 's/.*"status": "\([^"]*\)".*/\1/')
                        local status_class="test-$status"
                        local status_display=$(echo "$status" | sed 's/^./\U&/')
                        echo "                    <tr>"
                        echo "                        <td>$test_name</td>"
                        echo "                        <td class=\"$status_class\">$status_display</td>"
                        echo "                        <td>"
                        if echo "$line" | grep -q '"error":'; then
                            local next_line
                            read -r next_line
                            local error=$(echo "$next_line" | sed 's/.*"error": "\([^"]*\)".*/\1/' | sed 's/\\n/\n/g; s/\\"/"/g; s/\\\\/\\/g')
                            echo "                            <div class=\"test-error\">$error</div>"
                        fi
                        echo "                        </td>"
                        echo "                    </tr>"
                    elif echo "$line" | grep -q '"summary": {'; then
                        # Use stored category summary from associative array
                        local cat_total=""
                        local cat_passed=""
                        local cat_failed=""
                        local cat_skipped=""
                        
                        if [ -n "$current_category" ] && [ -n "${category_summaries[$current_category]}" ]; then
                            IFS='|' read -r cat_total cat_passed cat_failed cat_skipped <<< "${category_summaries[$current_category]}"
                        fi
                        
                        echo "                </table>"
                        if [ -n "$cat_total" ]; then
                            echo "                <p><strong>Category Summary:</strong> Total: $cat_total, Passed: $cat_passed, Failed: $cat_failed, Skipped: $cat_skipped</p>"
                        fi
                    fi
                done < "$test_results_file"
                
                if [ "$in_category" -eq 1 ]; then
                    echo "            </div>"
                    echo "        </div>"
                fi
            fi
        fi
        
        cat << 'EOF'
    </div>
</body>
</html>
EOF
    } > "$output_file"
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to write HTML report to $output_file" >&2
        return 1
    fi
    
    echo "HTML report generated: $output_file"
}

generate_text_report() {
    local results_dir=$1
    local output_file=$2
    
    # Validate inputs
    if [ ! -d "$results_dir" ]; then
        echo "Error: Results directory does not exist: $results_dir" >&2
        return 1
    fi
    
    if [ -z "$output_file" ]; then
        echo "Error: Output file not specified" >&2
        return 1
    fi
    
    local test_results_file
    test_results_file=$(collect_test_results "$results_dir")
    
    if [ $? -ne 0 ] || [ ! -f "$test_results_file" ]; then
        echo "Error: Failed to collect test results" >&2
        return 1
    fi
    
    {
        echo "QB64-PE Test Report"
        echo "=================="
        echo "Generated: $(date)"
        echo ""
        echo "Summary:"
        echo "--------"
        
        if [ -f "$test_results_file" ]; then
            # Extract summary using jq with fallback
            local total
            total=$(extract_json_value "$test_results_file" ".summary.total" "0")
            local passed
            passed=$(extract_json_value "$test_results_file" ".summary.passed" "0")
            local failed
            failed=$(extract_json_value "$test_results_file" ".summary.failed" "0")
            local skipped
            skipped=$(extract_json_value "$test_results_file" ".summary.skipped" "0")
            
            echo "Total Tests:  $total"
            echo "Passed:      $passed"
            echo "Failed:      $failed"
            echo "Skipped:     $skipped"
            
            if [ "$total" -gt 0 ] && [ -n "$total" ] && [ "$total" != "null" ]; then
                local coverage=$((passed * 100 / total))
                echo "Coverage:    ${coverage}%"
            fi
        fi
        
        echo ""
        echo "Detailed Results:"
        echo "----------------"
        
        # Process each category using JSON data if available, otherwise fallback to file system
        if [ -f "$test_results_file" ] && [ "$JQ_AVAILABLE" -eq 1 ]; then
            # Use jq to extract category and test information from JSON
            local categories
            categories=$(jq -r '.categories | keys[]' "$test_results_file" 2>/dev/null || echo "")
            
            while IFS= read -r category; do
                [ -z "$category" ] && continue
                
                echo ""
                echo "Category: $category"
                echo "$(printf '=%.0s' {1..50})"
                
                # Extract tests for this category using jq
                jq -r ".categories.\"$category\".tests[] | \"\(.name)|\(.status)\"" "$test_results_file" 2>/dev/null | while IFS='|' read -r test_name test_status; do
                    local status_display=$(echo "$test_status" | tr '[:lower:]' '[:upper:]')
                    printf "  %-40s %s\n" "$test_name" "$status_display"
                done
            done <<< "$categories"
        else
            # Fallback: process from file system (handles nested subdirectories)
            for category_dir in "$results_dir"/*/; do
                if [ ! -d "$category_dir" ]; then
                    continue
                fi
                
                local category=$(basename "$category_dir")
                echo ""
                echo "Category: $category"
                echo "$(printf '=%.0s' {1..50})"
                
                # Find all test result files recursively
                while IFS= read -r result_file; do
                    if [ ! -f "$result_file" ]; then
                        continue
                    fi
                    
                    local test_name=$(basename "$result_file" | sed 's/-compile_result\.txt$//; s/-run-output\.txt$//')
                    
                    # Extract subcategory from path if nested
                    local relative_path="${result_file#$category_dir}"
                    local dir_part=$(dirname "$relative_path")
                    if [ "$dir_part" != "." ]; then
                        test_name="${dir_part}/${test_name}"
                    fi
                    
                    # Use improved test status detection
                    local status_result
                    status_result=$(determine_test_status "$result_file")
                    local test_status=$(echo "$status_result" | cut -d'|' -f1)
                    local status_display=$(echo "$test_status" | tr '[:lower:]' '[:upper:]')
                    
                    printf "  %-40s %s\n" "$test_name" "$status_display"
                done < <(find "$category_dir" -type f \( -name "*-compile_result.txt" -o -name "*-run-output.txt" \) 2>/dev/null | sort)
            done
        fi
    } > "$output_file"
    
    if [ $? -ne 0 ]; then
        echo "Error: Failed to write text report to $output_file" >&2
        return 1
    fi
    
    echo "Text report generated: $output_file"
}

# Main
if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <results_dir> <output_file> [format]"
    echo "  format: html or text (default: text)"
    exit 1
fi

RESULTS_DIR=$1
OUTPUT_FILE=$2
FORMAT=${3:-text}

# Validate format
if [ "$FORMAT" != "html" ] && [ "$FORMAT" != "text" ]; then
    echo "Error: Invalid format '$FORMAT'. Must be 'html' or 'text'" >&2
    exit 1
fi

# Generate report
if [ "$FORMAT" = "html" ]; then
    if ! generate_html_report "$RESULTS_DIR" "$OUTPUT_FILE"; then
        exit 1
    fi
else
    if ! generate_text_report "$RESULTS_DIR" "$OUTPUT_FILE"; then
        exit 1
    fi
fi
