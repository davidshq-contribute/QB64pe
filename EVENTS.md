# EVENTS.md

Documentation of the QB64pe event trapping system.

## Overview

QB64pe supports event-driven programming through ON...GOSUB handlers for timers, keyboard, and joystick input.

## ON TIMER Events

```basic
ON TIMER(seconds#) GOSUB label
ON TIMER(seconds#, timerID%) GOSUB label
TIMER ON/OFF/STOP
_FREETIMER timerID%
```

### States

| State | Value | Meaning |
|-------|-------|---------|
| OFF | 0 | Events ignored |
| ON | 1 | Events processed |
| STOP | 2 | Events queued |

Timer thread checks every 1ms in background.

## ON KEY Events

```basic
ON KEY(n%) GOSUB label
KEY(n%) ON/OFF/STOP
```

### Key Codes

| n | Key |
|---|-----|
| 1-10 | F1-F10 |
| 11-14 | Arrow keys |
| 30-31 | F11, F12 |

## ON STRIG Events

```basic
ON STRIG(button%) GOSUB label
ON STRIG(button%, controller%) GOSUB label
STRIG(button%) ON/OFF/STOP
```

Button index: `(controller - 1) * 256 + (button - 1)`

## Event Checking

`evnt()` function called throughout generated code:
1. Check for pending events
2. Call `events()` to process
3. Execute handlers via included files

## Event Processing

```c
void events() {
    // Check STRIG events
    // Check KEY events
    // Check TIMER events
    // Re-check after each handler
}
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | Event parsing |
| `internal/c/common.h` | Event structures |
| `internal/c/qbx.cpp` | Event processing |

## Example Usage

### Timer Events

```basic
ON TIMER(1) GOSUB UpdateClock
TIMER ON

DO
    _LIMIT 60
LOOP UNTIL INKEY$ = CHR$(27)

UpdateClock:
    PRINT TIME$
    RETURN
```

### Keyboard Events

```basic
ON KEY(1) GOSUB F1Handler
KEY(1) ON

DO
    _LIMIT 60
LOOP

F1Handler:
    PRINT "F1 pressed!"
    RETURN
```

### TIMER STOP Example

```basic
ON TIMER(1) GOSUB Counter
TIMER ON

TIMER STOP  ' Pause, queue events
SLEEP 3

TIMER ON    ' Resume, process queued

Counter:
    STATIC count
    count = count + 1
    PRINT count
    RETURN
```
