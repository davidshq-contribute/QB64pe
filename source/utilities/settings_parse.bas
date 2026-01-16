'
' Settings parsing utility functions
' Parse key=value style settings from tokens
'

'
' Parses a boolean value from a "key=value" token
' Returns -1 (true) if successfully parsed, 0 if not
' Sets 'setting' to the parsed boolean value (_TRUE or _FALSE)
'
FUNCTION ParseBooleanSetting& (token$, setting AS LONG)
    DIM equals AS LONG
    DIM value AS STRING

    equals = INSTR(token$, "=")
    IF equals = 0 THEN ParseBooleanSetting& = 0: EXIT FUNCTION

    value = LCASE$(MID$(token$, equals + 1))

    SELECT CASE value
        CASE "true", "on", "yes", "1", "-1"
            setting = _TRUE
            ParseBooleanSetting& = -1

        CASE "false", "off", "no", "0"
            setting = _FALSE
            ParseBooleanSetting& = -1

        CASE ELSE
            ParseBooleanSetting& = 0
    END SELECT
END FUNCTION

'
' Parses a long integer value from a "key=value" token
' Returns -1 (true) if successfully parsed, 0 if not
' Sets 'setting' to the parsed numeric value
'
FUNCTION ParseLongSetting& (token$, setting AS LONG)
    DIM equals AS LONG

    equals = INSTR(token$, "=")
    IF equals = 0 THEN ParseLongSetting& = 0: EXIT FUNCTION

    setting = VAL(MID$(token$, equals + 1))

    ParseLongSetting& = -1
END FUNCTION

'
' Parses a string value from a "key=value" token
' Returns -1 (true) if successfully parsed, 0 if not
' Sets 'setting' to the parsed string value
'
FUNCTION ParseStringSetting& (token$, setting AS STRING)
    DIM equals AS LONG

    equals = INSTR(token$, "=")
    IF equals = 0 THEN ParseStringSetting& = 0: EXIT FUNCTION

    setting = MID$(token$, equals + 1)

    ParseStringSetting& = -1
END FUNCTION
