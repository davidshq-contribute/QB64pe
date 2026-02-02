# INPUT-DEVICES.md

Documentation of the QB64pe input device system implementation.

## Overview

QB64pe provides comprehensive input handling for keyboard, mouse, and game controllers through event queues and state tracking.

## Keyboard Input

### Event Queue System

```c
struct keyboard_event {
    int32 code;                  // Key code
    int32 state;                 // Press/release
    int64 timestamp;             // Event time
};

keyboard_event keyboard_queue[256];
int32 keyboard_queue_head;
int32 keyboard_queue_tail;
```

### _KEYHIT Function

```c
int32 func__keyhit();
```

**Return values:**
- Positive: Key pressed (Unicode or extended code)
- Negative: Key released (negated code)
- 0: No key event

**Extended codes (> 256):**
| Code | Key |
|------|-----|
| 18432 | Up Arrow |
| 20480 | Down Arrow |
| 19200 | Left Arrow |
| 19712 | Right Arrow |
| 15104 | F1 |
| 15360 | F2 |
| ... | ... |

### _KEYDOWN Function

```c
int32 func__keydown(int32 code);
```

Returns -1 if key currently pressed, 0 otherwise.

### _KEYCLEAR

```c
void sub__keyclear(int32 buffer, int32 passed);
```

Clears keyboard buffer (0=all, 1=hardware, 2=INKEY$).

### INKEY$ Function

```c
qbs *func_inkey();
```

Returns 1 or 2 character string:
- 1 char: ASCII character
- 2 char: CHR$(0) + extended code

### _KEYBOARDHIDE / _KEYBOARDSHOW

```c
void sub__keyboardhide();
void sub__keyboardshow();
```

Mobile virtual keyboard control.

## Mouse Input

### Position Functions

```c
float func__mousex();           // X position in current coordinate system
float func__mousey();           // Y position
int32 func__mousemovementx();   // Delta X (raw movement)
int32 func__mousemovementy();   // Delta Y
```

### Button Functions

```c
int32 func__mousebutton(int32 button);
```

| Button | Description |
|--------|-------------|
| 1 | Left button |
| 2 | Right button |
| 3 | Middle button |
| 4+ | Extended buttons |

Returns -1 if pressed, 0 if not.

### Wheel Function

```c
int32 func__mousewheel();
```

Returns scroll direction: -1 (up), 0 (none), 1 (down).

### Input Function

```c
int32 func__mouseinput();
```

Advances to next mouse event in queue. Returns -1 if event available.

### Mouse State Structure

```c
struct mouse_state {
    float x, y;                  // Position
    int32 buttons[16];           // Button states
    int32 wheel;                 // Wheel accumulator
    int32 movementx, movementy;  // Raw movement
};

mouse_state mouse_queue[1024];
int32 mouse_queue_head;
int32 mouse_queue_tail;
```

### _MOUSEMOVE

```c
void sub__mousemove(float x, float y);
```

Repositions mouse cursor (requires window focus).

### _MOUSESHOW / _MOUSEHIDE

```c
void sub__mouseshow(qbs *shape, int32 passed);
void sub__mousehide();
```

Controls cursor visibility. Shape parameter sets cursor type.

## Game Controller Input

### Device System

```c
struct device_struct {
    char name[256];              // Device name
    int32 connected;             // Connection status
    int32 buttons;               // Button count
    int32 axes;                  // Axis count
    float *axis_value;           // Axis values array
    int32 *button_state;         // Button states array
    int32 queue_head, queue_tail; // Event queue
};

device_struct devices[16];
int32 device_count;
```

### _DEVICES Function

```c
int32 func__devices();
```

Returns count of input devices. Initializes device subsystem.

### _DEVICE$ Function

```c
qbs *func__device(int32 index);
```

Returns device description string:
```
[KEYBOARD][MOUSE][CONTROLLER]
```

### _DEVICEINPUT Function

```c
int32 func__deviceinput(int32 index, int32 passed);
```

Advances to next event for device. Returns device number or 0.

### _LASTBUTTON / _LASTAXIS

```c
int32 func__lastbutton(int32 device);
int32 func__lastaxis(int32 device);
int32 func__lastwheel(int32 device);
```

Returns count of buttons/axes/wheels for device.

### _BUTTON / _BUTTONCHANGE

```c
int32 func__button(int32 index, int32 passed);
int32 func__buttonchange(int32 index, int32 passed);
```

- `_BUTTON`: Current state (-1 pressed, 0 released)
- `_BUTTONCHANGE`: Change since last check (-1, 0, or 1)

### _AXIS Function

```c
float func__axis(int32 index, int32 passed);
```

Returns axis value (-1.0 to 1.0).

### _WHEEL Function

```c
float func__wheel(int32 index, int32 passed);
```

Returns wheel delta since last check.

## Light Pen (Legacy)

### PEN Function

```c
int32 func_pen(int32 option);
```

| Option | Returns |
|--------|---------|
| 0 | Light pen pressed |
| 1 | X coordinate when pressed |
| 2 | Y coordinate when pressed |
| 3 | Current X position |
| 4 | Current Y position |

**Note:** Implemented using mouse for compatibility.

## Joystick (Legacy)

### STICK Function

```c
int32 func_stick(int32 axis, int32 passed);
```

| Axis | Returns |
|------|---------|
| 0 | Joystick 1 X (0-254) |
| 1 | Joystick 1 Y |
| 2 | Joystick 2 X |
| 3 | Joystick 2 Y |

### STRIG Function

```c
int32 func_strig(int32 button, int32 passed);
```

| Button | Returns |
|--------|---------|
| 0 | Button 1 pressed since last check |
| 1 | Button 1 current state |
| 2 | Button 2 pressed since last check |
| 3 | Button 2 current state |

## Event Trapping

### ON KEY(n) GOSUB

```basic
ON KEY(n) GOSUB label
KEY(n) ON
KEY(n) OFF
KEY(n) STOP
```

### ON STRIG(n) GOSUB

```basic
ON STRIG(n) GOSUB label
STRIG(n) ON
```

### Event Processing

```c
void check_events() {
    // Called each iteration of main loop
    if (key_event_pending && key_trap_enabled) {
        execute_key_handler();
    }
    // Similar for STRIG, TIMER, etc.
}
```

## Platform Implementation

### GLUT Input Callbacks

```c
void keyboard_func(unsigned char key, int x, int y);
void keyboard_up_func(unsigned char key, int x, int y);
void special_func(int key, int x, int y);
void special_up_func(int key, int x, int y);
void mouse_func(int button, int state, int x, int y);
void motion_func(int x, int y);
void passive_motion_func(int x, int y);
```

### SDL Alternative (if enabled)

```c
void process_sdl_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEWHEEL:
            case SDL_CONTROLLERAXISMOTION:
            case SDL_CONTROLLERBUTTONDOWN:
            // ...
        }
    }
}
```

## Key Code Tables

### ASCII Keys

Standard ASCII codes 0-127.

### Extended Codes

| Range | Description |
|-------|-------------|
| 15104-15872 | F1-F12 |
| 18176-18432 | Insert/Delete/Home/End/PgUp/PgDn |
| 18432 | Up Arrow |
| 20480 | Down Arrow |
| 19200 | Left Arrow |
| 19712 | Right Arrow |

### Modifier Detection

```c
#define KEY_LSHIFT 100000
#define KEY_RSHIFT 100001
#define KEY_LCTRL  100002
#define KEY_RCTRL  100003
#define KEY_LALT   100004
#define KEY_RALT   100005
```

## Error Handling

| Error | Cause |
|-------|-------|
| 5 | Illegal function call |
| 52 | Bad file name or number (device) |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | Input functions |
| `internal/c/libqb/src/gui.cpp` | Event callbacks |
| `internal/c/parts/input/` | Device handling |

## Example Usage

### Keyboard Input

```basic
DO
    k& = _KEYHIT
    IF k& <> 0 THEN
        IF k& > 0 THEN
            PRINT "Key pressed:"; k&
        ELSE
            PRINT "Key released:"; -k&
        END IF
    END IF
    _LIMIT 60
LOOP UNTIL k& = 27  ' ESC
```

### Continuous Key State

```basic
DO
    IF _KEYDOWN(32) THEN PRINT "Space held"
    IF _KEYDOWN(18432) THEN y = y - 1  ' Up arrow
    IF _KEYDOWN(20480) THEN y = y + 1  ' Down arrow
    _LIMIT 60
LOOP
```

### Mouse Input

```basic
DO WHILE _MOUSEINPUT
    PRINT "Mouse:"; _MOUSEX; _MOUSEY
    IF _MOUSEBUTTON(1) THEN PRINT "Left click"
    w% = _MOUSEWHEEL
    IF w% THEN PRINT "Wheel:"; w%
LOOP
```

### Game Controller

```basic
PRINT "Devices:"; _DEVICES
FOR i = 1 TO _DEVICES
    PRINT _DEVICE$(i)
NEXT

DO
    d% = _DEVICEINPUT(1)  ' Controller 1
    IF d% THEN
        x! = _AXIS(1)     ' Left stick X
        y! = _AXIS(2)     ' Left stick Y
        fire% = _BUTTON(1)
    END IF
    _LIMIT 60
LOOP
```
