'
' Unit Tests for Code Generation
'
' Tests C++ code emission functionality.
' Uses component test harness for isolated testing.
'
' Note: test_framework.bi, test_state_manager.bi, and test_output_verification.bi are included by test_runner.bas
'$INCLUDE:'../test_output_verification.bas'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer.bi'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer.bm'

SUB Test_WriteBufLine
    Test_Start "WriteBufLine function"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM testText$
    DIM readBack$
    
    ' Test that WriteBufLine writes to buffer correctly
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        ' Write a line to the buffer
        testText$ = "int x = 5;"
        WriteBufLine bufHandle, testText$
        
        ' Read it back
        SeekBuf bufHandle, 0, 0 ' Seek to beginning
        readBack$ = ReadBufLine$(bufHandle)
        result = Test_AssertEqualString&(testText$, readBack$, "Written text should match read text")
        
        ' Clean up
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_WriteBufLineMultiple
    Test_Start "WriteBufLine multiple lines"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM line1$, line2$, line3$
    DIM readBack$
    
    ' Test writing multiple lines
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        line1$ = "int x = 5;"
        line2$ = "int y = 10;"
        line3$ = "return x + y;"
        
        WriteBufLine bufHandle, line1$
        WriteBufLine bufHandle, line2$
        WriteBufLine bufHandle, line3$
        
        ' Read back all lines
        SeekBuf bufHandle, 0, 0
        readBack$ = ReadBufLine$(bufHandle)
        result = Test_AssertEqualString&(line1$, readBack$, "First line should match")
        
        IF result THEN
            readBack$ = ReadBufLine$(bufHandle)
            result = Test_AssertEqualString&(line2$, readBack$, "Second line should match")
        END IF
        
        IF result THEN
            readBack$ = ReadBufLine$(bufHandle)
            result = Test_AssertEqualString&(line3$, readBack$, "Third line should match")
        END IF
        
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_WriteBufLineEmpty
    Test_Start "WriteBufLine empty string"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM readBack$
    
    ' Test writing empty string
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        WriteBufLine bufHandle, ""
        
        SeekBuf bufHandle, 0, 0
        readBack$ = ReadBufLine$(bufHandle)
        result = Test_AssertEqualString&("", readBack$, "Empty string should be written correctly")
        
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_WriteBufLineLong
    Test_Start "WriteBufLine long line"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM longLine$
    DIM readBack$
    DIM i AS LONG
    
    ' Test writing a long line
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        ' Create a long line (1000 characters)
        longLine$ = ""
        FOR i = 1 TO 1000
            longLine$ = longLine$ + "x"
        NEXT
        
        WriteBufLine bufHandle, longLine$
        
        SeekBuf bufHandle, 0, 0
        readBack$ = ReadBufLine$(bufHandle)
        result = Test_AssertEqualString&(longLine$, readBack$, "Long line should be written correctly")
        
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_GeneratedCodeVerification
    Test_Start "Generated code verification"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM bufHandle AS INTEGER
    DIM code AS CodeStructure
    DIM expectedLines$(1 TO 4) AS STRING
    
    ' Create buffer and write code
    bufHandle = CreateBuf%
    result = Test_Assert&(bufHandle >= 0, "Should create buffer successfully")
    
    IF result THEN
        expectedLines$(1) = "int main() {"
        expectedLines$(2) = "    int x = 5;"
        expectedLines$(3) = "    return x;"
        expectedLines$(4) = "}"
        
        WriteBufLine bufHandle, expectedLines$(1)
        WriteBufLine bufHandle, expectedLines$(2)
        WriteBufLine bufHandle, expectedLines$(3)
        WriteBufLine bufHandle, expectedLines$(4)
        
        ' Extract and verify code
        result = VerifyCode_ExtractFromBuffer&(bufHandle, code)
        result = Test_Assert&(result, "Should extract code from buffer")
        
        ' Verify structure
        IF result THEN
            result = Test_AssertEqual&(4, code.totalLines, "Should have 4 lines")
        END IF
        
        ' Verify content
        IF result THEN
            result = VerifyCode_Contains&(code, "int main()")
            result = Test_Assert&(result, "Should contain 'int main()'")
        END IF
        
        IF result THEN
            result = VerifyCode_ContainsLine&(code, "int main() {")
            result = Test_Assert&(result, "Should contain exact line 'int main() {'")
        END IF
        
        ' Verify structure counts
        IF result THEN
            result = VerifyCode_VerifyStructure&(code, 0, 4)
            result = Test_Assert&(result, "Should have correct structure")
        END IF
        
        VerifyCode_CleanupStructure code
        DisposeBuf bufHandle
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all code emission tests
SUB RunCodeEmissionTests
    Test_WriteBufLine
    Test_WriteBufLineMultiple
    Test_WriteBufLineEmpty
    Test_WriteBufLineLong
    Test_GeneratedCodeVerification
END SUB
