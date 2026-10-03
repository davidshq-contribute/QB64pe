# BINARY-OPS.md

Documentation of the QB64pe bitwise operations implementation.

## Overview

QB64pe provides comprehensive bitwise operations including classic QBasic operators (AND, OR, XOR, NOT) and extended shift/rotate operations (_SHL, _SHR, _ROL, _ROR) plus individual bit manipulation functions.

## Classic Bitwise Operators

```basic
result = a AND b     ' Bitwise AND
result = a OR b      ' Bitwise OR
result = a XOR b     ' Bitwise exclusive OR
result = NOT a       ' Bitwise complement
result = a EQV b     ' Bitwise equivalence
result = a IMP b     ' Bitwise implication
```

### Truth Tables

| A | B | AND | OR | XOR | EQV | IMP |
|---|---|-----|-----|-----|-----|-----|
| 0 | 0 | 0 | 0 | 0 | 1 | 1 |
| 0 | 1 | 0 | 1 | 1 | 0 | 1 |
| 1 | 0 | 0 | 1 | 1 | 0 | 0 |
| 1 | 1 | 1 | 1 | 0 | 1 | 1 |

### C Code Generation

```c
// Generated for: result = a AND b
result = a & b;

// Generated for: result = a OR b
result = a | b;

// Generated for: result = a XOR b
result = a ^ b;

// Generated for: result = NOT a
result = ~a;
```

## Shift Operations

```basic
result = _SHL(value, bits)   ' Shift left
result = _SHR(value, bits)   ' Shift right (unsigned)
```

### Implementation

Defined as inline constexpr functions in `libqb.h`:

```c
template <typename T>
inline constexpr T func__shl(T a, unsigned int b) {
    return a << b;
}

template <typename T>
inline constexpr T func__shr(T a, unsigned int b) {
    return (typename std::make_unsigned<T>::type)a >> b;
}
```

### Behavior

| Operation | Description |
|-----------|-------------|
| `_SHL(x, n)` | Multiply by 2^n (bits shift left, zeros fill right) |
| `_SHR(x, n)` | Divide by 2^n (bits shift right, zeros fill left) |

## Rotate Operations

```basic
result = _ROL(value, bits)   ' Rotate left
result = _ROR(value, bits)   ' Rotate right
```

### Implementation

```c
template <typename T>
inline constexpr T func__rol(T a, unsigned int b) {
    constexpr auto bits = sizeof(T) * 8;
    b %= bits;
    if (!b) return a;
    return (a << b) | ((typename std::make_unsigned<T>::type)a >> (bits - b));
}

template <typename T>
inline constexpr T func__ror(T a, unsigned int b) {
    constexpr auto bits = sizeof(T) * 8;
    b %= bits;
    if (!b) return a;
    return ((typename std::make_unsigned<T>::type)a >> b) | (a << (bits - b));
}
```

### Behavior

| Operation | Description |
|-----------|-------------|
| `_ROL(x, n)` | Bits wrap around from left to right |
| `_ROR(x, n)` | Bits wrap around from right to left |

## Individual Bit Manipulation

```basic
result = _SETBIT(value, bit)      ' Set bit to 1
result = _RESETBIT(value, bit)    ' Set bit to 0
result = _TOGGLEBIT(value, bit)   ' Flip bit
result = _READBIT(value, bit)     ' Read bit (0 or 1)
```

### Implementation

```c
template <typename T>
inline constexpr T func__setbit(T a, unsigned int b) {
    return a | ((T)1 << b);
}

template <typename T>
inline constexpr T func__resetbit(T a, unsigned int b) {
    return a & ~((T)1 << b);
}

template <typename T>
inline constexpr T func__togglebit(T a, unsigned int b) {
    return a ^ ((T)1 << b);
}

template <typename T>
inline constexpr int32 func__readbit(T a, unsigned int b) {
    return (a >> b) & 1;
}
```

### Bit Numbering

Bits are numbered starting from 0 (least significant):

```
Bit:    7  6  5  4  3  2  1  0
Value: 128 64 32 16  8  4  2  1
```

## Type Considerations

All bitwise operations preserve the type of the operands:

| Type | Bits | Range |
|------|------|-------|
| `_BYTE` | 8 | 0-255 or -128 to 127 |
| `INTEGER` | 16 | -32768 to 32767 |
| `LONG` | 32 | -2B to 2B |
| `_INTEGER64` | 64 | Full 64-bit range |
| `_UNSIGNED` | varies | Unsigned variants |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/include/libqb.h` | Inline templates |
| `source/qb64pe.bas` | Operator parsing |

## Example Usage

```basic
' Flags with AND/OR
flags = flags OR FLAG_ENABLED      ' Set flag
flags = flags AND NOT FLAG_ENABLED ' Clear flag
IF flags AND FLAG_ENABLED THEN     ' Check flag

' Fast multiply/divide by powers of 2
x = _SHL(value, 3)   ' Multiply by 8
x = _SHR(value, 2)   ' Divide by 4

' Rotate for circular operations
rotated = _ROL(byte, 1)   ' Rotate left 1 bit

' Individual bits
value = _SETBIT(0, 3)     ' Set bit 3: result = 8
value = _TOGGLEBIT(value, 0)  ' Toggle bit 0
IF _READBIT(value, 3) THEN PRINT "Bit 3 is set"

' Extract byte from integer
lowByte = value AND &HFF
highByte = _SHR(value, 8) AND &HFF
```
