# COMMON-PITFALLS.md

Known gotchas, non-obvious behavior, and common mistakes in QB64pe.

## Critical Issues

### 1. Operator Precedence with Functions

**The Problem:** Functions like `Len()`, `Int()`, `Abs()` are not automatically parenthesized when followed by binary operators.

**Example that was broken:**
```basic
Print Len(f()) / 4    ' Precedence issue with division
```

**Fix:** The compiler now handles this, but be aware that complex expressions with function calls may have surprising precedence.

**Location:** Fixed in commit `56757cd88`

### 2. Temporary String Cleanup Index

**The Problem:** The first index in `qbs_tmp_list` MUST be 0.

**From the code:**
```cpp
intptr_t *qbs_tmp_list = (intptr_t *)calloc(65536 * sizeof(intptr_t), 1);
// first index MUST be 0
```

**What can go wrong:** Modifying temporary string handling without understanding this invariant causes memory corruption.

**Location:** `internal/c/libqb/src/qbs.cpp` line 72

### 3. Numeric Conversion Precision

**The Problem:** String-to-number and number-to-string conversions have hardcoded assumptions about sprintf output length.

**From the code:**
```cpp
l = sprintf((char *)&qbs_str_buffer, "% .6E", value);
// IMPORTANT: assumed l==14
```

**Critical limits:**
- SINGLE: First 7 digits MUST be scanned when exponent is ±39
- DOUBLE: First 8 digits MUST be scanned when exponent is ±309

**What can go wrong:** Off-by-one errors cause silent numeric truncation.

**Location:** `internal/c/libqb/src/qbs_str.cpp`, `internal/c/libqb.cpp`

## Auto-Include State Machine

### 4. State Transition Order

**The Problem:** The auto-include system uses a state machine with specific order requirements.

**States:**
- 0 = inactive
- 1 = triggered
- 2 = in progress
- 3 = done

**The three injection points must progress in order:**
1. `firstLine` (AtTop)
2. `mainEndLine` (AfterMain)
3. `lastLine` (AtBottom)

**What can go wrong:** Modifying state transitions can break library injection or debug support.

### 5. Marker Bar for SUB-less Programs

**The Problem:** For programs without SUB/FUNCTION definitions, AfterMain and AtBottom are triggered simultaneously. A marker bar separates them.

**What can go wrong:** Removing or modifying the marker bar breaks auto-include ordering.

## PRINT Statement

### 6. Auto-Semicolon Insertion

**The Problem:** PRINT automatically inserts semicolons between certain token pairs.

**Semicolons ARE inserted between:**
- Numeric literals and string literals: `Print 123"abc"`
- Variables and strings: `Print x$"abc"`
- Function calls and strings: `Print Int(1.1)"abc"`

**Semicolons are NOT inserted:**
- Around concatenation: `Print "abc"+"def"`
- Around comparisons: `Print "abc">"def"`
- Inside parentheses: `Print ("abc") + "def"`

**Location:** Fixed edge cases in commit `eaf084395`

## Type System

### 7. Type Checking with AND vs =

**WRONG:**
```basic
IF t = ISSTRING THEN  ' Almost never matches!
```

**RIGHT:**
```basic
IF t AND ISSTRING THEN  ' Correct bitwise check
```

**Why:** Type values have multiple flags set. Equality check only works for exact matches.

### 8. Array Type Location

**The Problem:** Array element type is NOT in `id.t`.

**Wrong:**
```basic
elementType = id.t  ' Usually 0 for arrays!
```

**Right:**
```basic
elementType = id.arraytype
```

### 9. Fixed-Length String Size

**The Problem:** Fixed-length string size is in `id.tsize`, not the type bits.

```basic
IF (t AND ISSTRING) AND (t AND ISFIXEDLENGTH) THEN
    length = id.tsize  ' Not (t AND 511)
END IF
```

## IDE vs CLI Differences

### 10. Mode-Specific Behavior

**The Problem:** The compiler behaves differently in IDE mode vs CLI mode.

**Differences:**
- Input: IDE uses command protocol, CLI reads files
- Error reporting: IDE shows in editor, CLI prints to console
- Metacommand processing differs

**What can go wrong:** Bugs that only manifest in one mode won't be caught by testing the other.

## Source Ordering

### 11. Forward References

**Mostly fixed (Jan 2026):** SUB/FUNCTIONs can now be interleaved with main code.

**Still be careful with:**
- Forward references to SUBs may not always work in generated C++
- DATA statements and RESTORE labels
- The compiler does two passes to handle these cases

## Hash Table Lookups

### 12. Hash Collision Performance

**The Problem:** Symbol lookup uses hash table with collision chaining.

**The hash function** uses only:
- First 5 characters
- Last 2 characters
- Length

**What can go wrong:**
- Many similarly-named identifiers cause slow lookups
- Function overload resolution is O(n) through collision chain

## Global State

### 13. RCStateVar Triggers Recompilation

**The Problem:** Changing certain configuration variables triggers a full recompile.

**These variables trigger recompile when changed:**
- `ColorSet` ($COLOR)
- `AssertsOn` ($ASSERTS)
- `ConsoleOn` ($CONSOLE)
- `vWatchOn` ($DEBUG)

**What can go wrong:** Unexpected recompilation loops if state is toggled.

### 14. Control Level Array Bounds

**The Problem:** Nested control structures are tracked with fixed-size arrays.

```basic
DIM controllevel
REDIM controltype(1000)
```

**What can go wrong:** Deeply nested code (1000+ levels) can overflow.

## Legacy Compatibility

### 15. Default Palette Initialization

**The Problem:** QB's default palette doesn't match computed values.

**From the code:**
```cpp
//*IMPORTANT* QB sets initial values up different to default palette!
```

**What can go wrong:** Changing palette initialization breaks QB4.5 compatibility.

### 16. DEFLNG A-Z in Compiler

**The Problem:** The compiler itself uses `DEFLNG A-Z`, making implicit declarations LONG.

**What can go wrong:** When reading compiler code, remember that undeclared variables are LONG, not INTEGER.

## Memory Safety

### 17. Game Controller Name Field

**The Problem:** The name field is modified by implementation.

```cpp
char *name; // FIXME: this is modified by game_controller.cpp
```

**What can go wrong:** Code assuming `name` is constant may have memory safety issues.

### 18. Error State Direct Modification

**The Problem:** Some code directly modifies error state instead of using APIs.

```cpp
// FIXME: Should be removed in the future, use `is_error_pending()`.
```

**What can go wrong:** Race conditions and error masking bugs.

## Constant Evaluation

### 19. Compile-Time vs Runtime Evaluation

**The Problem:** `const_eval.bas` must match runtime evaluation exactly.

**What can go wrong:**
- Operator precedence differs between compile-time and runtime
- Complex expressions may fail at compile-time but work at runtime (or vice versa)

## Summary Table

| Issue | Severity | Category |
|-------|----------|----------|
| Temporary string index must be 0 | CRITICAL | Memory |
| Numeric conversion precision | CRITICAL | Data |
| Type checking with AND vs = | HIGH | Type System |
| Auto-include state order | HIGH | Architecture |
| Operator precedence with functions | HIGH | Expressions |
| Array type location | MEDIUM | Type System |
| IDE vs CLI differences | MEDIUM | Testing |
| Hash collision performance | MEDIUM | Performance |
| Control level array bounds | MEDIUM | Limits |
| Auto-semicolon insertion | MEDIUM | PRINT |
| Forward references | LOW | Source Order |
| Default palette values | LOW | Compatibility |

## Best Practices

1. **Always use bitwise AND for type checks**
2. **Test in both IDE and CLI modes**
3. **Be careful with the auto-include state machine**
4. **Don't modify temporary string handling without understanding invariants**
5. **Remember array element type is in `arraytype`, not `t`**
6. **Watch for precedence issues in complex expressions**
7. **Preserve QB4.5 compatibility quirks unless explicitly changing them**
