'
' Integration Test: Symbol Resolution
'
' Tests that symbols are correctly resolved during compilation,
' including scope resolution and type checking.
'

' This test compiles a QB64 program and verifies symbol resolution
' Expected output: Program should compile successfully with correct symbol resolution

DIM testVar AS LONG
testVar = 42

' Test that global variable is accessible
PRINT "Global variable value: "; testVar

' Test that local variables work
SUB TestSub
    DIM localVar AS LONG
    localVar = 100
    PRINT "Local variable value: "; localVar
END SUB

CALL TestSub

' Expected output:
' Global variable value: 42
' Local variable value: 100
