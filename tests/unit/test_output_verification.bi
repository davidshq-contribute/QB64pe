'
' Output Verification Utilities for Unit Tests
'
' Provides functions to verify symbol tables, generated code, and other compiler outputs.
' This module enables comprehensive verification of compiler component outputs.
'

$INCLUDEONCE

' Note: hash.bi should be included before this file to ensure CONST declarations
' come before FUNCTION declarations. It's included in test_runner.bas.

' Symbol table verification structures
TYPE SymbolInfo
    name AS STRING
    flags AS LONG
    reference AS LONG
END TYPE

TYPE SymbolTableSnapshot
    symbolCount AS LONG
    ' Note: symbols array stored separately due to QB64 TYPE limitations
    variableCount AS LONG
    functionCount AS LONG
    subCount AS LONG
    constantCount AS LONG
    typeCount AS LONG
END TYPE

' Generated code verification structures
TYPE CodeStructure
    totalLines AS LONG
    ' Note: lines array stored separately due to QB64 TYPE limitations
    functionCount AS LONG
    classCount AS LONG
    includeCount AS LONG
END TYPE

' Note: snapshotSymbols and snapshotLines arrays are declared in test_global_state_declarations.bi
' Note: Function implementations are in test_output_verification.bas

' Forward declarations for output verification functions
DECLARE FUNCTION VerifySymbolTable_InitSnapshot& (snapshot AS SymbolTableSnapshot)
DECLARE FUNCTION VerifySymbolTable_Enumerate& (snapshot AS SymbolTableSnapshot)
DECLARE FUNCTION VerifySymbolTable_GetCount& ()
DECLARE FUNCTION VerifySymbolTable_GetCountByFlag& (flagMask AS LONG)
DECLARE FUNCTION VerifySymbolTable_VerifySymbol& (name$, expectedFlags AS LONG, expectedRef AS LONG)
DECLARE FUNCTION VerifySymbolTable_VerifyExactSymbols& (expectedSymbols$(), expectedCount AS LONG)
DECLARE FUNCTION VerifySymbolTable_CompareSnapshots& (snapshot1 AS SymbolTableSnapshot, snapshot2 AS SymbolTableSnapshot, differences$)
DECLARE SUB VerifySymbolTable_CleanupSnapshot (snapshot AS SymbolTableSnapshot)

DECLARE FUNCTION VerifyCode_ExtractFromBuffer& (bufHandle AS INTEGER, code AS CodeStructure)
DECLARE FUNCTION VerifyCode_Contains& (code AS CodeStructure, expectedContent$)
DECLARE FUNCTION VerifyCode_ContainsLine& (code AS CodeStructure, expectedLine$)
DECLARE FUNCTION VerifyCode_VerifyStructure& (code AS CodeStructure, expectedFunctionCount AS LONG, expectedLineCount AS LONG)
DECLARE FUNCTION VerifyCode_CompareStructures& (code1 AS CodeStructure, code2 AS CodeStructure, differences$)
DECLARE SUB VerifyCode_CleanupStructure (code AS CodeStructure)
