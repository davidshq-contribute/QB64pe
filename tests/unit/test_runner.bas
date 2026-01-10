'
' Unit Test Runner
'
' Discovers and runs unit tests for QB64 compiler components.
'

'$INCLUDE:'test_framework.bi'

' Test discovery and execution
SUB RunAllTests
    TestFramework_Init
    TestFramework_SetVerbose 1
    
    PRINT "QB64 Compiler Unit Tests"
    PRINT "========================"
    PRINT ""
    
    ' Run tests for each component
    ' Note: Individual test files will be included and their test functions called
    
    TestFramework_PrintSummary
    
    IF NOT TestFramework_AllPassed& THEN
        SYSTEM 1 ' Exit with error code if tests failed
    END IF
END SUB

' Main entry point
TestFramework_Init
RunAllTests
