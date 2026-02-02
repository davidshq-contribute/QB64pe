# DATETIME.md

Documentation of the QB64pe date/time implementation.

## Overview

QB64pe provides cross-platform date and time functions using high-resolution monotonic clocks for timing and localtime for calendar functions.

## DATE$ Function

```basic
d$ = DATE$  ' Returns "MM-DD-YYYY"
```

Uses `localtime()` for timezone-aware conversion.

## TIME$ Function

```basic
t$ = TIME$  ' Returns "HH:MM:SS"
```

24-hour format, local time.

## TIMER Function

```basic
t! = TIMER              ' Default 18.2 Hz
t# = TIMER(accuracy#)   ' Custom Hz
```

Returns seconds since midnight.

## _DELAY Function

```basic
_DELAY seconds#
```

- Maximum: ~2147 seconds
- Handles timer wraparound
- Checks events during long delays

## _LIMIT Function

```basic
_LIMIT fps#
```

Frame rate limiter:
- Maintains accurate timing
- Recovers from overshoots
- Checks events during waits

## GetTicks() Implementation

### Linux
```c
clock_gettime(CLOCK_MONOTONIC, &tp);
```

### Windows
```c
QueryPerformanceCounter(&count);
```

### macOS
```c
mach_absolute_time();
```

All return milliseconds from program start.

## ON TIMER Events

```basic
ON TIMER(seconds#) GOSUB label
TIMER ON/OFF/STOP
```

Timer thread checks every 1ms, sets `qbevent` flag when triggered.

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/src/datetime.cpp` | Core functions |
| `internal/c/common.h` | Timer structures |
| `internal/c/qbx.cpp` | Timer thread |

## Example Usage

```basic
' Precise timing
start# = TIMER(0.001)
' ... operation ...
elapsed# = TIMER(0.001) - start#

' Frame rate control
DO
    ' Game logic
    _LIMIT 60
LOOP

' Timer events
ON TIMER(1) GOSUB UpdateClock
TIMER ON
```
