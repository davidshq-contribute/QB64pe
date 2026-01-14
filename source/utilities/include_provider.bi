'
' Include Provider Abstraction Interface
'
' This module provides an abstraction layer for file include operations,
' enabling testability by allowing different implementations (file system,
' in-memory, etc.) to be swapped in.
'
' OVERVIEW:
' The include provider system abstracts file I/O operations for $INCLUDE
' directive processing. This allows the compiler to use different backends:
'   - Filesystem provider: Normal file I/O (default, production use)
'   - Memory provider: In-memory file content (for testing)
'   - Test provider: Extended memory provider with call tracking and mocking
'
' USAGE:
' The compiler uses this system when processing $INCLUDE and $INCLUDEONCE
' directives. The provider is set via IncludeProvider_SetType() and all
' include operations go through the abstracted interface functions.
'
' STATE MANAGEMENT:
' Each include level (nested includes) has its own state tracked in
' includeProviderStates(). The level parameter (0-based) indicates which
' include file is being accessed. Level 0 is the main source file.
'
' SKIP INCLUDES MODE:
' For unit testing, skipIncludes can be enabled to ignore all $INCLUDE
' directives. This allows testing individual functions without processing
' their include dependencies.
'

' Include provider type constants (must be declared before any SUB/FUNCTION)
CONST INCLUDE_PROVIDER_FILESYSTEM = 0
CONST INCLUDE_PROVIDER_MEMORY = 1
CONST INCLUDE_PROVIDER_TEST = 2

' Global variable to track current provider type
' All DIM SHARED declarations moved to qb64pe.bas

' Include provider state structure
' This tracks the state of an open include file at each include level
' Each nested include has its own state entry (indexed by include level)
' TYPE definitions moved to qb64pe.bas

' Array to track include provider states (one per include level)
' All DIM SHARED declarations moved to qb64pe.bas

' ============================================
' Memory Provider Type Definitions
' ============================================
' These must be declared before any SUB/FUNCTION declarations

' TYPE definitions moved to qb64pe.bas

' All DIM SHARED declarations moved to qb64pe.bas

' TYPE definitions moved to qb64pe.bas

' All DIM SHARED declarations moved to qb64pe.bas  ' 0 = none, 1 = file not found, 2 = read error

' TYPE definitions moved to qb64pe.bas

' All DIM SHARED declarations moved to qb64pe.bas

' TYPE definitions moved to qb64pe.bas

' All DIM SHARED declarations moved to qb64pe.bas

' Initialize include provider system
' Sets default provider to filesystem
SUB IncludeProvider_Init
    includeProviderType = INCLUDE_PROVIDER_FILESYSTEM
    skipIncludes = 0
    DIM i AS LONG
    FOR i = 0 TO 100
        includeProviderStates(i).isOpen = 0
        includeProviderStates(i).currentLine = 0
        includeProviderStates(i).fileHandle = 0
        includeProviderStates(i).content = ""
        includeProviderStates(i).fileName = ""
        includeProviderStates(i).providerType = INCLUDE_PROVIDER_FILESYSTEM
    NEXT
END SUB

' Set the include provider type
' providerType: INCLUDE_PROVIDER_FILESYSTEM, INCLUDE_PROVIDER_MEMORY, or INCLUDE_PROVIDER_TEST
SUB IncludeProvider_SetType (providerType AS LONG)
    includeProviderType = providerType
END SUB

' Get the current include provider type
FUNCTION IncludeProvider_GetType&
    IncludeProvider_GetType& = includeProviderType
END FUNCTION

' Check if a file exists using the current provider
' fileName$: File name/path to check
' Returns: -1 if exists, 0 if not
FUNCTION IncludeProvider_FileExists& (fileName$)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_FileExists& = IncludeProvider_Filesystem_FileExists&(fileName$)
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_FileExists& = IncludeProvider_Memory_FileExists&(fileName$)
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_FileExists& = IncludeProvider_Test_FileExists&(fileName$)
        CASE ELSE
            IncludeProvider_FileExists& = 0
    END SELECT
END FUNCTION

' Open a file for reading using the current provider
' fileName$: File name/path to open
' level: Include level (0-based)
' Returns: -1 on success, 0 on failure
FUNCTION IncludeProvider_Open& (fileName$, level AS LONG)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_Open& = IncludeProvider_Filesystem_Open&(fileName$, level)
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_Open& = IncludeProvider_Memory_Open&(fileName$, level)
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_Open& = IncludeProvider_Test_Open&(fileName$, level)
        CASE ELSE
            IncludeProvider_Open& = 0
    END SELECT
END FUNCTION

' Read a line from the current include file
' level: Include level (0-based)
' Returns: Line content, or empty string if EOF
FUNCTION IncludeProvider_ReadLine$ (level AS LONG)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_ReadLine$ = IncludeProvider_Filesystem_ReadLine$(level)
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_ReadLine$ = IncludeProvider_Memory_ReadLine$(level)
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_ReadLine$ = IncludeProvider_Test_ReadLine$(level)
        CASE ELSE
            IncludeProvider_ReadLine$ = ""
    END SELECT
END FUNCTION

' Check if end of file reached
' level: Include level (0-based)
' Returns: -1 if EOF, 0 if not
FUNCTION IncludeProvider_EOF& (level AS LONG)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_EOF& = IncludeProvider_Filesystem_EOF&(level)
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_EOF& = IncludeProvider_Memory_EOF&(level)
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_EOF& = IncludeProvider_Test_EOF&(level)
        CASE ELSE
            IncludeProvider_EOF& = -1
    END SELECT
END FUNCTION

' Close an include file
' level: Include level (0-based)
SUB IncludeProvider_Close (level AS LONG)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_Filesystem_Close level
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_Memory_Close level
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_Test_Close level
    END SELECT
END SUB

' Get full path for a file using the current provider
' fileName$: File name/path
' basePath$: Base path for relative resolution
' Returns: Full resolved path
FUNCTION IncludeProvider_ResolvePath$ (fileName$, basePath$)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_ResolvePath$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_ResolvePath$ = IncludeProvider_Memory_ResolvePath$(fileName$, basePath$)
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_ResolvePath$ = IncludeProvider_Test_ResolvePath$(fileName$, basePath$)
        CASE ELSE
            IncludeProvider_ResolvePath$ = fileName$
    END SELECT
END FUNCTION

' Read entire file content (for $INCLUDEONCE checking)
' fileName$: File name/path
' Returns: File content as string
FUNCTION IncludeProvider_ReadAll$ (fileName$)
    SELECT CASE includeProviderType
        CASE INCLUDE_PROVIDER_FILESYSTEM
            IncludeProvider_ReadAll$ = IncludeProvider_Filesystem_ReadAll$(fileName$)
        CASE INCLUDE_PROVIDER_MEMORY
            IncludeProvider_ReadAll$ = IncludeProvider_Memory_ReadAll$(fileName$)
        CASE INCLUDE_PROVIDER_TEST
            IncludeProvider_ReadAll$ = IncludeProvider_Test_ReadAll$(fileName$)
        CASE ELSE
            IncludeProvider_ReadAll$ = ""
    END SELECT
END FUNCTION

' Enable skip includes mode (for unit testing)
' When enabled, $INCLUDE directives will be ignored
SUB IncludeProvider_SkipIncludes (enable AS LONG)
    skipIncludes = enable
END SUB

' Check if skip includes mode is enabled
' Returns: -1 if enabled, 0 if disabled
FUNCTION IncludeProvider_ShouldSkipIncludes&
    IncludeProvider_ShouldSkipIncludes& = skipIncludes
END FUNCTION

' ============================================
' Test Provider Extended Functions
' ============================================
' Note: TestProviderCall TYPE and DIM SHARED declarations moved to top of file

' Note: Function implementations are in include_provider.bas
' The .bi file contains TYPE definitions and DIM SHARED declarations to ensure proper declaration order