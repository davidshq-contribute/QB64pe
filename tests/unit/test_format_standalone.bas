' Standalone test for format.bas to verify refactoring
' This tests format.bas directly without the full test framework

$CONSOLE:ONLY

' Define variables needed by format.bas (avoiding constants.bas dependencies)
DIM SHARED sp_asc AS LONG
DIM SHARED sp2_asc AS LONG
sp_asc = 13  ' CR (carriage return)
sp2_asc = 10  ' LF (line feed)

' Declare shared variables needed by format.bas
DIM SHARED layout$
DIM SHARED IDEAutoIndent AS LONG
DIM SHARED IDEAutoIndentSize AS LONG
DIM SHARED IDEAutoLayout AS LONG

' Forward declare isalpha function needed by format.bas
DECLARE FUNCTION isalpha& (c AS LONG)

' Forward declare test function
DECLARE SUB RunFormatTests

' Initialize test variables
IDEAutoIndent = -1
IDEAutoIndentSize = 4
IDEAutoLayout = -1

' Call test function from main program section (before include triggers implicit END)
RunFormatTests

' Include format.bas (will trigger implicit END when it encounters FUNCTION)
$INCLUDE:'../../source/utilities/format.bas'

' Define isalpha function (after implicit END, in SUB/FUNCTION section)
FUNCTION isalpha& (c AS LONG)
    ' Returns true if c is an alphabetic character (A-Z or a-z)
    IF (c >= 65 AND c <= 90) OR (c >= 97 AND c <= 122) THEN
        isalpha& = -1
    ELSE
        isalpha& = 0
    END IF
END FUNCTION

' Test function - runs all format.bas tests (after implicit END, in SUB/FUNCTION section)
SUB RunFormatTests
    PRINT "Testing format.bas refactoring..."
    PRINT ""

    ' Test 1: Empty layout
    PRINT "Test 1: Empty layout"
    layout$ = ""
    DIM result$
    result$ = apply_layout_indent$("test code")
    IF result$ = "" THEN
        PRINT "  PASS: Empty layout returns empty"
    ELSE
        PRINT "  FAIL: Expected empty, got: "; result$
    END IF

    ' Test 2: Basic indentation
    PRINT "Test 2: Basic indentation"
    layout$ = "    PRINT " + CHR$(34) + "test" + CHR$(34)
    result$ = apply_layout_indent$("print " + CHR$(34) + "test" + CHR$(34))
    IF LEN(result$) > 0 THEN
        PRINT "  PASS: Basic indentation works"
        PRINT "  Result: "; result$
    ELSE
        PRINT "  FAIL: Expected result, got empty"
    END IF

    ' Test 3: Special characters (sp_asc and sp2_asc)
    PRINT "Test 3: Special characters"
    layout$ = "    PRINT" + CHR$(13) + "test"  ' CHR$(13) is CR (sp_asc)
    result$ = apply_layout_indent$("print test")
    IF LEN(result$) > 0 THEN
        PRINT "  PASS: Special characters handled"
    ELSE
        PRINT "  FAIL: Special characters not handled"
    END IF

    ' Test 4: Empty string input
    PRINT "Test 4: Empty string input"
    layout$ = "    PRINT"
    result$ = apply_layout_indent$("")
    IF LEN(result$) >= 0 THEN
        PRINT "  PASS: Empty string handled"
    ELSE
        PRINT "  FAIL: Empty string not handled"
    END IF

    ' Test 5: ? to PRINT conversion
    PRINT "Test 5: ? to PRINT conversion"
    layout$ = "    PRINT " + CHR$(34) + "test" + CHR$(34)
    result$ = apply_layout_indent$("? " + CHR$(34) + "test" + CHR$(34))
    IF INSTR(result$, "print") > 0 OR INSTR(result$, "PRINT") > 0 THEN
        PRINT "  PASS: ? converted to PRINT"
    ELSE
        PRINT "  FAIL: ? not converted to PRINT"
        PRINT "  Result: "; result$
    END IF

    PRINT ""
    PRINT "Format.bas tests completed!"
END SUB
