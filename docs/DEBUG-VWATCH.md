# DEBUG-VWATCH.md

Documentation of the QB64pe debug and variable watch (vwatch) system.

## Overview

The vwatch system provides IDE-integrated debugging with breakpoints, variable watching, stepping, and call stack inspection via TCP/IP communication between the running program and IDE.

## Core Files

| File | Size | Purpose |
|------|------|---------|
| `internal/support/vwatch/vwatch.bi` | 571 bytes | Variable declarations |
| `internal/support/vwatch/vwatch.bm` | 40,712 bytes | Debug runtime SUB |
| `internal/support/vwatch/vwatch_stub.bm` | 63 bytes | No-op stub when debug disabled |

## Enabling Debug Mode

### $DEBUG Metacommand

```basic
$DEBUG
```

**Processing:** (source/qb64pe.bas line 1817-1821)

```basic
IF temp$ = "$DEBUG" THEN
    SetRCStateVar vWatchOn, 1
    SetPreLET "_DEBUG_", "1"
    GOTO finishedlinepp
END IF
```

This:
1. Sets `vWatchOn` state variable to 1
2. Defines `_DEBUG_` preprocessor variable as "1"
3. Triggers auto-include of vwatch files

### Auto-Include Injection

When `vWatchOn = 1`:
1. `internal/support/vwatch/vwatch.bi` included at top
2. `internal/support/vwatch/vwatch.bm` included at bottom (or stub if compiling vwatch itself)

## vwatch.bi Variables

```basic
DIM SHARED vwatch_linenumber AS LONG        ' Current line being executed
DIM SHARED vwatch_sublevel AS LONG          ' Call stack depth
DIM SHARED vwatch_goto AS LONG              ' Jump target from IDE
DIM SHARED vwatch_subname AS STRING         ' Current subroutine (display)
DIM SHARED vwatch_internalsubname AS STRING ' Internal compiled name
DIM SHARED vwatch_callstack AS STRING       ' Call stack history
DIM SHARED vwatch_breakpoints() AS _BYTE    ' Breakpoint flags by line
DIM SHARED vwatch_skiplines() AS _BYTE      ' Skip line flags by line
DIM SHARED vwatch_stack(1000) AS STRING     ' Call stack storage
```

## Code Generation

### At Every Executable Line

When `vWatchOn` enabled, compiler inserts:

```c
*__LONG_VWATCH_LINENUMBER = <linenumber>;
SUB_VWATCH((ptrszint*)vwatch_global_vars, (ptrszint*)vwatch_local_vars);
if (*__LONG_VWATCH_GOTO > 0) goto VWATCH_SETNEXTLINE;
if (*__LONG_VWATCH_GOTO < 0) goto VWATCH_SKIPLINE;
```

### Dispatch Jump Tables

Two files generated during compilation:

**vw_main_dispatch.txt** (for GOTO commands):
```c
case 100:
    goto VWATCH_LABEL_100;
    break;
case 150:
    goto VWATCH_LABEL_150;
    break;
```

**vw_main_skip.txt** (for skip lines):
```c
case -100:
    goto VWATCH_SKIPLABEL_100;
    break;
```

### Integration in Main Function

```c
VWATCH_SETNEXTLINE:;
switch (*__LONG_VWATCH_GOTO) {
#include "vw_main_dispatch.txt"
    default:
        *__LONG_VWATCH_GOTO = *__LONG_VWATCH_LINENUMBER;
        goto VWATCH_SETNEXTLINE;
}

VWATCH_SKIPLINE:;
switch (*__LONG_VWATCH_GOTO) {
#include "vw_main_skip.txt"
}
```

## Variable Tracking

### Global Variables Array

Generated in `maindata.txt`:
```c
void *vwatch_global_vars[totalGlobalVars];
vwatch_global_vars[0] = &globalVar1;
vwatch_global_vars[1] = &globalVar2;
// ...
```

### Local Variables Array

Generated per-subroutine in `mainX.txt`:
```c
void *vwatch_local_vars[totalLocalVars];
vwatch_local_vars[0] = &localVar1;
vwatch_local_vars[1] = &localVar2;
// ...
```

### Variable Metadata (usedVarList TYPE)

| Field | Purpose |
|-------|---------|
| `id` | Symbol table ID |
| `name` | Display name with suffix (e.g., "x%") |
| `cname` | C-compatible name |
| `varType` | Full type string |
| `subfunc` | Scope (empty=global) |
| `scope` | Numeric scope ID |
| `linenumber` | Declaration line |
| `localIndex` | Index in variable array |
| `isarray` | -1 if array |
| `arrayElementSize` | Size per element |
| `elements` | UDT element list |
| `elementTypes` | UDT element types |
| `elementOffset` | UDT element offsets |
| `watch` | In watch list flag |
| `displayFormat` | Binary/hex/decimal |

### Excluded Variables

Internal vwatch variables are hidden from watch list:

```basic
vWatchVariableExclusions$ = "@__LONG_VWATCH_LINENUMBER@__LONG_VWATCH_SUBLEVEL@" +
    "@__LONG_VWATCH_GOTO@__STRING_VWATCH_SUBNAME@" +
    "@__STRING_VWATCH_CALLSTACK@__ARRAY_BYTE_VWATCH_BREAKPOINTS@" +
    "@__ARRAY_BYTE_VWATCH_SKIPLINES@__STRING_VWATCH_INTERNALSUBNAME@" +
    "@__ARRAY_STRING_VWATCH_STACK@"
```

## TCP/IP Communication

### Connection Establishment

**IDE Side:**
1. Open TCP listener: `host& = _OPENHOST("TCP/IP:" + hostport$)`
2. Set environment: `ENVIRON "QB64DEBUGPORT=" + hostport$`
3. Wait for connection: `debugClient& = _OPENCONNECTION(host&)`

**Program Side:**
1. Read port: `vw_ideport$ = ENVIRON$("QB64DEBUGPORT")`
2. Connect: `vw_ideHost = _OPENCLIENT("QB64IDE:" + vw_ideport$ + ":localhost")`
3. Retry up to 10 seconds (ESC to abort)

### Handshake

1. Program sends: `"me:" + COMMAND$(0)` (executable path)
2. Program sends: `"hwnd:" + window_handle`
3. IDE verifies executable matches
4. IDE sends: `"vwatch:ok"`

### Message Format

```
[4 bytes: little-endian length][message data]
```

Message format: `"command:value"` or just `"command"`

## Debug Commands

### IDE → Program

| Command | Value | Purpose |
|---------|-------|---------|
| `line count` | 4-byte count | Total source lines |
| `breakpoint count` | count | Number of breakpoints |
| `breakpoint list` | packed MKL$ | Line numbers |
| `skip count` | count | Skip line count |
| `skip list` | packed MKL$ | Skip line numbers |
| `run` | - | Continue execution |
| `break` | - | Pause execution |
| `step` | - | Single step |
| `step over` | - | Step over SUB calls |
| `step out` | - | Run until SUB returns |
| `run to line` | 4-byte line | Execute until line |
| `set breakpoint` | 4-byte line | Add breakpoint |
| `clear breakpoint` | 4-byte line | Remove breakpoint |
| `set skip line` | 4-byte line | Mark line to skip |
| `clear skip line` | 4-byte line | Unmark skip |
| `get global var` | metadata | Fetch global value |
| `get local var` | metadata | Fetch local value |
| `set global address` | metadata+value | Modify global |
| `set local address` | metadata+value | Modify local |
| `set global watchpoint` | metadata+expr | Add watch |
| `set local watchpoint` | metadata+expr | Add watch |
| `clear global watchpoint` | metadata | Remove watch |
| `clear local watchpoint` | metadata | Remove watch |
| `call stack` | - | Request call stack |
| `current sub` | - | Request current scope |
| `set next line` | 4-byte line | Jump to line |

### Program → IDE

| Response | Value | Meaning |
|----------|-------|---------|
| `me` | path | Program identification |
| `hwnd` | 8-byte handle | Window handle |
| `line number` | 4-byte line | Reached line |
| `breakpoint` | 4-byte line | Breakpoint hit |
| `watchpoint` | metadata+data | Watchpoint triggered |
| `address read` | index+data | Variable value |
| `current sub` | name | Current scope |
| `call stack size` | count | Stack depth |
| `call stack` | names | Call chain |
| `error` | 4-byte line | Error at line |

## Breakpoint System

### Storage

```basic
' IDE side
REDIM IdeBreakpoints(linecount) AS _BYTE

' Program side
REDIM vwatch_breakpoints(linecount) AS _BYTE
```

### Setting Breakpoints

**IDE sends:**
```basic
cmd$ = "breakpoint count:" + MKL$(count)
cmd$ = "breakpoint list:" + breakpointList$  ' MKL$ for each line
```

**Program receives:**
```basic
CASE "breakpoint count"
    REDIM vwatch_breakpoints(CVL(vw_value$)) AS _BYTE
CASE "breakpoint list"
    FOR i = 1 TO count
        vwatch_breakpoints(CVL(MID$(vw_value$, i*4-3, 4))) = -1
    NEXT
```

### Runtime Check

At each line in vwatch.bm:
```basic
IF vwatch_breakpoints(vwatch_linenumber) = 0 AND NOT vw_pauseMode THEN EXIT SUB
```

If breakpoint set or pause mode: pause and notify IDE.

### Skip Lines

Similar to breakpoints but stored separately:
- `IdeSkipLines()` on IDE side
- `vwatch_skiplines()` on program side
- Sets `vwatch_goto = -linenumber` to skip execution
- Requires SHIFT key to set in IDE

## Watchpoint System

### Expression Format

```
<operator><value>
```

Operators: `=`, `<`, `>`, `<=`, `>=`, `<>`

### Checking Watchpoints

Before each execution in vwatch.bm:
1. Fetch current value
2. Parse expression operator and comparison value
3. Evaluate condition
4. If true: pause and send `"watchpoint:"` notification

### Types

- STRING: String comparison operators
- NUMERIC: Numeric value comparison

## Variable Resolution

### Resolution Process

1. **Scope Resolution**: Match global vs local based on scope
2. **Address Resolution**: Double dereference through symbol table
3. **Array Index Calculation**:
   - Calculate real index from multidimensional indexes
   - Check bounds
   - Calculate offset: `(index - lBound) * blockSize`
4. **Data Retrieval**:
   - Copy array pointer from symbol table
   - Calculate element address
   - Handle STRING double-dereference
   - Handle _BIT types specially

### Variable Data Format (Get Command)

```
[4 bytes] Variable index
[1 byte]  Is array flag
[4 bytes] Declaration line
[4 bytes] Local index
[4 bytes] Array dimension count
[N bytes] Array indexes (4 bytes each)
[4 bytes] Array element size
[4 bytes] Is UDT flag
[4 bytes] Element number
[4 bytes] Element offset
[4 bytes] Variable size
[4 bytes] Storage index
[2 bytes] Scope name length
[N bytes] Scope name
[2 bytes] Type name length
[N bytes] Type name
```

### Response Format (Address Read)

```
[4 bytes] Variable index
[4 bytes] Array index (if array)
[4 bytes] Element number (if UDT)
[4 bytes] Storage index
[N bytes] Variable data (binary)
```

## Special Line Numbers

| Value | Meaning |
|-------|---------|
| 0 | Program ended normally |
| -1 | Error occurred |
| -2 | SUB/FUNCTION entered |
| -3 | STOP statement |
| -4 | INPUT started |
| -5 | INPUT completed |
| >0 | Normal line execution |

### Generated Code for Special Cases

**Sub Entry (line -2):**
```c
*__LONG_VWATCH_LINENUMBER = -2;
SUB_VWATCH((ptrszint*)vwatch_global_vars, (ptrszint*)vwatch_local_vars);
```

**STOP Statement (line -3):**
```c
*__LONG_VWATCH_LINENUMBER = -3;
SUB_VWATCH(...);
```

**INPUT (lines -4, -5):**
```c
*__LONG_VWATCH_LINENUMBER = -4;  // Enter input mode
SUB_VWATCH(...);
// ... INPUT statement ...
*__LONG_VWATCH_LINENUMBER = -5;  // Leave input mode
SUB_VWATCH(...);
```

**Error:**
```c
if (stop_program) {
    *__LONG_VWATCH_LINENUMBER = 0;
    SUB_VWATCH(...);
}
if (new_error) {
    bkp_new_error = new_error;
    new_error = 0;
    *__LONG_VWATCH_LINENUMBER = -1;
    SUB_VWATCH(...);
    new_error = bkp_new_error;
}
```

## Watch Window UI

### idevariablewatchbox$() Function

Provides variable selection dialog:
- Filter text box for searching
- Listbox with all variables in scope
- Columns: Module, Variable, Type
- Dynamic column width calculation

### Watch Panel Display

During debug session:
- Scrollable panel with watched variables
- Real-time value updates
- Mouse support for scrolling/resizing
- Right-click context menu
- Format toggle (decimal/hex/binary)

### Display Information

| Field | Purpose |
|-------|---------|
| Module name | GLOBAL or function name |
| Variable name | With type suffix |
| Type name | BYTE, INTEGER, etc. |
| Current value | Live during debug |
| Display format | Decimal/hex/binary/octal |

## Execution Control

### Main Loop in vwatch.bm

After breakpoint/pause:

1. **Command Loop**: Read and execute IDE commands
   - Process breakpoint toggles
   - Handle variable read/write
   - Handle watchpoint operations
   - Return variable values

2. **Control Commands**:
   - `run`: Clear pause, exit SUB
   - `step`: Single step (pause at next line)
   - `step over`: Skip subroutine calls
   - `step out`: Run until current SUB returns
   - `run to line`: Continue until specific line
   - `free`: Disconnect and exit debug mode

3. **Watchpoint Checking**: Before each execution
   - Check all watchpoints
   - Evaluate expressions
   - Pause if condition true

## Internal States

| Variable | Purpose |
|----------|---------|
| `vw_ideHost` | File handle to IDE connection |
| `vw_pauseMode` | Program paused flag |
| `vw_stepOver` | Step over mode |
| `vw_runToLine` | Run-to-line target |
| `vw_globalWatchpoints$` | Serialized global watches |
| `vw_localWatchpoints$` | Serialized local watches |
| `vw_timeout` | Connection timeout (10 sec) |
| `vw_checkingWatchpoints` | Watchpoint evaluation state |

## Architecture Summary

1. **Compile-time**: $DEBUG enables variable tracking and vwatch call insertion
2. **Code generation**: Dispatch tables and line-based vwatch calls injected
3. **Runtime**: vwatch.bm SUB implements debug protocol via TCP/IP
4. **Protocol**: Binary-encoded commands with metadata-rich variable serialization
5. **Breakpoints**: Efficient line-based arrays, dynamic modification
6. **Watchpoints**: Expression-based condition evaluation with auto-pause
7. **Symbol resolution**: Two-level dereference with array/UDT support
8. **UI Integration**: Watch panel with filtering, format selection, real-time updates
