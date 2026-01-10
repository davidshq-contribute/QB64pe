'
' Output Verification Utilities for Unit Tests
'
' Provides functions to verify symbol tables, generated code, and other compiler outputs.
' This module enables comprehensive verification of compiler component outputs.
'

' Symbol table verification structures
TYPE SymbolInfo
    name AS STRING
    flags AS LONG
    reference AS LONG
END TYPE

TYPE SymbolTableSnapshot
    symbolCount AS LONG
    symbols() AS SymbolInfo
    variableCount AS LONG
    functionCount AS LONG
    subCount AS LONG
    constantCount AS LONG
    typeCount AS LONG
END TYPE

' Generated code verification structures
TYPE CodeStructure
    totalLines AS LONG
    lines() AS STRING
    functionCount AS LONG
    classCount AS LONG
    includeCount AS LONG
END TYPE

' Initialize a symbol table snapshot
' Returns: 1 on success, 0 on failure
FUNCTION VerifySymbolTable_InitSnapshot& (snapshot AS SymbolTableSnapshot)

' Enumerate all symbols in the hash table and populate snapshot
' snapshot: SymbolTableSnapshot to populate
' Returns: 1 on success, 0 on failure
FUNCTION VerifySymbolTable_Enumerate& (snapshot AS SymbolTableSnapshot)

' Get symbol count in hash table
' Returns: Number of symbols
FUNCTION VerifySymbolTable_GetCount& ()

' Get symbol count by flag type
' flagMask: Flag mask to filter by (e.g., HASHFLAG_VARIABLE)
' Returns: Number of symbols matching the flag
FUNCTION VerifySymbolTable_GetCountByFlag& (flagMask AS LONG)

' Verify that a symbol exists with expected properties
' name$: Symbol name to check
' expectedFlags: Expected flags
' expectedRef: Expected reference (0 to ignore)
' Returns: 1 if symbol matches, 0 otherwise
FUNCTION VerifySymbolTable_VerifySymbol& (name$, expectedFlags AS LONG, expectedRef AS LONG)

' Verify symbol table contains exactly the expected symbols
' expectedSymbols$(): Array of expected symbol names
' expectedCount: Expected total count (0 to use array size)
' Returns: 1 if all symbols present, 0 otherwise
FUNCTION VerifySymbolTable_VerifyExactSymbols& (expectedSymbols$(), expectedCount AS LONG)

' Compare two symbol table snapshots
' snapshot1: First snapshot
' snapshot2: Second snapshot
' differences$: Output string describing differences
' Returns: 1 if identical, 0 if different
FUNCTION VerifySymbolTable_CompareSnapshots& (snapshot1 AS SymbolTableSnapshot, snapshot2 AS SymbolTableSnapshot, differences$)

' Extract complete code from buffer
' bufHandle: Buffer handle
' code AS CodeStructure: Structure to populate with code
' Returns: 1 on success, 0 on failure
FUNCTION VerifyCode_ExtractFromBuffer& (bufHandle AS INTEGER, code AS CodeStructure)

' Verify generated code contains expected content
' code AS CodeStructure: Code structure to check
' expectedContent$: Expected string content
' Returns: 1 if found, 0 if not found
FUNCTION VerifyCode_Contains& (code AS CodeStructure, expectedContent$)

' Verify generated code contains expected line
' code AS CodeStructure: Code structure to check
' expectedLine$: Expected line content
' Returns: 1 if found, 0 if not found
FUNCTION VerifyCode_ContainsLine& (code AS CodeStructure, expectedLine$)

' Verify generated code structure (function count, etc.)
' code AS CodeStructure: Code structure to check
' expectedFunctionCount: Expected number of functions (0 to ignore)
' expectedLineCount: Expected number of lines (0 to ignore)
' Returns: 1 if structure matches, 0 otherwise
FUNCTION VerifyCode_VerifyStructure& (code AS CodeStructure, expectedFunctionCount AS LONG, expectedLineCount AS LONG)

' Compare two code structures
' code1: First code structure
' code2: Second code structure
' differences$: Output string describing differences
' Returns: 1 if identical, 0 if different
FUNCTION VerifyCode_CompareStructures& (code1 AS CodeStructure, code2 AS CodeStructure, differences$)

' Clean up symbol table snapshot
SUB VerifySymbolTable_CleanupSnapshot (snapshot AS SymbolTableSnapshot)

' Clean up code structure
SUB VerifyCode_CleanupStructure (code AS CodeStructure)
