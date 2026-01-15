# QB64-PE Module Interface Documentation

This document describes the public interfaces for completed libqb modules, providing contributors with clear understanding of available functions and their usage.

## Table of Contents

- [File I/O Module (`fileio.h`)](#file-io-module)
- [Color & Palette Module (`color.h`)](#color--palette-module)
- [Screen Management Module (`screen.h`)](#screen-management-module)
- [Text & Font Module (`text.h`)](#text--font-module)
- [Graphics Module (`graphics.h`)](#graphics-module)
- [Networking Module (`networking.h`)](#networking-module)
- [Keyboard Module (`keyboard.h`)](#keyboard-module)
- [Mouse Module (`mouse.h`)](#mouse-module)
- [Platform Module (`platform.h`)](#platform-module)
- [Utility Module (`utility.h`)](#utility-module)
- [Window Module (`window.h`)](#window-module)
- [Console Module (`console.h`)](#console-module)
- [Port I/O Module (`port_io.h`)](#port-io-module)
- [Legacy Memory Module (`mem_legacy.h`)](#legacy-memory-module)
- [State Accessor Module (`libqb_state.h`)](#state-accessor-module)

---

## File I/O Module {#file-io-module}

**Header**: `internal/c/libqb/include/fileio.h`

The File I/O module provides comprehensive file handling capabilities including opening, closing, reading, writing, and file status operations.

### Core File Operations

#### `void sub_open(qbs *name, int32 type, int32 access, int32 sharing, int32 i, int64 record_length, int32 passed)`
Opens a file with specified parameters.
- **name**: File path (QB64 string)
- **type**: File mode (INPUT, OUTPUT, RANDOM, etc.)
- **access**: Read/write access permissions
- **sharing**: File sharing mode
- **i**: File handle number
- **record_length**: Record size for random access files
- **passed**: Parameter passing flags

#### `void sub_open_gwbasic(qbs *typestr, int32 i, qbs *name, int64 record_length, int32 passed)`
Opens a file using GW-BASIC style syntax.
- **typestr**: Mode string ("I", "O", "R", "A", "B")
- **i**: File handle number
- **name**: File path (QB64 string)
- **record_length**: Record size for random access
- **passed**: Parameter passing flags

#### `void sub_close(int32 i2, int32 passed)`
Closes a file handle.
- **i2**: File handle number to close
- **passed**: Parameter passing flags

#### `int32 func_freefile()`
Returns the next available file handle number.
- **Returns**: Available file handle number, or 0 if none available

### Binary File Operations

#### `void sub_bload(qbs *filename, int32 offset, int32 passed)`
Loads binary data from file into memory.
- **filename**: File path (QB64 string)
- **offset**: Memory offset where data will be loaded
- **passed**: Parameter passing flags

#### `void sub_bsave(qbs *filename, int32 offset, int32 size)`
Saves binary data from memory to file.
- **filename**: File path (QB64 string)
- **offset**: Memory offset where data starts
- **size**: Number of bytes to save

### File Status Functions

#### `int64 func_lof(int32 i)`
Returns the length of a file in bytes.
- **i**: File handle number
- **Returns**: File size in bytes

#### `int32 func_eof(int32 i)`
Checks if end of file has been reached.
- **i**: File handle number
- **Returns**: Non-zero if EOF, zero otherwise

#### `void sub_seek(int32 i, int64 pos)`
Sets the file position pointer.
- **i**: File handle number
- **pos**: New position (bytes from start)

#### `int64 func_seek(int32 i)`
Returns the current file position.
- **i**: File handle number
- **Returns**: Current position in bytes

#### `int64 func_loc(int32 i)`
Returns the current record number (for random access files).
- **i**: File handle number
- **Returns**: Current record number

### Generic I/O Helpers

#### `int32 generic_put(int32 i, int32 offset, uint8 *cp, int32 bytes)`
Writes raw bytes to a file.
- **i**: File handle number
- **offset**: Offset within file
- **cp**: Pointer to data buffer
- **bytes**: Number of bytes to write
- **Returns**: Success status

#### `int32 generic_get(int32 i, int32 offset, uint8 *cp, int32 bytes)`
Reads raw bytes from a file.
- **i**: File handle number
- **offset**: Offset within file
- **cp**: Pointer to data buffer
- **bytes**: Number of bytes to read
- **Returns**: Success status

### Number Input Parsing

These functions handle parsing numeric values from files and data statements, used by both file INPUT and DATA/READ operations.

#### Global Variables
- `n_digits`: Number of digits parsed
- `n_digit[256]`: Individual digit values
- `n_exp`: Exponent value
- `n_neg`: Negative flag
- `n_hex`: Hexadecimal flag
- `n_float_value`: Parsed floating-point value
- `n_int64_value`: Parsed 64-bit integer value
- `n_uint64_value`: Parsed unsigned 64-bit integer value

#### Functions
- `int32 n_roundincrement()`: Handles rounding increment parsing
- `int32 n_float()`: Parses floating-point numbers
- `int32 n_int64()`: Parses 64-bit integers
- `int32 n_uint64()`: Parses unsigned 64-bit integers
- `int32 n_inputnumberfromdata(uint8 *data, ptrszint *data_offset, ptrszint data_size)`: Parses numbers from data statements
- `int32 n_inputnumberfromfile(int32 fileno)`: Parses numbers from file input

### File Input Character Helpers

#### `int32 file_input_chr(int32 i)`
Reads a single character from file.
- **i**: File handle number
- **Returns**: Character value

#### `void file_input_skip1310(int32 i, int32 c)`
Skips CR/LF combinations in file input.
- **i**: File handle number
- **c**: Current character

#### `void file_input_nextitem(int32 i, int32 lastc)`
Advances to next item in file input.
- **i**: File handle number
- **lastc**: Last character read

---

## Color & Palette Module {#color--palette-module}

**Header**: `internal/c/libqb/include/color.h`

The Color & Palette module provides comprehensive color management including RGB color creation, palette operations, and color channel extraction.

### Color Matching

#### `uint32_t matchcol(int32_t r, int32_t g, int32_t b)`
Finds the closest palette entry to the specified RGB color.
- **r**: Red component (0-255)
- **g**: Green component (0-255)
- **b**: Blue component (0-255)
- **Returns**: Palette index

#### `uint32_t matchcol(int32_t r, int32_t g, int32_t b, int32_t i)`
Finds the closest palette entry with image support.
- **r**: Red component (0-255)
- **g**: Green component (0-255)
- **b**: Blue component (0-255)
- **i**: Image handle (optional)
- **Returns**: Palette index

### RGB/RGBA Color Creation

#### `uint32_t func__rgb(int32_t r, int32_t g, int32_t b, int32_t i, int32_t passed)`
Creates an RGB color value.
- **r**: Red component (0-255)
- **g**: Green component (0-255)
- **b**: Blue component (0-255)
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: 32-bit color value

#### `uint32_t func__rgba(int32_t r, int32_t g, int32_t b, int32_t a, int32_t i, int32_t passed)`
Creates an RGBA color value with alpha channel.
- **r**: Red component (0-255)
- **g**: Green component (0-255)
- **b**: Blue component (0-255)
- **a**: Alpha component (0-255)
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: 32-bit color value with alpha

### Color Channel Extraction

#### `int32_t func__red(uint32_t col, int32_t i, int32_t passed)`
Extracts the red component from a color value.
- **col**: 32-bit color value
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Red component (0-255)

#### `int32_t func__green(uint32_t col, int32_t i, int32_t passed)`
Extracts the green component from a color value.
- **col**: 32-bit color value
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Green component (0-255)

#### `int32_t func__blue(uint32_t col, int32_t i, int32_t passed)`
Extracts the blue component from a color value.
- **col**: 32-bit color value
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Blue component (0-255)

#### `int32_t func__alpha(uint32_t col, int32_t i, int32_t passed)`
Extracts the alpha component from a color value.
- **col**: 32-bit color value
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Alpha component (0-255)

### Palette Operations

#### `uint32_t func__palettecolor(int32_t n, int32_t i, int32_t passed)`
Gets the color value at a specific palette index.
- **n**: Palette index (0-255)
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: 32-bit color value

#### `void sub__palettecolor(int32_t n, uint32_t c, int32_t i, int32_t passed)`
Sets the color value at a specific palette index.
- **n**: Palette index (0-255)
- **c**: 32-bit color value to set
- **i**: Image handle (optional)
- **passed**: Parameter passing flags

#### `void sub__copypalette(int32_t src, int32_t dst, int32_t passed)`
Copies palette entries from source to destination.
- **src**: Source palette index
- **dst**: Destination palette index
- **passed**: Parameter passing flags

### Clear/Transparent Color

#### `void sub__clearcolor(uint32_t c, int32_t i, int32_t passed)`
Sets the clear/transparent color for drawing operations.
- **c**: 32-bit color value
- **i**: Image handle (optional)
- **passed**: Parameter passing flags

#### `int32_t func__clearcolor(int32_t i, int32_t passed)`
Gets the current clear/transparent color.
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: 32-bit color value

### Default Colors

#### `uint32_t func__defaultcolor(int32_t i, int32_t passed)`
Gets the default drawing color.
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: 32-bit color value

#### `uint32_t func__backgroundcolor(int32_t i, int32_t passed)`
Gets the default background color.
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: 32-bit color value

---

## Screen Management Module {#screen-management-module}

**Header**: `internal/c/libqb/include/screen.h`

The Screen Management module provides display control, fullscreen management, and screen dimension handling capabilities.

### Display Control

#### `void sub__display()`
Manually updates the display when autodisplay is disabled.

#### `void sub__autodisplay()`
Enables automatic display updates after drawing operations.

### Fullscreen Control

#### `void sub__fullscreen(int32_t method, int32_t passed)`
Sets fullscreen mode using specified method.
- **method**: Fullscreen transition method
- **passed**: Parameter passing flags

#### `int32_t func__fullscreen()`
Gets the current fullscreen state.
- **Returns**: Non-zero if in fullscreen mode

#### `int32_t func__fullscreensmooth()`
Gets the fullscreen smoothing state.
- **Returns**: Non-zero if smoothing is enabled

#### `void sub__allowfullscreen(int32_t method, int32_t smooth)`
Configures fullscreen behavior and smoothing.
- **method**: Allowed fullscreen methods
- **smooth**: Smoothing preference

### Resize Handling

#### `void sub__resize(int32_t on_off, int32_t stretch_smooth)`
Controls window resizing behavior.
- **on_off**: Enable/disable resizing
- **stretch_smooth**: Stretching and smoothing options

#### `int32_t func__resize()`
Gets the current resize state.
- **Returns**: Non-zero if resizing is enabled

#### `int32_t func__resizewidth()`
Gets the current window width.
- **Returns**: Window width in pixels

#### `int32_t func__resizeheight()`
Gets the current window height.
- **Returns**: Window height in pixels

### Scaled Dimensions

#### `int32_t func__scaledwidth()`
Gets the scaled display width.
- **Returns**: Scaled width in pixels

#### `int32_t func__scaledheight()`
Gets the scaled display height.
- **Returns**: Scaled height in pixels

### Screen Position

#### `int32_t func__screenx()`
Gets the screen X position.
- **Returns**: X coordinate in pixels

#### `int32_t func__screeny()`
Gets the screen Y position.
- **Returns**: Y coordinate in pixels

---

## Text & Font Module {#text--font-module}

**Header**: `internal/c/libqb/include/text.h`
**Source**: `internal/c/libqb/src/text.cpp` (2,121 lines)

The Text & Font module provides text output, cursor control, and font management functionality.

### Core Text Output

#### `void printchr(int32_t character)`
Renders a single character at the current cursor position.
- **character**: Character code to render (ASCII or Unicode)

#### `int32_t chrwidth(uint32_t character)`
Gets the width of a character in pixels.
- **character**: Character code to measure
- **Returns**: Width in pixels

#### `void newline()`
Advances cursor to beginning of next line, scrolling if necessary.

#### `void qbs_print(qbs *str, int32_t finish_on_new_line)`
Prints a string at the current cursor position.
- **str**: QB64 string to print
- **finish_on_new_line**: Non-zero to add newline after printing

#### `void tab()`
Advances cursor to next tab stop (every 14 characters).

#### `void makefit(qbs *text)`
Ensures text fits on current line, wrapping if necessary.

### Cursor Positioning

#### `void qbg_sub_locate(int32_t row, int32_t column, int32_t cursor, int32_t start, int32_t stop, int32_t passed)`
Sets cursor position and attributes (LOCATE statement).
- **row**: Target row (1-based)
- **column**: Target column (1-based)
- **cursor**: Cursor visibility (0=hide, 1=show)
- **start, stop**: Cursor shape (scanline range)
- **passed**: Parameter passing flags

#### `int32_t func_csrlin()`
Gets current cursor row (CSRLIN function).
- **Returns**: Current row number (1-based)

#### `int32_t func_pos(int32_t ignore)`
Gets current cursor column (POS function).
- **ignore**: Ignored parameter (for QB compatibility)
- **Returns**: Current column number (1-based)

#### `qbs *func_tab(int32_t pos)`
Generates spacing to reach specified column (TAB function).
- **pos**: Target column position
- **Returns**: String of spaces/newlines to reach position

#### `qbs *func_spc(int32_t spaces)`
Generates specified number of spaces (SPC function).
- **spaces**: Number of spaces to generate
- **Returns**: String of spaces

### Font Management

#### `int32_t func__loadfont(const qbs *filename, int32_t size, const qbs *requirements, int32_t font_index, int32_t passed)`
Loads a TrueType font file (_LOADFONT).
- **filename**: Path to font file (or font data if MEMORY option)
- **size**: Font height in pixels
- **requirements**: Options string (MONOSPACE, UNICODE, DONTBLEND, MEMORY, AUTOMONO)
- **font_index**: Font face index for multi-font files
- **passed**: Parameter passing flags
- **Returns**: Font handle (>=32), or 0 on failure

#### `void sub__font(int32_t f, int32_t i, int32_t passed)`
Sets the active font for an image (_FONT).
- **f**: Font handle (8, 14, 16 for built-in, >=32 for loaded)
- **i**: Image handle (optional)
- **passed**: Parameter passing flags

#### `int32_t func__fontwidth(int32_t f, int32_t passed)`
Gets font width in pixels (_FONTWIDTH).
- **f**: Font handle (optional, uses current font if not passed)
- **passed**: Parameter passing flags
- **Returns**: Width in pixels (0 for variable-width fonts)

#### `int32_t func__fontheight(int32_t f, int32_t passed)`
Gets font height in pixels (_FONTHEIGHT).
- **f**: Font handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Height in pixels

#### `int32_t func__font(int32_t i, int32_t passed)`
Gets the font handle for an image (_FONT query).
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Current font handle

#### `void sub__freefont(int32_t f)`
Releases a loaded font (_FREEFONT).
- **f**: Font handle to free

### Print Modes

#### `void sub__printmode(int32_t mode, int32_t i, int32_t passed)`
Sets the print mode for text rendering (_PRINTMODE).
- **mode**: 1=_FILLBACKGROUND, 2=_KEEPBACKGROUND, 3=_ONLYBACKGROUND
- **i**: Image handle (optional)
- **passed**: Parameter passing flags

#### `int32_t func__printmode(int32_t i, int32_t passed)`
Gets the current print mode (_PRINTMODE query).
- **i**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Current print mode (1, 2, or 3)

#### `void sub__printstring(float x, float y, qbs *text, int32_t i, int32_t passed)`
Prints text at specific pixel coordinates (_PRINTSTRING).
- **x, y**: Pixel coordinates
- **text**: String to print
- **i**: Image handle (optional)
- **passed**: Parameter passing flags

#### `int32_t func__printwidth(qbs *text, int32_t screenhandle, int32_t passed)`
Gets the width of text in pixels (_PRINTWIDTH).
- **text**: String to measure
- **screenhandle**: Image handle (optional)
- **passed**: Parameter passing flags
- **Returns**: Width in pixels

### View Print

#### `void qbg_sub_view_print(int32_t topline, int32_t bottomline, int32_t passed)`
Sets the text viewport (VIEW PRINT statement).
- **topline**: First row of viewport
- **bottomline**: Last row of viewport
- **passed**: Parameter passing flags

### Clear Screen

#### `void sub_cls(int32_t method, uint32_t use_color, int32_t passed)`
Clears the screen or viewport (CLS statement).
- **method**: 0=all, 1=graphics viewport, 2=text viewport
- **use_color**: Color to use for clearing
- **passed**: Parameter passing flags

#### `void sub_clsDest(int32_t method, uint32_t use_color, int32_t dest, int32_t passed)`
Clears a specific destination image.
- **dest**: Destination image handle
- Other parameters same as sub_cls

### LPRINT Support

#### `void qbs_lprint(qbs *str, int32_t finish_on_new_line)`
Prints to the printer (LPRINT statement).
- **str**: String to print
- **finish_on_new_line**: Non-zero to add newline

#### `int32_t func_lpos(int32_t lpt)`
Gets the printer column position (LPOS function).
- **lpt**: Printer number (0-3)
- **Returns**: Current column position

#### `void lprint_makefit(qbs *text)`
Ensures text fits on printer line, wrapping if necessary (internal helper for LPRINT).
- **text**: Text to format for printer

### Internal Helpers

#### `int32_t selectfont(int32_t f, img_struct *im)`
Selects and activates a font for an image (internal helper function).
- **f**: Font handle
- **im**: Image structure pointer
- **Returns**: Success status

### Global Variables

#### `int32_t no_control_characters`
Global flag controlling control character interpretation (0=interpret, 1=print as-is).

---

## Graphics Module {#graphics-module}

**Header**: `internal/c/libqb/include/graphics.h`
**Source**: `internal/c/libqb/src/graphics.cpp` (7,589 lines)

The Graphics module provides drawing primitives, image management, and hardware rendering support.

### Drawing Primitives

#### `void sub_line(float x1, float y1, float x2, float y2, uint32_t col, int32_t bf, uint32_t style, int32_t passed)`
Draws a line or box (LINE statement).
- **x1, y1**: Start point (or first corner)
- **x2, y2**: End point (or second corner)
- **col**: Color value
- **bf**: Box flag (B=box, BF=filled box)
- **style**: Line style pattern
- **passed**: Parameter passing flags

#### `void sub_circle(double x, double y, double r, uint32_t col, double start, double end, double aspect, int32_t passed)`
Draws a circle or arc (CIRCLE statement).
- **x, y**: Center coordinates
- **r**: Radius
- **col**: Color value
- **start, end**: Arc angles (radians)
- **aspect**: Aspect ratio
- **passed**: Parameter passing flags

#### `void sub_pset(float x, float y, uint32_t col, int32_t passed)`
Sets a pixel color (PSET statement).
- **x, y**: Pixel coordinates
- **col**: Color value
- **passed**: Parameter passing flags

#### `void sub_preset(float x, float y, uint32_t col, int32_t passed)`
Resets a pixel to background color (PRESET statement).

#### `void sub_paint(float x, float y, uint32_t fillcol, uint32_t bordercol, qbs *backgroundstr, int32_t passed)`
Fills an area with color (PAINT statement).
- **x, y**: Start point
- **fillcol**: Fill color
- **bordercol**: Border color (stops filling)
- **backgroundstr**: Fill pattern (optional)
- **passed**: Parameter passing flags

#### `double func_point(float x, float y, int32_t passed)`
Gets pixel color or coordinates (POINT function).
- **x, y**: Coordinates or attribute selector
- **passed**: Parameter passing flags
- **Returns**: Color value or coordinate

#### `void sub_draw(qbs *s)`
Executes DRAW command string (turtle graphics).
- **s**: DRAW command string

### Image Management

#### `int32_t func__newimage(int32_t x, int32_t y, int32_t bpp, int32_t passed)`
Creates a new image (_NEWIMAGE).
- **x, y**: Image dimensions
- **bpp**: Bits per pixel (0, 1, 2, 4, 8, 32, 256)
- **passed**: Parameter passing flags
- **Returns**: Image handle, or 0 on error

#### `int32_t func__copyimage(int32_t i, int32_t mode, int32_t passed)`
Creates a copy of an image (_COPYIMAGE).
- **i**: Source image handle
- **mode**: Copy mode (0=software, 1=hardware)
- **passed**: Parameter passing flags
- **Returns**: New image handle

#### `void sub__freeimage(int32_t i, int32_t passed)`
Frees an image (_FREEIMAGE).
- **i**: Image handle to free
- **passed**: Parameter passing flags

#### `void freeallimages()`
Frees all images and releases all image resources (cleanup function).
Called automatically during program shutdown.

#### `void sub__source(int32_t i)`
Sets the source image for operations (_SOURCE).
- **i**: Source image handle

#### `void sub__dest(int32_t i)`
Sets the destination image for drawing (_DEST).
- **i**: Destination image handle

#### `int32_t func__source()`
Gets the current source image handle.

#### `int32_t func__dest()`
Gets the current destination image handle.

#### `int32_t func__display()`
Gets the display image handle.

#### `int32_t func__width(int32_t i, int32_t passed)`
Gets image width (_WIDTH).

#### `int32_t func__height(int32_t i, int32_t passed)`
Gets image height (_HEIGHT).

#### `int32_t func__pixelsize(int32_t i, int32_t passed)`
Gets bits per pixel (_PIXELSIZE).

### Alpha Blending

#### `void sub__blend(int32_t i, int32_t passed)`
Enables alpha blending (_BLEND).

#### `void sub__dontblend(int32_t i, int32_t passed)`
Disables alpha blending (_DONTBLEND).

#### `int32_t func__blend(int32_t i, int32_t passed)`
Checks if blending is enabled.

#### `void sub__setalpha(int32_t a, uint32_t c, uint32_t c2, int32_t i, int32_t passed)`
Sets alpha values for colors (_SETALPHA).

### Image Operations

#### `void sub__putimage(double f_dx1, double f_dy1, double f_dx2, double f_dy2, int32_t src, int32_t dst, double f_sx1, double f_sy1, double f_sx2, double f_sy2, int32_t passed)`
Copies image data between surfaces (_PUTIMAGE).
- **f_dx1, f_dy1, f_dx2, f_dy2**: Destination rectangle
- **src**: Source image handle
- **dst**: Destination image handle
- **f_sx1, f_sy1, f_sx2, f_sy2**: Source rectangle
- **passed**: Parameter passing flags

#### `void sub_graphics_get(float x1f, float y1f, float x2f, float y2f, void *element, uint32_t mask, int32_t passed)`
Captures screen area to array (GET statement).

#### `void sub_graphics_put(float x1f, float y1f, void *element, int32_t option, uint32_t mask, int32_t passed)`
Draws array data to screen (PUT statement).

### HSB Color Functions

#### `uint32_t func__hsb32(double hue, double sat, double bri)`
Creates RGB from HSB values (_HSB32).

#### `double func__hue32(uint32_t argb)`
Extracts hue from color (_HUE32).

#### `double func__sat32(uint32_t argb)`
Extracts saturation (_SAT32).

#### `double func__bri32(uint32_t argb)`
Extracts brightness (_BRI32).

#### `uint32_t func__hsba32(double hue, double sat, double bri, double alf)`
Creates an RGBA color from HSB values with alpha channel (_HSBA32).
- **hue**: Hue value (0-360 degrees)
- **sat**: Saturation (0.0-1.0)
- **bri**: Brightness (0.0-1.0)
- **alf**: Alpha channel (0.0-1.0)
- **Returns**: 32-bit RGBA color value

### 3D Graphics

#### `void sub__depthbuffer(int32_t options, int32_t dst, int32_t passed)`
Controls depth buffer settings (_DEPTHBUFFER).

#### `void sub__maptriangle(...)`
Maps textured triangles for 3D rendering (_MAPTRIANGLE).

---

## Networking Module {#networking-module}

**Header**: `internal/c/libqb/include/networking.h`
**Source**: `internal/c/libqb/src/networking.cpp` (894 lines)

The Networking module provides TCP/IP socket functionality.

### Initialization

#### `void networking_init()`
Initializes the networking subsystem.

#### `void tcp_init()`
Initializes TCP/IP (called automatically).

#### `void tcp_done()`
Cleans up TCP/IP resources.

### Connection Functions

#### `int32_t func__openclient(qbs *info)`
Opens a client connection (_OPENCLIENT).
- **info**: Connection string (e.g., "TCP/IP:12345:host.example.com")
- **Returns**: Negative handle on success, 0 on failure

#### `int32_t func__openhost(qbs *info)`
Opens a listening server (_OPENHOST).
- **info**: Host string (e.g., "TCP/IP:12345")
- **Returns**: Negative handle on success, 0 on failure

#### `int32_t func__openconnection(int32_t i)`
Accepts a connection from a host (_OPENCONNECTION).
- **i**: Host handle
- **Returns**: Negative handle on success, 0 if no pending connections

#### `int32_t func__connected(int32_t i)`
Checks if connection is active (_CONNECTED).
- **i**: Connection handle
- **Returns**: -1 if connected, 0 if disconnected

#### `qbs *func__connectionaddress(int32_t i)`
Gets connection address info (_CONNECTIONADDRESS$).
- **i**: Connection handle
- **Returns**: Address information string

### Internal Functions

#### `int32_t connection_new(int32_t method, qbs *info, int32_t value)`
Creates a new network connection (internal helper).
- **method**: Connection method (0=_OPENCLIENT, 1=_OPENHOST, 2=_OPENCONNECTION)
- **info**: Connection info string
- **value**: Host handle for _OPENCONNECTION method
- **Returns**: Handle (>0), 0 on failure, -1 on invalid arguments

#### `int32_t tcp_connected(void *connection)`
Checks if a TCP connection is active (internal helper).
- **connection**: TCP connection pointer
- **Returns**: Non-zero if connected

---

## Keyboard Module {#keyboard-module}

**Header**: `internal/c/libqb/include/keyboard.h`
**Source**: `internal/c/libqb/src/keyboard.cpp` (177 lines)

The Keyboard module provides keyboard input and lock key control.

### Lock Key Queries

#### `int32_t func__capslock()`
Checks Caps Lock state (_CAPSLOCK query).
- **Returns**: -1 if on, 0 if off

#### `int32_t func__scrolllock()`
Checks Scroll Lock state (_SCROLLLOCK query).

#### `int32_t func__numlock()`
Checks Num Lock state (_NUMLOCK query).

### Lock Key Control

#### `void sub__capslock(int32_t options)`
Controls Caps Lock (_CAPSLOCK).
- **options**: 1=ON, 2=OFF, 3=TOGGLE

#### `void sub__scrolllock(int32_t options)`
Controls Scroll Lock (_SCROLLLOCK).

#### `void sub__numlock(int32_t options)`
Controls Num Lock (_NUMLOCK).

### Keyboard Input

#### `int32_t func__keyhit()`
Gets next key from buffer (_KEYHIT).
- **Returns**: Key code, or 0 if buffer empty

#### `int32_t func__keydown(int32_t x)`
Checks if key is pressed (_KEYDOWN).
- **x**: Key code to check
- **Returns**: -1 if pressed, 0 otherwise

### Unicode Mapping

#### `void sub__mapunicode(int32_t unicode_code, int32_t ascii_code)`
Maps Unicode to ASCII (_MAPUNICODE statement).

#### `int32_t func__mapunicode(int32_t ascii_code)`
Gets Unicode for ASCII (_MAPUNICODE function).

---

## Mouse Module {#mouse-module}

**Header**: `internal/c/libqb/include/mouse.h`
**Source**: `internal/c/libqb/src/mouse.cpp` (355 lines)

The Mouse module provides mouse input and cursor control.

### Cursor Visibility

#### `void sub__mousehide()`
Hides mouse cursor (_MOUSEHIDE).

#### `void sub__mouseshow(qbs *style, int32_t passed)`
Shows mouse cursor (_MOUSESHOW).
- **style**: Cursor style (optional)
- **passed**: Parameter passing flags

#### `int32_t func__mousehidden()`
Checks if cursor is hidden (_MOUSEHIDDEN).

### Position Functions

#### `float func__mousex()`
Gets mouse X coordinate (_MOUSEX).

#### `float func__mousey()`
Gets mouse Y coordinate (_MOUSEY).

#### `float func__mousemovementx()`
Gets X movement delta (_MOUSEMOVEMENTX).

#### `float func__mousemovementy()`
Gets Y movement delta (_MOUSEMOVEMENTY).

#### `void sub__mousemove(float x, float y)`
Moves cursor to position (_MOUSEMOVE).

### Input Functions

#### `int32_t func__mouseinput()`
Polls for mouse events (_MOUSEINPUT).
- **Returns**: -1 if event available, 0 otherwise

#### `int32_t func__mousebutton(int32_t i)`
Checks button state (_MOUSEBUTTON).
- **i**: Button number (1=left, 2=right, 3=middle)
- **Returns**: -1 if pressed, 0 otherwise

#### `int32_t func__mousewheel()`
Gets wheel movement (_MOUSEWHEEL).
- **Returns**: Wheel delta (positive=up, negative=down)

---

## Platform Module {#platform-module}

**Header**: `internal/c/libqb/include/platform.h`
**Source**: `internal/c/libqb/src/platform.cpp` (870 lines)

The Platform module provides platform-specific functionality for keyboard input simulation and other platform-dependent operations.

### Keyboard Input Simulation

#### `void sub__screenprint(qbs *txt)`
Simulates keyboard input to send text to the active window (_SCREENPRINT).
- **txt**: Text string to send as keyboard input
- **Platform support**: 
  - Windows: Uses SendInput API to inject keystrokes
  - macOS: Uses CGEvent API with virtual key codes
  - Linux: Not currently implemented

**Note**: This function sends keystrokes to the currently active window, which may be outside the QB64 program window. Use with caution as it can interfere with other applications.

---

## Utility Module {#utility-module}

**Header**: `internal/c/libqb/include/utility.h`
**Source**: `internal/c/libqb/src/utility.cpp` (173 lines)

The Utility module provides miscellaneous utility functions.

### Random Number Generation

#### `void sub_randomize(double seed, int32_t passed)`
Seeds random number generator (RANDOMIZE).
- **seed**: Seed value
- **passed**: Parameter passing flags

#### `float func_rnd(float n, int32_t passed)`
Generates random number (RND).
- **n**: Control value (n<0: seed, n=0: repeat, n>0: next)
- **passed**: Parameter passing flags
- **Returns**: Random value 0.0 to 1.0

### Frame Rate Control

#### `void sub__fps(double fps, int32_t passed)`
Sets frame rate limit (_FPS).
- **fps**: Target frames per second
- **passed**: 1 for _AUTO mode, 2 for specific fps

### Blink Control

#### `int32_t func__blink()`
Gets text blink state (_BLINK query).
- **Returns**: -1 if on, 0 if off

#### `void sub__blink(int32_t onoff)`
Sets text blink mode (_BLINK).

### Control Characters

#### `void sub__controlchr(int32_t onoff)`
Controls character interpretation (_CONTROLCHR).
- **onoff**: 1=ON (interpret), 0=OFF (print as-is)

#### `int32_t func__controlchr()`
Gets control character mode (_CONTROLCHR query).
- **Returns**: Non-zero if control characters are enabled, 0 if disabled

### Internal Helper Functions

These functions are used internally by other modules and are not part of the public API:

#### `double get_max_fps()`
Gets the maximum frame rate limit.
- **Returns**: Maximum FPS value

#### `int32_t get_auto_fps()`
Gets the auto FPS mode state.
- **Returns**: Non-zero if auto FPS is enabled

#### `void set_auto_fps(int32_t value)`
Sets the auto FPS mode state.
- **value**: Non-zero to enable, 0 to disable

#### `void reset_rnd_state()`
Resets the random number generator state (used by RUN command).

#### `int32_t get_control_characters_disabled()`
Gets the control character disabled state (used by text module).
- **Returns**: Non-zero if control characters are disabled

---

## Window Module {#window-module}

**Header**: `internal/c/libqb/include/window.h`
**Source**: `internal/c/libqb/src/window.cpp` (77 lines)

The Window module provides window handle and focus functions.

#### `int64_t func__handle()`
Gets native window handle (_HANDLE).
- **Returns**: HWND on Windows, 0 on other platforms

#### `qbs *func__title()`
Gets window title (_TITLE$ query).
- **Returns**: Current title string

#### `int32_t func__hasfocus()`
Checks if window has focus (_HASFOCUS).
- **Returns**: -1 if focused, 0 otherwise

### Internal Helper Functions

#### `void set_foreground_window(intptr_t handle)`
Brings a window to the foreground (Windows platform only).
- **handle**: Window handle (HWND on Windows)
- **Platform**: Windows only, no-op on other platforms

---

## Console Module {#console-module}

**Header**: `internal/c/libqb/include/console.h`
**Source**: `internal/c/libqb/src/console.cpp` (171 lines)

The Console module provides console window management.

### Window Control

#### `int32_t func__console()`
Gets console state (_CONSOLE query).
- **Returns**: 1=visible, 0=hidden, -1=no console

#### `void sub__console(int32_t onoff)`
Shows/hides console (_CONSOLE).

#### `void sub__consoletitle(qbs *s)`
Sets console title (_CONSOLETITLE).

#### `void sub__consolefont(qbs *FontName, int32_t FontSize)`
Sets console font (_CONSOLEFONT).

#### `void sub__console_cursor(int32_t visible, int32_t cursorsize, int32_t passed)`
Controls console cursor (_CONSOLECURSOR).

### Console Input

#### `int32_t func__getconsoleinput()`
Polls for console input (_GETCONSOLEINPUT).

#### `int32_t func__cinp(int32_t toggle, int32_t passed)`
Gets console character input (_CINP).

---

## Port I/O Module {#port-io-module}

**Header**: `internal/c/libqb/include/port_io.h`
**Source**: `internal/c/libqb/src/port_io.cpp` (249 lines)

The Port I/O module provides legacy port access emulation.

#### `void sub_out(int32_t port, int32_t data)`
Writes to I/O port (OUT statement).
- **port**: Port address (0x3C0-0x3C9 for VGA palette)
- **data**: Byte value to write

#### `int32_t func_inp(int32_t port)`
Reads from I/O port (INP function).
- **port**: Port address (0x3C9=palette, 0x3DA=retrace, 0x60=keyboard)
- **Returns**: Byte value read

#### `void sub_wait(int32_t port, int32_t andexpression, int32_t xorexpression, int32_t passed)`
Waits for port condition (WAIT statement).

---

## Legacy Memory Module {#legacy-memory-module}

**Header**: `internal/c/libqb/include/mem_legacy.h`
**Source**: `internal/c/libqb/src/mem_legacy.cpp` (61 lines)

The Legacy Memory module provides PEEK/POKE memory access.

#### `void sub_defseg(int32_t segment, int32_t passed)`
Sets memory segment (DEF SEG).
- **segment**: Segment address
- **passed**: Parameter passing flags

#### `int32_t func_peek(int32_t offset)`
Reads byte from memory (PEEK).
- **offset**: Memory offset
- **Returns**: Byte value

#### `void sub_poke(int32_t offset, int32_t value)`
Writes byte to memory (POKE).
- **offset**: Memory offset
- **value**: Byte value to write

---

## State Accessor Module {#state-accessor-module}

**Header**: `internal/c/libqb/include/libqb_state.h`
**Source**: `internal/c/libqb/src/libqb_state.cpp` (168 lines)

The State Accessor module provides controlled access to global state, enabling modularization.

### Image Accessors

#### `img_struct* libqb_get_write_page()`
Gets the current write page pointer.

#### `img_struct* libqb_get_read_page()`
Gets the current read page pointer.

#### `img_struct* libqb_get_display_page()`
Gets the current display page pointer.

#### `img_struct* libqb_get_image(int32_t handle)`
Gets image by handle.

### Page Index Accessors

#### `int32_t libqb_get_write_page_index()`
Gets write page index.

#### `void libqb_set_write_page_index(int32_t index)`
Sets write page index.

#### `int32_t libqb_get_read_page_index()`
Gets read page index.

#### `int32_t libqb_get_display_page_index()`
Gets display page index.

### Font Accessors

#### `int32_t libqb_get_font_width(int32_t font_handle)`
Gets font width for handle.

#### `int32_t libqb_get_font_height(int32_t font_handle)`
Gets font height for handle.

#### `int32_t libqb_get_font_flags(int32_t font_handle)`
Gets font flags for handle.

### Screen Dimension Accessors

#### `int32_t libqb_get_screen_width()`
Gets screen width in pixels.

#### `int32_t libqb_get_screen_height()`
Gets screen height in pixels.

---

## Usage Guidelines

### Parameter Passing Convention
Most functions follow the QB64 parameter passing convention:
- `passed` parameter indicates which optional parameters were provided
- Image handle parameters (`i`) are typically optional for operations on the current screen

### Error Handling
- File operations return status codes (0 for failure, non-zero for success)
- Color functions typically return 0 for invalid inputs
- Screen functions may return -1 for error conditions

### Thread Safety
These modules are designed for single-threaded use in the QB64 environment. Concurrent access to file handles or screen operations may cause undefined behavior.

### Memory Management
- String parameters (`qbs *`) are managed by the QB64 runtime
- Color values are 32-bit integers in RGBA format
- File positions use 64-bit integers for large file support

---

## Future Development

### Planned Enhancements
- Additional file format support in fileio module
- Extended color space support in color module
- Multi-monitor support in screen module

### Migration Notes
Some functions remain in `libqb.cpp` and will be migrated to appropriate modules in future releases:
- File print/input operations
- Advanced file I/O operations
- Extended screen manipulation functions

---

*This documentation is maintained as part of the QB64-PE modularization effort. For the most current information, check the source headers directly.*
