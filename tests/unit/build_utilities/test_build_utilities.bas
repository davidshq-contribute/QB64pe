'
' Unit Tests for Build Utilities (build.bas)
'
' Tests make executable resolution and filename generation.
' Uses component test harness for isolated testing.
'
' Note: test_framework.bi and test_state_manager.bi are included by test_runner.bas
'$INCLUDE:'../../../source/global/constants.bas'
'$INCLUDE:'../../../source/utilities/build.bas'
'$INCLUDE:'../../../source/utilities/strings.bas'

' Initialize OS-specific variables for testing
' Note: These must match the declarations in qb64pe.bas and cfg_global.bas
' os$ is already declared in test_global_state_declarations.bi
' pathsep, tmpdir$, and UseSystemMinGW are already declared in test_global_state_declarations.bi

SUB InitOSTestVars
    ' Initialize os$ and pathsep$ similar to qb64pe.bas
    os$ = "WIN"
    IF INSTR(_OS$, "[LINUX]") THEN os$ = "LNX"
    
    pathsep$ = "\"
    IF os$ = "LNX" THEN pathsep$ = "/"
    
    ' Initialize tmpdir$ for testing
    tmpdir$ = "./internal/temp/"
    IF os$ = "WIN" THEN tmpdir$ = ".\internal\temp\"
END SUB

SUB Test_GetMakeExecutable
    Test_Start "GetMakeExecutable$ - Make executable path resolution"
    
    InitOSTestVars
    DIM result AS LONG
    DIM makeExe$
    
    makeExe$ = GetMakeExecutable$
    
    IF os$ = "WIN" THEN
        ' On Windows, should return path with mingw32-make.exe
        result = Test_AssertContains&(makeExe$, "mingw32-make.exe", "Windows: Should include mingw32-make.exe")
    ELSE
        ' On Linux, should return just "make"
        result = Test_AssertEqualString&("make", makeExe$, "Linux: Should return 'make'")
    END IF
    
    Test_End result
END SUB

SUB Test_GetCompilerPath
    Test_Start "GetCompilerPath$ - Compiler path resolution"
    
    DIM result AS LONG
    DIM compilerPath$
    
    ' Test with UseSystemMinGW = 0 (should return path)
    UseSystemMinGW = 0
    compilerPath$ = GetCompilerPath$
    result = Test_AssertContains&(compilerPath$, "internal", "Should return path when not using system MinGW")
    
    ' Test with UseSystemMinGW = -1 (should return empty)
    IF result THEN
        UseSystemMinGW = -1
        compilerPath$ = GetCompilerPath$
        result = Test_AssertEqualString&("", compilerPath$, "Should return empty when using system MinGW")
    END IF
    
    Test_End result
END SUB

SUB Test_MakeNMOutputFilename
    Test_Start "MakeNMOutputFilename$ - Output filename generation"
    
    InitOSTestVars
    DIM result AS LONG
    DIM libfile$, filename$, expected$
    
    ' Test basic filename generation (non-dynamic)
    libfile$ = "testlib"
    filename$ = MakeNMOutputFilename$(libfile$, 0)
    result = Test_AssertContains&(filename$, "nm_output_", "Should contain 'nm_output_' prefix")
    IF result THEN result = Test_AssertContains&(filename$, "testlib", "Should contain library name")
    IF result THEN result = Test_AssertContains&(filename$, ".txt", "Should have .txt extension")
    
    ' Test dynamic filename
    IF result THEN
        filename$ = MakeNMOutputFilename$(libfile$, -1)
        result = Test_AssertContains&(filename$, "_dynamic", "Should contain '_dynamic' for dynamic library")
    END IF
    
    ' Test with path separators (should be replaced)
    IF result THEN
        libfile$ = "path" + pathsep$ + "to" + pathsep$ + "lib"
        filename$ = MakeNMOutputFilename$(libfile$, 0)
        result = Test_AssertNotContains&(filename$, pathsep$, "Should replace path separators")
    END IF
    
    ' Test with colon (Windows drive, should be replaced)
    IF result THEN
        libfile$ = "C:lib"
        filename$ = MakeNMOutputFilename$(libfile$, 0)
        result = Test_AssertNotContains&(filename$, ":", "Should replace colons")
    END IF
    
    Test_End result
END SUB

' Run all build utility tests
SUB RunBuildUtilityTests
    Test_GetMakeExecutable
    Test_GetCompilerPath
    Test_MakeNMOutputFilename
END SUB
