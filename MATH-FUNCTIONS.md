# MATH-FUNCTIONS.md

Documentation of the QB64pe mathematical functions implementation.

## Overview

QB64pe provides comprehensive mathematical functions that map to C library functions, with additional QB64-specific extensions for angles, randomization, and convenience operations.

## Trigonometric Functions

```basic
y = SIN(radians)
y = COS(radians)
y = TAN(radians)
y = ATN(x)           ' Arctangent
y = _ASIN(x)         ' Arcsine
y = _ACOS(x)         ' Arccosine
y = _ATAN2(y, x)     ' Two-argument arctangent
```

### C Implementation

```c
double func_sin(double d) { return std::sin(d); }
double func_cos(double d) { return std::cos(d); }
double func_tan(double d) { return std::tan(d); }
double func_atn(double d) { return std::atan(d); }
double func_asin(double d) { return std::asin(d); }
double func_acos(double d) { return std::acos(d); }
double func_atan2(double y, double x) { return std::atan2(y, x); }
```

## Angle Conversion

```basic
radians = _D2R(degrees)   ' Degrees to radians
degrees = _R2D(radians)   ' Radians to degrees
radians = _D2G(degrees)   ' Degrees to gradians
degrees = _G2D(gradians)  ' Gradians to degrees
radians = _G2R(gradians)  ' Gradians to radians
gradians = _R2G(radians)  ' Radians to gradians
```

### Implementation

```c
double func_deg2rad(double d) { return d * 0.01745329251994329577; }
double func_rad2deg(double r) { return r * 57.29577951308232087680; }
double func_deg2grad(double d) { return d * 1.11111111111111111111; }
double func_grad2deg(double g) { return g * 0.9; }
double func_grad2rad(double g) { return g * 0.01570796326794896619; }
double func_rad2grad(double r) { return r * 63.66197723675813430755; }
```

## Exponential and Logarithmic

```basic
y = EXP(x)           ' e^x
y = LOG(x)           ' Natural logarithm
y = SQR(x)           ' Square root
y = _HYPOT(x, y)     ' Hypotenuse (sqrt(x^2 + y^2))
```

### C Implementation

```c
double func_exp(double d) { return std::exp(d); }
double func_log(double d) { return std::log(d); }
double func_sqr(double d) { return std::sqrt(d); }
double func_hypot(double x, double y) { return std::hypot(x, y); }
```

## Rounding Functions

```basic
y = INT(x)           ' Floor (toward negative infinity)
y = FIX(x)           ' Truncate (toward zero)
y = CINT(x)          ' Round to INTEGER (-32768 to 32767)
y = CLNG(x)          ' Round to LONG
y = _ROUND(x)        ' Round to nearest (banker's rounding)
```

### QBR Rounding

The `qbr()` function implements classic QBasic rounding:

```c
int64 qbr(double f) {
    if (f < 0) {
        return (int64)(f - 0.5);
    } else {
        return (int64)(f + 0.5);
    }
}
```

## Random Number Generation

```basic
x = RND              ' 0 to 1 (exclusive)
x = RND(seed)        ' With seed control
RANDOMIZE [seed]     ' Initialize generator
RANDOMIZE USING seed ' Repeatable sequence
```

### RND Behavior

| Argument | Behavior |
|----------|----------|
| RND | Next random number |
| RND(0) | Repeat last number |
| RND(n > 0) | Next random number |
| RND(n < 0) | Seed with n, return first |

### Implementation

```c
double func_rnd(double n, int32 passed) {
    if (!passed) n = 1.0;
    if (n == 0) return last_rnd;
    if (n < 0) rnd_seed = (uint32)(n * -1);
    last_rnd = (double)rnd_sequence() / 16777216.0;
    return last_rnd;
}
```

## Sign and Absolute Value

```basic
y = ABS(x)           ' Absolute value
y = SGN(x)           ' Sign (-1, 0, or 1)
```

### Implementation

```c
double func_abs(double d) { return std::fabs(d); }
int32 func_sgn(double d) {
    if (d > 0) return 1;
    if (d < 0) return -1;
    return 0;
}
```

## Mathematical Constants

```basic
pi = _PI             ' Pi (3.14159...)
pi2 = _PI(2)         ' 2 * Pi
```

### Implementation

```c
double func_pi(double m, int32 passed) {
    if (passed) return 3.14159265358979323846 * m;
    return 3.14159265358979323846;
}
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/src/math.cpp` | Core math functions |
| `internal/c/libqb/src/rounding.cpp` | Rounding functions |
| `internal/c/libqb/src/rnd.cpp` | Random number generator |
| `source/qb64pe.bas` | Function parsing |

## Example Usage

```basic
' Trigonometry
angle = _D2R(45)        ' Convert 45 degrees to radians
x = COS(angle)
y = SIN(angle)

' Distance calculation
distance = _HYPOT(x2 - x1, y2 - y1)

' Random numbers
RANDOMIZE TIMER
roll = INT(RND * 6) + 1  ' Dice roll 1-6

' Rounding
rounded = _ROUND(3.5)    ' Returns 4
truncated = FIX(-3.7)    ' Returns -3
floored = INT(-3.7)      ' Returns -4
```
