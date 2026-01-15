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
