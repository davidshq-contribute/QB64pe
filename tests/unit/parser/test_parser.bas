'
' Main Parser Test Suite
'
' Includes and runs all parser tests.
'
' Note: test_framework_declarations.bi and test_framework_implementations.bas are included by test_runner.bas
'$INCLUDE:'test_expression_parsing.bas'
'$INCLUDE:'test_statement_parsing.bas'
'$INCLUDE:'test_error_handling.bas'

' Run all parser tests
SUB RunParserTests
    RunExpressionParsingTests
    RunStatementParsingTests
    RunParserErrorHandlingTests
END SUB
