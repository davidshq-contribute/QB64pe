'
' Unit Test Runner
'
' Discovers and runs unit tests for QB64 compiler components.
'

'$INCLUDE:'test_framework.bi'
'$INCLUDE:'test_state_manager.bi'
'$INCLUDE:'test_output_verification.bi'
'$INCLUDE:'type_system/test_type_system.bas'
'$INCLUDE:'symbol_table/test_hash.bas'
'$INCLUDE:'const_eval/test_const_eval.bas'
'$INCLUDE:'parser/test_parser.bas'
'$INCLUDE:'code_generation/test_code_generation.bas'

' Test discovery and execution
SUB RunAllTests
    TestFramework_Init
    TestFramework_SetVerbose 1
    
    PRINT "QB64 Compiler Unit Tests"
    PRINT "========================"
    PRINT ""
    
    ' Run tests for each component
    PRINT "Running type system tests..."
    RunTypeSystemTests
    
    PRINT "Running symbol table tests..."
    RunSymbolTableTests
    
    PRINT "Running constant evaluation tests..."
    RunConstEvalTests
    
    PRINT "Running parser tests..."
    RunParserTests
    
    PRINT "Running code generation tests..."
    RunCodeGenerationTests
    
    PRINT ""
    TestFramework_PrintSummary
    
    IF NOT TestFramework_AllPassed& THEN
        SYSTEM 1 ' Exit with error code if tests failed
    END IF
END SUB

' Main entry point
TestFramework_Init
RunAllTests
