'
' Unit Testing Framework Declarations
'
' Forward declarations for all test framework functions.
' This file contains only DECLARE statements and must be included before
' any SUB/FUNCTION definitions to allow forward references.
'

' Test framework initialization and configuration
DECLARE SUB TestFramework_Init
DECLARE SUB TestFramework_SetVerbose (verbose AS LONG)
DECLARE FUNCTION TestFramework_AllPassed& ()
DECLARE SUB TestFramework_PrintSummary
DECLARE FUNCTION TestFramework_GetStats$ (stats AS TestStats)
DECLARE SUB TestFramework_SkipIncludes (enable AS LONG)

' Test execution functions
DECLARE SUB Test_Start (testName$)
DECLARE SUB Test_End (result AS LONG)
DECLARE SUB Test_Skip (reason$)

' Assertion functions
DECLARE FUNCTION Test_Assert& (condition AS LONG, message$)
DECLARE FUNCTION Test_AssertEqual& (expected AS LONG, actual AS LONG, message$)
DECLARE FUNCTION Test_AssertEqualString& (expected$, actual$, message$)
DECLARE FUNCTION Test_AssertInRange& (value AS LONG, minVal AS LONG, maxVal AS LONG, message$)
DECLARE FUNCTION Test_AssertApproxEqual& (expected AS DOUBLE, actual AS DOUBLE, tolerance AS DOUBLE, message$)
DECLARE FUNCTION Test_AssertContains& (haystack$, needle$, message$)
DECLARE FUNCTION Test_AssertNotContains& (haystack$, needle$, message$)
DECLARE FUNCTION Test_AssertGreaterThan& (actual AS LONG, expected AS LONG, message$)
DECLARE FUNCTION Test_AssertLessThan& (actual AS LONG, expected AS LONG, message$)
DECLARE FUNCTION Test_AssertGreaterThanOrEqual& (actual AS LONG, expected AS LONG, message$)
DECLARE FUNCTION Test_AssertLessThanOrEqual& (actual AS LONG, expected AS LONG, message$)
DECLARE FUNCTION Test_AssertEmpty& (testStr$, message$)
DECLARE FUNCTION Test_AssertNotEmpty& (testStr$, message$)
DECLARE FUNCTION Test_AssertNull& (value AS LONG, message$)
DECLARE FUNCTION Test_AssertNotNull& (value AS LONG, message$)
DECLARE FUNCTION Test_AssertEqualStringIgnoreCase& (expected$, actual$, message$)
