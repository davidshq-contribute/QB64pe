#!/bin/bash
# Test Report Generator
# Generates HTML and text reports from test results

generate_html_report() {
    local results_dir=$1
    local output_file=$2
    
    cat > "$output_file" << 'EOF'
<!DOCTYPE html>
<html>
<head>
    <title>QB64-PE Test Report</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .test-section { margin: 20px 0; }
        .test-pass { color: green; }
        .test-fail { color: red; }
        .test-skip { color: orange; }
        table { border-collapse: collapse; width: 100%; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
    </style>
</head>
<body>
    <h1>QB64-PE Test Report</h1>
    <p>Generated: $(date)</p>
    <h2>Summary</h2>
    <table>
        <tr><th>Category</th><th>Total</th><th>Passed</th><th>Failed</th><th>Skipped</th></tr>
        <!-- Test results will be inserted here -->
    </table>
</body>
</html>
EOF
    echo "HTML report generated: $output_file"
}

generate_text_report() {
    local results_dir=$1
    local output_file=$2
    
    {
        echo "QB64-PE Test Report"
        echo "=================="
        echo "Generated: $(date)"
        echo ""
        echo "Summary:"
        echo "--------"
        # Add test statistics here
        echo ""
        echo "Detailed Results:"
        echo "----------------"
        # Add detailed test results here
    } > "$output_file"
    
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

if [ "$FORMAT" = "html" ]; then
    generate_html_report "$RESULTS_DIR" "$OUTPUT_FILE"
else
    generate_text_report "$RESULTS_DIR" "$OUTPUT_FILE"
fi
