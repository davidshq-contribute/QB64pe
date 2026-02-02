# FONTS.md

Documentation of the QB64pe font system implementation.

## Overview

QB64pe provides TrueType/OpenType font support via FreeType integration, with built-in fonts for legacy compatibility and Unicode text rendering.

## Font Handle System

### Handle Types

| Range | Type |
|-------|------|
| 0 | Current screen font |
| 1-16 | Built-in fonts |
| > 16 | Loaded custom fonts |
| -1 | Invalid handle |

### font_struct (internal/c/libqb.cpp)

```c
struct font_struct {
    int32 handle;                // FreeType face handle
    int32 w, h;                  // Character dimensions
    int32 baseline;              // Baseline offset
    int32 flags;                 // Font properties
    int32 monospace;             // Fixed-width flag
    FT_Face face;                // FreeType face object
    uint8 *data;                 // Font file data
    int32 data_size;             // Data size
};

font_struct fonts[256];
int32 font_count;
```

## Built-in Fonts

### Font 8 (Default)

- 8×8 pixel fixed-width
- CP437 character set
- Screen modes 0, 1, 2, 7, 8, 9, 10, 11, 12, 13

### Font 14

- 8×14 pixel fixed-width
- EGA-style font
- Screen mode 9

### Font 16

- 8×16 pixel fixed-width
- VGA-style font
- Screen modes 11, 12

### Font Data Location

```
internal/c/parts/video/font/ttf/
```

Built-in bitmap fonts encoded as C arrays.

## Font Loading

### _LOADFONT

```c
int32 func__loadfont(qbs *filename, int32 size, qbs *requirements, int32 passed);
```

**Syntax:**
```basic
handle& = _LOADFONT("fontfile.ttf", height%, "requirements")
```

**Requirements string:**
| Option | Description |
|--------|-------------|
| `BOLD` | Synthetic bold |
| `ITALIC` | Synthetic italic |
| `UNDERLINE` | Underline text |
| `DONTBLEND` | Disable anti-aliasing |
| `MONOSPACE` | Force fixed-width |
| `UNICODE` | Enable Unicode |

**Implementation:**
1. Load font file into memory
2. Initialize FreeType face
3. Set pixel size
4. Store in fonts array
5. Return handle

### _FREEFONT

```c
void sub__freefont(int32 handle);
```

Releases font handle and FreeType resources.

## Font Selection

### _FONT

```c
void sub__font(int32 handle, int32 dest, int32 passed);
```

**Sets active font for:**
- PRINT statement
- _PRINTSTRING function
- Text rendering operations

**For image handles:**
```basic
_FONT handle&, imageHandle&
```

### Current Font Query

```c
int32 func__font(int32 dest, int32 passed);
```

Returns current font handle for destination.

## Text Rendering

### _PRINTSTRING

```c
void sub__printstring(float x, float y, qbs *text, int32 dest, int32 passed);
```

**Features:**
- Pixel-positioned text
- Anti-aliased rendering
- Unicode support
- Alpha blending

### _PRINTWIDTH

```c
int32 func__printwidth(qbs *text, int32 dest, int32 passed);
```

Returns pixel width of text string in current font.

### _FONTHEIGHT

```c
int32 func__fontheight(int32 handle, int32 passed);
```

Returns pixel height of font.

### _FONTWIDTH

```c
int32 func__fontwidth(int32 handle, int32 passed);
```

Returns pixel width (monospace fonts) or 0 (proportional).

## FreeType Integration

### Initialization

```c
FT_Library ft_library;

int32 font_init() {
    return FT_Init_FreeType(&ft_library);
}
```

### Face Loading

```c
FT_Face face;
FT_Error error = FT_New_Memory_Face(
    ft_library,
    font_data,
    font_size,
    0,              // Face index
    &face
);
```

### Glyph Rendering

```c
FT_Load_Char(face, charcode, FT_LOAD_RENDER);
FT_GlyphSlot glyph = face->glyph;
FT_Bitmap *bitmap = &glyph->bitmap;
```

## Print Modes

### _PRINTMODE

```c
void sub__printmode(int32 mode, int32 dest, int32 passed);
```

| Mode | Constant | Description |
|------|----------|-------------|
| 1 | `_KEEPBACKGROUND` | Transparent background |
| 2 | `_ONLYBACKGROUND` | Only draw background |
| 3 | `_FILLBACKGROUND` | Solid background (default) |

### Mode Implementation

```c
switch (print_mode) {
    case 1:  // KEEPBACKGROUND
        // Only draw foreground pixels
        break;
    case 2:  // ONLYBACKGROUND
        // Only draw background pixels
        break;
    case 3:  // FILLBACKGROUND
        // Draw both foreground and background
        break;
}
```

## Unicode Support

### Character Mapping

```c
FT_UInt glyph_index = FT_Get_Char_Index(face, unicode_codepoint);
```

### UTF-8 Processing

```c
int32 utf8_to_unicode(const uint8 *text, int32 *bytes_consumed) {
    // Decode UTF-8 sequence
    if ((text[0] & 0x80) == 0) {
        *bytes_consumed = 1;
        return text[0];
    }
    // Multi-byte sequences...
}
```

### Unicode Enable

```basic
handle& = _LOADFONT("font.ttf", 24, "UNICODE")
```

## Text Measurement

### Glyph Metrics

```c
struct glyph_metrics {
    int32 width;             // Bitmap width
    int32 height;            // Bitmap height
    int32 advance_x;         // Horizontal advance
    int32 bearing_x;         // Left side bearing
    int32 bearing_y;         // Top side bearing
};
```

### String Width Calculation

```c
int32 calc_string_width(FT_Face face, qbs *text) {
    int32 width = 0;
    for (int i = 0; i < text->len; i++) {
        FT_Load_Char(face, text->chr[i], FT_LOAD_DEFAULT);
        width += face->glyph->advance.x >> 6;
    }
    return width;
}
```

## Glyph Caching

### Cache Structure

```c
struct glyph_cache_entry {
    uint32 charcode;
    uint8 *bitmap;
    int32 width, height;
    int32 bearing_x, bearing_y;
    int32 advance;
};

glyph_cache_entry glyph_cache[CACHE_SIZE];
```

### Cache Lookup

```c
glyph_cache_entry *get_cached_glyph(int32 font_handle, uint32 charcode) {
    // Hash lookup
    int32 index = charcode % CACHE_SIZE;
    if (glyph_cache[index].charcode == charcode) {
        return &glyph_cache[index];
    }
    // Load and cache
    return load_and_cache_glyph(font_handle, charcode);
}
```

## Screen Font Modes

### Text Mode Fonts

```basic
SCREEN 0
WIDTH 80, 25      ' Default VGA text
WIDTH 80, 43      ' EGA 43-line
WIDTH 80, 50      ' VGA 50-line
```

### Graphics Mode Fonts

```basic
SCREEN 12
_FONT 16          ' Use 16-pixel font
PRINT "Text in graphics mode"
```

## Error Handling

| Error | Cause |
|-------|-------|
| 5 | Illegal function call |
| 53 | File not found |
| 258 | Invalid handle |
| 298 | Font not loaded |

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | Font functions |
| `internal/c/parts/video/font/` | FreeType wrapper |
| `internal/c/parts/video/font/ttf/` | Built-in fonts |

## Example Usage

### Basic Font Loading

```basic
f& = _LOADFONT("arial.ttf", 24)
IF f& > 0 THEN
    _FONT f&
    PRINT "Hello in Arial!"
    _FREEFONT f&
END IF
```

### Pixel-Positioned Text

```basic
SCREEN _NEWIMAGE(800, 600, 32)
f& = _LOADFONT("times.ttf", 32, "BOLD")
_FONT f&

_PRINTSTRING (100, 100), "Positioned Text"

PRINT "_PRINTWIDTH:"; _PRINTWIDTH("Test")
PRINT "_FONTHEIGHT:"; _FONTHEIGHT(f&)

_FREEFONT f&
```

### Unicode Text

```basic
f& = _LOADFONT("unifont.ttf", 16, "UNICODE")
_FONT f&
_PRINTSTRING (10, 10), "Hello 世界 مرحبا"
_FREEFONT f&
```

### Print Modes

```basic
SCREEN _NEWIMAGE(640, 480, 32)
CLS , _RGB(0, 0, 128)  ' Blue background

f& = _LOADFONT("arial.ttf", 24)
_FONT f&
COLOR _RGB(255, 255, 0), _RGB(255, 0, 0)

_PRINTMODE _FILLBACKGROUND
_PRINTSTRING (10, 50), "Filled Background"

_PRINTMODE _KEEPBACKGROUND
_PRINTSTRING (10, 100), "Transparent Background"

_FREEFONT f&
```

### Font Measurements

```basic
f& = _LOADFONT("courier.ttf", 16, "MONOSPACE")
_FONT f&

text$ = "Hello, World!"
w% = _PRINTWIDTH(text$)
h% = _FONTHEIGHT(f&)
fw% = _FONTWIDTH(f&)

PRINT "Text width:"; w%
PRINT "Font height:"; h%
PRINT "Char width:"; fw%

_FREEFONT f&
```
