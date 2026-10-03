# PRINT-FORMATTING.md

Documentation of the QB64pe PRINT statement and text formatting system.

## Overview

QB64pe provides QBasic-compatible PRINT functionality with zones, TAB, SPC, LOCATE, COLOR, and PRINT USING formatting.

## Basic PRINT Statement

### Syntax

```basic
PRINT [expression][{, | ;} expression]...
```

### Separators

| Separator | Behavior |
|-----------|----------|
| `;` | No space between items |
| `,` | Advance to next print zone |
| (none) | Newline after output |

### Implementation (libqb.cpp)

```c
void sub_print(qbs *text, int32 newline) {
    // Output text to current destination
    // Handle zones, tabs, newlines
}
```

## Print Zones

### Zone Width

Default zone width: 14 characters

```c
#define PRINT_ZONE_WIDTH 14
```

### Zone Calculation

```c
int32 next_zone(int32 column) {
    return ((column / PRINT_ZONE_WIDTH) + 1) * PRINT_ZONE_WIDTH + 1;
}
```

### Example

```basic
PRINT 1, 2, 3
' Output:
' 1             2             3
' ^col 1        ^col 15       ^col 29
```

## TAB Function

### Syntax

```basic
PRINT TAB(column); "text"
```

### Implementation

```c
void func_tab(int32 column) {
    if (column < 1) column = 1;
    if (column > current_column) {
        // Print spaces to reach column
        int spaces = column - current_column;
        while (spaces--) output_char(' ');
    } else {
        // Wrap to next line if needed
        newline();
        tab(column);
    }
}
```

### Behavior

- Column 1 is leftmost
- If already past column, goes to next line
- Maximum column depends on screen width

## SPC Function

### Syntax

```basic
PRINT SPC(count); "text"
```

### Implementation

```c
void func_spc(int32 count) {
    while (count-- > 0) {
        output_char(' ');
    }
}
```

## LOCATE Statement

### Syntax

```basic
LOCATE [row][, [column][, [cursor][, [start][, stop]]]]
```

### Implementation

```c
void sub_locate(int32 row, int32 col, int32 cursor,
                int32 start, int32 stop, int32 passed) {
    if (passed & 1) current_row = row;
    if (passed & 2) current_column = col;
    if (passed & 4) cursor_visible = cursor;
    if (passed & 8) cursor_start = start;
    if (passed & 16) cursor_stop = stop;
}
```

### Cursor Shape

| start | stop | Appearance |
|-------|------|------------|
| 0 | 7 | Full block |
| 6 | 7 | Underline |
| 0 | 3 | Top half |

## CSRLIN and POS Functions

### CSRLIN

```c
int32 func_csrlin() {
    return current_row;
}
```

Returns current cursor row (1-based).

### POS

```c
int32 func_pos(int32 dummy) {
    return current_column;
}
```

Returns current cursor column (1-based).

## COLOR Statement

### Text Mode Syntax

```basic
COLOR [foreground][, [background]]
```

### Graphics Mode Syntax

```basic
COLOR [foreground][, [background]]
```

### 32-bit Color

```basic
COLOR _RGB(255, 0, 0)           ' Red foreground
COLOR , _RGB(0, 0, 255)         ' Blue background
```

### Implementation

```c
void sub_color(uint32 foreground, uint32 background, int32 passed) {
    if (passed & 1) {
        write_page->draw_color = foreground;
    }
    if (passed & 2) {
        write_page->background_color = background;
    }
}
```

### Legacy Palette

| Attribute | Color |
|-----------|-------|
| 0 | Black |
| 1 | Blue |
| 2 | Green |
| 3 | Cyan |
| 4 | Red |
| 5 | Magenta |
| 6 | Brown/Yellow |
| 7 | White |
| 8-15 | Bright variants |

## PRINT USING

### Syntax

```basic
PRINT USING format$; expression[; expression]...
```

### Numeric Format Characters

| Character | Meaning |
|-----------|---------|
| `#` | Digit position |
| `.` | Decimal point |
| `,` | Thousands separator |
| `+` | Show sign |
| `-` | Trailing minus for negative |
| `$$` | Floating dollar sign |
| `**` | Fill with asterisks |
| `^^^^` | Exponential format |

### String Format Characters

| Character | Meaning |
|-----------|---------|
| `!` | First character only |
| `&` | Entire string |
| `\ \` | Fixed-width field (n+2 chars) |
| `_` | Literal next character |

### Implementation (libqb.cpp)

```c
qbs *func_print_using(qbs *format, int32 argcount, ...) {
    va_list args;
    va_start(args, argcount);

    qbs *result = qbs_new(0, 0);
    int fpos = 0;

    while (fpos < format->len) {
        // Parse format specifier
        // Format argument
        // Append to result
    }

    va_end(args);
    return result;
}
```

### Numeric Formatting Rules

**Right-justified:**
```basic
PRINT USING "####"; 42    ' Output: "  42"
```

**Decimal places:**
```basic
PRINT USING "##.##"; 3.1  ' Output: " 3.10"
```

**Thousands separator:**
```basic
PRINT USING "##,###"; 1234 ' Output: " 1,234"
```

**Exponential:**
```basic
PRINT USING "##.##^^^^"; 1234 ' Output: "1.23E+03"
```

### String Formatting Rules

**First character:**
```basic
PRINT USING "!"; "Hello"   ' Output: "H"
```

**Fixed width:**
```basic
PRINT USING "\   \"; "Hi" ' Output: "Hi   " (5 chars)
```

**Full string:**
```basic
PRINT USING "&"; "Hello"   ' Output: "Hello"
```

## WRITE Statement

### Syntax

```basic
WRITE [expression][, expression]...
```

### Behavior

- Strings enclosed in quotes
- Items separated by commas
- Numbers without leading space

### Implementation

```c
void sub_write(qbs *text, int32 newline) {
    if (is_string) {
        output_char('"');
        output_string(text);
        output_char('"');
    } else {
        output_string(text);  // No leading space
    }
}
```

### Example

```basic
WRITE "Hello", 42, 3.14
' Output: "Hello",42,3.14
```

## LPRINT Statement

### Syntax

```basic
LPRINT [expression][{, | ;} expression]...
```

### Implementation

Outputs to default printer device (platform-specific).

## Screen Output

### Text Mode Output

```c
void print_text_char(uint8 c) {
    // Calculate screen position
    int offset = (current_row - 1) * screen_width + (current_column - 1);

    // Write character and attribute
    screen_text[offset * 2] = c;
    screen_text[offset * 2 + 1] = current_attribute;

    // Advance cursor
    advance_cursor();
}
```

### Graphics Mode Output

```c
void print_graphics_char(uint8 c) {
    // Render character using current font
    render_glyph(c, cursor_x, cursor_y, foreground_color);

    // Advance cursor by character width
    cursor_x += char_width;
}
```

## Scrolling

### Scroll Trigger

```c
void check_scroll() {
    if (current_row > screen_rows) {
        scroll_up(1);
        current_row = screen_rows;
    }
}
```

### Scroll Implementation

```c
void scroll_up(int lines) {
    // Move screen contents up
    memmove(screen_buffer,
            screen_buffer + lines * screen_width * 2,
            (screen_rows - lines) * screen_width * 2);

    // Clear bottom lines
    memset(screen_buffer + (screen_rows - lines) * screen_width * 2,
           0, lines * screen_width * 2);
}
```

## VIEW PRINT

### Syntax

```basic
VIEW PRINT [top TO bottom]
```

### Implementation

```c
void sub_view_print(int32 top, int32 bottom, int32 passed) {
    if (passed == 0) {
        // Reset to full screen
        view_print_top = 1;
        view_print_bottom = screen_rows;
    } else {
        view_print_top = top;
        view_print_bottom = bottom;
    }
}
```

Restricts scrolling region.

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | PRINT, LOCATE, COLOR |
| `internal/c/libqb/src/print_using.cpp` | PRINT USING formatting |
| `source/qb64pe.bas` | Code generation |

## Example Usage

### Basic Output

```basic
PRINT "Hello, World!"
PRINT 42; "is the answer"
PRINT 1, 2, 3             ' Zoned output
```

### Cursor Control

```basic
LOCATE 10, 20
PRINT "Centered"
PRINT "Row:"; CSRLIN; "Col:"; POS(0)
```

### Color Output

```basic
COLOR 14, 1               ' Yellow on blue
PRINT "Highlighted"
COLOR 7, 0                ' Reset
```

### PRINT USING Examples

```basic
' Numeric formatting
PRINT USING "###.##"; 3.14159     ' "  3.14"
PRINT USING "+###"; -42           ' " -42"
PRINT USING "$$###.##"; 99.5      ' " $99.50"
PRINT USING "**###.##"; 5.25      ' "***5.25"

' String formatting
PRINT USING "!"; "Hello"          ' "H"
PRINT USING "\     \"; "Hi"       ' "Hi     "
PRINT USING "& &"; "Hello"; "World" ' "Hello World"
```

### TAB and SPC

```basic
PRINT "Col 1"; TAB(20); "Col 20"; TAB(40); "Col 40"
PRINT "One"; SPC(5); "Two"; SPC(10); "Three"
```

### VIEW PRINT Scrolling

```basic
VIEW PRINT 5 TO 20        ' Scroll only rows 5-20
FOR i = 1 TO 100
    PRINT i
NEXT
VIEW PRINT                ' Reset
```
