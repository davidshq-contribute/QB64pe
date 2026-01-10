# Component Testing Strategy

## Problem Statement

The QB64-PE compiler components (parser, type system, symbol table, code generator) are tightly coupled to global compiler state, making it difficult to test them in isolation with real implementations.

## Current State

- ✅ Include abstraction and skip includes mode implemented
- ✅ Unit test framework exists
- ❌ Components use extensive `DIM SHARED` global state
- ❌ Components are deeply integrated into the monolithic compiler
- ❌ No way to call components with isolated state

## Solution: Test Harness with State Isolation

Instead of massive refactoring, we'll create **test harnesses** that:
1. Initialize minimal compiler state needed for components
2. Call components with test inputs
3. Capture and verify outputs
4. Clean up state between tests

## Implementation Plan

### Phase 1: Component Test Harness Infrastructure

**Goal**: Create infrastructure to test components with minimal compiler state.

#### 1.1 Test State Manager (`tests/unit/test_state_manager.bi`)

Create a state manager that can:
- Initialize minimal compiler state for a component
- Save/restore global state between tests
- Provide isolated test contexts

```basic
' Test state manager for component isolation
TYPE TestStateContext
    ' Component-specific state
    hashTableInitialized AS LONG
    typeSystemInitialized AS LONG
    constEvalInitialized AS LONG
END TYPE

SUB TestState_Init (context AS TestStateContext)
    ' Initialize only what's needed for testing
    ' This allows testing components without full compiler initialization
END SUB

SUB TestState_Cleanup (context AS TestStateContext)
    ' Clean up test state
END SUB
```

#### 1.2 Component Test Wrappers

Create wrapper functions that:
- Set up minimal state
- Call the real component function
- Return results in a testable format

**Example for Type System**:
```basic
' Wrapper for testing type system functions
FUNCTION TestType_ConvertSymbol$ (typeValue AS LONG)
    ' Initialize minimal type system state if needed
    ' Call real function: typevalue2symbol$(typeValue)
    ' Return result
END FUNCTION
```

### Phase 2: Test Individual Components

#### 2.1 Type System Tests (`tests/unit/type_system/`)

**Approach**: Test type system functions that are relatively self-contained.

**Testable Functions**:
- `typevalue2symbol$()` - Type value to symbol conversion
- `type2symbol$()` - Type name to symbol conversion  
- `typname2typ()` - Type name to type value
- Type flag checking functions (if they don't require full compiler state)

**Implementation**:
```basic
'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'../test_state_manager.bi'

SUB Test_TypeSymbolConversion
    Test_Start "Type symbol conversion"
    
    DIM context AS TestStateContext
    TestState_Init context
    
    ' Test with real implementation
    DIM result$
    result$ = typevalue2symbol$(STRINGTYPE)
    result = Test_AssertEqualString&("$", result$, "String type should return $")
    
    TestState_Cleanup context
    Test_End result
END SUB
```

#### 2.2 Symbol Table Tests (`tests/unit/symbol_table/`)

**Approach**: Test hash table operations with isolated state.

**Testable Functions**:
- `HashAdd()` - Add symbol to table
- `HashFind()` - Lookup symbol
- `HashRemove()` - Remove symbol
- Hash collision handling

**Implementation**:
```basic
SUB Test_HashAddAndFind
    Test_Start "Hash add and find"
    
    DIM context AS TestStateContext
    TestState_Init context
    
    ' Initialize hash table
    HashListSize = 1000
    HashListNext = 1
    REDIM HashList(1 TO HashListSize) AS HashListItem
    REDIM HashListName(1 TO HashListSize) AS STRING * 256
    REDIM HashTable(16777215) AS LONG
    
    ' Test adding and finding
    HashAdd "testVar", HASHFLAG_VARIABLE, 1
    DIM found AS LONG, flags AS LONG, ref AS LONG
    found = HashFind("testVar", HASHFLAG_VARIABLE, flags, ref)
    
    result = Test_Assert&(found, "Should find added symbol")
    
    TestState_Cleanup context
    Test_End result
END SUB
```

#### 2.3 Constant Evaluation Tests (`tests/unit/const_eval/`)

**Approach**: Test constant evaluation with minimal state.

**Testable Functions**:
- `Evaluate_Expression$()` - Evaluate constant expressions
- Constant folding logic
- Error handling

**Implementation**:
```basic
SUB Test_ConstantFolding
    Test_Start "Constant folding"
    
    DIM context AS TestStateContext
    TestState_Init context
    
    ' Initialize const arrays
    constmax = 100
    REDIM constname(constmax) AS STRING
    REDIM consttype(constmax) AS LONG
    ' ... other const arrays
    
    ' Test evaluation
    DIM tempNum AS ParseNum
    DIM result$
    result$ = Evaluate_Expression$("2 + 3", tempNum)
    
    result = Test_AssertEqual&(5, tempNum.i, "Should evaluate to 5")
    
    TestState_Cleanup context
    Test_End result
END SUB
```

### Phase 3: Parser Testing (More Complex)

**Challenge**: Parser is deeply integrated into the compiler loop.

**Approach**: 
1. Use skip includes mode to test parsing of isolated code snippets
2. Create minimal test programs that exercise specific parsing features
3. Verify parsing results through symbol table inspection

**Example**:
```basic
SUB Test_StatementParsing
    Test_Start "Statement parsing"
    
    ' Use skip includes to test parsing without dependencies
    TestFramework_SkipIncludes -1
    
    ' Create minimal test source
    DIM testSource$
    testSource$ = "DIM x AS LONG" + CHR$(10) + "x = 5"
    
    ' Compile with minimal state and inspect results
    ' (This requires more infrastructure)
    
    TestFramework_SkipIncludes 0
    Test_End result
END SUB
```

### Phase 4: Code Generator Testing

**Approach**: Test code generation by:
1. Compiling minimal test programs
2. Inspecting generated C++ code
3. Verifying code structure

**Example**:
```basic
SUB Test_CodeGeneration
    Test_Start "Code generation"
    
    ' Compile minimal program
    ' Inspect generated files in temp/
    ' Verify expected C++ code structure
    
    Test_End result
END SUB
```

## Testing Infrastructure Components

### 1. Test State Manager (`tests/unit/test_state_manager.bi`)

Manages component state initialization and cleanup.

### 2. Component Test Utilities (`tests/unit/test_component_utils.bi`)

Helper functions for:
- Setting up test contexts
- Inspecting component outputs
- Comparing results

### 3. Minimal Compiler Context (`tests/unit/test_compiler_context.bi`)

Provides minimal compiler initialization for testing:
- Initializes only required global state
- Provides mock implementations for non-essential parts
- Allows components to run in isolation

## Benefits

1. **No Major Refactoring Required**: Works with existing code structure
2. **Real Implementations**: Tests actual component code, not mocks
3. **Incremental**: Can be implemented component by component
4. **Practical**: Focuses on testable parts first

## Limitations

1. **Not All Components Testable**: Some components are too tightly coupled
2. **State Management Overhead**: Need to carefully manage global state
3. **Partial Coverage**: May not test all code paths

## Next Steps

1. **Start with Type System**: Easiest to test, relatively self-contained
2. **Then Symbol Table**: Hash operations are testable
3. **Then Constant Evaluation**: Can test with minimal state
4. **Parser and Code Gen**: More complex, require more infrastructure

## Example: Complete Type System Test

```basic
'$INCLUDE:'../test_framework.bi'
'$INCLUDE:'../../source/utilities/type.bi'

SUB Test_TypeSystem_Complete
    TestFramework_Init
    
    ' Test type symbol conversion
    Test_Start "Type symbol conversion"
    DIM result$
    result$ = typevalue2symbol$(STRINGTYPE)
    DIM testResult AS LONG
    testResult = Test_AssertEqualString&("$", result$, "String type")
    Test_End testResult
    
    ' Test type name conversion
    Test_Start "Type name conversion"
    result$ = type2symbol$("SINGLE")
    testResult = Test_AssertEqualString&("!", result$, "SINGLE type")
    Test_End testResult
    
    ' More tests...
    
    TestFramework_PrintSummary
END SUB
```

## Conclusion

While full isolation isn't possible without major refactoring, we can achieve significant test coverage by:
1. Creating test harnesses with minimal state
2. Testing self-contained functions first
3. Using skip includes mode for parsing tests
4. Inspecting outputs through available interfaces

This approach provides practical value while working within the constraints of the existing codebase.
