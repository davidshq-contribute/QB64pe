'
' Unit Tests for Include Provider (include_provider.bas)
'
' Tests filesystem, memory, and test provider implementations.
' Uses component test harness for isolated testing.
'
' Note: test_framework_declarations.bi, test_framework_implementations.bas, test_state_manager_declarations.bi, test_state_manager_implementations.bas, and include_provider.bi are included by test_runner.bas
'$INCLUDE:'../../../source/utilities/include_provider.bas'

SUB Test_IncludeProviderInit
    Test_Start "IncludeProvider_Init - Provider initialization"
    
    DIM result AS LONG
    
    IncludeProvider_Init
    result = Test_AssertEqual&(INCLUDE_PROVIDER_FILESYSTEM, includeProviderType, "Should initialize to filesystem provider")
    
    IF result THEN
        result = Test_AssertEqual&(0, skipIncludes, "skipIncludes should be 0")
    END IF
    
    Test_End result
END SUB

SUB Test_FilesystemProviderFileExists
    Test_Start "IncludeProvider_Filesystem_FileExists& - File existence check"
    
    DIM result AS LONG
    DIM testFile$, exists AS LONG
    
    ' Create a test file
    testFile$ = "test_include_file.tmp"
    _WRITEFILE testFile$, "test content"
    
    ' Test existing file
    exists = IncludeProvider_Filesystem_FileExists&(testFile$)
    result = Test_Assert&(exists <> 0, "Should detect existing file")
    
    ' Test non-existent file
    IF result THEN
        exists = IncludeProvider_Filesystem_FileExists&("nonexistent_file_12345.tmp")
        result = Test_Assert&(exists = 0, "Should not detect non-existent file")
    END IF
    
    ' Cleanup
    IF _FILEEXISTS(testFile$) THEN KILL testFile$
    
    Test_End result
END SUB

SUB Test_FilesystemProviderOpenReadClose
    Test_Start "IncludeProvider_Filesystem - Open, ReadLine, EOF, Close"
    
    DIM result AS LONG
    DIM testFile$, line$, eofResult AS LONG
    DIM level AS LONG
    level = 0
    
    ' Create a test file with multiple lines
    testFile$ = "test_include_read.tmp"
    _WRITEFILE testFile$, "Line 1" + CHR$(10) + "Line 2" + CHR$(10) + "Line 3"
    
    ' Test open
    result = Test_Assert&(IncludeProvider_Filesystem_Open&(testFile$, level) <> 0, "Should open file successfully")
    
    ' Test read first line
    IF result THEN
        line$ = IncludeProvider_Filesystem_ReadLine$(level)
        result = Test_AssertEqualString&("Line 1", line$, "Should read first line")
    END IF
    
    ' Test EOF (should be false)
    IF result THEN
        eofResult = IncludeProvider_Filesystem_EOF&(level)
        result = Test_Assert&(eofResult = 0, "Should not be EOF after first line")
    END IF
    
    ' Test read second line
    IF result THEN
        line$ = IncludeProvider_Filesystem_ReadLine$(level)
        result = Test_AssertEqualString&("Line 2", line$, "Should read second line")
    END IF
    
    ' Test read third line
    IF result THEN
        line$ = IncludeProvider_Filesystem_ReadLine$(level)
        result = Test_AssertEqualString&("Line 3", line$, "Should read third line")
    END IF
    
    ' Test EOF (should be true)
    IF result THEN
        eofResult = IncludeProvider_Filesystem_EOF&(level)
        result = Test_Assert&(eofResult <> 0, "Should be EOF after last line")
    END IF
    
    ' Test read beyond EOF
    IF result THEN
        line$ = IncludeProvider_Filesystem_ReadLine$(level)
        result = Test_AssertEqualString&("", line$, "Should return empty string at EOF")
    END IF
    
    ' Test close
    IF result THEN
        IncludeProvider_Filesystem_Close level
        ' Try to read after close (should return empty)
        line$ = IncludeProvider_Filesystem_ReadLine$(level)
        result = Test_AssertEqualString&("", line$, "Should return empty after close")
    END IF
    
    ' Cleanup
    IF _FILEEXISTS(testFile$) THEN KILL testFile$
    
    Test_End result
END SUB

SUB Test_FilesystemProviderResolvePath
    Test_Start "IncludeProvider_Filesystem_ResolvePath$ - Path resolution"
    
    DIM result AS LONG
    DIM fileName$, basePath$, resolved$
    
    ' Test absolute path (should return as-is)
    fileName$ = "/absolute/path/file.bas"
    basePath$ = "/base/path"
    resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
    result = Test_AssertEqualString&(fileName$, resolved$, "Should return absolute path as-is")
    
    ' Test Windows absolute path
    IF result THEN
        fileName$ = "C:\absolute\path\file.bas"
        basePath$ = "C:\base\path"
        resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        result = Test_AssertEqualString&(fileName$, resolved$, "Should return Windows absolute path as-is")
    END IF
    
    ' Test relative path with base
    IF result THEN
        fileName$ = "file.bas"
        basePath$ = "/base/path"
        resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        result = Test_AssertContains&(resolved$, "file.bas", "Should combine base path with file")
    END IF
    
    ' Test relative path with ./ prefix
    IF result THEN
        fileName$ = "./file.bas"
        basePath$ = "/base/path"
        resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        result = Test_AssertContains&(resolved$, "file.bas", "Should remove ./ prefix")
    END IF
    
    ' Test relative path with .\ prefix
    IF result THEN
        fileName$ = ".\file.bas"
        basePath$ = "C:\base\path"
        resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        result = Test_AssertContains&(resolved$, "file.bas", "Should remove .\ prefix")
    END IF
    
    ' Test with empty base path
    IF result THEN
        fileName$ = "file.bas"
        basePath$ = ""
        resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        result = Test_AssertEqualString&(fileName$, resolved$, "Should return fileName when basePath is empty")
    END IF
    
    ' Test with base path that already has trailing separator
    IF result THEN
        fileName$ = "file.bas"
        basePath$ = "/base/path/"
        resolved$ = IncludeProvider_Filesystem_ResolvePath$(fileName$, basePath$)
        result = Test_AssertContains&(resolved$, "file.bas", "Should handle base path with trailing separator")
    END IF
    
    Test_End result
END SUB

SUB Test_MemoryProviderBasic
    Test_Start "IncludeProvider_Memory - Basic operations"
    
    DIM result AS LONG
    DIM level AS LONG
    level = 0
    
    ' Clear memory provider
    IncludeProvider_Memory_Clear
    
    ' Add a test file
    IncludeProvider_Memory_AddFile "test.bas", "Line 1" + CHR$(10) + "Line 2"
    
    ' Test file exists
    result = Test_Assert&(IncludeProvider_Memory_FileExists&("test.bas") <> 0, "Should detect file in memory")
    
    ' Test non-existent file
    IF result THEN
        result = Test_Assert&(IncludeProvider_Memory_FileExists&("nonexistent.bas") = 0, "Should not detect non-existent file")
    END IF
    
    ' Test open
    IF result THEN
        result = Test_Assert&(IncludeProvider_Memory_Open&("test.bas", level) <> 0, "Should open memory file")
    END IF
    
    ' Test read line
    IF result THEN
        DIM line$
        line$ = IncludeProvider_Memory_ReadLine$(level)
        result = Test_AssertEqualString&("Line 1", line$, "Should read first line from memory")
    END IF
    
    ' Test EOF
    IF result THEN
        DIM eofResult AS LONG
        eofResult = IncludeProvider_Memory_EOF&(level)
        result = Test_Assert&(eofResult = 0, "Should not be EOF after first line")
    END IF
    
    ' Test read second line
    IF result THEN
        line$ = IncludeProvider_Memory_ReadLine$(level)
        result = Test_AssertEqualString&("Line 2", line$, "Should read second line from memory")
    END IF
    
    ' Test EOF at end
    IF result THEN
        eofResult = IncludeProvider_Memory_EOF&(level)
        result = Test_Assert&(eofResult <> 0, "Should be EOF after last line")
    END IF
    
    ' Test close
    IF result THEN
        IncludeProvider_Memory_Close level
        line$ = IncludeProvider_Memory_ReadLine$(level)
        result = Test_AssertEqualString&("", line$, "Should return empty after close")
    END IF
    
    Test_End result
END SUB

SUB Test_MemoryProviderReadAll
    Test_Start "IncludeProvider_Memory_ReadAll$ - Read entire file"
    
    DIM result AS LONG
    DIM testContent$, readContent$
    
    IncludeProvider_Memory_Clear
    testContent$ = "Line 1" + CHR$(10) + "Line 2" + CHR$(10) + "Line 3"
    IncludeProvider_Memory_AddFile "test.bas", testContent$
    
    readContent$ = IncludeProvider_Memory_ReadAll$("test.bas")
    result = Test_AssertEqualString&(testContent$, readContent$, "Should read entire file content")
    
    ' Test non-existent file
    IF result THEN
        readContent$ = IncludeProvider_Memory_ReadAll$("nonexistent.bas")
        result = Test_AssertEqualString&("", readContent$, "Should return empty for non-existent file")
    END IF
    
    Test_End result
END SUB

SUB Test_TestProviderCallTracking
    Test_Start "IncludeProvider_Test - Call tracking"
    
    DIM result AS LONG
    DIM callCount AS LONG
    DIM testCall AS TestProviderCall
    
    IncludeProvider_Test_Clear
    IncludeProvider_Memory_AddFile "test.bas", "content"
    
    ' Test FileExists call tracking
    DIM fileExistsResult AS LONG
    fileExistsResult = IncludeProvider_Test_FileExists&("test.bas")
    callCount = IncludeProvider_Test_GetCallCount&
    result = Test_AssertEqual&(1, callCount, "Should track FileExists call")
    
    ' Test Open call tracking
    IF result THEN
        DIM openResult AS LONG
        openResult = IncludeProvider_Test_Open&("test.bas", 0)
        callCount = IncludeProvider_Test_GetCallCount&
        result = Test_AssertEqual&(2, callCount, "Should track Open call")
    END IF
    
    ' Test ReadLine call tracking
    IF result THEN
        DIM readLineResult$
        readLineResult$ = IncludeProvider_Test_ReadLine$(0)
        callCount = IncludeProvider_Test_GetCallCount&
        result = Test_AssertEqual&(3, callCount, "Should track ReadLine call")
    END IF
    
    ' Test Close call tracking
    IF result THEN
        IncludeProvider_Test_Close 0
        callCount = IncludeProvider_Test_GetCallCount&
        result = Test_AssertEqual&(4, callCount, "Should track Close call")
    END IF
    
    ' Test GetCall
    IF result THEN
        DIM status$
        status$ = IncludeProvider_Test_GetCall$(0, testCall)
        result = Test_AssertEqualString&("OK", status$, "Should retrieve call successfully")
        IF result THEN result = Test_AssertEqualString&("FileExists", testCall.callType, "Should have correct call type")
    END IF
    
    Test_End result
END SUB

SUB Test_TestProviderErrorInjection
    Test_Start "IncludeProvider_Test - Error injection"
    
    DIM result AS LONG
    
    IncludeProvider_Test_Clear
    IncludeProvider_Memory_AddFile "test.bas", "content"
    
    ' Test error injection for FileExists
    IncludeProvider_Test_SetError "test.bas", 1
    result = Test_Assert&(IncludeProvider_Test_FileExists&("test.bas") = 0, "Should return false when error injected")
    
    ' Test error injection for Open
    IF result THEN
        IncludeProvider_Test_ClearError
        IncludeProvider_Test_SetError "test.bas", 1
        result = Test_Assert&(IncludeProvider_Test_Open&("test.bas", 0) = 0, "Should fail to open when error injected")
    END IF
    
    ' Test error injection for ReadLine
    IF result THEN
        IncludeProvider_Test_ClearError
        IncludeProvider_Memory_AddFile "test2.bas", "content"
        DIM openResult2 AS LONG
        openResult2 = IncludeProvider_Test_Open&("test2.bas", 0)
        IncludeProvider_Test_SetError "test2.bas", 2
        DIM line$
        line$ = IncludeProvider_Test_ReadLine$(0)
        result = Test_AssertEqualString&("", line$, "Should return empty when read error injected")
    END IF
    
    ' Test clear error
    IF result THEN
        IncludeProvider_Test_ClearError
        IncludeProvider_Test_Clear
        IncludeProvider_Memory_AddFile "test3.bas", "content"
        result = Test_Assert&(IncludeProvider_Test_FileExists&("test3.bas") <> 0, "Should work after error cleared")
    END IF
    
    Test_End result
END SUB

SUB Test_TestProviderPathMapping
    Test_Start "IncludeProvider_Test - Path mapping"
    
    DIM result AS LONG
    
    IncludeProvider_Test_Clear
    IncludeProvider_Memory_AddFile "mapped.bas", "content"
    
    ' Add path mapping
    IncludeProvider_Test_AddPathMap "original.bas", "mapped.bas"
    
    ' Test path resolution with mapping
    DIM resolved$
    resolved$ = IncludeProvider_Test_ResolvePath$("original.bas", "")
    result = Test_AssertEqualString&("mapped.bas", resolved$, "ResolvePath should use mapping")
    
    ' Test that mapped file can be accessed
    IF result THEN
        IncludeProvider_Test_AddPathMap "test.bas", "mapped.bas"
        result = Test_Assert&(IncludeProvider_Test_FileExists&("mapped.bas") <> 0, "Should access mapped file")
    END IF
    
    Test_End result
END SUB

SUB Test_ProviderSwitching
    Test_Start "IncludeProvider_SetType - Provider switching"
    
    DIM result AS LONG
    
    IncludeProvider_Init
    
    ' Switch to memory provider
    IncludeProvider_SetType INCLUDE_PROVIDER_MEMORY
    result = Test_AssertEqual&(INCLUDE_PROVIDER_MEMORY, includeProviderType, "Should switch to memory provider")
    
    ' Switch to test provider
    IF result THEN
        IncludeProvider_SetType INCLUDE_PROVIDER_TEST
        result = Test_AssertEqual&(INCLUDE_PROVIDER_TEST, includeProviderType, "Should switch to test provider")
    END IF
    
    ' Switch back to filesystem
    IF result THEN
        IncludeProvider_SetType INCLUDE_PROVIDER_FILESYSTEM
        result = Test_AssertEqual&(INCLUDE_PROVIDER_FILESYSTEM, includeProviderType, "Should switch back to filesystem provider")
    END IF
    
    Test_End result
END SUB

' Run all include provider tests
SUB RunIncludeProviderTests
    Test_IncludeProviderInit
    Test_FilesystemProviderFileExists
    Test_FilesystemProviderOpenReadClose
    Test_FilesystemProviderResolvePath
    Test_MemoryProviderBasic
    Test_MemoryProviderReadAll
    Test_TestProviderCallTracking
    Test_TestProviderErrorInjection
    Test_TestProviderPathMapping
    Test_ProviderSwitching
END SUB
