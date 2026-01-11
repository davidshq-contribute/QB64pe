'
' Test Global State Variable Declarations
'
' Declares all global state variables as SHARED so they can be reset consistently across all tests.
' This file contains only DIM SHARED declarations and must be included before any SUB/FUNCTION declarations.
'

' Error handling state (from give_error.bi)
DIM SHARED Error_Happened AS LONG
DIM SHARED Error_Message AS STRING

' Recompile flag (from statevars tests)
DIM SHARED recompile AS INTEGER

' Config file path (from string_utilities tests)
DIM SHARED ConfigFile$

' OS-specific variables (from file_utilities and build_utilities tests)
DIM SHARED os$
DIM SHARED pathsep AS STRING * 1
DIM SHARED tmpdir$
DIM SHARED UseSystemMinGW AS LONG

' Format variables (from format tests)
DIM SHARED layout$
DIM SHARED IDEAutoIndent AS LONG
DIM SHARED IDEAutoIndentSize AS LONG
DIM SHARED IDEAutoLayout AS LONG
' Note: sp_asc and sp2_asc are declared in constants.bas

' Test framework shared variables
TYPE TestStats
    totalTests AS LONG
    passedTests AS LONG
    failedTests AS LONG
    skippedTests AS LONG
    totalAssertions AS LONG
    passedAssertions AS LONG
    failedAssertions AS LONG
END TYPE

DIM SHARED testStats AS TestStats
DIM SHARED currentTestName$
DIM SHARED testOutput$
DIM SHARED testVerbose AS LONG

' Output verification arrays (must be declared before SUB/FUNCTION declarations)
' Note: SymbolInfo TYPE is defined in test_output_verification.bi
' These arrays are declared here to ensure they come before SUB/FUNCTION declarations
' The TYPE definition will be included later via test_output_verification.bi
DIM SHARED snapshotSymbols(10000) AS SymbolInfo
DIM SHARED snapshotLines(10000) AS STRING
