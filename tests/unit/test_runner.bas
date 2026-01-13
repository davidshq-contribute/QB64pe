'
' Unit Test Runner
'
' Discovers and runs unit tests for QB64 compiler components.
'
' This file follows the recommended structure from docs/problems_encountered/qb64_main_program_structure.md:
' 1. Include declaration files (.bi) with only CONST, TYPE, DIM SHARED, DECLARE
' 2. Include main program code (test_runner_main.bas) with RunAllTests call
' 3. Include implementation files (.bas) with SUB/FUNCTION definitions
'
'$CONSOLE:ONLY

' Error handler stub for utility files that reference qberror_test
' This label is normally defined in qb64pe.bas, but we need it for test compilation
' Must be defined before any SUB/FUNCTION declarations
' Note: Error handler must be in main program section
' Skip over error handler during normal execution
GOTO SkipErrorHandler
qberror_test:
    RESUME NEXT
SkipErrorHandler:

' ============================================================================
' PHASE 1: Include declaration files (CONST, TYPE, DIM SHARED, DECLARE only)
' ============================================================================
' These files contain only declarations and do not trigger implicit END

' Define constants needed for test compilation
' Debug constant (from source/global/settings.bas) - needed by constants.bas
CONST Debug = 0

' Now we can include the split declaration files (no executable code)
'$INCLUDE:'../../source/utilities/hash_declarations.bi'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer_declarations.bi'
'$INCLUDE:'../../source/utilities/type_declarations.bi'

' Test framework infrastructure declarations
'$INCLUDE:'test_framework_constants.bi'
'$INCLUDE:'test_global_state_declarations.bi'
'$INCLUDE:'include_provider_declarations.bi'
'$INCLUDE:'test_framework_declarations.bi'
'$INCLUDE:'test_state_manager_declarations.bi'
'$INCLUDE:'test_global_state_reset_declarations.bi'
'$INCLUDE:'test_output_verification.bi'

' ============================================================================
' PHASE 2: Main program code (executes before SUB/FUNCTION definitions)
' ============================================================================
' This file contains the RunAllTests call and must be included after
' all declaration files but before any implementation files with SUB/FUNCTION

' Include constants.bas here (in main program section) since it has initialization code
'$INCLUDE:'../../source/global/constants.bas'

' Include initialization files (executable code to initialize arrays and variables)
'$INCLUDE:'../../source/utilities/hash_init.bas'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer_init.bas'
'$INCLUDE:'../../source/utilities/type_init.bas'

'$INCLUDE:'test_runner_main.bas'

' ============================================================================
' PHASE 3: Include implementation files (SUB/FUNCTION definitions)
' ============================================================================
' These files contain SUB/FUNCTION implementations and will trigger implicit END
' They are included after the main program code to ensure proper execution order

' Type system functions (FUNCTION definitions, must be before test suites)
'$INCLUDE:'../../source/utilities/type.bas'

' Include provider implementations
'$INCLUDE:'include_provider_implementations.bas'
'$INCLUDE:'../../source/utilities/include_provider.bas'

' Test framework implementations
'$INCLUDE:'test_framework_implementations.bas'

' Test state manager implementations
'$INCLUDE:'test_state_manager_implementations.bas'

' Test global state reset implementations
'$INCLUDE:'test_global_state_reset_implementations.bas'

' Utility functions needed by test files
' Note: give_error.bas now has $INCLUDEONCE, so it can be included multiple times safely
'$INCLUDE:'../../source/utilities/give_error.bas'

' Output verification implementations
'$INCLUDE:'test_output_verification.bas'

' Parser utility functions needed by tests
'$INCLUDE:'../../source/utilities/parser_utils.bas'

' Test suite implementations
'$INCLUDE:'type_system/test_type_system.bas'
'$INCLUDE:'type_system/test_type_conversion.bas'
'$INCLUDE:'symbol_table/test_hash.bas'
'$INCLUDE:'parser/test_parser.bas'
'$INCLUDE:'parser/test_parser_utils.bas'
'$INCLUDE:'code_generation/test_code_generation.bas'
'$INCLUDE:'file_utilities/test_file_utilities.bas'
'$INCLUDE:'string_utilities/test_string_utilities.bas'
'$INCLUDE:'include_provider/test_include_provider.bas'
'$INCLUDE:'error_handling/test_error_handling.bas'
'$INCLUDE:'statevars/test_statevars.bas'
'$INCLUDE:'build_utilities/test_build_utilities.bas'
'$INCLUDE:'build_utilities/test_elements.bas'
'$INCLUDE:'format/test_format.bas'

SUB RunAllTests
    STATIC initialized AS LONG
    IF NOT initialized THEN
        TestFramework_Init
        TestFramework_SetVerbose 1
        initialized = -1
    END IF
    
    PRINT "QB64 Compiler Unit Tests"
    PRINT "========================"
    PRINT ""

    PRINT "Running type system tests..."
    RunTypeSystemTests

    PRINT "Running type conversion tests..."
    RunTypeConversionTests

    PRINT "Running symbol table tests..."
    RunSymbolTableTests

    PRINT "Running parser tests..."
    RunParserTests

    PRINT "Running parser utility tests..."
    RunParserUtilityTests

    PRINT "Running code generation tests..."
    RunCodeGenerationTests

    PRINT "Running file utility tests..."
    RunFileUtilityTests

    PRINT "Running string utility tests..."
    RunStringUtilityTests

    PRINT "Running include provider tests..."
    RunIncludeProviderTests

    PRINT "Running error handling tests..."
    RunErrorHandlingTests

    PRINT "Running state variable tests..."
    RunStateVarTests

    PRINT "Running build utility tests..."
    RunBuildUtilityTests

    PRINT "Running element utility tests..."
    RunElementTests

    PRINT "Running format utility tests..."
    RunFormatTests
    
    PRINT ""
    TestFramework_PrintSummary
    
    IF NOT TestFramework_AllPassed& THEN
        SYSTEM 1
    END IF
END SUB
