'
' Unit Tests for Type System (type.bas)
'
' Tests type inference, conversions, and checking functionality.
' Uses component test harness for isolated testing.
'
' Note: test_framework.bi and test_state_manager.bi are included by test_runner.bas
'$INCLUDE:'../../../source/utilities/type.bi'
'$INCLUDE:'../../../source/utilities/type.bas'

SUB Test_TypeSymbolConversion
    Test_Start "Type symbol conversion"
    
    DIM context AS TestStateContext
    TestState_Init context, "type"
    
    DIM result AS LONG
    
    ' Test basic type symbol conversions
    result = Test_AssertEqualString&("$", typevalue2symbol$(STRINGTYPE), "String type should return $")
    IF result THEN result = Test_AssertEqualString&("!", typevalue2symbol$(SINGLETYPE), "SINGLE type should return !")
    IF result THEN result = Test_AssertEqualString&("#", typevalue2symbol$(DOUBLETYPE), "DOUBLE type should return #")
    IF result THEN result = Test_AssertEqualString&("&", typevalue2symbol$(LONGTYPE), "LONG type should return &")
    IF result THEN result = Test_AssertEqualString&("%", typevalue2symbol$(INTEGERTYPE), "INTEGER type should return %")
    IF result THEN result = Test_AssertEqualString&("%%", typevalue2symbol$(BYTETYPE), "BYTE type should return %%")
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_TypeNameConversion
    Test_Start "Type name to symbol conversion"
    
    DIM context AS TestStateContext
    TestState_Init context, "type"
    
    DIM result AS LONG
    
    ' Test type2symbol$ function
    result = Test_AssertEqualString&("!", type2symbol$("SINGLE"), "SINGLE should convert to !")
    IF result THEN result = Test_AssertEqualString&("#", type2symbol$("DOUBLE"), "DOUBLE should convert to #")
    IF result THEN result = Test_AssertEqualString&("&", type2symbol$("LONG"), "LONG should convert to &")
    IF result THEN result = Test_AssertEqualString&("%", type2symbol$("INTEGER"), "INTEGER should convert to %")
    IF result THEN result = Test_AssertEqualString&("$", type2symbol$("STRING"), "STRING should convert to $")
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_TypeSizeFunctions
    Test_Start "Type size functions"
    
    DIM context AS TestStateContext
    TestState_Init context, "type"
    
    DIM result AS LONG
    
    ' Test Type_GetSizeInBits function
    result = Test_AssertEqual&(32, Type_GetSizeInBits~&(LONGTYPE), "LONG should be 32 bits")
    IF result THEN result = Test_AssertEqual&(64, Type_GetSizeInBits~&(DOUBLETYPE), "DOUBLE should be 64 bits")
    IF result THEN result = Test_AssertEqual&(16, Type_GetSizeInBits~&(INTEGERTYPE), "INTEGER should be 16 bits")
    IF result THEN result = Test_AssertEqual&(8, Type_GetSizeInBits~&(BYTETYPE), "BYTE should be 8 bits")
    IF result THEN result = Test_AssertEqual&(32, Type_GetSizeInBits~&(SINGLETYPE), "SINGLE should be 32 bits")
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_TypeFlags
    Test_Start "Type flag checking"
    
    DIM context AS TestStateContext
    TestState_Init context, "type"
    
    DIM result AS LONG
    
    ' Test Type_IsString function
    result = Test_Assert&(Type_IsString%%(STRINGTYPE), "STRINGTYPE should be identified as string")
    IF result THEN result = Test_Assert&(NOT Type_IsString%%(LONGTYPE), "LONGTYPE should not be identified as string")
    
    ' Test Type_IsFloatingPoint function
    IF result THEN result = Test_Assert&(Type_IsFloatingPoint%%(SINGLETYPE), "SINGLETYPE should be floating point")
    IF result THEN result = Test_Assert&(Type_IsFloatingPoint%%(DOUBLETYPE), "DOUBLETYPE should be floating point")
    IF result THEN result = Test_Assert&(NOT Type_IsFloatingPoint%%(LONGTYPE), "LONGTYPE should not be floating point")
    
    ' Test Type_IsUnsigned function
    IF result THEN result = Test_Assert&(Type_IsUnsigned%%(ULONGTYPE), "ULONGTYPE should be unsigned")
    IF result THEN result = Test_Assert&(NOT Type_IsUnsigned%%(LONGTYPE), "LONGTYPE should not be unsigned")
    
    ' Test Type_IsIntegral function
    IF result THEN result = Test_Assert&(Type_IsIntegral%%(LONGTYPE), "LONGTYPE should be integral")
    IF result THEN result = Test_Assert&(NOT Type_IsIntegral%%(SINGLETYPE), "SINGLETYPE should not be integral")
    
    TestState_Cleanup context
    Test_End result
END SUB

SUB Test_TypeConversions
    Test_Start "Type conversion functions"
    
    DIM context AS TestStateContext
    TestState_Init context, "type"
    
    DIM result AS LONG
    
    ' Test typ2ctyp$ function for C++ type conversion
    result = Test_AssertEqualString&("int32", typ2ctyp$(LONGTYPE, ""), "LONG should convert to int32")
    IF result THEN result = Test_AssertEqualString&("float", typ2ctyp$(SINGLETYPE, ""), "SINGLE should convert to float")
    IF result THEN result = Test_AssertEqualString&("double", typ2ctyp$(DOUBLETYPE, ""), "DOUBLE should convert to double")
    IF result THEN result = Test_AssertEqualString&("qbs", typ2ctyp$(STRINGTYPE, ""), "STRING should convert to qbs")
    
    ' Test typname2typ& function
    IF result THEN result = Test_AssertEqual&(LONGTYPE, typname2typ&("LONG"), "typname2typ should convert LONG name to LONGTYPE")
    IF result THEN result = Test_AssertEqual&(SINGLETYPE, typname2typ&("SINGLE"), "typname2typ should convert SINGLE name to SINGLETYPE")
    IF result THEN result = Test_AssertEqual&(STRINGTYPE, typname2typ&("STRING"), "typname2typ should convert STRING name to STRINGTYPE")
    
    TestState_Cleanup context
    Test_End result
END SUB

' Run all type system tests
SUB RunTypeSystemTests
    Test_TypeSymbolConversion
    Test_TypeNameConversion
    Test_TypeSizeFunctions
    Test_TypeFlags
    Test_TypeConversions
END SUB
