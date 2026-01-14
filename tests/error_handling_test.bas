' Test program to verify error handling refactoring works correctly
' This tests all the error handling functionality that was refactored

' Test 1: Basic error handling
ON ERROR GOTO ErrorHandler

PRINT "Testing basic error handling..."

' Cause an error
DIM x AS INTEGER
x = 1 / 0  ' Division by zero error

END

ErrorHandler:
PRINT "Error caught! ERR="; ERR
PRINT "ERL="; ERL
RESUME NEXT

' Test 2: Error handling with RESUME
PRINT "Testing RESUME..."
ON ERROR GOTO ErrorHandler2

DIM y AS INTEGER
y = 1 / 0

END

ErrorHandler2:
PRINT "Error 2 caught! RESUMING..."
RESUME

' Test 3: Error handling with RESUME label
PRINT "Testing RESUME label..."
ON ERROR GOTO ErrorHandler3

DIM z AS INTEGER
z = 1 / 0

END

ErrorHandler3:
PRINT "Error 3 caught! RESUMING to ContinuePoint..."
RESUME ContinuePoint

ContinuePoint:
PRINT "Resumed to ContinuePoint"

' Test 4: ON ERROR GOTO 0 (disable error handling)
PRINT "Testing error handling disable..."
ON ERROR GOTO 0

' This should cause program termination
DIM w AS INTEGER
w = 1 / 0
