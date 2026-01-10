'
' Integration Test: Error Recovery
'
' Tests that the compiler handles errors gracefully and
' can continue processing after encountering errors (where applicable).
'

' Test ON ERROR GOTO error recovery
ON ERROR GOTO ErrorHandler

DIM x AS LONG
x = 1 / 0 ' This will trigger an error

PRINT "This should not print"

ErrorHandler:
PRINT "Error handled successfully"
RESUME NEXT

PRINT "Program continued after error"

' Expected output:
' Error handled successfully
' Program continued after error
