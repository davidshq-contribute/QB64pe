'
' Minimal Test Runner for format.bas
'
' This is a minimal test runner that only includes what's needed for format.bas testing
'

$CONSOLE:ONLY

' Error handler stub
qberror_test:
    RESUME NEXT

' ============================================================================
' PHASE 1: Declarations
' ============================================================================

' Define Debug constant needed by constants.bas
CONST Debug = 0

' Include test framework declaration files (following test_runner.bas pattern exactly)
'$INCLUDE:'test_framework_constants.bi'
'$INCLUDE:'test_global_state_declarations.bi'
'$INCLUDE:'test_framework_declarations.bi'
'$INCLUDE:'test_state_manager_declarations.bi'
'$INCLUDE:'test_global_state_reset_declarations.bi'
'$INCLUDE:'test_output_verification.bi'

' Note: sp, sp2, sp3, sp_asc, sp2_asc, etc. will be declared by constants.bas in Phase 2

' Forward declarations for functions needed by format.bas
DECLARE FUNCTION isalpha& (c AS LONG)

' Forward declarations for test functions
DECLARE SUB RunFormatTests
DECLARE SUB RunFormatTestsWrapper

' Stub declaration for IncludeProvider_SkipIncludes (needed by test_framework_implementations.bas)
' We don't include include_provider for format.bas testing, so we provide a stub
DECLARE SUB IncludeProvider_SkipIncludes (enable AS LONG)

' ============================================================================
' PHASE 2: Main program code
' ============================================================================

' Include constants.bas here (in main program section) since it has initialization code
' This provides _CHR_CR, _ASC_CR, etc. that format.bas might need
$INCLUDE:'../../source/global/constants.bas'

' Call wrapper SUB from main program (wrapper will handle test framework initialization)
RunFormatTestsWrapper

' ============================================================================
' PHASE 3: Implementations
' ============================================================================

' Include format.bas FIRST (before any FUNCTION/SUB definitions to avoid implicit END)
' format.bas uses isalpha which is forward-declared in Phase 1
' This must come before isalpha definition to avoid "between SUB/FUNCTIONs" error
$INCLUDE:'../../source/utilities/format.bas'

' Define isalpha function needed by format.bas (after format.bas include, forward-declared in Phase 1)
FUNCTION isalpha& (c AS LONG)
    ' Returns true if c is an alphabetic character (A-Z or a-z)
    IF (c >= 65 AND c <= 90) OR (c >= 97 AND c <= 122) THEN
        isalpha& = -1
    ELSE
        isalpha& = 0
    END IF
END FUNCTION

' Stub implementation for IncludeProvider_SkipIncludes (needed by test framework)
SUB IncludeProvider_SkipIncludes (enable AS LONG)
    ' Stub - does nothing, format.bas doesn't need include provider
END SUB

' Include test framework implementations
'$INCLUDE:'test_framework_implementations.bas'

' Include test global state reset implementations (needed by test framework)
'$INCLUDE:'test_global_state_reset_implementations.bas'

' Utility functions needed by test files (give_error.bas has $INCLUDEONCE)
'$INCLUDE:'../../source/utilities/give_error.bas'

' Include format test (format.bas already included above, $INCLUDEONCE will skip duplicate)
'$INCLUDE:'format/test_format.bas'

' Wrapper SUB that initializes test framework and runs format tests
' This pattern allows calling test framework functions from main program
' by wrapping them in a SUB that's called from main program
SUB RunFormatTestsWrapper
    ' Initialize test framework
    TestFramework_Init
    TestFramework_SetVerbose 1
    
    PRINT "QB64 Compiler Unit Tests - Format.bas"
    PRINT "======================================"
    PRINT ""
    
    PRINT "Running format utility tests..."
    RunFormatTests
    
    PRINT ""
    TestFramework_PrintSummary
    
    IF NOT TestFramework_AllPassed& THEN
        SYSTEM 1
    END IF
END SUB
