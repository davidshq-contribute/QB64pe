$CONSOLE:ONLY
' Regression test: Empty string handling
' Tests that empty strings are handled correctly in various operations

DIM s AS STRING
DIM s2 AS STRING
DIM s3 AS STRING * 10

' Test empty string assignment
s = ""
s2 = s

' Test empty string concatenation
s = s + ""
s = "" + s
s = s + "test"
s = "test" + s

' Test empty string comparison
IF s = "" THEN
    PRINT "ERROR: Should not be empty"
ELSE
    PRINT "OK: Empty string comparison works"
END IF

' Test empty fixed-length string
s3 = ""
IF LEN(s3) = 10 THEN
    PRINT "OK: Fixed-length empty string has correct length"
ELSE
    PRINT "ERROR: Fixed-length string length wrong"
END IF

' Test empty string in function calls
IF LEN("") = 0 THEN
    PRINT "OK: LEN of empty string is 0"
ELSE
    PRINT "ERROR: LEN of empty string wrong"
END IF

' Test MID$ with empty string
s = MID$("", 1, 1)
IF s = "" THEN
    PRINT "OK: MID$ with empty string returns empty"
ELSE
    PRINT "ERROR: MID$ with empty string failed"
END IF

' Test LEFT$ and RIGHT$ with empty string
IF LEFT$("", 5) = "" AND RIGHT$("", 5) = "" THEN
    PRINT "OK: LEFT$/RIGHT$ with empty string work"
ELSE
    PRINT "ERROR: LEFT$/RIGHT$ with empty string failed"
END IF

PRINT "All empty string tests passed"

SYSTEM
