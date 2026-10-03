# QUICK-REFERENCE.md

One-page cheat sheet for QB64 Phoenix Edition.

## Type Suffixes

| Suffix | Type | Bits | Range |
|--------|------|------|-------|
| `%%` | _BYTE | 8 | -128 to 127 |
| `~%%` | _UNSIGNED _BYTE | 8 | 0 to 255 |
| `%` | INTEGER | 16 | -32768 to 32767 |
| `~%` | _UNSIGNED INTEGER | 16 | 0 to 65535 |
| `&` | LONG | 32 | -2B to 2B |
| `~&` | _UNSIGNED LONG | 32 | 0 to 4B |
| `&&` | _INTEGER64 | 64 | -9E18 to 9E18 |
| `~&&` | _UNSIGNED _INTEGER64 | 64 | 0 to 18E18 |
| `!` | SINGLE | 32 | 7 digits |
| `#` | DOUBLE | 64 | 15 digits |
| `##` | _FLOAT | 80+ | 18+ digits |
| `$` | STRING | var | Text |
| `%&` | _OFFSET | 32/64 | Pointer |

## Common Screen Modes

```basic
SCREEN 0                          ' Text mode (default)
SCREEN 12                         ' 640x480, 16 colors
SCREEN 13                         ' 320x200, 256 colors
SCREEN _NEWIMAGE(800, 600, 32)    ' Custom 32-bit color
SCREEN _NEWIMAGE(80, 25, 0)       ' Custom text mode
```

## Graphics Basics

```basic
' Drawing
PSET (x, y), color                ' Single pixel
LINE (x1, y1)-(x2, y2), color     ' Line
LINE (x1, y1)-(x2, y2), color, B  ' Box outline
LINE (x1, y1)-(x2, y2), color, BF ' Filled box
CIRCLE (x, y), radius, color      ' Circle
PAINT (x, y), fill, border        ' Flood fill

' Colors (32-bit mode)
c& = _RGB(255, 0, 0)              ' Red
c& = _RGBA(255, 0, 0, 128)        ' Semi-transparent red
r% = _RED(c&): g% = _GREEN(c&)    ' Extract components

' Images
img& = _LOADIMAGE("file.png", 32)
_PUTIMAGE (0, 0), img&, 0         ' Draw image
_FREEIMAGE img&                    ' Release memory
```

## File I/O

```basic
' Text files
OPEN "file.txt" FOR OUTPUT AS #1
PRINT #1, "Hello"
CLOSE #1

OPEN "file.txt" FOR INPUT AS #1
LINE INPUT #1, a$
CLOSE #1

' Binary files
OPEN "file.bin" FOR BINARY AS #1
PUT #1, , value&
GET #1, , value&
CLOSE #1

' Quick read/write (QB64pe extensions)
text$ = _READFILE$("file.txt")
_WRITEFILE "file.txt", text$
```

## Arrays

```basic
DIM arr(10) AS INTEGER            ' 0 to 10 (11 elements)
DIM arr(1 TO 100) AS LONG         ' 1 to 100
DIM arr(10, 20) AS SINGLE         ' 2D array
REDIM arr(n) AS STRING            ' Dynamic sizing
REDIM _PRESERVE arr(n*2) AS STRING ' Keep contents
ERASE arr                         ' Clear/deallocate
```

## User Types

```basic
TYPE PlayerType
    name AS STRING * 20
    x AS SINGLE
    y AS SINGLE
    health AS INTEGER
END TYPE

DIM player AS PlayerType
player.name = "Hero"
player.health = 100
```

## Control Flow

```basic
' Conditionals
IF x > 0 THEN
    PRINT "positive"
ELSEIF x < 0 THEN
    PRINT "negative"
ELSE
    PRINT "zero"
END IF

SELECT CASE x
    CASE 1: PRINT "one"
    CASE 2, 3: PRINT "two or three"
    CASE 4 TO 10: PRINT "four to ten"
    CASE IS > 10: PRINT "big"
    CASE ELSE: PRINT "other"
END SELECT

' Loops
FOR i = 1 TO 10
    PRINT i
NEXT

DO WHILE condition
    ' code
LOOP

DO
    ' code
LOOP UNTIL condition
```

## Subroutines and Functions

```basic
SUB DrawBox (x1, y1, x2, y2, c AS _UNSIGNED LONG)
    LINE (x1, y1)-(x2, y2), c, BF
END SUB

FUNCTION Distance! (x1!, y1!, x2!, y2!)
    Distance = SQR((x2 - x1)^2 + (y2 - y1)^2)
END FUNCTION

' Call them
DrawBox 10, 10, 100, 100, _RGB(255, 0, 0)
d! = Distance(0, 0, 100, 100)
```

## Input

```basic
' Keyboard
k$ = INKEY$                       ' Non-blocking
k& = _KEYHIT                      ' Key code (non-blocking)
IF _KEYDOWN(27) THEN END          ' Check specific key

' Mouse
DO WHILE _MOUSEINPUT: LOOP        ' Update mouse state
x% = _MOUSEX: y% = _MOUSEY
IF _MOUSEBUTTON(1) THEN           ' Left button

' Text input
INPUT "Name: ", name$
LINE INPUT "Full line: ", text$
```

## Timing

```basic
_DELAY 1.5                        ' Pause 1.5 seconds
_LIMIT 60                         ' Cap at 60 iterations/sec

t# = TIMER                        ' Seconds since midnight
t# = TIMER(.001)                  ' Millisecond precision
```

## String Functions

| Function | Purpose | Example |
|----------|---------|---------|
| `LEN(s$)` | Length | `LEN("Hi") = 2` |
| `LEFT$(s$, n)` | Left chars | `LEFT$("Hello", 2) = "He"` |
| `RIGHT$(s$, n)` | Right chars | `RIGHT$("Hello", 2) = "lo"` |
| `MID$(s$, p, n)` | Substring | `MID$("Hello", 2, 3) = "ell"` |
| `INSTR(s$, f$)` | Find | `INSTR("Hello", "l") = 3` |
| `UCASE$(s$)` | Uppercase | `UCASE$("Hi") = "HI"` |
| `LCASE$(s$)` | Lowercase | `LCASE$("Hi") = "hi"` |
| `LTRIM$(s$)` | Trim left | Remove leading spaces |
| `RTRIM$(s$)` | Trim right | Remove trailing spaces |
| `_TRIM$(s$)` | Trim both | Remove both |
| `VAL(s$)` | To number | `VAL("42") = 42` |
| `STR$(n)` | To string | `STR$(42) = " 42"` |
| `CHR$(n)` | ASCII char | `CHR$(65) = "A"` |
| `ASC(s$)` | ASCII code | `ASC("A") = 65` |

## Math Functions

| Function | Purpose |
|----------|---------|
| `ABS(x)` | Absolute value |
| `SGN(x)` | Sign (-1, 0, 1) |
| `INT(x)` | Floor |
| `FIX(x)` | Truncate toward zero |
| `SQR(x)` | Square root |
| `SIN(x)`, `COS(x)`, `TAN(x)` | Trig (radians) |
| `ATN(x)` | Arctangent |
| `LOG(x)` | Natural log |
| `EXP(x)` | e^x |
| `RND` | Random 0-1 |
| `RANDOMIZE TIMER` | Seed RNG |

## Memory (_MEM System)

```basic
DIM m AS _MEM
DIM arr(1000) AS LONG

m = _MEM(arr())                   ' Get memory block
_MEMPUT m, m.OFFSET, 42 AS LONG   ' Write value
value& = _MEMGET(m, m.OFFSET, LONG) ' Read value
_MEMFREE m                        ' Release

' Direct memory copy
_MEMCOPY src, src.OFFSET, bytes TO dst, dst.OFFSET
```

## Metacommands

```basic
$CONSOLE                          ' Enable console window
$CONSOLE:ONLY                     ' Console only, no graphics
$SCREENHIDE                       ' Start hidden
$RESIZE:ON                        ' Allow window resize
$EXEICON:'icon.ico'               ' Set executable icon
$VERSIONINFO:...                  ' Set version info

$DEBUG                            ' Enable $DEBUG features
$ASSERTS                          ' Enable ASSERT checks

$COLOR:32                         ' Include 32-bit color constants
$INCLUDE:'file.bi'                ' Include source file

'$DYNAMIC                         ' Dynamic arrays by default
'$STATIC                          ' Static arrays by default
```

## Error Handling

```basic
ON ERROR GOTO handler
' risky code
ON ERROR GOTO 0                   ' Disable handler

handler:
e% = ERR                          ' Error number
l& = ERL                          ' Error line
RESUME NEXT                       ' Continue after error
```

## Common Patterns

### Game Loop
```basic
SCREEN _NEWIMAGE(800, 600, 32)
DO
    _LIMIT 60
    ' Update
    ' Draw
    _DISPLAY
LOOP UNTIL _KEYHIT = 27
```

### Double Buffering
```basic
SCREEN _NEWIMAGE(640, 480, 32)
_SCREENHIDE
' Draw to back buffer
_DISPLAY
_SCREENSHOW
```

### Mouse Drag
```basic
DO WHILE _MOUSEINPUT: LOOP
IF _MOUSEBUTTON(1) THEN
    x% = _MOUSEX: y% = _MOUSEY
END IF
```

### Frame-Rate Independent Movement
```basic
DIM lastTime AS DOUBLE
lastTime = TIMER
DO
    dt# = TIMER - lastTime
    lastTime = TIMER
    x = x + speed * dt#
    _LIMIT 60
LOOP
```

## Key Codes (_KEYHIT / _KEYDOWN)

| Key | Code |
|-----|------|
| Esc | 27 |
| Enter | 13 |
| Space | 32 |
| Arrows | 18432, 19200, 19712, 20480 (Up/Left/Right/Down) |
| F1-F12 | 15104-15872 (step 256) |
| A-Z | 65-90 (uppercase) or 97-122 (lowercase) |

## Quick Tips

1. **32-bit graphics**: Use `_NEWIMAGE(w, h, 32)` and `_RGB()` colors
2. **Smooth animation**: Use `_LIMIT` and `_DISPLAY`
3. **Fast file I/O**: Use `_READFILE$` / `_WRITEFILE` for simple cases
4. **Mouse input**: Always call `_MOUSEINPUT` in a loop first
5. **Memory safety**: Use `_MEM` system instead of PEEK/POKE
6. **String building**: Use `+` concatenation, avoid repeated `MID$` assignments
7. **Performance**: INTEGER and LONG are faster than SINGLE/DOUBLE for counters
