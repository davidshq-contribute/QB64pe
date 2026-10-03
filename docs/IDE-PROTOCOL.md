# IDE-PROTOCOL.md

Documentation of the QB64pe IDE-Compiler communication protocol.

## Overview

The QB64pe IDE and compiler communicate through a command-response protocol using shared BASIC variables, enabling real-time syntax checking, error reporting, and progress updates.

## Shared Variables

```basic
DIM SHARED idecommand AS STRING    ' Command from IDE/compiler
DIM SHARED idereturn AS STRING     ' Response data back
DIM SHARED ideerror AS LONG        ' Error flag
DIM SHARED idecompiled AS LONG     ' Compilation status
DIM SHARED idemode                 ' 1=IDE active, 0=Console
DIM SHARED ideerrorline AS LONG    ' Error line number
DIM SHARED idemessage AS STRING    ' Error message
```

## Entry Point

The `ide(ignore)` function in `source/ide/ide_methods.bas` is the sole coordination point between compiler and IDE.

## Command Protocol

### Command Bytes

| CMD | Direction | Purpose |
|-----|-----------|---------|
| 0 | IDE->Compiler | No IDE present |
| 1 | Compiler->IDE | Open file request |
| 2 | IDE->Compiler | Begin compilation |
| 3 | Compiler->IDE | Request next line |
| 4 | IDE->Compiler | Here is next line |
| 5 | IDE->Compiler | No more lines |
| 6 | Compiler->IDE | Compilation OK |
| 7 | Compiler->IDE | Rewind request |
| 8 | Compiler->IDE | Error occurred |
| 9 | IDE->Compiler | Run/Execute |
| 10 | Compiler->IDE | Return specific line |
| 11 | Compiler->IDE | ".EXE file created" |
| 12 | Compiler->IDE | Exe filename |
| 100 | Compiler->IDE | Simplified next line |
| 254 | Compiler->IDE | Debug mode done |
| 255 | IDE/Compiler | QB error in IDE |

### Message Format

```basic
idecommand$ = CHR$(command) + data$
idereturn$ = response_data$
```

For errors:
```basic
idecommand$ = CHR$(8) + message$ + MKL$(line_number)
```

## Line-by-Line Compilation

### Request Flow

```
Compiler                    IDE
   |                         |
   |-- CHR$(3) ------------->|  Request next line
   |                         |
   |<-- status=4, line$ -----|  Return line content
   |                         |
   |-- CHR$(3) ------------->|  Request next line
   |                         |
   |<-- status=5 ------------|  No more lines
```

### Implementation

```basic
' IDE returns next line
idecompiledline = idecompiledline + 1
indented$ = apply_layout_indent$(idecompiledline$)
IF idecompiledline$ <> indented$ THEN
    idesetline idecompiledline, indented$
END IF
idereturn$ = idetxt(idecompiledline)
RETURN 4
```

## Two-Pass Compilation

### Pass 1: Prepass

```basic
idepass = 1
prepass = 1
' Process $IF, $ELSE, types, CONST, etc.
```

### Pass 2: Main Compilation

```basic
idepass = 2
prepass = 0
' Full syntax checking and code generation
```

### Rewind Between Passes

```basic
sendc$ = CHR$(7)  ' Request rewind
GOTO sendcommand
```

## Error Reporting

### Error Assembly

```basic
errmes:
    IF idemode THEN
        ideerrorline = linenumber + erldiff
        idemessage$ = a$
        GOTO ideerror
    END IF

ideerror:
    sendc$ = CHR$(8) + idemessage$ + MKL$(ideerrorline)
    GOTO sendcommand
```

### Include File Errors

```basic
IF inclevel > 0 AND incerror$ <> "" THEN
    a$ = a$ + CHR$(1) + incerror$
END IF
```

## Progress Reporting

### Calculation

```basic
IF prepass THEN
    ' 0-50% during prepass
    progress$ = _TOSTR$(INT((idecompiledline * 100) / (iden * 2)))
ELSE
    ' 50-100% during main pass
    progress$ = _TOSTR$(INT(((iden + idecompiledline) * 100) / (iden * 2)))
END IF
```

### Display Update

```basic
IdeInfo = CHR$(0) + progress$
UpdateIdeInfo
```

## Command-Line Flags

| Flag | Purpose |
|------|---------|
| `-c` | Compile with progress window |
| `-x` | Compile with console output |
| `-p` | Purge pre-compiled content |

### Mode Variables

```basic
DIM SHARED AS _BYTE NoIDEMode, ConsoleMode
DIM SHARED idemode  ' 1 if using IDE
```

## Initial Connection

```basic
idemode = 1
sendc$ = ""
IF CMDLineSrcFile$ <> "" THEN
    sendc$ = CHR$(1) + CMDLineSrcFile$
END IF

sendcommand:
    idecommand$ = sendc$
    C = ide(0)
    ideerror = 0

IF C = 2 THEN  ' Begin compilation
    ideerrorline = 0
    idepass = 1
    GOTO fullrecompile
```

## Line Tracking Variables

```basic
DIM SHARED idecompiledline   ' Current line number
DIM SHARED idecompiledline$  ' Current line content
DIM SHARED iden              ' Total line count
```

## Architecture

```
+--------------------------------------------------+
| IDE (ide2/ide_methods.bas)                       |
| +- File display and editing                      |
| +- Error highlighting                            |
| +- Progress display                              |
+--------------------------------------------------+
| ide() - Lightweight coordinator                  |
| +- Receive: idecommand$                          |
| +- Return: status byte + idereturn$              |
+--------------------------------------------------+
| COMPILER (qb64pe.bas)                            |
| +- Request lines (cmd 3)                         |
| +- Send errors (cmd 8)                           |
| +- Request rewind (cmd 7)                        |
| +- Notify completion (cmd 6)                     |
+--------------------------------------------------+
```

## Auto-Formatting

During compilation, the IDE applies formatting:

```basic
indented$ = apply_layout_indent$(idecompiledline$)
IF LEN(indented$) _ANDALSO idecompiledline$ <> indented$ THEN
    idesetline idecompiledline, indented$
END IF
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | Compiler protocol (380-395, 784-855) |
| `source/ide/ide_methods.bas` | IDE coordinator (1-88) |
| `source/ide/ide_global.bas` | Line tracking (147-148) |

## Example Flow

### Successful Compilation

```
1. IDE sets idemode=1
2. Compiler sends CHR$(1)+filename
3. IDE returns status=2 (begin)
4. Compiler requests lines with CHR$(3)
5. IDE returns lines with status=4
6. After all lines, IDE returns status=5
7. Compiler sends CHR$(7) (rewind) for pass 2
8. Repeat line requests
9. Compiler sends CHR$(6) (success)
```

### Error During Compilation

```
1. Compiler detects syntax error
2. Sets ideerrorline, idemessage$
3. Sends CHR$(8)+message$+MKL$(line)
4. IDE highlights error line
5. User fixes error
6. IDE sends CHR$(2) (restart)
```

### Progress Display

```
Line 50 of 200 (prepass):  12%
Line 100 of 200 (prepass): 25%
Line 200 of 200 (prepass): 50%
Line 50 of 200 (main):     62%
Line 100 of 200 (main):    75%
Line 200 of 200 (main):   100%
```
