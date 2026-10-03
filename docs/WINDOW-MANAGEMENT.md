# WINDOW-MANAGEMENT.md

Documentation of the QB64pe window management system implementation.

## Overview

QB64pe manages application windows through GLUT (OpenGL Utility Toolkit) with additional platform-specific functionality for positioning, sizing, and decoration control.

## Window Creation

### Initialization Flow

1. GLUT initialization (`glutInit`)
2. Display mode setup (`glutInitDisplayMode`)
3. Window size/position (`glutInitWindowSize`, `glutInitWindowPosition`)
4. Window creation (`glutCreateWindow`)
5. Callback registration

### GLUT Setup (internal/c/libqb/src/gui.cpp)

```c
void window_init() {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(default_width, default_height);
    window_handle = glutCreateWindow(window_title);

    // Register callbacks
    glutDisplayFunc(display_callback);
    glutReshapeFunc(reshape_callback);
    glutKeyboardFunc(keyboard_callback);
    glutMouseFunc(mouse_callback);
    glutIdleFunc(idle_callback);
}
```

## Window Title

### _TITLE Statement

```c
void sub__title(qbs *title);
```

**Usage:**
```basic
_TITLE "My Application"
```

**Implementation:**
```c
void sub__title(qbs *title) {
    char *str = qbs_to_cstr(title);
    glutSetWindowTitle(str);
    free(str);
}
```

### _TITLE$ Function

```c
qbs *func__title();
```

Returns current window title.

## Window Icon

### _ICON Statement

```c
void sub__icon(int32 handle, int32 passed);
```

**Syntax:**
```basic
_ICON                     ' Use embedded icon
_ICON imageHandle&        ' Use image as icon
```

**Implementation:**
- Windows: Uses `SetClassLongPtr` with `GCLP_HICON`
- Linux: Uses X11 `_NET_WM_ICON` property
- macOS: Uses Dock icon API

### Platform-Specific

**Windows:**
```c
HICON icon = CreateIconFromResource(data, size, TRUE, 0x30000);
SetClassLongPtr(hwnd, GCLP_HICON, (LONG_PTR)icon);
```

**Linux/X11:**
```c
XChangeProperty(display, window,
    XInternAtom(display, "_NET_WM_ICON", False),
    XA_CARDINAL, 32, PropModeReplace,
    (unsigned char *)icon_data, icon_size);
```

## Window Position

### _SCREENX / _SCREENY

```c
int32 func__screenx();
int32 func__screeny();
```

Returns window position relative to screen origin.

### _SCREENMOVE

```c
void sub__screenmove(int32 x, int32 y, int32 passed);
```

**Syntax:**
```basic
_SCREENMOVE x%, y%
_SCREENMOVE _MIDDLE       ' Center on screen
```

**Special values:**
| Value | Constant | Meaning |
|-------|----------|---------|
| -1 | `_MIDDLE` | Center window |

**Implementation:**
```c
void sub__screenmove(int32 x, int32 y, int32 passed) {
    if (x == -1 && y == -1) {  // _MIDDLE
        int sw = glutGet(GLUT_SCREEN_WIDTH);
        int sh = glutGet(GLUT_SCREEN_HEIGHT);
        x = (sw - window_width) / 2;
        y = (sh - window_height) / 2;
    }
    glutPositionWindow(x, y);
}
```

## Window Visibility

### _SCREENHIDE / _SCREENSHOW

```c
void sub__screenhide();
void sub__screenshow();
```

**Implementation:**
```c
void sub__screenhide() {
    glutHideWindow();
    screen_hide = 1;
}

void sub__screenshow() {
    glutShowWindow();
    screen_hide = 0;
}
```

### $SCREENHIDE / $SCREENSHOW Metacommands

```basic
$SCREENHIDE              ' Start hidden
' ... initialization ...
$SCREENSHOW              ' Show when ready
```

## Fullscreen Mode

### _FULLSCREEN

```c
void sub__fullscreen(int32 mode, int32 passed);
```

**Syntax:**
```basic
_FULLSCREEN              ' Toggle
_FULLSCREEN _OFF         ' Windowed
_FULLSCREEN _STRETCH     ' Stretch to fill
_FULLSCREEN _SQUAREPIXELS ' Maintain aspect ratio
_FULLSCREEN _SMOOTH      ' Smoothed scaling
```

**Mode constants:**
| Constant | Value | Description |
|----------|-------|-------------|
| `_OFF` | 0 | Windowed mode |
| `_STRETCH` | 1 | Fill screen |
| `_SQUAREPIXELS` | 2 | Maintain ratio |
| `_SMOOTH` | 256 | Add smoothing |

### _FULLSCREEN Function

```c
int32 func__fullscreen();
```

Returns current fullscreen mode (0 = windowed).

### Implementation

```c
void sub__fullscreen(int32 mode, int32 passed) {
    if (mode == 0) {
        // Exit fullscreen
        glutReshapeWindow(saved_width, saved_height);
        glutPositionWindow(saved_x, saved_y);
        fullscreen_mode = 0;
    } else {
        // Enter fullscreen
        saved_width = window_width;
        saved_height = window_height;
        glutFullScreen();
        fullscreen_mode = mode;
    }
}
```

## Window Resizing

### $RESIZE Metacommand

```basic
$RESIZE:ON               ' Enable user resizing
$RESIZE:OFF              ' Disable resizing (default)
$RESIZE:SMOOTH           ' Resize with smoothing
$RESIZE:STRETCH          ' Resize with stretching
```

### _RESIZE Function

```c
int32 func__resize();
```

Returns -1 if window has been resized since last check.

### _RESIZEWIDTH / _RESIZEHEIGHT

```c
int32 func__resizewidth();
int32 func__resizeheight();
```

Returns new dimensions after resize event.

### Resize Callback

```c
void reshape_callback(int width, int height) {
    if (resize_enabled) {
        new_width = width;
        new_height = height;
        resize_pending = 1;
    }
    glViewport(0, 0, width, height);
}
```

### Resize Handling Example

```basic
$RESIZE:ON
SCREEN _NEWIMAGE(800, 600, 32)

DO
    IF _RESIZE THEN
        SCREEN _NEWIMAGE(_RESIZEWIDTH, _RESIZEHEIGHT, 32)
    END IF
    ' ... drawing code ...
    _DISPLAY
    _LIMIT 60
LOOP
```

## Window Dimensions

### _WIDTH / _HEIGHT

```c
int32 func__width(int32 handle, int32 passed);
int32 func__height(int32 handle, int32 passed);
```

Returns pixel dimensions of image/window.

### _DESKTOPWIDTH / _DESKTOPHEIGHT

```c
int32 func__desktopwidth();
int32 func__desktopheight();
```

Returns screen/desktop dimensions.

**Implementation:**
```c
int32 func__desktopwidth() {
    return glutGet(GLUT_SCREEN_WIDTH);
}
```

## Display Management

### _DISPLAY Statement

```c
void sub__display();
```

Manually updates display (double-buffering swap).

### _AUTODISPLAY

```c
void sub__autodisplay(int32 onoff);
```

Controls automatic display updates.

| Mode | Behavior |
|------|----------|
| ON | Auto-refresh after each operation |
| OFF | Manual refresh with _DISPLAY |

### _LIMIT

```c
void sub__limit(double fps);
```

Limits frame rate to conserve CPU.

## Focus and Activation

### _SCREENEXISTS

```c
int32 func__screenexists();
```

Returns -1 if window exists and is valid.

### _WINDOWHANDLE

```c
int64 func__windowhandle();
```

Returns platform-specific window handle:
- Windows: HWND
- Linux: X11 Window ID
- macOS: NSWindow pointer

### _WINDOWHASFOCUS

```c
int32 func__windowhasfocus();
```

Returns -1 if window has input focus.

## Console Window

### $CONSOLE Metacommand

```basic
$CONSOLE               ' Enable console
$CONSOLE:ONLY          ' Console only, no graphics
```

### _CONSOLE Statement

```c
void sub__console(int32 mode);
```

Shows or hides console window.

### _CONSOLETITLE

```c
void sub__consoletitle(qbs *title);
```

Sets console window title.

## Platform Integration

### Windows-Specific

```c
HWND hwnd = GetActiveWindow();
SetWindowPos(hwnd, HWND_TOP, x, y, w, h, SWP_SHOWWINDOW);
```

### X11-Specific (Linux)

```c
Display *display = XOpenDisplay(NULL);
Window window = glXGetCurrentDrawable();
XMoveWindow(display, window, x, y);
```

### macOS-Specific

Uses Cocoa/AppKit through GLUT abstraction.

## Event Loop

### Main Loop Structure

```c
void main_loop() {
    while (!quit_requested) {
        // Process events
        glutMainLoopEvent();

        // Update game state
        update_logic();

        // Render
        render_frame();

        // Swap buffers
        glutSwapBuffers();

        // Frame limiting
        apply_frame_limit();
    }
}
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/src/gui.cpp` | Window management |
| `internal/c/libqb.cpp` | High-level functions |
| `internal/c/parts/core/freeglut/` | GLUT library |

## Example Usage

### Basic Window Setup

```basic
_TITLE "My Application"
SCREEN _NEWIMAGE(800, 600, 32)
_SCREENMOVE _MIDDLE

DO
    CLS
    PRINT "Window at:"; _SCREENX; ","; _SCREENY
    _DISPLAY
    _LIMIT 60
LOOP UNTIL INKEY$ = CHR$(27)
```

### Fullscreen Toggle

```basic
SCREEN _NEWIMAGE(800, 600, 32)
fs% = 0

DO
    k$ = INKEY$
    IF k$ = "f" THEN
        IF fs% THEN
            _FULLSCREEN _OFF
            fs% = 0
        ELSE
            _FULLSCREEN _SQUAREPIXELS
            fs% = -1
        END IF
    END IF
    _DISPLAY
    _LIMIT 60
LOOP UNTIL k$ = CHR$(27)
```

### Resizable Window

```basic
$RESIZE:ON
SCREEN _NEWIMAGE(640, 480, 32)

DO
    IF _RESIZE THEN
        SCREEN _NEWIMAGE(_RESIZEWIDTH, _RESIZEHEIGHT, 32)
    END IF

    CLS
    PRINT "Size:"; _WIDTH; "x"; _HEIGHT
    _DISPLAY
    _LIMIT 60
LOOP
```

### Hidden Startup

```basic
$SCREENHIDE

' Perform initialization
DIM bigArray(1000000) AS LONG
' Load resources...

$SCREENSHOW
SCREEN _NEWIMAGE(800, 600, 32)
```
