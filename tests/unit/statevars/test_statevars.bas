'
' Unit Tests for State Variables (statevars.bas)
'
' Tests RCStateVar lifecycle, recompile triggers, and state precedence.
' Uses component test harness for isolated testing.
'
' Note: test_framework_declarations.bi, test_framework_implementations.bas, test_state_manager_declarations.bi, and test_state_manager_implementations.bas are included by test_runner.bas
'$INCLUDE:'../../../source/utilities/statevars.bi'
'$INCLUDE:'../../../source/utilities/statevars.bas'

' Initialize recompile variable for testing
' Note: Must match type in qb64pe.bas (INTEGER, not LONG)
' recompile is already declared in test_global_state_declarations.bi

SUB Test_ClearRCStateVar
    Test_Start "ClearRCStateVar - State variable clearing"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    
    ' Set some values
    stVar.wanted = 1
    stVar.actual = 1
    stVar.locked = 1
    stVar.forced = 1
    
    ' Clear the state variable
    ClearRCStateVar stVar
    
    ' Verify all values are cleared
    result = Test_AssertEqual&(0, stVar.wanted, "wanted should be cleared")
    IF result THEN result = Test_AssertEqual&(0, stVar.actual, "actual should be cleared")
    IF result THEN result = Test_AssertEqual&(0, stVar.locked, "locked should be cleared")
    IF result THEN result = Test_AssertEqual&(0, stVar.forced, "forced should be cleared")
    
    Test_End result
END SUB

SUB Test_SetRCStateVar
    Test_Start "SetRCStateVar - State setting with recompile trigger"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    
    ' Initialize state
    ClearRCStateVar stVar
    recompile = 0
    
    ' Set state when actual != wanted and not locked (should trigger recompile)
    SetRCStateVar stVar, 1
    result = Test_AssertEqual&(1, stVar.wanted, "wanted should be set")
    IF result THEN result = Test_AssertEqual&(1, recompile, "recompile should be triggered")
    
    ' Set same value again (should not trigger recompile)
    IF result THEN
        recompile = 0
        SetRCStateVar stVar, 1
        result = Test_AssertEqual&(0, recompile, "recompile should not be triggered for same value")
    END IF
    
    ' Set different value (should trigger recompile)
    IF result THEN
        recompile = 0
        SetRCStateVar stVar, 2
        result = Test_AssertEqual&(2, stVar.wanted, "wanted should be updated")
        IF result THEN result = Test_AssertEqual&(1, recompile, "recompile should be triggered for different value")
    END IF
    
    Test_End result
END SUB

SUB Test_SetRCStateVarLocked
    Test_Start "SetRCStateVar - Locked state behavior"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    
    ' Initialize state with locked value
    ClearRCStateVar stVar
    stVar.actual = 1
    stVar.locked = 1
    recompile = 0
    
    ' Set state when locked (should not trigger recompile)
    SetRCStateVar stVar, 2
    result = Test_AssertEqual&(2, stVar.wanted, "wanted should be set")
    IF result THEN result = Test_AssertEqual&(0, recompile, "recompile should not be triggered when locked")
    
    Test_End result
END SUB

SUB Test_ForceRCStateVar
    Test_Start "ForceRCStateVar - Forced state override"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    
    ' Initialize state
    ClearRCStateVar stVar
    stVar.actual = 1
    stVar.wanted = 1
    
    ' Force a different value
    ForceRCStateVar stVar, 2
    result = Test_AssertEqual&(2, stVar.forced, "forced should be set")
    
    ' Forced value should not affect wanted or actual yet
    IF result THEN
        result = Test_AssertEqual&(1, stVar.wanted, "wanted should remain unchanged")
    END IF
    
    IF result THEN
        result = Test_AssertEqual&(1, stVar.actual, "actual should remain unchanged until ExecuteRCStateVar")
    END IF
    
    Test_End result
END SUB

SUB Test_ExecuteRCStateVar
    Test_Start "ExecuteRCStateVar - State execution"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    
    ' Initialize state with wanted != actual
    ClearRCStateVar stVar
    stVar.wanted = 1
    stVar.actual = 0
    stVar.locked = 0
    
    ' Execute state (should update actual and lock)
    ExecuteRCStateVar stVar
    result = Test_AssertEqual&(1, stVar.actual, "actual should be updated to wanted")
    IF result THEN result = Test_AssertEqual&(1, stVar.locked, "should be locked after execution")
    
    ' Test with forced value (should override)
    IF result THEN
        ClearRCStateVar stVar
        stVar.wanted = 1
        stVar.actual = 0
        stVar.forced = 2
        stVar.locked = 0
        
        ExecuteRCStateVar stVar
        result = Test_AssertEqual&(2, stVar.actual, "actual should be forced value")
    END IF
    
    ' Test when already locked (should not change)
    IF result THEN
        ClearRCStateVar stVar
        stVar.wanted = 2
        stVar.actual = 1
        stVar.locked = 1
        
        ExecuteRCStateVar stVar
        result = Test_AssertEqual&(1, stVar.actual, "actual should not change when locked")
    END IF
    
    Test_End result
END SUB

SUB Test_GetRCStateVar
    Test_Start "GetRCStateVar%% - State retrieval"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    DIM value AS _BYTE
    
    ' Initialize state
    ClearRCStateVar stVar
    stVar.actual = 5
    
    ' Get state value
    value = GetRCStateVar%%(stVar)
    result = Test_AssertEqual&(5, value, "Should return actual value")
    
    ' Test with different value
    IF result THEN
        stVar.actual = 10
        value = GetRCStateVar%%(stVar)
        result = Test_AssertEqual&(10, value, "Should return updated actual value")
    END IF
    
    Test_End result
END SUB

SUB Test_StateVarLifecycle
    Test_Start "RCStateVar - Complete lifecycle"
    
    DIM result AS LONG
    DIM stVar AS RCStateVar
    
    ' Start with cleared state
    ClearRCStateVar stVar
    recompile = 0
    
    ' Set wanted value (triggers recompile)
    SetRCStateVar stVar, 1
    result = Test_AssertEqual&(1, recompile, "Should trigger recompile")
    
    ' Execute state (updates actual and locks)
    IF result THEN
        ExecuteRCStateVar stVar
        result = Test_AssertEqual&(1, stVar.actual, "actual should be updated")
        IF result THEN result = Test_AssertEqual&(1, stVar.locked, "should be locked")
    END IF
    
    ' Try to set new value (should not trigger recompile when locked)
    IF result THEN
        recompile = 0
        SetRCStateVar stVar, 2
        result = Test_AssertEqual&(2, stVar.wanted, "wanted should be updated")
        IF result THEN result = Test_AssertEqual&(0, recompile, "should not trigger recompile when locked")
    END IF
    
    ' Force a value
    IF result THEN
        ForceRCStateVar stVar, 3
        result = Test_AssertEqual&(3, stVar.forced, "forced should be set")
    END IF
    
    ' Execute again (forced should override)
    IF result THEN
        ExecuteRCStateVar stVar
        result = Test_AssertEqual&(3, stVar.actual, "actual should be forced value")
    END IF
    
    Test_End result
END SUB

' Run all state variable tests
SUB RunStateVarTests
    Test_ClearRCStateVar
    Test_SetRCStateVar
    Test_SetRCStateVarLocked
    Test_ForceRCStateVar
    Test_ExecuteRCStateVar
    Test_GetRCStateVar
    Test_StateVarLifecycle
END SUB
