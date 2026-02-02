# ASSERTIONS.md

Documentation of the QB64pe assertions system.

## Overview

QB64pe provides a compile-time assertions system through the `$ASSERTS` metacommand and `_ASSERT` statement, enabling runtime condition checking with detailed error reporting including file and line information.

## Enabling Assertions

### $ASSERTS Metacommand

```basic
$ASSERTS           ' Enable assertions (graphical dialog)
$ASSERTS:CONSOLE   ' Enable assertions (console output)
```

When not enabled, `_ASSERT` statements are compiled out (zero overhead).

## _ASSERT Statement

```basic
_ASSERT condition
_ASSERT condition, "optional message"
```

### Behavior

| Condition | Result |
|-----------|--------|
| True (non-zero) | Continue execution |
| False (zero) | Trigger assertion failure |

### Failure Response

When an assertion fails:

1. **Without $ASSERTS**: Statement ignored (not compiled)
2. **With $ASSERTS**: Display dialog with details, option to continue
3. **With $ASSERTS:CONSOLE**: Print to console, exit program

## Error Codes

| Code | Meaning |
|------|---------|
| 314 | Assertion failed (dialog shown) |
| 315 | Assertion failed (console mode) |

## Compiler Implementation

### Parsing ($ASSERTS)

```basic
' In source/qb64pe.bas
IF a$ = "$ASSERTS" THEN
    asserts_enabled = -1
    asserts_console = 0
END IF
IF a$ = "$ASSERTS:CONSOLE" THEN
    asserts_enabled = -1
    asserts_console = -1
END IF
```

### Code Generation (_ASSERT)

When assertions are enabled:

```c
// Generated C++ for: _ASSERT x > 0, "x must be positive"
if (!(x > 0)) {
    sub__assert("x > 0", "x must be positive", "myfile.bas", 42);
}
```

When assertions are disabled:
```c
// Nothing generated - zero overhead
```

## Runtime Function

```c
// internal/c/libqb/src/error.cpp
void sub__assert(const char* condition, const char* message,
                 const char* file, int32 line) {
    // Format assertion failure message
    // Show dialog or print to console
    // Optionally continue or exit
}
```

### Dialog Display

```
Assertion Failed!

Condition: x > 0
Message: x must be positive
File: myfile.bas
Line: 42

[Continue] [Exit]
```

### Console Output

```
Assertion failed: x > 0
Message: x must be positive
File: myfile.bas, Line: 42
```

## Debug Information

Assertions automatically capture:

| Information | Source |
|-------------|--------|
| Condition text | Parsed from source |
| Custom message | Optional second parameter |
| Source file | `inclession$(inclession)` |
| Line number | `liession` |

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | _ASSERT parsing, $ASSERTS handling |
| `internal/c/libqb/src/error.cpp` | `sub__assert` implementation |
| `internal/c/libqb/include/error.h` | Function declaration |

## Comparison with _ASSERT$ Function

```basic
' _ASSERT statement (condition check)
_ASSERT x > 0, "x must be positive"

' _ASSERT$ function (returns condition as string for debugging)
' Not the same feature - different purpose
```

## Example Usage

### Basic Assertions

```basic
$ASSERTS

DIM x AS INTEGER
x = GetValue()
_ASSERT x >= 0, "Value cannot be negative"
_ASSERT x <= 100, "Value out of range"
```

### Array Bounds Checking

```basic
$ASSERTS

DIM arr(1 TO 100) AS INTEGER

SUB SetElement(index AS INTEGER, value AS INTEGER)
    _ASSERT index >= 1 AND index <= 100, "Index out of bounds"
    arr(index) = value
END SUB
```

### Preconditions and Postconditions

```basic
$ASSERTS

FUNCTION Divide#(a AS DOUBLE, b AS DOUBLE)
    _ASSERT b <> 0, "Division by zero"
    result# = a / b
    _ASSERT NOT (result# <> result#), "Result is NaN"
    Divide# = result#
END FUNCTION
```

### Console Mode for Automated Testing

```basic
$ASSERTS:CONSOLE

' Run tests
_ASSERT TestCase1(), "Test case 1 failed"
_ASSERT TestCase2(), "Test case 2 failed"
_ASSERT TestCase3(), "Test case 3 failed"

PRINT "All tests passed!"
```

### Debug vs Release

```basic
$IF DEBUG THEN
    $ASSERTS
$END IF

' Assertions only active in debug builds
_ASSERT ValidState(), "Invalid program state"
```

## Best Practices

1. **Use meaningful messages**: Describe what went wrong, not just what failed
2. **Check invariants**: Assert conditions that should always be true
3. **Validate inputs**: Check function parameters at entry
4. **Use $ASSERTS:CONSOLE for CI**: Automated testing should use console mode
5. **Remove $ASSERTS for release**: Or use conditional compilation
