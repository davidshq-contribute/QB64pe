'QB64 Unit Test Harness - Interface
'Provides assertion functions and test runner for unit testing QB64 modules

TYPE TestResultType
    testName AS STRING
    passed AS _BYTE
    message AS STRING
END TYPE

DIM SHARED TestResults(1 TO 1000) AS TestResultType
DIM SHARED TestCount AS LONG
DIM SHARED TestSuiteName AS STRING

DECLARE SUB InitTestSuite (suiteName AS STRING)
DECLARE SUB AssertEquals (expected AS STRING, actual AS STRING, testName AS STRING)
DECLARE SUB AssertEqualsLong (expected AS LONG, actual AS LONG, testName AS STRING)
DECLARE SUB AssertTrue (condition AS _BYTE, testName AS STRING)
DECLARE SUB AssertFalse (condition AS _BYTE, testName AS STRING)
DECLARE SUB AssertNotEmpty (value AS STRING, testName AS STRING)
DECLARE SUB RunTests ()
DECLARE FUNCTION GetTestsPassed& ()
DECLARE FUNCTION GetTestsFailed& ()
