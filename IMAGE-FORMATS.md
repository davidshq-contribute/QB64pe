# IMAGE-FORMATS.md

Documentation of the QB64pe image loading and saving system.

## Overview

QB64pe supports loading and saving images in multiple formats through integrated libraries including stb_image, nanosvg, and format-specific handlers.

## _LOADIMAGE Function

```basic
handle& = _LOADIMAGE(filename$[, mode%])
handle& = _LOADIMAGE(filename$, mode%, requirements$)
```

### Mode Parameter

| Mode | Description |
|------|-------------|
| 32 | 32-bit ARGB (software) |
| 33 | 32-bit hardware texture |
| 256 | 256-color indexed |
| 257 | 256-color adaptive palette |

### Supported Input Formats

| Format | Library | Notes |
|--------|---------|-------|
| PNG | stb_image | 1/2/4/8/16-bit channels |
| JPEG | stb_image | Baseline & progressive |
| BMP | stb_image | Non-RLE |
| TGA | stb_image | Uncompressed/RLE |
| GIF | stb_image | Single frame |
| PSD | stb_image | Composited only |
| HDR | stb_image | Radiance format |
| PIC | stb_image | Softimage |
| PNM | stb_image | PPM/PGM binary |
| PCX | sg_pcx | Custom decoder |
| QOI | qoi | Quite OK Image |
| ICO/CUR | sg_curico | Windows icons |
| SVG | nanosvg | Vector (rasterized) |

### Requirements String

```basic
' Hardware texture
img& = _LOADIMAGE("sprite.png", 33, "HARDWARE")

' Load from memory buffer
img& = _LOADIMAGE(buffer$, 32, "MEMORY")

' Preserve original palette
img& = _LOADIMAGE("indexed.png", 256, "ADAPTIVE")

' Pixel scaling
img& = _LOADIMAGE("small.png", 32, "SXBR2")
```

| Requirement | Description |
|-------------|-------------|
| HARDWARE | GPU texture |
| MEMORY | Load from string data |
| ADAPTIVE | Keep original palette |
| SXBR2/3/4 | Super XBR scaling |
| MMPX2 | MMPX 2x scaling |
| HQ2XA/B | HQ2x variants |
| HQ3XA/B | HQ3x variants |

## _SAVEIMAGE Function

```basic
_SAVEIMAGE filename$, imageHandle&
_SAVEIMAGE filename$, imageHandle&, requirements$
```

### Supported Output Formats

| Format | Extension | Quality |
|--------|-----------|---------|
| PNG | .png | Lossless |
| BMP | .bmp | Lossless |
| TGA | .tga | Lossless |
| JPEG | .jpg | Lossy (Q=100) |
| QOI | .qoi | Lossless |
| GIF | .gif | Single frame |
| ICO | .ico | Windows icon |
| HDR | .hdr | Linear float |

### Format Detection

1. Scan requirements string for format name
2. Check file extension
3. Default to PNG

```basic
' Save as PNG (default)
_SAVEIMAGE "output.png", img&

' Force JPEG format
_SAVEIMAGE "output", img&, "JPG"
```

## Color Conversion

### Loading Process

```
File → Decode to RGBA → Convert to BGRA → QB64 image
```

### 8-bit Conversion

For 256-color modes:
- Extract unique colors (if ≤256)
- Or apply color quantization
- Ordered dithering (Bayer 4x4 matrix)
- Palette mapping to VGA colors

### 32-bit Format

Internal storage: BGRA
```c
color = blue | (green << 8) | (red << 16) | (alpha << 24)
```

## Hardware vs Software Images

### Software Images (Mode 32, 256)

- Stored in RAM
- Direct pixel access via _MEM
- Full graphics primitive support

### Hardware Images (Mode 33)

- Stored as GPU textures
- Fast rendering with _PUTIMAGE
- Limited direct access
- Created via _COPYIMAGE or _LOADIMAGE

```basic
' Software to hardware conversion
hw& = _COPYIMAGE(sw&, 33)
```

## Image Management

### _COPYIMAGE

```basic
copy& = _COPYIMAGE(handle&[, mode%])
```

Duplicates image, optionally converting mode.

### _FREEIMAGE

```basic
_FREEIMAGE handle&
```

Releases image memory.

### Validation

```basic
IF handle& < -1 THEN
    ' Valid image
ELSE
    ' Failed to load (-1)
END IF
```

## Integrated Libraries

### stb_image (Public Domain)

Primary decoder for common formats.
- Source: github.com/nothings/stb
- Formats: PNG, JPEG, BMP, TGA, GIF, PSD, HDR, PIC, PNM

### nanosvg (zlib License)

SVG vector graphics rasterizer.
- Source: github.com/memononen/nanosvg
- Scales at render time

### qoi (MIT License)

Quite OK Image format.
- Source: qoiformat.org
- 20-50% smaller than PNG
- 3-4x faster decoding

### Pixel Scalers

- SXBR: Super XBR algorithm (2x, 3x, 4x)
- MMPX: 2x scaling
- HQx: High-quality scaling

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/parts/video/image/image.cpp` | Main functions |
| `internal/c/parts/video/image/stb/` | stb_image library |
| `internal/c/parts/video/image/nanosvg/` | SVG support |
| `internal/c/parts/video/image/qoi/` | QOI format |
| `internal/c/parts/video/image/sg_pcx/` | PCX decoder |
| `internal/c/parts/video/image/sg_curico/` | ICO/CUR handler |

## Example Usage

### Basic Loading

```basic
SCREEN _NEWIMAGE(800, 600, 32)
img& = _LOADIMAGE("photo.jpg", 32)
IF img& < -1 THEN
    _PUTIMAGE (0, 0), img&
    _FREEIMAGE img&
ELSE
    PRINT "Failed to load image"
END IF
```

### Hardware Textures

```basic
SCREEN _NEWIMAGE(800, 600, 32)
sprite& = _LOADIMAGE("sprite.png", 33, "HARDWARE")
DO
    CLS
    _PUTIMAGE (x, y), sprite&
    _DISPLAY
    x = x + 1
    _LIMIT 60
LOOP
_FREEIMAGE sprite&
```

### Saving Screenshots

```basic
SCREEN _NEWIMAGE(640, 480, 32)
' Draw something...
_SAVEIMAGE "screenshot.png", 0
```

### Indexed Images

```basic
img& = _LOADIMAGE("palette.png", 256)
IF img& < -1 THEN
    SCREEN img&
END IF
```

### SVG Loading

```basic
' SVG rasterized at load time
logo& = _LOADIMAGE("logo.svg", 32)
_PUTIMAGE (100, 100), logo&
```

### Memory Loading

```basic
' Load image from embedded data
DIM buffer AS STRING
buffer = LoadResourceData("embedded.png")
img& = _LOADIMAGE(buffer, 32, "MEMORY")
```

### Scaled Loading

```basic
' Load with 2x Super XBR upscaling
img& = _LOADIMAGE("pixel_art.png", 32, "SXBR2")
```
