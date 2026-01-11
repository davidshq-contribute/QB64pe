'
' Parser Utilities Test Suite
'
' Tests for parser utility functions in source/utilities/parser_utils.bas
'

' ============================================================================
' OPERATOR DETECTION TESTS
' ============================================================================

' Test _ORELSE operator (precedence level 1)
SUB Test_IsOperator_OrElse
    Test_Start "isoperator - _ORELSE operator"
    DIM result AS LONG
    result = Test_AssertEqual&(1, isoperator("_ORELSE"), "_ORELSE precedence")
    IF result THEN result = Test_AssertEqual&(1, isoperator("_orelse"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(1, isoperator("_OrElse"), "Mixed case")
    Test_End result
END SUB

' Test _ANDALSO operator (precedence level 2)
SUB Test_IsOperator_AndAlso
    Test_Start "isoperator - _ANDALSO operator"
    DIM result AS LONG
    result = Test_AssertEqual&(2, isoperator("_ANDALSO"), "_ANDALSO precedence")
    IF result THEN result = Test_AssertEqual&(2, isoperator("_andalso"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(2, isoperator("_AndAlso"), "Mixed case")
    Test_End result
END SUB

' Test IMP operator (precedence level 3)
SUB Test_IsOperator_Imp
    Test_Start "isoperator - IMP operator"
    DIM result AS LONG
    result = Test_AssertEqual&(3, isoperator("IMP"), "IMP precedence")
    IF result THEN result = Test_AssertEqual&(3, isoperator("imp"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(3, isoperator("Imp"), "Mixed case")
    Test_End result
END SUB

' Test EQV operator (precedence level 4)
SUB Test_IsOperator_Eqv
    Test_Start "isoperator - EQV operator"
    DIM result AS LONG
    result = Test_AssertEqual&(4, isoperator("EQV"), "EQV precedence")
    IF result THEN result = Test_AssertEqual&(4, isoperator("eqv"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(4, isoperator("Eqv"), "Mixed case")
    Test_End result
END SUB

' Test XOR operator (precedence level 5)
SUB Test_IsOperator_Xor
    Test_Start "isoperator - XOR operator"
    DIM result AS LONG
    result = Test_AssertEqual&(5, isoperator("XOR"), "XOR precedence")
    IF result THEN result = Test_AssertEqual&(5, isoperator("xor"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(5, isoperator("Xor"), "Mixed case")
    Test_End result
END SUB

' Test OR operator (precedence level 6)
SUB Test_IsOperator_Or
    Test_Start "isoperator - OR operator"
    DIM result AS LONG
    result = Test_AssertEqual&(6, isoperator("OR"), "OR precedence")
    IF result THEN result = Test_AssertEqual&(6, isoperator("or"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(6, isoperator("Or"), "Mixed case")
    Test_End result
END SUB

' Test AND operator (precedence level 7)
SUB Test_IsOperator_And
    Test_Start "isoperator - AND operator"
    DIM result AS LONG
    result = Test_AssertEqual&(7, isoperator("AND"), "AND precedence")
    IF result THEN result = Test_AssertEqual&(7, isoperator("and"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(7, isoperator("And"), "Mixed case")
    Test_End result
END SUB

' Test _NEGATE operator (precedence level 8)
SUB Test_IsOperator_Negate
    Test_Start "isoperator - _NEGATE operator"
    DIM result AS LONG
    result = Test_AssertEqual&(8, isoperator("_NEGATE"), "_NEGATE precedence")
    IF result THEN result = Test_AssertEqual&(8, isoperator("_negate"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(8, isoperator("_Negate"), "Mixed case")
    Test_End result
END SUB

' Test NOT operator (precedence level 9)
SUB Test_IsOperator_Not
    Test_Start "isoperator - NOT operator"
    DIM result AS LONG
    result = Test_AssertEqual&(9, isoperator("NOT"), "NOT precedence")
    IF result THEN result = Test_AssertEqual&(9, isoperator("not"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(9, isoperator("Not"), "Mixed case")
    Test_End result
END SUB

' Test comparison operators (precedence level 10)
SUB Test_IsOperator_ComparisonEqual
    Test_Start "isoperator - = operator"
    DIM result AS LONG
    result = Test_AssertEqual&(10, isoperator("="), "= precedence")
    Test_End result
END SUB

SUB Test_IsOperator_ComparisonGreater
    Test_Start "isoperator - > operator"
    DIM result AS LONG
    result = Test_AssertEqual&(10, isoperator(">"), "> precedence")
    Test_End result
END SUB

SUB Test_IsOperator_ComparisonLess
    Test_Start "isoperator - < operator"
    DIM result AS LONG
    result = Test_AssertEqual&(10, isoperator("<"), "< precedence")
    Test_End result
END SUB

SUB Test_IsOperator_ComparisonNotEqual
    Test_Start "isoperator - <> operator"
    DIM result AS LONG
    result = Test_AssertEqual&(10, isoperator("<>"), "<> precedence")
    Test_End result
END SUB

SUB Test_IsOperator_ComparisonLessEqual
    Test_Start "isoperator - <= operator"
    DIM result AS LONG
    result = Test_AssertEqual&(10, isoperator("<="), "<= precedence")
    Test_End result
END SUB

SUB Test_IsOperator_ComparisonGreaterEqual
    Test_Start "isoperator - >= operator"
    DIM result AS LONG
    result = Test_AssertEqual&(10, isoperator(">="), ">= precedence")
    Test_End result
END SUB

' Test addition and subtraction operators (precedence level 11)
SUB Test_IsOperator_Addition
    Test_Start "isoperator - + operator"
    DIM result AS LONG
    result = Test_AssertEqual&(11, isoperator("+"), "+ precedence")
    Test_End result
END SUB

SUB Test_IsOperator_Subtraction
    Test_Start "isoperator - - operator"
    DIM result AS LONG
    result = Test_AssertEqual&(11, isoperator("-"), "- precedence")
    Test_End result
END SUB

' Test MOD operator (precedence level 12)
SUB Test_IsOperator_Mod
    Test_Start "isoperator - MOD operator"
    DIM result AS LONG
    result = Test_AssertEqual&(12, isoperator("MOD"), "MOD precedence")
    IF result THEN result = Test_AssertEqual&(12, isoperator("mod"), "Case insensitive")
    IF result THEN result = Test_AssertEqual&(12, isoperator("Mod"), "Mixed case")
    Test_End result
END SUB

' Test integer division operator (precedence level 13)
SUB Test_IsOperator_IntegerDivision
    Test_Start "isoperator - \ operator"
    DIM result AS LONG
    result = Test_AssertEqual&(13, isoperator("\"), "\ precedence")
    Test_End result
END SUB

' Test multiplication and division operators (precedence level 14)
SUB Test_IsOperator_Multiplication
    Test_Start "isoperator - * operator"
    DIM result AS LONG
    result = Test_AssertEqual&(14, isoperator("*"), "* precedence")
    Test_End result
END SUB

SUB Test_IsOperator_Division
    Test_Start "isoperator - / operator"
    DIM result AS LONG
    result = Test_AssertEqual&(14, isoperator("/"), "/ precedence")
    Test_End result
END SUB

' Test special negation character (precedence level 15)
SUB Test_IsOperator_SpecialNegation
    Test_Start "isoperator - CHR$(241) negation"
    DIM result AS LONG
    result = Test_AssertEqual&(15, isoperator(CHR$(241)), "Special negation precedence")
    Test_End result
END SUB

' Test exponentiation operator (precedence level 16)
SUB Test_IsOperator_Exponentiation
    Test_Start "isoperator - ^ operator"
    DIM result AS LONG
    result = Test_AssertEqual&(16, isoperator("^"), "^ precedence")
    Test_End result
END SUB

' ============================================================================
' NON-OPERATOR TESTS
' ============================================================================

' Test that non-operators return 0
SUB Test_IsOperator_NonOperator_Variable
    Test_Start "isoperator - variable name returns 0"
    DIM result AS LONG
    result = Test_AssertEqual&(0, isoperator("x"), "Single letter variable")
    IF result THEN result = Test_AssertEqual&(0, isoperator("myVar"), "Multi-letter variable")
    IF result THEN result = Test_AssertEqual&(0, isoperator("VARIABLE"), "Uppercase variable")
    Test_End result
END SUB

SUB Test_IsOperator_NonOperator_Number
    Test_Start "isoperator - number returns 0"
    DIM result AS LONG
    result = Test_AssertEqual&(0, isoperator("123"), "Integer number")
    IF result THEN result = Test_AssertEqual&(0, isoperator("3.14"), "Floating point")
    IF result THEN result = Test_AssertEqual&(0, isoperator("0"), "Zero")
    Test_End result
END SUB

SUB Test_IsOperator_NonOperator_Keyword
    Test_Start "isoperator - keyword returns 0"
    DIM result AS LONG
    result = Test_AssertEqual&(0, isoperator("IF"), "IF keyword")
    IF result THEN result = Test_AssertEqual&(0, isoperator("THEN"), "THEN keyword")
    IF result THEN result = Test_AssertEqual&(0, isoperator("PRINT"), "PRINT keyword")
    IF result THEN result = Test_AssertEqual&(0, isoperator("DIM"), "DIM keyword")
    Test_End result
END SUB

SUB Test_IsOperator_NonOperator_EmptyString
    Test_Start "isoperator - empty string returns 0"
    DIM result AS LONG
    result = Test_AssertEqual&(0, isoperator(""), "Empty string")
    Test_End result
END SUB

SUB Test_IsOperator_NonOperator_Space
    Test_Start "isoperator - space returns 0"
    DIM result AS LONG
    result = Test_AssertEqual&(0, isoperator(" "), "Single space")
    IF result THEN result = Test_AssertEqual&(0, isoperator("  "), "Multiple spaces")
    Test_End result
END SUB

' ============================================================================
' PRECEDENCE LEVEL VERIFICATION
' ============================================================================

' Verify operator precedence ordering is correct
SUB Test_IsOperator_PrecedenceOrdering
    Test_Start "isoperator - precedence ordering"
    DIM result AS LONG

    ' Verify that higher precedence operators have higher return values
    result = Test_Assert&(isoperator("^") > isoperator("*"), "^ higher than *")
    IF result THEN result = Test_Assert&(isoperator("*") > isoperator("+"), "* higher than +")
    IF result THEN result = Test_Assert&(isoperator("+") > isoperator("="), "+ higher than =")
    IF result THEN result = Test_Assert&(isoperator("=") > isoperator("NOT"), "= higher than NOT")
    IF result THEN result = Test_Assert&(isoperator("NOT") > isoperator("AND"), "NOT higher than AND")
    IF result THEN result = Test_Assert&(isoperator("AND") > isoperator("OR"), "AND higher than OR")
    IF result THEN result = Test_Assert&(isoperator("OR") > isoperator("XOR"), "OR higher than XOR")

    Test_End result
END SUB

' ============================================================================
' EDGE CASES
' ============================================================================

' Test partial operator strings
SUB Test_IsOperator_PartialStrings
    Test_Start "isoperator - partial operator strings"
    DIM result AS LONG

    result = Test_AssertEqual&(0, isoperator("AN"), "Partial AND")
    IF result THEN result = Test_AssertEqual&(0, isoperator("O"), "Partial OR")
    IF result THEN result = Test_AssertEqual&(0, isoperator("NO"), "Partial NOT")
    IF result THEN result = Test_AssertEqual&(0, isoperator("MO"), "Partial MOD")

    Test_End result
END SUB

' Test operators with extra characters
SUB Test_IsOperator_ExtraCharacters
    Test_Start "isoperator - operators with extra chars"
    DIM result AS LONG

    result = Test_AssertEqual&(0, isoperator("AND1"), "AND with number")
    IF result THEN result = Test_AssertEqual&(0, isoperator("OR_"), "OR with underscore")
    IF result THEN result = Test_AssertEqual&(0, isoperator("NOT!"), "NOT with exclamation")

    Test_End result
END SUB

' ============================================================================
' TEST SUITE RUNNER
' ============================================================================

SUB RunParserUtilityTests
    ' Logical operators (low to high precedence)
    Test_IsOperator_OrElse
    Test_IsOperator_AndAlso
    Test_IsOperator_Imp
    Test_IsOperator_Eqv
    Test_IsOperator_Xor
    Test_IsOperator_Or
    Test_IsOperator_And
    Test_IsOperator_Negate
    Test_IsOperator_Not

    ' Comparison operators
    Test_IsOperator_ComparisonEqual
    Test_IsOperator_ComparisonGreater
    Test_IsOperator_ComparisonLess
    Test_IsOperator_ComparisonNotEqual
    Test_IsOperator_ComparisonLessEqual
    Test_IsOperator_ComparisonGreaterEqual

    ' Arithmetic operators
    Test_IsOperator_Addition
    Test_IsOperator_Subtraction
    Test_IsOperator_Mod
    Test_IsOperator_IntegerDivision
    Test_IsOperator_Multiplication
    Test_IsOperator_Division
    Test_IsOperator_SpecialNegation
    Test_IsOperator_Exponentiation

    ' Non-operators
    Test_IsOperator_NonOperator_Variable
    Test_IsOperator_NonOperator_Number
    Test_IsOperator_NonOperator_Keyword
    Test_IsOperator_NonOperator_EmptyString
    Test_IsOperator_NonOperator_Space

    ' Precedence verification
    Test_IsOperator_PrecedenceOrdering

    ' Edge cases
    Test_IsOperator_PartialStrings
    Test_IsOperator_ExtraCharacters
END SUB
