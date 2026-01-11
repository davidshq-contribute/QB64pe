' Main program entry point for test runner
' This file is included after declaration includes but before implementation includes
' to ensure the RunAllTests call is in the main program section

' Forward declaration - allows calling RunAllTests before it's defined
DECLARE SUB RunAllTests

' Forward declarations for all test runner functions
DECLARE SUB RunTypeSystemTests
DECLARE SUB RunSymbolTableTests
DECLARE SUB RunParserTests
DECLARE SUB RunCodeGenerationTests
DECLARE SUB RunFileUtilityTests
DECLARE SUB RunStringUtilityTests
DECLARE SUB RunIncludeProviderTests
DECLARE SUB RunErrorHandlingTests
DECLARE SUB RunStateVarTests
DECLARE SUB RunBuildUtilityTests
DECLARE SUB RunFormatTests

' Forward declarations for test framework functions
DECLARE SUB TestFramework_Init
DECLARE SUB TestFramework_SetVerbose (verbose AS LONG)
DECLARE FUNCTION TestFramework_AllPassed& ()
DECLARE SUB TestFramework_PrintSummary

' Main program entry point
' This executes in the main program section before any SUB/FUNCTION definitions
RunAllTests
