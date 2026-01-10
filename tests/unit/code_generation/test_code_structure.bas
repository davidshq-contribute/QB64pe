'
' Unit Tests for Code Structure Generation
'
' Tests code structure and formatting in generated C++ code.
' Uses component test harness for isolated testing.
'

'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'../test_state_manager.bi'
'$INCLUDE:'../test_output_verification.bi'
'$INCLUDE:'../test_output_verification.bas'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer.bi'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer.bm'

SUB Test_CodeStructureGeneration
    Test_Start "Code structure generation"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM codeLines$(1 TO 5)
    DIM readBack$
    DIM i AS LONG
    
    ' Test generating a structured code block
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        ' Generate a simple function structure
        codeLines$(1) = "void test_function() {"
        codeLines$(2) = "    int x = 5;"
        codeLines$(3) = "    int y = 10;"
        codeLines$(4) = "    return x + y;"
        codeLines$(5) = "}"
        
        FOR i = 1 TO 5
            WriteBufLine bufHandle, codeLines$(i)
        NEXT
        
        ' Verify structure
        SeekBuf bufHandle, 0, 0
        FOR i = 1 TO 5
            readBack$ = ReadBufLine$(bufHandle)
            result = Test_AssertEqualString&(codeLines$(i), readBack$, "Line " + _TOSTR$(i) + " should match")
            IF NOT result THEN EXIT FOR
        NEXT
        
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_BufferLength
    Test_Start "Buffer length tracking"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM initialLen AS LONG
    DIM afterWriteLen AS LONG
    
    ' Test that buffer length is tracked correctly
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        initialLen = GetBufLen&(bufHandle)
        result = Test_AssertEqual&(0, initialLen, "New buffer should have length 0")
        
        IF result THEN
            WriteBufLine bufHandle, "test line"
            afterWriteLen = GetBufLen&(bufHandle)
            result = Test_Assert&(afterWriteLen > initialLen, "Buffer length should increase after write")
        END IF
        
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_BufferPosition
    Test_Start "Buffer position tracking"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM pos AS LONG
    
    ' Test that buffer position is tracked correctly
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        ' Write multiple lines
        WriteBufLine bufHandle, "line 1"
        WriteBufLine bufHandle, "line 2"
        WriteBufLine bufHandle, "line 3"
        
        ' Position should be at end
        pos = GetBufPos&(bufHandle)
        result = Test_Assert&(pos > 0, "Position should be greater than 0 after writes")
        
        ' Seek to beginning
        IF result THEN
            SeekBuf bufHandle, 0, 0
            pos = GetBufPos&(bufHandle)
            result = Test_AssertEqual&(0, pos, "Position should be 0 after seeking to beginning")
        END IF
        
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_CodeStructureComparison
    Test_Start "Code structure comparison"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle1 AS INTEGER, bufHandle2 AS INTEGER
    DIM code1 AS CodeStructure, code2 AS CodeStructure
    DIM differences$ AS STRING
    
    ' Create two buffers with identical code
    bufHandle1 = CreateBuf%
    bufHandle2 = CreateBuf%
    result = Test_Assert&(bufHandle1 >= 0 AND bufHandle2 >= 0, "Should create buffers successfully")
    
    IF result THEN
        WriteBufLine bufHandle1, "int x = 5;"
        WriteBufLine bufHandle1, "int y = 10;"
        WriteBufLine bufHandle1, "return x + y;"
        
        WriteBufLine bufHandle2, "int x = 5;"
        WriteBufLine bufHandle2, "int y = 10;"
        WriteBufLine bufHandle2, "return x + y;"
        
        ' Extract code from both
        result = VerifyCode_ExtractFromBuffer&(bufHandle1, code1)
        IF result THEN result = VerifyCode_ExtractFromBuffer&(bufHandle2, code2)
        result = Test_Assert&(result, "Should extract code from both buffers")
        
        ' Compare structures
        IF result THEN
            result = VerifyCode_CompareStructures&(code1, code2, differences$)
            result = Test_Assert&(result, "Structures should be identical")
            IF NOT result AND differences$ <> "" THEN
                PRINT "    Differences: " + differences$
            END IF
        END IF
        
        VerifyCode_CleanupStructure code1
        VerifyCode_CleanupStructure code2
        DisposeBuf bufHandle1
        DisposeBuf bufHandle2
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all code structure tests
SUB RunCodeStructureTests
    Test_CodeStructureGeneration
    Test_BufferLength
    Test_BufferPosition
    Test_CodeStructureComparison
END SUB
