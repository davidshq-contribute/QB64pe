'Unit Tests for QB64 Compiler Parser Module
'Tests pure utility functions from the parser module
$CONSOLE:ONLY
_DEST _CONSOLE

'$INCLUDE:'../test_harness.bi'

'Define sp for testing (normally from global/constants.bas)
DIM SHARED sp AS STRING * 1
sp = CHR$(13)

'=============================================================================
' Main Test Runner
'=============================================================================
InitTestSuite "Parser Module Tests"

TestStrNth
TestRemoveTrailingSpaces
TestIsValidIdentifierChar
TestCountElements

RunTests

END

'Include parser functions
'Note: In production, these would be included via the module system
'For testing, we define them inline to test in isolation

FUNCTION str_nth$ (x AS LONG)
    IF x = 1 THEN str_nth$ = "1st": EXIT FUNCTION
    IF x = 2 THEN str_nth$ = "2nd": EXIT FUNCTION
    IF x = 3 THEN str_nth$ = "3rd": EXIT FUNCTION
    str_nth$ = _TOSTR$(x) + "th"
END FUNCTION

FUNCTION RemoveTrailingSpaces$ (text$)
    DIM i AS LONG
    i = LEN(text$)
    DO WHILE i > 0
        IF ASC(text$, i) <> 32 THEN EXIT DO
        i = i - 1
    LOOP
    IF i = 0 THEN
        RemoveTrailingSpaces$ = ""
    ELSE
        RemoveTrailingSpaces$ = LEFT$(text$, i)
    END IF
END FUNCTION

FUNCTION IsValidIdentifierChar%% (c AS LONG)
    IF c >= 65 AND c <= 90 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION
    IF c >= 97 AND c <= 122 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION
    IF c >= 48 AND c <= 57 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION
    IF c = 95 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION
    IsValidIdentifierChar%% = 0
END FUNCTION

'Helper functions for countelements (simplified from elements.bas)
FUNCTION numelements& (a$)
    DIM p AS LONG, n AS LONG, i AS LONG
    IF a$ = "" THEN EXIT FUNCTION
    n = 1
    p = 1
    DO
        i = INSTR(p, a$, sp)
        IF i = 0 THEN numelements& = n: EXIT FUNCTION
        n = n + 1
        p = i + 1
    LOOP
END FUNCTION

FUNCTION getelement$ (a$, elenum AS LONG)
    DIM p AS LONG, n AS LONG, i AS LONG
    IF a$ = "" THEN EXIT FUNCTION
    n = 1
    p = 1
    DO
        i = INSTR(p, a$, sp)
        IF elenum = n THEN
            IF i THEN
                getelement$ = MID$(a$, p, i - p)
            ELSE
                getelement$ = RIGHT$(a$, LEN(a$) - p + 1)
            END IF
            EXIT FUNCTION
        END IF
        IF i = 0 THEN EXIT FUNCTION
        n = n + 1
        p = i + 1
    LOOP
END FUNCTION

'Stub for Give_Error (used by countelements)
DIM SHARED lastError$
SUB Give_Error (msg$)
    lastError$ = msg$
END SUB

FUNCTION countelements& (a$)
    DIM n AS LONG, c AS LONG, i AS LONG, b AS LONG
    DIM e$
    n = numelements&(a$)
    c = 1
    FOR i = 1 TO n
        e$ = getelement$(a$, i)
        IF e$ = "(" THEN b = b + 1
        IF e$ = ")" THEN b = b - 1
        IF b < 0 THEN Give_Error "Unexpected ) encountered": EXIT FUNCTION
        IF e$ = "," AND b = 0 THEN c = c + 1
    NEXT
    countelements& = c
END FUNCTION

'=============================================================================
' Test Cases
'=============================================================================

SUB TestStrNth
    AssertEquals "1st", str_nth$(1), "str_nth: 1 -> 1st"
    AssertEquals "2nd", str_nth$(2), "str_nth: 2 -> 2nd"
    AssertEquals "3rd", str_nth$(3), "str_nth: 3 -> 3rd"
    AssertEquals "4th", str_nth$(4), "str_nth: 4 -> 4th"
    AssertEquals "5th", str_nth$(5), "str_nth: 5 -> 5th"
    AssertEquals "10th", str_nth$(10), "str_nth: 10 -> 10th"
    AssertEquals "21th", str_nth$(21), "str_nth: 21 -> 21th"
    AssertEquals "100th", str_nth$(100), "str_nth: 100 -> 100th"
END SUB

SUB TestRemoveTrailingSpaces
    AssertEquals "hello", RemoveTrailingSpaces$("hello"), "No trailing spaces"
    AssertEquals "hello", RemoveTrailingSpaces$("hello   "), "Remove 3 trailing spaces"
    AssertEquals "hello world", RemoveTrailingSpaces$("hello world  "), "Remove spaces, keep internal"
    AssertEquals "", RemoveTrailingSpaces$(""), "Empty string"
    AssertEquals "", RemoveTrailingSpaces$("   "), "All spaces"
    AssertEquals "a", RemoveTrailingSpaces$("a "), "Single char with space"
END SUB

SUB TestIsValidIdentifierChar
    'Test uppercase letters
    AssertTrue IsValidIdentifierChar%%(65), "IsValidIdentifierChar: A"
    AssertTrue IsValidIdentifierChar%%(90), "IsValidIdentifierChar: Z"

    'Test lowercase letters
    AssertTrue IsValidIdentifierChar%%(97), "IsValidIdentifierChar: a"
    AssertTrue IsValidIdentifierChar%%(122), "IsValidIdentifierChar: z"

    'Test digits
    AssertTrue IsValidIdentifierChar%%(48), "IsValidIdentifierChar: 0"
    AssertTrue IsValidIdentifierChar%%(57), "IsValidIdentifierChar: 9"

    'Test underscore
    AssertTrue IsValidIdentifierChar%%(95), "IsValidIdentifierChar: _"

    'Test invalid characters
    AssertFalse IsValidIdentifierChar%%(32), "IsValidIdentifierChar: space (invalid)"
    AssertFalse IsValidIdentifierChar%%(33), "IsValidIdentifierChar: ! (invalid)"
    AssertFalse IsValidIdentifierChar%%(64), "IsValidIdentifierChar: @ (invalid)"
    AssertFalse IsValidIdentifierChar%%(45), "IsValidIdentifierChar: - (invalid)"
END SUB

SUB TestCountElements
    'Helper to build element strings with sp separator
    DIM s$

    'Single element
    s$ = "a"
    AssertEqualsLong 1, countelements&(s$), "countelements: single element"

    'Two elements separated by comma
    s$ = "a" + sp + "," + sp + "b"
    AssertEqualsLong 2, countelements&(s$), "countelements: two elements"

    'Three elements
    s$ = "a" + sp + "," + sp + "b" + sp + "," + sp + "c"
    AssertEqualsLong 3, countelements&(s$), "countelements: three elements"

    'Elements with parentheses (commas inside parens don't count)
    s$ = "a" + sp + "," + sp + "(" + sp + "b" + sp + "," + sp + "c" + sp + ")"
    AssertEqualsLong 2, countelements&(s$), "countelements: nested parens"

    'Multiple commas in nested parens
    s$ = "func" + sp + "(" + sp + "x" + sp + "," + sp + "y" + sp + "," + sp + "z" + sp + ")"
    AssertEqualsLong 1, countelements&(s$), "countelements: all inside parens"

    'Empty string
    s$ = ""
    AssertEqualsLong 1, countelements&(s$), "countelements: empty string"
END SUB

'$INCLUDE:'../test_harness.bas'
