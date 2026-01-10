'
' Unit Testing Framework for QB64 Compiler Components
'
' Provides test harness, assertion library, and test runner functionality
' for testing individual compiler components in isolation.
'

' Test result constants
CONST TEST_RESULT_PASS = 0
CONST TEST_RESULT_FAIL = 1
CONST TEST_RESULT_SKIP = 2

' Test statistics
TYPE TestStats
    totalTests AS LONG
    passedTests AS LONG
    failedTests AS LONG
    skippedTests AS LONG
    totalAssertions AS LONG
    passedAssertions AS LONG
    failedAssertions AS LONG
END TYPE

DIM SHARED testStats AS TestStats
DIM SHARED currentTestName$ AS STRING
DIM SHARED testOutput$ AS STRING
DIM SHARED testVerbose AS LONG

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
SUB Test_Start (testName$)
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
        message$ = "Expected """ + expected$ + """ but got """ + actual$ + """"
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
    PRINT ""
    PRINT "=== Test Summary ==="
    PRINT "Total tests: " + _TOSTR$(testStats.totalTests)
    PRINT "Passed: " + _TOSTR$(testStats.passedTests)
    PRINT "Failed: " + _TOSTR$(testStats.failedTests)
    PRINT "Skipped: " + _TOSTR$(testStats.skippedTests)
    PRINT ""
    PRINT "Total assertions: " + _TOSTR$(testStats.totalAssertions)
    PRINT "Passed: " + _TOSTR$(testStats.passedAssertions)
    PRINT "Failed: " + _TOSTR$(testStats.failedAssertions)
    PRINT ""
    
    IF testStats.failedTests = 0 AND testStats.failedAssertions = 0 THEN
        PRINT "ALL TESTS PASSED"
    ELSE
        PRINT "SOME TESTS FAILED"
    END IF
    PRINT ""
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