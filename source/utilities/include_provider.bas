'
' Include Provider Implementation
'
' Provides concrete implementations of include providers:
' - Filesystem provider (default, uses actual file I/O)
' - Memory provider (for testing, uses in-memory content)
' - Test provider (for unit testing, allows mocking)
'

' ============================================
' Filesystem Provider Implementation
' ============================================

FUNCTION IncludeProvider_Filesystem_FileExists& (fileName$)
    IncludeProvider_Filesystem_FileExists& = _FILEEXISTS(fileName$)
END FUNCTION

FUNCTION IncludeProvider_Filesystem_Open& (fileName$, level AS LONG)
    DIM fh AS LONG
    fh = 199 + level + 1
    
    IF EOF(fh) = 0 THEN
        ' File handle might already be open, close it first
        CLOSE #fh
    END IF
    
    ON ERROR GOTO filesystem_open_error
    OPEN fileName$ FOR BINARY AS #fh
    ON ERROR GOTO _LASTHANDLER
    
    includeProviderStates(level).fileHandle = fh
    includeProviderStates(level).fileName = fileName$
    includeProviderStates(level).providerType = INCLUDE_PROVIDER_FILESYSTEM
    includeProviderStates(level).isOpen = -1
    includeProviderStates(level).currentLine = 0
    
    IncludeProvider_Filesystem_Open& = -1
    EXIT FUNCTION
    
filesystem_open_error:
    ON ERROR GOTO _LASTHANDLER
    includeProviderStates(level).isOpen = 0
    IncludeProvider_Filesystem_Open& = 0
END FUNCTION

FUNCTION IncludeProvider_Filesystem_ReadLine$ (level AS LONG)
    DIM fh AS LONG, line$ AS STRING
    fh = includeProviderStates(level).fileHandle
    
    IF includeProviderStates(level).isOpen = 0 THEN
        IncludeProvider_Filesystem_ReadLine$ = ""
        EXIT FUNCTION
    END IF
    
    IF EOF(fh) THEN
        IncludeProvider_Filesystem_ReadLine$ = ""
        EXIT FUNCTION
    END IF
    
    LINE INPUT #fh, line$
    includeProviderStates(level).currentLine = includeProviderStates(level).currentLine + 1
    IncludeProvider_Filesystem_ReadLine$ = line$
END FUNCTION

FUNCTION IncludeProvider_Filesystem_EOF& (level AS LONG)
    DIM fh AS LONG
    fh = includeProviderStates(level).fileHandle
    
    IF includeProviderStates(level).isOpen = 0 THEN
        IncludeProvider_Filesystem_EOF& = -1
        EXIT FUNCTION
    END IF
    
    IncludeProvider_Filesystem_EOF& = EOF(fh)
END FUNCTION

SUB IncludeProvider_Filesystem_Close (level AS LONG)
    DIM fh AS LONG
    fh = includeProviderStates(level).fileHandle
    
    IF includeProviderStates(level).isOpen <> 0 AND fh > 0 THEN
        CLOSE #fh
    END IF
    
    includeProviderStates(level).isOpen = 0
    includeProviderStates(level).fileHandle = 0
    includeProviderStates(level).currentLine = 0
    includeProviderStates(level).fileName = ""
END SUB

FUNCTION IncludeProvider_Filesystem_ResolvePath$ (fileName$, basePath$)
    ' If absolute path, return as-is
    IF LEFT$(fileName$, 1) = "/" OR (LEN(fileName$) >= 2 AND MID$(fileName$, 2, 1) = ":") THEN
        IncludeProvider_Filesystem_ResolvePath$ = fileName$
        EXIT FUNCTION
    END IF
    
    ' Remove leading .\ or ./
    IF LEFT$(fileName$, 2) = ".\" OR LEFT$(fileName$, 2) = "./" THEN
        fileName$ = MID$(fileName$, 3)
    END IF
    
    ' If basePath is empty, return fileName as-is
    IF basePath$ = "" THEN
        IncludeProvider_Filesystem_ResolvePath$ = fileName$
        EXIT FUNCTION
    END IF
    
    ' Combine base path with file name
    DIM pathSep$
    IF INSTR(basePath$, "\") > 0 THEN
        pathSep$ = "\"
    ELSE
        pathSep$ = "/"
    END IF
    
    IF RIGHT$(basePath$, 1) <> "\" AND RIGHT$(basePath$, 1) <> "/" THEN
        IncludeProvider_Filesystem_ResolvePath$ = basePath$ + pathSep$ + fileName$
    ELSE
        IncludeProvider_Filesystem_ResolvePath$ = basePath$ + fileName$
    END IF
END FUNCTION

FUNCTION IncludeProvider_Filesystem_ReadAll$ (fileName$)
    ' Read entire file content for $INCLUDEONCE checking
    DIM content$ AS STRING
    ON ERROR GOTO filesystem_readall_error
    content$ = _READFILE$(fileName$)
    ON ERROR GOTO _LASTHANDLER
    IncludeProvider_Filesystem_ReadAll$ = content$
    EXIT FUNCTION
    
filesystem_readall_error:
    ON ERROR GOTO _LASTHANDLER
    IncludeProvider_Filesystem_ReadAll$ = ""
END FUNCTION

' ============================================
' Memory Provider Implementation (for testing)
' ============================================

' In-memory file storage for testing
TYPE MemoryFile
    fileName AS STRING
    content AS STRING
END TYPE

DIM SHARED memoryFiles(1000) AS MemoryFile
DIM SHARED memoryFileCount AS LONG

SUB IncludeProvider_Memory_Clear
    DIM i AS LONG
    FOR i = 0 TO 1000
        memoryFiles(i).fileName = ""
        memoryFiles(i).content = ""
    NEXT
    memoryFileCount = 0
END SUB

SUB IncludeProvider_Memory_AddFile (fileName$, content$)
    DIM i AS LONG
    ' Check if file already exists, replace it
    FOR i = 0 TO memoryFileCount - 1
        IF memoryFiles(i).fileName = fileName$ THEN
            memoryFiles(i).content = content$
            EXIT SUB
        END IF
    NEXT
    
    ' Add new file
    IF memoryFileCount < 1000 THEN
        memoryFiles(memoryFileCount).fileName = fileName$
        memoryFiles(memoryFileCount).content = content$
        memoryFileCount = memoryFileCount + 1
    END IF
END SUB

FUNCTION IncludeProvider_Memory_FileExists& (fileName$)
    DIM i AS LONG
    FOR i = 0 TO memoryFileCount - 1
        IF memoryFiles(i).fileName = fileName$ THEN
            IncludeProvider_Memory_FileExists& = -1
            EXIT FUNCTION
        END IF
    NEXT
    IncludeProvider_Memory_FileExists& = 0
END FUNCTION

FUNCTION IncludeProvider_Memory_Open& (fileName$, level AS LONG)
    DIM i AS LONG, found AS LONG
    found = 0
    
    FOR i = 0 TO memoryFileCount - 1
        IF memoryFiles(i).fileName = fileName$ THEN
            found = -1
            EXIT FOR
        END IF
    NEXT
    
    IF found = 0 THEN
        IncludeProvider_Memory_Open& = 0
        EXIT FUNCTION
    END IF
    
    includeProviderStates(level).content = memoryFiles(i).content
    includeProviderStates(level).fileName = fileName$
    includeProviderStates(level).providerType = INCLUDE_PROVIDER_MEMORY
    includeProviderStates(level).isOpen = -1
    includeProviderStates(level).currentLine = 0
    
    IncludeProvider_Memory_Open& = -1
END FUNCTION

FUNCTION IncludeProvider_Memory_ReadLine$ (level AS LONG)
    DIM content$ AS STRING, line$ AS STRING, pos AS LONG, newline AS LONG
    DIM crlf AS LONG, lf AS LONG
    
    IF includeProviderStates(level).isOpen = 0 THEN
        IncludeProvider_Memory_ReadLine$ = ""
        EXIT FUNCTION
    END IF
    
    content$ = includeProviderStates(level).content
    pos = includeProviderStates(level).currentLine
    
    ' Find next newline
    crlf = INSTR(pos + 1, content$, CHR$(13) + CHR$(10))
    lf = INSTR(pos + 1, content$, CHR$(10))
    
    IF crlf > 0 AND (lf = 0 OR crlf < lf) THEN
        newline = crlf
        line$ = MID$(content$, pos + 1, newline - pos - 1)
        includeProviderStates(level).currentLine = newline + 1
    ELSEIF lf > 0 THEN
        newline = lf
        line$ = MID$(content$, pos + 1, newline - pos - 1)
        includeProviderStates(level).currentLine = newline + 1
    ELSE
        ' Last line or EOF
        IF pos < LEN(content$) THEN
            line$ = MID$(content$, pos + 1)
            includeProviderStates(level).currentLine = LEN(content$) + 1
        ELSE
            line$ = ""
        END IF
    END IF
    
    IncludeProvider_Memory_ReadLine$ = line$
END FUNCTION

FUNCTION IncludeProvider_Memory_EOF& (level AS LONG)
    DIM content$ AS STRING, pos AS LONG
    content$ = includeProviderStates(level).content
    pos = includeProviderStates(level).currentLine
    
    IF includeProviderStates(level).isOpen = 0 THEN
        IncludeProvider_Memory_EOF& = -1
        EXIT FUNCTION
    END IF
    
    IF pos >= LEN(content$) THEN
        IncludeProvider_Memory_EOF& = -1
    ELSE
        IncludeProvider_Memory_EOF& = 0
    END IF
END FUNCTION

SUB IncludeProvider_Memory_Close (level AS LONG)
    includeProviderStates(level).isOpen = 0
    includeProviderStates(level).content = ""
    includeProviderStates(level).currentLine = 0
    includeProviderStates(level).fileName = ""
END SUB

FUNCTION IncludeProvider_Memory_ResolvePath$ (fileName$, basePath$)
    ' For memory provider, just return fileName as-is
    ' Path resolution is handled by the test setup
    IncludeProvider_Memory_ResolvePath$ = fileName$
END FUNCTION

FUNCTION IncludeProvider_Memory_ReadAll$ (fileName$)
    DIM i AS LONG
    FOR i = 0 TO memoryFileCount - 1
        IF memoryFiles(i).fileName = fileName$ THEN
            IncludeProvider_Memory_ReadAll$ = memoryFiles(i).content
            EXIT FUNCTION
        END IF
    NEXT
    IncludeProvider_Memory_ReadAll$ = ""
END FUNCTION

' ============================================
' Test Provider Implementation (for unit testing)
' ============================================

' Test provider uses same implementation as memory provider
' but can be extended with additional test-specific functionality

FUNCTION IncludeProvider_Test_FileExists& (fileName$)
    IncludeProvider_Test_FileExists& = IncludeProvider_Memory_FileExists&(fileName$)
END FUNCTION

FUNCTION IncludeProvider_Test_Open& (fileName$, level AS LONG)
    IncludeProvider_Test_Open& = IncludeProvider_Memory_Open&(fileName$, level)
END FUNCTION

FUNCTION IncludeProvider_Test_ReadLine$ (level AS LONG)
    IncludeProvider_Test_ReadLine$ = IncludeProvider_Memory_ReadLine$(level)
END FUNCTION

FUNCTION IncludeProvider_Test_EOF& (level AS LONG)
    IncludeProvider_Test_EOF& = IncludeProvider_Memory_EOF&(level)
END FUNCTION

SUB IncludeProvider_Test_Close (level AS LONG)
    IncludeProvider_Memory_Close level
END SUB

FUNCTION IncludeProvider_Test_ResolvePath$ (fileName$, basePath$)
    IncludeProvider_Test_ResolvePath$ = IncludeProvider_Memory_ResolvePath$(fileName$, basePath$)
END FUNCTION

FUNCTION IncludeProvider_Test_ReadAll$ (fileName$)
    IncludeProvider_Test_ReadAll$ = IncludeProvider_Memory_ReadAll$(fileName$)
END FUNCTION
