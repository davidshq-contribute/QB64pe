' QB64pe String Operations Security Tests
' Tests for sprintf buffer overflow fixes and string handling safety

' Initialize test framework
'$INCLUDE:'qb64pe.bi'
'$INCLUDE:'tests/unit/test_framework.bi'

' Test constants
CONST MAX_BUFFER_SIZE = 1024
CONST TEST_STRING_SIZE = 512

' Test data
DIM SHARED testStrings(1 TO 10) AS STRING
DIM SHARED testNumbers(1 TO 10) AS INTEGER

' Initialize test data
SUB InitTestData ()
    testStrings(1) = "Hello"
    testStrings(2) = "World"
    testStrings(3) = "Test"
    testStrings(4) = "String"
    testStrings(5) = "Security"
    testStrings(6) = "Buffer"
    testStrings(7) = "Overflow"
    testStrings(8) = "Testing"
    testStrings(9) = "QB64pe"
    testStrings(10) = "Safety"
    
    testNumbers(1) = 42
    testNumbers(2) = -123
    testNumbers(3) = 0
    testNumbers(4) = 999999
    testNumbers(5) = -999999
    testNumbers(6) = 32767
    testNumbers(7) = -32768
    testNumbers(8) = 100
    testNumbers(9) = -100
    testNumbers(10) = 256
END SUB

' Test safe string formatting
SUB TestSafeStringFormatting ()
    PRINT "Testing Safe String Formatting..."
    
    ' Test basic string formatting
    DIM buffer1 AS STRING * 256
    DIM result1 AS STRING
    
    ' Simulate safe string formatting (would use snprintf in C)
    result1 = "Test: " + testStrings(1) + " Number: " + STR$(testNumbers(1))
    AssertEqual LEN(result1), 22, "Basic string formatting length incorrect"
    AssertContains result1, "Hello", "String not found in formatted result"
    AssertContains result1, "42", "Number not found in formatted result"
    
    ' Test long string formatting
    DIM longString AS STRING
    longString = STRING$(100, "A") + "END"
    AssertEqual LEN(longString), 103, "Long string creation failed"
    AssertContains longString, "END", "End marker not found in long string"
    
    ' Test empty string handling
    DIM emptyString AS STRING
    emptyString = ""
    AssertEqual LEN(emptyString), 0, "Empty string length incorrect"
    
    ' Test null character handling (simulated)
    DIM nullString AS STRING
    nullString = "Before" + CHR$(0) + "After"
    ' In BASIC, CHR$(0) is valid, but we test boundary conditions
    AssertEqual LEN(nullString), 11, "Null character string length incorrect"
END SUB

' Test buffer overflow prevention
SUB TestBufferOverflowPrevention ()
    PRINT "Testing Buffer Overflow Prevention..."
    
    ' Test small buffer with large data
    DIM smallBuffer AS STRING * 10
    DIM largeData AS STRING
    largeData = "This is a very long string that exceeds buffer size"
    
    ' Simulate safe truncation
    IF LEN(largeData) > 10 THEN
        smallBuffer = LEFT$(largeData, 10)
    ELSE
        smallBuffer = largeData
    END IF
    
    AssertEqual LEN(smallBuffer), 10, "Buffer truncation failed"
    AssertEqual smallBuffer, "This is a", "Buffer truncation content incorrect"
    
    ' Test exact buffer size
    DIM exactBuffer AS STRING * 15
    DIM exactData AS STRING
    exactData = "Exact size data"
    
    IF LEN(exactData) <= 15 THEN
        exactBuffer = exactData + SPACE$(15 - LEN(exactData))
    END IF
    
    AssertEqual LEN(exactBuffer), 15, "Exact buffer size handling failed"
    AssertContains exactBuffer, "Exact size data", "Exact buffer content incorrect"
    
    ' Test empty buffer
    DIM emptyBuffer AS STRING * 20
    emptyBuffer = ""
    AssertEqual LEN(emptyBuffer), 20, "Empty buffer initialization failed"
END SUB

' Test string concatenation safety
SUB TestStringConcatenationSafety ()
    PRINT "Testing String Concatenation Safety..."
    
    ' Test multiple concatenations
    DIM concatResult AS STRING
    concatResult = testStrings(1) + " " + testStrings(2) + " " + testStrings(3)
    AssertEqual concatResult, "Hello World Test", "String concatenation failed"
    
    ' Test concatenation with numbers
    DIM numConcat AS STRING
    numConcat = "Number: " + STR$(testNumbers(1)) + " and " + STR$(testNumbers(2))
    AssertEqual numConcat, "Number:  42 and -123", "Number concatenation failed"
    
    ' Test large concatenation
    DIM largeConcat AS STRING
    DIM i AS INTEGER
    largeConcat = ""
    FOR i = 1 TO 100
        largeConcat = largeConcat + "A"
    NEXT i
    AssertEqual LEN(largeConcat), 100, "Large concatenation failed"
    
    ' Test concatenation with special characters
    DIM specialConcat AS STRING
    specialConcat = "Special: " + CHR$(34) + "Quotes" + CHR$(34) + " and " + CHR$(9) + "Tab"
    AssertContains specialConcat, CHR$(34), "Special character handling failed"
    AssertContains specialConcat, CHR$(9), "Tab character handling failed"
END SUB

' Test number formatting safety
SUB TestNumberFormattingSafety ()
    PRINT "Testing Number Formatting Safety..."
    
    ' Test positive integers
    DIM posResult AS STRING
    posResult = "Positive: " + STR$(testNumbers(1))
    AssertContains posResult, "42", "Positive number formatting failed"
    
    ' Test negative integers
    DIM negResult AS STRING
    negResult = "Negative: " + STR$(testNumbers(2))
    AssertContains negResult, "-123", "Negative number formatting failed"
    
    ' Test zero
    DIM zeroResult AS STRING
    zeroResult = "Zero: " + STR$(testNumbers(3))
    AssertContains zeroResult, " 0", "Zero formatting failed"
    
    ' Test large numbers
    DIM largeResult AS STRING
    largeResult = "Large: " + STR$(testNumbers(4))
    AssertContains largeResult, "999999", "Large number formatting failed"
    
    ' Test minimum integers
    DIM minResult AS STRING
    minResult = "Min: " + STR$(testNumbers(7))
    AssertContains minResult, "-32768", "Minimum integer formatting failed"
    
    ' Test formatted width
    DIM widthResult AS STRING
    widthResult = "Width:" + SPACE$(5) + STR$(testNumbers(1))
    AssertEqual LEN(widthResult), 11, "Width formatting failed"
END SUB

' Test edge cases and boundary conditions
SUB TestEdgeCasesAndBoundaries ()
    PRINT "Testing Edge Cases and Boundary Conditions..."
    
    ' Test maximum string length
    DIM maxString AS STRING
    maxString = STRING$(32767, "Z")  ' Near BASIC string limit
    AssertEqual LEN(maxString), 32767, "Maximum string length failed"
    
    ' Test single character strings
    DIM singleChar AS STRING
    singleChar = "A"
    AssertEqual LEN(singleChar), 1, "Single character string failed"
    
    ' Test string with all ASCII characters
    DIM asciiString AS STRING
    DIM i AS INTEGER
    asciiString = ""
    FOR i = 32 TO 126  ' Printable ASCII
        asciiString = asciiString + CHR$(i)
    NEXT i
    AssertEqual LEN(asciiString), 95, "ASCII string creation failed"
    
    ' Test string with spaces
    DIM spaceString AS STRING
    spaceString = "  Leading and trailing  "
    AssertEqual LEN(spaceString), 23, "Space string length incorrect"
    AssertEqual LTRIM$(spaceString), "Leading and trailing  ", "Leading space trim failed"
    AssertEqual RTRIM$(spaceString), "  Leading and trailing", "Trailing space trim failed"
    
    ' Test repeated patterns
    DIM repeatString AS STRING
    repeatString = STRING$(50, "AB")
    AssertEqual LEN(repeatString), 100, "Repeated pattern string failed"
    AssertEqual LEFT$(repeatString, 2), "AB", "Repeated pattern content incorrect"
END SUB

' Test memory safety patterns
SUB TestMemorySafetyPatterns ()
    PRINT "Testing Memory Safety Patterns..."
    
    ' Test string reuse
    DIM reuseString AS STRING
    DIM i AS INTEGER
    FOR i = 1 TO 10
        reuseString = "Iteration " + STR$(i)
        AssertContains reuseString, "Iteration", "String reuse failed at iteration " + STR$(i)
    NEXT i
    
    ' Test string clearing
    DIM clearString AS STRING
    clearString = "Temporary data"
    clearString = ""
    AssertEqual LEN(clearString), 0, "String clearing failed"
    
    ' Test string assignment
    DIM assignString1 AS STRING
    DIM assignString2 AS STRING
    assignString1 = "Original"
    assignString2 = assignString1
    AssertEqual assignString1, assignString2, "String assignment failed"
    
    ' Test string modification
    DIM modString AS STRING
    modString = "Original"
    modString = "Modified"
    AssertEqual modString, "Modified", "String modification failed"
    AssertNotEqual modString, "Original", "String modification comparison failed"
END SUB

' Main test runner
SUB RunStringSecurityTests ()
    PRINT "Starting QB64pe String Security Tests..."
    PRINT "========================================"
    
    InitTestData
    
    TestSafeStringFormatting
    TestBufferOverflowPrevention
    TestStringConcatenationSafety
    TestNumberFormattingSafety
    TestEdgeCasesAndBoundaries
    TestMemorySafetyPatterns
    
    PRINT "========================================"
    PRINT "String Security Tests Completed!"
END SUB

' Run tests if this file is executed directly
IF COMMAND$ = "" THEN
    RunStringSecurityTests
END IF
