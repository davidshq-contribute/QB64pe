# PRINTING.md

Documentation of the QB64pe printer output system.

## Overview

QB64pe supports printer output through LPRINT for text and _PRINTIMAGE for graphics, with automatic buffering and page management.

## LPRINT Statement

```basic
LPRINT "Hello, World!"
LPRINT "Value:"; x; TAB(20); "More text"
LPRINT USING "###.##"; value#
```

Works like PRINT but directs output to printer.

### Implementation

```c
void qbs_lprint(qbs *str, int32 finish_on_new_line) {
    while (lprint_locked) Sleep(64);  // Wait if locked
    lprint = 1;

    if (!lprint_image) {
        lprint_image = func__newimage(640, 960, 13, 1);
        sub__dest(lprint_image);
        sub_cls(NULL, 15, 2);           // White background
        sub__font(16, NULL, 0);         // 16-pixel font
        qbg_sub_color(0, 15, NULL, NULL, 3);
        qbg_sub_view_print(1, 60, 1);   // 60 lines
    }

    lprint_buffered = 1;
    lprint_last = func_timer(0.001, 1);
    qbs_print(str, finish_on_new_line);
    sub__dest(old_dest);
}
```

### State Variables

```c
int32 lprint = 0;           // Active during LPRINT
int32 lprint_image = 0;     // Buffer image handle
double lprint_last = 0;     // Timer of last LPRINT
int32 lprint_buffered = 0;  // Content pending
int32 lprint_locked = 0;    // Locked during print
int32 lpos = 1;             // Column position
int32 width_lprint = 80;    // Line width
```

## _PRINTIMAGE Function

```basic
_PRINTIMAGE imageHandle&
```

Prints an image to the default printer.

### Windows Implementation

```c
void sub__printimage(int32 i) {
    // Get default printer
    GetDefaultPrinterA(szPrinterName, &dwNameLen);
    dc = CreateDCA("WINSPOOL", szPrinterName, NULL, NULL);

    // Start document
    DOCINFOA di;
    di.lpszDocName = "Document";
    StartDocA(dc, &di);
    StartPage(dc);

    // Scale image to printer resolution
    w = GetDeviceCaps(dc, HORZRES);
    h = GetDeviceCaps(dc, VERTRES);
    i2 = func__newimage(w, h, 32, 1);
    sub__putimage(NULL, NULL, ..., i, i2, ...);

    // Render bitmap
    for (y = 0; y < h; y++) {
        SetDIBitsToDevice(dc, 0, y, w, 1, 0, 0, 0, 1,
            s2->offset32 + (y * w), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    }

    // Finish
    EndPage(dc);
    EndDoc(dc);
    DeleteDC(dc);
}
```

### Platform Support

| Platform | Status |
|----------|--------|
| Windows | Full implementation |
| Linux | Stub only |
| macOS | Stub only |

## WIDTH LPRINT

```basic
WIDTH LPRINT 132
```

Sets print line width (1-255 characters, default 80).

## LPOS Function

```basic
column% = LPOS(0)    ' Current column position
```

Returns current print column (1-based).

## Form Feed

```basic
LPRINT CHR$(12)      ' Form feed
```

- Prints current page
- Clears buffer
- Resets position to column 1

## Auto-Flush Behavior

### On Page Full

When cursor exceeds bottom row:
```c
if (write_page->cursor_y > write_page->bottom_row) {
    if (lprint) {
        sub__printimage(lprint_image);
        sub_cls(NULL, 15, 2);
        lprint_buffered = 0;
    }
}
```

### On Idle Timeout

After 10 seconds of inactivity:
```c
if (lprint_buffered) {
    if (fabs(func_timer(0.001, 1) - lprint_last) >= 10.0) {
        sub__printimage(lprint_image);
        lprint_buffered = 0;
    }
}
```

### On Program Exit

```c
if (lprint_buffered) {
    sub__printimage(lprint_image);
}
```

## TAB/SPC with LPRINT

TAB and SPC functions work with LPRINT:

```basic
LPRINT "Name"; TAB(20); "Score"; TAB(40); "Grade"
LPRINT name$; TAB(20); score%; TAB(40); grade$
```

## Dependency System

LPRINT usage triggers:
```basic
SetDependency DEPENDENCY_PRINTER
```

Compilation includes:
```makefile
DEP_PRINTER=y
```

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | LPRINT code generation (23150-23364) |
| `internal/c/libqb.cpp` | qbs_lprint (11447-11466), sub__printimage (24926-25032) |
| `source/subs_functions/subs_functions.bas` | Function registration |

## Example Usage

### Basic Printing

```basic
LPRINT "Report Title"
LPRINT STRING$(40, "-")

FOR i = 1 TO 10
    LPRINT USING "Item ## - Value: ###.##"; i; values(i)
NEXT

LPRINT CHR$(12)  ' Eject page
```

### Formatted Report

```basic
WIDTH LPRINT 132

LPRINT TAB(50); "SALES REPORT"
LPRINT TAB(50); DATE$
LPRINT

LPRINT "Product"; TAB(30); "Quantity"; TAB(50); "Price"; TAB(70); "Total"
LPRINT STRING$(80, "-")

FOR i = 1 TO numItems
    LPRINT products$(i); TAB(30); qty(i); TAB(50);
    LPRINT USING "$$###.##"; price(i);
    LPRINT TAB(70);
    LPRINT USING "$$#,###.##"; qty(i) * price(i)
NEXT
```

### Print Graphics

```basic
SCREEN _NEWIMAGE(640, 480, 32)

' Draw chart
LINE (50, 50)-(590, 430), _RGB(0, 0, 0), B
FOR i = 0 TO 10
    LINE (50, 50 + i * 38)-(590, 50 + i * 38), _RGB(200, 200, 200)
NEXT

' Print it
_PRINTIMAGE 0
```

### Print with Multiple Pages

```basic
pageNum = 1
lineNum = 0

FOR i = 1 TO 1000
    LPRINT record$(i)
    lineNum = lineNum + 1

    IF lineNum >= 55 THEN
        LPRINT
        LPRINT "Page"; pageNum
        LPRINT CHR$(12)
        pageNum = pageNum + 1
        lineNum = 0
    END IF
NEXT

' Print final partial page
IF lineNum > 0 THEN
    LPRINT
    LPRINT "Page"; pageNum
END IF
```
