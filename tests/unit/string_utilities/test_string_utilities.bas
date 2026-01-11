'
' Unit Tests for String Utilities (strings.bas)
'
' Tests string manipulation functions and config read/write operations.
' Uses component test harness for isolated testing.
'
' Note: test_framework_declarations.bi, test_framework_implementations.bas, test_state_manager_declarations.bi, and test_state_manager_implementations.bas are included by test_runner.bas
'$INCLUDE:'../../../source/utilities/strings.bas'
'$INCLUDE:'../../../source/utilities/ini-manager/ini.bm'

' Initialize test config file
' Note: ConfigFile$ must be declared SHARED to match cfg_global.bas
' ConfigFile$ is already declared in test_global_state_declarations.bi

SUB InitTestConfig
    ConfigFile$ = "test_config.ini"
    ' Ensure file exists and is clean
    IF _FILEEXISTS(ConfigFile$) THEN KILL ConfigFile$
END SUB

' Cleanup test config file
SUB CleanupTestConfig
    IF _FILEEXISTS(ConfigFile$) THEN KILL ConfigFile$
END SUB

SUB Test_StrRemove
    Test_Start "StrRemove$ - Case-insensitive string removal"
    
    DIM result AS LONG
    DIM testStr$, expected$, actual$
    
    ' Test basic removal
    testStr$ = "Hello World"
    actual$ = StrRemove$(testStr$, "World")
    expected$ = "Hello "
    result = Test_AssertEqualString&(expected$, actual$, "Should remove 'World'")
    
    ' Test case-insensitive removal
    IF result THEN
        testStr$ = "Hello World"
        actual$ = StrRemove$(testStr$, "world")
        expected$ = "Hello "
        result = Test_AssertEqualString&(expected$, actual$, "Should be case-insensitive")
    END IF
    
    ' Test multiple occurrences
    IF result THEN
        testStr$ = "test test test"
        actual$ = StrRemove$(testStr$, "test")
        expected$ = "  "
        result = Test_AssertEqualString&(expected$, actual$, "Should remove all occurrences")
    END IF
    
    ' Test removal of non-existent string
    IF result THEN
        testStr$ = "Hello World"
        actual$ = StrRemove$(testStr$, "xyz")
        expected$ = "Hello World"
        result = Test_AssertEqualString&(expected$, actual$, "Should leave unchanged when not found")
    END IF
    
    ' Test empty string
    IF result THEN
        testStr$ = ""
        actual$ = StrRemove$(testStr$, "test")
        expected$ = ""
        result = Test_AssertEqualString&(expected$, actual$, "Should handle empty string")
    END IF
    
    Test_End result
END SUB

SUB Test_StrReplace
    Test_Start "StrReplace$ - Case-insensitive string replacement"
    
    DIM result AS LONG
    DIM testStr$, expected$, actual$
    
    ' Test basic replacement
    testStr$ = "Hello World"
    actual$ = StrReplace$(testStr$, "World", "Universe")
    expected$ = "Hello Universe"
    result = Test_AssertEqualString&(expected$, actual$, "Should replace 'World' with 'Universe'")
    
    ' Test case-insensitive replacement
    IF result THEN
        testStr$ = "Hello World"
        actual$ = StrReplace$(testStr$, "world", "Universe")
        expected$ = "Hello Universe"
        result = Test_AssertEqualString&(expected$, actual$, "Should be case-insensitive")
    END IF
    
    ' Test multiple occurrences
    IF result THEN
        testStr$ = "test test test"
        actual$ = StrReplace$(testStr$, "test", "pass")
        expected$ = "pass pass pass"
        result = Test_AssertEqualString&(expected$, actual$, "Should replace all occurrences")
    END IF
    
    ' Test replacement with empty string
    IF result THEN
        testStr$ = "Hello World"
        actual$ = StrReplace$(testStr$, "World", "")
        expected$ = "Hello "
        result = Test_AssertEqualString&(expected$, actual$, "Should replace with empty string")
    END IF
    
    ' Test empty input string
    IF result THEN
        testStr$ = ""
        actual$ = StrReplace$(testStr$, "test", "pass")
        expected$ = ""
        result = Test_AssertEqualString&(expected$, actual$, "Should handle empty input")
    END IF
    
    ' Test non-existent string
    IF result THEN
        testStr$ = "Hello World"
        actual$ = StrReplace$(testStr$, "xyz", "abc")
        expected$ = "Hello World"
        result = Test_AssertEqualString&(expected$, actual$, "Should leave unchanged when not found")
    END IF
    
    Test_End result
END SUB

SUB Test_HasStringEnclosingPair
    Test_Start "HasStringEnclosingPair%% - Enclosing pair detection"
    
    DIM result AS LONG
    
    ' Test with single quotes
    result = Test_Assert&(HasStringEnclosingPair%%("'hello'", "''") <> 0, "Should detect single quotes")
    
    ' Test with double quotes
    IF result THEN
        result = Test_Assert&(HasStringEnclosingPair%%(CHR$(34) + "hello" + CHR$(34), CHR$(34) + CHR$(34)) <> 0, "Should detect double quotes")
    END IF
    
    ' Test with parentheses
    IF result THEN
        result = Test_Assert&(HasStringEnclosingPair%%("(hello)", "()") <> 0, "Should detect parentheses")
    END IF
    
    ' Test with brackets
    IF result THEN
        result = Test_Assert&(HasStringEnclosingPair%%("[hello]", "[]") <> 0, "Should detect brackets")
    END IF
    
    ' Test without enclosing pair
    IF result THEN
        result = Test_Assert&(HasStringEnclosingPair%%("hello", "''") = 0, "Should not detect when not enclosed")
    END IF
    
    ' Test with mismatched pair
    IF result THEN
        result = Test_Assert&(HasStringEnclosingPair%%("'hello)", "''") = 0, "Should not detect mismatched pair")
    END IF
    
    ' Test with too short string
    IF result THEN
        result = Test_Assert&(HasStringEnclosingPair%%("a", "''") = 0, "Should not detect when string too short")
    END IF
    
    Test_End result
END SUB

SUB Test_RemoveStringEnclosingPair
    Test_Start "RemoveStringEnclosingPair$ - Enclosing pair removal"
    
    DIM result AS LONG
    DIM testStr$, expected$, actual$
    
    ' Test with single quotes
    testStr$ = "'hello'"
    actual$ = RemoveStringEnclosingPair$(testStr$, "''")
    expected$ = "hello"
    result = Test_AssertEqualString&(expected$, actual$, "Should remove single quotes")
    
    ' Test with double quotes
    IF result THEN
        testStr$ = CHR$(34) + "hello" + CHR$(34)
        actual$ = RemoveStringEnclosingPair$(testStr$, CHR$(34) + CHR$(34))
        expected$ = "hello"
        result = Test_AssertEqualString&(expected$, actual$, "Should remove double quotes")
    END IF
    
    ' Test with parentheses
    IF result THEN
        testStr$ = "(hello)"
        actual$ = RemoveStringEnclosingPair$(testStr$, "()")
        expected$ = "hello"
        result = Test_AssertEqualString&(expected$, actual$, "Should remove parentheses")
    END IF
    
    ' Test without enclosing pair
    IF result THEN
        testStr$ = "hello"
        actual$ = RemoveStringEnclosingPair$(testStr$, "''")
        expected$ = "hello"
        result = Test_AssertEqualString&(expected$, actual$, "Should leave unchanged when not enclosed")
    END IF
    
    Test_End result
END SUB

SUB Test_AddQuotes
    Test_Start "AddQuotes$ - Quote wrapping"
    
    DIM result AS LONG
    DIM testStr$, expected$, actual$
    
    ' Test basic quote addition
    testStr$ = "hello"
    actual$ = AddQuotes$(testStr$)
    expected$ = CHR$(34) + "hello" + CHR$(34)
    result = Test_AssertEqualString&(expected$, actual$, "Should wrap with double quotes")
    
    ' Test with empty string
    IF result THEN
        testStr$ = ""
        actual$ = AddQuotes$(testStr$)
        expected$ = CHR$(34) + CHR$(34)
        result = Test_AssertEqualString&(expected$, actual$, "Should wrap empty string")
    END IF
    
    ' Test with string already containing quotes
    IF result THEN
        testStr$ = "hello" + CHR$(34) + "world"
        actual$ = AddQuotes$(testStr$)
        expected$ = CHR$(34) + "hello" + CHR$(34) + "world" + CHR$(34)
        result = Test_AssertEqualString&(expected$, actual$, "Should wrap even if quotes present")
    END IF
    
    Test_End result
END SUB

SUB Test_BoolToTFString
    Test_Start "BoolToTFString$ - Boolean to string conversion"
    
    DIM result AS LONG
    DIM expected$, actual$
    
    ' Test true value
    actual$ = BoolToTFString$(-1)
    expected$ = "True"
    result = Test_AssertEqualString&(expected$, actual$, "Should convert -1 to 'True'")
    
    ' Test false value
    IF result THEN
        actual$ = BoolToTFString$(0)
        expected$ = "False"
        result = Test_AssertEqualString&(expected$, actual$, "Should convert 0 to 'False'")
    END IF
    
    ' Test non-zero as true
    IF result THEN
        actual$ = BoolToTFString$(1)
        expected$ = "True"
        result = Test_AssertEqualString&(expected$, actual$, "Should convert non-zero to 'True'")
    END IF
    
    Test_End result
END SUB

SUB Test_TFStringToBool
    Test_Start "TFStringToBool% - String to boolean conversion"
    
    DIM result AS LONG
    DIM actual AS INTEGER
    
    ' Test "True"
    actual = TFStringToBool%("True")
    result = Test_AssertEqual&(-1, actual, "Should convert 'True' to -1")
    
    ' Test "true" (lowercase)
    IF result THEN
        actual = TFStringToBool%("true")
        result = Test_AssertEqual&(-1, actual, "Should be case-insensitive for 'true'")
    END IF
    
    ' Test "FALSE"
    IF result THEN
        actual = TFStringToBool%("FALSE")
        result = Test_AssertEqual&(0, actual, "Should convert 'FALSE' to 0")
    END IF
    
    ' Test "false" (lowercase)
    IF result THEN
        actual = TFStringToBool%("false")
        result = Test_AssertEqual&(0, actual, "Should be case-insensitive for 'false'")
    END IF
    
    ' Test invalid string
    IF result THEN
        actual = TFStringToBool%("invalid")
        result = Test_AssertEqual&(-2, actual, "Should return -2 for invalid string")
    END IF
    
    ' Test empty string
    IF result THEN
        actual = TFStringToBool%("")
        result = Test_AssertEqual&(-2, actual, "Should return -2 for empty string")
    END IF
    
    ' Test with whitespace (should be trimmed)
    IF result THEN
        actual = TFStringToBool%("  True  ")
        result = Test_AssertEqual&(-1, actual, "Should trim whitespace")
    END IF
    
    Test_End result
END SUB

SUB Test_ReadWriteBooleanSettingValue
    Test_Start "ReadWriteBooleanSettingValue% - Config boolean read/write"
    
    InitTestConfig
    DIM result AS LONG
    DIM actual AS INTEGER
    
    ' Test reading non-existent setting (should write default)
    actual = ReadWriteBooleanSettingValue%("TEST_SECTION", "TestBool", -1)
    result = Test_AssertEqual&(-1, actual, "Should return default for non-existent setting")
    
    ' Verify it was written
    IF result THEN
        DIM value$
        DIM readResult AS INTEGER
        readResult = ReadConfigSetting("TEST_SECTION", "TestBool", value$)
        result = Test_Assert&(readResult <> 0, "Setting should be written")
        IF result THEN result = Test_AssertEqualString&("True", value$, "Written value should be 'True'")
    END IF
    
    ' Test reading existing setting
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestBool2", "False"
        actual = ReadWriteBooleanSettingValue%("TEST_SECTION", "TestBool2", -1)
        result = Test_AssertEqual&(0, actual, "Should read existing 'False' value")
    END IF
    
    ' Test invalid value (should write default)
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestBool3", "Invalid"
        actual = ReadWriteBooleanSettingValue%("TEST_SECTION", "TestBool3", -1)
        result = Test_AssertEqual&(-1, actual, "Should return default for invalid value")
    END IF
    
    CleanupTestConfig
    Test_End result
END SUB

SUB Test_ReadWriteStringSettingValue
    Test_Start "ReadWriteStringSettingValue$ - Config string read/write"
    
    InitTestConfig
    DIM result AS LONG
    DIM actual$, expected$
    
    ' Test reading non-existent setting (should write default)
    actual$ = ReadWriteStringSettingValue$("TEST_SECTION", "TestString", "default")
    expected$ = "default"
    result = Test_AssertEqualString&(expected$, actual$, "Should return default for non-existent setting")
    
    ' Verify it was written
    IF result THEN
        DIM value$
        DIM readResult AS INTEGER
        readResult = ReadConfigSetting("TEST_SECTION", "TestString", value$)
        result = Test_Assert&(readResult <> 0, "Setting should be written")
        IF result THEN result = Test_AssertEqualString&("default", value$, "Written value should match default")
    END IF
    
    ' Test reading existing setting
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestString2", "existing"
        actual$ = ReadWriteStringSettingValue$("TEST_SECTION", "TestString2", "default")
        expected$ = "existing"
        result = Test_AssertEqualString&(expected$, actual$, "Should read existing value")
    END IF
    
    CleanupTestConfig
    Test_End result
END SUB

SUB Test_ReadWriteLongSettingValue
    Test_Start "ReadWriteLongSettingValue& - Config integer read/write"
    
    InitTestConfig
    DIM result AS LONG
    DIM actual AS LONG
    
    ' Test reading non-existent setting (should write default)
    actual = ReadWriteLongSettingValue&("TEST_SECTION", "TestLong", 42)
    result = Test_AssertEqual&(42, actual, "Should return default for non-existent setting")
    
    ' Test reading existing valid setting
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestLong2", "100"
        actual = ReadWriteLongSettingValue&("TEST_SECTION", "TestLong2", 42)
        result = Test_AssertEqual&(100, actual, "Should read existing valid value")
    END IF
    
    ' Test invalid value (zero or negative should use default)
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestLong3", "0"
        actual = ReadWriteLongSettingValue&("TEST_SECTION", "TestLong3", 42)
        result = Test_AssertEqual&(42, actual, "Should use default for zero value")
    END IF
    
    ' Test negative value (should use default)
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestLong4", "-10"
        actual = ReadWriteLongSettingValue&("TEST_SECTION", "TestLong4", 42)
        result = Test_AssertEqual&(42, actual, "Should use default for negative value")
    END IF
    
    ' Test non-numeric value (should use default)
    IF result THEN
        WriteConfigSetting "TEST_SECTION", "TestLong5", "invalid"
        actual = ReadWriteLongSettingValue&("TEST_SECTION", "TestLong5", 42)
        result = Test_AssertEqual&(42, actual, "Should use default for non-numeric value")
    END IF
    
    CleanupTestConfig
    Test_End result
END SUB

' Run all string utility tests
SUB RunStringUtilityTests
    Test_StrRemove
    Test_StrReplace
    Test_HasStringEnclosingPair
    Test_RemoveStringEnclosingPair
    Test_AddQuotes
    Test_BoolToTFString
    Test_TFStringToBool
    Test_ReadWriteBooleanSettingValue
    Test_ReadWriteStringSettingValue
    Test_ReadWriteLongSettingValue
END SUB
