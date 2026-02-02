# BUILTIN-FUNCTIONS.md

Documentation of the QB64pe built-in function implementation system.

## Overview

Built-in functions are registered at compiler startup and mapped to C/C++ implementations. The system supports function overloading, type checking, and feature dependencies.

## Core Files

| File | Lines | Purpose |
|------|-------|---------|
| `source/subs_functions/subs_functions.bas` | 4,309 | Function registration |
| `source/subs_functions/syntax_highlighter_list.bas` | 153 | IDE keyword list |
| `source/subs_functions/extensions/opengl/` | - | OpenGL bindings |

## Registration Pattern

All built-in functions are registered in the `reginternal` SUB at startup:

```basic
clearid                              ' Reset id structure
id.n = "FunctionName"                ' QB64 name
id.subfunc = 1                       ' 1=FUNCTION, 2=SUB
id.callname = "func__myfunc"         ' C function name
id.args = 2                          ' Argument count
id.arg = MKL$(TYPE1) + MKL$(TYPE2)   ' Encoded argument types
id.ret = RETURNTYPE                  ' Return type (functions)
id.musthave = ""                     ' Required suffix (e.g., "$")
id.specialformat = "?[,?]"           ' Argument format
id.hr_syntax = "_MYFUNC(a&, b$)"     ' Human-readable syntax
id.Dependency = DEPENDENCY_NONE      ' Feature dependency
regid                                ' Register the identifier
```

## SUB vs FUNCTION

### SUB Pattern (subfunc = 2)

```basic
clearid
id.n = "_Delay"
id.subfunc = 2                       ' This is a SUB
id.callname = "sub__delay"
id.args = 1
id.arg = MKL$(DOUBLETYPE - ISPOINTER)
id.hr_syntax = "_DELAY seconds!"
regid
```

**C Implementation:**
```c
void sub__delay(double seconds) {
    // Implementation
}
```

### FUNCTION Pattern (subfunc = 1)

```basic
clearid
id.n = "_FileExists"
id.subfunc = 1                       ' This is a FUNCTION
id.callname = "func__fileexists"
id.args = 1
id.arg = MKL$(STRINGTYPE - ISPOINTER)
id.ret = LONGTYPE - ISPOINTER        ' Return type
id.hr_syntax = "_FILEEXISTS(fileName$)"
regid
```

**C Implementation:**
```c
int32_t func__fileexists(qbs *fileName) {
    // Implementation
    return result;
}
```

## Type Encoding

### Basic Types

| Type Constant | QB64 Type |
|---------------|-----------|
| `BYTETYPE - ISPOINTER` | _BYTE |
| `UBYTETYPE - ISPOINTER` | _UNSIGNED _BYTE |
| `INTEGERTYPE - ISPOINTER` | INTEGER |
| `UINTEGERTYPE - ISPOINTER` | _UNSIGNED INTEGER |
| `LONGTYPE - ISPOINTER` | LONG |
| `ULONGTYPE - ISPOINTER` | _UNSIGNED LONG |
| `INTEGER64TYPE - ISPOINTER` | _INTEGER64 |
| `SINGLETYPE - ISPOINTER` | SINGLE |
| `DOUBLETYPE - ISPOINTER` | DOUBLE |
| `STRINGTYPE - ISPOINTER` | STRING |
| `OFFSETTYPE - ISPOINTER` | _OFFSET |
| `UDTTYPE + (1)` | _MEM type |

### Special Argument Codes

| Code | Meaning |
|------|---------|
| -1 | Any numeric type (C cast applied) |
| -2 | Offset + Size (largest safe block) |
| -3 | Offset + Size (restricted) |
| -4 | Offset + Size (element size) |
| -5 | Offset only |
| -6 | Size only |
| -7 | _MEM structure for variable |
| -8 | Element reference for arrays |

### Multiple Arguments

Arguments concatenated with `MKL$()`:

```basic
id.arg = MKL$(LONGTYPE - ISPOINTER) + _
         MKL$(STRINGTYPE - ISPOINTER) + _
         MKL$(DOUBLETYPE - ISPOINTER)
```

## Function Overloading

Multiple registrations with same name but different signatures:

```basic
' SUB version: _BLINK {On|Off}
clearid
id.n = "_Blink"
id.subfunc = 2
id.callname = "sub__blink"
id.args = 1
id.arg = MKL$(LONGTYPE - ISPOINTER)
id.specialformat = "{On|Off}"
regid

' FUNCTION version: x = _BLINK
clearid
id.n = "_Blink"
id.subfunc = 1
id.callname = "func__blink"
id.args = 0
id.ret = LONGTYPE - ISPOINTER
regid
```

**Resolution:**
- Matched by function name + argument count + required suffixes
- `subfunc` value determines function vs sub
- `musthave` field creates distinct overloads

## The id Structure

**Location:** `source/qb64pe.bas` lines 545-591

```basic
TYPE idstruct
    n AS STRING * 256           ' Function name
    cn AS STRING * 256          ' Case-sensitive name
    subfunc AS INTEGER          ' 1=Function, 2=Sub
    callname AS STRING * 256    ' C/C++ function name
    args AS INTEGER             ' Number of arguments
    minargs AS INTEGER          ' Minimum arguments
    arg AS STRING * 400         ' Encoded argument types
    ret AS LONG                 ' Return type
    musthave AS STRING * 8      ' Required suffix
    mayhave AS STRING * 8       ' Optional suffix
    specialformat AS STRING * 256 ' Argument format
    hr_syntax AS STRING         ' Human-readable syntax
    Dependency AS INTEGER       ' Feature dependency
    ' ... more fields
END TYPE
```

## Special Format Syntax

The `specialformat` field describes argument parsing:

| Format | Meaning |
|--------|---------|
| `?` | Required argument |
| `[?]` | Optional argument |
| `?,?` | Two required arguments |
| `?[,?]` | One required, one optional |
| `{On\|Off}` | Keyword choice |
| `?{To}?` | Keyword between arguments |

## Feature Dependencies

```basic
id.Dependency = DEPENDENCY_MINIAUDIO  ' Requires audio
```

| Constant | Feature |
|----------|---------|
| `DEPENDENCY_MINIAUDIO` | Sound system |
| `DEPENDENCY_SCREENIMAGE` | Graphics |
| `DEPENDENCY_GL` | OpenGL |
| `DEPENDENCY_LOADFONT` | Font loading |
| `DEPENDENCY_ICON` | Window icons |
| `DEPENDENCY_DEVICEINPUT` | Input devices |
| `DEPENDENCY_PRINTER` | Printing |

## Example Implementations

### Simple Time Function

**Registration:**
```basic
clearid
id.n = "_Delay"
id.subfunc = 2
id.callname = "sub__delay"
id.args = 1
id.arg = MKL$(DOUBLETYPE - ISPOINTER)
id.hr_syntax = "_DELAY seconds!"
regid
```

**C Implementation (datetime.cpp):**
```c
void sub__delay(double seconds) {
    double base = GetTicks();
    if (new_error) return;
    if (seconds < 0) { error(5); return; }
    // Wait implementation...
}
```

### Color Function

**Registration:**
```basic
clearid
id.n = "_RGB"
id.subfunc = 1
id.callname = "func__rgb"
id.args = 4
id.arg = MKL$(LONGTYPE - ISPOINTER) + _
         MKL$(LONGTYPE - ISPOINTER) + _
         MKL$(LONGTYPE - ISPOINTER) + _
         MKL$(LONGTYPE - ISPOINTER)
id.specialformat = "?,?,?[,?]"
id.ret = ULONGTYPE - ISPOINTER
id.hr_syntax = "_RGB(red&, green&, blue&[, imageHandle&])"
regid
```

**C Implementation (libqb.cpp):**
```c
uint32 func__rgb(int32 r, int32 g, int32 b, int32 i, int32 passed) {
    if (is_error_pending()) return 0;
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    return (r << 16) | (g << 8) | b | 0xFF000000;
}
```

### String Function

**Registration:**
```basic
clearid
id.n = "_ReadFile"
id.musthave = "$"                    ' Must have $ suffix
id.subfunc = 1
id.callname = "func__readfile"
id.args = 1
id.arg = MKL$(STRINGTYPE - ISPOINTER)
id.ret = STRINGTYPE - ISPOINTER
id.hr_syntax = "_READFILE$(fileSpec$)"
regid
```

### Memory Function

**Registration:**
```basic
clearid
id.n = "_MemGet"
id.subfunc = 1
id.callname = "func__memget"
id.args = 3
id.arg = MKL$(UDTTYPE + (1)) + _     ' _MEM structure
         MKL$(OFFSETTYPE - ISPOINTER) + _
         MKL$(-1)                     ' Any numeric type
id.ret = -1                           ' Variable return type
id.hr_syntax = "_MEMGET(block, offset, type)"
regid
```

## C Implementation Locations

| Directory | Contents |
|-----------|----------|
| `internal/c/libqb/src/string_functions.cpp` | LSET, RSET, SPACE$, etc. |
| `internal/c/libqb/src/datetime.cpp` | _DELAY, TIMER, _FPS |
| `internal/c/libqb/src/graphics.cpp` | Color functions |
| `internal/c/libqb/src/filesystem.cpp` | File operations |
| `internal/c/libqb/src/mem.cpp` | _MEM operations |
| `internal/c/libqb/src/environ.cpp` | ENVIRON$ |
| `internal/c/libqb/src/error_handle.cpp` | Error handling |
| `internal/c/libqb.cpp` | Many consolidated functions |

## Adding a New Function

### Step 1: Create C Implementation

```c
// In appropriate .cpp file
int32_t func__myfunc(qbs *arg1, int32_t arg2) {
    if (is_error_pending()) return 0;
    // Implementation
    return result;
}
```

### Step 2: Register in subs_functions.bas

```basic
clearid
id.n = "_MyFunc"
id.subfunc = 1
id.callname = "func__myfunc"
id.args = 2
id.arg = MKL$(STRINGTYPE - ISPOINTER) + MKL$(LONGTYPE - ISPOINTER)
id.ret = LONGTYPE - ISPOINTER
id.specialformat = "?[,?]"
id.hr_syntax = "_MYFUNC(text$[, number&])"
regid
```

### Step 3: Add to Syntax Highlighter

```basic
' In syntax_highlighter_list.bas
listOfKeywords$ = listOfKeywords$ + "_MYFUNC@"
```

### Step 4: Recompile QB64pe

## Code Generation Flow

1. Parser reads QB64 code
2. `reginternal` registers all built-in functions
3. Each registration stores QB64 name → C function mapping
4. During code generation:
   - Compiler finds matching registered ID
   - Generates C code calling `id.callname`
   - Applies proper type casting

**Example:**
```basic
' QB64 code
x& = _FileExists("test.txt")
```

**Generated C:**
```c
int32_t x = func__fileexists(qbs_temp);
```

## OpenGL Extension System

OpenGL functions dynamically registered by parsing GL headers:

```basic
SUB gl_scan_header
    ' Reads gl.h and generates QB64 function registrations
    OPEN "internal\c\parts\core\gl_header_for_parsing\gl.h" FOR BINARY AS #h
    ' Parses #define and function declarations
END SUB
```

Type mapping via `gl2qb_type_convert$()`:
- `GLenum` → `_UNSIGNED LONG`
- `GLint` → `LONG`
- `GLfloat` → `SINGLE`
- etc.

## Key Functions

| Function | Purpose |
|----------|---------|
| `clearid` | Reset id to blank template |
| `regid` | Store id in global array |
| `reginternal` | Register all built-in functions |

## Summary

| Concept | Location |
|---------|----------|
| Registration | `subs_functions.bas` |
| ID Structure | `qb64pe.bas` lines 545-591 |
| Global Array | `ids(1 to ids_max)` |
| C Implementations | `internal/c/libqb/src/` |
