# GRAPHICS.md

Documentation of the QB64pe graphics system implementation.

## Overview

QB64pe provides a comprehensive graphics system supporting legacy QBasic screen modes, modern 32-bit graphics, hardware-accelerated rendering via OpenGL, and image manipulation.

## Screen Modes

### Legacy Modes (SCREEN Statement)

| Mode | Resolution | Colors | Type |
|------|------------|--------|------|
| 0 | Text mode | 16 | Text |
| 1 | 320×200 | 4 | CGA |
| 2 | 640×200 | 2 | CGA |
| 7 | 320×200 | 16 | EGA |
| 8 | 640×200 | 16 | EGA |
| 9 | 640×350 | 16 | EGA |
| 10 | 640×350 | 4 | EGA mono |
| 11 | 640×480 | 2 | VGA |
| 12 | 640×480 | 16 | VGA |
| 13 | 320×200 | 256 | VGA |

### Modern Modes (_NEWIMAGE)

```basic
handle& = _NEWIMAGE(width&, height&, colorDepth%)
```

| Depth | Description |
|-------|-------------|
| 0 | Text mode (variable size) |
| 1 | 2-color (1-bit) |
| 2 | 4-color (2-bit) |
| 7-13 | Legacy compatibility |
| 256 | 8-bit indexed color |
| 32 | 32-bit ARGB |

## Image Handle System

### Handle Types

| Range | Type |
|-------|------|
| > 0 | Display pages |
| < 0 | Off-screen images |
| 0 | Current display |
| -1 | Invalid handle |

### img_struct (internal/c/libqb.cpp)

```c
struct img_struct {
    void *lock_offset;           // Memory lock
    int64 lock_id;               // Lock identifier
    uint8 valid;                 // Handle is valid
    uint8 text;                  // Text mode flag
    uint8 graphical_pages;       // Number of pages
    uint8 bytes_per_pixel;       // 1, 2, or 4
    uint8 bits_per_pixel;        // Color depth
    int32 width, height;         // Dimensions
    int32 mask;                  // Transparency mask
    uint8 *offset;               // Pixel data pointer
    int32 flags;                 // Image flags
    int32 compatible_mode;       // SCREEN mode compatibility
    uint32 *pal;                 // Palette (256 entries)
    int32 transparent_color;     // Transparent index
    uint8 alpha_disabled;        // Alpha blending off
    uint8 depthbuffer_mode;      // 3D depth buffer
    uint8 draw_ta, draw_tb;      // Text attributes
    int32 draw_width;            // Current width
    uint32 draw_color;           // Current color
    // ... more fields
};
```

### Image Functions

| Function | Purpose |
|----------|---------|
| `_NEWIMAGE(w, h, depth)` | Create new image |
| `_FREEIMAGE(handle)` | Release image memory |
| `_COPYIMAGE(handle)` | Duplicate image |
| `_SOURCE(handle)` | Set read source |
| `_DEST(handle)` | Set write destination |
| `_DISPLAY` | Copy work page to visible |

## Graphics Primitives

### Point Operations

```c
void sub_pset(float x, float y, uint32 col, int32 passed);
uint32 func_point(float x, float y);
```

### Line Drawing

```c
void sub_line(float x1, float y1, float x2, float y2,
              uint32 col, int32 bf, uint32 style, int32 passed);
```

**bf parameter:**
- 0 = Line only
- 1 = Box outline (B)
- 2 = Filled box (BF)

**style:** 16-bit line pattern

### Circle/Ellipse

```c
void sub_circle(float x, float y, float radius,
                uint32 col, float start, float end,
                float aspect, int32 passed);
```

### Paint Fill

```c
void sub_paint(float x, float y, uint32 fillcol,
               uint32 bordercol, qbs *pattern, int32 passed);
```

## Color System

### Color Functions

| Function | Purpose |
|----------|---------|
| `_RGB(r, g, b)` | Create 32-bit color |
| `_RGBA(r, g, b, a)` | Create with alpha |
| `_RGB32(r, g, b)` | Explicit 32-bit |
| `_RED(c)` | Extract red component |
| `_GREEN(c)` | Extract green component |
| `_BLUE(c)` | Extract blue component |
| `_ALPHA(c)` | Extract alpha component |

### Color Representation

**32-bit ARGB format:**
```
Bits: AAAA AAAA RRRR RRRR GGGG GGGG BBBB BBBB
      31-24     23-16     15-8      7-0
```

### Palette System

```c
void sub_palette(int32 attr, int32 col, int32 passed);
uint32 *pal;  // 256-entry palette array
```

**Legacy palette mapping:**
- Attributes 0-15 for 16-color modes
- Attributes 0-255 for 256-color mode

## Image Loading

### _LOADIMAGE

```c
int32 func__loadimage(qbs *filename, int32 bpp, qbs *requirements, int32 passed);
```

**Supported formats:** PNG, JPG, BMP, GIF, TGA, PSD, HDR, PIC, PNM

**Implementation:** Uses stb_image library (internal/c/parts/video/image/)

### _SAVEIMAGE

```c
int32 func__saveimage(qbs *filename, int32 handle, qbs *requirements, int32 passed);
```

**Supported formats:** PNG, JPG, BMP

## Block Operations

### GET/PUT Graphics

```c
void sub_get(float x1, float y1, float x2, float y2, void *element, int32 passed);
void sub_put(float x, float y, void *element, int32 action, int32 passed);
```

**PUT actions:**
| Value | Mode | Description |
|-------|------|-------------|
| 0 | XOR | XOR with background |
| 1 | PSET | Overwrite |
| 2 | PRESET | Inverted overwrite |
| 3 | AND | AND with background |
| 4 | OR | OR with background |

### _PUTIMAGE

```c
void sub__putimage(float dx1, float dy1, float dx2, float dy2,
                   int32 src, int32 dst,
                   float sx1, float sy1, float sx2, float sy2,
                   int32 passed);
```

**Features:**
- Scaling and stretching
- Clipping to destination
- Alpha blending
- Smooth vs. nearest-neighbor interpolation

## Hardware vs Software Rendering

### Display Modes

```c
int32 display_page_hardware;     // Hardware rendering active
struct hardware_surface *hw_img; // Hardware image array
```

### Hardware Images

```c
struct hardware_img_struct {
    int32 w, h;                  // Dimensions
    int32 type;                  // 1=hardware, 2=software
    uint32 *software_pixel_buffer; // Staging buffer
    int32 texture_handle;        // OpenGL texture ID
    int32 alpha_disabled;        // Blending control
    int32 depthbuffer_mode;      // Depth testing
};
```

### _DISPLAYORDER

```c
void sub__displayorder(int32 o1, int32 o2, int32 o3, int32 o4, int32 passed);
```

Controls rendering order of:
- `_HARDWARE` - Hardware layer
- `_HARDWARE1` - Hardware layer 1
- `_SOFTWARE` - Software layer

## OpenGL Integration

### GL Constants

```basic
CONST _GL = -5                  ' OpenGL mode indicator
```

### GL Sub Entry Point

```basic
SUB _GL
    ' OpenGL rendering callback
    ' Called each frame when enabled
END SUB
```

### OpenGL State

```c
int32 sub_gl_called;            // _GL sub exists
int32 gl_render_method;         // Rendering approach
```

## Coordinate Systems

### WINDOW Statement

```c
void sub_window(float x1, float y1, float x2, float y2, int32 passed);
```

Creates logical coordinate system with optional Y-axis flip.

### VIEW Statement

```c
void sub_view(int32 x1, int32 y1, int32 x2, int32 y2,
              uint32 fillcol, uint32 bordercol, int32 passed);
```

Creates clipping viewport within screen.

### Coordinate Mapping

```c
float view_x1, view_y1, view_x2, view_y2;  // Physical viewport
float window_x1, window_y1;                 // Logical origin
float window_x_scale, window_y_scale;       // Scale factors
```

## Screen Operations

### _DISPLAY/_AUTODISPLAY

```c
void sub__display();                        // Manual refresh
void sub__autodisplay(int32 onoff);         // Auto refresh toggle
```

### _SCREENHIDE/_SCREENSHOW

```c
void sub__screenhide();                     // Hide window
void sub__screenshow();                     // Show window
```

### _SCREENIMAGE

```c
int32 func__screenimage(int32 x, int32 y, int32 w, int32 h, int32 passed);
```

Captures screen region to new image handle.

## Double Buffering

### PCOPY

```c
void sub_pcopy(int32 src, int32 dst);
```

Copies between display pages.

### Page Management

```c
int32 display_page;             // Visible page
int32 write_page;               // Active write page
```

## Text in Graphics

### _PRINTSTRING

```c
void sub__printstring(float x, float y, qbs *text, int32 dst, int32 passed);
```

Renders text at pixel coordinates.

### _PRINTWIDTH/_FONTHEIGHT

```c
int32 func__printwidth(qbs *text, int32 dst, int32 passed);
int32 func__fontheight(int32 handle, int32 passed);
```

## Transparency

### _CLEARCOLOR

```c
void sub__clearcolor(uint32 col, int32 dst, int32 passed);
```

Sets transparent color for indexed images.

### _SETALPHA

```c
void sub__setalpha(int32 alpha, uint32 c1, uint32 c2, int32 dst, int32 passed);
```

Modifies alpha channel for color range.

### _BLEND/_DONTBLEND

```c
void sub__blend(int32 handle, int32 passed);
void sub__dontblend(int32 handle, int32 passed);
```

Controls alpha blending behavior.

## Error Handling

| Error | Cause |
|-------|-------|
| 5 | Illegal function call |
| 258 | Invalid handle |
| 259 | Hardware image not supported |
| 260 | Invalid coordinate |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | Core graphics functions |
| `internal/c/libqb/src/gui.cpp` | Window/display management |
| `internal/c/parts/video/image/` | Image loading (stb_image) |
| `internal/c/parts/core/gl_header_for_parsing/` | OpenGL definitions |

## Example Usage

### Basic Drawing

```basic
SCREEN _NEWIMAGE(800, 600, 32)
LINE (100, 100)-(700, 500), _RGB(255, 0, 0), BF
CIRCLE (400, 300), 100, _RGB(0, 255, 0)
PSET (400, 300), _RGB(255, 255, 255)
_DISPLAY
```

### Image Manipulation

```basic
img& = _LOADIMAGE("picture.png", 32)
_PUTIMAGE (0, 0)-(799, 599), img&, 0
_FREEIMAGE img&
```

### Hardware Rendering

```basic
SCREEN _NEWIMAGE(800, 600, 32)
hw& = _COPYIMAGE(0, 33)  ' 33 = hardware
_PUTIMAGE , hw&, 0
_FREEIMAGE hw&
```
