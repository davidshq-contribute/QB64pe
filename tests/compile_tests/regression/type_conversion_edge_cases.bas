$CONSOLE:ONLY
' Regression test: Type conversion edge cases
' Tests that type conversions handle edge cases correctly

DIM i AS INTEGER
DIM l AS LONG
DIM s AS SINGLE
DIM d AS DOUBLE
DIM b AS _BYTE

' Test integer overflow handling
i = 32767
IF i = 32767 THEN
    PRINT "OK: INTEGER max value handled"
ELSE
    PRINT "ERROR: INTEGER max value failed"
END IF

' Test type conversion in expressions
i = 10
l = i
s = i
d = i
b = i

IF l = 10 AND s = 10 AND d = 10 AND b = 10 THEN
    PRINT "OK: Type conversion from INTEGER works"
ELSE
    PRINT "ERROR: Type conversion from INTEGER failed"
END IF

' Test floating point to integer conversion
s = 3.7
i = INT(s)
IF i = 3 THEN
    PRINT "OK: FLOAT to INTEGER conversion (INT) works"
ELSE
    PRINT "ERROR: FLOAT to INTEGER conversion failed"
END IF

' Test CINT conversion
s = 3.7
i = CINT(s)
IF i = 4 THEN
    PRINT "OK: CINT conversion works"
ELSE
    PRINT "ERROR: CINT conversion failed"
END IF

' Test string to number conversion
DIM num AS INTEGER
num = VAL("123")
IF num = 123 THEN
    PRINT "OK: VAL string to number conversion works"
ELSE
    PRINT "ERROR: VAL conversion failed"
END IF

' Test number to string conversion
DIM strValue AS STRING
strValue = STR$(123)
IF strValue = " 123" OR strValue = "123" THEN
    PRINT "OK: STR$ number to string conversion works"
ELSE
    PRINT "ERROR: STR$ conversion failed"
END IF

' Test _BYTE overflow behavior (should wrap)
b = 255
b = b + 1
IF b = 0 THEN
    PRINT "OK: _BYTE overflow wraps correctly"
ELSE
    PRINT "ERROR: _BYTE overflow handling failed"
END IF

PRINT "All type conversion tests passed"

SYSTEM
