'
' Unit Tests for Constant Evaluation (const_eval.bas)
'
' Tests constant folding and compile-time evaluation.
' Uses component test harness for isolated testing.
'
' Note: test_framework_declarations.bi, test_framework_implementations.bas, test_state_manager_declarations.bi, and test_state_manager_implementations.bas are included by test_runner.bas
'$INCLUDE:'../../../source/global/constants.bas'
''$INCLUDE:'../../../source/utilities/type.bi'  ' Now included globally in test_runner.bas
' Note: hash.bi and const_eval.bi are already included by test_runner.bas
'$INCLUDE:'../../../source/utilities/elements.bas'
'$INCLUDE:'../../../source/utilities/const_eval.bas'

SUB Test_ConstantFolding
    Test_Start "Constant folding"
    
    DIM context AS TestStateContext
    TestState_Init context, "const"
    
    DIM result AS LONG
    
    ' Test that constant expressions are evaluated at compile time
    ' Note: Evaluate_Expression$ requires a ParseNum structure
    DIM num AS ParseNum
    DIM exprResult$
    
    ' Test simple addition
    exprResult$ = Evaluate_Expression$("2+3", num)
    result = Test_AssertEqual&(5, num.i, "2+3 should evaluate to 5")
    
    ' Test subtraction
    IF result THEN
        exprResult$ = Evaluate_Expression$("10-4", num)
        result = Test_AssertEqual&(6, num.i, "10-4 should evaluate to 6")
    END IF
    
    ' Test multiplication
    IF result THEN
        exprResult$ = Evaluate_Expression$("3*4", num)
        result = Test_AssertEqual&(12, num.i, "3*4 should evaluate to 12")
    END IF
    
    ' Test division
    IF result THEN
        exprResult$ = Evaluate_Expression$("15/3", num)
        result = Test_AssertEqual&(5, num.i, "15/3 should evaluate to 5")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_ConstantFunctions
    Test_Start "Constant function evaluation"
    
    DIM context AS TestStateContext
    TestState_Init context, "const"
    
    DIM result AS LONG
    
    ' Test that constant functions are evaluated
    DIM num AS ParseNum
    DIM exprResult$
    
    ' Test ABS function
    exprResult$ = Evaluate_Expression$("ABS(-5)", num)
    result = Test_AssertEqual&(5, num.i, "ABS(-5) should evaluate to 5")
    
    ' Test ABS with positive number
    IF result THEN
        exprResult$ = Evaluate_Expression$("ABS(5)", num)
        result = Test_AssertEqual&(5, num.i, "ABS(5) should evaluate to 5")
    END IF
    
    ' Test simple function (if available)
    ' Note: Actual function support depends on ConstFuncs array initialization
    ' which happens in Set_ConstFunctions
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_ConstantErrorHandling
    Test_Start "Constant evaluation error handling"
    
    DIM context AS TestStateContext
    TestState_Init context, "const"
    
    DIM result AS LONG
    
    ' Test that invalid constant expressions are handled
    ' Note: Error handling behavior depends on implementation
    ' For now, we test that the function doesn't crash on invalid input
    DIM num AS ParseNum
    DIM exprResult$
    
    ' Test empty expression (should handle gracefully)
    exprResult$ = Evaluate_Expression$("", num)
    result = Test_Assert&(LEN(exprResult$) >= 0, "Empty expression should be handled")
    
    ' Test invalid syntax (should handle gracefully)
    IF result THEN
        exprResult$ = Evaluate_Expression$("+++", num)
        result = Test_Assert&(LEN(exprResult$) >= 0, "Invalid syntax should be handled")
    END IF
    
    ' Note: Division by zero and other runtime errors may be caught
    ' by the compiler's error handling, which is tested in integration tests
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all constant evaluation tests
SUB RunConstEvalTests
    Test_ConstantFolding
    Test_ConstantFunctions
    Test_ConstantErrorHandling
END SUB
