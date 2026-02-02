# DATA-RESTORE.md

Documentation of the QB64pe DATA/READ/RESTORE implementation.

## Overview

The DATA/READ/RESTORE system provides compile-time data embedding and runtime sequential access, compatible with QBasic.

## DATA Statement

### Compile-Time Processing

DATA statements are parsed during compilation (qb64pe.bas lines 20832-20930):

1. Character-by-character parsing
2. Comma-separated values extracted
3. Quoted strings preserved with quotes
4. Leading/trailing whitespace trimmed

### Data Buffers

| Buffer | File | Purpose |
|--------|------|---------|
| `DataBinBuf` | `temp/data.bin` | Raw binary data bytes |
| `DataTxtBuf` | `temp/maindata.txt` | Text declarations |

### Encoding

- Raw bytes written to `DataBinBuf`
- Each byte converted to decimal for C embedding
- `DataOffset` incremented by data length

### Final Embedding

At compilation end, data embedded as C array:

```c
uint8 inline_data[] = {
    72, 101, 108, 108, 111, ...  // "Hello"
};
```

## Label Tracking

### Label_Type Structure

```basic
TYPE Label_Type
    State AS _UNSIGNED _BYTE      ' 0=referenced, 1=created
    cn AS STRING * 256            ' Canonical name
    Scope AS LONG                 ' Scope identifier
    Data_Offset AS _INTEGER64     ' Offset within DATA
    Data_Referenced AS _BYTE      ' Referenced by RESTORE
    Error_Line AS LONG            ' Error reporting line
    SourceLineNumber AS LONG      ' Source line number
END TYPE
```

### Data Offset Capture

When a label is created:
```basic
linedataoffset = DataOffset       ' Capture current position
Labels(r).Data_Offset = linedataoffset
```

Generated global variable:
```c
ptrszint data_at_LABEL_mylabel = 42;  // Offset in bytes
```

## RESTORE Statement

### Syntax

```basic
RESTORE              ' Reset to beginning
RESTORE labelname    ' Reset to label position
```

### Generated Code

**Without label:**
```c
data_offset = 0;
```

**With label:**
```c
data_offset = data_at_LABEL_mylabel;
```

### Constraints

- RESTORE labels must be globally unique (no scope)
- Forward references allowed (label can be defined later)
- Pre-compilation validation prevents ambiguous references

## READ Statement

### Type-Specific Functions

| QB64 Type | C Function |
|-----------|------------|
| STRING | `sub_read_string()` |
| SINGLE/DOUBLE | `func_read_float()` |
| INTEGER/LONG/_INTEGER64 | `func_read_int64()` |
| _UNSIGNED variants | `func_read_uint64()` |

### String Reading (libqb.cpp 15459-15514)

```c
void sub_read_string(uint8 *data, ptrszint *data_offset,
                     ptrszint data_size, qbs *deststr)
```

**States:**
- 0 = Normal (outside quotes)
- 1 = Inside quotes
- 2 = After closing quote

**Delimiters:**
- Comma outside quotes ends value
- Closing quote followed by comma

### Numeric Reading

```c
int64 func_read_int64(uint8 *data, ptrszint *data_offset,
                      ptrszint data_size)
```

**Process:**
1. Call `n_inputnumberfromdata()` to parse
2. Validate range based on target type
3. Return converted value

## Number Parsing (n_inputnumberfromdata)

### Supported Formats

| Format | Example |
|--------|---------|
| Decimal | `123`, `-456`, `3.14`, `1.5e10` |
| Hexadecimal | `&H1AF`, `&h2c3` |
| Octal | `&O755`, `&o123` |
| Binary | `&B1010`, `&b11` |
| Type Suffix | `123!`, `456#`, `789&&` |
| Exponent | `1.5E10`, `2.3D-5` |

### State Machine

| Step | State |
|------|-------|
| 0 | Initial (wait for sign/digit) |
| 1 | Integer part (before decimal) |
| 2 | Fraction part (after decimal) |
| 3 | Exponent marker detected |
| 4 | Exponent value |
| 5 | Type suffix |

### Return Values

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Overflow |
| 2 | Out of DATA |
| 3 | Syntax error |

## Runtime Data Access

### Global Variables

```c
extern ptrszint data_size;      // Total size in bytes
extern uint8 *data;             // Pointer to data array
extern ptrszint data_offset;    // Current read position
```

### Read Sequence

1. Call read function with current offset
2. Function parses and increments offset
3. On error, offset restored to pre-call state
4. Value returned or `error()` called

## Error Handling

| Error | Code | Cause |
|-------|------|-------|
| Out of DATA | 4 | READ past end |
| Syntax error | 2 | Invalid number format |
| Overflow | 6 | Value too large |

### Offset Restoration

On error, offset is restored:

```c
static ptrszint old_data_offset;
old_data_offset = *data_offset;
i = n_inputnumberfromdata(data, data_offset, data_size);
if (i == 1 || i == 3) {
    *data_offset = old_data_offset;  // Restore
    error(...);
}
```

## Limitations

### Parsing Edge Cases

1. **Type Suffix**: Must come after number, not before
2. **Quoted Strings**: No escape mechanism for quotes inside strings
3. **Decimal Point**: `.5` valid (treated as 0.5)
4. **Whitespace**: Leading/trailing trimmed, inside quotes preserved

### Overflow Limits

| Limit | Value |
|-------|-------|
| Digits per number | 256 max |
| Exponent digits | 18 max |
| Float range | ±1.7976931348623157E308 |
| Int64 range | -9.2E18 to 9.2E18 |
| Hex digits | 16 max |
| Octal digits | 22 max |
| Binary digits | 64 max |

### Type Conversion

- Hex/octal/binary in signed context: top bit set → negative
- Float to integer: Range check, then `qbr()` rounding
- String to numeric: Type error (no auto-conversion)

### Label Limitations

- RESTORE labels have no scope (globally unique)
- DATA statements cannot be nested in blocks
- No multi-line DATA in standard syntax

## Example

### QB64 Code

```basic
DATA "Hello", 123, 45.67, &HFF

start:
DATA "World", 789

DIM s AS STRING, i AS LONG, f AS SINGLE

READ s, i, f      ' s="Hello", i=123, f=45.67

RESTORE start
READ s, i         ' s="World", i=789
```

### Generated C (simplified)

```c
uint8 inline_data[] = {
    // "Hello",123,45.67,255,"World",789
    72,101,108,108,111,44,49,50,51,44,...
};
ptrszint data_size = sizeof(inline_data);
uint8 *data = inline_data;
ptrszint data_offset = 0;

ptrszint data_at_LABEL_start = 18;  // Offset to "World"

// READ s, i, f
sub_read_string(data, &data_offset, data_size, s);
i = func_read_int64(data, &data_offset, data_size);
f = func_read_float(data, &data_offset, data_size, SINGLETYPE);

// RESTORE start
data_offset = data_at_LABEL_start;

// READ s, i
sub_read_string(data, &data_offset, data_size, s);
i = func_read_int64(data, &data_offset, data_size);
```

## Error Precedence

1. **Out of DATA (4)** - Checked first
2. **Syntax Error (2)** - Invalid format
3. **Overflow (6)** - Value too large
