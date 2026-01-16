'
' Preprocessor utilities module
' Contains functions for handling preprocessor directives ($IF, $LET, etc.)
'
' This module contains preprocessor functions extracted from qb64pe.bas for better modularity.
' All functions depend on global variables and functions defined in qb64pe.bas and other modules.

'
' Sets a preprocessor LET value ($LET flag=value)
' First looks to see if we have an existing setting like this and if so, updates it
' Otherwise creates a new setting and sets the initial value for it
'
SUB SetPreLET (flagName$, flagValue$)
    DIM i AS LONG
    'First look to see if we have an existing setting like this and if so, update it
    FOR i = UserDefineCountPresets + 1 TO UserDefineCount 'exclude OS/BIT/Compiler detection & version
        IF UserDefine(0, i) = flagName$ THEN UserDefine(1, i) = flagValue$: EXIT SUB
    NEXT
    'Otherwise create a new setting and set the initial value for it
    UserDefineCount = UserDefineCount + 1
    IF UserDefineCount > UBOUND(UserDefine, 2) THEN
        REDIM _PRESERVE UserDefine(1, UBOUND(UserDefine, 2) + 10) 'Add another 10 elements to the array so it'll expand as the user adds to it
    END IF
    UserDefine(0, UserDefineCount) = flagName$
    UserDefine(1, UserDefineCount) = flagValue$
    UserDefineList$ = UserDefineList$ + flagName$ + "@" 'for highlighting
END SUB

'
' Evaluates a preprocessor IF condition ($IF condition)
' Returns: -1 if condition is true, 0 if false
' err$ is set to an error message if evaluation fails
'
FUNCTION EvalPreIF (text$, err$)
    DIM temp$, firstsymbol$, secondsymbol$, leftside$, rightside$, result$, l$, r$, symbol$, m$, t$, a$
    DIM first AS LONG, second AS LONG, rightstart AS LONG, rightstop AS LONG, temp AS LONG, t AS LONG
    DIM firstsymbol AS LONG, secondsymbol AS LONG, i AS LONG
    DIM UserFound AS LONG, leftresult AS LONG, rightresult AS LONG, result AS LONG
    DIM PC_Op(3) AS STRING
    temp$ = text$ 'so we don't corrupt the string sent to us for evaluation
    err$ = "" 'null the err message to begin with
    'first order of business is to solve for <>=
    PC_Op(1) = "="
    PC_Op(2) = "<"
    PC_Op(3) = ">"
    DO
        'look for the existence of the first symbol if there is any
        firstsymbol$ = "": first = 0
        FOR i = 1 TO UBOUND(PC_Op)
            temp = INSTR(temp$, PC_Op(i))
            IF first = 0 THEN first = temp: firstsymbol$ = PC_Op(i)
            IF temp <> 0 AND temp < first THEN first = temp: firstsymbol$ = PC_Op(i)
        NEXT
        IF firstsymbol$ <> "" THEN 'we've got = < >; let's see if we have a combination of them
            secondsymbol = 0: second = 0
            FOR i = first + 1 TO LEN(temp$)
                a$ = MID$(temp$, i, 1)
                SELECT CASE a$
                    CASE " " 'ignore spaces
                    CASE "=", "<", ">"
                        IF a$ = firstsymbol$ THEN err$ = "Duplicate operator (" + a$ + ")": EXIT FUNCTION
                        second = i: secondsymbol$ = a$
                    CASE ELSE 'we found a symbol we don't recognize
                        EXIT FOR
                END SELECT
            NEXT
        END IF
        IF first THEN 'we found a symbol
            l$ = RTRIM$(LEFT$(temp$, first - 1))
            IF second THEN rightstart = second + 1 ELSE rightstart = first + 1

            r$ = LTRIM$(MID$(temp$, rightstart))
            symbol$ = MID$(temp$, first, 1) + MID$(temp$, second, 1)
            'now we check for spaces to separate this segment from any other AND/OR conditions and such
            FOR i = LEN(l$) TO 1 STEP -1
                IF ASC(l$, i) = 32 THEN EXIT FOR
            NEXT
            leftside$ = RTRIM$(LEFT$(temp$, i))
            l$ = LTRIM$(RTRIM$(MID$(temp$, i + 1, LEN(l$) - i)))
            rightstop = LEN(r$)
            FOR i = 1 TO LEN(r$)
                IF ASC(r$, i) = 32 THEN EXIT FOR
            NEXT
            rightside$ = LTRIM$(MID$(r$, i + 1))
            r$ = LTRIM$(RTRIM$(LEFT$(r$, i - 1)))
            IF symbol$ = "=<" THEN symbol$ = "<="
            IF symbol$ = "=>" THEN symbol$ = ">="
            IF symbol$ = "><" THEN symbol$ = "<>"
            result$ = " 0 "
            IF symbol$ = "<>" THEN 'check to see if we're NOT equal in any case with <>
                FOR i = 0 TO UserDefineCount
                    IF UserDefine(0, i) = l$ AND UserDefine(1, i) <> r$ THEN result$ = " -1 ": GOTO finishedcheck
                NEXT
            END IF
            IF INSTR(symbol$, "=") THEN 'check to see if we're equal in any case with =
                UserFound = 0
                IF l$ = UserDefine(0, 7) THEN 'we're comparing VERSION numbers
                    result = CompareVersions(Version$, r$) '-1 is less than, 0 is equal, +1 is greater than
                    IF result = 0 THEN result$ = " -1 ": GOTO finishedcheck
                END IF
                FOR i = 0 TO UserDefineCount
                    IF i = 7 THEN _CONTINUE
                    IF UserDefine(0, i) = l$ THEN
                        UserFound = -1
                        IF UserDefine(1, i) = r$ THEN result$ = " -1 ": GOTO finishedcheck
                    END IF
                NEXT
                IF UserFound = 0 AND LTRIM$(RTRIM$(r$)) = "UNDEFINED" THEN result$ = " -1 ": GOTO finishedcheck
                IF UserFound = -1 AND LTRIM$(RTRIM$(r$)) = "DEFINED" THEN result$ = " -1 ": GOTO finishedcheck
            END IF

            IF INSTR(symbol$, ">") THEN 'check to see if we're greater than in any case with >
                IF l$ = UserDefine(0, 7) THEN 'we're comparing VERSION numbers
                    result = CompareVersions(Version$, r$) '-1 is less than, 0 is equal, +1 is greater than
                    IF result = 1 THEN result$ = " -1 ": GOTO finishedcheck
                END IF
                FOR i = 0 TO UserDefineCount
                    IF i = 7 THEN _CONTINUE
                    IF VerifyNumber(r$) AND VerifyNumber(UserDefine(1, i)) THEN 'we're comparing numeric values
                        IF UserDefine(0, i) = l$ AND VAL(UserDefine(1, i)) > VAL(r$) THEN result$ = " -1 ": GOTO finishedcheck
                    ELSE
                        IF UserDefine(0, i) = l$ AND UserDefine(1, i) > r$ THEN result$ = " -1 ": GOTO finishedcheck
                    END IF
                NEXT
            END IF

            IF INSTR(symbol$, "<") THEN 'check to see if we're less than in any case with <
                IF l$ = UserDefine(0, 7) THEN 'we're comparing VERSION numbers
                    result = CompareVersions(Version$, r$) '-1 is less than, 0 is equal, +1 is greater than
                    IF result = -1 THEN result$ = " -1 ": GOTO finishedcheck
                END IF
                FOR i = 0 TO UserDefineCount
                    IF i = 7 THEN _CONTINUE
                    IF VerifyNumber(r$) AND VerifyNumber(UserDefine(1, i)) THEN 'we're comparing numeric values
                        IF UserDefine(0, i) = l$ AND VAL(UserDefine(1, i)) < VAL(r$) THEN result$ = " -1 ": GOTO finishedcheck
                    ELSE
                        IF UserDefine(0, i) = l$ AND UserDefine(1, i) < r$ THEN result$ = " -1 ": GOTO finishedcheck
                    END IF
                NEXT
            END IF

            finishedcheck:
            temp$ = leftside$ + result$ + rightside$
        END IF
    LOOP UNTIL first = 0

    'And at this point we should now be down to a statement with nothing but AND/OR/XORS in it

    PC_Op(1) = " AND "
    PC_Op(2) = " OR "
    PC_Op(3) = " XOR "

    DO
        first = 0
        FOR i = 1 TO UBOUND(PC_Op)
            IF PC_Op(i) <> "" THEN
                t = INSTR(temp$, PC_Op(i))
                IF first <> 0 THEN
                    IF t < first AND t <> 0 THEN first = t: firstsymbol = i
                ELSE
                    first = t: firstsymbol = i
                END IF
            END IF
        NEXT
        IF first = 0 THEN EXIT DO
        leftside$ = RTRIM$(LEFT$(temp$, first - 1))
        symbol$ = MID$(temp$, first, LEN(PC_Op(firstsymbol)))
        t$ = MID$(temp$, first + LEN(PC_Op(firstsymbol)))
        t = INSTR(t$, " ") 'the first space we come to
        IF t THEN
            m$ = LTRIM$(RTRIM$(LEFT$(t$, t - 1)))
            rightside$ = LTRIM$(MID$(t$, t))
        ELSE
            m$ = LTRIM$(MID$(t$, t))
            rightside$ = ""
        END IF
        leftresult = 0
        IF VerifyNumber(leftside$) THEN
            IF VAL(leftside$) <> 0 THEN leftresult = -1
        ELSE
            FOR i = 0 TO UserDefineCount
                IF UserDefine(0, i) = leftside$ THEN
                    t$ = LTRIM$(RTRIM$(UserDefine(1, i)))
                    IF t$ <> "0" AND t$ <> "" THEN leftresult = -1: EXIT FOR
                END IF
            NEXT
        END IF
        rightresult = 0
        IF VerifyNumber(m$) THEN
            IF VAL(m$) <> 0 THEN rightresult = -1
        ELSE
            FOR i = 0 TO UserDefineCount
                IF UserDefine(0, i) = m$ THEN
                    t$ = LTRIM$(RTRIM$(UserDefine(1, i)))
                    IF t$ <> "0" AND t$ <> "" THEN rightresult = -1: EXIT FOR
                END IF
            NEXT
        END IF
        SELECT CASE LTRIM$(RTRIM$(symbol$))
            CASE "AND"
                IF leftresult <> 0 AND rightresult <> 0 THEN result$ = " -1 " ELSE result$ = " 0 "
            CASE "OR"
                IF leftresult <> 0 OR rightresult <> 0 THEN result$ = " -1 " ELSE result$ = " 0 "
            CASE "XOR"
                IF leftresult <> rightresult THEN result$ = " -1 " ELSE result$ = " 0 "
        END SELECT
        temp$ = result$ + rightside$
    LOOP

    IF VerifyNumber(temp$) THEN
        EvalPreIF = VAL(temp$)
    ELSE
        IF INSTR(temp$, " ") THEN err$ = "Invalid Resolution of $IF; check statements" 'If we've got more than 1 statement, it's invalid
        FOR i = 0 TO UserDefineCount
            IF UserDefine(0, i) = temp$ THEN
                t$ = LTRIM$(RTRIM$(UserDefine(1, i)))
                IF t$ <> "0" AND t$ <> "" THEN EvalPreIF = -1: EXIT FOR
            END IF
        NEXT
    END IF

END FUNCTION
