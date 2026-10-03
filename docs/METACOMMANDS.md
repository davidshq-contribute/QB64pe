# METACOMMANDS.md

Documentation of all `$` metacommands in QB64pe.

## Preprocessor Metacommands

### $IF / $ELSEIF / $ELSE / $END IF

Conditional compilation based on preprocessor conditions.

```basic
$IF condition THEN
    [code block]
$ELSEIF condition THEN
    [code block]
$ELSE
    [code block]
$END IF
```

**Alternative syntax:** `$ELSE IF` (two words) works like `$ELSEIF`. `$ENDIF` works like `$END IF`.

**Supported operators:** `=`, `<>`, `<`, `>`, `<=`, `>=`, `AND`, `OR`, `XOR`

**Built-in variables:**
| Variable | True when |
|----------|-----------|
| `WINDOWS` / `WIN` | Windows OS |
| `LINUX` | Linux OS |
| `MAC` / `MACOSX` | macOS |
| `32BIT` | 32-bit system |
| `64BIT` | 64-bit system |
| `VERSION` | QB64 version string |
| `_QB64PE_` | Always -1 (QB64-PE) |
| `_ARM_` | ARM architecture |
| `DEFINED(name)` | Variable is defined |
| `UNDEFINED(name)` | Variable is not defined |

### $LET

Define preprocessor variables for use in $IF conditions.

```basic
$LET MYFEATURE = 1
$LET MYVERSION = "2.0"
```

**Constraints:** Values cannot contain spaces.

### $INCLUDE

Include external source files.

```basic
$INCLUDE:'filename.bi'
$INCLUDE:'./relative/path.bas'
$INCLUDE:'/absolute/path.bm'
```

**File resolution:**
1. Relative to source file's directory
2. Absolute path

**Nesting limit:** 100 levels

### $INCLUDEONCE

Prevent duplicate inclusion of a file.

```basic
$INCLUDEONCE
```

Place at the top of header files. The file will only be included once even if `$INCLUDE` is called multiple times.

## Configuration Metacommands

### $COLOR:0 / $COLOR:32

Set color palette mode.

```basic
$COLOR:0     ' 8-color mode (legacy)
$COLOR:32    ' 32-bit color mode
```

**Constraint:** Cannot use both in the same program.

### $CONSOLE / $CONSOLE:ONLY

Enable console window support.

```basic
$CONSOLE           ' Enable console alongside graphics
$CONSOLE:ONLY      ' Console-only mode (no graphics window)
```

### $SCREENHIDE / $SCREENSHOW

Control initial screen visibility.

```basic
$SCREENHIDE    ' Start with screen hidden
$SCREENSHOW    ' Start with screen visible
```

### $RESIZE

Configure window resize behavior.

```basic
$RESIZE:OFF       ' Window not resizable
$RESIZE:ON        ' Window resizable
$RESIZE:STRETCH   ' Resizable with pixel stretching
$RESIZE:SMOOTH    ' Resizable with smooth scaling
```

### $DEBUG

Enable IDE debugger features.

```basic
$DEBUG
```

Creates `_DEBUG_` preprocessor variable. Enables variable watch window functionality. IDE-only feature.

### $CHECKING:ON / $CHECKING:OFF

Control variable bounds checking.

```basic
$CHECKING:OFF    ' Disable checking (faster)
[code]
$CHECKING:ON     ' Re-enable checking
```

### $ASSERTS / $ASSERTS:CONSOLE

Enable assertion checking.

```basic
$ASSERTS             ' Enable assertions
$ASSERTS:CONSOLE     ' Enable assertions, output to console
```

Creates `_ASSERTS_` preprocessor variable. Use with `_ASSERT` statement.

### $FORMAT:OFF / $FORMAT:ON

Control IDE auto-formatting.

```basic
$FORMAT:OFF
    [code that shouldn't be auto-formatted]
$FORMAT:ON
```

## Executable Metacommands

### $EXEICON

Set executable icon (Windows only).

```basic
$EXEICON:'icon.ico'
$EXEICON:'./icons/myapp.ico'
```

**Constraints:**
- Only one icon per program
- File must exist
- Windows only

### $VERSIONINFO

Embed version information into executable.

```basic
$VERSIONINFO:FILEVERSION#=1,0,0,0
$VERSIONINFO:PRODUCTVERSION#=1,0,0,0
$VERSIONINFO:CompanyName='My Company'
$VERSIONINFO:FileDescription='My Application'
$VERSIONINFO:FileVersion='1.0.0.0'
$VERSIONINFO:InternalName='myapp'
$VERSIONINFO:LegalCopyright='Copyright 2024'
$VERSIONINFO:LegalTrademarks='Trademark Info'
$VERSIONINFO:OriginalFilename='myapp.exe'
$VERSIONINFO:ProductName='My Product'
$VERSIONINFO:ProductVersion='1.0.0.0'
$VERSIONINFO:Comments='Additional comments'
$VERSIONINFO:Web='https://example.com'
```

**Numeric versions:** Must have 4 comma-separated values.
**String values:** Enclosed in the metacommand string pair. The compiler uses `METACOMMAND_STRING_ENCLOSING_PAIR = "''"` (two single quotes) for string values in `$VERSIONINFO`, `$LET`, `$ERROR`, and similar; values are delimited by this pair (e.g. `'value'`).

### $EMBED

Embed binary files into executable.

```basic
$EMBED:'data.dat','MYDATA'
$EMBED:'./images/logo.png','LOGO'
```

Access at runtime with `_EMBEDDED$("HANDLE")`.

**Constraints:**
- Handle must be valid identifier
- Handle cannot be duplicated
- File cannot be embedded twice

## Library Metacommands

### $USELIBRARY

Include a QB64-PE library package.

```basic
$USELIBRARY:'author/libraryname'
```

Libraries are searched in `libraries/descriptors/author/libraryname.ini`.

### DECLARE LIBRARY (Statement, not metacommand)

While not a `$` metacommand, `DECLARE LIBRARY` is essential for calling external C/C++ functions:

```basic
DECLARE LIBRARY "libraryname"
    FUNCTION CFunction& (arg AS LONG)
    SUB CSub (BYVAL x AS LONG)
END DECLARE

DECLARE DYNAMIC LIBRARY "libraryname"
    ' For runtime-loaded DLLs/shared libraries
END DECLARE

DECLARE CUSTOMTYPE LIBRARY "libraryname"
    ' For libraries with custom type definitions
END DECLARE
```

**Notes:**
- `BYVAL` and `ALIAS` keywords only work within DECLARE LIBRARY blocks
- Library name is without extension (.dll/.so/.dylib added automatically)

## Array Metacommands

### $DYNAMIC / $STATIC

Control array allocation strategy.

```basic
DIM array(100) $STATIC    ' Static allocation
DIM array(100) $DYNAMIC   ' Dynamic allocation
```

Can appear multiple times on same line; last one wins.

## Error Metacommand

### $ERROR

Generate a compile-time error.

```basic
$IF VERSION < "3.0" THEN
    $ERROR 'This program requires QB64-PE 3.0 or higher'
$END IF
```

## Deprecated Metacommands

### $NOPREFIX (Deprecated)

No longer supported. QB64-PE specific keywords must have underscore prefix.

### $MIDISOUNDFONT (Deprecated)

Use `_MIDISOUNDBANK` instead.

### $UNSTABLE (Deprecated)

```basic
$UNSTABLE:MIDI    ' No longer required
$UNSTABLE:HTTP    ' No longer required
```

These features are now stable and always enabled.

## Metacommand Processing Order

1. **Preprocessing phase (prepass=1):**
   - `$IF`, `$ELSE`, `$LET` - conditional compilation
   - `$INCLUDE`, `$INCLUDEONCE` - file inclusion
   - `$USELIBRARY` - library resolution
   - `$COLOR`, `$DEBUG`, `$ASSERTS`, `$CONSOLE` - configuration

2. **Layout/syntax phase:**
   - `$EXEICON`, `$VERSIONINFO`, `$EMBED` - executable metadata
   - `$RESIZE`, `$SCREENHIDE`, `$SCREENSHOW` - window configuration
   - `$FORMAT`, `$CHECKING` - code formatting

## Summary Table

| Metacommand | Type | Scope | Effect |
|-------------|------|-------|--------|
| `$IF/$ELSEIF/$ELSE/$END IF` | Preprocessor | Global | Conditional compilation |
| `$LET` | Preprocessor | Global | Define variable |
| `$INCLUDE` | Include | File | Include source file |
| `$INCLUDEONCE` | Include | File | Prevent duplicate inclusion |
| `$COLOR:0/32` | Config | Global | Color palette mode |
| `$CONSOLE` | Config | Global | Enable console |
| `$CONSOLE:ONLY` | Config | Global | Console-only mode |
| `$SCREENHIDE` | Config | Global | Hide screen on startup |
| `$SCREENSHOW` | Config | Global | Show screen on startup |
| `$RESIZE:*` | Config | Global | Window resize behavior |
| `$DEBUG` | Config | Global | Enable debugger |
| `$CHECKING:ON/OFF` | Config | Block | Bounds checking |
| `$ASSERTS` | Config | Global | Enable assertions |
| `$EXEICON` | Executable | Global | Set icon |
| `$VERSIONINFO` | Executable | Global | Version metadata |
| `$EMBED` | Executable | Global | Embed file |
| `$USELIBRARY` | Library | File | Include library |
| `$DYNAMIC/$STATIC` | Array | Declaration | Allocation strategy |
| `$FORMAT:OFF/ON` | Formatting | Block | IDE formatting |
| `$ERROR` | Error | Conditional | Compilation error |
