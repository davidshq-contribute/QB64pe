'
' Include Provider Implementation
'
' Provides concrete implementations of include providers:

$INCLUDEONCE
' - Filesystem provider (default, uses actual file I/O)
' - Memory provider (for testing, uses in-memory content)
' - Test provider (for unit testing, allows mocking)
'

' ============================================
' Memory Provider Implementation (for testing)
' ============================================
' Note: TYPE MemoryFile and DIM SHARED declarations are in include_provider.bi

' ============================================
' Filesystem Provider Implementation
' ============================================

FUNCTION IncludeProvider_Filesystem_FileExists& (fileName$)
    IncludeProvider_Filesystem_FileExists& = _FILEEXISTS(fileName$)
END FUNCTION

' Open a file for reading using filesystem I/O
' fileName$: Full path to the file to open
' level: Include nesting level (0-based, 0=main file, 1=first include, etc.)
' Returns: -1 on success, 0 on failure
' Note: Uses file handle #(199 + level + 1) to ensure unique handles per level
FUNCTION IncludeProvider_Filesystem_Open& (fileName$, level AS LONG)
    DIM fh AS LONG
    fh = 199 + level + 1

    ' Close handle if already open (shouldn't happen, but be safe)
    ' Check if handle is open by checking the includeProviderStates instead of using EOF()
    IF level >= 0 AND level <= UBOUND(includeProviderStates) THEN
        IF includeProviderStates(level).isOpen AND includeProviderStates(level).fileHandle = fh THEN
            CLOSE #fh
        END IF
    END IF

    ' Attempt to open the file
    ' Check if file exists first
    IF _FILEEXISTS(fileName$) = 0 THEN
        ' Mark as closed on error
        includeProviderStates(level).isOpen = 0
        IncludeProvider_Filesystem_Open& = 0
        EXIT FUNCTION
    END IF

    OPEN fileName$ FOR BINARY AS #fh

    ' Update state for this include level
    includeProviderStates(level).fileHandle = fh
    includeProviderStates(level).fileName = fileName$
    includeProviderStates(level).providerType = INCLUDE_PROVIDER_FILESYSTEM
    includeProviderStates(level).isOpen = -1
    includeProviderStates(level).currentLine = 0

    IncludeProvider_Filesystem_Open& = -1
END FUNCTION

FUNCTION IncludeProvider_Filesystem_ReadLine$ (level AS LONG)
    DIM fh AS LONG, line$
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

' Resolve a relative file path to an absolute path
' fileName$: File name/path (may be relative or absolute)
' basePath$: Base directory path (directory of the file that's including this one)
' Returns: Full resolved path
' 
' PATH RESOLUTION LOGIC:
' 1. If fileName$ is absolute (starts with / or drive letter), return as-is
' 2. Remove leading .\ or ./ if present
' 3. If basePath$ is empty, return fileName$ as-is
' 4. Otherwise, combine basePath$ + separator + fileName$
' 
' This ensures that includes like $INCLUDE:'../file.bas' resolve correctly
' relative to the directory of the file containing the $INCLUDE directive.
FUNCTION IncludeProvider_Filesystem_ResolvePath$ (fileName$, basePath$)
    ' If absolute path (Unix / or Windows C:), return as-is
    IF LEFT$(fileName$, 1) = "/" OR (LEN(fileName$) >= 2 AND MID$(fileName$, 2, 1) = ":") THEN
        IncludeProvider_Filesystem_ResolvePath$ = fileName$
        EXIT FUNCTION
    END IF
    
    ' Remove leading .\ or ./ (current directory reference)
    IF LEFT$(fileName$, 2) = ".\" OR LEFT$(fileName$, 2) = "./" THEN
        fileName$ = MID$(fileName$, 3)
    END IF
    
    ' If basePath is empty, return fileName as-is
    IF basePath$ = "" THEN
        IncludeProvider_Filesystem_ResolvePath$ = fileName$
        EXIT FUNCTION
    END IF
    
    ' Determine path separator based on basePath format
    DIM pathSep$
    IF INSTR(basePath$, "\") > 0 THEN
        pathSep$ = "\"  ' Windows-style
    ELSE
        pathSep$ = "/"  ' Unix-style
    END IF
    
    ' Combine paths (add separator if basePath doesn't end with one)
    IF RIGHT$(basePath$, 1) <> "\" AND RIGHT$(basePath$, 1) <> "/" THEN
        IncludeProvider_Filesystem_ResolvePath$ = basePath$ + pathSep$ + fileName$
    ELSE
        IncludeProvider_Filesystem_ResolvePath$ = basePath$ + fileName$
    END IF
END FUNCTION

FUNCTION IncludeProvider_Filesystem_ReadAll$ (fileName$)
    ' Read entire file content for $INCLUDEONCE checking
    DIM content$
    IF _FILEEXISTS(fileName$) = 0 THEN
        IncludeProvider_Filesystem_ReadAll$ = ""
        EXIT FUNCTION
    END IF
    content$ = _READFILE$(fileName$)
    IncludeProvider_Filesystem_ReadAll$ = content$
END FUNCTION

' ============================================
' Memory Provider Implementation (for testing)
' ============================================
' Note: TYPE MemoryFile and DIM SHARED declarations moved to top of file

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
    DIM content$, line$, currentPos AS LONG, newline AS LONG
    DIM crlf AS LONG, lf AS LONG
    
    IF includeProviderStates(level).isOpen = 0 THEN
        IncludeProvider_Memory_ReadLine$ = ""
        EXIT FUNCTION
    END IF
    
    content$ = includeProviderStates(level).content
    currentPos = includeProviderStates(level).currentLine
    
    ' Find next newline
    crlf = INSTR(currentPos + 1, content$, CHR$(13) + CHR$(10))
    lf = INSTR(currentPos + 1, content$, CHR$(10))
    
    IF crlf > 0 AND (lf = 0 OR crlf < lf) THEN
        newline = crlf
        line$ = MID$(content$, currentPos + 1, newline - currentPos - 1)
        includeProviderStates(level).currentLine = newline + 1
    ELSEIF lf > 0 THEN
        newline = lf
        line$ = MID$(content$, currentPos + 1, newline - currentPos - 1)
        includeProviderStates(level).currentLine = newline + 1
    ELSE
        ' Last line or EOF
        IF currentPos < LEN(content$) THEN
            line$ = MID$(content$, currentPos + 1)
            includeProviderStates(level).currentLine = LEN(content$) + 1
        ELSE
            line$ = ""
        END IF
    END IF
    
    IncludeProvider_Memory_ReadLine$ = line$
END FUNCTION

FUNCTION IncludeProvider_Memory_EOF& (level AS LONG)
    DIM content$, currentPos AS LONG
    content$ = includeProviderStates(level).content
    currentPos = includeProviderStates(level).currentLine
    
    IF includeProviderStates(level).isOpen = 0 THEN
        IncludeProvider_Memory_EOF& = -1
        EXIT FUNCTION
    END IF
    
    IF currentPos >= LEN(content$) THEN
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

' Test provider extends memory provider with additional test-specific functionality
' including call tracking, error injection, and path mapping

' Note: TestProviderCall TYPE and all DIM SHARED declarations moved to include_provider.bi

' Clear test provider state
SUB IncludeProvider_Test_Clear
    IncludeProvider_Memory_Clear
    testProviderCallCount = 0
    testProviderErrorFile$ = ""
    testProviderErrorType = 0
    testProviderPathMapCount = 0
    DIM i AS LONG
    FOR i = 0 TO 100
        testProviderPathMaps(i).fromPath = ""
        testProviderPathMaps(i).toPath = ""
    NEXT
END SUB

' Track a provider call
' Uses sequence number instead of TIMER to avoid midnight reset issues
SUB IncludeProvider_Test_TrackCall (callType$, fileName$)
    IF testProviderCallCount < 1000 THEN
        testProviderCalls(testProviderCallCount).callType = callType$
        testProviderCalls(testProviderCallCount).fileName = fileName$
        ' Use sequence number (0-based) instead of TIMER to ensure deterministic ordering
        ' TIMER resets at midnight which could cause issues for tests running across midnight
        ' callOrder provides deterministic ordering for test verification
        testProviderCalls(testProviderCallCount).callOrder = testProviderCallCount
        testProviderCallCount = testProviderCallCount + 1
    END IF
END SUB

' Get call history
FUNCTION IncludeProvider_Test_GetCallCount&
    IncludeProvider_Test_GetCallCount& = testProviderCallCount
END FUNCTION

' Get a specific call from history
' Returns call information including callOrder for deterministic test verification
' Example usage for verifying call ordering:
'   DIM call AS TestProviderCall
'   IncludeProvider_Test_GetCall$ 0, call
'   IF call.callOrder = 0 THEN ' First call
'   IncludeProvider_Test_GetCall$ 1, call
'   IF call.callOrder = 1 THEN ' Second call, etc.
FUNCTION IncludeProvider_Test_GetCall$ (index AS LONG, callType AS TestProviderCall)
    IF index >= 0 AND index < testProviderCallCount THEN
        callType.callType = testProviderCalls(index).callType
        callType.fileName = testProviderCalls(index).fileName
        callType.callOrder = testProviderCalls(index).callOrder
        IncludeProvider_Test_GetCall$ = "OK"
    ELSE
        IncludeProvider_Test_GetCall$ = "ERROR"
    END IF
END FUNCTION

' Set error injection for a specific file
SUB IncludeProvider_Test_SetError (fileName$, errorType AS LONG)
    testProviderErrorFile$ = fileName$
    testProviderErrorType = errorType
END SUB

' Clear error injection
SUB IncludeProvider_Test_ClearError
    testProviderErrorFile$ = ""
    testProviderErrorType = 0
END SUB

' Add path mapping (useful for testing include resolution)
SUB IncludeProvider_Test_AddPathMap (fromPath$, toPath$)
    IF testProviderPathMapCount < 100 THEN
        testProviderPathMaps(testProviderPathMapCount).fromPath = fromPath$
        testProviderPathMaps(testProviderPathMapCount).toPath = toPath$
        testProviderPathMapCount = testProviderPathMapCount + 1
    END IF
END SUB

' Resolve path using mappings
FUNCTION IncludeProvider_Test_ResolveMappedPath$ (fileName$)
    DIM i AS LONG
    FOR i = 0 TO testProviderPathMapCount - 1
        IF testProviderPathMaps(i).fromPath = fileName$ THEN
            IncludeProvider_Test_ResolveMappedPath$ = testProviderPathMaps(i).toPath
            EXIT FUNCTION
        END IF
    NEXT
    IncludeProvider_Test_ResolveMappedPath$ = fileName$
END FUNCTION

FUNCTION IncludeProvider_Test_FileExists& (fileName$)
    IncludeProvider_Test_TrackCall "FileExists", fileName$
    
    ' Check for error injection
    IF testProviderErrorType = 1 AND testProviderErrorFile$ = fileName$ THEN
        IncludeProvider_Test_FileExists& = 0
        EXIT FUNCTION
    END IF
    
    IncludeProvider_Test_FileExists& = IncludeProvider_Memory_FileExists&(fileName$)
END FUNCTION

FUNCTION IncludeProvider_Test_Open& (fileName$, level AS LONG)
    IncludeProvider_Test_TrackCall "Open", fileName$
    
    ' Check for error injection
    IF testProviderErrorType > 0 AND testProviderErrorFile$ = fileName$ THEN
        IncludeProvider_Test_Open& = 0
        EXIT FUNCTION
    END IF
    
    IncludeProvider_Test_Open& = IncludeProvider_Memory_Open&(fileName$, level)
END FUNCTION

FUNCTION IncludeProvider_Test_ReadLine$ (level AS LONG)
    IncludeProvider_Test_TrackCall "ReadLine", includeProviderStates(level).fileName
    
    ' Check for error injection
    IF testProviderErrorType = 2 AND testProviderErrorFile$ = includeProviderStates(level).fileName THEN
        IncludeProvider_Test_ReadLine$ = ""
        EXIT FUNCTION
    END IF
    
    IncludeProvider_Test_ReadLine$ = IncludeProvider_Memory_ReadLine$(level)
END FUNCTION

FUNCTION IncludeProvider_Test_EOF& (level AS LONG)
    IncludeProvider_Test_EOF& = IncludeProvider_Memory_EOF&(level)
END FUNCTION

SUB IncludeProvider_Test_Close (level AS LONG)
    IncludeProvider_Test_TrackCall "Close", includeProviderStates(level).fileName
    IncludeProvider_Memory_Close level
END SUB

FUNCTION IncludeProvider_Test_ResolvePath$ (fileName$, basePath$)
    ' Apply path mappings first
    DIM mappedPath$
    mappedPath$ = IncludeProvider_Test_ResolveMappedPath$(fileName$)
    
    ' Then use memory provider resolution
    IncludeProvider_Test_ResolvePath$ = IncludeProvider_Memory_ResolvePath$(mappedPath$, basePath$)
END FUNCTION

FUNCTION IncludeProvider_Test_ReadAll$ (fileName$)
    IncludeProvider_Test_TrackCall "ReadAll", fileName$
    IncludeProvider_Test_ReadAll$ = IncludeProvider_Memory_ReadAll$(fileName$)
END FUNCTION

' Stub utilities for runtime functions
' These can be used to mock runtime behavior in tests

' Note: RuntimeStub TYPE and DIM SHARED declarations moved to include_provider.bi

' Register a runtime function stub
SUB IncludeProvider_Test_RegisterStub (functionName$, returnValue$)
    DIM i AS LONG
    ' Check if stub already exists
    FOR i = 0 TO runtimeStubCount - 1
        IF runtimeStubs(i).functionName = functionName$ THEN
            runtimeStubs(i).returnValue = returnValue$
            EXIT SUB
        END IF
    NEXT
    
    ' Add new stub
    IF runtimeStubCount < 100 THEN
        runtimeStubs(runtimeStubCount).functionName = functionName$
        runtimeStubs(runtimeStubCount).returnValue = returnValue$
        runtimeStubs(runtimeStubCount).callCount = 0
        runtimeStubCount = runtimeStubCount + 1
    END IF
END SUB

' Get stub return value
FUNCTION IncludeProvider_Test_GetStubValue$ (functionName$)
    DIM i AS LONG
    FOR i = 0 TO runtimeStubCount - 1
        IF runtimeStubs(i).functionName = functionName$ THEN
            runtimeStubs(i).callCount = runtimeStubs(i).callCount + 1
            IncludeProvider_Test_GetStubValue$ = runtimeStubs(i).returnValue
            EXIT FUNCTION
        END IF
    NEXT
    IncludeProvider_Test_GetStubValue$ = ""
END FUNCTION

' Get stub call count
FUNCTION IncludeProvider_Test_GetStubCallCount& (functionName$)
    DIM i AS LONG
    FOR i = 0 TO runtimeStubCount - 1
        IF runtimeStubs(i).functionName = functionName$ THEN
            IncludeProvider_Test_GetStubCallCount& = runtimeStubs(i).callCount
            EXIT FUNCTION
        END IF
    NEXT
    IncludeProvider_Test_GetStubCallCount& = 0
END FUNCTION

' Clear all stubs
SUB IncludeProvider_Test_ClearStubs
    DIM i AS LONG
    FOR i = 0 TO runtimeStubCount - 1
        runtimeStubs(i).functionName = ""
        runtimeStubs(i).returnValue = ""
        runtimeStubs(i).callCount = 0
    NEXT
    runtimeStubCount = 0
END SUB
