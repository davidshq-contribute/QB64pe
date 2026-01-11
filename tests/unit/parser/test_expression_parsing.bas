'
' Unit Tests for Expression Parsing
'
' Tests expression parsing functionality.
' Uses component test harness for isolated testing.
'
' Note: test_framework.bi and test_state_manager.bi are included by test_runner.bas
'$INCLUDE:'../../../source/global/constants.bas'
''$INCLUDE:'../../../source/utilities/type.bi'  ' Now included globally in test_runner.bas
' Note: hash.bi is already included by test_runner.bas
'$INCLUDE:'../../../source/utilities/elements.bas'
'$INCLUDE:'../../../source/utilities/parser_utils.bas'

' Note: These tests require significant compiler state initialization.
' For now, we test simpler parsing functions that can be tested in isolation.

SUB Test_IsOperator
    Test_Start "isoperator function"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test that isoperator correctly identifies operators
    ' Note: isoperator is defined in parser_utils.bas and requires minimal state
    ' We test basic operator recognition
    
    ' Test logical operators
    result = Test_Assert&(isoperator("AND"), "AND should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("OR"), "OR should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("XOR"), "XOR should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("NOT"), "NOT should be identified as operator")
    
    ' Test comparison operators
    IF result THEN result = Test_Assert&(isoperator("="), "= should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator(">"), "> should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("<"), "< should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("<>"), "<> should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("<="), "<= should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator(">="), ">= should be identified as operator")
    
    ' Test arithmetic operators
    IF result THEN result = Test_Assert&(isoperator("+"), "+ should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("-"), "- should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("*"), "* should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("/"), "/ should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("MOD"), "MOD should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("\"), "\ should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("^"), "^ should be identified as operator")
    
    ' Test QB64-specific operators
    IF result THEN result = Test_Assert&(isoperator("_ORELSE"), "_ORELSE should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("_ANDALSO"), "_ANDALSO should be identified as operator")
    
    ' Test that non-operators are not identified
    IF result THEN result = Test_Assert&(NOT isoperator("VARIABLE"), "VARIABLE should not be identified as operator")
    IF result THEN result = Test_Assert&(NOT isoperator("FUNCTION"), "FUNCTION should not be identified as operator")
    IF result THEN result = Test_Assert&(NOT isoperator(""), "Empty string should not be identified as operator")
    
    ' Test case insensitivity
    IF result THEN result = Test_Assert&(isoperator("and"), "Lowercase 'and' should be identified as operator")
    IF result THEN result = Test_Assert&(isoperator("Or"), "Mixed case 'Or' should be identified as operator")
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_OperatorPrecedence
    Test_Start "Operator precedence recognition"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    
    ' Test that isoperator returns correct precedence levels
    ' Note: isoperator returns a precedence level (l variable)
    ' Higher values indicate higher precedence (e.g., ^ returns 15, AND returns 7)
    
    ' Test that arithmetic operators have higher precedence than logical
    ' (This is verified by the order in isoperator function)
    result = Test_Assert&(isoperator("^") > isoperator("AND"), "Exponentiation should have higher precedence than AND")
    IF result THEN result = Test_Assert&(isoperator("*") > isoperator("OR"), "Multiplication should have higher precedence than OR")
    
    ' Note: Full expression parsing tests require more complex setup
    ' and are better suited for integration tests
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all expression parsing tests
SUB RunExpressionParsingTests
    Test_IsOperator
    Test_OperatorPrecedence
END SUB
