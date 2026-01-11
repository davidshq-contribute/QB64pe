'
' Unit Tests for Type Conversion Functions (type.bas)
'
' Tests pure type conversion functions: typevalue2symbol$, id2fulltypename$,
' id2shorttypename$, and symbol2fulltypename$.
' These are pure functions with no side effects, perfect for unit testing.
'
' Note: test_framework_declarations.bi is included by test_runner.bas
' Note: Type constants (STRINGTYPE, etc.) are initialized in type_init.bas
' Note: type.bas is now included globally in test_runner.bas

' ======================================================================
' TYPE VALUE TO SYMBOL CONVERSION
' ======================================================================

SUB Test_TypeValue2Symbol_String
    Test_Start "typevalue2symbol$ - STRING type"
    DIM result AS LONG
    result = Test_AssertEqualString&("$", typevalue2symbol$(STRINGTYPE), "STRING → $")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Integer
    Test_Start "typevalue2symbol$ - INTEGER type"
    DIM result AS LONG
    result = Test_AssertEqualString&("%", typevalue2symbol$(INTEGERTYPE), "INTEGER → %")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Long
    Test_Start "typevalue2symbol$ - LONG type"
    DIM result AS LONG
    result = Test_AssertEqualString&("&", typevalue2symbol$(LONGTYPE), "LONG → &")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Integer64
    Test_Start "typevalue2symbol$ - INTEGER64 type"
    DIM result AS LONG
    result = Test_AssertEqualString&("&&", typevalue2symbol$(INTEGER64TYPE), "_INTEGER64 → &&")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Byte
    Test_Start "typevalue2symbol$ - BYTE type"
    DIM result AS LONG
    result = Test_AssertEqualString&("%%", typevalue2symbol$(BYTETYPE), "_BYTE → %%")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Single
    Test_Start "typevalue2symbol$ - SINGLE type"
    DIM result AS LONG
    result = Test_AssertEqualString&("!", typevalue2symbol$(SINGLETYPE), "SINGLE → !")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Double
    Test_Start "typevalue2symbol$ - DOUBLE type"
    DIM result AS LONG
    result = Test_AssertEqualString&("#", typevalue2symbol$(DOUBLETYPE), "DOUBLE → #")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Float
    Test_Start "typevalue2symbol$ - FLOAT type"
    DIM result AS LONG
    result = Test_AssertEqualString&("##", typevalue2symbol$(FLOATTYPE), "_FLOAT → ##")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Unsigned
    Test_Start "typevalue2symbol$ - unsigned types"
    DIM result AS LONG
    result = Test_AssertEqualString&("~%%", typevalue2symbol$(UBYTETYPE), "_UNSIGNED _BYTE → ~%%")
    IF result THEN result = Test_AssertEqualString&("~%", typevalue2symbol$(UINTEGERTYPE), "_UNSIGNED INTEGER → ~%")
    IF result THEN result = Test_AssertEqualString&("~&", typevalue2symbol$(ULONGTYPE), "_UNSIGNED LONG → ~&")
    IF result THEN result = Test_AssertEqualString&("~&&", typevalue2symbol$(UINTEGER64TYPE), "_UNSIGNED _INTEGER64 → ~&&")
    Test_End result
END SUB

SUB Test_TypeValue2Symbol_Bit
    Test_Start "typevalue2symbol$ - _BIT types"
    DIM result AS LONG
    ' _BIT (1 bit)
    result = Test_AssertEqualString&("`", typevalue2symbol$(BITTYPE), "_BIT → `")
    ' _UNSIGNED _BIT (1 bit)
    IF result THEN result = Test_AssertEqualString&("~`", typevalue2symbol$(UBITTYPE), "_UNSIGNED _BIT → ~`")
    ' _BIT*8 (8 bits)
    IF result THEN result = Test_AssertEqualString&("`8", typevalue2symbol$(8 + ISPOINTER + ISOFFSETINBITS), "_BIT*8 → `8")
    ' _UNSIGNED _BIT*16 (16 bits)
    IF result THEN result = Test_AssertEqualString&("~`16", typevalue2symbol$(16 + ISPOINTER + ISUNSIGNED + ISOFFSETINBITS), "_UNSIGNED _BIT*16 → ~`16")
    Test_End result
END SUB

' ======================================================================
' SYMBOL TO FULL TYPE NAME CONVERSION
' ======================================================================

SUB Test_Symbol2FullTypeName_Basic
    Test_Start "symbol2fulltypename$ - basic symbols"
    DIM result AS LONG
    result = Test_AssertEqualString&("_BYTE", symbol2fulltypename$("%%"), "%% → _BYTE")
    IF result THEN result = Test_AssertEqualString&("INTEGER", symbol2fulltypename$("%"), "% → INTEGER")
    IF result THEN result = Test_AssertEqualString&("LONG", symbol2fulltypename$("&"), "& → LONG")
    IF result THEN result = Test_AssertEqualString&("_INTEGER64", symbol2fulltypename$("&&"), "&& → _INTEGER64")
    IF result THEN result = Test_AssertEqualString&("SINGLE", symbol2fulltypename$("!"), "! → SINGLE")
    IF result THEN result = Test_AssertEqualString&("DOUBLE", symbol2fulltypename$("#"), "# → DOUBLE")
    IF result THEN result = Test_AssertEqualString&("_FLOAT", symbol2fulltypename$("##"), "## → _FLOAT")
    Test_End result
END SUB

SUB Test_Symbol2FullTypeName_Unsigned
    Test_Start "symbol2fulltypename$ - unsigned symbols"
    DIM result AS LONG
    result = Test_AssertEqualString&("_UNSIGNED _BYTE", symbol2fulltypename$("~%%"), "~%% → _UNSIGNED _BYTE")
    IF result THEN result = Test_AssertEqualString&("_UNSIGNED INTEGER", symbol2fulltypename$("~%"), "~% → _UNSIGNED INTEGER")
    IF result THEN result = Test_AssertEqualString&("_UNSIGNED LONG", symbol2fulltypename$("~&"), "~& → _UNSIGNED LONG")
    IF result THEN result = Test_AssertEqualString&("_UNSIGNED _INTEGER64", symbol2fulltypename$("~&&"), "~&& → _UNSIGNED _INTEGER64")
    Test_End result
END SUB

SUB Test_Symbol2FullTypeName_BitTypes
    Test_Start "symbol2fulltypename$ - _BIT symbols"
    DIM result AS LONG
    result = Test_AssertEqualString&("_BIT * 1", symbol2fulltypename$("`"), "` → _BIT * 1")
    IF result THEN result = Test_AssertEqualString&("_UNSIGNED _BIT * 1", symbol2fulltypename$("~`"), "~` → _UNSIGNED _BIT * 1")
    IF result THEN result = Test_AssertEqualString&("_BIT * 8", symbol2fulltypename$("`8"), "`8 → _BIT * 8")
    IF result THEN result = Test_AssertEqualString&("_UNSIGNED _BIT * 16", symbol2fulltypename$("~`16"), "~`16 → _UNSIGNED _BIT * 16")
    Test_End result
END SUB

SUB Test_Symbol2FullTypeName_Offset
    Test_Start "symbol2fulltypename$ - _OFFSET symbol"
    DIM result AS LONG
    result = Test_AssertEqualString&("_OFFSET", symbol2fulltypename$("%&"), "%& → _OFFSET")
    IF result THEN result = Test_AssertEqualString&("_UNSIGNED _OFFSET", symbol2fulltypename$("~%&"), "~%& → _UNSIGNED _OFFSET")
    Test_End result
END SUB

' ======================================================================
' ID TO FULL TYPE NAME CONVERSION
' Note: id2fulltypename$ and id2shorttypename$ use a global 'id' variable
' ======================================================================

SUB Test_ID2FullTypeName_Basic
    Test_Start "id2fulltypename$ - basic types"
    DIM result AS LONG

    ' STRING
    id.t = STRINGTYPE
    result = Test_AssertEqualString&("STRING", id2fulltypename$, "STRING type")

    ' INTEGER
    IF result THEN
        id.t = INTEGERTYPE
        result = Test_AssertEqualString&("INTEGER", id2fulltypename$, "INTEGER type")
    END IF

    ' LONG
    IF result THEN
        id.t = LONGTYPE
        result = Test_AssertEqualString&("LONG", id2fulltypename$, "LONG type")
    END IF

    ' _INTEGER64
    IF result THEN
        id.t = INTEGER64TYPE
        result = Test_AssertEqualString&("_INTEGER64", id2fulltypename$, "_INTEGER64 type")
    END IF

    Test_End result
END SUB

SUB Test_ID2FullTypeName_Float
    Test_Start "id2fulltypename$ - floating point types"
    DIM result AS LONG

    ' SINGLE
    id.t = SINGLETYPE
    result = Test_AssertEqualString&("SINGLE", id2fulltypename$, "SINGLE type")

    ' DOUBLE
    IF result THEN
        id.t = DOUBLETYPE
        result = Test_AssertEqualString&("DOUBLE", id2fulltypename$, "DOUBLE type")
    END IF

    ' _FLOAT
    IF result THEN
        id.t = FLOATTYPE
        result = Test_AssertEqualString&("_FLOAT", id2fulltypename$, "_FLOAT type")
    END IF

    Test_End result
END SUB

SUB Test_ID2FullTypeName_Unsigned
    Test_Start "id2fulltypename$ - unsigned types"
    DIM result AS LONG

    ' _UNSIGNED _BYTE
    id.t = UBYTETYPE
    result = Test_AssertEqualString&("_UNSIGNED _BYTE", id2fulltypename$, "_UNSIGNED _BYTE")

    ' _UNSIGNED INTEGER
    IF result THEN
        id.t = UINTEGERTYPE
        result = Test_AssertEqualString&("_UNSIGNED INTEGER", id2fulltypename$, "_UNSIGNED INTEGER")
    END IF

    ' _UNSIGNED LONG
    IF result THEN
        id.t = ULONGTYPE
        result = Test_AssertEqualString&("_UNSIGNED LONG", id2fulltypename$, "_UNSIGNED LONG")
    END IF

    ' _UNSIGNED _INTEGER64
    IF result THEN
        id.t = UINTEGER64TYPE
        result = Test_AssertEqualString&("_UNSIGNED _INTEGER64", id2fulltypename$, "_UNSIGNED _INTEGER64")
    END IF

    Test_End result
END SUB

SUB Test_ID2FullTypeName_FixedString
    Test_Start "id2fulltypename$ - fixed-length string"
    DIM result AS LONG

    ' STRING * 10
    id.t = STRINGTYPE + ISFIXEDLENGTH - ISPOINTER
    id.tsize = 10
    result = Test_AssertEqualString&("STRING * 10", id2fulltypename$, "Fixed-length string")

    Test_End result
END SUB

' ======================================================================
' ID TO SHORT TYPE NAME CONVERSION
' ======================================================================

SUB Test_ID2ShortTypeName_Basic
    Test_Start "id2shorttypename$ - basic types"
    DIM result AS LONG

    ' STRING
    id.t = STRINGTYPE
    result = Test_AssertEqualString&("STRING", id2shorttypename$, "STRING type")

    ' INTEGER
    IF result THEN
        id.t = INTEGERTYPE
        result = Test_AssertEqualString&("INTEGER", id2shorttypename$, "INTEGER type")
    END IF

    ' LONG
    IF result THEN
        id.t = LONGTYPE
        result = Test_AssertEqualString&("LONG", id2shorttypename$, "LONG type")
    END IF

    Test_End result
END SUB

SUB Test_ID2ShortTypeName_Abbreviated
    Test_Start "id2shorttypename$ - abbreviated names"
    DIM result AS LONG

    ' _BYTE → _BYTE
    id.t = BYTETYPE
    result = Test_AssertEqualString&("_BYTE", id2shorttypename$, "_BYTE abbreviated")

    ' _UNSIGNED _BYTE → _UBYTE
    IF result THEN
        id.t = UBYTETYPE
        result = Test_AssertEqualString&("_UBYTE", id2shorttypename$, "_UBYTE abbreviated")
    END IF

    ' _UNSIGNED INTEGER → UINTEGER
    IF result THEN
        id.t = UINTEGERTYPE
        result = Test_AssertEqualString&("UINTEGER", id2shorttypename$, "UINTEGER abbreviated")
    END IF

    ' _UNSIGNED LONG → ULONG
    IF result THEN
        id.t = ULONGTYPE
        result = Test_AssertEqualString&("ULONG", id2shorttypename$, "ULONG abbreviated")
    END IF

    ' _UNSIGNED _INTEGER64 → _UINTEGER64
    IF result THEN
        id.t = UINTEGER64TYPE
        result = Test_AssertEqualString&("_UINTEGER64", id2shorttypename$, "_UINTEGER64 abbreviated")
    END IF

    Test_End result
END SUB

SUB Test_ID2ShortTypeName_BitTypes
    Test_Start "id2shorttypename$ - _BIT types"
    DIM result AS LONG

    ' _BIT (1 bit) → _BIT1
    id.t = BITTYPE
    result = Test_AssertEqualString&("_BIT1", id2shorttypename$, "_BIT → _BIT1")

    ' _UNSIGNED _BIT (1 bit) → _UBIT1
    IF result THEN
        id.t = UBITTYPE
        result = Test_AssertEqualString&("_UBIT1", id2shorttypename$, "_UNSIGNED _BIT → _UBIT1")
    END IF

    ' _BIT*8 → _BIT8
    IF result THEN
        id.t = 8 + ISPOINTER + ISOFFSETINBITS
        result = Test_AssertEqualString&("_BIT8", id2shorttypename$, "_BIT*8 → _BIT8")
    END IF

    ' _UNSIGNED _BIT*16 → _UBIT16
    IF result THEN
        id.t = 16 + ISPOINTER + ISUNSIGNED + ISOFFSETINBITS
        result = Test_AssertEqualString&("_UBIT16", id2shorttypename$, "_UNSIGNED _BIT*16 → _UBIT16")
    END IF

    Test_End result
END SUB

SUB Test_ID2ShortTypeName_FixedString
    Test_Start "id2shorttypename$ - fixed-length string abbreviated"
    DIM result AS LONG

    ' STRING * 20 → STRING20
    id.t = STRINGTYPE + ISFIXEDLENGTH - ISPOINTER
    id.tsize = 20
    result = Test_AssertEqualString&("STRING20", id2shorttypename$, "Fixed-length string abbreviated")

    Test_End result
END SUB

' ======================================================================
' TEST RUNNER - Run all type conversion tests
' ======================================================================

SUB RunTypeConversionTests
    ' typevalue2symbol$ tests
    Test_TypeValue2Symbol_String
    Test_TypeValue2Symbol_Integer
    Test_TypeValue2Symbol_Long
    Test_TypeValue2Symbol_Integer64
    Test_TypeValue2Symbol_Byte
    Test_TypeValue2Symbol_Single
    Test_TypeValue2Symbol_Double
    Test_TypeValue2Symbol_Float
    Test_TypeValue2Symbol_Unsigned
    Test_TypeValue2Symbol_Bit

    ' symbol2fulltypename$ tests
    Test_Symbol2FullTypeName_Basic
    Test_Symbol2FullTypeName_Unsigned
    Test_Symbol2FullTypeName_BitTypes
    Test_Symbol2FullTypeName_Offset

    ' id2fulltypename$ tests
    Test_ID2FullTypeName_Basic
    Test_ID2FullTypeName_Float
    Test_ID2FullTypeName_Unsigned
    Test_ID2FullTypeName_FixedString

    ' id2shorttypename$ tests
    Test_ID2ShortTypeName_Basic
    Test_ID2ShortTypeName_Abbreviated
    Test_ID2ShortTypeName_BitTypes
    Test_ID2ShortTypeName_FixedString
END SUB
