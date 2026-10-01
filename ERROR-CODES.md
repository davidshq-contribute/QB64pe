# ERROR-CODES.md

Comprehensive documentation of QB64pe error codes and error handling.

## Error Code Reference

### Syntax & Control Flow Errors (1-40)

| Code | Error | Description |
|------|-------|-------------|
| 1 | NEXT without FOR | NEXT statement without matching FOR |
| 2 | Syntax error | General syntax error |
| 3 | RETURN without GOSUB | RETURN without matching GOSUB |
| 4 | Out of DATA | READ past end of DATA |
| 5 | Illegal function call | Invalid argument to function |
| 6 | Overflow | Numeric overflow |
| 7 | Out of memory | Memory allocation failed |
| 8 | Label not defined | GOTO/GOSUB to undefined label |
| 9 | Subscript out of range | Array index out of bounds |
| 10 | Duplicate definition | Variable/label already defined |
| 11 | Division by zero | **CRITICAL** - Exits program |
| 12 | Illegal in direct mode | Statement not allowed in immediate mode |
| 13 | Type mismatch | Incompatible types |
| 14 | Out of string space | String memory exhausted |
| 16 | String formula too complex | Expression too nested |
| 17 | Cannot continue | Cannot resume after error |
| 18 | Function not defined | Called undefined function |
| 19 | No RESUME | Error handler didn't RESUME |
| 20 | RESUME without error | RESUME when not in error handler |
| 24 | Device timeout | I/O operation timed out |
| 25 | Device fault | Hardware device error |
| 26 | FOR without NEXT | FOR loop not closed |
| 27 | Out of paper | Printer out of paper |
| 29 | WHILE without WEND | WHILE loop not closed |
| 30 | WEND without WHILE | WEND without matching WHILE |
| 33 | Duplicate label | Label already defined |
| 35 | Subprogram not defined | Called undefined SUB |
| 37 | Argument-count mismatch | Wrong number of arguments |
| 38 | Array not defined | Used undefined array |
| 40 | Variable required | Expected variable, got expression |

### File I/O Errors (50-76)

| Code | Error | Description |
|------|-------|-------------|
| 50 | FIELD overflow | FIELD exceeds record length |
| 51 | Internal error | Internal QB64 error |
| 52 | Bad file name or number | Invalid file handle |
| 53 | File not found | File doesn't exist |
| 54 | Bad file mode | Wrong OPEN mode for operation |
| 55 | File already open | File handle already in use |
| 56 | FIELD statement active | Can't change file while FIELD active |
| 57 | Device I/O error | Read/write error |
| 58 | File already exists | File exists (for new file) |
| 59 | Bad record length | Record length mismatch |
| 61 | Disk full | No space on disk |
| 62 | Input past end of file | READ past EOF |
| 63 | Bad record number | Invalid record number |
| 64 | Bad file name | Invalid filename characters |
| 67 | Too many files | Too many open files |
| 68 | Device unavailable | Device not accessible |
| 69 | Communication-buffer overflow | Serial buffer overflow |
| 70 | Permission denied | Access denied by OS |
| 71 | Disk not ready | Removable media not ready |
| 72 | Disk-media error | Media read/write error |
| 73 | Feature unavailable | Feature not supported |
| 74 | Rename across disks | Can't rename across drives |
| 75 | Path/File access error | Path access denied |
| 76 | Path not found | Directory doesn't exist |

### System & Stack Errors (256-271)

| Code | Error | Critical | Description |
|------|-------|----------|-------------|
| 256 | Out of stack space | Yes | Call stack overflow |
| 257 | Out of memory | Yes | Fatal memory allocation |
| 258 | Invalid handle | No | Bad handle value |
| 259 | Cannot find dynamic library | Yes | DLL/SO not found |
| 260 | Function not in dynamic library | Yes | Symbol lookup failed |
| 261 | Function not in dynamic library | Yes | Symbol lookup failed (alt) |
| 270 | _GL outside SUB _GL | Yes | OpenGL call in wrong scope |
| 271 | END/SYSTEM in SUB _GL | Yes | Illegal exit from _GL |

### Memory Management Errors (300-315)

All memory errors are **CRITICAL** and cannot be caught:

| Code | Error |
|------|-------|
| 300 | Memory region out of range |
| 301 | Invalid size |
| 302 | Source memory region out of range |
| 303 | Destination memory region out of range |
| 304 | Source and destination memory regions out of range |
| 305 | Source memory has been freed |
| 306 | Destination memory has been freed |
| 307 | Memory already freed |
| 308 | Memory has been freed |
| 309 | Memory not initialized |
| 310 | Source memory not initialized |
| 311 | Destination memory not initialized |
| 312 | Source and destination memory not initialized |
| 313 | Source and destination memory have been freed |
| 314 | _ASSERT failed |
| 315 | _ASSERT failed (with description) |

### Out of Memory Fatal Errors (502-518)

Codes 502-518 are all "Out of memory" variants that cannot be caught. These represent different allocation contexts within the runtime.

## ON ERROR GOTO Implementation

### Syntax Forms

```basic
ON ERROR GOTO label           ' Standard error handler
ON ERROR GOTO 0               ' Clear all handlers and history
ON ERROR GOTO _NEWHANDLER label   ' Push new handler (track history)
ON ERROR GOTO _LASTHANDLER    ' Pop to previous handler
```

### Generated C++ Code

**Standard handler:**
```c
error_goto_line = <label_number>;
if (error_goto_line == <label_number>) {
    error_handling = 1;
    goto LABEL_<labelname>;
}
```

**_NEWHANDLER (push):**
```c
qbs_set(error_handler_history,
    qbs_add(qbs_add(qbs_str((int32)(error_goto_line)),
                    qbs_new_txt_len("|", 1)),
            error_handler_history));
qbs_cleanup(qbs_tmp_base, 0);
error_goto_line = <label_number>;
```

**_LASTHANDLER (pop):**
```c
error_goto_line = qbr(qbs_val<uint64_t>(error_handler_history));
qbs_set(error_handler_history,
    func_mid(error_handler_history,
             func_instr(NULL, error_handler_history, "|", 0) + 1,
             NULL, 0));
qbs_cleanup(qbs_tmp_base, 0);
```

**Clear all (0):**
```c
error_goto_line = 0;
qbs_set(error_handler_history, qbs_new_txt_len("\0", 0));
qbs_cleanup(qbs_tmp_base, 0);
```

## RESUME Variants

### RESUME (retry)

Re-executes the line that caused the error.

```c
if (!error_handling) { error(20); }
else {
    error_retry = 1;
    qbevent = 1;
    error_handling = 0;
    error_err = 0;
    return;
}
```

### RESUME NEXT

Continues at the line after the error.

```c
if (!error_handling) { error(20); }
else {
    error_handling = 0;
    error_err = 0;
    return;
}
```

### RESUME label

Jumps to specified label.

```c
if (!error_handling) { error(20); }
else {
    error_handling = 0;
    error_err = 0;
    goto LABEL_<label>;
}
```

### Comparison

| Feature | RESUME | RESUME NEXT | RESUME label |
|---------|--------|-------------|--------------|
| Sets `error_retry` | Yes | No | No |
| Execution Point | Same line | Next line | Label |
| Sets `qbevent` | Yes | No | No |

## Error State Variables

**File:** `internal/c/libqb/include/error_handle.h`

| Variable | Type | Purpose |
|----------|------|---------|
| `new_error` | uint32_t | Pending error code (0 = none) |
| `error_occurred` | uint32_t | Error was triggered flag |
| `error_retry` | uint32_t | Retry flag (RESUME only) |
| `error_err` | uint32_t | Current error being handled |
| `error_goto_line` | uint32_t | Active handler line (0 = none) |
| `error_handler_history` | qbs* | Stack of previous handlers |
| `error_handling` | uint32_t | Currently in handler (1 = yes) |

### Static Variables

| Variable | Type | Purpose |
|----------|------|---------|
| `error_erl` | double | ERL value (error line) |
| `ercl` | uint32_t | Error line in current file |
| `inclercl` | uint32_t | Error line in include file |
| `includedfilename` | const char* | Include file name |

## Error Functions

### Runtime Functions

```c
void error(int32_t error_number);      // Report error
void fix_error();                       // Process pending error
void clear_error();                     // Clear error state

double get_error_erl();                 // Get ERL
uint32_t get_error_err();               // Get ERR

int32_t func__errorline();              // _ERRORLINE
int32_t func__inclerrorline();          // _INCLERRORLINE
qbs *func__inclerrorfile();             // _INCLERRORFILE$
qbs *func__errormessage(int32_t code, int32_t passed);  // _ERRORMESSAGE$

static inline bool is_error_pending();  // Check for error
```

### QB64 Functions

| Function | Returns | Description |
|----------|---------|-------------|
| `ERR` | INTEGER | Current error code |
| `ERL` | LONG | Error line number |
| `_ERRORLINE` | LONG | Line in main module |
| `_INCLERRORLINE` | LONG | Line in include file |
| `_INCLERRORFILE$` | STRING | Include filename |
| `_ERRORMESSAGE$` | STRING | Error description |
| `_ERRORMESSAGE$(n)` | STRING | Description for code n |

## Error Handler History

The handler history is a pipe-delimited string stack:

```
"<current>|<prev1>|<prev2>|..."
```

### Push Operation

```basic
' Before: error_goto_line = 100, history = "50|"
ON ERROR GOTO _NEWHANDLER newhandler
' After: error_goto_line = 200, history = "100|50|"
```

### Pop Operation

```basic
' Before: error_goto_line = 200, history = "100|50|"
ON ERROR GOTO _LASTHANDLER
' After: error_goto_line = 100, history = "50|"
```

### Clear Operation

```basic
ON ERROR GOTO 0
' After: error_goto_line = 0, history = ""
```

## Error Dialog Display

### Critical Errors

Cannot be handled by user code. Shows dialog:

```
Title: "Critical Error #<code> - <program_name>"
Message: "Line: <line_number> (in <filename>)
          <error_description>"
Button: [OK]
```

Program exits after dialog dismissed.

### Regular Unhandled Errors

Shows dialog with option to continue:

```
Title: "Unhandled Error #<code> - <program_name>"
Message: "Line: <line_number> (in <filename>)
          <error_description>
          Continue?"
Buttons: [Yes] [No]
```

- Yes: Continue execution
- No/Timeout: Exit program

## Error Categories

### Catchable Errors

Can be caught with ON ERROR GOTO:
- 1-10, 12-20, 24-27, 29-30, 33, 35, 37-40 (Syntax/control)
- 50-76 (File I/O)
- 258 (Invalid handle)

### Critical Errors (Uncatchable)

Always terminate or show critical dialog:
- 11 (Division by zero)
- 256, 257 (Stack/memory)
- 259-261 (Dynamic library)
- 270-271 (Graphics scope)
- 300-315 (Memory violations)
- 502-518 (Out of memory variants)

## Example: Nested Error Handlers

```basic
ON ERROR GOTO _NEWHANDLER MainHandler

' Main code
OPEN "file.txt" FOR INPUT AS #1
ON ERROR GOTO _NEWHANDLER FileHandler
    ' File operations with FileHandler active
    LINE INPUT #1, a$
ON ERROR GOTO _LASTHANDLER  ' Back to MainHandler
CLOSE #1

ON ERROR GOTO _LASTHANDLER  ' Back to no handler
END

MainHandler:
    PRINT "Main error:"; ERR
    RESUME NEXT

FileHandler:
    PRINT "File error:"; ERR; "at line"; ERL
    RESUME NEXT
```

## Logging

All errors are logged via the logging system:

```c
libqb_log_error("QB64 Error %d reported: %s", error_number, human_error(error_number));
```

Logged with:
- Scope: Libqb
- Level: Error
- Contains: Error code and human-readable description
