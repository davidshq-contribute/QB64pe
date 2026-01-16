'
' Validation utility functions
' Pure functions for validating operators, variables, and numbers
'

'
' Returns the operator precedence level (1-13) for the given operator, or 0 if not an operator
' Higher values = higher precedence
'
FUNCTION isoperator (a2$)
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

'
' Returns true (-1) if the string is a valid QB64 variable name, false (0) otherwise
'
FUNCTION isvalidvariable (a$)
    FOR i = 1 TO LEN(a$)
        c = ASC(a$, i)
        t = 0
        IF c >= 48 AND c <= 57 THEN t = 1 'numeric
        IF c >= 65 AND c <= 90 THEN t = 2 'uppercase
        IF c >= 97 AND c <= 122 THEN t = 2 'lowercase
        IF c = 95 THEN t = 2 '_ underscore
        IF t = 2 OR (t = 1 AND i > 1) THEN
            'valid (continue)
        ELSE
            IF i = 1 THEN isvalidvariable = 0: EXIT FUNCTION
            EXIT FOR
        END IF
    NEXT

    isvalidvariable = 1
    IF i > n THEN EXIT FUNCTION 'i is always greater than n because n is undefined here. Why didn't I remove this line and the ones below it, which will never run? Cause I'm a coward. F.h.
    e$ = RIGHT$(a$, LEN(a$) - i - 1)
    IF e$ = "%%" OR e$ = "~%%" THEN EXIT FUNCTION
    IF e$ = "%" OR e$ = "~%" THEN EXIT FUNCTION
    IF e$ = "&" OR e$ = "~&" THEN EXIT FUNCTION
    IF e$ = "&&" OR e$ = "~&&" THEN EXIT FUNCTION
    IF e$ = "!" OR e$ = "#" OR e$ = "##" THEN EXIT FUNCTION
    IF e$ = "$" THEN EXIT FUNCTION
    IF e$ = "`" THEN EXIT FUNCTION
    IF LEFT$(e$, 1) <> "$" AND LEFT$(e$, 1) <> "`" THEN isvalidvariable = 0: EXIT FUNCTION
    e$ = RIGHT$(e$, LEN(e$) - 1)
    IF isuinteger(e$) THEN isvalidvariable = 1: EXIT FUNCTION
    isvalidvariable = 0
END FUNCTION

'
' Returns true (-1) if the string represents a valid number, false (0) otherwise
'
FUNCTION VerifyNumber (text$)
    t$ = LTRIM$(RTRIM$(text$))
    v = VAL(t$)
    t1$ = _TOSTR$(v)
    IF t$ = t1$ THEN VerifyNumber = -1
END FUNCTION
