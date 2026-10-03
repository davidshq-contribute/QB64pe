# LEGACY-COMPAT.md

Documentation of QB64pe's QBasic/QuickBASIC legacy compatibility features.

## Overview

QB64pe provides compatibility with legacy QBasic features including direct memory access (PEEK/POKE), segment addressing (DEF SEG), pointer operations (VARPTR), file operations (BLOAD/BSAVE), hardware I/O emulation (INP/OUT), and interrupt calls. These are emulated through the CMEM (Conventional Memory) subsystem.

## CMEM - Conventional Memory Emulation

The CMEM system emulates DOS conventional memory for legacy compatibility:

```c
// internal/c/libqb/src/mem.cpp
uint8 *cmem;           // 1MB emulated memory block
int32 cmem_size;       // Current allocation size
```

### Memory Layout

```
Segment:Offset addressing emulated
Linear address = segment * 16 + offset
Maximum: 1MB (0x00000 - 0xFFFFF)
```

## DEF SEG Statement

```basic
DEF SEG              ' Reset to default segment
DEF SEG = segment    ' Set current segment
```

### Implementation

```c
// Sets the current segment for PEEK/POKE operations
void sub_defseg(int32 segment, int32 passed) {
    if (!passed) {
        defseg = 0;  // Default segment
    } else {
        defseg = segment;
    }
}
```

## PEEK and POKE

```basic
value = PEEK(offset)     ' Read byte from segment:offset
POKE offset, value       ' Write byte to segment:offset
```

### Implementation

```c
uint8 func_peek(int32 offset) {
    int32 addr = defseg * 16 + offset;
    if (addr < 0 || addr >= cmem_size) return 0;
    return cmem[addr];
}

void sub_poke(int32 offset, int32 value) {
    int32 addr = defseg * 16 + offset;
    if (addr >= 0 && addr < cmem_size) {
        cmem[addr] = value & 0xFF;
    }
}
```

## Pointer Functions

### VARPTR - Variable Pointer

```basic
ptr = VARPTR(variable)   ' Get offset of variable in CMEM
```

Returns the offset within the current segment.

### VARSEG - Variable Segment

```basic
seg = VARSEG(variable)   ' Get segment of variable
```

Returns the segment containing the variable.

### SADD - String Address

```basic
addr = SADD(string$)     ' Get address of string data
```

Returns pointer to the actual string character data.

### _OFFSET - Modern Pointer

```basic
ptr~%& = _OFFSET(variable)   ' Get native memory pointer
```

Returns actual memory address (not CMEM emulated).

## BLOAD and BSAVE

```basic
BSAVE filename$, offset, length   ' Save memory to file
BLOAD filename$, offset           ' Load file to memory
```

### Implementation

```c
void sub_bsave(qbs *filename, int32 offset, int32 length) {
    int32 addr = defseg * 16 + offset;
    // Write cmem[addr] to cmem[addr+length-1] to file
}

void sub_bload(qbs *filename, int32 offset, int32 passed) {
    int32 addr = defseg * 16 + offset;
    // Read file into cmem starting at addr
}
```

## INP and OUT - Port I/O Emulation

```basic
value = INP(port)        ' Read from I/O port
OUT port, value          ' Write to I/O port
```

### Emulated Ports

| Port | Function |
|------|----------|
| `&H60` | Keyboard scancode |
| `&H61` | PC speaker control |
| `&H201` | Joystick port |
| `&H3C7-3C9` | VGA palette |
| `&H3DA` | VGA status |

### Implementation

Most ports return 0 or perform no operation. Some have partial emulation for compatibility:

```c
uint8 func_inp(int32 port) {
    switch (port) {
        case 0x60: return last_scancode;
        case 0x3DA: return vga_status;
        default: return 0;
    }
}
```

## CALL INTERRUPT

```basic
CALL INTERRUPT(intnum, inregs, outregs)
CALL INTERRUPTX(intnum, inregs, outregs)
```

### Register TYPE

```basic
TYPE RegType
    AX AS INTEGER
    BX AS INTEGER
    CX AS INTEGER
    DX AS INTEGER
    BP AS INTEGER
    SI AS INTEGER
    DI AS INTEGER
    FLAGS AS INTEGER
END TYPE
```

### Emulated Interrupts

| INT | Function |
|-----|----------|
| `&H10` | Video services (partial) |
| `&H21` | DOS services (partial) |
| `&H33` | Mouse services |

Most interrupts are no-ops or return dummy values for compatibility.

## CALL ABSOLUTE

```basic
CALL ABSOLUTE(address)
```

Legacy machine code execution - not supported in QB64pe (returns immediately).

## Screen Memory Access

```basic
DEF SEG = &HB800         ' Text mode video memory
POKE offset, char        ' Write character
POKE offset + 1, attr    ' Write attribute
```

QB64pe emulates text mode video memory at segment &HB800.

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/src/mem.cpp` | CMEM, PEEK/POKE |
| `internal/c/libqb/src/legacy.cpp` | Legacy functions |
| `internal/c/libqb/src/io.cpp` | INP/OUT emulation |
| `source/qb64pe.bas` | Statement parsing |

## Compatibility Notes

| Feature | Status |
|---------|--------|
| PEEK/POKE | Full emulation via CMEM |
| DEF SEG | Segment tracking |
| VARPTR/VARSEG | CMEM offsets |
| BLOAD/BSAVE | File I/O supported |
| INP/OUT | Limited emulation |
| CALL INTERRUPT | Partial emulation |
| CALL ABSOLUTE | Not supported |

## Example Usage

```basic
' Screen memory access (text mode)
DEF SEG = &HB800
FOR i = 0 TO 79
    POKE i * 2, ASC("*")      ' Character
    POKE i * 2 + 1, 14        ' Yellow on black
NEXT

' Variable pointer
DIM x AS INTEGER
x = 12345
DEF SEG = VARSEG(x)
lowByte = PEEK(VARPTR(x))
highByte = PEEK(VARPTR(x) + 1)

' Save/Load memory
DIM buffer(1 TO 1000) AS _BYTE
DEF SEG = VARSEG(buffer(1))
BSAVE "data.bin", VARPTR(buffer(1)), 1000
BLOAD "data.bin", VARPTR(buffer(1))

' Modern alternative: _MEM
DIM m AS _MEM
m = _MEM(x)
_MEMPUT m, m.OFFSET, 12345 AS INTEGER
```
