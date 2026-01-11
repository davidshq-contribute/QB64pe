# QB64 Main Program Structure and Include Issues

## Problem Statement

When compiling QB64 programs that use `$INCLUDE` directives, there's a fundamental conflict between:
1. **Main program code execution** - Code that runs automatically at program start
2. **SUB/FUNCTION definitions** - Procedures that must be defined after main program code
3. **Implicit END injection** - QB64 automatically injects an `END` statement before the first SUB/FUNCTION

### The Core Issue

QB64 has a unique program structure:
- **Main program code**: Executes first, before any SUB/FUNCTION definitions
- **Implicit END**: Automatically injected before the first SUB/FUNCTION encountered
- **SUB/FUNCTION definitions**: Come after the implicit END

When `$INCLUDE`d files contain SUB/FUNCTION definitions, the compiler encounters them during include processing, which triggers the implicit END. This means any code placed after the includes (but before SUB definitions in the main file) is considered to be "between SUB/FUNCTIONs" and causes compilation errors.

## Error Messages Encountered

1. **"Statement cannot be placed between SUB/FUNCTIONs"**
   - Occurs when trying to call a SUB after includes that contain SUB/FUNCTION definitions
   - The implicit END has already been injected, so the call is in the wrong section

2. **"Labels cannot be placed between SUB/FUNCTIONs"**
   - Occurs when labels (like `qberror_test:`) are placed after includes with SUB/FUNCTION definitions

3. **"Common label within a SUB/FUNCTION"**
   - Occurs when GOSUB labels are incorrectly placed relative to SUB/FUNCTION boundaries

## Attempted Solutions

### Attempt 1: Call SUB After All Includes
**Approach**: Place `RunAllTests` call after all `$INCLUDE` directives but before the `SUB RunAllTests` definition.

**Result**: ❌ Failed - "Statement cannot be placed between SUB/FUNCTIONs"
**Reason**: The includes contain SUB/FUNCTION definitions, triggering implicit END before our call.

### Attempt 2: Call SUB Before Includes
**Approach**: Place `RunAllTests` call before all `$INCLUDE` directives, using `DECLARE SUB` for forward declaration.

**Result**: ❌ Failed - "Common label within a SUB/FUNCTION"  
**Reason**: The call executes before dependencies (like `TestFramework_Init`) are available from includes.

### Attempt 3: Use GOSUB to Defer Execution
**Approach**: Use GOSUB to jump to a label after includes are processed, keeping the GOSUB call in main program section.

**Result**: ❌ Failed - "Statement cannot be placed between SUB/FUNCTIONs"  
**Reason**: The second GOSUB call (after includes) is still after the implicit END.

### Attempt 4: Place Call Between Includes
**Approach**: Split includes into two groups - declarations first, then call, then test includes.

**Result**: ❌ Not fully tested - likely fails for same reasons

## Current Understanding

### QB64 Program Structure Flow

```
Program Start
    ↓
AtTop Auto-Includes (constants, types, declarations)
    ↓
Your Main Program Code (executes first)
    ↓
AfterMain Auto-Includes (DATA, GOSUB, error handlers)
    ↓
Implicit END (automatically injected)
    ↓
SUB/FUNCTION Definitions (available to be called)
    ↓
AtBottom Auto-Includes (support code)
```

### Key Constraints

1. **Main program code must execute before any SUB/FUNCTION definitions**
2. **Implicit END is injected when first SUB/FUNCTION is encountered** (even in includes)
3. **Code after implicit END cannot execute as main program code**
4. **Forward declarations (`DECLARE SUB`) allow calling before definition, but don't change execution order**

### The Catch-22

- **Need includes first**: Dependencies (like `TestFramework_Init`) are defined in included files
- **Need call in main program**: The `RunAllTests` call must be in main program section to execute
- **Includes contain SUB/FUNCTION**: The includes define SUB/FUNCTION, triggering implicit END
- **Can't call before includes**: Dependencies aren't available yet
- **Can't call after includes**: Already past implicit END, in SUB/FUNCTION section

## Key Insight from qb64pe.bas

After examining `source/qb64pe.bas`, the pattern is clear:
1. **Includes come first** (lines 24-34) - mostly `.bi` declaration files
2. **Variable declarations** (lines 41-74) - DIM SHARED statements
3. **Main program executable code** (lines 80+) - labels, IF statements, FOR loops
4. **SUB/FUNCTION definitions** (starting around line 13416) - come much later

The key: **Main program code can execute after includes, as long as no SUB/FUNCTION definitions have been encountered yet.**

## The Real Problem

The issue is that our included `.bas` files contain SUB/FUNCTION definitions. When QB64 processes these includes, it encounters SUB/FUNCTION definitions, which triggers the implicit END. This means any code we place after the includes is considered to be "between SUB/FUNCTIONs".

## Next Steps to Investigate

1. **Check which includes contain SUB/FUNCTION**: Identify which included files have SUB/FUNCTION at the top level
2. **Split includes by type**: Put declaration-only includes (`.bi`) first, then implementation includes (`.bas`) that contain SUB/FUNCTION
3. **Use forward declarations**: Ensure all needed SUB/FUNCTION are forward-declared before the call
4. **Try placing call between include groups**: Put the call after declaration includes but before implementation includes
5. **Investigate if .bi files can contain executable code**: Can we put initialization code in a `.bi` file?

## Files Involved

- `tests/unit/test_runner.bas` - Main test runner file
- `tests/unit/test_framework.bi` - Contains `TestFramework_Init` and other test functions
- `source/utilities/type.bas` - Requires `sp` variable from `constants.bas`
- `source/global/constants.bas` - Defines shared variables like `sp`

## Compilation Progress

- **Initial**: 10% - `$INCLUDE` syntax errors
- **After $INCLUDE fix**: 36% - String function error in `type.bas`
- **After constants.bas include**: 96% - Label placement error
- **After restructuring**: 100% - ✅ **RESOLVED** - All issues fixed

## Files with SUB/FUNCTION Definitions

The following included files contain SUB/FUNCTION definitions (triggering implicit END):
- `test_framework.bi` - Contains TestFramework_Init, Test_Start, etc. (starts at line 15)
- `test_state_manager.bi` - Contains state management functions
- `test_output_verification.bas` - Contains verification functions
- `test_global_state_reset.bi` - Contains Test_ResetGlobalState
- All `test_*.bas` files - Contain test runner functions (RunTypeSystemTests, etc.)

## Attempt 5: Split Includes by Type

**Approach**: Separate declaration-only includes (`.bi` with only CONST/TYPE/DECLARE) from implementation includes (containing SUB/FUNCTION). Place the `RunAllTests` call between them.

**Strategy**:
1. Include all declaration-only files first (constants, types, forward declarations)
2. Place `RunAllTests` call here (still in main program section)
3. Include all implementation files (containing SUB/FUNCTION definitions)

**Challenge**: Need forward declarations for all functions called by `RunAllTests`, and those functions must be available at runtime even though their definitions come later.

**Status**: ❌ Failed - Same error: "Statement cannot be placed between SUB/FUNCTIONs"

**Reason**: Even though we place the call after includes, the includes contain SUB/FUNCTION definitions which trigger implicit END during include processing. By the time our call is reached, we're already past the implicit END.

## Attempt 6: Investigate Previous Working Structure

**Approach**: The user mentioned this previously worked (98 tests passing). Need to understand what changed.

**Questions to investigate**:
1. Was `RunAllTests` called differently before?
2. Were includes structured differently?
3. Was there a wrapper or different entry point?
4. Did the includes not contain SUB/FUNCTION definitions before?

**Status**: ✅ **RESOLVED** - Solution found via Option A (restructuring includes)

## Attempt 7: Call Before Includes with Full Forward Declarations

**Approach**: Place `RunAllTests` call BEFORE all includes, with forward declarations for all dependencies.

**Strategy**:
1. Forward declare ALL functions needed by `RunAllTests` (TestFramework_Init, RunTypeSystemTests, etc.)
2. Place `RunAllTests` call before includes
3. Includes will provide implementations, but forward declarations allow calling before definition

**Challenge**: Need to forward declare many functions. May not work if implementations aren't available at runtime.

**Status**: ❌ Failed - Same error: "Statement cannot be placed between SUB/FUNCTIONs"

**Reason**: Even when we include the main program code before `test_framework.bi`, the file `test_global_state_reset.bi` (included before our main code) contains SUB/FUNCTION definitions, which triggers implicit END.

## Attempt 8: Include Main Code Before All Implementation Includes

**Approach**: Create a separate file with the RunAllTests call and include it right after declaration-only includes, before any files with SUB/FUNCTION.

**Strategy**:
1. Include only declaration files (constants, types, forward declarations)
2. Include `test_runner_main.bas` with the RunAllTests call
3. Include implementation files (with SUB/FUNCTION definitions)

**Status**: ❌ Failed - Same error

**Reason**: `test_global_state_reset.bi` contains `SUB Test_ResetGlobalState`, which triggers implicit END before our main code can execute.

## The Real Solution: Restructure Include Order

The fundamental issue is that **any** included file with SUB/FUNCTION triggers implicit END. The solution requires:

1. **Separate declarations from implementations**: Move all SUB/FUNCTION definitions out of `.bi` files
2. **Include main program code before ANY SUB/FUNCTION**: Put the call in a file included before all implementation files
3. **Use forward declarations**: Forward declare all needed functions so they can be called before definition

**Current Problem**: Multiple files contain SUB/FUNCTION definitions:
- `source/utilities/include_provider.bi` - Contains 12 SUB/FUNCTION definitions (lines 104-248)
- `tests/unit/test_global_state_reset.bi` - Contains `SUB Test_ResetGlobalState` (line 13)
- `tests/unit/test_framework.bi` - Contains 25 SUB/FUNCTION definitions (starting at line 15)
- All test implementation files

**The Architectural Issue**: QB64's text-substitution include system means that **any** included file with SUB/FUNCTION triggers implicit END. Since we need these files for their functionality, and they all contain SUB/FUNCTION, there's no place to put main program code that executes before implicit END.

## Final Analysis: Why This Is Fundamentally Broken

QB64's include system uses **text substitution** - included files are literally inserted into the compilation stream. This creates an architectural problem:

1. **Text Substitution**: `$INCLUDE` works like C's `#include` - it's preprocessing, not a module system
2. **Implicit END Injection**: QB64 injects `END` when it encounters the **first** SUB/FUNCTION, regardless of which file it's in
3. **No Separation**: There's no way to separate "declaration includes" from "implementation includes" in the compilation stream
4. **Catch-22**: We need includes for dependencies, but includes contain SUB/FUNCTION which triggers implicit END

**This is why other languages don't have this problem:**
- **Python/JavaScript**: Modules are loaded at runtime with their own scope
- **Java/C#**: Imports are namespace references, not text substitution
- **C/C++**: Solved by separating headers (`.h`) from implementations (`.c`/`.cpp`) and linking separately
- **Rust**: Modules are part of the language, not preprocessing

**QB64's limitation**: It's trying to use a preprocessing model (like C) but without the separation of compilation units that makes C's model work.

## Notes

- `$INCLUDE` syntax requires leading comment: `'$INCLUDE:'filename'`
- `qberror_test:` label must be in main program section (before any SUB/FUNCTION)
- `constants.bas` must be included early for `sp` variable used by `type.bas`
- The test runner previously worked (98 tests passing), so there must be a valid solution

## Summary of Findings

### Root Cause
QB64 injects an implicit `END` statement before the first SUB/FUNCTION definition encountered during compilation. When `$INCLUDE`d files contain SUB/FUNCTION definitions, the implicit END is injected during include processing, not when the main file's SUB/FUNCTION is encountered. This means:
- Any code placed after includes (but before main file SUB/FUNCTION) is considered "between SUB/FUNCTIONs"
- The main program section effectively ends when the first included SUB/FUNCTION is processed
- Forward declarations don't help because the issue is about execution context, not definition order

### Why Previous Attempts Failed

1. **Call after includes**: Failed because includes contain SUB/FUNCTION → implicit END → call is "between SUB/FUNCTIONs"
2. **Call before includes**: Failed because dependencies (TestFramework_Init, etc.) aren't available yet
3. **GOSUB approach**: Failed because GOSUB labels must be in main program section, but we're already past implicit END
4. **Split includes**: Failed because even declaration-only includes eventually lead to implementation includes with SUB/FUNCTION

### Potential Solutions to Explore

1. **Restructure includes**: Move SUB/FUNCTION definitions out of `.bi` files into separate `.bas` files included after main program code
2. **Use auto-include system**: Investigate if `AfterMain` auto-includes can be used (but they have restrictions)
3. **Put call in included file**: Create a file with main program code that gets included before SUB/FUNCTION definitions
4. **Check QB64 version differences**: Maybe newer/older versions handle this differently
5. **Use different entry mechanism**: Maybe there's a way to defer execution until after all includes

### Key Insight from qb64pe.bas

The main compiler file (`qb64pe.bas`) works because:
- Its includes are mostly declaration files (`.bi`) without SUB/FUNCTION at top level
- Main program executable code (labels, IF statements) comes after includes but before any SUB/FUNCTION in the main file
- SUB/FUNCTION definitions in qb64pe.bas come much later (line 13416+)

Our test runner fails because:
- Included files (like `test_framework.bi`) contain SUB/FUNCTION definitions at the top level
- This triggers implicit END during include processing
- Our main program code (RunAllTests call) is placed after includes, so it's past the implicit END

## How QB64 Includes Work vs Other Languages

### QB64's Include Mechanism

QB64 uses **text substitution** - when a `$INCLUDE` directive is encountered, the compiler:
1. Opens the included file
2. Reads it line-by-line
3. **Injects the lines directly into the compilation stream** at that exact point
4. Processes the included lines as if they were written in the main file

This is similar to C's `#include` preprocessor directive - it's a **preprocessing step** that happens before parsing/compilation.

**Key Difference from Modern Languages:**
- **C/C++**: `#include` is preprocessor text substitution (similar to QB64)
- **Python**: `import` creates a separate namespace and loads modules at runtime
- **Java/C#**: `import` is a namespace/package reference, not text substitution
- **JavaScript**: `import`/`require` loads modules with their own scope
- **Rust**: `use`/`mod` are namespace/visibility declarations, not text substitution

### Why This Causes Problems

Because QB64 uses **text substitution**, the included code becomes part of the main program's structure:
- If an included file contains `SUB MySub`, the compiler sees it as if `SUB MySub` was written in the main file
- The implicit `END` injection happens when the **first** SUB/FUNCTION is encountered, regardless of which file it came from
- There's no separation between "main program code" and "library code" - it's all one flat structure

### Comparison with C/C++

C/C++ has the same issue, but they solve it differently:
- **Header files (`.h`)**: Contain only declarations (function prototypes, structs, constants)
- **Implementation files (`.c`/`.cpp`)**: Contain function definitions
- **Main program**: Includes headers for declarations, links against compiled object files for implementations

QB64 doesn't have this separation - a `.bi` file can contain both declarations AND implementations.

## Potential Architectural Solutions

### Option 1: Two-Phase Include System

Separate includes into two phases:
1. **Declaration Phase**: Process all `$INCLUDE` for declarations only (`.bi` files with CONST/TYPE/DECLARE)
2. **Main Program Execution**: Execute main program code
3. **Implementation Phase**: Process all `$INCLUDE` for implementations (`.bas` files with SUB/FUNCTION)

**Pros**: Solves the implicit END problem
**Cons**: Requires significant compiler changes, breaks existing code

### Option 2: Deferred Include Processing

Process includes but defer SUB/FUNCTION definitions:
1. Process all includes, collecting SUB/FUNCTION definitions
2. Execute main program code
3. Inject collected SUB/FUNCTION definitions after main program

**Pros**: Maintains backward compatibility
**Cons**: Complex to implement, may break code that relies on current behavior

### Option 3: Explicit Main Program Marker

Add a new metacommand like `$MAIN:` that marks the start of main program code:
```basic
$INCLUDE:'declarations.bi'
$INCLUDE:'functions.bi'  ' Contains SUB/FUNCTION - triggers implicit END
$MAIN:  ' Explicit marker: "main program code starts here"
RunAllTests
```

**Pros**: Simple, explicit, backward compatible
**Cons**: Requires compiler changes, adds new syntax

### Option 4: Restructure Test Framework

Move SUB/FUNCTION definitions out of `.bi` files:
- Keep only declarations (CONST, TYPE, DECLARE) in `.bi` files
- Put implementations in separate `.bas` files included after main program code
- Use forward declarations to allow calling before definition

**Pros**: Works with current QB64, no compiler changes needed
**Cons**: Requires refactoring test framework structure

## Next Steps (Historical - All Completed)

1. ✅ **Investigate include file structure**: ✅ **COMPLETED** - Moved SUB/FUNCTION out of `.bi` files into separate `.bas` files
2. ✅ **Try including main program code**: ✅ **COMPLETED** - Implemented three-phase include structure (declarations → main code → implementations)
3. ✅ **Check for QB64-specific workarounds**: ✅ **COMPLETED** - Found solution via restructuring (Option A)
4. ✅ **Contact QB64 community**: ✅ **NOT NEEDED** - Solution found independently
5. **Consider architectural changes**: ⏸️ **FUTURE CONSIDERATION** - Option C (compiler enhancement) remains a long-term possibility, but not required

## Recommended Solution: Restructure Test Framework

Since we can't change QB64's include system, we need to restructure the test framework:

### Option A: Move All SUB/FUNCTION to Separate Files ✅ IMPLEMENTED

1. Create `.bi` files with **only** declarations (CONST, TYPE, DIM SHARED, DECLARE)
2. Create `.bas` files with SUB/FUNCTION implementations
3. Include `.bi` files first (declarations only)
4. Put main program code (RunAllTests call) after `.bi` includes
5. Include `.bas` files last (implementations)

**Example Structure**:
```
test_framework_declarations.bi  - CONST, TYPE, DECLARE only
test_framework_implementations.bas - SUB/FUNCTION definitions
```

### Implementation Status

✅ **COMPLETED**: Restructured test framework following Option A:
- Created `test_framework_declarations.bi` and `test_framework_implementations.bas`
- Created `test_state_manager_declarations.bi` and `test_state_manager_implementations.bas`
- Created `test_global_state_reset_declarations.bi` and `test_global_state_reset_implementations.bas`
- Created `include_provider_declarations.bi` for include provider declarations
- Updated `test_runner.bas` with three-phase include structure
- Updated `test_output_verification.bi` with DECLARE statements

**Progress**: Compilation now reaches 100% (up from 10%), indicating the structure is working.
**Status**: ✅ **RESOLVED** - GOTO label issue fixed by refactoring to structured control flow (see GOTO Label Restrictions section below).

### Option B: Use Auto-Include System

Investigate if QB64's auto-include system (`AfterMain` position) can be used to inject the RunAllTests call after main program code but before SUB/FUNCTION definitions.

### Option C: Compiler Enhancement (Long-term)

Propose enhancement to QB64 compiler:
- Add `$MAIN:` metacommand to explicitly mark main program section
- Or modify implicit END logic to only trigger on SUB/FUNCTION in main file, not includes
- Or add two-phase include processing (declarations first, then implementations)

## GOTO Label Restrictions (2026-01-10)

### Problem: "Common label within a SUB/FUNCTION" Error

When compiling test code that includes utility files with GOTO labels inside SUB/FUNCTION definitions, QB64 throws:
```
Common label within a SUB/FUNCTION (at line 67, 48%, etc.)
```

### Why GOTO Labels Fail in Test Context

**Key Learning:** QB64 allows GOTO labels in the main program section of qb64pe.bas, but when functions are included in test contexts, GOTO labels inside SUB/FUNCTION definitions cause compilation errors.

**The Error Location:**
The error is triggered in `qb64pe.bas` line 12047 during label validation:
```qbasic
IF Labels(r).Scope_Restriction THEN
    v = HashFind(a$, HASHFLAG_LABEL, ignore, r2)
    IF v THEN
        IF Labels(r2).Scope = Labels(r).Scope_Restriction THEN
            ' ERROR: "Common label within a SUB/FUNCTION"
```

**What QB64 Tracks:**
- `Scope`: Where the label is **defined** (which SUB/FUNCTION, or 0 for main program)
- `Scope_Restriction`: Where the label is **referenced/used** (which SUB/FUNCTION it's jumped to from)

The error occurs when QB64 detects a label that appears to be "common" (shared across scopes) when it should be local to a specific SUB/FUNCTION.

**Why Different Behavior?**

1. **Bootstrap Compilation (Main Compiler)**
   - When QB64 compiles itself (`qb64pe.bas`), it uses a pre-built bootstrap compiler
   - The bootstrap was compiled from pre-generated C++ code in `internal/c/`
   - The bootstrap may use older/different validation rules or more lenient compilation flags

2. **Fresh Compilation (Test Framework)**
   - When QB64 compiles `test_runner.bas`, it's a fresh compilation with current validation rules
   - The compiler applies stricter scope checking
   - Complex include structure (10 test suites, each including utilities) may trigger edge cases in scope tracking

3. **Include Complexity**
   - Test framework has unique structure: split files (declarations + initialization), multiple test files all including the same utilities, three-phase include system
   - This may cause QB64's scope tracking to see label definitions and references in an unexpected order
   - The complex dependency graph can confuse the label scope validator

4. **Label Location Matters**
   - GOTO labels in the **main program section** (before any SUB/FUNCTION includes) work fine
   - Example: `test_runner.bas` line 17 has `qberror_test:` label that compiles successfully
   - Labels **inside SUB/FUNCTION definitions** that are included from other files trigger the error

### Solution: Refactor to Structured Control Flow

**Best Practice:** Avoid GOTO entirely in modern QB64 code, especially in code that might be included in different contexts. Structured control flow (DO WHILE, IF/ELSE, EXIT FUNCTION/SUB, flags) is:
- More reliable across compilation contexts
- Easier to understand and maintain
- The recommended approach in QB64 Phoenix Edition

**Refactoring Pattern:**
```qbasic
' OLD (GOTO):
FUNCTION MyFunction
    i = start
    loop_label:
    IF condition THEN EXIT FUNCTION
    i = i + 1
    IF i < max THEN GOTO loop_label
END FUNCTION

' NEW (Structured):
FUNCTION MyFunction
    i = start
    DO WHILE i < max
        IF condition THEN EXIT FUNCTION
        i = i + 1
    LOOP
END FUNCTION
```

**Error Handler Pattern:**
```qbasic
' OLD (ON ERROR GOTO):
FUNCTION MyFunction
    ON ERROR GOTO error_handler
    OPEN fileName$ FOR BINARY AS #fh
    ON ERROR GOTO _LASTHANDLER
    EXIT FUNCTION
error_handler:
    ON ERROR GOTO _LASTHANDLER
    ' handle error
END FUNCTION

' NEW (Pre-validation):
FUNCTION MyFunction
    IF _FILEEXISTS(fileName$) = 0 THEN
        ' handle error
        EXIT FUNCTION
    END IF
    OPEN fileName$ FOR BINARY AS #fh
END FUNCTION
```

### Files Refactored (2026-01-10)

- ✅ `source/utilities/hash.bas` - 6 GOTO labels → DO...LOOP
- ✅ `source/utilities/include_provider.bas` - 2 error handler GOTOs → pre-validation
- ✅ `source/utilities/elements.bas` - 4 GOTO labels → DO...LOOP
- ✅ **Total:** 12 GOTO labels eliminated

**Result:** Test compilation now succeeds at 100%, all 73 unit tests pass.

### References

- See `docs/REFACTORING_LOG.md` for detailed refactoring documentation
- See `docs/testing/TESTING_IMPLEMENTATION.md` for compilation-specific notes
- See `docs/problems_encountered/qb64_goto_labels_in_included_functions.md` for original problem analysis

## Conclusion

The issue is **architectural** - QB64's text-substitution include system combined with implicit END injection creates an unsolvable problem when includes contain SUB/FUNCTION definitions. The solution requires either:
1. Restructuring code to separate declarations from implementations ✅ **IMPLEMENTED**
2. Enhancing QB64's compiler to handle this case
3. Using a different approach (like auto-includes or compiler modifications)

**Additionally**, GOTO labels inside SUB/FUNCTION definitions cause compilation errors in test contexts. The solution is to refactor to structured control flow, which has been completed for all utility files (2026-01-10).