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

'$INCLUDE:'../test_harness.bas'
