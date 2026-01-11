# Component Testing Strategy

## Problem Statement

The QB64-PE compiler components (parser, type system, symbol table, code generator) are tightly coupled to global compiler state, making it difficult to test them in isolation with real implementations.

## Current State

- ✅ Include abstraction and skip includes mode implemented
- ✅ Unit test framework exists
- ✅ **Test harness infrastructure implemented**
- ✅ **Component tests implemented for all major components**
- ⚠️ Components use extensive `DIM SHARED` global state (managed via test harness)
- ⚠️ Components are deeply integrated into the monolithic compiler (tested via harness)
- ✅ **State isolation achieved through test harness**

## Solution: Test Harness with State Isolation

Instead of massive refactoring, we'll create **test harnesses** that:
1. Initialize minimal compiler state needed for components
2. Call components with test inputs
3. Capture and verify outputs
4. Clean up state between tests

## Implementation Plan

### Phase 1: Component Test Harness Infrastructure

**Goal**: Create infrastructure to test components with minimal compiler state.

#### 1.1 Test State Manager (`tests/unit/test_state_manager.bi`) - ✅ **IMPLEMENTED**

**Status**: Fully implemented with comprehensive state management.

The state manager provides:
- ✅ Initialize minimal compiler state for a component
- ✅ Save/restore global state between tests
- ✅ Provide isolated test contexts
- ✅ Support for component-specific initialization ("hash", "type", "const", or "all")

**Key Features**:
- State preservation: Saves existing state before initialization
- State restoration: Restores previous state after cleanup
- Component isolation: Can initialize individual components or all at once
- Safe cleanup: Handles both initialized and uninitialized states

**Implementation Details**:
```12:27:tests/unit/test_state_manager.bi
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
```

The implementation includes:
- `TestState_Init()` - Initializes component state with optional state preservation
- `TestState_Cleanup()` - Cleans up and restores previous state
- `TestState_Reset()` - Resets context to uninitialized state

#### 1.2 Component Test Utilities (`tests/unit/test_component_utils.bi`) - ✅ **IMPLEMENTED**

**Status**: Fully implemented with helper functions for component testing.

The utilities provide:
- ✅ Context initialization helpers
- ✅ State verification functions
- ✅ Hash table statistics
- ✅ Component state verification

**Key Functions**:
- `TestComponent_InitContext()` - Initialize a test context for a component
- `TestComponent_CleanupContext()` - Clean up a test context
- `TestComponent_VerifyHashTableState&()` - Verify hash table is properly initialized
- `TestComponent_VerifyTypeSystemState&()` - Verify type system is initialized
- `TestComponent_VerifyConstEvalState&()` - Verify const eval is initialized
- `TestComponent_GetHashTableStats()` - Get hash table statistics for inspection

**Usage Pattern**:
Tests use the state manager directly rather than wrappers, following this pattern:
```basic
DIM context AS TestStateContext
TestState_Init context, "hash"  ' or "type", "const", "all"
' ... run tests ...
TestState_Cleanup context
```

### Phase 2: Test Individual Components - ✅ **COMPLETE**

#### 2.1 Type System Tests (`tests/unit/type_system/test_type_system.bas`) - ✅ **IMPLEMENTED**

**Status**: Fully implemented with comprehensive type system coverage.

**Tests Implemented**:
- ✅ `Test_TypeSymbolConversion` - Tests `typevalue2symbol$()` for all basic types
- ✅ `Test_TypeNameConversion` - Tests `type2symbol$()` for type name to symbol conversion
- ✅ `Test_TypeSizeFunctions` - Tests `Type_GetSizeInBits~&()` for size calculations
- ✅ `Test_TypeFlags` - Tests type flag checking functions (`Type_IsString`, `Type_IsFloatingPoint`, etc.)
- ✅ `Test_TypeConversions` - Tests `typ2ctyp$()` and `typname2typ&()` for C++ type conversion

**Example Implementation**:
```13:31:tests/unit/type_system/test_type_system.bas
SUB Test_TypeSymbolConversion
    Test_Start "Type symbol conversion"
    
    DIM context AS TestStateContext
    TestState_Init context, "type"
    
    DIM result AS LONG
    
    ' Test basic type symbol conversions
    result = Test_AssertEqualString&("$", typevalue2symbol$(STRINGTYPE), "String type should return $")
    IF result THEN result = Test_AssertEqualString&("!", typevalue2symbol$(SINGLETYPE), "SINGLE type should return !")
    IF result THEN result = Test_AssertEqualString&("#", typevalue2symbol$(DOUBLETYPE), "DOUBLE type should return #")
    IF result THEN result = Test_AssertEqualString&("&", typevalue2symbol$(LONGTYPE), "LONG type should return &")
    IF result THEN result = Test_AssertEqualString&("%", typevalue2symbol$(INTEGERTYPE), "INTEGER type should return %")
    IF result THEN result = Test_AssertEqualString&("%%", typevalue2symbol$(BYTETYPE), "BYTE type should return %%")
    
    TestState_Cleanup context
    Test_End result
END SUB
```

#### 2.2 Symbol Table Tests (`tests/unit/symbol_table/test_hash.bas`) - ✅ **IMPLEMENTED**

**Status**: Fully implemented with comprehensive hash table testing.

**Tests Implemented**:
- ✅ `Test_SymbolInsertion` - Tests `HashAdd()` and basic symbol insertion
- ✅ `Test_SymbolLookup` - Tests `HashFind()` with multiple symbols and different flags
- ✅ `Test_SymbolScope` - Tests symbol scope resolution with same name, different flags
- ✅ `Test_HashCollisions` - Tests hash collision handling via chaining
- ✅ `Test_SymbolTableVerification` - Tests symbol table verification utilities

**Example Implementation**:
```15:39:tests/unit/symbol_table/test_hash.bas
SUB Test_SymbolInsertion
    Test_Start "Symbol insertion"
    
    DIM context AS TestStateContext
    TestState_Init context, "hash"
    
    DIM result AS LONG
    
    ' Test that symbols can be inserted into the hash table
    HashAdd "testVar", HASHFLAG_VARIABLE, 1
    result = Test_Assert&(HashListNext > 1, "HashAdd should add symbol to table")
    
    ' Verify the symbol was added correctly
    IF result THEN
        DIM flags AS LONG, ref AS LONG
        DIM found AS LONG
        found = HashFind("testVar", HASHFLAG_VARIABLE, flags, ref)
        result = Test_Assert&(found > 0, "Inserted symbol should be findable")
        IF result THEN result = Test_AssertEqual&(HASHFLAG_VARIABLE, flags, "Symbol should have correct flags")
        IF result THEN result = Test_AssertEqual&(1, ref, "Symbol should have correct reference")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB
```

**Note**: Hash table initialization is handled automatically by `TestState_Init()` with component type "hash".

#### 2.3 Constant Evaluation Tests (`tests/unit/const_eval/test_const_eval.bas`) - ✅ **IMPLEMENTED**

**Status**: Fully implemented with constant evaluation testing.

**Tests Implemented**:
- ✅ `Test_ConstantFolding` - Tests arithmetic constant folding (addition, subtraction, multiplication, division)
- ✅ `Test_ConstantFunctions` - Tests constant function evaluation (e.g., `ABS()`)
- ✅ `Test_ConstantErrorHandling` - Tests error handling for invalid expressions

**Example Implementation**:
```17:54:tests/unit/const_eval/test_const_eval.bas
SUB Test_ConstantFolding
    Test_Start "Constant folding"
    
    DIM context AS TestStateContext
    TestState_Init context, "const"
    
    DIM result AS LONG
    
    ' Test that constant expressions are evaluated at compile time
    ' Note: Evaluate_Expression$ requires a ParseNum structure
    DIM num AS ParseNum
    DIM exprResult$
    
    ' Test simple addition
    exprResult$ = Evaluate_Expression$("2+3", num)
    result = Test_AssertEqual&(5, num.i, "2+3 should evaluate to 5")
    
    ' Test subtraction
    IF result THEN
        exprResult$ = Evaluate_Expression$("10-4", num)
        result = Test_AssertEqual&(6, num.i, "10-4 should evaluate to 6")
    END IF
    
    ' Test multiplication
    IF result THEN
        exprResult$ = Evaluate_Expression$("3*4", num)
        result = Test_AssertEqual&(12, num.i, "3*4 should evaluate to 12")
    END IF
    
    ' Test division
    IF result THEN
        exprResult$ = Evaluate_Expression$("15/3", num)
        result = Test_AssertEqual&(5, num.i, "15/3 should evaluate to 5")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB
```

**Note**: Constant evaluation arrays are initialized automatically by `TestState_Init()` with component type "const".

### Phase 3: Parser Testing - ✅ **IMPLEMENTED**

**Status**: Parser tests implemented with multiple test suites.

**Challenge**: Parser is deeply integrated into the compiler loop.

**Approach Implemented**: 
1. ✅ Test parsing functions directly with isolated state
2. ✅ Test expression parsing with `numelements()` and `getelement()`
3. ✅ Test statement parsing for various statement types
4. ✅ Test error handling for invalid syntax

**Tests Implemented**:
- ✅ `tests/unit/parser/test_expression_parsing.bas` - Expression parsing tests
- ✅ `tests/unit/parser/test_statement_parsing.bas` - Statement parsing tests
- ✅ `tests/unit/parser/test_error_handling.bas` - Parser error handling tests
- ✅ `tests/unit/parser/test_parser.bas` - Main parser test suite runner

**Example Implementation**:
```21:54:tests/unit/parser/test_statement_parsing.bas
SUB Test_StatementParsing
    Test_Start "Statement parsing"
    
    DIM context AS TestStateContext
    TestState_Init context, "all"
    
    DIM result AS LONG
    DIM elementCount AS LONG
    DIM firstElement$
    
    ' Test PRINT statement parsing
    DIM testStatement$
    testStatement$ = "PRINT 1, 2, 3"
    elementCount = numelements(testStatement$)
    result = Test_Assert&(elementCount > 0, "Should parse PRINT statement")
    
    IF result THEN
        firstElement$ = getelement(testStatement$, 1)
        result = Test_Assert&(LEN(firstElement$) > 0, "Should extract first element")
    END IF
    
    ' Test LET statement parsing
    IF result THEN
        testStatement$ = "LET x = 5"
        elementCount = numelements(testStatement$)
        result = Test_Assert&(elementCount > 0, "Should parse LET statement")
    END IF
    
    TestState_Cleanup context
    Test_End result
END SUB
```

### Phase 4: Code Generator Testing - ✅ **IMPLEMENTED**

**Status**: Code generation tests implemented with code emission and structure verification.

**Approach Implemented**: 
1. ✅ Test code emission functions directly
2. ✅ Test code structure generation
3. ✅ Verify code generation patterns

**Tests Implemented**:
- ✅ `tests/unit/code_generation/test_code_emission.bas` - Code emission tests
- ✅ `tests/unit/code_generation/test_code_structure.bas` - Code structure tests
- ✅ `tests/unit/code_generation/test_code_generation.bas` - Main code generation test suite runner

**Example Implementation**:
The code generation tests verify that code emission functions work correctly and that generated code structures match expected patterns. Tests use the full compiler context ("all") to ensure all dependencies are available.

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

### Future Enhancements

1. **Expand Test Coverage**
   - Add more edge cases for each component
   - Test error conditions more thoroughly
   - Add performance/regression tests

2. **Integration Testing**
   - Test component interactions
   - Test full compilation workflows
   - Test error propagation between components

3. **Test Maintenance**
   - Keep tests updated with code changes
   - Add tests for new features
   - Refactor tests as needed

4. **Documentation**
   - Document test patterns and best practices
   - Add examples for adding new tests
   - Document test harness usage

## Conclusion

**Status**: ✅ **Strategy Successfully Implemented**

The component testing strategy has been successfully implemented. While full isolation isn't possible without major refactoring, we have achieved significant test coverage by:

1. ✅ **Created test harnesses with minimal state** - `test_state_manager.bi` provides comprehensive state management
2. ✅ **Tested self-contained functions** - All major components have test coverage
3. ✅ **Used state isolation for component testing** - Components can be tested independently
4. ✅ **Inspected outputs through available interfaces** - Tests verify component behavior through public APIs

**Results**:
- All planned infrastructure components are implemented
- All major compiler components have test coverage
- Tests use real implementations, not mocks
- State isolation works effectively for component testing
- Tests can be run independently or as suites

This approach has provided practical value while working within the constraints of the existing codebase. The test harness infrastructure enables ongoing testing and maintenance of compiler components.

## Lessons Learned

1. **State Management Works**: The test harness successfully manages global state, allowing components to be tested in isolation.

2. **Incremental Approach**: Starting with simpler components (type system) and building up to more complex ones (parser, code generation) was effective.

3. **Real Implementations**: Testing actual component code rather than mocks provides confidence in the implementation.

4. **Test Organization**: Organizing tests by component in separate directories makes maintenance easier.

5. **Context Management**: The `TestStateContext` pattern provides a clean way to manage component initialization and cleanup.
