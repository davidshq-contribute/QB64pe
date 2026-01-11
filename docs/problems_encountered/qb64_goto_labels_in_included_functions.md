# QB64 GOTO Labels in Included Functions - Compiler Limitation

## Problem

When including files in QB64 that contain SUB/FUNCTION definitions with GOTO labels, the compiler may report "Common label within a SUB/FUNCTION" errors, even when the labels are properly scoped within the function.

## Root Cause

QB64 has a compiler limitation with GOTO labels inside SUB/FUNCTION definitions when those files are included in Phase 3 (after the implicit END statement). The compiler's label resolution mechanism has trouble with labels in included files that are processed after the main program section.

## Solution

Refactor GOTO-based control flow to use structured control flow statements:

### 1. Simple Loop Skip (skipchar pattern)

**Original (problematic):**
```qbasic
FOR i = 1 TO LEN(str$)
    IF condition THEN GOTO skipchar
    ' process character
    skipchar:
NEXT
```

**Refactored (structured):**
```qbasic
FOR i = 1 TO LEN(str$)
    IF condition THEN
        ' Skip this character
    ELSE
        ' process character
    END IF
NEXT
```

### 2. Complex Recheck Logic (recheckdiff pattern)

**Original (problematic):**
```qbasic
WHILE condition
    lcnt = lcnt + 1: ocnt = ocnt + 1
    recheckdiff:
    ' comparison logic
    IF need_recheck THEN GOTO recheckdiff
WEND
```

**Refactored (structured with flag):**
```qbasic
recheck_needed = 0
DO
    IF recheck_needed = 0 THEN
        lcnt = lcnt + 1
        ocnt = ocnt + 1
    END IF
    recheck_needed = 0
    
    ' comparison logic
    IF need_recheck THEN
        ' Handle recheck case (may increment specific counter)
        recheck_needed = -1
    END IF
LOOP WHILE condition
```

## Key Considerations

1. **Counter Increments**: When refactoring recheck logic, ensure counters are incremented at the same points as the original code. The flag-based approach allows skipping the general increment when a specific counter was already incremented.

2. **Exit Conditions**: Preserve all exit conditions from the original code, including boundary checks.

3. **String Modifications**: When strings are modified and recheck is needed, ensure the recheck happens with the same counter values (no increment).

4. **Testing**: After refactoring, test thoroughly with the same inputs that the original code handled, especially edge cases involving:
   - String modifications during comparison
   - Multiple consecutive rechecks
   - Boundary conditions (end of strings)

## Example: format.bas Refactoring

The `apply_layout_indent$` function in `source/utilities/format.bas` was refactored to remove two GOTO labels:

- `skipchar:` - Replaced with IF-ELSEIF-ELSE structure
- `recheckdiff:` - Replaced with DO...LOOP using `recheck_needed` flag

The refactoring maintained exact functional equivalence while making the code compatible with QB64's Phase 3 include processing.

## Prevention

When writing new code for QB64 that will be included in Phase 3:
- Avoid GOTO labels inside SUB/FUNCTION definitions
- Use structured control flow (IF-ELSE, DO-LOOP, WHILE-WEND, FOR-NEXT)
- If GOTO is necessary, consider if the function can be split or the logic restructured

## Related Files

- `source/utilities/format.bas` - Example of successful refactoring
- `tests/unit/TROUBLESHOOTING_LOG.md` - Detailed troubleshooting process
- `source/ide/ide_methods.bas` - Contains similar code that may need refactoring if included
