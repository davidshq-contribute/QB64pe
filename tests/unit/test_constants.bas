'
' Test Constants - Minimal version for format.bas testing
'
' Provides only the constants and variables that format.bas needs,
' avoiding dependencies on Debug and _OS$ that aren't available in test context
'

$INCLUDEONCE

' String spacer/delimiter constants (from constants.bas, minimal version)
DIM SHARED sp AS STRING * 1, sp2 AS STRING * 1, sp3 AS STRING * 1
sp = CHR$(13): sp2 = CHR$(10): sp3 = CHR$(26)  ' CR, LF, SUB

DIM SHARED sp_asc AS LONG, sp2_asc AS LONG, sp3_asc AS LONG
sp_asc = 13: sp2_asc = 10: sp3_asc = 26  ' CR, LF, SUB

' Other constants format.bas might need
DIM SHARED CHR_QUOTE AS STRING * 1: CHR_QUOTE = CHR$(34)
DIM SHARED CHR_TAB AS STRING * 1: CHR_TAB = CHR$(9)
DIM SHARED CRLF AS STRING: CRLF = CHR$(13) + CHR$(10)

' OS-specific (simplified for testing)
DIM SHARED NATIVE_LINEENDING AS STRING
NATIVE_LINEENDING = CHR$(13) + CHR$(10)  ' Default to CRLF for testing

DIM SHARED OS_BITS AS LONG: OS_BITS = 64
