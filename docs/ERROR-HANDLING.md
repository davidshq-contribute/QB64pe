# ERROR-HANDLING.md

Documentation of the QB64pe error handling system.

## Overview

QB64pe provides comprehensive error trapping through ON ERROR GOTO with support for nested handlers, multiple recovery options, and detailed error information including file and line tracking.

## ON ERROR GOTO Statement

```basic
ON ERROR GOTO label           ' Standard handler
ON ERROR GOTO _NEWHANDLER label ' Push new handler (preserve previous)
ON ERROR GOTO _LASTHANDLER    ' Pop to previous handler
ON ERROR GOTO 0               ' Disable all handlers
```

### Handler Stack

The `_NEWHANDLER` and `_LASTHANDLER` keywords enable nested error handling:

```basic
ON ERROR GOTO MainHandler
' ... code ...
ON ERROR GOTO _NEWHANDLER SpecialHandler  ' Push
' ... special code ...
ON ERROR GOTO _LASTHANDLER                ' Pop back to MainHandler
```

### C Code Generation

```c
// ON ERROR GOTO label
error_goto_line = <label_id>;

// ON ERROR GOTO _NEWHANDLER label
qbs_set(error_handler_history,
    qbs_add(qbs_str((int32)(error_goto_line)),
            qbs_add(qbs_new_txt("|"), error_handler_history)));
error_goto_line = <new_label_id>;

// ON ERROR GOTO _LASTHANDLER
error_goto_line = qbr(qbs_val<uint64_t>(error_handler_history));
qbs_set(error_handler_history,
    func_mid(error_handler_history, func_instr(NULL, error_handler_history, "|", 0) + 1, NULL, 0));

// ON ERROR GOTO 0
error_goto_line = 0;
qbs_set(error_handler_history, qbs_new_txt(""));
```

## RESUME Statement

```basic
RESUME            ' Retry the line that caused the error
RESUME NEXT       ' Skip to next line after error
RESUME label      ' Jump to specific label
```

### Behavior Differences

| Statement | Sets error_retry | Sets qbevent | Jumps to |
|-----------|-----------------|--------------|----------|
| RESUME | Yes | Yes | Error line (retry) |
| RESUME NEXT | No | No | Next line |
| RESUME label | No | No | Specified label |

## Error Information Functions

```basic
code% = ERR                ' Current error code
line# = ERL                ' Error line number (DOUBLE)
line& = _ERRORLINE         ' Error line in main module
line& = _INCLERRORLINE     ' Error line in include file
file$ = _INCLERRORFILE$    ' Include filename
msg$ = _ERRORMESSAGE$      ' Current error description
msg$ = _ERRORMESSAGE$(n)   ' Description for error code n
```

## Error Codes

### Syntax & Control Flow (1-40)

| Code | Error |
|------|-------|
| 1 | NEXT without FOR |
| 2 | Syntax error |
| 3 | RETURN without GOSUB |
| 4 | Out of DATA |
| 5 | Illegal function call |
| 6 | Overflow |
| 7 | Out of memory |
| 8 | Label not defined |
| 9 | Subscript out of range |
| 10 | Duplicate definition |
| 11 | Division by zero (CRITICAL) |
| 13 | Type mismatch |
| 19 | No RESUME |
| 20 | RESUME without error |

### File I/O Errors (50-76)

| Code | Error |
|------|-------|
| 52 | Bad file name or number |
| 53 | File not found |
| 54 | Bad file mode |
| 55 | File already open |
| 62 | Input past end of file |
| 64 | Bad file name |
| 70 | Permission denied |
| 75 | Path/File access error |
| 76 | Path not found |

### Critical Errors (256+)

| Code | Error |
|------|-------|
| 256 | Out of stack space |
| 257 | Out of memory |
| 259 | Cannot find dynamic library |
| 260-261 | Function not in dynamic library |
| 270 | _GL outside SUB _GL |
| 300-315 | Memory violations |
| 314-315 | _ASSERT failures |

## Error State Variables

```c
uint32_t new_error;           // Pending error code
uint32_t error_occurred;      // Error triggered flag
uint32_t error_retry;         // RESUME retry flag
uint32_t error_err;           // Current error being handled
uint32_t error_goto_line;     // Active handler (0 = none)
qbs *error_handler_history;   // Stack of previous handlers
uint32_t error_handling;      // In handler flag (1 = yes)
```

## Error Dispatch

Generated in `mainerr.txt`:

```c
if (!error_handler_history) error_handler_history = qbs_new(0, 0);
if (error_occurred) {
    error_occurred = 0;
    if (error_goto_line == 1) { error_handling = 1; goto LABEL_HANDLER1; }
    if (error_goto_line == 2) { error_handling = 1; goto LABEL_HANDLER2; }
    // ... more handlers
    exit(99);  // No handler matched
}
```

## Critical vs Catchable Errors

**Critical errors** (cannot be caught):
- Division by zero (11)
- Out of stack space (256)
- Out of memory (257)
- Dynamic library errors (259-261)
- GL errors (270-271)
- Memory violations (300-315)

These display a dialog and exit immediately.

**Catchable errors** can be handled with ON ERROR GOTO.

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | ON ERROR/RESUME parsing (lines 9132-9298) |
| `internal/c/libqb/src/error_handle.cpp` | Runtime error functions |
| `internal/c/libqb/include/error_handle.h` | Error declarations |
| `internal/source/mainerr.txt` | Error dispatch table |

## Example Usage

### Basic Error Handling

```basic
ON ERROR GOTO ErrorHandler
OPEN "nonexistent.txt" FOR INPUT AS #1
CLOSE #1
END

ErrorHandler:
    PRINT "Error"; ERR; "on line"; ERL
    RESUME NEXT
```

### Nested Handlers

```basic
ON ERROR GOTO MainHandler
' Main code...

SUB RiskyOperation
    ON ERROR GOTO _NEWHANDLER LocalHandler
    ' Risky code...
    ON ERROR GOTO _LASTHANDLER
    EXIT SUB

    LocalHandler:
    PRINT "Local error handled"
    RESUME NEXT
END SUB

MainHandler:
    PRINT "Main error handler"
    RESUME NEXT
```

### Error Information

```basic
ON ERROR GOTO Handler
ERROR 53  ' Simulate file not found

Handler:
    PRINT "Error:"; ERR
    PRINT "Line:"; _ERRORLINE
    PRINT "Message:"; _ERRORMESSAGE$
    RESUME NEXT
```
