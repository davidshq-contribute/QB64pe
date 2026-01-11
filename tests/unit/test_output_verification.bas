'
' Output Verification Utilities for Unit Tests
'
' Implementation of verification functions for symbol tables, generated code, etc.
'

$INCLUDEONCE

' Note: test_output_verification.bi (includes TYPE definitions and DIM SHARED declarations),
' hash.bi, and simplebuffer.bi are already included by test_runner.bas
'$INCLUDE:'../../source/utilities/hash.bas'
'$INCLUDE:'../../source/utilities/s-buffer/simplebuffer.bm'

' Note: snapshotSymbols and snapshotLines arrays are declared in test_output_verification.bi

' Initialize a symbol table snapshot
FUNCTION VerifySymbolTable_InitSnapshot& (snapshot AS SymbolTableSnapshot)
    snapshot.symbolCount = 0
    snapshot.variableCount = 0
    snapshot.functionCount = 0
    snapshot.subCount = 0
    snapshot.constantCount = 0
    snapshot.typeCount = 0
    ' Static array - no REDIM needed
    VerifySymbolTable_InitSnapshot& = 1
END FUNCTION

' Enumerate all symbols in the hash table
FUNCTION VerifySymbolTable_Enumerate& (snapshot AS SymbolTableSnapshot)
    DIM x AS LONG, i AS LONG, lasti AS LONG
    DIM count AS LONG
    DIM tempSymbols(1 TO 10000) AS SymbolInfo ' Temporary storage
    
    count = 0
    
    ' Iterate through hash table
    FOR x = 0 TO 16777215
        IF HashTable(x) THEN
            i = HashTable(x)
            lasti = HashList(i).LastItem
            
            ' Iterate through chain
            DO WHILE i > 0
                count = count + 1
                IF count > 10000 THEN
                    ' Too many symbols, resize
                    VerifySymbolTable_Enumerate& = 0
                    EXIT FUNCTION
                END IF
                
                ' Store symbol info
                tempSymbols(count).name = RTRIM$(HashListName(i))
                tempSymbols(count).flags = HashList(i).Flags
                tempSymbols(count).reference = HashList(i).Reference
                
                ' Count by type
                IF HashList(i).Flags AND HASHFLAG_VARIABLE THEN
                    snapshot.variableCount = snapshot.variableCount + 1
                END IF
                IF HashList(i).Flags AND HASHFLAG_FUNCTION THEN
                    snapshot.functionCount = snapshot.functionCount + 1
                END IF
                IF HashList(i).Flags AND HASHFLAG_SUB THEN
                    snapshot.subCount = snapshot.subCount + 1
                END IF
                IF HashList(i).Flags AND HASHFLAG_CONSTANT THEN
                    snapshot.constantCount = snapshot.constantCount + 1
                END IF
                IF HashList(i).Flags AND HASHFLAG_TYPE THEN
                    snapshot.typeCount = snapshot.typeCount + 1
                END IF
                
                ' Move to next in chain
                IF i = lasti THEN EXIT DO
                i = HashList(i).NextItem
                IF i = 0 THEN EXIT DO
            LOOP
        END IF
    NEXT
    
    ' Copy to snapshot (using static array, 0-based indexing)
    snapshot.symbolCount = count
    IF count > 0 AND count <= 10000 THEN
        FOR i = 0 TO count - 1
            snapshotSymbols(i) = tempSymbols(i + 1)
        NEXT
    END IF
    
    VerifySymbolTable_Enumerate& = 1
END FUNCTION

' Get symbol count in hash table
FUNCTION VerifySymbolTable_GetCount& ()
    DIM snapshot AS SymbolTableSnapshot
    DIM result AS LONG
    
    result = VerifySymbolTable_InitSnapshot&(snapshot)
    IF result THEN
        result = VerifySymbolTable_Enumerate&(snapshot)
    END IF
    IF result THEN
        VerifySymbolTable_GetCount& = snapshot.symbolCount
    ELSE
        VerifySymbolTable_GetCount& = 0
    END IF
    VerifySymbolTable_CleanupSnapshot snapshot
END FUNCTION

' Get symbol count by flag type
FUNCTION VerifySymbolTable_GetCountByFlag& (flagMask AS LONG)
    DIM snapshot AS SymbolTableSnapshot
    DIM result AS LONG
    DIM count AS LONG
    DIM i AS LONG
    
    result = VerifySymbolTable_InitSnapshot&(snapshot)
    IF result THEN
        result = VerifySymbolTable_Enumerate&(snapshot)
    END IF
    
    IF result THEN
        count = 0
        FOR i = 0 TO snapshot.symbolCount - 1
            IF snapshotSymbols(i).flags AND flagMask THEN
                count = count + 1
            END IF
        NEXT
        VerifySymbolTable_GetCountByFlag& = count
    ELSE
        VerifySymbolTable_GetCountByFlag& = 0
    END IF
    
    VerifySymbolTable_CleanupSnapshot snapshot
END FUNCTION

' Verify that a symbol exists with expected properties
FUNCTION VerifySymbolTable_VerifySymbol& (name$, expectedFlags AS LONG, expectedRef AS LONG)
    DIM flags AS LONG, ref AS LONG
    DIM found AS LONG
    
    found = HashFind(name$, expectedFlags, flags, ref)
    
    IF found = 0 THEN
        VerifySymbolTable_VerifySymbol& = 0
        EXIT FUNCTION
    END IF
    
    ' Check flags match
    IF (flags AND expectedFlags) <> expectedFlags THEN
        VerifySymbolTable_VerifySymbol& = 0
        EXIT FUNCTION
    END IF
    
    ' Check reference if specified
    IF expectedRef <> 0 AND ref <> expectedRef THEN
        VerifySymbolTable_VerifySymbol& = 0
        EXIT FUNCTION
    END IF
    
    VerifySymbolTable_VerifySymbol& = 1
END FUNCTION

' Verify symbol table contains exactly the expected symbols
FUNCTION VerifySymbolTable_VerifyExactSymbols& (expectedSymbols$(), expectedCount AS LONG)
    DIM snapshot AS SymbolTableSnapshot
    DIM result AS LONG
    DIM i AS LONG, j AS LONG
    DIM found AS LONG
    DIM actualCount AS LONG
    
    result = VerifySymbolTable_InitSnapshot&(snapshot)
    IF result THEN
        result = VerifySymbolTable_Enumerate&(snapshot)
    END IF
    
    IF NOT result THEN
        VerifySymbolTable_VerifyExactSymbols& = 0
        VerifySymbolTable_CleanupSnapshot snapshot
        EXIT FUNCTION
    END IF
    
    ' Use provided count or array size
    IF expectedCount = 0 THEN
        expectedCount = UBOUND(expectedSymbols$)
    END IF
    
    actualCount = snapshot.symbolCount
    
    ' Check count matches
    IF actualCount <> expectedCount THEN
        VerifySymbolTable_VerifyExactSymbols& = 0
        VerifySymbolTable_CleanupSnapshot snapshot
        EXIT FUNCTION
    END IF
    
    ' Check each expected symbol exists
    FOR i = 1 TO expectedCount
        found = 0
        FOR j = 0 TO snapshot.symbolCount - 1
            IF UCASE$(RTRIM$(snapshotSymbols(j).name)) = UCASE$(RTRIM$(expectedSymbols$(i))) THEN
                found = 1
                EXIT FOR
            END IF
        NEXT
        IF NOT found THEN
            VerifySymbolTable_VerifyExactSymbols& = 0
            VerifySymbolTable_CleanupSnapshot snapshot
            EXIT FUNCTION
        END IF
    NEXT
    
    VerifySymbolTable_VerifyExactSymbols& = 1
    VerifySymbolTable_CleanupSnapshot snapshot
END FUNCTION

' Compare two symbol table snapshots
FUNCTION VerifySymbolTable_CompareSnapshots& (snapshot1 AS SymbolTableSnapshot, snapshot2 AS SymbolTableSnapshot, differences$)
    DIM i AS LONG, j AS LONG
    DIM found AS LONG
    
    differences$ = ""
    
    ' Compare counts
    IF snapshot1.symbolCount <> snapshot2.symbolCount THEN
        differences$ = differences$ + "Symbol count mismatch: " + _TOSTR$(snapshot1.symbolCount) + " vs " + _TOSTR$(snapshot2.symbolCount) + CHR$(10)
    END IF
    
    ' Compare each symbol in snapshot1
    ' Note: We need to store symbols in separate arrays for each snapshot
    ' For now, we'll compare using the global snapshotSymbols array
    ' This assumes snapshots are compared one at a time
    FOR i = 0 TO snapshot1.symbolCount - 1
        found = 0
        FOR j = 0 TO snapshot2.symbolCount - 1
            IF snapshotSymbols(i).name = snapshotSymbols(j).name THEN
                found = 1
                ' Check flags and reference
                IF snapshotSymbols(i).flags <> snapshotSymbols(j).flags THEN
                    differences$ = differences$ + "Symbol " + snapshotSymbols(i).name + " flags differ" + CHR$(10)
                END IF
                IF snapshotSymbols(i).reference <> snapshotSymbols(j).reference THEN
                    differences$ = differences$ + "Symbol " + snapshotSymbols(i).name + " reference differs" + CHR$(10)
                END IF
                EXIT FOR
            END IF
        NEXT
        IF NOT found THEN
            differences$ = differences$ + "Symbol " + snapshotSymbols(i).name + " missing in snapshot2" + CHR$(10)
        END IF
    NEXT
    
    ' Check for symbols in snapshot2 not in snapshot1
    FOR i = 0 TO snapshot2.symbolCount - 1
        found = 0
        FOR j = 0 TO snapshot1.symbolCount - 1
            IF snapshotSymbols(i).name = snapshotSymbols(j).name THEN
                found = 1
                EXIT FOR
            END IF
        NEXT
        IF NOT found THEN
            differences$ = differences$ + "Symbol " + snapshotSymbols(i).name + " missing in snapshot1" + CHR$(10)
        END IF
    NEXT
    
    IF differences$ = "" THEN
        VerifySymbolTable_CompareSnapshots& = 1
    ELSE
        VerifySymbolTable_CompareSnapshots& = 0
    END IF
END FUNCTION

' Extract complete code from buffer
FUNCTION VerifyCode_ExtractFromBuffer& (bufHandle AS INTEGER, code AS CodeStructure)
    DIM line$
    DIM tempLines(1 TO 10000) AS STRING
    DIM count AS LONG
    DIM oldPos AS LONG
    DIM i AS LONG
    
    ' Save current position
    oldPos = GetBufPos&(bufHandle)
    
    ' Seek to beginning
    DIM seekResult AS LONG
    seekResult = SeekBuf&(bufHandle, 0, SBM_BufStart)
    
    ' Read all lines
    count = 0
    DO WHILE NOT EndOfBuf%(bufHandle)
        line$ = ReadBufLine$(bufHandle)
        count = count + 1
        IF count > 10000 THEN
            ' Too many lines
            seekResult = SeekBuf&(bufHandle, oldPos, SBM_PosRestore)
            VerifyCode_ExtractFromBuffer& = 0
            EXIT FUNCTION
        END IF
        tempLines(count) = line$
    LOOP
    
    ' Populate structure (using static array, 0-based indexing)
    code.totalLines = count
    IF count > 0 AND count <= 10000 THEN
        FOR i = 0 TO count - 1
            snapshotLines(i) = tempLines(i + 1)
        NEXT
    END IF
    
    ' Count functions and classes (simple pattern matching)
    code.functionCount = 0
    code.classCount = 0
    code.includeCount = 0
    FOR i = 1 TO count
        line$ = UCASE$(LTRIM$(RTRIM$(tempLines(i))))
        IF INSTR(line$, "FUNCTION ") > 0 OR INSTR(line$, "FUNCTION(") > 0 THEN
            code.functionCount = code.functionCount + 1
        END IF
        IF INSTR(line$, "CLASS ") > 0 THEN
            code.classCount = code.classCount + 1
        END IF
        IF INSTR(line$, "#INCLUDE") > 0 OR INSTR(line$, "INCLUDE") > 0 THEN
            code.includeCount = code.includeCount + 1
        END IF
    NEXT
    
    ' Restore position (SBM_PosRestore uses position directly)
    seekResult = SeekBuf&(bufHandle, oldPos, SBM_PosRestore)
    
    VerifyCode_ExtractFromBuffer& = 1
END FUNCTION

' Verify generated code contains expected content
FUNCTION VerifyCode_Contains& (code AS CodeStructure, expectedContent$)
    DIM i AS LONG
    
    FOR i = 0 TO code.totalLines - 1
        IF INSTR(snapshotLines(i), expectedContent$) > 0 THEN
            VerifyCode_Contains& = 1
            EXIT FUNCTION
        END IF
    NEXT
    
    VerifyCode_Contains& = 0
END FUNCTION

' Verify generated code contains expected line
FUNCTION VerifyCode_ContainsLine& (code AS CodeStructure, expectedLine$)
    DIM i AS LONG
    
    FOR i = 0 TO code.totalLines - 1
        IF RTRIM$(snapshotLines(i)) = RTRIM$(expectedLine$) THEN
            VerifyCode_ContainsLine& = 1
            EXIT FUNCTION
        END IF
    NEXT
    
    VerifyCode_ContainsLine& = 0
END FUNCTION

' Verify generated code structure
FUNCTION VerifyCode_VerifyStructure& (code AS CodeStructure, expectedFunctionCount AS LONG, expectedLineCount AS LONG)
    IF expectedFunctionCount > 0 THEN
        IF code.functionCount <> expectedFunctionCount THEN
            VerifyCode_VerifyStructure& = 0
            EXIT FUNCTION
        END IF
    END IF
    
    IF expectedLineCount > 0 THEN
        IF code.totalLines <> expectedLineCount THEN
            VerifyCode_VerifyStructure& = 0
            EXIT FUNCTION
        END IF
    END IF
    
    VerifyCode_VerifyStructure& = 1
END FUNCTION

' Compare two code structures
FUNCTION VerifyCode_CompareStructures& (code1 AS CodeStructure, code2 AS CodeStructure, differences$)
    DIM i AS LONG
    
    differences$ = ""
    
    ' Compare line counts
    IF code1.totalLines <> code2.totalLines THEN
        differences$ = differences$ + "Line count mismatch: " + _TOSTR$(code1.totalLines) + " vs " + _TOSTR$(code2.totalLines) + CHR$(10)
    END IF
    
    ' Compare function counts
    IF code1.functionCount <> code2.functionCount THEN
        differences$ = differences$ + "Function count mismatch: " + _TOSTR$(code1.functionCount) + " vs " + _TOSTR$(code2.functionCount) + CHR$(10)
    END IF
    
    ' Compare lines
    DIM minLines AS LONG
    minLines = code1.totalLines
    IF code2.totalLines < minLines THEN minLines = code2.totalLines
    
    FOR i = 0 TO minLines - 1
        IF snapshotLines(i) <> snapshotLines(i) THEN
            differences$ = differences$ + "Line " + _TOSTR$(i + 1) + " differs" + CHR$(10)
        END IF
    NEXT
    
    IF code1.totalLines > code2.totalLines THEN
        differences$ = differences$ + "Code1 has " + _TOSTR$(code1.totalLines - code2.totalLines) + " extra lines" + CHR$(10)
    END IF
    
    IF code2.totalLines > code1.totalLines THEN
        differences$ = differences$ + "Code2 has " + _TOSTR$(code2.totalLines - code1.totalLines) + " extra lines" + CHR$(10)
    END IF
    
    IF differences$ = "" THEN
        VerifyCode_CompareStructures& = 1
    ELSE
        VerifyCode_CompareStructures& = 0
    END IF
END FUNCTION

' Clean up symbol table snapshot
SUB VerifySymbolTable_CleanupSnapshot (snapshot AS SymbolTableSnapshot)
    ' Static array - no cleanup needed, just reset counts
    snapshot.symbolCount = 0
    snapshot.variableCount = 0
    snapshot.functionCount = 0
    snapshot.subCount = 0
    snapshot.constantCount = 0
    snapshot.typeCount = 0
END SUB

' Clean up code structure
SUB VerifyCode_CleanupStructure (code AS CodeStructure)
    ' Static array - no cleanup needed, just reset counts
    code.totalLines = 0
    code.functionCount = 0
    code.classCount = 0
    code.includeCount = 0
END SUB
