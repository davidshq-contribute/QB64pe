#!/bin/bash
# Test Discovery System
# Automatically discovers, categorizes, and filters tests
#
# Usage:
#   source ./tests/test_discovery.sh
#   discover_tests [options]
#
# Options:
#   --category CAT     Filter by category (compile, unit, integration, runtime, format, qbasic)
#   --tag TAG          Filter by tag
#   --pattern PAT      Filter by filename pattern
#   --path PATH        Filter by path
#   --list             List all discovered tests
#   --json             Output in JSON format
#   --verbose          Verbose output

# Test categories
TEST_CATEGORY_COMPILE="compile"
TEST_CATEGORY_UNIT="unit"
TEST_CATEGORY_INTEGRATION="integration"
TEST_CATEGORY_RUNTIME="runtime"
TEST_CATEGORY_FORMAT="format"
TEST_CATEGORY_QBASIC="qbasic"

# Internal variables
_DISCOVERY_VERBOSE=0
_DISCOVERY_JSON=0
_DISCOVERY_LIST=0
_DISCOVERY_CATEGORY=""
_DISCOVERY_TAG=""
_DISCOVERY_PATTERN=""
_DISCOVERY_PATH=""

# Discover all tests and categorize them
# Output format: category|path|name|tags
discover_all_tests() {
    local tests=""
    
    # Discover compile tests
    if [ -d "./tests/compile_tests" ]; then
        while IFS= read -r test; do
            local category=$(basename "$(dirname "$test")")
            local name=$(basename "$test" .bas)
            local tags=$(get_test_tags "$test" "$TEST_CATEGORY_COMPILE")
            tests="${tests}${TEST_CATEGORY_COMPILE}|${test}|${name}|${tags}"$'\n'
        done < <(find "./tests/compile_tests" -name "*.bas" -type f 2>/dev/null | sort)
    fi
    
    # Discover unit tests
    if [ -d "./tests/unit" ]; then
        while IFS= read -r test; do
            # Skip framework files and test runner
            local basename=$(basename "$test")
            # Use case statement for pattern matching (not [[ == ]] which doesn't support globs)
            case "$basename" in
                test_runner.bas|test_framework.bi)
                    continue
                    ;;
                test_*.bi)
                    continue
                    ;;
            esac
            local category=$(basename "$(dirname "$test")")
            local name=$(basename "$test" .bas)
            local tags=$(get_test_tags "$test" "$TEST_CATEGORY_UNIT")
            tests="${tests}${TEST_CATEGORY_UNIT}|${test}|${name}|${tags}"$'\n'
        done < <(find "./tests/unit" -name "*.bas" -type f 2>/dev/null | sort)
    fi
    
    # Discover integration tests
    if [ -d "./tests/integration" ]; then
        while IFS= read -r test; do
            local category=$(basename "$(dirname "$test")")
            local name=$(basename "$test" .bas)
            local tags=$(get_test_tags "$test" "$TEST_CATEGORY_INTEGRATION")
            tests="${tests}${TEST_CATEGORY_INTEGRATION}|${test}|${name}|${tags}"$'\n'
        done < <(find "./tests/integration" -name "*.bas" -type f 2>/dev/null | sort)
    fi
    
    # Discover format tests
    if [ -d "./tests/format_tests" ]; then
        while IFS= read -r test; do
            local category=$(basename "$(dirname "$test")")
            local name=$(basename "$test" .bas)
            local tags=$(get_test_tags "$test" "$TEST_CATEGORY_FORMAT")
            tests="${tests}${TEST_CATEGORY_FORMAT}|${test}|${name}|${tags}"$'\n'
        done < <(find "./tests/format_tests" -name "*.bas" -type f 2>/dev/null | sort)
    fi
    
    # Discover QBasic tests
    if [ -d "./tests/qbasic_testcases" ]; then
        while IFS= read -r test; do
            local category=$(basename "$(dirname "$test")")
            local name=$(basename "$test" .bas)
            local tags=$(get_test_tags "$test" "$TEST_CATEGORY_QBASIC")
            tests="${tests}${TEST_CATEGORY_QBASIC}|${test}|${name}|${tags}"$'\n'
        done < <(find "./tests/qbasic_testcases" -name "*.bas" -type f 2>/dev/null | sort)
    fi
    
    # Discover C++ runtime tests
    if [ -d "./tests/c" ]; then
        while IFS= read -r test; do
            # Skip framework files
            local basename=$(basename "$test")
            if [[ "$basename" == "test.h" ]] || [[ "$basename" == "test.cpp" ]]; then
                continue
            fi
            local name=$(basename "$test" .cpp)
            local tags=$(get_test_tags "$test" "$TEST_CATEGORY_RUNTIME")
            tests="${tests}${TEST_CATEGORY_RUNTIME}|${test}|${name}|${tags}"$'\n'
        done < <(find "./tests/c" -name "*.cpp" -type f 2>/dev/null | sort)
    fi
    
    # Remove trailing newline
    echo -n "$tests" | sed '/^$/d'
}

# Get test tags from metadata file or infer from path/name
# Format: tag1,tag2,tag3
get_test_tags() {
    local test_path="$1"
    local category="$2"
    local tags=""
    
    # Check for .tags metadata file
    local tags_file="${test_path}.tags"
    if [ -f "$tags_file" ]; then
        tags=$(cat "$tags_file" 2>/dev/null | tr '\n' ',' | sed 's/,$//' | tr ' ' ',')
        [ -z "$tags" ] && tags=""  # Ensure empty if file is empty or read fails
    fi
    
    # Infer tags from directory structure
    local dir=$(dirname "$test_path")
    local basename=$(basename "$test_path")
    
    # Add category as a tag
    if [ -z "$tags" ]; then
        tags="$category"
    else
        tags="$tags,$category"
    fi
    
    # Infer tags from directory names
    case "$category" in
        "$TEST_CATEGORY_COMPILE")
            # Check for error tests
            if [[ "$basename" == *"error"* ]] || [ -f "${test_path%.bas}.err" ]; then
                tags="$tags,error"
            fi
            # Check for subdirectory tags
            if [[ "$dir" == *"graphics"* ]]; then
                tags="$tags,graphics"
            fi
            if [[ "$dir" == *"audio"* ]]; then
                tags="$tags,audio"
            fi
            ;;
        "$TEST_CATEGORY_INTEGRATION")
            # Infer from subdirectory
            if [[ "$dir" == *"error_handling"* ]]; then
                tags="$tags,error"
            fi
            if [[ "$dir" == *"performance"* ]]; then
                tags="$tags,performance"
            fi
            if [[ "$dir" == *"compiler_state"* ]]; then
                tags="$tags,compiler_state"
            fi
            ;;
        "$TEST_CATEGORY_UNIT")
            # Infer from subdirectory
            local subdir=$(basename "$dir")
            tags="$tags,$subdir"
            ;;
    esac
    
    # Normalize tags (remove duplicates, sort)
    echo "$tags" | tr ',' '\n' | sort -u | tr '\n' ',' | sed 's/,$//'
}

# Filter tests based on criteria
filter_tests() {
    local tests="$1"
    local filtered=""
    
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        
        # Filter by category
        if [ -n "$_DISCOVERY_CATEGORY" ] && [ "$category" != "$_DISCOVERY_CATEGORY" ]; then
            continue
        fi
        
        # Filter by tag (use comma-separated matching to avoid partial matches)
        # This prevents "error" from matching "errors", "error_handling", etc.
        if [ -n "$_DISCOVERY_TAG" ]; then
            if ! echo ",$tags," | grep -q ",$_DISCOVERY_TAG,"; then
                continue
            fi
        fi
        
        # Filter by pattern
        if [ -n "$_DISCOVERY_PATTERN" ]; then
            if ! echo "$name" | grep -q "$_DISCOVERY_PATTERN"; then
                continue
            fi
        fi
        
        # Filter by path
        if [ -n "$_DISCOVERY_PATH" ]; then
            if ! echo "$path" | grep -q "$_DISCOVERY_PATH"; then
                continue
            fi
        fi
        
        filtered="${filtered}${category}|${path}|${name}|${tags}"$'\n'
    done <<< "$tests"
    
    echo -n "$filtered" | sed '/^$/d'
}

# Format output
format_output() {
    local tests="$1"
    
    if [ "$_DISCOVERY_JSON" -eq 1 ]; then
        format_json "$tests"
    elif [ "$_DISCOVERY_LIST" -eq 1 ]; then
        format_list "$tests"
    else
        format_default "$tests"
    fi
}

# Escape JSON strings properly (handles quotes, backslashes, newlines, tabs, and carriage returns)
escape_json() {
    local str="$1"
    # Escape backslashes first (must be done before other escapes)
    str=$(echo "$str" | sed 's/\\/\\\\/g')
    # Escape quotes
    str=$(echo "$str" | sed 's/"/\\"/g')
    # Escape tabs
    str=$(echo "$str" | sed 's/\t/\\t/g')
    # Escape carriage returns
    str=$(echo "$str" | sed 's/\r/\\r/g')
    # Escape newlines (replace actual newlines with \n)
    # Use a placeholder approach since sed processes line-by-line
    str=$(printf '%s' "$str" | sed ':a;N;$!ba;s/\n/\\n/g' 2>/dev/null || printf '%s' "$str" | tr '\n' '\001' | sed 's/\001/\\n/g')
    echo "$str"
}

# Format as JSON
format_json() {
    local tests="$1"
    local count=0
    local json=""
    
    echo "{"
    echo "  \"tests\": ["
    
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        [ "$count" -gt 0 ] && echo ","
        
        # Escape JSON strings properly
        local escaped_category=$(escape_json "$category")
        local escaped_path=$(escape_json "$path")
        local escaped_name=$(escape_json "$name")
        
        # Format tags as JSON array
        local tags_array=""
        if [ -n "$tags" ]; then
            IFS=',' read -ra tag_array <<< "$tags"
            local first=1
            for tag in "${tag_array[@]}"; do
                [ "$first" -eq 0 ] && tags_array="${tags_array},"
                local escaped_tag=$(escape_json "$tag")
                tags_array="${tags_array}\"${escaped_tag}\""
                first=0
            done
        fi
        
        echo "    {"
        echo "      \"category\": \"$escaped_category\","
        echo "      \"path\": \"$escaped_path\","
        echo "      \"name\": \"$escaped_name\","
        echo "      \"tags\": [$tags_array]"
        echo -n "    }"
        
        count=$((count + 1))
    done <<< "$tests"
    
    echo ""
    echo "  ],"
    echo "  \"count\": $count"
    echo "}"
}

# Format as list
format_list() {
    local tests="$1"
    local count=0
    
    printf "%-15s %-50s %-30s %s\n" "CATEGORY" "PATH" "NAME" "TAGS"
    printf "%-15s %-50s %-30s %s\n" "---------------" "--------------------------------------------------" "------------------------------" "------------------------------"
    
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        printf "%-15s %-50s %-30s %s\n" "$category" "$path" "$name" "$tags"
        count=$((count + 1))
    done <<< "$tests"
    
    echo ""
    echo "Total: $count tests"
}

# Format as default (simple)
format_default() {
    local tests="$1"
    local count=0
    
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        echo "$category: $name ($path)"
        [ "$_DISCOVERY_VERBOSE" -eq 1 ] && echo "  Tags: $tags"
        count=$((count + 1))
    done <<< "$tests"
    
    [ "$_DISCOVERY_VERBOSE" -eq 1 ] && echo ""
    [ "$_DISCOVERY_VERBOSE" -eq 1 ] && echo "Total: $count tests"
}

# Main discovery function
discover_tests() {
    # Reset internal variables
    _DISCOVERY_VERBOSE=0
    _DISCOVERY_JSON=0
    _DISCOVERY_LIST=0
    _DISCOVERY_CATEGORY=""
    _DISCOVERY_TAG=""
    _DISCOVERY_PATTERN=""
    _DISCOVERY_PATH=""
    
    # Parse arguments
    while [[ $# -gt 0 ]]; do
        case $1 in
            --category)
                _DISCOVERY_CATEGORY="$2"
                shift 2
                ;;
            --tag)
                _DISCOVERY_TAG="$2"
                shift 2
                ;;
            --pattern)
                _DISCOVERY_PATTERN="$2"
                shift 2
                ;;
            --path)
                _DISCOVERY_PATH="$2"
                shift 2
                ;;
            --list)
                _DISCOVERY_LIST=1
                shift
                ;;
            --json)
                _DISCOVERY_JSON=1
                shift
                ;;
            --verbose)
                _DISCOVERY_VERBOSE=1
                shift
                ;;
            *)
                echo "Unknown option: $1" >&2
                return 1
                ;;
        esac
    done
    
    # Discover all tests
    local all_tests=$(discover_all_tests)
    
    # Filter tests
    local filtered_tests=$(filter_tests "$all_tests")
    
    # Format and output to stdout
    format_output "$filtered_tests"
    
    # Note: For programmatic use, use get_raw_test_list() instead of discover_tests()
    # This function outputs formatted results for human consumption
}

# Get raw test list (without formatting) - for programmatic use
get_raw_test_list() {
    local all_tests=$(discover_all_tests)
    local filtered_tests=$(filter_tests "$all_tests")
    echo "$filtered_tests"
}

# Get tests by category (helper function)
get_tests_by_category() {
    local category="$1"
    _DISCOVERY_CATEGORY="$category"
    get_raw_test_list
}

# Get tests by tag (helper function)
get_tests_by_tag() {
    local tag="$1"
    _DISCOVERY_TAG="$tag"
    get_raw_test_list
}

# Count tests
count_tests() {
    local tests="$1"
    local count=0
    while IFS='|' read -r category path name tags; do
        [ -z "$category" ] && continue
        count=$((count + 1))
    done <<< "$tests"
    echo "$count"
}

# Export functions for use in other scripts
export -f discover_tests
export -f get_tests_by_category
export -f get_tests_by_tag
export -f get_raw_test_list
export -f discover_all_tests
export -f filter_tests
export -f count_tests
