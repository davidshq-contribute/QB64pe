'
' Color utility functions
' Convert between color values and _RGB32() string representations
'

'
' Parses an _RGB32(r,g,b) string and returns the color value
' If the string is not in the proper format, returns DefaultColor
'
FUNCTION VRGBS~& (text$, DefaultColor AS _UNSIGNED LONG)
    'Value of RGB String = VRGBS without a ton of typing
    'A function to get the RGB value back from a string such as _RGB32(255,255,255)
    'text$ is the string that we send to check for a value
    'DefaultColor is the value we send back if the string isn't in the proper format

    VRGBS~& = DefaultColor 'A return the default value if we can't parse the string properly
    IF UCASE$(LEFT$(text$, 4)) = "_RGB" THEN
        rpos = INSTR(text$, "(")
        gpos = INSTR(rpos, text$, ",")
        bpos = INSTR(gpos + 1, text$, ",")
        IF rpos <> 0 AND bpos <> 0 AND gpos <> 0 THEN
            red = VAL(_TRIM$(MID$(text$, rpos + 1)))
            green = VAL(_TRIM$(MID$(text$, gpos + 1)))
            blue = VAL(_TRIM$(MID$(text$, bpos + 1)))
            VRGBS~& = _RGB32(red, green, blue)
        END IF
    END IF
END FUNCTION

'
' Converts an unsigned long color value to an _RGB32(r,g,b) string
'
FUNCTION rgbs$ (c AS _UNSIGNED LONG)
    rgbs$ = "_RGB32(" + _TOSTR$(_RED32(c)) + ", " + _TOSTR$(_GREEN32(c)) + ", " + _TOSTR$(_BLUE32(c)) + ")"
END FUNCTION
