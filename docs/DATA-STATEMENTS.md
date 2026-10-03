# DATA-STATEMENTS.md

Documentation of the QB64pe DATA/READ/RESTORE system.

## Overview

QB64pe implements DATA statements that embed constant values directly in the executable, accessible via READ statements with optional RESTORE for repositioning.

## DATA Statement

```basic
DATA 1, 2, 3, "Hello", 4.5
DATA "Quoted, with comma", 100
```

### Parsing Rules

- Comma-separated values
- Quoted strings preserve commas and spaces
- Unquoted values trimmed of whitespace
- Decimal points allowed (`.5` = `0.5`)
- Colon `:` terminates DATA line

### Storage Format

Data is embedded as a byte array in the executable:

```c
uint8 inline_data[]={
    72,101,108,108,111,44,49,50,51,44,45,52,53,46,54,55,
    0};
ptrszint data_size=16;
uint8 *data=&inline_data[0];
```

## READ Statement

```basic
READ a%, b$, c#
```

### Type-Specific Functions

| QB64 Type | C Function |
|-----------|------------|
| STRING | `sub_read_string()` |
| SINGLE/DOUBLE | `func_read_float()` |
| INTEGER/LONG/_INTEGER64 | `func_read_int64()` |
| _UNSIGNED variants | `func_read_uint64()` |

### Number Parsing

Supported formats:
- Decimal: `123`, `-456`, `3.14`, `1.5e10`
- Hexadecimal: `&H1AF`, `&h2c3`
- Octal: `&O755`
- Binary: `&B1010`
- Type suffixes: `123!`, `456#`, `789&&`

## RESTORE Statement

```basic
RESTORE              ' Reset to beginning
RESTORE labelname    ' Reset to specific label
```

### Label Data Offsets

```c
// Generated for RESTORE label
ptrszint data_at_LABEL_mydata = 42;

// RESTORE mydata generates:
data_offset = data_at_LABEL_mydata;
```

### Label Scope

- RESTORE labels have global scope
- Only one instance of each label allowed
- Forward references permitted

## Data Pointer Management

```c
extern ptrszint data_offset;  // Current read position
extern ptrszint data_size;    // Total size in bytes
extern uint8 *data;           // Pointer to data array
```

### String Reading

State machine with quote tracking:
- State 0: Outside quotes
- State 1: Inside quotes
- State 2: After closing quote

```c
void sub_read_string(uint8 *data, ptrszint *data_offset,
                     ptrszint data_size, qbs *deststr) {
    if (*data_offset >= data_size) {
        error(4);  // Out of DATA
        return;
    }
    // Parse until comma or end
    // Handle quoted strings
}
```

### Number Reading

```c
int64 func_read_int64(uint8 *data, ptrszint *data_offset,
                      ptrszint data_size) {
    ptrszint old_offset = *data_offset;  // Save for rollback
    int result = n_inputnumberfromdata(data, data_offset, data_size);

    if (result == 1) { *data_offset = old_offset; error(6); }  // Overflow
    if (result == 2) { error(4); }  // Out of DATA
    if (result == 3) { *data_offset = old_offset; error(2); }  // Syntax

    return n_int64_value;
}
```

## Error Codes

| Code | Error |
|------|-------|
| 2 | Syntax error (invalid number format) |
| 4 | Out of DATA |
| 6 | Overflow (number too large) |

## Compiler Implementation

### Data Buffer

```basic
DIM SHARED DataBinBuf
DataBinBuf = OpenBuffer%("O", tmpdir$ + "data.bin")
```

### Offset Tracking

```basic
DIM SHARED DataOffset AS _INTEGER64  ' Cumulative data length
```

### Label Structure

```basic
TYPE Label_Type
    State AS _UNSIGNED _BYTE
    cn AS STRING * 256          ' Canonical name
    Scope AS LONG
    Data_Offset AS _INTEGER64   ' Position in data stream
    Data_Referenced AS _UNSIGNED _BYTE
END TYPE
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | DATA parsing (20832-20930), READ (23369-23424), RESTORE (9262-9298) |
| `internal/c/libqb.cpp` | Runtime read functions (14829-15650) |
| `internal/temp/maindata.txt` | Generated data declarations |

## Example Usage

### Basic Data Reading

```basic
DATA 10, 20, 30, "Hello", "World"

FOR i = 1 TO 3
    READ n%
    PRINT n%
NEXT

READ a$, b$
PRINT a$; " "; b$
```

### Using RESTORE

```basic
StartData:
DATA 1, 2, 3

MoreData:
DATA 4, 5, 6

' Read first set
FOR i = 1 TO 3: READ n%: PRINT n%;: NEXT
PRINT

' Go back and read again
RESTORE StartData
FOR i = 1 TO 3: READ n%: PRINT n%;: NEXT
PRINT

' Jump to second set
RESTORE MoreData
FOR i = 1 TO 3: READ n%: PRINT n%;: NEXT
```

### Mixed Types

```basic
DATA "Player1", 100, 95.5
DATA "Player2", 85, 88.3

TYPE Player
    name AS STRING * 20
    score AS INTEGER
    accuracy AS SINGLE
END TYPE

DIM p AS Player

FOR i = 1 TO 2
    READ p.name, p.score, p.accuracy
    PRINT p.name; p.score; p.accuracy
NEXT
```

### Hex/Binary Data

```basic
DATA &HFF, &B11110000, &O777

READ hex%, bin%, oct%
PRINT HEX$(hex%), BIN$(bin%), OCT$(oct%)
' Output: FF            11110000      777
```
