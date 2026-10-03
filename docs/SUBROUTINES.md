# SUBROUTINES.md

Documentation of the QB64pe SUB/FUNCTION system implementation.

## Overview

QB64pe compiles SUBs and FUNCTIONs to C functions with support for recursion, parameter passing by reference/value, and scope management.

## SUB Declaration

### Syntax

```basic
SUB name [(parameters)]
    statements
END SUB
```

### Code Generation

```c
void SUB_<name>(parameters) {
    // Local variable declarations
    // Body statements
    exit_subfunc:
    // Cleanup code
}
```

## FUNCTION Declaration

### Syntax

```basic
FUNCTION name [(parameters)] [AS type]
    statements
    name = returnvalue
END FUNCTION
```

### Code Generation

```c
returntype FUNC_<name>(parameters) {
    returntype __FUNC_<name>;  // Return variable
    // Body statements
    exit_subfunc:
    return __FUNC_<name>;
}
```

## Parameter Passing

### Default: By Reference (BYREF)

```basic
SUB Modify(x AS INTEGER)
    x = x + 1  ' Modifies original
END SUB
```

**Generated C:**
```c
void SUB_Modify(int16 *x) {
    *x = *x + 1;
}
```

### By Value (BYVAL)

Only in DECLARE LIBRARY:
```basic
DECLARE LIBRARY
    SUB cfunc(BYVAL x AS LONG)
END DECLARE
```

## SHARED Variables

### Declaration

```basic
DIM SHARED globalVar AS INTEGER

SUB MySub
    SHARED localShare AS STRING
END SUB
```

### Implementation

- Bit 0 (value 1): Globally shared
- Bit 1 (value 2): Temporarily shared (COMMON)
- Shared variables persist for entire program lifetime

## STATIC Variables

### Declaration

```basic
SUB Counter
    STATIC count AS INTEGER
    count = count + 1
END SUB
```

- Variables persist between calls
- Stored in static memory pool
- Initialized only on first call

## Scope Rules

### Hierarchy

1. **Local**: Parameters and DIM variables
2. **Static Local**: STATIC variables
3. **Module Shared**: SHARED declaration
4. **Global**: Variables outside procedures

## Recursion Support

Each procedure call gets its own frame:
- Stack grows via `mem_static_pointer`
- Memory restored on return
- Limited by `mem_static_size` (auto-grows)

## DECLARE LIBRARY

### Types

| Syntax | Purpose |
|--------|---------|
| `DECLARE LIBRARY "lib"` | Static C library |
| `DECLARE DYNAMIC LIBRARY "lib"` | Dynamic library |
| `DECLARE CUSTOMTYPE LIBRARY "lib"` | Custom type |

### ALIAS Keyword

```basic
DECLARE LIBRARY
    SUB myfunc ALIAS "_actual_func"
END DECLARE
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | SUB/FUNCTION parsing |
| Lines 4864-5546 | Procedure handling |
| `internal/c/libqb.cpp` | Runtime support |

## Example Usage

### Basic SUB

```basic
SUB Greet(name AS STRING)
    PRINT "Hello, "; name; "!"
END SUB
```

### Recursion

```basic
FUNCTION Factorial&&(n AS _INTEGER64)
    IF n <= 1 THEN
        Factorial = 1
    ELSE
        Factorial = n * Factorial(n - 1)
    END IF
END FUNCTION
```

### STATIC Variables

```basic
SUB UniqueID$
    STATIC nextID AS LONG
    nextID = nextID + 1
    UniqueID = "ID-" + _TRIM$(STR$(nextID))
END SUB
```
