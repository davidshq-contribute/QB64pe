'
' Unit Tests for Statement Parsing
'
' Tests statement parsing functionality.
' Uses component test harness for isolated testing.
'

'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'../test_state_manager.bi'
'$INCLUDE:'../../source/global/constants.bas'
'$INCLUDE:'../../source/utilities/type.bi'
'$INCLUDE:'../../source/utilities/hash.bi'
'$INCLUDE:'../../source/utilities/elements.bas'

' Note: Statement parsing tests require significant compiler state.
' These tests focus on testable aspects of statement parsing.

SUB Test_StatementStructure
    Test_Start "Statement structure recognition"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test basic statement structure recognition
    ' Note: Full statement parsing requires extensive compiler state
    ' This test verifies that basic parsing infrastructure is available
    
    ' Test that elements can be extracted from statements
    ' (This tests the elements.bas utility which is used by parser)
    DIM testStatement$
    DIM elementCount AS LONG
    
    testStatement$ = "PRINT 1, 2, 3"
    elementCount = numelements(testStatement$)
    result = Test_Assert&(elementCount > 0, "Statement should have elements")
    
    ' Test that elements can be retrieved
    IF result THEN
        DIM firstElement$
        firstElement$ = getelement(testStatement$, 1)
        result = Test_Assert&(LEN(firstElement$) > 0, "Should be able to get first element")
    END IF
    
    ' Test multi-element statement
    IF result THEN
        testStatement$ = "LET x = 5"
        elementCount = numelements(testStatement$)
        result = Test_Assert&(elementCount >= 3, "LET statement should have multiple elements")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_StatementElementExtraction
    Test_Start "Statement element extraction"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test that getelement correctly extracts elements from statements
    DIM testStatement$
    DIM element$
    
    ' Test simple statement
    testStatement$ = "PRINT hello"
    element$ = getelement(testStatement$, 1)
    result = Test_AssertEqualString&("PRINT", element$, "First element should be PRINT")
    
    IF result THEN
        element$ = getelement(testStatement$, 2)
        result = Test_AssertEqualString&("hello", element$, "Second element should be hello")
    END IF
    
    ' Test statement with multiple elements
    IF result THEN
        testStatement$ = "IF x > 5 THEN PRINT yes"
        element$ = getelement(testStatement$, 1)
        result = Test_AssertEqualString&("IF", element$, "First element should be IF")
    END IF
    
    ' Test empty statement
    IF result THEN
        testStatement$ = ""
        element$ = getelement(testStatement$, 1)
        result = Test_Assert&(LEN(element$) = 0, "Empty statement should return empty element")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all statement parsing tests
SUB RunStatementParsingTests
    Test_StatementStructure
    Test_StatementElementExtraction
END SUB
