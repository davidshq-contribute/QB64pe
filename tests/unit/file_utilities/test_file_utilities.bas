'
' Unit Tests for File Utilities (file.bas)
'
' Tests file path operations, extension handling, and path normalization.
' Uses component test harness for isolated testing.
'
' Note: test_framework.bi and test_state_manager.bi are included by test_runner.bas
'$INCLUDE:'../../../source/global/constants.bas'
'$INCLUDE:'../../../source/utilities/file.bas'

' Initialize OS-specific variables for testing
' Note: os$ and pathsep are already declared in test_global_state_declarations.bi

SUB InitOSTestVars_FileUtilities
    ' Initialize os$ and pathsep$ similar to qb64pe.bas
    os$ = "WIN"
    IF INSTR(_OS$, "[LINUX]") THEN os$ = "LNX"
    
    pathsep$ = "\"
    IF os$ = "LNX" THEN pathsep$ = "/"
END SUB

SUB Test_GetFilePath
    Test_Start "getfilepath$ - Path extraction"
    
    InitOSTestVars_FileUtilities
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    ' Test with forward slash
    testPath$ = "path/to/file.bas"
    actual$ = getfilepath$(testPath$)
    expected$ = "path/to/"
    result = Test_AssertEqualString&(expected$, actual$, "Should extract path with forward slash")
    
    ' Test with backslash
    IF result THEN
        testPath$ = "path\to\file.bas"
        actual$ = getfilepath$(testPath$)
        expected$ = "path\to\"
        result = Test_AssertEqualString&(expected$, actual$, "Should extract path with backslash")
    END IF
    
    ' Test with no path (just filename)
    IF result THEN
        testPath$ = "file.bas"
        actual$ = getfilepath$(testPath$)
        expected$ = ""
        result = Test_AssertEqualString&(expected$, actual$, "Should return empty for filename without path")
    END IF
    
    ' Test with root path
    IF result THEN
        testPath$ = "/root/file.bas"
        actual$ = getfilepath$(testPath$)
        expected$ = "/root/"
        result = Test_AssertEqualString&(expected$, actual$, "Should handle root path")
    END IF
    
    ' Test with Windows drive path
    IF result THEN
        testPath$ = "C:\path\file.bas"
        actual$ = getfilepath$(testPath$)
        expected$ = "C:\path\"
        result = Test_AssertEqualString&(expected$, actual$, "Should handle Windows drive path")
    END IF
    
    Test_End result
END SUB

SUB Test_FileHasExtension
    Test_Start "FileHasExtension - Extension detection"
    
    DIM result AS LONG
    
    ' Test with extension
    result = Test_Assert&(FileHasExtension("file.bas") <> 0, "Should detect .bas extension")
    
    ' Test without extension
    IF result THEN
        result = Test_Assert&(FileHasExtension("file") = 0, "Should not detect extension when none exists")
    END IF
    
    ' Test with multiple dots
    IF result THEN
        result = Test_Assert&(FileHasExtension("file.name.bas") <> 0, "Should detect extension with multiple dots")
    END IF
    
    ' Test with path and extension
    IF result THEN
        result = Test_Assert&(FileHasExtension("path/to/file.bas") <> 0, "Should detect extension in path")
    END IF
    
    ' Test with path but no extension
    IF result THEN
        result = Test_Assert&(FileHasExtension("path/to/file") = 0, "Should not detect extension when path has no extension")
    END IF
    
    ' Test with dot in path but extension at end
    IF result THEN
        result = Test_Assert&(FileHasExtension("path.to/file.bas") <> 0, "Should detect extension when dot in path")
    END IF
    
    Test_End result
END SUB

SUB Test_RemoveFileExtension
    Test_Start "RemoveFileExtension$ - Extension removal"
    
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    ' Test basic extension removal
    testPath$ = "file.bas"
    actual$ = RemoveFileExtension$(testPath$)
    expected$ = "file"
    result = Test_AssertEqualString&(expected$, actual$, "Should remove .bas extension")
    
    ' Test with no extension
    IF result THEN
        testPath$ = "file"
        actual$ = RemoveFileExtension$(testPath$)
        expected$ = "file"
        result = Test_AssertEqualString&(expected$, actual$, "Should return unchanged when no extension")
    END IF
    
    ' Test with multiple dots
    IF result THEN
        testPath$ = "file.name.bas"
        actual$ = RemoveFileExtension$(testPath$)
        expected$ = "file.name"
        result = Test_AssertEqualString&(expected$, actual$, "Should remove only last extension")
    END IF
    
    ' Test with path
    IF result THEN
        testPath$ = "path/to/file.bas"
        actual$ = RemoveFileExtension$(testPath$)
        expected$ = "path/to/file"
        result = Test_AssertEqualString&(expected$, actual$, "Should remove extension from path")
    END IF
    
    ' Test with dot in path
    IF result THEN
        testPath$ = "path.to/file.bas"
        actual$ = RemoveFileExtension$(testPath$)
        expected$ = "path.to/file"
        result = Test_AssertEqualString&(expected$, actual$, "Should handle dot in path")
    END IF
    
    Test_End result
END SUB

SUB Test_GetFileExtension
    Test_Start "GetFileExtension$ - Extension extraction"
    
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    ' Test basic extension
    testPath$ = "file.bas"
    actual$ = GetFileExtension$(testPath$)
    expected$ = "bas"
    result = Test_AssertEqualString&(expected$, actual$, "Should extract .bas extension")
    
    ' Test with no extension
    IF result THEN
        testPath$ = "file"
        actual$ = GetFileExtension$(testPath$)
        expected$ = ""
        result = Test_AssertEqualString&(expected$, actual$, "Should return empty for no extension")
    END IF
    
    ' Test with multiple dots
    IF result THEN
        testPath$ = "file.name.bas"
        actual$ = GetFileExtension$(testPath$)
        expected$ = "bas"
        result = Test_AssertEqualString&(expected$, actual$, "Should extract last extension only")
    END IF
    
    ' Test with path
    IF result THEN
        testPath$ = "path/to/file.bas"
        actual$ = GetFileExtension$(testPath$)
        expected$ = "bas"
        result = Test_AssertEqualString&(expected$, actual$, "Should extract extension from path")
    END IF
    
    ' Test with dot in path
    IF result THEN
        testPath$ = "path.to/file.bas"
        actual$ = GetFileExtension$(testPath$)
        expected$ = "bas"
        result = Test_AssertEqualString&(expected$, actual$, "Should handle dot in path")
    END IF
    
    Test_End result
END SUB

SUB Test_PathSlashCorrect
    Test_Start "PATH_SLASH_CORRECT - Path separator normalization"
    
    InitOSTestVars_FileUtilities
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    IF os$ = "WIN" THEN
        ' On Windows, forward slashes should become backslashes
        testPath$ = "path/to/file.bas"
        PATH_SLASH_CORRECT testPath$
        expected$ = "path\to\file.bas"
        result = Test_AssertEqualString&(expected$, testPath$, "Windows: Should convert / to \")
        
        ' Backslashes should remain
        IF result THEN
            testPath$ = "path\to\file.bas"
            PATH_SLASH_CORRECT testPath$
            expected$ = "path\to\file.bas"
            result = Test_AssertEqualString&(expected$, testPath$, "Windows: Should keep \ unchanged")
        END IF
    ELSE
        ' On Linux, backslashes should become forward slashes
        testPath$ = "path\to\file.bas"
        PATH_SLASH_CORRECT testPath$
        expected$ = "path/to/file.bas"
        result = Test_AssertEqualString&(expected$, testPath$, "Linux: Should convert \ to /")
        
        ' Forward slashes should remain
        IF result THEN
            testPath$ = "path/to/file.bas"
            PATH_SLASH_CORRECT testPath$
            expected$ = "path/to/file.bas"
            result = Test_AssertEqualString&(expected$, testPath$, "Linux: Should keep / unchanged")
        END IF
    END IF
    
    Test_End result
END SUB

SUB Test_GetEscapedPath
    Test_Start "GetEscapedPath$ - Path escaping"
    
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    ' Test with backslashes (should be escaped)
    testPath$ = "path\to\file"
    actual$ = GetEscapedPath$(testPath$)
    expected$ = "path\\to\\file"
    result = Test_AssertEqualString&(expected$, actual$, "Should escape backslashes")
    
    ' Test with no backslashes
    IF result THEN
        testPath$ = "path/to/file"
        actual$ = GetEscapedPath$(testPath$)
        expected$ = "path/to/file"
        result = Test_AssertEqualString&(expected$, actual$, "Should leave forward slashes unchanged")
    END IF
    
    ' Test with mixed
    IF result THEN
        testPath$ = "path\to/file"
        actual$ = GetEscapedPath$(testPath$)
        expected$ = "path\\to/file"
        result = Test_AssertEqualString&(expected$, actual$, "Should escape only backslashes")
    END IF
    
    Test_End result
END SUB

SUB Test_RemoveDoubleSlashes
    Test_Start "RemoveDoubleSlashes$ - Double slash removal"
    
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    ' Test with double forward slashes
    testPath$ = "path//to//file"
    actual$ = RemoveDoubleSlashes$(testPath$)
    expected$ = "path/to/file"
    result = Test_AssertEqualString&(expected$, actual$, "Should remove double forward slashes")
    
    ' Test with double backslashes
    IF result THEN
        testPath$ = "path\\to\\file"
        actual$ = RemoveDoubleSlashes$(testPath$)
        expected$ = "path\to\file"
        result = Test_AssertEqualString&(expected$, actual$, "Should remove double backslashes")
    END IF
    
    ' Test with triple slashes
    IF result THEN
        testPath$ = "path///to///file"
        actual$ = RemoveDoubleSlashes$(testPath$)
        expected$ = "path/to/file"
        result = Test_AssertEqualString&(expected$, actual$, "Should remove multiple consecutive slashes")
    END IF
    
    ' Test with no double slashes
    IF result THEN
        testPath$ = "path/to/file"
        actual$ = RemoveDoubleSlashes$(testPath$)
        expected$ = "path/to/file"
        result = Test_AssertEqualString&(expected$, actual$, "Should leave single slashes unchanged")
    END IF
    
    Test_End result
END SUB

SUB Test_FixDirectoryName
    Test_Start "FixDirectoryName$ - Directory path normalization"
    
    InitOSTestVars_FileUtilities
    DIM result AS LONG
    DIM testPath$, expected$, actual$
    
    ' Test without trailing separator
    testPath$ = "path/to/dir"
    actual$ = FixDirectoryName$(testPath$)
    expected$ = "path/to/dir" + pathsep$
    result = Test_AssertEqualString&(expected$, actual$, "Should add trailing separator")
    
    ' Test with trailing separator
    IF result THEN
        testPath$ = "path/to/dir" + pathsep$
        actual$ = FixDirectoryName$(testPath$)
        expected$ = "path/to/dir" + pathsep$
        result = Test_AssertEqualString&(expected$, actual$, "Should keep existing trailing separator")
    END IF
    
    ' Test with empty string
    IF result THEN
        testPath$ = ""
        actual$ = FixDirectoryName$(testPath$)
        expected$ = ""
        result = Test_AssertEqualString&(expected$, actual$, "Should return empty for empty input")
    END IF
    
    Test_End result
END SUB

SUB Test_CopyFile
    Test_Start "CopyFile& - File copying"
    
    DIM result AS LONG
    DIM sourceFile$, destFile$, testContent$, readContent$
    DIM copyResult AS LONG
    
    ' Create a test file
    sourceFile$ = "test_copy_source.tmp"
    destFile$ = "test_copy_dest.tmp"
    testContent$ = "Test file content" + CHR$(10) + "Line 2"
    
    ' Write test content
    _WRITEFILE sourceFile$, testContent$
    
    ' Test successful copy
    copyResult = CopyFile&(sourceFile$, destFile$)
    result = Test_AssertEqual&(0, copyResult, "CopyFile should return 0 on success")
    
    ' Verify destination file exists and has correct content
    IF result THEN
        IF _FILEEXISTS(destFile$) THEN
            readContent$ = _READFILE$(destFile$)
            result = Test_AssertEqualString&(testContent$, readContent$, "Copied file should have correct content")
        ELSE
            result = Test_Assert&(0, "Destination file should exist after copy")
        END IF
    END IF
    
    ' Test copying non-existent file
    IF result THEN
        copyResult = CopyFile&("nonexistent_file_12345.tmp", "test_copy_dest2.tmp")
        result = Test_AssertEqual&(-1, copyResult, "CopyFile should return -1 for non-existent source")
    END IF
    
    ' Cleanup
    IF _FILEEXISTS(sourceFile$) THEN KILL sourceFile$
    IF _FILEEXISTS(destFile$) THEN KILL destFile$
    IF _FILEEXISTS("test_copy_dest2.tmp") THEN KILL "test_copy_dest2.tmp"
    
    Test_End result
END SUB

' Run all file utility tests
SUB RunFileUtilityTests
    Test_GetFilePath
    Test_FileHasExtension
    Test_RemoveFileExtension
    Test_GetFileExtension
    Test_PathSlashCorrect
    Test_GetEscapedPath
    Test_RemoveDoubleSlashes
    Test_FixDirectoryName
    Test_CopyFile
END SUB
