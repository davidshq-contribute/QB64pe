'
' Parser Utility Functions
'
' Utility functions for parser operations that can be used independently.
'

' Determines if a string is an operator and returns its precedence level.
' Returns 0 if the string is not an operator.
' Higher return values indicate higher precedence (e.g., ^ returns 15, AND returns 7).
'
' @param a2$ The string to check
' @return The precedence level (1-15) if operator, 0 if not an operator
FUNCTION isoperator (a2$)
    DIM a$
    DIM l AS LONG
    
    a$ = UCASE$(a2$)
    l = 0
    l = l + 1: IF a$ = "_ORELSE" THEN GOTO opfound
    l = l + 1: IF a$ = "_ANDALSO" THEN GOTO opfound
    l = l + 1: IF a$ = "IMP" THEN GOTO opfound
    l = l + 1: IF a$ = "EQV" THEN GOTO opfound
    l = l + 1: IF a$ = "XOR" THEN GOTO opfound
    l = l + 1: IF a$ = "OR" THEN GOTO opfound
    l = l + 1: IF a$ = "AND" THEN GOTO opfound
    l = l + 1: IF a$ = "_NEGATE" THEN GOTO opfound
    l = l + 1: IF a$ = "NOT" THEN GOTO opfound
    l = l + 1
    IF a$ = "=" THEN GOTO opfound
    IF a$ = ">" THEN GOTO opfound
    IF a$ = "<" THEN GOTO opfound
    IF a$ = "<>" THEN GOTO opfound
    IF a$ = "<=" THEN GOTO opfound
    IF a$ = ">=" THEN GOTO opfound
    l = l + 1
    IF a$ = "+" THEN GOTO opfound
    IF a$ = "-" THEN GOTO opfound '!CAREFUL! could be negation
    l = l + 1: IF a$ = "MOD" THEN GOTO opfound
    l = l + 1: IF a$ = "\" THEN GOTO opfound
    l = l + 1
    IF a$ = "*" THEN GOTO opfound
    IF a$ = "/" THEN GOTO opfound
    'NEGATION LEVEL (MUST BE SET AFTER CALLING ISOPERATOR BY CONTEXT)
    l = l + 1: IF a$ = CHR$(241) THEN GOTO opfound
    l = l + 1: IF a$ = "^" THEN GOTO opfound
    EXIT FUNCTION
    opfound:
    isoperator = l
END FUNCTION
