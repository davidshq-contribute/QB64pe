# LIBRARY-SYSTEM.md

Documentation of the QB64pe library and external function binding system.

## Overview

QB64pe supports two library mechanisms:
1. **$USELIBRARY** - Include QB64 library packages with auto-include
2. **DECLARE LIBRARY** - Bind to external C/C++ functions

## $USELIBRARY Metacommand

### Syntax

```basic
$USELIBRARY:'author/libraryname'
```

### Descriptor File Format

Library descriptors are INI files at `libraries/descriptors/{library_name}.ini`:

```ini
[LIBRARY INCLUDES]
IncAtTop=path/to/header.bi
IncAfterMain=path/to/init.bas
IncAtBottom=path/to/cleanup.bm
```

| Field | Purpose |
|-------|---------|
| `IncAtTop` | Included before main code (constants, types) |
| `IncAfterMain` | Included after SUB/FUNCTION declarations |
| `IncAtBottom` | Included at end of program (cleanup) |

### Source File Location

Library source files expected at: `libraries/includes/{library_name}/`

### Implementation

**Location:** `source/qb64pe.bas` lines 1728-1788

```basic
IF temp$ = "$USELIBRARY" THEN
    ' Parse library name from quotes
    ' Verify descriptor INI exists
    ' Read three file entries from [LIBRARY INCLUDES]
    ' Track referrer (who requested this library)
    ' Store in useLibList$ array
    ' Trigger recompilation
    GOTO do_recompile
END IF
```

### Tracking Array

```basic
REDIM SHARED useLibList$(4, 10)
CONST ullName = 0      ' Library name
CONST ullNeedy = 1     ' Referrer (file + line)
CONST ullTop = 2       ' AtTop file path
CONST ullMain = 3      ' AfterMain file path
CONST ullBottom = 4    ' AtBottom file path
```

### Dependency Resolution

**Phase 1 - Registration:**
1. Determine referrer (source file + line number)
2. Check if library already registered for this referrer
3. Find empty slot in `useLibList$` or expand
4. Store name, referrer, and three file paths
5. Trigger recompilation with `GOTO do_recompile`

**Phase 2 - Injection:**
During auto-include processing, files are injected at three points:

| Point | Processing Order | Purpose |
|-------|-----------------|---------|
| AtTop | **Reversed** (dependencies first) | Core infrastructure, types |
| AfterMain | Forward | Initialization code |
| AtBottom | Forward | Cleanup routines |

**Marker System:**
A separator `"-----"` marks the transition between AfterMain and AtBottom when no SUB/FUNCTION definitions exist.

## DECLARE LIBRARY

### Syntax Forms

```basic
' Static library (linked at compile time)
DECLARE LIBRARY "libraryname"
    FUNCTION CFunction& (BYVAL param AS LONG)
    SUB CSub (arg AS STRING)
END DECLARE

' Dynamic library (loaded at runtime)
DECLARE DYNAMIC LIBRARY "path/to/library"
    FUNCTION DllFunction& (BYVAL x AS LONG)
END DECLARE

' Custom type library (function pointers)
DECLARE CUSTOMTYPE LIBRARY "libraryname"
    FUNCTION CustomFunc& (BYVAL ptr AS _OFFSET)
END DECLARE
```

### Library Search Order (Linux)

1. Direct path with version: `libpath/libname.so.VERSION`
2. Direct path without version: `libpath/libname.so`
3. Archive extension: `libpath/libname.a`
4. System `/usr/lib64/`
5. System `/usr/lib/`
6. Current directory

### State Variables

| Variable | Purpose |
|----------|---------|
| `declaringlibrary` | 0=off, 1=declaring, 2=in block |
| `dynamiclibrary` | 0=static, 1=dynamic |
| `customtypelibrary` | 0=off, 1=custom type mode |
| `indirectlibrary` | 0=direct, 1=indirect/header |
| `staticlinkedlibrary` | 0=dynamic, 1=static linked |

## DECLARE LIBRARY vs DECLARE DYNAMIC LIBRARY

| Aspect | DECLARE LIBRARY | DECLARE DYNAMIC LIBRARY |
|--------|-----------------|-------------------------|
| Linking | Compile time | Runtime |
| Library Type | .lib, .a, .o | .dll, .so, .dylib |
| `indirectlibrary` | 0 | 1 |
| Initialization | None | LoadLibrary/dlopen |
| Symbol Resolution | Linker | GetProcAddress/dlsym |

## Function Binding Process

### Step 1: Declaration Parsing

Parameters processed with pattern: `[BYVAL]name[%][(1)][AS][type]`

```basic
byvalue = 0
IF e$ = "BYVAL" THEN byvalue = 1: ...
```

### Step 2: Type Mapping

The `typ2ctyp$()` function maps QB64 types to C:

| QB64 Type | C Type |
|-----------|--------|
| STRING | qbs → char* |
| INTEGER | int16 |
| LONG | int32 |
| _INTEGER64 | int64 |
| SINGLE | float |
| DOUBLE | double |
| _OFFSET | ptrszint |

BYVAL parameters pass value directly; non-BYVAL passes pointer.

### Step 3: Typedef Generation

**For dynamic libraries (Windows):**
```c
typedef returntype (CALLBACK* DLLCALL_funcname)(param1, param2, ...);
```

**For dynamic libraries (Linux):**
```c
typedef returntype (*DLLCALL_funcname)(param1, param2, ...);
```

**For custom types:**
```c
typedef returntype CUSTOMCALL_funcname(param1, param2, ...);
```

### Step 4: Function Pointer Initialization

**Dynamic Library (Windows):**
```c
HINSTANCE DLL_name = NULL;
if (!DLL_name) {
    DLL_name = LoadLibrary("path");
    if (!DLL_name) error(259);
}
if (!funcname) {
    funcname = (DLLCALL_funcname)GetProcAddress(DLL_name, "aliasname");
    if (!funcname) error(260);
}
```

**Dynamic Library (Linux):**
```c
void *DLL_name = NULL;
if (!DLL_name) {
    DLL_name = dlopen("path", RTLD_LAZY);
    if (!DLL_name) error(259);
}
if (!funcname) {
    funcname = (DLLCALL_funcname)dlsym(DLL_name, "aliasname");
    if (dlerror()) error(260);
}
```

**Custom Type:**
```c
CUSTOMCALL_funcname *funcname = NULL;
funcname = (CUSTOMCALL_funcname*)&aliasname;
```

### Step 5: Function Call

When declared function is called:
1. Generate call with resolved function name
2. BYVAL parameters: pass value
3. Regular parameters: pass address/pointer
4. Invoke via function pointer

## BYVAL and ALIAS Keywords

These keywords only work within DECLARE LIBRARY blocks:

### BYVAL

```basic
DECLARE LIBRARY
    SUB memcpy (BYVAL dest AS _OFFSET, BYVAL src AS _OFFSET, BYVAL n AS _OFFSET)
END DECLARE
```

Passes the value directly instead of a pointer.

### ALIAS

```basic
DECLARE LIBRARY
    FUNCTION GetTickCount& ALIAS "GetTickCount64"
END DECLARE
```

Maps QB64 function name to different C function name.

## Static Function Resolution

For static libraries, functions are resolved at link time:

```basic
ResolveStaticFunction_Name()   ' Function name in library
ResolveStaticFunction_File()   ' Library file path
ResolveStaticFunction_Method() ' 1=CustomType, 2=Static
```

## Error Codes

| Code | Meaning |
|------|---------|
| 259 | Dynamic library failed to load |
| 260 | Function symbol not found in library |

## Compilation Flow

1. **Parse Phase**: Recognize DECLARE LIBRARY blocks, $USELIBRARY
2. **Preprocessing**: Load descriptor INIs, register dependencies
3. **Recompilation**: Repeat if new libraries found
4. **Library Resolution**: Search for files in platform-specific paths
5. **Declaration Phase**: Parse SUB/FUNCTION in DECLARE blocks
6. **Typedef Generation**: Create C function pointer types
7. **Auto-Include**: Insert library files at injection points
8. **Runtime Binding**: Generate dynamic loading code
9. **Code Generation**: Emit C code with type conversions

## Example: Complete Library Usage

### QB64 Code

```basic
$USELIBRARY:'mylib/utilities'

DECLARE LIBRARY "mylib"
    FUNCTION fast_add& (BYVAL a AS LONG, BYVAL b AS LONG)
END DECLARE

DECLARE DYNAMIC LIBRARY "plugins/extra"
    SUB plugin_init ()
END DECLARE

PRINT fast_add(10, 20)
plugin_init
```

### Generated C++ (simplified)

```cpp
// Static library binding
extern "C" int32_t fast_add(int32_t a, int32_t b);

// Dynamic library binding
void *DLL_plugins_extra = NULL;
typedef void (*DLLCALL_plugin_init)();
DLLCALL_plugin_init plugin_init = NULL;

void init_dynamic_libs() {
    if (!DLL_plugins_extra) {
        DLL_plugins_extra = dlopen("plugins/extra.so", RTLD_LAZY);
        if (!DLL_plugins_extra) error(259);
    }
    if (!plugin_init) {
        plugin_init = (DLLCALL_plugin_init)dlsym(DLL_plugins_extra, "plugin_init");
        if (dlerror()) error(260);
    }
}

// Usage
qbs_print(qbs_str(fast_add(10, 20)));
init_dynamic_libs();
plugin_init();
```

## Library Descriptor Example

**File:** `libraries/descriptors/mylib/utilities.ini`

```ini
[LIBRARY INCLUDES]
IncAtTop=mylib/utilities/utilities.bi
IncAfterMain=mylib/utilities/utilities_init.bas
IncAtBottom=mylib/utilities/utilities_cleanup.bm
```

**utilities.bi:**
```basic
' Constants and type definitions
CONST UTIL_VERSION = "1.0"
TYPE UtilData
    value AS LONG
    name AS STRING * 32
END TYPE
```

**utilities_init.bas:**
```basic
' Initialization code that runs after main declarations
DIM SHARED utilInitialized AS INTEGER
utilInitialized = -1
```

**utilities_cleanup.bm:**
```basic
' Cleanup SUBs included at bottom
SUB UtilCleanup
    ' Cleanup code
END SUB
```
