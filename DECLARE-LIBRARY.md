# DECLARE-LIBRARY.md

Documentation of the QB64pe external library binding system.

## Overview

QB64pe provides comprehensive external library support through DECLARE LIBRARY statements, enabling calls to C/C++ functions in DLLs, shared objects, and static libraries.

## Declaration Forms

### DECLARE LIBRARY (Static)

```basic
DECLARE LIBRARY "libraryname"
    FUNCTION func& (BYVAL param AS LONG)
END DECLARE
```

Links at compile-time. Library included in executable.

### DECLARE DYNAMIC LIBRARY

```basic
DECLARE DYNAMIC LIBRARY "path/to/lib"
    FUNCTION func& (BYVAL param AS LONG)
END DECLARE
```

Loads at runtime via LoadLibrary/dlopen.

### DECLARE CUSTOMTYPE LIBRARY

```basic
DECLARE CUSTOMTYPE LIBRARY "libraryname"
    FUNCTION func& (BYVAL ptr AS _OFFSET)
END DECLARE
```

For function pointers and custom calling conventions.

### DECLARE STATIC LIBRARY

```basic
DECLARE STATIC LIBRARY "libraryname"
    FUNCTION func& (BYVAL param AS LONG)
END DECLARE
```

Explicitly static linking.

## Library Loading

### Windows

```c
HINSTANCE DLL_libname = NULL;
if (!DLL_libname) {
    DLL_libname = LoadLibrary("path/to/library.dll");
    if (!DLL_libname) error(259);
}
```

### Linux/macOS

```c
void *DLL_libname = NULL;
if (!DLL_libname) {
    DLL_libname = dlopen("path/to/library.so", RTLD_LAZY);
    if (!DLL_libname) error(259);
}
```

## Function Resolution

### Windows

```c
funcname = (DLLCALL_funcname)GetProcAddress(DLL_name, "funcname");
if (!funcname) error(260);
```

### Linux/macOS

```c
funcname = (DLLCALL_funcname)dlsym(DLL_name, "funcname");
if (dlerror()) error(260);
```

## ALIAS Keyword

```basic
DECLARE LIBRARY "kernel32"
    FUNCTION GetTicks& ALIAS "GetTickCount64"
    FUNCTION MaxInt& ALIAS "Max<int>" (BYVAL a AS LONG, BYVAL b AS LONG)
END DECLARE
```

Maps QB64 function name to different library function name.

## Type Mapping

| QB64 Type | C Type | Notes |
|-----------|--------|-------|
| INTEGER | int16 | 16-bit signed |
| LONG | int32 | 32-bit signed |
| _INTEGER64 | int64 | 64-bit signed |
| SINGLE | float | 32-bit float |
| DOUBLE | double | 64-bit float |
| STRING | char* | Null-terminated |
| _OFFSET | ptrszint | Pointer-sized |
| ANY | void* | Generic pointer |

## Parameter Passing

### BYVAL

```basic
FUNCTION func& (BYVAL x AS LONG)
```

Passes value directly.

### Default (BYREF)

```basic
FUNCTION func& (x AS LONG)
```

Passes pointer to variable.

## Platform Differences

### Windows DLLs

- Extension: `.dll`
- Calling: `CALLBACK` (WINAPI)
- Path: Backslashes or forward slashes

### Linux Shared Objects

- Extension: `.so`, `.so.VERSION`
- Calling: Standard C ABI
- Path: Forward slashes only

### macOS Dynamic Libraries

- Extension: `.dylib`
- Calling: Standard C ABI
- Symbol prefix: `_` convention

## Library Search Paths

### Windows

1. Direct path
2. Application directory
3. System directories

### Linux/macOS

1. Direct path with version
2. Direct path without version
3. `/usr/lib64/`
4. `/usr/lib/`
5. Current directory (`./`)

## Generated Code

### Typedef (Windows)

```c
typedef returntype (CALLBACK* DLLCALL_funcname)(param1, param2);
```

### Typedef (Linux/macOS)

```c
typedef returntype (*DLLCALL_funcname)(param1, param2);
```

### Function Pointer

```c
DLLCALL_funcname funcname = NULL;
```

### Resolution

```c
if (!funcname) {
    funcname = (DLLCALL_funcname)GetProcAddress(DLL_lib, "funcname");
    if (!funcname) error(260);
}
result = funcname(arg1, arg2);
```

## Error Codes

| Code | Error |
|------|-------|
| 259 | Cannot find dynamic library |
| 260 | Function not in dynamic library |
| 261 | Function not in dynamic library |

## Header File Integration

```basic
DECLARE LIBRARY "./mylib"
    FUNCTION my_func& (BYVAL x AS LONG)
END DECLARE
```

Searches for `mylib.h` in:
1. Library path
2. Local directory
3. `/usr/include/` (Unix)

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | DECLARE parsing (4000-5400) |

## Example Usage

### Windows API

```basic
DECLARE DYNAMIC LIBRARY "kernel32"
    FUNCTION GetTickCount~& ()
    FUNCTION Sleep (BYVAL ms AS LONG)
END DECLARE

PRINT "Ticks:"; GetTickCount
Sleep 1000
PRINT "After 1 second:"; GetTickCount
```

### MessageBox

```basic
DECLARE DYNAMIC LIBRARY "user32"
    FUNCTION MessageBoxA& (BYVAL hwnd AS _OFFSET, msg AS STRING, title AS STRING, BYVAL flags AS LONG)
END DECLARE

result = MessageBoxA(0, "Hello!" + CHR$(0), "Title" + CHR$(0), 0)
```

### C Library Functions

```basic
DECLARE LIBRARY
    FUNCTION strlen~& (BYVAL s AS _OFFSET)
    FUNCTION strcmp& (BYVAL s1 AS _OFFSET, BYVAL s2 AS _OFFSET)
END DECLARE

s$ = "Hello" + CHR$(0)
PRINT "Length:"; strlen(_OFFSET(s$))
```

### Custom Header

```basic
' mymath.h contains: double square(double x);

DECLARE LIBRARY "mymath"
    FUNCTION square# (BYVAL x AS DOUBLE)
END DECLARE

PRINT square(5)  ' Prints 25
```

### Dynamic Loading

```basic
$IF WIN THEN
    libname$ = "mylib.dll"
$ELSE
    libname$ = "./mylib.so"
$END IF

DECLARE DYNAMIC LIBRARY libname$
    FUNCTION process& (BYVAL data AS _OFFSET, BYVAL size AS LONG)
END DECLARE

result = process(_OFFSET(buffer$), LEN(buffer$))
```

### ALIAS for Mangled Names

```basic
DECLARE LIBRARY "mathlib"
    ' C++ template function: template<typename T> T max(T a, T b)
    FUNCTION MaxInt& ALIAS "max<int>" (BYVAL a AS LONG, BYVAL b AS LONG)
    FUNCTION MaxDouble# ALIAS "max<double>" (BYVAL a AS DOUBLE, BYVAL b AS DOUBLE)
END DECLARE

PRINT MaxInt(5, 10)       ' 10
PRINT MaxDouble(3.14, 2.71)  ' 3.14
```

### Passing Strings

```basic
DECLARE LIBRARY
    SUB puts (s AS STRING)
END DECLARE

' Strings automatically null-terminated
puts "Hello, World!"
```

### Return Pointer

```basic
DECLARE LIBRARY
    FUNCTION malloc~%& (BYVAL size AS _OFFSET)
    SUB free (BYVAL ptr AS _OFFSET)
END DECLARE

ptr~%& = malloc(1024)
IF ptr~%& THEN
    ' Use memory...
    free ptr~%&
END IF
```
