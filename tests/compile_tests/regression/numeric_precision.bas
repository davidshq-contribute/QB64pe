$CONSOLE:ONLY
' Regression test: Numeric precision edge cases
' Tests that numeric operations maintain correct precision

DIM s AS SINGLE
DIM d AS DOUBLE
DIM i AS INTEGER
DIM l AS LONG

' Test SINGLE precision
s = 0.1
s = s + 0.2
IF ABS(s - 0.3) < 0.0001 THEN
    PRINT "OK: SINGLE precision addition works"
ELSE
    PRINT "ERROR: SINGLE precision addition failed"
END IF

' Test DOUBLE precision
d = 0.1
d = d + 0.2
IF ABS(d - 0.3) < 0.0000001 THEN
    PRINT "OK: DOUBLE precision addition works"
ELSE
    PRINT "ERROR: DOUBLE precision addition failed"
END IF

' Test integer division
i = 7 \ 3
IF i = 2 THEN
    PRINT "OK: Integer division works"
ELSE
    PRINT "ERROR: Integer division failed"
END IF

' Test MOD operation
i = 7 MOD 3
IF i = 1 THEN
    PRINT "OK: MOD operation works"
ELSE
    PRINT "ERROR: MOD operation failed"
END IF

' Test floating point division
d = 7 / 3
IF ABS(d - 2.333333) < 0.0001 THEN
    PRINT "OK: Floating point division works"
ELSE
    PRINT "ERROR: Floating point division failed"
END IF

' Test power operation
d = 2 ^ 10
IF d = 1024 THEN
    PRINT "OK: Power operation works"
ELSE
    PRINT "ERROR: Power operation failed"
END IF

' Test negative numbers
i = -5
IF i = -5 THEN
    PRINT "OK: Negative number handling works"
ELSE
    PRINT "ERROR: Negative number handling failed"
END IF

' Test large numbers
l = 2147483647
IF l = 2147483647 THEN
    PRINT "OK: Large number handling works"
ELSE
    PRINT "ERROR: Large number handling failed"
END IF

PRINT "All numeric precision tests passed"

SYSTEM
