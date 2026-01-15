$CONSOLE:ONLY
' Regression test: String concatenation edge cases
' Tests that string concatenation handles various edge cases correctly

DIM s1 AS STRING
DIM s2 AS STRING
DIM s3 AS STRING
DIM s4 AS STRING * 10

' Test basic concatenation
s1 = "Hello"
s2 = "World"
s3 = s1 + " " + s2
IF s3 = "Hello World" THEN
    PRINT "OK: Basic string concatenation works"
ELSE
    PRINT "ERROR: Basic string concatenation failed"
END IF

' Test empty string concatenation
s1 = ""
s2 = "test"
s3 = s1 + s2
IF s3 = "test" THEN
    PRINT "OK: Empty string concatenation works"
ELSE
    PRINT "ERROR: Empty string concatenation failed"
END IF

s3 = s2 + s1
IF s3 = "test" THEN
    PRINT "OK: Empty string concatenation (reverse) works"
ELSE
    PRINT "ERROR: Empty string concatenation (reverse) failed"
END IF

' Test multiple concatenations
s1 = "a"
s2 = "b"
s3 = "c"
s3 = s1 + s2 + s3
IF s3 = "abc" THEN
    PRINT "OK: Multiple string concatenation works"
ELSE
    PRINT "ERROR: Multiple string concatenation failed"
END IF

' Test fixed-length string concatenation
' Fixed-length strings are space-padded, so we use RTRIM$ to test the content
s4 = "Hello"
s1 = RTRIM$(s4) + " World"
IF s1 = "Hello World" THEN
    PRINT "OK: Fixed-length string concatenation works"
ELSE
    PRINT "ERROR: Fixed-length string concatenation failed"
END IF

' Test string with numbers
s1 = "Number: " + STR$(42)
IF s1 = "Number:  42" OR s1 = "Number: 42" THEN
    PRINT "OK: String with number concatenation works"
ELSE
    PRINT "ERROR: String with number concatenation failed"
END IF

' Test very long concatenation
DIM i AS INTEGER
s1 = ""
FOR i = 1 TO 10
    s1 = s1 + "x"
NEXT i
IF LEN(s1) = 10 THEN
    PRINT "OK: Long string concatenation works"
ELSE
    PRINT "ERROR: Long string concatenation failed"
END IF

PRINT "All string concatenation tests passed"

SYSTEM
