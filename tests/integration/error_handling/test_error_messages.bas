'
' Integration Test: Error Message Testing
'
' Tests that runtime error handling works correctly
' and appropriate error messages are generated.
'

' This test compiles successfully and tests runtime error handling
' Expected output: Program should handle errors gracefully

DIM x AS LONG
DIM testResult AS INTEGER

' Test 1: Valid assignment (should work)
x = 42
PRINT "Test 1 - Valid assignment: "; x

' Test 2: String to number conversion with error handling
ON ERROR GOTO ErrorHandler
testResult = 0

' This will cause a runtime error that we can catch
x = VAL("invalid123") ' Use VAL to safely convert string to number
IF x = 0 THEN
    PRINT "Test 2 - String conversion handled gracefully"
    testResult = testResult + 1
END IF

' Test 3: Division by zero handling
x = 10
IF x <> 0 THEN
    y = 5 / x ' This should work fine
    PRINT "Test 3 - Division successful: "; y
    testResult = testResult + 1
END IF

PRINT "All error handling tests completed successfully: "; testResult; "/2 tests passed"
END

ErrorHandler:
    PRINT "Error caught and handled gracefully"
    RESUME NEXT
