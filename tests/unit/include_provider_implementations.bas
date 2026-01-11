'
' Include Provider Interface Implementations
'
' This file contains only the SUB/FUNCTION implementations from include_provider.bi
' The type definitions and declarations are in include_provider_declarations.bi
' This allows us to include implementations in Phase 3 without type conflicts
'

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
