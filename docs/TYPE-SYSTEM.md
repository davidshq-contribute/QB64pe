# TYPE-SYSTEM.md

Documentation of the QB64pe type system implementation.

## Overview

QB64pe uses a bitfield-based type representation supporting QBasic-compatible types plus extended types like `_INTEGER64`, `_FLOAT`, `_OFFSET`, and `_BIT`.

## Type Encoding

### Type Flags (Bitfield)

| Bit | Constant | Value | Meaning |
|-----|----------|-------|---------|
| 30 | ISSTRING | 0x40000000 | String type |
| 29 | ISFLOAT | 0x20000000 | Floating point |
| 28 | ISUNSIGNED | 0x10000000 | Unsigned variant |
| 27 | ISPOINTER | 0x08000000 | Pointer/reference |
| 26 | ISFIXEDLENGTH | 0x04000000 | Fixed-length string |
| 25 | ISINCONVENTIONALMEMORY | 0x02000000 | In conventional memory |
| 24 | ISOFFSETINBITS | 0x01000000 | Bit-based offset |
| 23 | ISARRAY | 0x00800000 | Array type |
| 22 | ISREFERENCE | 0x00400000 | Reference/pointer |
| 21 | ISUDT | 0x00200000 | User-defined type |
| 20 | ISOFFSET | 0x00100000 | Offset type |

### Size Field (Lower 9 Bits)

Bits 0-8 store size in bits: 8, 16, 32, 64, 256 (for `_FLOAT`).

### Predefined Type Values

```basic
STRINGTYPE     = ISSTRING + ISPOINTER
BYTETYPE       = 8 + ISPOINTER
INTEGERTYPE    = 16 + ISPOINTER
LONGTYPE       = 32 + ISPOINTER
INTEGER64TYPE  = 64 + ISPOINTER
SINGLETYPE     = 32 + ISFLOAT + ISPOINTER
DOUBLETYPE     = 64 + ISFLOAT + ISPOINTER
FLOATTYPE      = 256 + ISFLOAT + ISPOINTER
OFFSETTYPE     = 64 + ISOFFSET + ISPOINTER
BITTYPE        = 1 + ISPOINTER + ISOFFSETINBITS
```

## Type Suffixes

| Symbol | Type | Bits | C Type |
|--------|------|------|--------|
| `%` | INTEGER | 16 | int16 |
| `%%` | _BYTE | 8 | int8 |
| `&` | LONG | 32 | int32 |
| `&&` | _INTEGER64 | 64 | int64 |
| `%&` | _OFFSET | 32/64 | intptr_t |
| `!` | SINGLE | 32 | float |
| `#` | DOUBLE | 64 | double |
| `##` | _FLOAT | 256 | long double |
| `$` | STRING | variable | qbs* |
| `~` prefix | _UNSIGNED | - | unsigned variant |
| `` ` `` | _BIT | 1-64 | int/long |

## User Defined Types (UDTs)

### TYPE...END TYPE Declaration

```basic
TYPE MyRecord
    name AS STRING * 20
    value AS LONG
    data(10) AS INTEGER
END TYPE
```

### Internal Storage

**UDT Arrays (source/utilities/type.bi):**

| Array | Purpose |
|-------|---------|
| `udtxname()` | UDT names |
| `udtxcname()` | Case-sensitive names |
| `udtxsize()` | Total size in bits |
| `udtxnext()` | Link to first element |
| `udtxvariable()` | Contains variable-length strings |

**Element Arrays:**

| Array | Purpose |
|-------|---------|
| `udtename()` | Element names |
| `udtesize()` | Element size in bits |
| `udtetype()` | Element type code |
| `udtetypesize()` | Fixed string size |
| `udtearrayelements()` | Array dimensions |
| `udtenext()` | Link to next element |

### Variable-Length String Handling

```basic
' Functions in source/utilities/type.bas
initialise_udt_varstrings()  ' Allocate strings in UDT
free_udt_varstrings()        ' Free strings
clear_udt_with_varstrings()  ' Clear strings
copy_full_udt()              ' Deep copy with strings
```

### Reserved UDT: _MEM

UDT #1 is reserved for the `_MEM` type with 8 elements (OFFSET, SIZE, TYPE, ELEMENTSIZE, IMAGE, SOUND, $_LOCK_ID, $_LOCK_OFFSET).

## Type Conversion Functions

### Explicit Conversions

| Function | Target Type | C Implementation |
|----------|-------------|------------------|
| CINT | INTEGER | func_cint_* |
| CLNG | LONG | func_clng_* |
| CSNG | SINGLE | func_csng_* |
| CDBL | DOUBLE | func_cdbl_* |
| _MK$ | Binary string | func_mk* |
| _CV | From binary string | func_cv* |

### Conversion Rules

**Float to Integer:**
- Uses `qbr()` rounding function
- Range checking applied
- Overflow triggers error

**Integer Type Promotion:**
- Float > Integer
- Unsigned > Signed
- Larger > Smaller

**String Conversions:**
- No automatic numeric-to-string
- STR$ for explicit conversion
- VAL for string-to-numeric

## Variable Declaration

### DIM Statement

```basic
DIM variable AS type
DIM array(bounds) AS type
DIM SHARED variable AS type
```

### Declaration Modes

| Keyword | Scope | Lifetime |
|---------|-------|----------|
| DIM | Local | Procedure |
| STATIC | Local | Program |
| SHARED | Module | Program |
| COMMON | Cross-module | Program |

### ID Structure Fields

```basic
TYPE idstruct
    n AS STRING * 256           ' Variable name
    t AS LONG                   ' Type value (bitfield)
    tsize AS LONG              ' Type size (fixed strings)
    arraytype AS LONG          ' Array element type
    arrayelements AS INTEGER   ' Array dimensions
    share AS INTEGER           ' Sharing flags
    staticscope AS INTEGER     ' Static scope flag
END TYPE
```

## DEFxxx Statements

### Syntax

```basic
DEFINT A-Z      ' All variables default to INTEGER
DEFLNG A-Z      ' All variables default to LONG
DEFSNG A-Z      ' All variables default to SINGLE (default)
DEFDBL A-Z      ' All variables default to DOUBLE
DEFSTR A-Z      ' All variables default to STRING
_DEFINE A-Z AS type  ' Extended version
```

### Implementation

```basic
DIM SHARED defineaz(1 TO 27) AS STRING * 20
' Index 1-26 = A-Z, Index 27 = underscore
' Default initialization: all set to "SINGLE"
```

## Type Helper Functions

### source/utilities/type.bas

| Function | Purpose |
|----------|---------|
| `symboltype()` | Parse symbol to type value |
| `typevalue2symbol$()` | Convert type to symbol |
| `symbol2fulltypename$()` | Expand to full name |
| `typ2ctyp$()` | Convert to C type name |
| `Type_GetSizeInBits()` | Extract size (AND 511) |
| `Type_IsString()` | Test ISSTRING flag |
| `Type_IsFloatingPoint()` | Test ISFLOAT flag |
| `Type_IsUnsigned()` | Test ISUNSIGNED flag |
| `Type_IsIntegral()` | Not float, not string |
| `Type_IsOffset()` | Test ISOFFSET flag |
| `Type_IsBit()` | Test ISOFFSETINBITS flag |
| `Type_PromoteArithmeticType()` | Binary op promotion |

## C Type Mapping

| QB64 Type | C Type | Size |
|-----------|--------|------|
| _BYTE | int8 | 8 bits |
| _UNSIGNED _BYTE | uint8 | 8 bits |
| INTEGER | int16 | 16 bits |
| _UNSIGNED INTEGER | uint16 | 16 bits |
| LONG | int32 | 32 bits |
| _UNSIGNED LONG | uint32 | 32 bits |
| _INTEGER64 | int64 | 64 bits |
| _UNSIGNED _INTEGER64 | uint64 | 64 bits |
| SINGLE | float | 32 bits |
| DOUBLE | double | 64 bits |
| _FLOAT | long double | 80-128 bits |
| _OFFSET | ptrszint | 32/64 bits |
| STRING | qbs* | variable |
| _BIT * n | int32/int64 | n bits |

## Runtime String Structure

```c
struct qbs {
    uint8_t *chr;              // Pointer to string data
    int32_t len;               // String length
    uint8_t in_cmem;           // In conventional memory
    uint16_t *cmem_descriptor; // CMEM descriptor
    uint32_t listi;            // String list index
    uint8_t tmp;               // Temporary flag
    uint8_t fixed;             // Fixed-length flag
    uint8_t readonly;          // Read-only flag
    qbs_field *field;          // File field info
};
```

## Key Source Files

| File | Content |
|------|---------|
| `source/utilities/type.bi` | Type constants |
| `source/utilities/type.bas` | Type functions |
| `source/qb64pe.bas` | Type parsing |
| `internal/c/libqb/include/qbs.h` | String structure |
| `internal/c/libqb/src/qbs_mk_cv.cpp` | Conversions |

## Example Usage

### Type Declarations

```basic
DIM b AS _BYTE
DIM i AS INTEGER
DIM l AS LONG
DIM i64 AS _INTEGER64
DIM s AS SINGLE
DIM d AS DOUBLE
DIM f AS _FLOAT
DIM off AS _OFFSET
DIM bits AS _BIT * 4
DIM ub AS _UNSIGNED _BYTE
```

### Type Suffixes

```basic
b%% = 127           ' _BYTE
i% = 32767          ' INTEGER
l& = 2147483647     ' LONG
i64&& = 9223372036854775807  ' _INTEGER64
s! = 3.14           ' SINGLE
d# = 3.14159265359  ' DOUBLE
f## = 3.14159265358979323846  ' _FLOAT
str$ = "Hello"      ' STRING
```

### User Defined Types

```basic
TYPE Vector3D
    x AS SINGLE
    y AS SINGLE
    z AS SINGLE
END TYPE

DIM v AS Vector3D
v.x = 1.0
v.y = 2.0
v.z = 3.0
```

### Type Conversions

```basic
i% = CINT(3.7)      ' i% = 4 (rounded)
l& = CLNG(i%)       ' Widen to LONG
s! = CSNG(d#)       ' Narrow to SINGLE
d# = CDBL(s!)       ' Widen to DOUBLE
```
