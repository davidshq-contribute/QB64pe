'
' Main Parser Test Suite
'
' Includes and runs all parser tests.
'

'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'test_expression_parsing.bas'
'$INCLUDE:'test_statement_parsing.bas'
'$INCLUDE:'test_error_handling.bas'

' Run all parser tests
SUB RunParserTests
    RunExpressionParsingTests
    RunStatementParsingTests
    RunParserErrorHandlingTests
END SUB
