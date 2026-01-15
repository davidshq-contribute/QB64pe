'QB64 Compiler State Container Types
'Consolidates related global variables into structured types for better organization

'Compiler path and environment state
TYPE CompilerPathState
    os AS STRING              'Operating system: "WIN", "LNX"
    pathExe AS STRING         'Path to executable output
    pathSource AS STRING      'Path to source file
    tmpdir AS STRING          'Temporary directory path
    tmpdir2 AS STRING         'Alternative temp directory path
    pathsep AS STRING * 1     'Path separator character
    extension AS STRING       'Executable extension (.exe or "")
    tempfolderindex AS LONG   'Index of temp folder being used
END TYPE

'Compiler flags and options
TYPE CompilerFlagsState
    prepass AS _BYTE          'True during pre-compilation pass
    recompile AS INTEGER      'Recompilation needed flag
    compfailed AS _BYTE       'Compilation failed flag
    idemode AS _BYTE          'IDE mode active
    noIDEMode AS _BYTE        'Command-line mode (no IDE)
    consoleMode AS _BYTE      'Console mode compilation
    formatMode AS _BYTE       'Format mode
    noCCompileMode AS _BYTE   'Skip C++ compilation
    quietMode AS _BYTE        'Suppress output
    debug AS _BYTE            'Debug mode
END TYPE

'Line and position tracking during compilation
TYPE CompilerPositionState
    linenumber AS LONG        'Current logical line number
    reallinenumber AS LONG    'Actual line number in source
    totallinenumber AS LONG   'Total lines processed
    linecontinuation AS _BYTE 'Line continuation active
END TYPE

'Scope and control flow state
TYPE CompilerScopeState
    subfunc AS STRING         'Current SUB/FUNCTION name
    subfuncn AS LONG          'SUB/FUNCTION index
    controllevel AS INTEGER   'Nesting level of control structures
END TYPE

'Include file handling state
TYPE CompilerIncludeState
    inclevel AS LONG                'Current include nesting level
    incname AS STRING               'Current include filename
    inclinenumber AS LONG           'Line number in current include
    incerror AS STRING              'Include error message
    autoIncludingFile AS LONG       'Auto-include file index
END TYPE

'Layout/formatting state for source code
TYPE CompilerLayoutState
    layout AS STRING              'Current layout string
    layoutok AS LONG              'Layout status for line
    layoutcomment AS STRING       'Comment portion of layout
    layoutcontinuations AS STRING 'Physical lines in logical line
    tlayout AS STRING             'Temporary layout string
    layoutdone AS LONG            'Layout complete flag
END TYPE

'IDE communication state
TYPE CompilerIDEState
    idecommand AS STRING      'Command from IDE (1 byte code + data)
    idereturn AS STRING       'Return data to IDE
    ideerror AS LONG          'Error code for IDE
    idecompiled AS LONG       'Lines compiled count
    idemessage AS STRING      'Error message for IDE
    ideerrorline AS LONG      'Error line number for IDE
END TYPE

'Error handling state
TYPE CompilerErrorState
    qberrorcode AS LONG       'QB error code
    qberrorline AS LONG       'Line where error occurred
    qberrormessage AS STRING  'Error message text
    E AS LONG                 'General error flag
END TYPE

'Note: State container instances (gPath, gFlags, etc.) are not declared here.
'The accessor functions in compiler_state.bas wrap existing globals for now.
'Future migration will instantiate these containers and update the accessors.
