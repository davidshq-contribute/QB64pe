'
' Unit Tests for Parser Error Handling
'
' Tests error handling in parser functions.
' Uses component test harness for isolated testing.
'
' Note: test_framework_declarations.bi, test_framework_implementations.bas, test_state_manager_declarations.bi, and test_state_manager_implementations.bas are included by test_runner.bas
'$INCLUDE:'../../../source/global/constants.bas'
''$INCLUDE:'../../../source/utilities/type.bi'  ' Now included globally in test_runner.bas
' Note: hash.bi is already included by test_runner.bas
'$INCLUDE:'../../../source/utilities/elements.bas'
'$INCLUDE:'../../../source/utilities/parser_utils.bas'

SUB Test_InvalidOperatorHandling
    Test_Start "Invalid operator handling"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test that isoperator handles invalid input gracefully
    result = Test_Assert&(NOT isoperator("INVALID_OP"), "Invalid operator should not be recognized")
    IF result THEN result = Test_Assert&(NOT isoperator("123"), "Numeric string should not be operator")
    IF result THEN result = Test_Assert&(NOT isoperator("ABC"), "Alphabetic string should not be operator")
    
    ' Test edge cases
    IF result THEN result = Test_Assert&(NOT isoperator(""), "Empty string should not be operator")
    IF result THEN result = Test_Assert&(NOT isoperator(" "), "Space should not be operator")
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_ElementExtractionErrors
    Test_Start "Element extraction error handling"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test that getelement handles edge cases
    DIM element$
    
    ' Test out of bounds access
    element$ = getelement("PRINT hello", 100)
    result = Test_Assert&(LEN(element$) = 0, "Out of bounds element should return empty string")
    
    ' Test negative index
    IF result THEN
        element$ = getelement("PRINT hello", -1)
        result = Test_Assert&(LEN(element$) = 0, "Negative index should return empty string")
    END IF
    
    ' Test zero index
    IF result THEN
        element$ = getelement("PRINT hello", 0)
        result = Test_Assert&(LEN(element$) = 0, "Zero index should return empty string")
    END IF
    
    ' Test empty statement
    IF result THEN
        element$ = getelement("", 1)
        result = Test_Assert&(LEN(element$) = 0, "Empty statement should return empty element")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_ElementCountErrors
    Test_Start "Element count error handling"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test that numelements handles edge cases
    DIM count AS LONG
    
    ' Test empty statement
    count = numelements("")
    result = Test_AssertEqual&(0, count, "Empty statement should have 0 elements")
    
    ' Test statement with only spaces
    IF result THEN
        count = numelements("   ")
        result = Test_AssertEqual&(0, count, "Statement with only spaces should have 0 elements")
    END IF
    
    ' Test single element
    IF result THEN
        count = numelements("PRINT")
        result = Test_Assert&(count >= 1, "Single element statement should have at least 1 element")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all error handling tests
SUB RunParserErrorHandlingTests
    Test_InvalidOperatorHandling
    Test_ElementExtractionErrors
    Test_ElementCountErrors
END SUB
