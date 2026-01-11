'
' Include Provider Declarations
'
' Declarations-only file for include_provider.bi
' Contains CONST, TYPE, DIM SHARED, and DECLARE statements only
' Must be included before any SUB/FUNCTION definitions
'
$INCLUDEONCE

' Include provider type constants
CONST INCLUDE_PROVIDER_FILESYSTEM = 0
CONST INCLUDE_PROVIDER_MEMORY = 1
CONST INCLUDE_PROVIDER_TEST = 2

' Global variable to track current provider type
DIM SHARED includeProviderType AS LONG

' Global variable to track whether includes should be skipped (for unit testing)
DIM SHARED skipIncludes AS LONG

' Include provider state structure
TYPE IncludeProviderState
    providerType AS LONG      ' Type of provider (FILESYSTEM, MEMORY, TEST)
    fileHandle AS LONG        ' File handle for filesystem provider (file #199+level+1)
    content AS STRING         ' Content for memory/test providers
    currentLine AS LONG       ' Current line position in content (for memory/test providers)
    fileName AS STRING        ' Full file name/path being included
    isOpen AS LONG            ' Whether this state is active (-1=open, 0=closed)
END TYPE

' Array to track include provider states (one per include level)
DIM SHARED includeProviderStates(100) AS IncludeProviderState

' In-memory file storage for testing
TYPE MemoryFile
    fileName AS STRING
    content AS STRING
END TYPE

DIM SHARED memoryFiles(1000) AS MemoryFile
DIM SHARED memoryFileCount AS LONG

' Test provider call tracking type
TYPE TestProviderCall
    callType AS STRING * 20    ' "FileExists", "Open", "ReadLine", etc.
    fileName AS STRING
    callOrder AS LONG          ' Sequence number (0-based) indicating call order for deterministic test verification
END TYPE

DIM SHARED testProviderCalls(1000) AS TestProviderCall
DIM SHARED testProviderCallCount AS LONG
DIM SHARED testProviderErrorFile$
DIM SHARED testProviderErrorType AS LONG  ' 0 = none, 1 = file not found, 2 = read error

' Path mapping for test scenarios
TYPE TestProviderPathMap
    fromPath AS STRING
    toPath AS STRING
END TYPE

DIM SHARED testProviderPathMaps(100) AS TestProviderPathMap
DIM SHARED testProviderPathMapCount AS LONG

' Runtime function stub registry
TYPE RuntimeStub
    functionName AS STRING * 50
    returnValue AS STRING
    callCount AS LONG
END TYPE

DIM SHARED runtimeStubs(100) AS RuntimeStub
DIM SHARED runtimeStubCount AS LONG

' Forward declarations for include provider functions
DECLARE SUB IncludeProvider_Init
DECLARE SUB IncludeProvider_SetType (providerType AS LONG)
DECLARE FUNCTION IncludeProvider_GetType& ()
DECLARE FUNCTION IncludeProvider_FileExists& (fileName$)
DECLARE FUNCTION IncludeProvider_Open& (fileName$, level AS LONG)
DECLARE FUNCTION IncludeProvider_ReadLine$ (level AS LONG)
DECLARE FUNCTION IncludeProvider_EOF& (level AS LONG)
DECLARE SUB IncludeProvider_Close (level AS LONG)
DECLARE FUNCTION IncludeProvider_ResolvePath$ (fileName$, basePath$)
DECLARE FUNCTION IncludeProvider_ReadAll$ (fileName$)
DECLARE SUB IncludeProvider_SkipIncludes (enable AS LONG)
DECLARE FUNCTION IncludeProvider_ShouldSkipIncludes& ()
