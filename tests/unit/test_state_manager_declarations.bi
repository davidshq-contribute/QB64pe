'
' Test State Manager Declarations
'
' TYPE definitions and forward declarations for test state management.
' This file contains only TYPE definitions and DECLARE statements and must
' be included before any SUB/FUNCTION definitions.
'

' Test state context structure
' Tracks which components have been initialized for testing
TYPE TestStateContext
    ' Component initialization flags
    hashTableInitialized AS LONG
    typeSystemInitialized AS LONG
    constEvalInitialized AS LONG
    
    ' Saved state for restoration
    savedHashListSize AS LONG
    savedHashListNext AS LONG
    savedHashListFreeSize AS LONG
    savedHashListFreeLast AS LONG
    savedConstMax AS LONG
    savedConstLast AS LONG
END TYPE

' Forward declarations for test state management functions
DECLARE SUB TestState_Init (context AS TestStateContext, componentType$)
DECLARE SUB TestState_Cleanup (context AS TestStateContext)
DECLARE SUB TestState_Reset (context AS TestStateContext)
