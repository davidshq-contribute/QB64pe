$CONSOLE:ONLY
' Regression test: File I/O edge cases
' Tests that file operations handle edge cases correctly

DIM f AS INTEGER
DIM testFile AS STRING
DIM lineContent AS STRING

testFile = "regression_test_file.tmp"

' Test file creation and writing
f = FREEFILE
OPEN testFile FOR OUTPUT AS #f
PRINT #f, "Line 1"
PRINT #f, "Line 2"
PRINT #f, ""
PRINT #f, "Line 4"
CLOSE #f

' Test file reading
f = FREEFILE
OPEN testFile FOR INPUT AS #f
LINE INPUT #f, lineContent
IF lineContent = "Line 1" THEN
    PRINT "OK: File read first line works"
ELSE
    PRINT "ERROR: File read first line failed"
END IF

LINE INPUT #f, lineContent
IF lineContent = "Line 2" THEN
    PRINT "OK: File read second line works"
ELSE
    PRINT "ERROR: File read second line failed"
END IF

LINE INPUT #f, lineContent
IF lineContent = "" THEN
    PRINT "OK: File read empty line works"
ELSE
    PRINT "ERROR: File read empty line failed"
END IF

LINE INPUT #f, lineContent
IF lineContent = "Line 4" THEN
    PRINT "OK: File read fourth line works"
ELSE
    PRINT "ERROR: File read fourth line failed"
END IF

CLOSE #f

' Test EOF detection
f = FREEFILE
OPEN testFile FOR INPUT AS #f
DO WHILE NOT EOF(f)
    LINE INPUT #f, lineContent
LOOP
IF EOF(f) THEN
    PRINT "OK: EOF detection works"
ELSE
    PRINT "ERROR: EOF detection failed"
END IF
CLOSE #f

' Test file existence
IF _FILEEXISTS(testFile) THEN
    PRINT "OK: _FILEEXISTS works"
ELSE
    PRINT "ERROR: _FILEEXISTS failed"
END IF

' Clean up
KILL testFile

IF NOT _FILEEXISTS(testFile) THEN
    PRINT "OK: File deletion works"
ELSE
    PRINT "ERROR: File deletion failed"
END IF

PRINT "All file I/O tests passed"

SYSTEM
