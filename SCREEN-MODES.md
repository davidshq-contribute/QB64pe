# SCREEN-MODES.md

Documentation of the QB64pe screen mode system.

## Overview

QB64pe supports legacy QBasic screen modes (0-13) plus custom modes via _NEWIMAGE, providing text mode, indexed color graphics, and 32-bit true color support.

## SCREEN Statement

```basic
SCREEN mode%                    ' Set screen mode
SCREEN mode%, , active, visual  ' With page selection
SCREEN imageHandle&             ' Use image as screen
```

## Legacy Screen Modes

### Mode 0 - Text Mode

```basic
SCREEN 0
WIDTH 80, 25    ' 80 columns, 25 rows
```

| Setting | Value |
|---------|-------|
| Type | Text only |
| Dimensions | 40x25, 40x43, 40x50, 80x25, 80x43, 80x50 |
| Colors | 16 foreground, 8 background |
| Character Box | 8x8, 8x14, 8x16, 9x14, 9x16 |
| Pages | Up to 8 |

### Mode 1 - CGA 4-Color

```basic
SCREEN 1
```

| Setting | Value |
|---------|-------|
| Resolution | 320x200 |
| Colors | 4 (from 16-color palette) |
| Bits Per Pixel | 2 |
| Text Grid | 40x25 |
| Palette | Cyan/Purple/White or Green/Red/Brown |

### Mode 2 - CGA Monochrome

```basic
SCREEN 2
```

| Setting | Value |
|---------|-------|
| Resolution | 640x200 |
| Colors | 2 (black/white) |
| Bits Per Pixel | 1 |
| Text Grid | 80x25 |

### Modes 7-8 - EGA 16-Color

```basic
SCREEN 7    ' 320x200
SCREEN 8    ' 640x200
```

| Mode | Resolution | Text Grid |
|------|------------|-----------|
| 7 | 320x200 | 40x25 |
| 8 | 640x200 | 80x25 |

Both: 16 colors, 4 bits per pixel.

### Mode 9 - EGA High Resolution

```basic
SCREEN 9
```

| Setting | Value |
|---------|-------|
| Resolution | 640x350 |
| Colors | 16 (from 64-color palette) |
| Bits Per Pixel | 4 |
| Text Grid | 80x25 or 80x43 |

### Mode 10 - EGA Monochrome

```basic
SCREEN 10
```

| Setting | Value |
|---------|-------|
| Resolution | 640x350 |
| Colors | 9 pseudocolors |
| Bits Per Pixel | 2 |
| Special | Alternating colors (blink effect) |

### Modes 11-12 - VGA

```basic
SCREEN 11   ' 2-color
SCREEN 12   ' 16-color
```

| Mode | Resolution | Colors | BPP |
|------|------------|--------|-----|
| 11 | 640x480 | 2 | 1 |
| 12 | 640x480 | 16 | 4 |

### Mode 13 - VGA 256-Color

```basic
SCREEN 13
```

| Setting | Value |
|---------|-------|
| Resolution | 320x200 |
| Colors | 256 |
| Bits Per Pixel | 8 |
| Text Grid | 40x25 |

## _NEWIMAGE Function

```basic
handle& = _NEWIMAGE(width%, height%, mode%)
```

### Supported Modes

| Mode | Description |
|------|-------------|
| 0 | Text mode |
| 1-2, 7-13 | Legacy compatible |
| 256 | 256-color indexed |
| 32 | 32-bit ARGB (true color) |

### Examples

```basic
' 32-bit graphics screen
SCREEN _NEWIMAGE(800, 600, 32)

' 256-color screen
SCREEN _NEWIMAGE(640, 480, 256)

' Custom text screen
SCREEN _NEWIMAGE(100, 50, 0)
```

## Color Depths

| BPP | Colors | Format |
|-----|--------|--------|
| 1 | 2 | Monochrome |
| 2 | 4 | 2-bit indexed |
| 4 | 16 | 4-bit indexed |
| 8 | 256 | 8-bit indexed |
| 32 | 16.7M | ARGB (8-8-8-8) |

## Palette Handling

### Indexed Modes (1-13, 256)

```basic
' Read palette entry
color& = _PALETTECOLOR(index%, imageHandle&)

' Set palette entry
_PALETTECOLOR index%, newColor&, imageHandle&

' Copy entire palette
_COPYPALETTE sourceHandle&, destHandle&
```

### Default Palettes

- **palette_256[256]**: Standard VGA 256-color palette
- **palette_64[64]**: EGA 64-color palette for Mode 9

## Mode Switching

```basic
' Switch modes
SCREEN 13
' ... graphics code ...
SCREEN 0       ' Return to text mode

' Page flipping
SCREEN 7, , 0, 1   ' Draw on page 0, display page 1
```

### Mode Validation

- Invalid modes (3, 4, 5, 6): Error 5 (Illegal function call)
- Negative mode: Treated as image handle
- Page ranges: 0-7 for mode 0, single page for others

## Screen Dimensions Summary

| Mode | Resolution | Text | BPP | Colors |
|------|------------|------|-----|--------|
| 0 | Variable | 40-80x25-50 | 16 | 16 |
| 1 | 320x200 | 40x25 | 2 | 4 |
| 2 | 640x200 | 80x25 | 1 | 2 |
| 7 | 320x200 | 40x25 | 4 | 16 |
| 8 | 640x200 | 80x25 | 4 | 16 |
| 9 | 640x350 | 80x25 | 4 | 16/64 |
| 10 | 640x350 | 80x25 | 2 | 9 |
| 11 | 640x480 | 80x30 | 1 | 2 |
| 12 | 640x480 | 80x30 | 4 | 16 |
| 13 | 320x200 | 40x25 | 8 | 256 |
| 256 | Custom | Custom | 8 | 256 |
| 32 | Custom | Custom | 32 | 16.7M |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | qbg_screen() (7750-8343) |
| `internal/c/libqb/include/graphics.h` | img_struct definition |
| `source/subs_functions/subs_functions.bas` | SCREEN syntax |

## Example Usage

### Text Mode

```basic
SCREEN 0
WIDTH 80, 25
COLOR 14, 1
CLS
PRINT "Yellow on blue"
```

### Graphics Mode

```basic
SCREEN 12
LINE (0, 0)-(639, 479), 15
CIRCLE (320, 240), 100, 12
PAINT (320, 240), 10, 12
```

### 32-bit Graphics

```basic
SCREEN _NEWIMAGE(800, 600, 32)
LINE (0, 0)-(799, 599), _RGB(255, 0, 0)
CIRCLE (400, 300), 150, _RGB(0, 255, 0)
```

### Page Flipping

```basic
SCREEN 7, , 0, 1
DO
    SCREEN , , 0, 1: CLS: DrawFrame 1
    SCREEN , , 1, 0: CLS: DrawFrame 2
LOOP
```
