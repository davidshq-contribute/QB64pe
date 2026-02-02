# PREPROCESSOR.md

Documentation of the QB64pe preprocessor implementation.

## Overview

QB64pe's preprocessor handles conditional compilation, constant definition, file inclusion, and metacommands before the main compilation pass.

## Conditional Compilation

### $IF/$ELSEIF/$ELSE/$END IF

```basic
$IF WIN THEN
    ' Windows-specific code
$ELSEIF LINUX THEN
    ' Linux-specific code
$ELSE
    ' Other platforms
$END IF
```

### Condition Evaluation

**Operators:**
- Comparison: `=`, `<`, `>`, `<=`, `>=`, `<>`
- Logical: `AND`, `OR`, `XOR`
- Special: `DEFINED`, `UNDEFINED`

## $LET Constant Definition

```basic
$LET flagname = value
```

Storage in `UserDefine(0, i)` (names) and `UserDefine(1, i)` (values).

## Predefined Constants

| Constant | True When |
|----------|-----------|
| WINDOWS, WIN | Windows |
| LINUX | Linux |
| MAC, MACOSX | macOS |
| 32BIT | 32-bit build |
| 64BIT | 64-bit build |
| VERSION | Version string |
| _QB64PE_ | Always true |
| _ARM_ | ARM processor |

## Metacommands

### File Inclusion
- `$INCLUDE:'file'` - Include file
- `$INCLUDEONCE` - Include once only

### Array Mode
- `$DYNAMIC` - Dynamic arrays
- `$STATIC` - Static arrays

### Debug/Console
- `$DEBUG` - Enable debugging
- `$CHECKING:ON/OFF` - Runtime checks
- `$CONSOLE` - Console window
- `$ASSERTS` - Assertions

### Screen/Window
- `$SCREENHIDE/$SCREENSHOW` - Window visibility
- `$RESIZE:OFF/ON/STRETCH/SMOOTH` - Resize mode

### Resources (Windows)
- `$EXEICON:'file'` - Executable icon
- `$VERSIONINFO:key=value` - Version info

### Data/Libraries
- `$EMBED:'file','handle'` - Embed file
- `$USELIBRARY:'author/lib'` - QB64 library
- `$UNSTABLE:MIDI/HTTP` - Unstable features

### Build Control
- `$ERROR message` - Force error
- `$COLOR:0/32` - Color mode

## Two-Pass System

**Pass 1 (Prepass):**
- $INCLUDE resolution
- Build program structure
- Process metacommands
- Register declarations

**Pass 2 (Main):**
- Full compilation
- Skip invalid lines
- Code generation

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | Preprocessor logic |
| Lines 1617-2865 | Prepass processing |
| Lines 3054-3516 | Metacommand handling |

## Example Usage

### Platform-Specific Code

```basic
$IF WIN THEN
    SHELL "cmd /c dir"
$ELSEIF LINUX THEN
    SHELL "ls -la"
$END IF
```

### Feature Flags

```basic
$LET DEBUG_MODE = -1

$IF DEBUG_MODE THEN
    $CONSOLE
    PRINT "Debug enabled"
$END IF
```

### Version Requirements

```basic
$IF VERSION < 3.0 THEN
    $ERROR Requires QB64pe 3.0+
$END IF
```
