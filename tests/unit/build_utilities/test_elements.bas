'
' Unit Tests for Element Manipulation Utilities (elements.bas)
'
' Tests element parsing and manipulation functions that use sp (CHR$(13)) delimiter.
' These are pure functions with no side effects, ideal for comprehensive testing.
'
' Note: test_framework_declarations.bi is included by test_runner.bas
'$INCLUDE:'../../../source/utilities/elements.bas'

' ======================================================================
' CORE ELEMENT FUNCTIONS - Basic Operations
' ======================================================================

SUB Test_GetElement_FirstElement
    Test_Start "getelement$ - first element"
    DIM testString AS STRING
    testString = "alpha" + sp + "beta" + sp + "gamma"

    DIM result AS LONG
    result = Test_AssertEqualString&("alpha", getelement$(testString, 1), "First element")
    Test_End result
END SUB

SUB Test_GetElement_MiddleElement
    Test_Start "getelement$ - middle element"
    DIM testString AS STRING
    testString = "alpha" + sp + "beta" + sp + "gamma"

    DIM result AS LONG
    result = Test_AssertEqualString&("beta", getelement$(testString, 2), "Middle element")
    Test_End result
END SUB

SUB Test_GetElement_LastElement
    Test_Start "getelement$ - last element"
    DIM testString AS STRING
    testString = "alpha" + sp + "beta" + sp + "gamma"

    DIM result AS LONG
    result = Test_AssertEqualString&("gamma", getelement$(testString, 3), "Last element")
    Test_End result
END SUB

SUB Test_GetElement_OutOfBounds
    Test_Start "getelement$ - out of bounds"
    DIM testString AS STRING
    testString = "alpha" + sp + "beta"

    DIM result AS LONG
    result = Test_AssertEqualString&("", getelement$(testString, 5), "Out of bounds returns empty")
    Test_End result
END SUB

SUB Test_GetElement_EmptyString
    Test_Start "getelement$ - empty string"
    DIM result AS LONG
    result = Test_AssertEqualString&("", getelement$("", 1), "Empty string returns empty")
    Test_End result
END SUB

SUB Test_NumElements_EmptyString
    Test_Start "numelements - empty string"
    DIM result AS LONG
    result = Test_AssertEqual&(0, numelements(""), "Empty string has 0 elements")
    Test_End result
END SUB

SUB Test_NumElements_SingleElement
    Test_Start "numelements - single element"
    DIM result AS LONG
    result = Test_AssertEqual&(1, numelements("single"), "Single element")
    Test_End result
END SUB

SUB Test_NumElements_MultipleElements
    Test_Start "numelements - multiple elements"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three" + sp + "four"

    DIM result AS LONG
    result = Test_AssertEqual&(4, numelements(testString), "Four elements")
    Test_End result
END SUB

SUB Test_PushElement_ToEmpty
    Test_Start "pushelement - to empty string"
    DIM testString AS STRING
    testString = ""
    pushelement testString, "first"

    DIM result AS LONG
    result = Test_AssertEqualString&("first", testString, "Push to empty")
    Test_End result
END SUB

SUB Test_PushElement_ToPopulated
    Test_Start "pushelement - to populated string"
    DIM testString AS STRING
    testString = "first"
    pushelement testString, "second"

    DIM result AS LONG
    result = Test_AssertEqualString&("first" + sp + "second", testString, "Push to populated")
    Test_End result
END SUB

SUB Test_PushElement_Multiple
    Test_Start "pushelement - multiple pushes"
    DIM testString AS STRING
    testString = ""
    pushelement testString, "one"
    pushelement testString, "two"
    pushelement testString, "three"

    DIM result AS LONG
    result = Test_AssertEqual&(3, numelements(testString), "Three elements after pushes")
    IF result THEN result = Test_AssertEqualString&("one", getelement$(testString, 1), "First element correct")
    IF result THEN result = Test_AssertEqualString&("two", getelement$(testString, 2), "Second element correct")
    IF result THEN result = Test_AssertEqualString&("three", getelement$(testString, 3), "Third element correct")
    Test_End result
END SUB

SUB Test_GetElements_ValidRange
    Test_Start "getelements$ - valid range"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three" + sp + "four" + sp + "five"

    DIM result AS LONG
    result = Test_AssertEqualString&("two" + sp + "three", getelements$(testString, 2, 3), "Elements 2-3")
    Test_End result
END SUB

SUB Test_GetElements_ReverseRange
    Test_Start "getelements$ - reverse range (i2 < i1)"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three"

    DIM result AS LONG
    result = Test_AssertEqualString&("", getelements$(testString, 3, 1), "Reverse range returns empty")
    Test_End result
END SUB

SUB Test_GetElements_FullRange
    Test_Start "getelements$ - full range"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three"

    DIM result AS LONG
    result = Test_AssertEqualString&(testString, getelements$(testString, 1, 3), "Full range")
    Test_End result
END SUB

SUB Test_GetElements_SingleElement
    Test_Start "getelements$ - single element range"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three"

    DIM result AS LONG
    result = Test_AssertEqualString&("two", getelements$(testString, 2, 2), "Single element range")
    Test_End result
END SUB

' ======================================================================
' ITERATOR FUNCTIONS - Forward and Backward
' ======================================================================

SUB Test_GetNextElement_FullIteration
    Test_Start "getnextelement$ - full iteration"
    DIM testString AS STRING
    testString = "alpha" + sp + "beta" + sp + "gamma"

    DIM index AS LONG
    DIM strIndex AS LONG
    DIM elem AS STRING
    DIM result AS LONG

    index = 0
    strIndex = 0

    ' First element
    elem = getnextelement$(testString, index, strIndex)
    result = Test_AssertEqualString&("alpha", elem, "First iteration")
    IF result THEN result = Test_AssertEqual&(1, index, "Index after first")

    ' Second element
    IF result THEN
        elem = getnextelement$(testString, index, strIndex)
        result = Test_AssertEqualString&("beta", elem, "Second iteration")
        IF result THEN result = Test_AssertEqual&(2, index, "Index after second")
    END IF

    ' Third element
    IF result THEN
        elem = getnextelement$(testString, index, strIndex)
        result = Test_AssertEqualString&("gamma", elem, "Third iteration")
        IF result THEN result = Test_AssertEqual&(3, index, "Index after third")
    END IF

    ' Past end
    IF result THEN
        elem = getnextelement$(testString, index, strIndex)
        result = Test_AssertEqual&(-1, index, "Index is -1 after last element")
    END IF

    Test_End result
END SUB

SUB Test_GetNextElement_EmptyString
    Test_Start "getnextelement$ - empty string"
    DIM index AS LONG
    DIM strIndex AS LONG

    index = 0
    strIndex = 0

    DIM elem AS STRING
    elem = getnextelement$("", index, strIndex)

    DIM result AS LONG
    result = Test_AssertEqual&(-1, index, "Empty string sets index to -1")
    Test_End result
END SUB

SUB Test_GetPrevElement_FullReverseIteration
    Test_Start "getprevelement$ - full reverse iteration"
    DIM testString AS STRING
    testString = "alpha" + sp + "beta" + sp + "gamma"

    DIM index AS LONG
    DIM strIndex AS LONG
    DIM elem AS STRING
    DIM result AS LONG

    index = 0
    strIndex = 0

    ' Last element
    elem = getprevelement$(testString, index, strIndex)
    result = Test_AssertEqualString&("gamma", elem, "First reverse iteration (last element)")
    IF result THEN result = Test_AssertEqual&(3, index, "Index after first reverse")

    ' Middle element
    IF result THEN
        elem = getprevelement$(testString, index, strIndex)
        result = Test_AssertEqualString&("beta", elem, "Second reverse iteration")
        IF result THEN result = Test_AssertEqual&(2, index, "Index after second reverse")
    END IF

    ' First element
    IF result THEN
        elem = getprevelement$(testString, index, strIndex)
        result = Test_AssertEqualString&("alpha", elem, "Third reverse iteration (first element)")
        IF result THEN result = Test_AssertEqual&(1, index, "Index after third reverse")
    END IF

    ' Past beginning
    IF result THEN
        elem = getprevelement$(testString, index, strIndex)
        result = Test_AssertEqual&(-1, index, "Index is -1 after first element")
    END IF

    Test_End result
END SUB

' ======================================================================
' ADVANCED ELEMENT FUNCTIONS - Insert, Remove, Extract
' ======================================================================

SUB Test_InsertElements_AtBeginning
    Test_Start "insertelements - at beginning"
    DIM testString AS STRING
    testString = "two" + sp + "three"
    insertelements testString, 0, "zero" + sp + "one"

    DIM result AS LONG
    result = Test_AssertEqual&(4, numelements(testString), "Four elements after insert")
    IF result THEN result = Test_AssertEqualString&("zero", getelement$(testString, 1), "First element")
    IF result THEN result = Test_AssertEqualString&("one", getelement$(testString, 2), "Second element")
    IF result THEN result = Test_AssertEqualString&("two", getelement$(testString, 3), "Third element")
    Test_End result
END SUB

SUB Test_InsertElements_InMiddle
    Test_Start "insertelements - in middle"
    DIM testString AS STRING
    testString = "one" + sp + "four"
    insertelements testString, 1, "two" + sp + "three"

    DIM result AS LONG
    result = Test_AssertEqual&(4, numelements(testString), "Four elements after insert")
    IF result THEN result = Test_AssertEqualString&("one", getelement$(testString, 1), "First element")
    IF result THEN result = Test_AssertEqualString&("two", getelement$(testString, 2), "Second element")
    IF result THEN result = Test_AssertEqualString&("three", getelement$(testString, 3), "Third element")
    IF result THEN result = Test_AssertEqualString&("four", getelement$(testString, 4), "Fourth element")
    Test_End result
END SUB

SUB Test_RemoveElements_Range
    Test_Start "removeelements - range without keepindexing"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three" + sp + "four" + sp + "five"
    removeelements testString, 2, 4, 0

    DIM result AS LONG
    result = Test_AssertEqual&(2, numelements(testString), "Two elements remain")
    IF result THEN result = Test_AssertEqualString&("one", getelement$(testString, 1), "First element")
    IF result THEN result = Test_AssertEqualString&("five", getelement$(testString, 2), "Second element")
    Test_End result
END SUB

SUB Test_RemoveElement_Single
    Test_Start "removeelement - single element"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three"
    removeelement testString, 2

    DIM result AS LONG
    result = Test_AssertEqual&(2, numelements(testString), "Two elements remain")
    IF result THEN result = Test_AssertEqualString&("one", getelement$(testString, 1), "First element")
    IF result THEN result = Test_AssertEqualString&("three", getelement$(testString, 2), "Second element")
    Test_End result
END SUB

SUB Test_GetElementsBefore
    Test_Start "getelementsbefore$ - get elements before position"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three" + sp + "four"

    DIM result AS LONG
    result = Test_AssertEqualString&("one" + sp + "two", getelementsbefore$(testString, 2), "Elements 1-2")
    Test_End result
END SUB

SUB Test_GetElementsAfter
    Test_Start "getelementsafter$ - get elements after position"
    DIM testString AS STRING
    testString = "one" + sp + "two" + sp + "three" + sp + "four"

    DIM result AS LONG
    result = Test_AssertEqualString&("three" + sp + "four", getelementsafter$(testString, 3), "Elements 3-4")
    Test_End result
END SUB

' ======================================================================
' FUNCTION ARGUMENT PARSING
' ======================================================================

SUB Test_CountFunctionElements_Empty
    Test_Start "countFunctionElements - empty string"
    DIM result AS LONG
    result = Test_AssertEqual&(0, countFunctionElements(""), "Empty has 0 arguments")
    Test_End result
END SUB

SUB Test_CountFunctionElements_Single
    Test_Start "countFunctionElements - single argument"
    DIM testString AS STRING
    testString = "arg1"

    DIM result AS LONG
    result = Test_AssertEqual&(1, countFunctionElements(testString), "Single argument")
    Test_End result
END SUB

SUB Test_CountFunctionElements_Multiple
    Test_Start "countFunctionElements - multiple arguments"
    DIM testString AS STRING
    ' Simulate: func(x, y, z) where elements are: ( x , y , z )
    testString = "(" + sp + "x" + sp + "," + sp + "y" + sp + "," + sp + "z" + sp + ")"

    DIM result AS LONG
    result = Test_AssertEqual&(3, countFunctionElements(testString), "Three arguments")
    Test_End result
END SUB

SUB Test_HasFunctionElement_Present
    Test_Start "hasFunctionElement - element present"
    DIM testString AS STRING
    ' Simulate: func(x, y, z)
    testString = "(" + sp + "x" + sp + "," + sp + "y" + sp + "," + sp + "z" + sp + ")"

    DIM result AS LONG
    result = Test_Assert&(hasFunctionElement(testString, 1) <> 0, "First argument present")
    IF result THEN result = Test_Assert&(hasFunctionElement(testString, 2) <> 0, "Second argument present")
    IF result THEN result = Test_Assert&(hasFunctionElement(testString, 3) <> 0, "Third argument present")
    Test_End result
END SUB

SUB Test_HasFunctionElement_Missing
    Test_Start "hasFunctionElement - element missing"
    DIM testString AS STRING
    ' Simulate: func(x, , z) - second argument omitted
    testString = "(" + sp + "x" + sp + "," + sp + "," + sp + "z" + sp + ")"

    DIM result AS LONG
    result = Test_Assert&(hasFunctionElement(testString, 2) = 0, "Second argument missing")
    Test_End result
END SUB

' ======================================================================
' ELEMENT TYPE DETECTION AND PARSING
' ======================================================================

SUB Test_ElementIsNumber_Integer
    Test_Start "elementIsNumber& - integer"
    DIM result AS LONG
    result = Test_Assert&(elementIsNumber&("42") <> 0, "Positive integer is number")
    IF result THEN result = Test_Assert&(elementIsNumber&("-42") <> 0, "Negative integer is number")
    IF result THEN result = Test_Assert&(elementIsNumber&("0") <> 0, "Zero is number")
    Test_End result
END SUB

SUB Test_ElementIsNumber_Decimal
    Test_Start "elementIsNumber& - decimal"
    DIM result AS LONG
    result = Test_Assert&(elementIsNumber&("3.14") <> 0, "Decimal is number")
    IF result THEN result = Test_Assert&(elementIsNumber&(".5") <> 0, "Leading dot is number")
    IF result THEN result = Test_Assert&(elementIsNumber&("-0.5") <> 0, "Negative decimal is number")
    Test_End result
END SUB

SUB Test_ElementIsNumber_NotNumber
    Test_Start "elementIsNumber& - not a number"
    DIM result AS LONG
    result = Test_Assert&(elementIsNumber&("abc") = 0, "Letters not number")
    IF result THEN result = Test_Assert&(elementIsNumber&("") = 0, "Empty not number")
    Test_End result
END SUB

SUB Test_ElementIsString_Quoted
    Test_Start "elementIsString& - quoted string"
    DIM testString AS STRING
    testString = CHR$(34) + "hello" + CHR$(34)

    DIM result AS LONG
    result = Test_Assert&(elementIsString&(testString) <> 0, "Quoted string detected")
    Test_End result
END SUB

SUB Test_ElementIsString_Unquoted
    Test_Start "elementIsString& - unquoted"
    DIM result AS LONG
    result = Test_Assert&(elementIsString&("hello") = 0, "Unquoted not string element")
    Test_End result
END SUB

' ======================================================================
' STRING ESCAPING AND ELEMENT CREATION
' ======================================================================

SUB Test_EscapeString_NoEscape
    Test_Start "escapeString$ - no special characters"
    DIM result AS LONG
    result = Test_AssertEqualString&("hello", escapeString$("hello"), "No escaping needed")
    Test_End result
END SUB

SUB Test_EscapeString_Backslash
    Test_Start "escapeString$ - backslash"
    DIM testInput AS STRING
    testInput = "path\to\file"

    DIM testOutput AS STRING
    testOutput = escapeString$(testInput)

    DIM result AS LONG
    ' Backslash should be escaped as \\
    result = Test_Assert&(INSTR(testOutput, "\\") > 0, "Backslash escaped")
    Test_End result
END SUB

SUB Test_CreateElementString_Basic
    Test_Start "createElementString$ - basic string"
    DIM testOutput AS STRING
    testOutput = createElementString$("test")

    DIM result AS LONG
    ' Should be: "test",4
    result = Test_Assert&(INSTR(testOutput, CHR$(34)) > 0, "Contains quotes")
    IF result THEN result = Test_Assert&(INSTR(testOutput, ",4") > 0, "Contains length")
    Test_End result
END SUB

SUB Test_EleUcase_WithoutQuotes
    Test_Start "eleucase$ - without quotes"
    DIM testString AS STRING
    testString = "hello" + sp + "world"

    DIM result AS LONG
    result = Test_AssertEqualString&("HELLO" + sp + "WORLD", eleucase$(testString), "All uppercase")
    Test_End result
END SUB

SUB Test_EleUcase_PreservesQuoted
    Test_Start "eleucase$ - preserves quoted strings"
    DIM testString AS STRING
    testString = "PRINT" + sp + CHR$(34) + "Hello World" + CHR$(34)

    DIM testOutput AS STRING
    testOutput = eleucase$(testString)

    DIM result AS LONG
    ' PRINT should be uppercase, but "Hello World" should remain as-is
    result = Test_Assert&(INSTR(testOutput, "PRINT") > 0, "Keyword uppercase")
    IF result THEN result = Test_Assert&(INSTR(testOutput, "Hello World") > 0, "Quoted string preserved")
    Test_End result
END SUB

' ======================================================================
' TEST RUNNER - Run all element tests
' ======================================================================

SUB RunElementTests
    ' Core element functions
    Test_GetElement_FirstElement
    Test_GetElement_MiddleElement
    Test_GetElement_LastElement
    Test_GetElement_OutOfBounds
    Test_GetElement_EmptyString
    Test_NumElements_EmptyString
    Test_NumElements_SingleElement
    Test_NumElements_MultipleElements
    Test_PushElement_ToEmpty
    Test_PushElement_ToPopulated
    Test_PushElement_Multiple
    Test_GetElements_ValidRange
    Test_GetElements_ReverseRange
    Test_GetElements_FullRange
    Test_GetElements_SingleElement

    ' Iterator functions
    Test_GetNextElement_FullIteration
    Test_GetNextElement_EmptyString
    Test_GetPrevElement_FullReverseIteration

    ' Advanced functions
    Test_InsertElements_AtBeginning
    Test_InsertElements_InMiddle
    Test_RemoveElements_Range
    Test_RemoveElement_Single
    Test_GetElementsBefore
    Test_GetElementsAfter

    ' Function argument parsing
    Test_CountFunctionElements_Empty
    Test_CountFunctionElements_Single
    Test_CountFunctionElements_Multiple
    Test_HasFunctionElement_Present
    Test_HasFunctionElement_Missing

    ' Element type detection
    Test_ElementIsNumber_Integer
    Test_ElementIsNumber_Decimal
    Test_ElementIsNumber_NotNumber
    Test_ElementIsString_Quoted
    Test_ElementIsString_Unquoted

    ' String escaping
    Test_EscapeString_NoEscape
    Test_EscapeString_Backslash
    Test_CreateElementString_Basic
    Test_EleUcase_WithoutQuotes
    Test_EleUcase_PreservesQuoted
END SUB
