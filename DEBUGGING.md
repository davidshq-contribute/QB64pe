# DEBUGGING.md

Documentation of the QB64pe debugging system.

## Overview

QB64pe provides integrated debugging through the `$DEBUG` metacommand, enabling breakpoints, variable watching, stepping, and call stack inspection via TCP/IP communication between the IDE and running program.

## $DEBUG Metacommand

```basic
$DEBUG
```

### Effects

- Sets `vWatchOn` compiler flag
- Defines `_DEBUG_` preprocessor variable to "1"
- Auto-includes vwatch system files
- IDE-only feature (warning in console mode)
- Incompatible with `CHAIN` and `RUN` commands

### Auto-Includes

```basic
' At top of program:
'$INCLUDE:'internal/support/vwatch/vwatch.bi'

' At bottom of program:
'$INCLUDE:'internal/support/vwatch/vwatch.bm'
```

## Breakpoints

### Setting Breakpoints

In the IDE:
- Click line number margin
- Press F9 on current line
- Right-click context menu

### Implementation

```basic
REDIM vwatch_breakpoints(linecount) AS _BYTE
```

### Runtime Check

```basic
IF vwatch_breakpoints(vwatch_linenumber) = 0 AND NOT vw_pauseMode THEN
    EXIT SUB
END IF
```

### Skip Lines

Hold SHIFT when setting breakpoint to mark as "skip line":
- Line is skipped during execution
- Useful for bypassing problematic code

## Watch Variables

### Variable Tracking

- Global variables: `vwatch_global_vars[]` array
- Local variables: `vwatch_local_vars[]` per SUB/FUNCTION

### Variable Metadata

| Field | Purpose |
|-------|---------|
| `id` | Symbol table ID |
| `name` | Display name with type suffix |
| `cname` | C-compatible name |
| `varType` | Full type string |
| `subfunc` | Scope (empty for globals) |
| `isarray` | -1 if array |
| `displayFormat` | Decimal/hex/binary |

### Watchpoint Expressions

```basic
' Supported operators:
=    ' Equality
<>   ' Inequality
<    ' Less than
>    ' Greater than
<=   ' Less than or equal
>=   ' Greater than or equal
```

When condition becomes true, execution pauses.

## Stepping

| Action | Behavior |
|--------|----------|
| Step (F8) | Execute one line |
| Step Over (Shift+F8) | Execute line, skip SUB calls |
| Step Out | Run until current SUB returns |
| Run to Line | Execute until specific line |
| Continue (F5) | Resume execution |

## IDE-Program Protocol

### Connection

1. IDE opens TCP listener on random port
2. IDE sets `QB64DEBUGPORT` environment variable
3. Program connects via `_OPENCLIENT("QB64IDE:" + port + ":localhost")`
4. Handshake exchange

### Message Format

```
[4 bytes: length][message data]
Format: "command:value"
```

### IDE to Program Commands

| Command | Purpose |
|---------|---------|
| `run` | Continue execution |
| `break` | Pause execution |
| `step` | Single step |
| `step over` | Step over SUB |
| `step out` | Run until return |
| `run to line` | Execute to line |
| `set breakpoint` | Add breakpoint |
| `clear breakpoint` | Remove breakpoint |
| `get global var` | Fetch variable |
| `set global address` | Modify variable |
| `call stack` | Request stack |
| `set next line` | Jump to line |

### Program to IDE Responses

| Response | Purpose |
|----------|---------|
| `line number` | Reached line |
| `breakpoint` | Breakpoint hit |
| `watchpoint` | Watch triggered |
| `address read` | Variable value |
| `call stack` | Call chain |
| `error` | Error occurred |

### Special Line Numbers

| Value | Meaning |
|-------|---------|
| 0 | Program ended |
| -1 | Error occurred |
| -2 | SUB/FUNCTION entered |
| -3 | STOP statement |
| -4 | INPUT started |
| -5 | INPUT completed |

## Generated Code

### At Every Line

```c
*__LONG_VWATCH_LINENUMBER = <linenumber>;
SUB_VWATCH((ptrszint*)vwatch_global_vars, (ptrszint*)vwatch_local_vars);
if (*__LONG_VWATCH_GOTO > 0) goto VWATCH_SETNEXTLINE;
if (*__LONG_VWATCH_GOTO < 0) goto VWATCH_SKIPLINE;
```

### Dispatch Tables

```c
// vw_main_dispatch.txt - GOTO commands
switch (*__LONG_VWATCH_GOTO) {
    case 100: goto VWATCH_LABEL_100; break;
    case 150: goto VWATCH_LABEL_150; break;
}

// vw_main_skip.txt - Skip lines
switch (*__LONG_VWATCH_GOTO) {
    case -100: goto VWATCH_SKIPLABEL_100; break;
}
```

## Call Stack

```basic
DIM SHARED vwatch_callstack AS STRING
REDIM SHARED vwatch_stack(1000) AS STRING
```

Tracks SUB/FUNCTION entry/exit for stack display.

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | $DEBUG handling (1817-1821) |
| `internal/support/vwatch/vwatch.bi` | Variable declarations |
| `internal/support/vwatch/vwatch.bm` | Debug runtime (885 lines) |
| `source/ide/ide_methods.bas` | IDE integration |

## Example Usage

### Basic Debugging

```basic
$DEBUG

DIM x AS INTEGER
x = 10

' Set breakpoint here in IDE
x = x + 5

PRINT x
```

### Conditional Watch

In IDE Watch window:
1. Add variable `x`
2. Set watchpoint: `x > 15`
3. Execution pauses when condition true

### Using $IF DEBUG

```basic
$IF DEBUG THEN
    $DEBUG
    PRINT "Debug mode enabled"
$END IF

' Main program code...
```

## Debug vs Release

| Feature | Debug | Release |
|---------|-------|---------|
| vwatch calls | Generated | None |
| Line tracking | Yes | No |
| Breakpoints | Supported | N/A |
| Performance | Slower | Full speed |
| Executable size | Larger | Smaller |

## Architecture

```
┌─────────────────────────────────────────────┐
│ IDE (ide2/ide_methods.bas)                  │
│ ├─ Breakpoint management                    │
│ ├─ Watch panel                              │
│ └─ Step controls                            │
├────────────TCP/IP Protocol──────────────────┤
│ VWATCH Runtime (vwatch.bm)                  │
│ ├─ Line-by-line checking                    │
│ ├─ Variable resolution                      │
│ ├─ Watchpoint evaluation                    │
│ └─ Command processing                       │
├─────────────────────────────────────────────┤
│ Generated Code (qbx.cpp)                    │
│ ├─ SUB_VWATCH calls at each line            │
│ ├─ vwatch_global_vars[]                     │
│ ├─ vwatch_local_vars[]                      │
│ └─ Dispatch tables                          │
└─────────────────────────────────────────────┘
```
