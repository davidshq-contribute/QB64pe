'QB64 Compiler State Accessor Functions
'Provides controlled access to compiler state (following libqb_state.cpp pattern)
'These functions initially wrap existing globals for backward compatibility

'=============================================================================
' Path State Accessors
'=============================================================================

FUNCTION GetOS$
    GetOS$ = os$
END FUNCTION

FUNCTION GetPathSep$
    GetPathSep$ = pathsep$
END FUNCTION

FUNCTION GetTmpDir$
    GetTmpDir$ = tmpdir$
END FUNCTION

FUNCTION GetExtension$
    GetExtension$ = extension$
END FUNCTION

'=============================================================================
' Compilation Position Accessors
'=============================================================================

FUNCTION GetLineNumber&
    GetLineNumber& = linenumber
END FUNCTION

SUB SetLineNumber (n AS LONG)
    linenumber = n
END SUB

'Get current line number (alternative name for GetLineNumber)
FUNCTION GetCurrentLineNumber&
    GetCurrentLineNumber& = linenumber
END FUNCTION

'Set current line number (alternative name for SetLineNumber)
SUB SetCurrentLineNumber (n AS LONG)
    linenumber = n
END SUB

FUNCTION IsPrepass%%
    IsPrepass%% = prepass
END FUNCTION

SUB SetPrepass (value AS _BYTE)
    prepass = value
END SUB

'=============================================================================
' Scope Accessors
'=============================================================================

FUNCTION GetSubFuncN&
    GetSubFuncN& = subfuncn
END FUNCTION

FUNCTION GetCurrentScopeName$
    IF subfuncn = 0 THEN
        GetCurrentScopeName$ = ""
    ELSE
        GetCurrentScopeName$ = subfunc$
    END IF
END FUNCTION

'Get current scope name (alternative name for GetCurrentScopeName)
FUNCTION GetCurrentScope$
    IF subfuncn = 0 THEN
        GetCurrentScope$ = ""
    ELSE
        GetCurrentScope$ = subfunc$
    END IF
END FUNCTION

FUNCTION IsInSubFunc%%
    IsInSubFunc%% = (subfuncn <> 0)
END FUNCTION

'=============================================================================
' Include State Accessors
'=============================================================================

FUNCTION GetIncludeLevel&
    GetIncludeLevel& = inclevel
END FUNCTION

FUNCTION GetIncludeFileName$
    IF inclevel > 0 THEN
        GetIncludeFileName$ = incname$(inclevel)
    ELSE
        GetIncludeFileName$ = ""
    END IF
END FUNCTION

'=============================================================================
' Layout State Accessors
'=============================================================================

FUNCTION GetLayout$
    GetLayout$ = layout$
END FUNCTION

SUB SetLayout (value AS STRING)
    layout$ = value
END SUB

FUNCTION IsLayoutOK%%
    IsLayoutOK%% = layoutok
END FUNCTION

SUB SetLayoutOK (value AS LONG)
    layoutok = value
END SUB

'=============================================================================
' IDE State Accessors
'=============================================================================

FUNCTION IsIDEMode%%
    IsIDEMode%% = idemode
END FUNCTION

FUNCTION GetIDECommand$
    GetIDECommand$ = idecommand$
END FUNCTION

SUB SetIDEReturn (value AS STRING)
    idereturn$ = value
END SUB

FUNCTION GetIDEError&
    GetIDEError& = ideerror
END FUNCTION

SUB SetIDEError (value AS LONG)
    ideerror = value
END SUB

'=============================================================================
' Error State Accessors
'=============================================================================

FUNCTION IsCompilationFailed%%
    IsCompilationFailed%% = compfailed
END FUNCTION

SUB SetCompilationFailed (value AS _BYTE)
    compfailed = value
END SUB

'=============================================================================
' Utility Accessors
'=============================================================================

FUNCTION GetUniqueNumber&
    uniquenumbern = uniquenumbern + 1
    GetUniqueNumber& = uniquenumbern
END FUNCTION

FUNCTION IsNumericChar%% (c AS INTEGER)
    IsNumericChar%% = isnumeric(c)
END FUNCTION

FUNCTION IsAlphaChar%% (c AS INTEGER)
    IsAlphaChar%% = isalpha(c)
END FUNCTION

FUNCTION IsAlphanumericChar%% (c AS INTEGER)
    IsAlphanumericChar%% = alphanumeric(c)
END FUNCTION
