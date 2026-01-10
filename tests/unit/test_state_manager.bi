'
' Test State Manager for Component Isolation
'
' Provides state management for testing compiler components in isolation.
' This allows testing components with minimal compiler state without
' requiring full compiler initialization.
'
' Note: This file should be included after the component .bi files
' that define the types and constants it uses.
'

' Test state context structure
' Tracks which components have been initialized for testing
TYPE TestStateContext
    ' Component initialization flags
    hashTableInitialized AS LONG
    typeSystemInitialized AS LONG
    constEvalInitialized AS LONG
    
    ' Saved state for restoration
    savedHashListSize AS LONG
    savedHashListNext AS LONG
    savedHashListFreeSize AS LONG
    savedHashListFreeLast AS LONG
    savedConstMax AS LONG
    savedConstLast AS LONG
END TYPE

' Initialize minimal state for testing a component
' context: Test state context to initialize
' componentType$: Type of component to initialize ("hash", "type", "const", or "all")
SUB TestState_Init (context AS TestStateContext, componentType$)
    componentType$ = UCASE$(componentType$)
    
    ' Initialize hash table if needed
    IF componentType$ = "HASH" OR componentType$ = "ALL" THEN
        IF NOT context.hashTableInitialized THEN
            ' Save current state if hash table was already initialized
            IF HashListSize > 0 THEN
                context.savedHashListSize = HashListSize
                context.savedHashListNext = HashListNext
                context.savedHashListFreeSize = HashListFreeSize
                context.savedHashListFreeLast = HashListFreeLast
            END IF
            
            ' Initialize hash table for testing
            ' Use default size (65536) to match HashClear behavior for consistency
            HashListSize = 65536
            HashListNext = 1
            HashListFreeSize = 1024
            HashListFreeLast = 0
            REDIM HashList(1 TO HashListSize) AS HashListItem
            REDIM HashListName(1 TO HashListSize) AS STRING * 256
            REDIM HashListFree(1 TO HashListFreeSize) AS LONG
            REDIM HashTable(16777215) AS LONG
            
            ' Initialize hash lookup tables (these are initialized in hash.bi)
            ' They should already be initialized, but we ensure they exist
            
            context.hashTableInitialized = -1
        END IF
    END IF
    
    ' Initialize type system if needed
    ' Note: Type system constants and arrays are initialized in type.bi
    ' We just mark it as initialized - arrays are already REDIM'd in type.bi
    IF componentType$ = "TYPE" OR componentType$ = "ALL" THEN
        IF NOT context.typeSystemInitialized THEN
            ' Type system constants and arrays are already initialized via $INCLUDE
            ' No additional initialization needed
            context.typeSystemInitialized = -1
        END IF
    END IF
    
    ' Initialize constant evaluation if needed
    IF componentType$ = "CONST" OR componentType$ = "ALL" THEN
        IF NOT context.constEvalInitialized THEN
            ' Save current state if const arrays were already initialized
            IF constmax > 0 THEN
                context.savedConstMax = constmax
                context.savedConstLast = constlast
            END IF
            
            ' Initialize const arrays for testing
            constmax = 100
            constlast = -1
            REDIM SHARED constname(constmax) AS STRING
            REDIM SHARED constcname(constmax) AS STRING
            REDIM SHARED constnamesymbol(constmax) AS STRING
            REDIM SHARED consttype(constmax) AS LONG
            REDIM SHARED constinteger(constmax) AS _INTEGER64
            REDIM SHARED constuinteger(constmax) AS _UNSIGNED _INTEGER64
            REDIM SHARED constfloat(constmax) AS _FLOAT
            REDIM SHARED conststring(constmax) AS STRING
            REDIM SHARED constsubfunc(constmax) AS LONG
            REDIM SHARED constdefined(constmax) AS LONG
            
            ' Initialize const functions if needed
            ' Note: ConstFuncs array is REDIM'd and Set_ConstFunctions is called in const_eval.bi
            ' The array should already be initialized via $INCLUDE
            ' No additional initialization needed here
            
            context.constEvalInitialized = -1
        END IF
    END IF
END SUB

' Clean up test state
' context: Test state context to clean up
SUB TestState_Cleanup (context AS TestStateContext)
    ' Clean up hash table if we initialized it
    IF context.hashTableInitialized THEN
        ' Restore saved state if it existed, otherwise clear
        IF context.savedHashListSize > 0 THEN
            ' Restore previous state - set size variables and REDIM arrays to match
            HashListSize = context.savedHashListSize
            HashListNext = context.savedHashListNext
            HashListFreeSize = context.savedHashListFreeSize
            HashListFreeLast = context.savedHashListFreeLast
            ' REDIM arrays to match saved sizes
            ' Note: Array contents are not restored (they will be empty/new)
            ' This ensures array sizes match the saved state for consistency
            REDIM HashList(1 TO HashListSize) AS HashListItem
            REDIM HashListName(1 TO HashListSize) AS STRING * 256
            REDIM HashListFree(1 TO HashListFreeSize) AS LONG
            REDIM HashTable(16777215) AS LONG
        ELSE
            ' No previous state, just clear
            HashClear
        END IF
        
        context.hashTableInitialized = 0
    END IF
    
    ' Clean up constant evaluation if we initialized it
    IF context.constEvalInitialized THEN
        ' Restore saved state if it existed
        IF context.savedConstMax > 0 THEN
            ' Restore previous state - set size variables and REDIM arrays to match
            constmax = context.savedConstMax
            constlast = context.savedConstLast
            ' REDIM arrays to match saved sizes
            ' Note: Array contents are not restored (they will be empty/new)
            ' This ensures array sizes match the saved state for consistency
            REDIM SHARED constname(constmax) AS STRING
            REDIM SHARED constcname(constmax) AS STRING
            REDIM SHARED constnamesymbol(constmax) AS STRING
            REDIM SHARED consttype(constmax) AS LONG
            REDIM SHARED constinteger(constmax) AS _INTEGER64
            REDIM SHARED constuinteger(constmax) AS _UNSIGNED _INTEGER64
            REDIM SHARED constfloat(constmax) AS _FLOAT
            REDIM SHARED conststring(constmax) AS STRING
            REDIM SHARED constsubfunc(constmax) AS LONG
            REDIM SHARED constdefined(constmax) AS LONG
        ELSE
            ' No previous state, just clear
            constlast = -1
        END IF
        
        context.constEvalInitialized = 0
    END IF
    
    ' Type system doesn't need cleanup (constants are read-only)
    context.typeSystemInitialized = 0
END SUB

' Reset a test state context to uninitialized state
SUB TestState_Reset (context AS TestStateContext)
    context.hashTableInitialized = 0
    context.typeSystemInitialized = 0
    context.constEvalInitialized = 0
    context.savedHashListSize = 0
    context.savedHashListNext = 0
    context.savedHashListFreeSize = 0
    context.savedHashListFreeLast = 0
    context.savedConstMax = 0
    context.savedConstLast = 0
END SUB
