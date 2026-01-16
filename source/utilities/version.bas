'
' Version comparison utility functions
'

'
' Compares two semantic version strings (e.g., "1.2.3" vs "1.2.4")
' Returns: -1 if v$ < v1$, 0 if equal, 1 if v$ > v1$
'
FUNCTION CompareVersions (v$, v1$)
    t$ = v$: t1$ = v1$ 'temp strings so we don't change the passed values
    IF RIGHT$(t$, 8) = "-UNKNOWN" THEN t$ = LEFT$(t$, LEN(t$) - 8)
    IF RIGHT$(t1$, 8) = "-UNKNOWN" THEN t1$ = LEFT$(t1$, LEN(t1$) - 8)
    DO
        l = INSTR(t$, "."): l1 = INSTR(t1$, ".")
        IF l THEN '                       the first value has a period still
            v& = VAL(LEFT$(t$, l - 1)) '  take what's to the left of that period for our value
            t$ = MID$(t$, l + 1) '        strip that period and everything to the left off for the next pass
        ELSE
            v& = VAL(t$) '                no period?  Then this is our final pass
            t$ = ""
        END IF
        IF l1 THEN
            v1& = VAL(LEFT$(t1$, l1 - 1))
            t1$ = MID$(t1$, l1 + 1)
        ELSE
            v1& = VAL(t1$)
            t1$ = ""
        END IF
        IF v& < v1& THEN CompareVersions = -1: EXIT FUNCTION
        IF v& > v1& THEN CompareVersions = 1: EXIT FUNCTION
        IF t$ = "" AND t1$ = "" THEN EXIT FUNCTION 'return value 0 -- they're equal
        IF t$ = "" AND t1$ <> "" THEN CompareVersions = -1: EXIT FUNCTION
        IF t1$ = "" AND t$ <> "" THEN CompareVersions = 1: EXIT FUNCTION
    LOOP
END FUNCTION
