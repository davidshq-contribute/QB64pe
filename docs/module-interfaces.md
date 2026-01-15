# QB64-PE Module Interface Documentation

This document describes the public interfaces for completed libqb modules, providing contributors with clear understanding of available functions and their usage.

## Table of Contents

- [File I/O Module (`fileio.h`)](#file-io-module)
- [Color & Palette Module (`color.h`)](#color--palette-module)
- [Screen Management Module (`screen.h`)](#screen-management-module)

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
