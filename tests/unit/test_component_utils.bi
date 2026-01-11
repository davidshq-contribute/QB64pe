'
' Component Test Utilities
'
' Helper functions for testing compiler components with the test harness.
' Provides utilities for setting up test contexts, inspecting outputs,
' and comparing results.
'

'$INCLUDE:'test_state_manager_declarations.bi'

' Initialize a test context for a specific component
' Returns initialized TestStateContext
FUNCTION TestComponent_InitContext (componentType$) AS TestStateContext
    DIM context AS TestStateContext
    TestState_Reset context
    TestState_Init context, componentType$
    TestComponent_InitContext = context
END FUNCTION

' Clean up a test context
SUB TestComponent_CleanupContext (context AS TestStateContext)
    TestState_Cleanup context
END SUB

' Helper to run a test with automatic context management
' componentType$: Type of component to initialize
' testSub: Name of test SUB to run
SUB TestComponent_RunWithContext (componentType$, testSub AS STRING)
    DIM context AS TestStateContext
    TestState_Init context, componentType$
    
    ' Note: QB64 doesn't support dynamic SUB calls, so this is a helper
    ' that sets up context. The actual test SUB should be called separately.
    ' This is mainly for documentation and future enhancement.
    
    ' For now, tests should call TestState_Init and TestState_Cleanup directly
END SUB

' Verify hash table state
' Returns 1 if hash table is properly initialized, 0 otherwise
FUNCTION TestComponent_VerifyHashTableState&
    IF HashListSize > 0 AND HashListNext >= 1 THEN
        TestComponent_VerifyHashTableState& = -1
    ELSE
        TestComponent_VerifyHashTableState& = 0
    END IF
END FUNCTION

' Verify type system state
' Returns 1 if type system is properly initialized, 0 otherwise
FUNCTION TestComponent_VerifyTypeSystemState&
    ' Check that type constants are initialized
    IF STRINGTYPE > 0 AND LONGTYPE > 0 AND SINGLETYPE > 0 THEN
        TestComponent_VerifyTypeSystemState& = -1
    ELSE
        TestComponent_VerifyTypeSystemState& = 0
    END IF
END FUNCTION

' Verify constant evaluation state
' Returns 1 if const eval is properly initialized, 0 otherwise
FUNCTION TestComponent_VerifyConstEvalState&
    ' Check that const arrays are initialized
    ' constmax should be > 0 and constname array should exist
    IF constmax > 0 THEN
        ' Try to access the array - if it exists, UBOUND will work
        DIM testBound AS LONG
        testBound = UBOUND(constname)
        IF testBound > 0 THEN
            TestComponent_VerifyConstEvalState& = -1
        ELSE
            TestComponent_VerifyConstEvalState& = 0
        END IF
    ELSE
        TestComponent_VerifyConstEvalState& = 0
    END IF
END FUNCTION

' Get hash table statistics for inspection
SUB TestComponent_GetHashTableStats (totalItems AS LONG, usedItems AS LONG, freeItems AS LONG)
    totalItems = HashListSize
    usedItems = HashListNext - 1
    freeItems = HashListFreeLast
END SUB

' Helper to create a minimal test constant
' name$: Constant name
' value$: Constant value (as string)
' Returns the index of the created constant, or -1 on failure
FUNCTION TestComponent_CreateTestConstant& (name$, value$)
    ' This is a placeholder - actual implementation would need to
    ' properly set up const arrays and call const evaluation functions
    ' For now, tests should set up constants manually
    TestComponent_CreateTestConstant& = -1
END FUNCTION
