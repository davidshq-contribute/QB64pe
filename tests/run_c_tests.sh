#!/bin/bash

. ./tests/colors.sh

result=0

for test in buffer http gfs logging threading graphics audio
do
    test_exe="./tests/exes/cpp/${test}_test"
    test_exe_exe="./tests/exes/cpp/${test}_test.exe"
    
    if [ -f "$test_exe" ]; then
        "$test_exe" || result=1
    elif [ -f "$test_exe_exe" ]; then
        "$test_exe_exe" || result=1
    else
        echo "Skipping ${test}_test (executable not found - may not be built)"
    fi
done

if [ "$result" != "1" ]; then
    echo "====== FINAL RESULT: ${GREEN}PASS${RESET} ======"
else
    echo "====== FINAL RESULT: ${RED}FAIL${RESET} ======"
fi

exit $result
