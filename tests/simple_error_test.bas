' Simple error handling test
PRINT "Starting error handling test..."

ON ERROR GOTO ErrorHandler

' Cause a simple error
PRINT "About to cause an error..."
DIM x AS INTEGER
x = 1 / 0

PRINT "This line should not be reached"
END

ErrorHandler:
PRINT "Error caught! ERR="; ERR
PRINT "Test completed successfully!"
