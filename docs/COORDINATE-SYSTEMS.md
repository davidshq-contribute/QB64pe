# COORDINATE-SYSTEMS.md

Documentation of the QB64pe coordinate system.

## Overview

QB64pe supports both screen (physical pixel) coordinates and world (logical) coordinates through the WINDOW and VIEW statements, with PMAP for conversion between systems.

## Screen Coordinates

Default coordinate system:
- Origin: (0, 0) at top-left corner
- X increases rightward
- Y increases downward
- Units: Pixels
- Range: 0 to (width-1), 0 to (height-1)

```basic
SCREEN _NEWIMAGE(640, 480, 32)
PSET (0, 0)        ' Top-left corner
PSET (639, 479)    ' Bottom-right corner
```

## WINDOW Statement

Maps arbitrary world coordinates to screen pixels.

```basic
WINDOW (x1, y1)-(x2, y2)           ' Cartesian (Y increases up)
WINDOW SCREEN (x1, y1)-(x2, y2)    ' Screen-like (Y increases down)
WINDOW                              ' Reset to screen coordinates
```

### Transformation

```c
screen_x = (world_x - x1) * scaling_x + offset_x
screen_y = (world_y - y1) * scaling_y + offset_y
```

### Internal Fields

```c
float window_x1, window_y1;  // World bounds min
float window_x2, window_y2;  // World bounds max
float scaling_x, scaling_y;  // Scale factors
float scaling_offset_x, scaling_offset_y;  // Offsets
```

### Examples

```basic
' Mathematical coordinates (-10 to 10)
SCREEN 12
WINDOW (-10, -10)-(10, 10)
LINE (-10, 0)-(10, 0)    ' X axis
LINE (0, -10)-(0, 10)    ' Y axis
CIRCLE (0, 0), 5         ' Circle at origin

' Fractional coordinates
WINDOW (0, 0)-(1.0, 1.0)
PSET (0.5, 0.5)          ' Center point
```

## VIEW Statement

Defines a rectangular viewport (clipping region).

```basic
VIEW (x1, y1)-(x2, y2)[, fillcolor][, bordercolor]
VIEW SCREEN (x1, y1)-(x2, y2)
VIEW                    ' Reset to full screen
```

### Parameters

| Parameter | Description |
|-----------|-------------|
| x1, y1 | Top-left corner (pixels) |
| x2, y2 | Bottom-right corner (pixels) |
| fillcolor | Optional background color |
| bordercolor | Optional border color |

### Behavior

- All graphics clipped to viewport
- Coordinates relative to viewport origin
- VIEW SCREEN: Coordinates stay absolute

### Internal Fields

```c
int32 view_x1, view_y1;      // Viewport bounds
int32 view_x2, view_y2;
int32 view_offset_x;         // Drawing offset X
int32 view_offset_y;         // Drawing offset Y
```

### Examples

```basic
SCREEN 12
' Create viewport in center
VIEW (100, 50)-(540, 430), 1, 15
CLS                          ' Clears only viewport
LINE (0, 0)-(439, 379), 14   ' Relative to viewport

' Multiple viewports
VIEW (10, 10)-(200, 150): CLS: PRINT "View 1"
VIEW (210, 10)-(400, 150): CLS: PRINT "View 2"
```

## PMAP Function

Converts between coordinate systems.

```basic
result! = PMAP(coordinate!, function%)
```

### Functions

| Function | Conversion |
|----------|------------|
| 0 | World X → Screen X |
| 1 | World Y → Screen Y |
| 2 | Screen X → World X |
| 3 | Screen Y → World Y |

### Implementation

```c
switch (function) {
    case 0: return val * scaling_x + scaling_offset_x;
    case 1: return val * scaling_y + scaling_offset_y;
    case 2: return (val - scaling_offset_x) / scaling_x;
    case 3: return (val - scaling_offset_y) / scaling_y;
}
```

### Examples

```basic
WINDOW (-100, -100)-(100, 100)

' Convert world to screen
screenX = PMAP(50, 0)
screenY = PMAP(25, 1)

' Convert screen to world
worldX = PMAP(320, 2)
worldY = PMAP(240, 3)
```

## STEP Keyword

Specifies relative coordinates from current position.

```basic
LINE (100, 100)-STEP(50, 30)    ' To (150, 130)
PSET STEP(10, 10)               ' 10 pixels from last point
CIRCLE STEP(0, 0), 25           ' Circle at current position
```

### Supported Commands

- `LINE`
- `PSET` / `PRESET`
- `CIRCLE`
- `PAINT`
- `DRAW` (relative moves)

## Dimension Functions

```basic
w% = _WIDTH[(imageHandle&)]    ' Width in pixels
h% = _HEIGHT[(imageHandle&)]   ' Height in pixels
```

### Console Dimensions

```basic
cols% = _WIDTH               ' Text columns
rows% = _HEIGHT              ' Text rows (in text mode)
```

## Coordinate Pipeline

```
World Coordinates (WINDOW)
    ↓ Apply scaling
Screen Coordinates
    ↓ Apply VIEW offset
Viewport Coordinates
    ↓ Clipping
Final Pixel Output
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | WINDOW (11878-11989) |
| `internal/c/libqb.cpp` | VIEW (12030-12165) |
| `internal/c/libqb.cpp` | PMAP (17544-17567) |
| `internal/c/libqb/include/graphics.h` | Coordinate fields |

## Example Usage

### Graph Plotting

```basic
SCREEN 12
WINDOW (-3.14159, -1.5)-(3.14159, 1.5)

' Draw axes
LINE (-3.14159, 0)-(3.14159, 0), 7
LINE (0, -1.5)-(0, 1.5), 7

' Plot sine wave
FOR x = -3.14159 TO 3.14159 STEP 0.01
    PSET (x, SIN(x)), 14
NEXT
```

### Split Screen

```basic
SCREEN _NEWIMAGE(640, 480, 32)

' Top viewport - world view
VIEW (0, 0)-(639, 239)
WINDOW (-100, -100)-(100, 100)
CIRCLE (0, 0), 50, _RGB(255, 0, 0)

' Bottom viewport - zoomed
VIEW (0, 240)-(639, 479)
WINDOW (-25, -25)-(25, 25)
CIRCLE (0, 0), 50, _RGB(0, 255, 0)
```

### Coordinate Conversion

```basic
SCREEN 12
WINDOW (0, 0)-(100, 100)

' Click handling with coordinate conversion
DO
    DO WHILE _MOUSEINPUT: LOOP
    IF _MOUSEBUTTON(1) THEN
        screenX = _MOUSEX
        screenY = _MOUSEY
        worldX = PMAP(screenX, 2)
        worldY = PMAP(screenY, 3)
        PRINT "World:"; worldX; worldY
    END IF
    _LIMIT 60
LOOP
```

### Relative Drawing

```basic
SCREEN 12
PSET (320, 240)           ' Start point
LINE -STEP(100, 0), 14    ' Right 100
LINE -STEP(0, 50), 14     ' Down 50
LINE -STEP(-100, 0), 14   ' Left 100
LINE -STEP(0, -50), 14    ' Up 50 (back to start)
```
