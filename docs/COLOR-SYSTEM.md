# COLOR-SYSTEM.md

Documentation of the QB64pe color system.

## Overview

QB64pe provides comprehensive color support from legacy 16-color modes to 32-bit true color with alpha channel, HSB color space, and palette management.

## Color Creation Functions

### _RGB / _RGBA

```basic
color~& = _RGB(red%, green%, blue%[, imageHandle&])
color~& = _RGBA(red%, green%, blue%, alpha%[, imageHandle&])
```

- Creates color value appropriate for target image
- For 32-bit: Full ARGB value
- For indexed: Nearest palette match

### _RGB32 / _RGBA32

```basic
color~& = _RGB32(red%, green%, blue%)
color~& = _RGB32(red%, green%, blue%, alpha%)
color~& = _RGB32(intensity%)           ' Grayscale
color~& = _RGB32(intensity%, alpha%)   ' Grayscale with alpha
```

Fast inline functions for 32-bit colors only.

### HSB Color Functions

```basic
color~& = _HSB32(hue#, saturation#, brightness#)
color~& = _HSBA32(hue#, saturation#, brightness#, alpha#)
```

| Parameter | Range | Description |
|-----------|-------|-------------|
| hue | 0-360 | Color angle in degrees |
| saturation | 0-100 | Color intensity % |
| brightness | 0-100 | Lightness % |
| alpha | 0-100 | Opacity % |

## Color Extraction Functions

### Component Extraction

```basic
r% = _RED(color~&[, imageHandle&])
g% = _GREEN(color~&[, imageHandle&])
b% = _BLUE(color~&[, imageHandle&])
a% = _ALPHA(color~&[, imageHandle&])
```

For indexed modes, extracts from palette entry.

### Fast 32-bit Extraction

```basic
r% = _RED32(color~&)
g% = _GREEN32(color~&)
b% = _BLUE32(color~&)
a% = _ALPHA32(color~&)
```

Inline functions - fastest for 32-bit colors.

### HSB Extraction

```basic
h# = _HUE32(color~&)         ' 0-360
s# = _SATURATION32(color~&)  ' 0-100
b# = _BRIGHTNESS32(color~&)  ' 0-100
```

## Internal Color Format

### 32-bit ARGB Layout

```
Bit:  31-24   23-16   15-8    7-0
      Alpha   Red     Green   Blue
```

```c
color = (alpha << 24) | (red << 16) | (green << 8) | blue
```

### Helper Macros

```c
// Extraction
red   = (color >> 16) & 0xFF
green = (color >> 8) & 0xFF
blue  = color & 0xFF
alpha = color >> 24

// Creation
color = blue | (green << 8) | (red << 16) | (alpha << 24)
```

## COLOR Statement

```basic
COLOR [foreground&][, background&]
```

### Mode-Specific Behavior

| Mode | Foreground | Background |
|------|------------|------------|
| 0 (text) | 0-31 | 0-7 |
| 1 | 0-3 | 0-15 (palette) |
| 2, 11 | 0-1 | 0-1 |
| 7-9, 12 | 0-15 | 0-15 |
| 13, 256 | 0-255 | 0-255 |
| 32 | ARGB value | ARGB value |

### Console Colors

```basic
$CONSOLE
COLOR 14, 1   ' Yellow on blue (ANSI codes)
```

## Palette Management

### _PALETTECOLOR

```basic
' Read palette entry
color~& = _PALETTECOLOR(index%, imageHandle&)

' Write palette entry
_PALETTECOLOR index%, newColor~&[, imageHandle&]
```

### _COPYPALETTE

```basic
_COPYPALETTE [sourceHandle&][, destHandle&]
```

Copies all 256 palette entries between images.

### Default Palettes

**palette_256[256]**: Standard VGA palette
- Indices 0-15: Standard 16 colors
- Indices 16-231: Color cube
- Indices 232-255: Grayscale ramp

**palette_64[64]**: EGA palette for Mode 9

## Query Functions

### _DEFAULTCOLOR

```basic
fg~& = _DEFAULTCOLOR[(imageHandle&)]
```

Returns current foreground color.

### _BACKGROUNDCOLOR

```basic
bg~& = _BACKGROUNDCOLOR[(imageHandle&)]
```

Returns current background color.

### _CLEARCOLOR

```basic
_CLEARCOLOR color~&[, imageHandle&]
_CLEARCOLOR _NONE[, imageHandle&]
tc& = _CLEARCOLOR[(imageHandle&)]
```

Sets or gets transparent color for sprites.

## Color in Different Screen Modes

| Mode | Type | Colors | Palette |
|------|------|--------|---------|
| 0 | Text | 16 | Attribute-based |
| 1 | Graphics | 4 | 2 palettes |
| 2 | Graphics | 2 | Black/White |
| 7-9 | Graphics | 16 | Configurable |
| 10 | Graphics | 4 | Pseudocolor |
| 11-12 | Graphics | 2/16 | Fixed/Configurable |
| 13, 256 | Graphics | 256 | Full VGA |
| 32 | Graphics | 16.7M | True color |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | _RGB, _RGBA (20283-20545) |
| `internal/c/libqb.cpp` | COLOR statement (7439-7689) |
| `internal/c/libqb/src/graphics.cpp` | HSB functions (87-148) |
| `internal/c/libqb/include/image.h` | 32-bit helpers (54-148) |

## Example Usage

### Basic Colors

```basic
SCREEN 12
COLOR 14          ' Yellow foreground
LINE (0, 0)-(639, 479), 12  ' Red line
```

### 32-bit Colors

```basic
SCREEN _NEWIMAGE(800, 600, 32)
LINE (0, 0)-(799, 599), _RGB(255, 128, 0)     ' Orange
CIRCLE (400, 300), 100, _RGBA(0, 255, 0, 128) ' Semi-transparent green
```

### HSB Color Wheel

```basic
SCREEN _NEWIMAGE(400, 400, 32)
FOR angle = 0 TO 359
    x = 200 + COS(_D2R(angle)) * 150
    y = 200 + SIN(_D2R(angle)) * 150
    PSET (x, y), _HSB32(angle, 100, 100)
NEXT
```

### Palette Manipulation

```basic
SCREEN 13
' Create grayscale palette
FOR i = 0 TO 255
    _PALETTECOLOR i, _RGB(i, i, i)
NEXT
```

### Extract and Modify

```basic
original~& = _RGB32(100, 150, 200)
r = _RED32(original~&)
g = _GREEN32(original~&)
b = _BLUE32(original~&)
brighter~& = _RGB32(r + 50, g + 50, b + 50)
```

### Transparency

```basic
SCREEN _NEWIMAGE(640, 480, 32)
sprite& = _LOADIMAGE("sprite.png", 32)
_CLEARCOLOR _RGB32(255, 0, 255), sprite&  ' Magenta = transparent
_PUTIMAGE (100, 100), sprite&
```
