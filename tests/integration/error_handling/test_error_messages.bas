'
' Integration Test: Error Message Testing
'
' Tests that compiler produces appropriate error messages
' for various error conditions.
'

' This test should produce a compilation error
' Expected error: Variable not defined or type mismatch

DIM x AS LONG
x = "invalid" ' This should produce a type mismatch error
