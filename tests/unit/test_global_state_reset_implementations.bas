'
' Test Global State Reset Implementation
'
' Implementation of test global state reset SUB definition.
' This file contains only SUB/FUNCTION implementations and must be included
' after the main program code section.
'

' Reset all global state variables to safe defaults
' This ensures test isolation by clearing state between tests
SUB Test_ResetGlobalState
    ' Reset error handling state
    Error_Happened = 0
    Error_Message = ""
    
    ' Reset recompile flag
    recompile = 0
    
    ' Reset config file path
    ConfigFile$ = ""
    
    ' Reset OS-specific variables to defaults
    ' Note: These will be properly initialized by individual tests if needed
    os$ = ""
    pathsep$ = ""
    tmpdir$ = ""
    UseSystemMinGW = 0
    
    ' Reset format variables to defaults
    layout$ = ""
    IDEAutoIndent = 0
    IDEAutoIndentSize = 0
    IDEAutoLayout = 0
    sp_asc = 0
    sp2_asc = 0
END SUB
