'
' Unit Tests for Symbol Table (hash.bas)
'
' Tests symbol insertion, lookup, and scope resolution.
' Uses component test harness for isolated testing.
'

'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'../test_state_manager.bi'
'$INCLUDE:'../test_output_verification.bi'
'$INCLUDE:'../test_output_verification.bas'
'$INCLUDE:'../../source/utilities/hash.bi'
'$INCLUDE:'../../source/utilities/hash.bas'

SUB Test_SymbolInsertion
    Test_Start "Symbol insertion"
    
    DIM context AS TestStateContext
    TestState_Init context, "hash"
    
    DIM result AS LONG
    
    ' Test that symbols can be inserted into the hash table
    HashAdd "testVar", HASHFLAG_VARIABLE, 1
    result = Test_Assert&(HashListNext > 1, "HashAdd should add symbol to table")
    
    ' Verify the symbol was added correctly
    IF result THEN
        DIM flags AS LONG, ref AS LONG
        DIM found AS LONG
        found = HashFind("testVar", HASHFLAG_VARIABLE, flags, ref)
        result = Test_Assert&(found > 0, "Inserted symbol should be findable")
        IF result THEN result = Test_AssertEqual&(HASHFLAG_VARIABLE, flags, "Symbol should have correct flags")
        IF result THEN result = Test_AssertEqual&(1, ref, "Symbol should have correct reference")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_SymbolLookup
    Test_Start "Symbol lookup"
    
    DIM context AS TestStateContext
    TestState_Init context, "hash"
    
    DIM result AS LONG
    
    ' Add multiple symbols
    HashAdd "var1", HASHFLAG_VARIABLE, 1
    HashAdd "var2", HASHFLAG_VARIABLE, 2
    HashAdd "func1", HASHFLAG_FUNCTION, 10
    
    ' Test that inserted symbols can be found
    DIM flags AS LONG, ref AS LONG
    DIM found AS LONG
    
    found = HashFind("var1", HASHFLAG_VARIABLE, flags, ref)
    result = Test_Assert&(found > 0, "var1 should be findable")
    IF result THEN result = Test_AssertEqual&(1, ref, "var1 should have correct reference")
    
    IF result THEN
        found = HashFind("var2", HASHFLAG_VARIABLE, flags, ref)
        result = Test_Assert&(found > 0, "var2 should be findable")
        IF result THEN result = Test_AssertEqual&(2, ref, "var2 should have correct reference")
    END IF
    
    IF result THEN
        found = HashFind("func1", HASHFLAG_FUNCTION, flags, ref)
        result = Test_Assert&(found > 0, "func1 should be findable")
        IF result THEN result = Test_AssertEqual&(10, ref, "func1 should have correct reference")
    END IF
    
    ' Test that non-existent symbols are not found
    IF result THEN
        found = HashFind("nonexistent", HASHFLAG_VARIABLE, flags, ref)
        result = Test_Assert&(found = 0, "Non-existent symbol should not be found")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_SymbolScope
    Test_Start "Symbol scope resolution"
    
    DIM context AS TestStateContext
    TestState_Init context, "hash"
    
    DIM result AS LONG
    
    ' Test that symbols with different flags can coexist
    ' Add same name with different flags
    HashAdd "test", HASHFLAG_VARIABLE, 1
    HashAdd "test", HASHFLAG_FUNCTION, 2
    
    DIM flags AS LONG, ref AS LONG
    DIM found AS LONG
    
    ' Find variable version
    found = HashFind("test", HASHFLAG_VARIABLE, flags, ref)
    result = Test_Assert&(found > 0, "Variable version should be findable")
    IF result THEN result = Test_AssertEqual&(1, ref, "Variable should have correct reference")
    
    ' Find function version
    IF result THEN
        found = HashFind("test", HASHFLAG_FUNCTION, flags, ref)
        result = Test_Assert&(found > 0, "Function version should be findable")
        IF result THEN result = Test_AssertEqual&(2, ref, "Function should have correct reference")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_HashCollisions
    Test_Start "Hash collision handling"
    
    DIM context AS TestStateContext
    TestState_Init context, "hash"
    
    DIM result AS LONG
    
    ' Test that hash collisions are handled correctly
    ' Insert multiple symbols - the hash table should handle collisions via chaining
    HashAdd "a", HASHFLAG_VARIABLE, 1
    HashAdd "b", HASHFLAG_VARIABLE, 2
    HashAdd "c", HASHFLAG_VARIABLE, 3
    HashAdd "d", HASHFLAG_VARIABLE, 4
    HashAdd "e", HASHFLAG_VARIABLE, 5
    
    ' Verify all symbols can be found
    DIM flags AS LONG, ref AS LONG
    DIM found AS LONG
    DIM i AS LONG
    DIM symbols$(1 TO 5)
    symbols$(1) = "a": symbols$(2) = "b": symbols$(3) = "c": symbols$(4) = "d": symbols$(5) = "e"
    
    FOR i = 1 TO 5
        found = HashFind(symbols$(i), HASHFLAG_VARIABLE, flags, ref)
        result = Test_Assert&(found > 0, "Symbol " + symbols$(i) + " should be findable")
        IF result THEN result = Test_AssertEqual&(i, ref, "Symbol " + symbols$(i) + " should have correct reference")
        IF NOT result THEN EXIT FOR
    NEXT
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_SymbolTableVerification
    Test_Start "Symbol table verification"
    
    DIM context AS TestStateContext
    TestState_Init context, "hash"
    
    DIM result AS LONG
    DIM snapshot AS SymbolTableSnapshot
    DIM expectedSymbols$(1 TO 5) AS STRING
    DIM count AS LONG
    
    ' Add multiple symbols
    HashAdd "var1", HASHFLAG_VARIABLE, 1
    HashAdd "var2", HASHFLAG_VARIABLE, 2
    HashAdd "func1", HASHFLAG_FUNCTION, 10
    HashAdd "sub1", HASHFLAG_SUB, 20
    HashAdd "const1", HASHFLAG_CONSTANT, 30
    
    ' Verify symbol count
    count = VerifySymbolTable_GetCount&()
    result = Test_AssertEqual&(5, count, "Should have 5 symbols")
    
    ' Verify counts by flag
    IF result THEN
        count = VerifySymbolTable_GetCountByFlag&(HASHFLAG_VARIABLE)
        result = Test_AssertEqual&(2, count, "Should have 2 variables")
    END IF
    
    IF result THEN
        count = VerifySymbolTable_GetCountByFlag&(HASHFLAG_FUNCTION)
        result = Test_AssertEqual&(1, count, "Should have 1 function")
    END IF
    
    IF result THEN
        count = VerifySymbolTable_GetCountByFlag&(HASHFLAG_SUB)
        result = Test_AssertEqual&(1, count, "Should have 1 sub")
    END IF
    
    IF result THEN
        count = VerifySymbolTable_GetCountByFlag&(HASHFLAG_CONSTANT)
        result = Test_AssertEqual&(1, count, "Should have 1 constant")
    END IF
    
    ' Verify exact symbols
    IF result THEN
        expectedSymbols$(1) = "VAR1"
        expectedSymbols$(2) = "VAR2"
        expectedSymbols$(3) = "FUNC1"
        expectedSymbols$(4) = "SUB1"
        expectedSymbols$(5) = "CONST1"
        result = VerifySymbolTable_VerifyExactSymbols&(expectedSymbols$(), 5)
        result = Test_Assert&(result, "Should contain exactly the expected symbols")
    END IF
    
    ' Verify symbol properties
    IF result THEN
        result = VerifySymbolTable_VerifySymbol&("var1", HASHFLAG_VARIABLE, 1)
        result = Test_Assert&(result, "var1 should have correct properties")
    END IF
    
    IF result THEN
        result = VerifySymbolTable_VerifySymbol&("func1", HASHFLAG_FUNCTION, 10)
        result = Test_Assert&(result, "func1 should have correct properties")
    END IF
    
    ' Test snapshot functionality
    IF result THEN
        VerifySymbolTable_InitSnapshot snapshot
        result = VerifySymbolTable_Enumerate&(snapshot)
        result = Test_Assert&(result, "Should enumerate symbols successfully")
        
        IF result THEN
            result = Test_AssertEqual&(5, snapshot.symbolCount, "Snapshot should have 5 symbols")
        END IF
        
        IF result THEN
            result = Test_AssertEqual&(2, snapshot.variableCount, "Snapshot should have 2 variables")
        END IF
        
        IF result THEN
            result = Test_AssertEqual&(1, snapshot.functionCount, "Snapshot should have 1 function")
        END IF
        
        VerifySymbolTable_CleanupSnapshot snapshot
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all symbol table tests
SUB RunSymbolTableTests
    Test_SymbolInsertion
    Test_SymbolLookup
    Test_SymbolScope
    Test_HashCollisions
    Test_SymbolTableVerification
END SUB
