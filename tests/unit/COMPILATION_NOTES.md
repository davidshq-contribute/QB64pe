# Unit Test Compilation Notes

## Progress
- **Compilation Status**: ~46% complete
- **Major Issues Resolved**: 
  - CONST declaration order
  - DIM SHARED declaration order
  - Include order
  - String variable syntax
  - Function call syntax
  - Quote escaping
  - REDIM SHARED in SUBs

## Known Issues

### VAL Function Overload Issue
**Location**: `source/utilities/elements.bas` lines 519, 522  
**Error**: "Incorrect number of arguments - Reference: VAL(string_value$)"  
**Code**:
```qb64
uintegral = VAL(num$, _UNSIGNED _INTEGER64)
integral = VAL(num$, _INTEGER64)
```

**Status**: VAL with two parameters is supported in QB64-PE (see `tests/compile_tests/overloaded/test.bas`), but the compiler may not recognize the overload in the test compilation context. This is source code, not test code, so it should not be modified.

**Potential Solutions**:
1. This may resolve when compiling with the full compiler (not just test runner)
2. May require compiler version update
3. May need to skip const_eval tests temporarily until resolved

## Next Steps
1. Continue fixing remaining compilation errors
2. Once compilation succeeds, proceed with:
   - Task 2: Check for test interference
   - Task 3: Verify cleanup
