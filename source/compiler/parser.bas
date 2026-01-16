'QB64 Compiler Parser Module - Implementation
'Contains utility functions for parsing and tokenization
'
'Note: Functions like str_nth$, countelements, validname still reside in qb64pe.bas
'They will be migrated here incrementally as the modularization progresses.

FUNCTION RemoveTrailingSpaces$ (text$)
    'Removes trailing spaces from a string
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
    'Returns true if character is valid in an identifier (letter, digit, or underscore)
    IF c >= 65 AND c <= 90 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION   'A-Z
    IF c >= 97 AND c <= 122 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION  'a-z
    IF c >= 48 AND c <= 57 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION   '0-9
    IF c = 95 THEN IsValidIdentifierChar%% = -1: EXIT FUNCTION                '_
    IsValidIdentifierChar%% = 0
END FUNCTION

'=============================================================================
' String Utilities (migrated from qb64pe.bas)
'=============================================================================

FUNCTION str_nth$ (x AS LONG)
    'Returns ordinal string for a number (1st, 2nd, 3rd, 4th, etc.)
    IF x = 1 THEN str_nth$ = "1st": EXIT FUNCTION
    IF x = 2 THEN str_nth$ = "2nd": EXIT FUNCTION
    IF x = 3 THEN str_nth$ = "3rd": EXIT FUNCTION
    str_nth$ = _TOSTR$(x) + "th"
END FUNCTION

'=============================================================================
' Element Counting (migrated from qb64pe.bas)
'=============================================================================

FUNCTION countelements& (a$)
    'Counts the number of comma-separated elements, respecting parentheses nesting
    'Uses numelements and getelement$ from elements.bas
    DIM n AS LONG, c AS LONG, i AS LONG, b AS LONG
    DIM e$
    n = numelements(a$)
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
