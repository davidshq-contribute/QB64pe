'QB64 Unit Test Harness - Implementation
'Run unit tests and report results

SUB InitTestSuite (suiteName AS STRING)
    TestSuiteName = suiteName
    TestCount = 0
    PRINT "========================================"
    PRINT "Test Suite: "; suiteName
    PRINT "========================================"
    PRINT
END SUB

SUB AssertEquals (expected AS STRING, actual AS STRING, testName AS STRING)
    TestCount = TestCount + 1
    TestResults(TestCount).testName = testName
    IF expected = actual THEN
        TestResults(TestCount).passed = -1
        TestResults(TestCount).message = ""
    ELSE
        TestResults(TestCount).passed = 0
        TestResults(TestCount).message = "Expected: [" + expected + "] Got: [" + actual + "]"
    END IF
END SUB

SUB AssertEqualsLong (expected AS LONG, actual AS LONG, testName AS STRING)
    TestCount = TestCount + 1
    TestResults(TestCount).testName = testName
    IF expected = actual THEN
        TestResults(TestCount).passed = -1
        TestResults(TestCount).message = ""
    ELSE
        TestResults(TestCount).passed = 0
        TestResults(TestCount).message = "Expected: " + _TRIM$(STR$(expected)) + " Got: " + _TRIM$(STR$(actual))
    END IF
END SUB

SUB AssertTrue (condition AS _BYTE, testName AS STRING)
    TestCount = TestCount + 1
    TestResults(TestCount).testName = testName
    IF condition THEN
        TestResults(TestCount).passed = -1
        TestResults(TestCount).message = ""
    ELSE
        TestResults(TestCount).passed = 0
        TestResults(TestCount).message = "Expected TRUE but got FALSE"
    END IF
END SUB

SUB AssertFalse (condition AS _BYTE, testName AS STRING)
    TestCount = TestCount + 1
    TestResults(TestCount).testName = testName
    IF condition = 0 THEN
        TestResults(TestCount).passed = -1
        TestResults(TestCount).message = ""
    ELSE
        TestResults(TestCount).passed = 0
        TestResults(TestCount).message = "Expected FALSE but got TRUE"
    END IF
END SUB

SUB AssertNotEmpty (value AS STRING, testName AS STRING)
    TestCount = TestCount + 1
    TestResults(TestCount).testName = testName
    IF LEN(value) > 0 THEN
        TestResults(TestCount).passed = -1
        TestResults(TestCount).message = ""
    ELSE
        TestResults(TestCount).passed = 0
        TestResults(TestCount).message = "Expected non-empty string but got empty"
    END IF
END SUB

SUB RunTests
    DIM i AS LONG
    DIM passed AS LONG, failed AS LONG

    PRINT "Results:"
    PRINT "--------"

    FOR i = 1 TO TestCount
        IF TestResults(i).passed THEN
            passed = passed + 1
            PRINT "[PASS] "; TestResults(i).testName
        ELSE
            failed = failed + 1
            PRINT "[FAIL] "; TestResults(i).testName
            IF LEN(TestResults(i).message) > 0 THEN
                PRINT "       "; TestResults(i).message
            END IF
        END IF
    NEXT i

    PRINT
    PRINT "========================================"
    PRINT "Total: "; _TRIM$(STR$(TestCount)); " tests"
    PRINT "Passed: "; _TRIM$(STR$(passed))
    PRINT "Failed: "; _TRIM$(STR$(failed))
    PRINT "========================================"

    IF failed > 0 THEN
        SYSTEM 1
    ELSE
        SYSTEM 0
    END IF
END SUB

FUNCTION GetTestsPassed&
    DIM i AS LONG, count AS LONG
    FOR i = 1 TO TestCount
        IF TestResults(i).passed THEN count = count + 1
    NEXT i
    GetTestsPassed = count
END FUNCTION

FUNCTION GetTestsFailed&
    DIM i AS LONG, count AS LONG
    FOR i = 1 TO TestCount
        IF TestResults(i).passed = 0 THEN count = count + 1
    NEXT i
    GetTestsFailed = count
END FUNCTION
