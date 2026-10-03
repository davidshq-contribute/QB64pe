# USER-INPUT.md

Documentation of the QB64pe user input system.

## Overview

QB64pe provides text-based input through INPUT, LINE INPUT, and INPUT$ with cursor positioning via LOCATE and simulated input via _SCREENPRINT.

## INPUT Statement

```basic
INPUT variable
INPUT "prompt"; variable
INPUT "prompt", variable
INPUT ; variable              ' No newline after
INPUT variable1, variable2    ' Multiple variables
INPUT #filenum, variable      ' File input
```

### Prompt Formatting

| Syntax | Behavior |
|--------|----------|
| `INPUT var` | Displays "? " |
| `INPUT "text"; var` | Displays "text? " |
| `INPUT "text", var` | Displays "text" only |
| `INPUT ; var` | No prompt, no newline |

### Type Handling

- **String**: Accepts any input
- **Numeric**: Validates format, rejects invalid
- **Multiple**: Comma-separated values

### Validation

```basic
INPUT "Enter number: "; n%
' Rejects: "abc", "12.5" (for integer)
' Accepts: "123", "-45", "&HFF"
```

Supported numeric formats:
- Decimal: `123`, `-45`, `3.14`
- Hex: `&HFF`, `&h2a`
- Octal: `&O77`
- Binary: `&B1010`

## LINE INPUT Statement

```basic
LINE INPUT text$
LINE INPUT "prompt"; text$
LINE INPUT #filenum, text$
```

### Characteristics

- Reads entire line including commas
- Single string variable only
- No type validation
- Ends at Enter/newline

### Examples

```basic
LINE INPUT "Enter full name: "; fullName$
LINE INPUT "Enter CSV data: "; csvLine$
```

## INPUT$ Function

```basic
char$ = INPUT$(1)           ' Read 1 character from keyboard
data$ = INPUT$(n)           ' Read n characters
data$ = INPUT$(n, #filenum) ' Read from file
```

### Behavior

- **Keyboard**: Waits for exactly n keypresses
- **File**: Reads up to n bytes
- No echo to screen
- Includes control characters

### Examples

```basic
' Wait for single keypress
PRINT "Press any key..."
k$ = INPUT$(1)

' Read 100 bytes from file
OPEN "data.bin" FOR BINARY AS #1
data$ = INPUT$(100, #1)
CLOSE #1
```

## LOCATE Statement

```basic
LOCATE [row][, column][, cursor][, start, stop]
```

### Parameters

| Parameter | Description |
|-----------|-------------|
| row | Vertical position (1-based) |
| column | Horizontal position (1-based) |
| cursor | 0=hide, 1=show |
| start | Cursor start scanline |
| stop | Cursor end scanline |

### Examples

```basic
LOCATE 10, 20           ' Move to row 10, column 20
LOCATE , 1              ' Move to column 1 (same row)
LOCATE 1, 1, 1          ' Top-left, cursor visible
LOCATE , , 0            ' Hide cursor
```

### Query Position

```basic
row% = CSRLIN           ' Current row
col% = POS(0)           ' Current column
```

## _SCREENPRINT Statement

```basic
_SCREENPRINT text$
```

### Purpose

Simulates keyboard input by injecting characters.

### Implementation

- Windows: `SendInput()` API
- macOS: `CGEventCreateKeyboardEvent()`
- Linux: XTest extension

### Examples

```basic
' Simulate typing
_SCREENPRINT "Hello, World!"

' Include special keys
_SCREENPRINT CHR$(13)   ' Enter key
```

## Input Loop Internals

### Character Processing

| Key | Action |
|-----|--------|
| Enter (13) | Accept input |
| Backspace (8) | Delete last char |
| Tab (9) | Expand to 8 spaces |
| Other ASCII | Add to buffer |

### Validation Flow

```
Input → Parse → Type Check → Convert → Accept/Reject
```

If invalid:
- Beep sound
- Return to input prompt
- User must re-enter

## File Input

### INPUT #

```basic
OPEN "data.txt" FOR INPUT AS #1
INPUT #1, name$, age%, score#
CLOSE #1
```

Parses comma-separated values with quote handling.

### LINE INPUT #

```basic
OPEN "text.txt" FOR INPUT AS #1
DO UNTIL EOF(1)
    LINE INPUT #1, line$
    PRINT line$
LOOP
CLOSE #1
```

### INPUT$

```basic
OPEN "binary.dat" FOR BINARY AS #1
header$ = INPUT$(10, #1)
CLOSE #1
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | INPUT parsing (10060-10280) |
| `internal/c/libqb.cpp` | qbs_input() (12651-13685) |
| `internal/c/libqb.cpp` | LOCATE (12386-12535) |
| `internal/c/libqb.cpp` | INPUT$ (18036-18165) |
| `internal/c/libqb.cpp` | _SCREENPRINT (24317+) |

## Example Usage

### Basic Input

```basic
INPUT "What is your name"; name$
INPUT "How old are you"; age%
PRINT "Hello, "; name$; "! You are"; age%; "years old."
```

### Menu System

```basic
DO
    CLS
    PRINT "1. Option One"
    PRINT "2. Option Two"
    PRINT "3. Exit"
    PRINT
    INPUT "Choose: "; choice%

    SELECT CASE choice%
        CASE 1: PRINT "You chose One"
        CASE 2: PRINT "You chose Two"
        CASE 3: EXIT DO
    END SELECT
    SLEEP 1
LOOP
```

### Form Input

```basic
CLS
LOCATE 5, 10: PRINT "Name:"
LOCATE 5, 20: LINE INPUT ; name$

LOCATE 7, 10: PRINT "Email:"
LOCATE 7, 20: LINE INPUT ; email$

LOCATE 9, 10: PRINT "Age:"
LOCATE 9, 20: INPUT ; age%
```

### Password Input

```basic
PRINT "Password: ";
password$ = ""
DO
    k$ = INPUT$(1)
    IF k$ = CHR$(13) THEN EXIT DO
    IF k$ = CHR$(8) AND LEN(password$) > 0 THEN
        password$ = LEFT$(password$, LEN(password$) - 1)
        PRINT CHR$(8); " "; CHR$(8);
    ELSEIF k$ >= " " THEN
        password$ = password$ + k$
        PRINT "*";
    END IF
LOOP
PRINT
```

### Data Entry with Validation

```basic
DO
    INPUT "Enter positive number (0 to quit): "; n%
    IF n% < 0 THEN
        PRINT "Must be positive!"
    ELSEIF n% > 0 THEN
        PRINT "You entered:"; n%
    END IF
LOOP UNTIL n% = 0
```

### File Processing

```basic
OPEN "config.txt" FOR INPUT AS #1
DO UNTIL EOF(1)
    LINE INPUT #1, line$
    IF LEFT$(line$, 1) <> "#" THEN  ' Skip comments
        eq = INSTR(line$, "=")
        IF eq > 0 THEN
            key$ = LEFT$(line$, eq - 1)
            value$ = MID$(line$, eq + 1)
            PRINT key$; " = "; value$
        END IF
    END IF
LOOP
CLOSE #1
```
