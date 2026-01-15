$CONSOLE:ONLY
' Regression test: Array bounds checking
' Tests that array access respects bounds and handles edge cases

DIM arr(0 TO 9) AS INTEGER
DIM arr2(1 TO 10) AS INTEGER
DIM arr3(-5 TO 5) AS INTEGER

' Test valid array access
arr(0) = 100
arr(9) = 200
arr2(1) = 300
arr2(10) = 400
arr3(-5) = 500
arr3(5) = 600

' Test array bounds with LBOUND and UBOUND
IF LBOUND(arr) = 0 AND UBOUND(arr) = 9 THEN
    PRINT "OK: Array bounds (0 TO 9) correct"
ELSE
    PRINT "ERROR: Array bounds wrong"
END IF

IF LBOUND(arr2) = 1 AND UBOUND(arr2) = 10 THEN
    PRINT "OK: Array bounds (1 TO 10) correct"
ELSE
    PRINT "ERROR: Array bounds wrong"
END IF

IF LBOUND(arr3) = -5 AND UBOUND(arr3) = 5 THEN
    PRINT "OK: Array bounds (-5 TO 5) correct"
ELSE
    PRINT "ERROR: Array bounds wrong"
END IF

' Test array element access
IF arr(0) = 100 AND arr(9) = 200 THEN
    PRINT "OK: Array element access works"
ELSE
    PRINT "ERROR: Array element access failed"
END IF

' Test multi-dimensional array
DIM arr2d(1 TO 3, 1 TO 3) AS INTEGER
arr2d(1, 1) = 10
arr2d(3, 3) = 30

IF arr2d(1, 1) = 10 AND arr2d(3, 3) = 30 THEN
    PRINT "OK: Multi-dimensional array access works"
ELSE
    PRINT "ERROR: Multi-dimensional array access failed"
END IF

IF LBOUND(arr2d, 1) = 1 AND UBOUND(arr2d, 1) = 3 THEN
    PRINT "OK: Multi-dimensional array bounds correct"
ELSE
    PRINT "ERROR: Multi-dimensional array bounds wrong"
END IF

PRINT "All array bounds tests passed"

SYSTEM
