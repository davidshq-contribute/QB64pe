'
' Unit Tests for Error Handling (give_error.bas)
'
' Tests error state setting and propagation.
' Uses component test harness for isolated testing.
'
' Note: test_framework.bi and test_state_manager.bi are included by test_runner.bas
'$INCLUDE:'../../../source/utilities/give_error.bi'
'$INCLUDE:'../../../source/utilities/give_error.bas'

SUB Test_GiveErrorBasic
    Test_Start "Give_Error - Basic error setting"
    
    DIM result AS LONG
    
    ' Initialize error state
    Error_Happened = 0
    Error_Message = ""
    
    ' Set an error
    Give_Error "Test error message"
    
    ' Verify error state was set
    result = Test_Assert&(Error_Happened <> 0, "Error_Happened should be set")
    
    ' Verify error message was set
    IF result THEN
        result = Test_AssertEqualString&("Test error message", Error_Message, "Error_Message should be set correctly")
    END IF
    
    Test_End result
END SUB

SUB Test_GiveErrorMultiple
    Test_Start "Give_Error - Multiple error calls"
    
    DIM result AS LONG
    
    ' Initialize error state
    Error_Happened = 0
    Error_Message = ""
    
    ' Set first error
    Give_Error "First error"
    result = Test_Assert&(Error_Happened <> 0, "First error should set Error_Happened")
    IF result THEN result = Test_AssertEqualString&("First error", Error_Message, "First error message should be set")
    
    ' Set second error (should overwrite)
    IF result THEN
        Give_Error "Second error"
        result = Test_Assert&(Error_Happened <> 0, "Error_Happened should remain set")
        IF result THEN result = Test_AssertEqualString&("Second error", Error_Message, "Error message should be overwritten")
    END IF
    
    Test_End result
END SUB

SUB Test_GiveErrorEmptyMessage
    Test_Start "Give_Error - Empty error message"
    
    DIM result AS LONG
    
    ' Initialize error state
    Error_Happened = 0
    Error_Message = ""
    
    ' Set error with empty message
    Give_Error ""
    
    ' Error should still be set even with empty message
    result = Test_Assert&(Error_Happened <> 0, "Error_Happened should be set even with empty message")
    
    IF result THEN
        result = Test_AssertEqualString&("", Error_Message, "Error_Message should be empty")
    END IF
    
    Test_End result
END SUB

SUB Test_ErrorStatePersistence
    Test_Start "Give_Error - Error state persistence"
    
    DIM result AS LONG
    
    ' Initialize error state
    Error_Happened = 0
    Error_Message = ""
    
    ' Set an error
    Give_Error "Persistent error"
    result = Test_Assert&(Error_Happened <> 0, "Error should be set")
    
    ' Error should persist until explicitly cleared
    IF result THEN
        ' Simulate some operations that don't clear error
        DIM dummy AS LONG
        dummy = 1 + 1
        result = Test_Assert&(Error_Happened <> 0, "Error should persist after operations")
    END IF
    
    IF result THEN
        result = Test_AssertEqualString&("Persistent error", Error_Message, "Error message should persist")
    END IF
    
    Test_End result
END SUB

' Run all error handling tests
SUB RunErrorHandlingTests
    Test_GiveErrorBasic
    Test_GiveErrorMultiple
    Test_GiveErrorEmptyMessage
    Test_ErrorStatePersistence
END SUB
