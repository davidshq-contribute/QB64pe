'
' Unit Testing Framework Implementations
'
' Implementation of all test framework SUB/FUNCTION definitions.
' This file contains only SUB/FUNCTION implementations and must be included
' after the main program code section.
'

' Initialize test framework
SUB TestFramework_Init
    testStats.totalTests = 0
    testStats.passedTests = 0
    testStats.failedTests = 0
    testStats.skippedTests = 0
    testStats.totalAssertions = 0
    testStats.passedAssertions = 0
    testStats.failedAssertions = 0
    testOutput$ = ""
    testVerbose = 0
END SUB

' Set verbose mode (1 = on, 0 = off)
SUB TestFramework_SetVerbose (verbose AS LONG)
    testVerbose = verbose
END SUB

' Start a test case
' Automatically resets global state to ensure test isolation
SUB Test_Start (testName$)
    ' Reset global state before each test to ensure isolation
    Test_ResetGlobalState
    
    currentTestName$ = testName$
    testStats.totalTests = testStats.totalTests + 1
    IF testVerbose THEN
        PRINT "Running test: " + testName$
    END IF
END SUB

' End a test case
SUB Test_End (result AS LONG)
    SELECT CASE result
        CASE TEST_RESULT_PASS
            testStats.passedTests = testStats.passedTests + 1
            IF testVerbose THEN
                PRINT "  PASS: " + currentTestName$
            END IF
        CASE TEST_RESULT_FAIL
            testStats.failedTests = testStats.failedTests + 1
            PRINT "  FAIL: " + currentTestName$
        CASE TEST_RESULT_SKIP
            testStats.skippedTests = testStats.skippedTests + 1
            IF testVerbose THEN
                PRINT "  SKIP: " + currentTestName$
            END IF
    END SELECT
    currentTestName$ = ""
END SUB

' Assert that a condition is true
' condition: Condition to check (non-zero = true, zero = false)
' message$: Optional message to display on failure
FUNCTION Test_Assert& (condition AS LONG, message$)
    testStats.totalAssertions = testStats.totalAssertions + 1
    
    IF condition THEN
        testStats.passedAssertions = testStats.passedAssertions + 1
        Test_Assert& = TEST_RESULT_PASS
    ELSE
        testStats.failedAssertions = testStats.failedAssertions + 1
        IF message$ <> "" THEN
            PRINT "    Assertion failed: " + message$
            testOutput$ = testOutput$ + "    Assertion failed: " + message$ + CHR$(10)
        ELSE
            PRINT "    Assertion failed"
            testOutput$ = testOutput$ + "    Assertion failed" + CHR$(10)
        END IF
        Test_Assert& = TEST_RESULT_FAIL
    END IF
END FUNCTION

' Assert that two values are equal
' expected: Expected value
' actual: Actual value
' message$: Optional message
FUNCTION Test_AssertEqual& (expected AS LONG, actual AS LONG, message$)
    DIM result AS LONG
    result = (expected = actual)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected " + _TOSTR$(expected) + " but got " + _TOSTR$(actual)
    END IF
    
    Test_AssertEqual& = Test_Assert&(result, message$)
END FUNCTION

' Assert that two strings are equal
' expected$: Expected string
' actual$: Actual string
' message$: Optional message
FUNCTION Test_AssertEqualString& (expected$, actual$, message$)
    DIM result AS LONG
    result = (expected$ = actual$)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected " + CHR$(34) + expected$ + CHR$(34) + " but got " + CHR$(34) + actual$ + CHR$(34)
    END IF
    
    Test_AssertEqualString& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is within a range
' value: Value to check
' minVal: Minimum value (inclusive)
' maxVal: Maximum value (inclusive)
' message$: Optional message
FUNCTION Test_AssertInRange& (value AS LONG, minVal AS LONG, maxVal AS LONG, message$)
    DIM result AS LONG
    result = (value >= minVal AND value <= maxVal)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Value " + _TOSTR$(value) + " is not in range [" + _TOSTR$(minVal) + ", " + _TOSTR$(maxVal) + "]"
    END IF
    
    Test_AssertInRange& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a floating point value is approximately equal (within tolerance)
' expected: Expected value
' actual: Actual value
' tolerance: Maximum allowed difference
' message$: Optional message
FUNCTION Test_AssertApproxEqual& (expected AS DOUBLE, actual AS DOUBLE, tolerance AS DOUBLE, message$)
    DIM diff AS DOUBLE
    diff = ABS(expected - actual)
    DIM result AS LONG
    result = (diff <= tolerance)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected approximately " + _TOSTR$(expected) + " but got " + _TOSTR$(actual) + " (difference: " + _TOSTR$(diff) + ")"
    END IF
    
    Test_AssertApproxEqual& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a string contains a substring
' haystack$: String to search in
' needle$: Substring to find
' message$: Optional message
FUNCTION Test_AssertContains& (haystack$, needle$, message$)
    DIM result AS LONG
    result = (INSTR(haystack$, needle$) > 0)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected string to contain " + CHR$(34) + needle$ + CHR$(34) + " but it doesn't"
    END IF
    
    Test_AssertContains& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a string does not contain a substring
' haystack$: String to search in
' needle$: Substring that should not be found
' message$: Optional message
FUNCTION Test_AssertNotContains& (haystack$, needle$, message$)
    DIM result AS LONG
    result = (INSTR(haystack$, needle$) = 0)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected string to not contain " + CHR$(34) + needle$ + CHR$(34) + " but it does"
    END IF
    
    Test_AssertNotContains& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is greater than another
' actual: Value to check
' expected: Value to compare against
' message$: Optional message
FUNCTION Test_AssertGreaterThan& (actual AS LONG, expected AS LONG, message$)
    DIM result AS LONG
    result = (actual > expected)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected " + _TOSTR$(actual) + " to be greater than " + _TOSTR$(expected)
    END IF
    
    Test_AssertGreaterThan& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is less than another
' actual: Value to check
' expected: Value to compare against
' message$: Optional message
FUNCTION Test_AssertLessThan& (actual AS LONG, expected AS LONG, message$)
    DIM result AS LONG
    result = (actual < expected)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected " + _TOSTR$(actual) + " to be less than " + _TOSTR$(expected)
    END IF
    
    Test_AssertLessThan& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is greater than or equal to another
' actual: Value to check
' expected: Value to compare against
' message$: Optional message
FUNCTION Test_AssertGreaterThanOrEqual& (actual AS LONG, expected AS LONG, message$)
    DIM result AS LONG
    result = (actual >= expected)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected " + _TOSTR$(actual) + " to be greater than or equal to " + _TOSTR$(expected)
    END IF
    
    Test_AssertGreaterThanOrEqual& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is less than or equal to another
' actual: Value to check
' expected: Value to compare against
' message$: Optional message
FUNCTION Test_AssertLessThanOrEqual& (actual AS LONG, expected AS LONG, message$)
    DIM result AS LONG
    result = (actual <= expected)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected " + _TOSTR$(actual) + " to be less than or equal to " + _TOSTR$(expected)
    END IF
    
    Test_AssertLessThanOrEqual& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a string is empty
' testStr$: String to check
' message$: Optional message
FUNCTION Test_AssertEmpty& (testStr$, message$)
    DIM result AS LONG
    result = (LEN(testStr$) = 0)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected empty string but got " + CHR$(34) + testStr$ + CHR$(34)
    END IF
    
    Test_AssertEmpty& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a string is not empty
' testStr$: String to check
' message$: Optional message
FUNCTION Test_AssertNotEmpty& (testStr$, message$)
    DIM result AS LONG
    result = (LEN(testStr$) > 0)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected non-empty string but got empty string"
    END IF
    
    Test_AssertNotEmpty& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is null/zero
' value: Value to check
' message$: Optional message
FUNCTION Test_AssertNull& (value AS LONG, message$)
    DIM result AS LONG
    result = (value = 0)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected null/zero but got " + _TOSTR$(value)
    END IF
    
    Test_AssertNull& = Test_Assert&(result, message$)
END FUNCTION

' Assert that a value is not null/non-zero
' value: Value to check
' message$: Optional message
FUNCTION Test_AssertNotNull& (value AS LONG, message$)
    DIM result AS LONG
    result = (value <> 0)
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected non-null/non-zero value but got zero"
    END IF
    
    Test_AssertNotNull& = Test_Assert&(result, message$)
END FUNCTION

' Assert that two strings are equal (case-insensitive)
' expected$: Expected string
' actual$: Actual string
' message$: Optional message
FUNCTION Test_AssertEqualStringIgnoreCase& (expected$, actual$, message$)
    DIM result AS LONG
    result = (UCASE$(expected$) = UCASE$(actual$))
    
    IF NOT result AND message$ = "" THEN
        message$ = "Expected (case-insensitive) " + CHR$(34) + expected$ + CHR$(34) + " but got " + CHR$(34) + actual$ + CHR$(34)
    END IF
    
    Test_AssertEqualStringIgnoreCase& = Test_Assert&(result, message$)
END FUNCTION

' Skip a test
SUB Test_Skip (reason$)
    IF reason$ <> "" THEN
        IF testVerbose THEN
            PRINT "  SKIP: " + currentTestName$ + " - " + reason$
        END IF
    END IF
    Test_End TEST_RESULT_SKIP
END SUB

' Print test summary
SUB TestFramework_PrintSummary
    DIM fh AS LONG
    fh = FREEFILE
    OPEN "test_results.txt" FOR OUTPUT AS #fh

    PRINT ""
    PRINT "=== Test Summary ==="
    PRINT #fh, "=== Test Summary ==="
    PRINT "Total tests: " + _TOSTR$(testStats.totalTests)
    PRINT #fh, "Total tests: " + _TOSTR$(testStats.totalTests)
    PRINT "Passed: " + _TOSTR$(testStats.passedTests)
    PRINT #fh, "Passed: " + _TOSTR$(testStats.passedTests)
    PRINT "Failed: " + _TOSTR$(testStats.failedTests)
    PRINT #fh, "Failed: " + _TOSTR$(testStats.failedTests)
    PRINT "Skipped: " + _TOSTR$(testStats.skippedTests)
    PRINT #fh, "Skipped: " + _TOSTR$(testStats.skippedTests)
    PRINT ""
    PRINT #fh, ""
    PRINT "Total assertions: " + _TOSTR$(testStats.totalAssertions)
    PRINT #fh, "Total assertions: " + _TOSTR$(testStats.totalAssertions)
    PRINT "Passed: " + _TOSTR$(testStats.passedAssertions)
    PRINT #fh, "Passed: " + _TOSTR$(testStats.passedAssertions)
    PRINT "Failed: " + _TOSTR$(testStats.failedAssertions)
    PRINT #fh, "Failed: " + _TOSTR$(testStats.failedAssertions)
    PRINT ""
    PRINT #fh, ""

    IF testStats.failedTests = 0 AND testStats.failedAssertions = 0 THEN
        PRINT "ALL TESTS PASSED"
        PRINT #fh, "ALL TESTS PASSED"
    ELSE
        PRINT "SOME TESTS FAILED"
        PRINT #fh, "SOME TESTS FAILED"
    END IF
    PRINT ""
    PRINT #fh, ""

    IF testOutput$ <> "" THEN
        PRINT #fh, "=== Detailed Output ==="
        PRINT #fh, testOutput$
    END IF

    CLOSE #fh
END SUB

' Get test statistics
FUNCTION TestFramework_GetStats$ (stats AS TestStats)
    stats = testStats
END FUNCTION

' Check if all tests passed
FUNCTION TestFramework_AllPassed&
    TestFramework_AllPassed& = (testStats.failedTests = 0 AND testStats.failedAssertions = 0)
END FUNCTION

' Enable skip includes mode for unit testing
' When enabled, $INCLUDE directives will be ignored, allowing you to test
' individual functions without processing all their dependencies
' enable: -1 to enable, 0 to disable
SUB TestFramework_SkipIncludes (enable AS LONG)
    IncludeProvider_SkipIncludes enable
END SUB
