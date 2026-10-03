# CONSOLE.md

Documentation of the QB64pe console system.

## Overview

QB64pe supports console-mode applications through the `$CONSOLE` metacommand, enabling text-based I/O alongside or instead of graphical windows.

## $CONSOLE Metacommand

```basic
$CONSOLE           ' Enable console with graphics window
$CONSOLE:ONLY      ' Console only, no graphics window
```

### Preprocessor Variables

| Metacommand | `_CONSOLE_` Value |
|-------------|-------------------|
| (none) | undefined |
| `$CONSOLE` | 1 |
| `$CONSOLE:ONLY` | 2 |

### Compile-Time Effects

```c
// $CONSOLE generates:
int32 console = 1;

// $CONSOLE:ONLY generates:
int32 console = 1;
// Also links console-only-main-thread.o instead of GLUT
```

## _CONSOLE Statement

```basic
_CONSOLE ON        ' Show console window
_CONSOLE OFF       ' Hide console window
```

### Implementation

```c
void sub__console(int32 onoff) {
    if (!console) return;  // No console available
    if (onoff == 1) {
        // Turn on
        if (!console_active) {
#ifdef QB64_WINDOWS
            if (console_child) {
                ShowWindow(GetConsoleWindow(), SW_SHOWNOACTIVATE);
            }
#endif
            console_active = 1;
        }
    } else {
        // Turn off
        if (console_active) {
#ifdef QB64_WINDOWS
            if (console_child) {
                ShowWindow(GetConsoleWindow(), SW_HIDE);
            }
#endif
            console_active = 0;
        }
    }
}
```

## Console Functions

### _CONSOLETITLE

```basic
_CONSOLETITLE "My Application"
```

Sets the console window title (Windows only).

### _CONSOLEINPUT

```basic
DO WHILE _CONSOLEINPUT
    IF _CINP = 27 THEN EXIT DO  ' ESC key
LOOP
```

Reads console input events:
- Returns 1 for keyboard events
- Returns 2 for mouse events
- Returns 0 when no input

### _CINP

```basic
scancode% = _CINP              ' Get scancode (pos=down, neg=up)
scancode% = _CINP(0)           ' Alternate format for key-up
```

Returns keyboard scan codes from console input.

### _DEST _CONSOLE

```basic
_DEST _CONSOLE
PRINT "This goes to console"
_DEST 0
```

Redirects PRINT output to console window.

## Console State Variables

```c
int32 console = 1;              // Console enabled
int32 console_active = 1;       // Console visible
int32 console_child = 0;        // Console created by this process
int32 console_image = -1;       // Console image handle
```

## Platform Differences

### Windows

- Uses `AttachConsole(ATTACH_PARENT_PROCESS)` for command-line invocation
- `AllocConsole()` creates new console if needed
- Full console input support (keyboard, mouse)
- `ShowWindow()`/`HideWindow()` for visibility

### Linux/macOS

- Console is inherited from terminal
- `AllocConsole()`/`FreeConsole()` are no-ops
- Standard I/O works automatically
- No window management needed

## CONSOLE:ONLY Mode

When `$CONSOLE:ONLY` is used:

1. `DEPENDENCY_CONSOLE_ONLY` flag set
2. `console-only-main-thread.cpp` linked instead of GLUT
3. No graphics window created
4. GLUT functions become stubs
5. `libqb_is_glut_up()` returns false

### Stub Functions

```c
// In console-only-main-thread.cpp
void libqb_glut_set_window_title(const char *title) { }
void libqb_glut_position_window(int x, int y) { }
void libqb_start_main_thread(void) {
    MAIN_LOOP();  // Direct call, no GLUT thread
}
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | $CONSOLE parsing |
| `internal/c/libqb.cpp` | Console functions (lines 25699-25760) |
| `internal/c/libqb/src/console-only-main-thread.cpp` | Console-only stubs |
| `internal/c/common.h` | Platform detection |

## Example Usage

### Console Application

```basic
$CONSOLE:ONLY

PRINT "Enter your name: ";
INPUT name$
PRINT "Hello, "; name$; "!"

SYSTEM
```

### Mixed Mode

```basic
$CONSOLE

SCREEN _NEWIMAGE(640, 480, 32)

_DEST _CONSOLE
PRINT "Debug output to console"
_DEST 0

LINE (0, 0)-(639, 479), _RGB(255, 0, 0)
_DISPLAY

SLEEP
```

### Console Input

```basic
$CONSOLE:ONLY

PRINT "Press ESC to exit..."
DO
    IF _CONSOLEINPUT THEN
        k = _CINP
        IF k > 0 THEN
            PRINT "Key down:"; k
        ELSE
            PRINT "Key up:"; ABS(k)
        END IF
        IF ABS(k) = 1 THEN EXIT DO  ' ESC scancode
    END IF
    _LIMIT 60
LOOP
```

### Setting Console Title

```basic
$CONSOLE

_CONSOLETITLE "My QB64 Application - v1.0"

DO
    _LIMIT 60
LOOP UNTIL INKEY$ = CHR$(27)
```
