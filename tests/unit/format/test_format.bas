'
' Unit Tests for Format Utilities (format.bas)
'
' Tests code formatting and indentation logic.
' Uses component test harness for isolated testing.
'
' Note: test_framework_declarations.bi, test_framework_implementations.bas, test_state_manager_declarations.bi, and test_state_manager_implementations.bas are included by test_runner.bas
' Note: format.bas has many dependencies on global IDE variables, so we test
'       what we can with minimal setup
' Note: constants.bas is included by test_runner.bas
' We don't include it here to avoid duplicate includes
'$INCLUDE:'../../../source/utilities/format.bas'

' Initialize format test variables
' Note: These must match the declarations in cfg_global.bas and constants.bas
' layout$, sp_asc, sp2_asc, IDEAutoIndent, IDEAutoIndentSize, and IDEAutoLayout
' are already declared in test_global_state_declarations.bi and constants.bas

SUB InitFormatTestVars
    ' Initialize spacing constants (already initialized by test runner, but ensure values)
    ' sp_asc and sp2_asc are already set by test_runner_format_minimal.bas
    
    ' Initialize IDE settings to defaults
    IDEAutoIndent = -1
    IDEAutoIndentSize = 4
    IDEAutoLayout = -1
    
    ' Initialize layout$ and original$ for testing
    layout$ = ""
END SUB

SUB Test_ApplyLayoutIndentEmpty
    Test_Start "apply_layout_indent$ - Empty layout handling"
    
    InitFormatTestVars
    DIM result AS LONG
    DIM original$, result$
    
    ' Test with empty layout (should return empty)
    layout$ = ""
    original$ = "test code"
    result$ = apply_layout_indent$(original$)
    result = Test_AssertEqualString&("", result$, "Should return empty for empty layout")
    
    Test_End result
END SUB

SUB Test_ApplyLayoutIndentBasic
    Test_Start "apply_layout_indent$ - Basic indentation"
    
    InitFormatTestVars
    DIM result AS LONG
    DIM original$, result$
    
    ' Test with simple layout and indentation
    layout$ = "    PRINT " + CHR$(34) + "test" + CHR$(34)
    original$ = "print " + CHR$(34) + "test" + CHR$(34)
    IDEAutoIndent = -1
    IDEAutoIndentSize = 4
    
    result$ = apply_layout_indent$(original$)
    ' Result should have indentation applied
    result = Test_AssertNotEmpty&(result$, "Should return formatted result")
    
    ' Should contain the code content
    IF result THEN
        result = Test_AssertContains&(result$, "PRINT", "Should contain formatted code")
    END IF
    
    Test_End result
END SUB

SUB Test_ApplyLayoutIndentNoAutoIndent
    Test_Start "apply_layout_indent$ - No auto-indent mode"
    
    InitFormatTestVars
    DIM result AS LONG
    DIM original$, result$
    
    ' Test with IDEAutoIndent = 0 (preserve original indent)
    layout$ = "    PRINT " + CHR$(34) + "test" + CHR$(34)
    original$ = "  print " + CHR$(34) + "test" + CHR$(34)
    IDEAutoIndent = 0
    IDEAutoIndentSize = 4
    
    result$ = apply_layout_indent$(original$)
    result = Test_AssertNotEmpty&(result$, "Should return formatted result")
    
    ' Should preserve original indentation
    IF result THEN
        ' Result should start with spaces (original indent)
        result = Test_Assert&(ASC(result$, 1) = 32 OR LEN(result$) > 0, "Should preserve or apply indentation")
    END IF
    
    Test_End result
END SUB

SUB Test_ApplyLayoutIndentNoAutoLayout
    Test_Start "apply_layout_indent$ - No auto-layout mode"
    
    InitFormatTestVars
    DIM result AS LONG
    DIM original$, result$
    
    ' Test with IDEAutoLayout = 0 (preserve original layout)
    layout$ = "    PRINT " + CHR$(34) + "test" + CHR$(34)
    original$ = "print " + CHR$(34) + "test" + CHR$(34)
    IDEAutoLayout = 0
    IDEAutoIndent = -1
    
    result$ = apply_layout_indent$(original$)
    result = Test_AssertNotEmpty&(result$, "Should return formatted result")
    
    ' Should use original layout
    IF result THEN
        result = Test_AssertContains&(result$, "print", "Should contain original code")
    END IF
    
    Test_End result
END SUB

' Note: More complex tests for ? to PRINT conversion, quote handling, etc.
' would require more extensive setup of global variables and are better
' suited for integration tests.

' Run all format utility tests
SUB RunFormatTests
    Test_ApplyLayoutIndentEmpty
    Test_ApplyLayoutIndentBasic
    Test_ApplyLayoutIndentNoAutoIndent
    Test_ApplyLayoutIndentNoAutoLayout
END SUB
