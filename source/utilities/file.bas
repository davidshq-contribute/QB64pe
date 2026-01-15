'
' Helper function: Checks if a character code represents a path separator
'
' Returns: True if the character is '/' (47) or '\' (92)
FUNCTION IsPathSeparator%% (charCode AS LONG)
    IsPathSeparator = (charCode = 47) OR (charCode = 92)
END FUNCTION

'
' Helper function: Finds the position of the last path separator in a string
'
' Returns: Position of last '/' or '\', or 0 if not found
FUNCTION FindLastPathSeparator& (path$ AS STRING)
    DIM AS LONG i
    DIM a$
    
    FOR i = LEN(path$) TO 1 STEP -1
        a$ = MID$(path$, i, 1)
        IF a$ = "/" OR a$ = "\" THEN
            FindLastPathSeparator = i
            EXIT FUNCTION
        END IF
    NEXT
    FindLastPathSeparator = 0
END FUNCTION

'
' Helper function: Finds the position of the last dot (for file extensions)
' Stops searching if a path separator is encountered
'
' Returns: Position of last '.', or 0 if not found
FUNCTION FindLastDot& (filename$ AS STRING)
    DIM AS LONG i, a
    
    FOR i = LEN(filename$) TO 1 STEP -1
        a = ASC(filename$, i)
        IF IsPathSeparator(a) THEN EXIT FOR
        IF a = 46 THEN
            FindLastDot = i
            EXIT FUNCTION
        END IF
    NEXT
    FindLastDot = 0
END FUNCTION

'
' Duplicates the contents of one file into another
'
' Returns: 0 on success, -1 error reading source
'                         1 error writing destination
FUNCTION CopyFile& (sourceFile$, destFile$)

    ON ERROR GOTO _NEWHANDLER qberror_test
    E = 0: dat$ = _READFILE$(sourceFile$)
    IF E = 0 THEN _WRITEFILE destFile$, dat$ ELSE E = -1
    ON ERROR GOTO _LASTHANDLER

    CopyFile& = E
END FUNCTION

'
' Splits the filename from its path, and returns the path
'
' Returns: The path + trailing separator, or empty if no path
FUNCTION getfilepath$ (f$)
    DIM AS LONG pos
    pos = FindLastPathSeparator(f$)
    IF pos > 0 THEN
        getfilepath$ = LEFT$(f$, pos)
    ELSE
        getfilepath$ = ""
    END IF
END FUNCTION

'
' Checks if a filename has an extension on the end
'
' Returns: True if provided filename has an extension
FUNCTION FileHasExtension (f$)
    FileHasExtension = (FindLastDot(f$) > 0)
END FUNCTION

'
' Strips the extension off of a filename
'
' Returns: Provided filename without extension on the end
FUNCTION RemoveFileExtension$ (f$)
    DIM AS LONG dotPos
    dotPos = FindLastDot(f$)
    IF dotPos > 0 THEN
        RemoveFileExtension$ = LEFT$(f$, dotPos - 1)
    ELSE
        RemoveFileExtension$ = f$
    END IF
END FUNCTION

'
' Returns the extension on the end of a file name
'
' Returns "" if there is no extension
'
FUNCTION GetFileExtension$ (f$)
    DIM AS LONG dotPos
    dotPos = FindLastDot(f$)
    IF dotPos > 0 THEN
        GetFileExtension$ = MID$(f$, dotPos + 1)
    ELSE
        GetFileExtension$ = ""
    END IF
END FUNCTION

'
' Fixes the provided filename and path to use the correct path separator
'
SUB PATH_SLASH_CORRECT (a$)
    IF os$ = "WIN" THEN
        FOR x = 1 TO LEN(a$)
            IF ASC(a$, x) = 47 THEN ASC(a$, x) = 92
        NEXT
    ELSE
        FOR x = 1 TO LEN(a$)
            IF ASC(a$, x) = 92 THEN ASC(a$, x) = 47
        NEXT
    END IF
END SUB

' Return a pathname where all "\" are correctly escaped
FUNCTION GetEscapedPath$ (path_name AS STRING)
    DIM buf AS STRING, z AS _UNSIGNED LONG, a AS _UNSIGNED _BYTE

    FOR z = 1 TO LEN(path_name)
        a = ASC(path_name, z)
        buf = buf + CHR$(a)
        IF a = 92 THEN buf = buf + "\"
    NEXT

    GetEscapedPath = buf
END FUNCTION

' Returns a path/file with single slashes only, effectively unescaping "\"
FUNCTION RemoveDoubleSlashes$ (f2$)
    f$ = f2$ 'avoid arg side effects

    DO 'sp% = 0 at function entry
        sp% = INSTR(sp% + 1, f$, "//")
        IF sp% > 0 THEN f$ = LEFT$(f$, sp% - 1) + MID$(f$, sp% + 1)
    LOOP UNTIL sp% = 0
    DO 'sp% = 0 again from 1st loop end
        sp% = INSTR(sp% + 1, f$, "\\")
        IF sp% > 0 THEN f$ = LEFT$(f$, sp% - 1) + MID$(f$, sp% + 1)
    LOOP UNTIL sp% = 0

    RemoveDoubleSlashes$ = f$
END FUNCTION

' Adds a trailing \ or / to a directory name if needed
FUNCTION FixDirectoryName$ (dir_name AS STRING)
    IF LEN(dir_name) > 0 AND RIGHT$(dir_name, 1) <> pathsep$ THEN
        FixDirectoryName = dir_name + pathsep$
    ELSE
        FixDirectoryName = dir_name
    END IF
END FUNCTION

