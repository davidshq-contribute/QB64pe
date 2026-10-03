# MIGRATION.md

Guide for users migrating from QBasic/QuickBASIC 4.5 to QB64 Phoenix Edition.

## Overview

QB64pe is designed to be highly compatible with QBasic and QB4.5. Most programs will compile and run without modification. This guide covers the differences and new features available.

## What Works Unchanged

These QBasic features work identically in QB64pe:

- All standard statements: IF, FOR, DO, SELECT CASE, GOSUB, GOTO
- SUB and FUNCTION definitions
- DIM, REDIM, SHARED, STATIC, COMMON
- TYPE...END TYPE user-defined types
- All numeric types: INTEGER, LONG, SINGLE, DOUBLE
- Fixed and variable-length strings
- Arrays (single and multi-dimensional)
- DATA, READ, RESTORE
- All standard math functions
- All string functions
- PRINT, INPUT, LINE INPUT
- File I/O: OPEN, CLOSE, GET, PUT, INPUT#, PRINT#
- SCREEN modes 0-13
- Graphics: LINE, CIRCLE, PSET, PAINT, GET, PUT
- COLOR, PALETTE
- SOUND, PLAY
- Basic keyboard input: INKEY$, INPUT$

## Key Differences

### Screen Modes

**QBasic:** Limited to modes 0-13, VGA resolution maximum.

**QB64pe:** All legacy modes work, plus modern high-resolution modes:

```basic
' Legacy (works same as QBasic)
SCREEN 12                         ' 640x480, 16 colors
SCREEN 13                         ' 320x200, 256 colors

' Modern (QB64pe extension)
SCREEN _NEWIMAGE(1920, 1080, 32)  ' Full HD, 32-bit color
SCREEN _NEWIMAGE(800, 600, 256)   ' 800x600, 256 colors
```

### Colors

**QBasic:** Palette-based, 16 or 256 colors maximum.

**QB64pe:** Full 32-bit RGBA color support:

```basic
' QBasic style (still works)
COLOR 14, 1                       ' Yellow on blue

' QB64pe 32-bit (new)
COLOR _RGB(255, 255, 0), _RGB(0, 0, 255)
LINE (0, 0)-(100, 100), _RGBA(255, 0, 0, 128)  ' Semi-transparent
```

### Memory Limits

**QBasic:** 64KB string space, ~160KB total memory, 32,767 array elements per dimension.

**QB64pe:** Limited only by system RAM:

```basic
' This would crash QBasic, works fine in QB64pe
DIM bigArray(1000000) AS LONG
DIM bigString AS STRING
bigString = SPACE$(10000000)      ' 10MB string
```

### Data Types

**QBasic:** INTEGER, LONG, SINGLE, DOUBLE, STRING.

**QB64pe adds:**

| New Type | Description |
|----------|-------------|
| `_BYTE` | 8-bit signed (-128 to 127) |
| `_INTEGER64` | 64-bit signed |
| `_FLOAT` | Extended precision (80+ bits) |
| `_OFFSET` | Pointer-sized integer |
| `_BIT` | Bit fields (1-64 bits) |
| `_UNSIGNED` | Unsigned variant prefix |

```basic
DIM b AS _BYTE
DIM i64 AS _INTEGER64
DIM ptr AS _OFFSET
DIM bits AS _BIT * 4              ' 4-bit field
DIM ub AS _UNSIGNED _BYTE         ' 0 to 255
```

### Timing

**QBasic:** TIMER returns seconds, SLEEP pauses in whole seconds.

**QB64pe:** Sub-millisecond precision:

```basic
' QBasic style
t! = TIMER                        ' Whole seconds

' QB64pe precision
t# = TIMER(.001)                  ' Milliseconds
_DELAY 0.016                      ' 16ms delay (fractional)
_LIMIT 60                         ' Cap loop to 60/sec
```

### Keyboard Input

**QBasic:** INKEY$ only, blocking INPUT.

**QB64pe adds non-blocking options:**

```basic
' QBasic style
k$ = INKEY$

' QB64pe extended
k& = _KEYHIT                      ' Returns key code, non-blocking
IF _KEYDOWN(32) THEN              ' Check if spacebar is held
```

### Mouse Support

**QBasic:** Required CALL INTERRUPT or library.

**QB64pe:** Built-in mouse functions:

```basic
DO WHILE _MOUSEINPUT: LOOP        ' Update mouse state
x% = _MOUSEX
y% = _MOUSEY
IF _MOUSEBUTTON(1) THEN           ' Left click
wheel% = _MOUSEWHEEL              ' Scroll wheel
```

### File Operations

**QBasic:** OPEN, CLOSE, basic I/O.

**QB64pe adds convenience functions:**

```basic
' QBasic style (still works)
OPEN "file.txt" FOR INPUT AS #1
' ... read line by line
CLOSE #1

' QB64pe one-liner
contents$ = _READFILE$("file.txt")
_WRITEFILE "output.txt", data$

' File existence check
IF _FILEEXISTS("config.ini") THEN
IF _DIREXISTS("data") THEN
```

### Image Support

**QBasic:** No image file loading.

**QB64pe:** Load PNG, JPG, BMP, GIF, etc.:

```basic
img& = _LOADIMAGE("picture.png", 32)
_PUTIMAGE (0, 0), img&, 0
_FREEIMAGE img&
```

### Sound and Music

**QBasic:** SOUND (beeps), PLAY (MML strings).

**QB64pe adds:** Full audio file playback:

```basic
' QBasic style (still works)
SOUND 440, 18                     ' 440Hz for 1 second
PLAY "O4 C D E F G"               ' Musical notes

' QB64pe audio files
handle& = _SNDOPEN("music.mp3")
_SNDPLAY handle&
_SNDLOOP handle&                  ' Loop playback
_SNDVOL handle&, 0.5              ' 50% volume
_SNDCLOSE handle&
```

## PEEK/POKE and Memory Access

**QBasic:** Direct memory access via DEF SEG, PEEK, POKE.

**QB64pe:** Emulated for compatibility, but prefer `_MEM` system:

```basic
' QBasic style (emulated, still works for most cases)
DEF SEG = &HB800
POKE 0, 65                        ' Write 'A' to screen memory

' QB64pe modern approach (recommended)
DIM m AS _MEM
DIM value AS LONG
m = _MEM(value)
_MEMPUT m, m.OFFSET, 12345 AS LONG
_MEMFREE m
```

### What's Different with PEEK/POKE

- Video memory (&HB800, &HA000) is emulated but not real
- Interrupt calls (CALL INTERRUPT) are partially emulated
- CALL ABSOLUTE does not work (machine code not supported)
- INP/OUT port I/O is mostly stubbed

## Source Code Changes Needed

### Line Numbers

**Optional in both**, but QB64pe doesn't require them:

```basic
' QBasic style with line numbers
10 PRINT "Hello"
20 GOTO 10

' Modern style (both support this)
DO
    PRINT "Hello"
LOOP
```

### DECLARE Statements

**QBasic:** Required for forward references.

**QB64pe:** Optional, forward references resolved automatically:

```basic
' QBasic required this
DECLARE SUB MySub (x AS INTEGER)

' QB64pe: just call it
MySub 5

SUB MySub (x AS INTEGER)
    PRINT x
END SUB
```

### $INCLUDE Files

**Same syntax**, but QB64pe searches additional paths:

```basic
'$INCLUDE: 'mylib.bi'
```

QB64pe searches: current directory, then `internal/support/` subdirectories.

## New Features to Explore

### Graphics Enhancements

```basic
' Hardware-accelerated images
hw& = _COPYIMAGE(img&, 33)        ' Create hardware image

' Scaling and rotation with _MAPTRIANGLE
' Alpha blending automatic in 32-bit mode
' _PUTIMAGE with scaling
_PUTIMAGE (0, 0)-(799, 599), img&, 0
```

### Networking

```basic
' TCP client
client& = _OPENCLIENT("TCP/IP:80:example.com")
PUT #client&, , request$
GET #client&, , response$
CLOSE client&

' TCP server
host& = _OPENHOST("TCP/IP:8080")
```

### Console Mode

```basic
$CONSOLE:ONLY                     ' No graphics window
_CONSOLE ON                       ' Show console alongside graphics
```

### Clipboard

```basic
_CLIPBOARD$ = "Copy this"         ' Write to clipboard
text$ = _CLIPBOARD$               ' Read from clipboard
```

### Threading (Limited)

```basic
' Display updates can run independently
_AUTODISPLAY                      ' Automatic screen refresh
_DISPLAY                          ' Manual refresh
```

## Compatibility Flags

For maximum QBasic compatibility:

```basic
OPTION _EXPLICIT                  ' Require variable declaration
OPTION BASE 1                     ' Arrays start at 1
DEFINT A-Z                        ' Default to INTEGER
```

## Programs That Won't Work

1. **Machine code** via CALL ABSOLUTE
2. **Direct hardware access** via INP/OUT (mostly stubbed)
3. **TSR programs** and interrupt handlers
4. **DOS-specific** file operations (DEV devices, etc.)
5. **EGA/VGA register manipulation** via port I/O

## Migration Checklist

- [ ] Replace CALL ABSOLUTE with SUB/FUNCTION
- [ ] Replace CALL INTERRUPT with built-in functions
- [ ] Replace PEEK/POKE screen writes with graphics commands
- [ ] Update timing code (TIMER precision, _DELAY vs SLEEP)
- [ ] Consider upgrading to 32-bit graphics
- [ ] Test memory-intensive operations (no 64K limit)
- [ ] Use _MEM instead of PEEK/POKE for new code
- [ ] Add _LIMIT to game loops for CPU efficiency

## Getting Help

- QB64pe Wiki: https://qb64phoenix.com/qb64wiki/
- QB64pe Forum: https://qb64phoenix.com/forum/
- GitHub Issues: https://github.com/QB64-Phoenix-Edition/QB64pe/issues

## Quick Comparison

| Feature | QBasic | QB64pe |
|---------|--------|--------|
| Max resolution | 640x480 | Unlimited |
| Colors | 256 | 16.7M (32-bit) |
| String length | ~32KB | ~2GB |
| Array elements | ~32K/dim | Billions |
| Sound | Beeps, MML | MP3, WAV, OGG, etc. |
| Images | None | PNG, JPG, BMP, etc. |
| Mouse | Library needed | Built-in |
| Networking | None | TCP/UDP built-in |
| Platform | DOS | Windows, Linux, macOS |
| Memory model | 16-bit segmented | Flat 64-bit |
| Timing precision | ~55ms | <1ms |
