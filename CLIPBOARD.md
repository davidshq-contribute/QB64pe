# CLIPBOARD.md

Documentation of the QB64pe clipboard system implementation.

## Overview

QB64pe provides cross-platform clipboard access for both text and image data using the "clip" library for text and platform-specific APIs for images.

## Text Clipboard

### _CLIPBOARD$ Function (Get)

```c
qbs *func__clipboard();
```

**Usage:**
```basic
text$ = _CLIPBOARD$
```

**Implementation:**
1. Query system clipboard for text
2. Convert to QB64 string
3. Handle encoding (UTF-8 to internal)

### _CLIPBOARD$ Statement (Set)

```c
void sub__clipboard(qbs *text);
```

**Usage:**
```basic
_CLIPBOARD$ = "Hello, World!"
```

**Implementation:**
1. Convert QB64 string to system format
2. Clear existing clipboard
3. Set new text content

## Image Clipboard

### _CLIPBOARDIMAGE Function (Get)

```c
int32 func__clipboardimage();
```

**Usage:**
```basic
imgHandle& = _CLIPBOARDIMAGE
IF imgHandle& < -1 THEN
    _PUTIMAGE , imgHandle&, 0
    _FREEIMAGE imgHandle&
END IF
```

**Returns:**
- Valid image handle (< -1): Image from clipboard
- -1: No image available or error

**Implementation:**
1. Query clipboard for image data
2. Create new image handle
3. Copy pixel data to image
4. Return handle

### _CLIPBOARDIMAGE Statement (Set)

```c
void sub__clipboardimage(int32 handle);
```

**Usage:**
```basic
_CLIPBOARDIMAGE = imageHandle&
```

**Implementation:**
1. Validate image handle
2. Extract pixel data
3. Convert to platform format (BMP/PNG)
4. Set clipboard image

## Clip Library Integration

### Location

```
internal/c/parts/os/clipboard/
```

### Core Functions

```c
namespace clip {
    bool has(format f);              // Check format availability
    bool clear();                    // Clear clipboard
    bool set_text(const std::string &value);
    bool get_text(std::string &value);
    bool set_image(const image &img);
    bool get_image(image &img);
}
```

### Format Constants

```c
namespace clip {
    enum class format {
        empty,
        text,
        image
    };
}
```

## Platform Implementations

### Windows

**Text:**
```c
bool get_text(std::string &value) {
    if (!OpenClipboard(NULL)) return false;

    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (h) {
        wchar_t *data = (wchar_t *)GlobalLock(h);
        // Convert to UTF-8
        GlobalUnlock(h);
    }

    CloseClipboard();
    return true;
}

bool set_text(const std::string &value) {
    if (!OpenClipboard(NULL)) return false;
    EmptyClipboard();

    // Allocate and set CF_UNICODETEXT
    HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, size);
    wchar_t *data = (wchar_t *)GlobalLock(h);
    // Copy data
    GlobalUnlock(h);
    SetClipboardData(CF_UNICODETEXT, h);

    CloseClipboard();
    return true;
}
```

**Image:**
```c
bool get_image(image &img) {
    if (!OpenClipboard(NULL)) return false;

    HANDLE h = GetClipboardData(CF_DIBV5);
    if (!h) h = GetClipboardData(CF_DIB);

    if (h) {
        BITMAPINFO *bmi = (BITMAPINFO *)GlobalLock(h);
        // Extract pixel data from DIB
        GlobalUnlock(h);
    }

    CloseClipboard();
    return true;
}
```

### Linux (X11)

**Text:**
```c
bool get_text(std::string &value) {
    Display *display = XOpenDisplay(NULL);
    Window window = create_helper_window(display);

    // Request CLIPBOARD selection
    Atom clipboard = XInternAtom(display, "CLIPBOARD", False);
    XConvertSelection(display, clipboard, XA_STRING,
                      clipboard, window, CurrentTime);

    // Wait for SelectionNotify event
    // Read selection data

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return true;
}
```

**Image:**
```c
bool get_image(image &img) {
    // Request image/png or image/bmp target
    Atom targets[] = {
        XInternAtom(display, "image/png", False),
        XInternAtom(display, "image/bmp", False)
    };
    // Similar selection request process
}
```

### macOS (Cocoa)

**Text:**
```c
bool get_text(std::string &value) {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    NSString *str = [pb stringForType:NSPasteboardTypeString];
    if (str) {
        value = [str UTF8String];
        return true;
    }
    return false;
}

bool set_text(const std::string &value) {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    [pb clearContents];
    NSString *str = [NSString stringWithUTF8String:value.c_str()];
    [pb setString:str forType:NSPasteboardTypeString];
    return true;
}
```

**Image:**
```c
bool get_image(image &img) {
    NSPasteboard *pb = [NSPasteboard generalPasteboard];
    NSImage *nsimg = [[NSImage alloc] initWithPasteboard:pb];
    if (nsimg) {
        // Convert NSImage to pixel data
    }
}
```

## Image Format Handling

### Internal Format

```c
struct clip_image {
    uint32_t *pixels;
    int width;
    int height;
    int bits_per_pixel;
};
```

### Conversion to QB64

```c
int32 clipboard_to_image() {
    clip::image img;
    if (!clip::get_image(img)) return -1;

    // Create QB64 image
    int32 handle = func__newimage(img.width, img.height, 32);

    // Copy pixel data
    img_struct *qbimg = &img[handle];
    memcpy(qbimg->offset, img.pixels,
           img.width * img.height * 4);

    return handle;
}
```

### Conversion from QB64

```c
void image_to_clipboard(int32 handle) {
    img_struct *qbimg = &img[handle];

    clip::image img;
    img.width = qbimg->width;
    img.height = qbimg->height;
    img.pixels = (uint32_t *)qbimg->offset;

    clip::set_image(img);
}
```

## Error Handling

### Clipboard Access Errors

| Error | Cause |
|-------|-------|
| 5 | Illegal function call |
| 258 | Invalid image handle |
| -1 return | Clipboard empty/unavailable |

### Platform-Specific Errors

**Windows:**
- `OpenClipboard()` failure (clipboard locked)
- `GetClipboardData()` returns NULL

**Linux:**
- X11 selection timeout
- No compatible format available

**macOS:**
- Pasteboard not available
- Type not found

## Thread Safety

Clipboard operations use mutex protection:

```c
std::mutex clipboard_mutex;

qbs *func__clipboard() {
    std::lock_guard<std::mutex> lock(clipboard_mutex);
    // Clipboard access
}
```

## Clipboard Change Detection

### Polling Method

```basic
DIM lastClip$
DO
    currentClip$ = _CLIPBOARD$
    IF currentClip$ <> lastClip$ THEN
        PRINT "Clipboard changed!"
        lastClip$ = currentClip$
    END IF
    _LIMIT 10
LOOP
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | QB64 wrapper functions |
| `internal/c/parts/os/clipboard/clip.h` | Clip library header |
| `internal/c/parts/os/clipboard/clip.cpp` | Common implementation |
| `internal/c/parts/os/clipboard/clip_win.cpp` | Windows backend |
| `internal/c/parts/os/clipboard/clip_x11.cpp` | X11 backend |
| `internal/c/parts/os/clipboard/clip_osx.mm` | macOS backend |

## Example Usage

### Text Clipboard

```basic
' Copy text to clipboard
_CLIPBOARD$ = "Hello from QB64pe!"

' Get text from clipboard
text$ = _CLIPBOARD$
IF LEN(text$) > 0 THEN
    PRINT "Clipboard contains: "; text$
ELSE
    PRINT "Clipboard is empty"
END IF
```

### Image Clipboard

```basic
' Get image from clipboard
img& = _CLIPBOARDIMAGE
IF img& < -1 THEN
    SCREEN _NEWIMAGE(_WIDTH(img&), _HEIGHT(img&), 32)
    _PUTIMAGE , img&, 0
    _FREEIMAGE img&
ELSE
    PRINT "No image in clipboard"
END IF
```

### Copy Screen to Clipboard

```basic
SCREEN _NEWIMAGE(640, 480, 32)
' Draw something
LINE (0, 0)-(639, 479), _RGB(255, 0, 0), BF
CIRCLE (320, 240), 100, _RGB(0, 255, 0)

' Copy to clipboard
_CLIPBOARDIMAGE = _DEST
PRINT "Screen copied to clipboard!"
```

### Clipboard Monitor

```basic
PRINT "Monitoring clipboard (press ESC to exit)..."
DIM last$

DO
    current$ = _CLIPBOARD$
    IF current$ <> last$ THEN
        CLS
        PRINT "=== Clipboard Updated ==="
        PRINT current$
        PRINT "========================="
        last$ = current$
    END IF
    _LIMIT 5
LOOP UNTIL INKEY$ = CHR$(27)
```

### Copy/Paste in Text Editor

```basic
DIM text$(100), currentLine%
currentLine% = 1

DO
    k$ = INKEY$

    ' Ctrl+C - Copy
    IF k$ = CHR$(3) THEN
        _CLIPBOARD$ = text$(currentLine%)
        PRINT "Copied!"
    END IF

    ' Ctrl+V - Paste
    IF k$ = CHR$(22) THEN
        text$(currentLine%) = _CLIPBOARD$
        PRINT "Pasted: "; text$(currentLine%)
    END IF

    _LIMIT 60
LOOP UNTIL k$ = CHR$(27)
```
