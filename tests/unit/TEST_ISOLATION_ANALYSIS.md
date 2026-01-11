# Test Isolation and SHARED Declarations Analysis

## Task 1: Verify SHARED Declarations Work

### Status: ✅ SHARED Declarations Properly Structured

All SHARED declarations have been properly organized:

1. **Centralized Declarations** (`test_global_state_declarations.bi`):
   - All test-related SHARED variables declared in one place
   - Includes: Error_Happened, Error_Message, recompile, ConfigFile$, os$, pathsep$, tmpdir$, UseSystemMinGW, layout$, IDEAutoIndent, etc.
   - Test framework variables: testStats, currentTestName$, testOutput$, testVerbose
   - Output verification arrays: snapshotSymbols, snapshotLines

2. **Proper Include Order**:
   - CONST declarations come first
   - DIM SHARED declarations come before SUB/FUNCTION declarations
   - All includes properly ordered in `test_runner.bas`

3. **State Reset Mechanism** (`test_global_state_reset.bi`):
   - `Test_ResetGlobalState` SUB resets all SHARED variables to safe defaults
   - Called automatically by `Test_Start` before each test

### Verification Points:
- ✅ All SHARED variables declared before SUB/FUNCTION declarations
- ✅ Test_Start automatically calls Test_ResetGlobalState
- ✅ State reset function properly resets all declared SHARED variables

## Task 2: Check for Test Interference

### Status: ✅ Tests Designed for Isolation, ⚠️ Some Potential Issues

### Good Practices Found:

1. **Automatic State Reset**:
   - `Test_Start` automatically calls `Test_ResetGlobalState` before each test
   - Ensures clean state for every test

2. **Component State Management**:
   - Tests use `TestState_Init` and `TestState_Cleanup` for component-specific state
   - Pattern: `TestState_Init context, "component"` → test code → `TestState_Cleanup context` → `Test_End`
   - State contexts track initialization and save/restore state

3. **Proper Cleanup**:
   - All tests follow the pattern of cleanup before ending
   - Hash table state is properly saved and restored

### Potential Issues Found:

1. **Hash Table State Restoration** (`test_state_manager.bi` lines 116-128):
   ```qb64
   IF context.savedHashListSize > 0 THEN
       ' Restore previous state - set size variables and REDIM arrays to match
       HashListSize = context.savedHashListSize
       HashListNext = context.savedHashListNext
       ' REDIM arrays to match saved sizes
       ' Note: Array contents are not restored (they will be empty/new)
   ```
   **Issue**: When restoring saved state, array contents are not cleared. If a test runs after another test that left data in the hash table, the arrays will have the correct size but may contain stale data.
   
   **Recommendation**: After REDIM, explicitly clear the arrays or call HashClear if the saved state indicates an empty table.

2. **Type System State** (`test_state_manager.bi` line 167):
   ```qb64
   ' Type system doesn't need cleanup (constants are read-only)
   context.typeSystemInitialized = 0
   ```
   **Status**: ✅ Correct - type system constants are read-only, no cleanup needed

3. **Constant Evaluation Arrays** (`test_state_manager.bi` lines 149-158):
   - Arrays are REDIM'd to match saved sizes
   - Array contents are not restored (will be empty/new)
   - **Status**: ✅ This is correct behavior - arrays are cleared by REDIM

### Test Order Independence Analysis:

**Tests that modify shared state:**
- Hash table tests: Use `TestState_Init context, "hash"` and `TestState_Cleanup`
- Type system tests: Use `TestState_Init context, "type"` (read-only, safe)
- Const eval tests: Use `TestState_Init context, "const"` and `TestState_Cleanup`

**Tests that should be order-independent:**
- ✅ Symbol table tests: Properly initialize and cleanup hash table
- ✅ Type system tests: Only read constants (read-only)
- ✅ Error handling tests: Reset Error_Happened and Error_Message
- ✅ File utilities tests: Reset os$, pathsep$, tmpdir$
- ✅ String utilities tests: Reset ConfigFile$
- ✅ Format tests: Reset layout$, IDEAutoIndent, etc.

### Recommendations:

1. **Improve Hash Table Cleanup**:
   - After restoring saved state with REDIM, consider explicitly zeroing array elements or calling HashClear if the table should be empty
   - Or ensure HashClear is always called when there's no saved state (already done)

2. **Add Test Order Randomization**:
   - Consider adding a test mode that randomizes test execution order
   - This would help catch any remaining interference issues

3. **Verify Snapshot Arrays**:
   - `snapshotSymbols` and `snapshotLines` are shared arrays
   - Ensure tests that use these don't interfere with each other
   - Current implementation: Each test initializes its own snapshot, which should be safe

### Conclusion:

✅ **SHARED declarations are properly structured and should work correctly**

✅ **Tests are designed for isolation with automatic state reset**

⚠️ **Minor improvement needed**: Hash table state restoration could be more explicit about clearing array contents

✅ **Tests should be able to run in any order** with the current implementation, though the hash table cleanup improvement would make this more robust
