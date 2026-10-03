# ARCHITECTURE-BASIC.md

Architecture of the QB64 BASIC source code (the compiler and IDE).

## Main Compiler: source/qb64pe.bas

The compiler is ~24,000 lines of QB64 code that transpiles QB64 source to C++.

### Tokenization

Lines are tokenized into space-delimited elements using special separator characters:

```basic
sp  = CHR$(13)  ' Primary element separator (CR)
sp2 = CHR$(10)  ' Secondary delimiter (LF)
sp3 = CHR$(26)  ' Tertiary delimiter (SUB)
```

Key tokenization functions:
- `lineformat(a$)` - Converts raw BASIC lines to tokenized format
- `numelements(a$)` - Counts space-separated elements
- `getelement$(a$, elenum)` - Retrieves nth element
- `getnextelement$()` / `getprevelement$()` - Element iterators
- `seperateargs(a$, ca$, pass&)` - Splits function arguments

### Symbol Table (Hash System)

Symbols are stored in a hash table with collision chaining:

```basic
TYPE HashListItem
    Flags AS LONG          ' Type and scope flags
    Reference AS LONG      ' User data (index into ids array)
    NextItem AS LONG       ' Next item in collision chain
    PrevItem AS LONG
    LastItem AS LONG
END TYPE
```

Key functions (`source/utilities/hash.bas`):
- `HashValue&(a$)` - Compute hash from string (uses first 5 chars, last 2 chars, length)
- `HashAdd(name$, flags, reference)` - Register symbol
- `HashFind(name$, searchflags, resultflags, resultreference)` - Lookup symbol
- `HashFindRev()` - Reverse search for overloaded names

Hash flags include: `HASHFLAG_RESERVED`, `HASHFLAG_TYPE`, `HASHFLAG_OPERATOR`, `HASHFLAG_UDTELEMENT`

### Identifier Registry

All variables, functions, and SUBs are tracked in the `ids()` array. The type below shows the main fields; the full `idstruct` in `source/qb64pe.bas` includes additional fields (e.g. `t`, `tsize`, `minargs`, `Dependency`, `internal_subfunc`, `ccall`, `overloaded`, `argsize`, `specialformat`, `secondargmustbe`, `secondargcantbe`, `insubfunc`, `insubfuncn`, `share`, `nele`, `nelereq`, `linkid`, `linkarg`, `staticscope`, `sfid`, `sfarg`, `staticarray`, `mayhave`, `musthave`).

```basic
TYPE idstruct
    n AS STRING * 256        ' Variable/function name (uppercase)
    cn AS STRING * 256       ' Case-sensitive name
    arraytype AS LONG        ' Array element type (similar to t)
    arrayelements AS INTEGER ' Array dimension count
    subfunc AS INTEGER       ' 1=function, 2=SUB
    callname AS STRING * 256 ' C++ function name
    args AS INTEGER          ' Parameter count
    minargs AS INTEGER       ' Minimum required arguments
    arg AS STRING * 400      ' Parameter types
    ret AS LONG              ' Return type
    t AS LONG                ' Type (bit-encoded)
    tsize AS LONG            ' Size (e.g. fixed-length string)
    hr_syntax AS STRING      ' Human-readable syntax for IDE
END TYPE

DIM SHARED ids(1 TO ids_max) AS idstruct
```

Key functions:
- `regid` - Register new identifier
- `findid&(n2$)` - Look up identifier by name
- `getid(i)` - Load identifier by index
- `clearid` - Reset ID to blank state

### Type System

Types are encoded as bit flags in a single LONG value:

```basic
' Size encoded in bits 0-8 (8, 16, 32, 64, 256)
ISPOINTER       = 1 << 9
ISUNSIGNED      = 1 << 10
ISSTRING        = 1 << 11
ISFIXEDLENGTH   = 1 << 12
ISUDT           = 1 << 13
ISARRAY         = 1 << 14
ISOFFSETINBITS  = 1 << 15
ISFLOAT         = 1 << 16
ISOFFSET        = 1 << 17
```

Type suffix symbols: `&` (LONG), `%` (INTEGER), `!` (SINGLE), `#` (DOUBLE), `$` (STRING), `~` (unsigned prefix), `&&` (_INTEGER64), `` ` `` (_BIT)

Key functions (`source/utilities/type.bas`):
- `typname2typ(typename$)` - Convert type name to encoded value
- `typevalue2symbol$(t)` - Convert type to QB64 symbol
- `id2fulltypename$()` / `id2shorttypename$()` - Readable type names

### Expression Evaluation

Expression parsing and C++ code generation:
- `evaluate$(a2$, typ)` - Evaluate QB64 expression, return C++ code
- `evaluatefunc$(a2$, args, typ)` - Handle function calls
- `fixoperationorder$(savea$)` - Apply operator precedence
- `refer$(a2$, typ, method)` - Resolve variable references

### Code Generation Buffers

C++ code is emitted to multiple buffers that are combined at the end:

| Buffer | Output File | Purpose |
|--------|-------------|---------|
| `MainTxtBuf` | main0.txt | Main executable code |
| `GlobTxtBuf` | global.txt | Global declarations |
| `DataTxtBuf` | maindata.txt | DATA statement contents |
| `DataBinBuf` | data.bin | Binary DATA statements |
| `RegTxtBuf` | regsf.txt | SUB/FUNCTION registration |
| `FreeTxtBuf` | mainfree.txt | Cleanup/destruction code |
| `ErrTxtBuf` | mainerr.txt | Error handling |
| `RetTxtBuf` | ret0.txt | Return point dispatch |
| `RunTxtBuf` | runline.txt | Line-by-line execution |
| `TimeTxtBuf` | ontimer.txt | ON TIMER handlers |
| `TimejTxtBuf` | ontimerj.txt | ON TIMER (j variant) |
| `KeyTxtBuf` | onkey.txt | ON KEY handlers |
| `KeyjTxtBuf` | onkeyj.txt | ON KEY (j variant) |
| `StrigTxtBuf` | onstrig.txt | ON STRIG handlers |
| `StrigjTxtBuf` | onstrigj.txt | ON STRIG (j variant) |
| `ChainTxtBuf` | chain.txt | CHAIN statement |
| `InpChainTxtBuf` | inpchain.txt | CHAIN input |
| `VWatchMainDispatchBuf` | vw_main_dispatch.txt | $DEBUG variable watch dispatch |
| `VWatchMainSkipBuf` | vw_main_skip.txt | $DEBUG variable watch skip |
| `ExtDepBuf` | extdep.txt | External deps: ICON:, DECL:, INCL: (icon path, DECLARE LIBRARY files, $INCLUDE paths) |
| `IncOneBuf` | incone.txt | $INCLUDEONCE guard (paths already included) |
| `FormatBuf` | format.out | Formatter output |
| `autoIncludeBuffer` | (internal) | Auto-include file list (beforefirstline.bi, color*.bi, vwatch, aftermain, afterlastline) |
| (temp) | dyninfo.txt | Dynamic array info (written via `bh`) |
| (temp) | clear.txt | Clear/cleanup block (written via MainTxtBuf) |
| `mainincbuf` | main.txt | Aggregator: contains `#include "main0.txt"`, `#include "main1.txt"`, ... (main0 = main program; main1..mainN = SUB/FUNCTION). qbx.cpp includes `main.txt`. |

Code emission pattern:
```basic
WriteBufLine MainTxtBuf, "C++ statement;"
WriteBufLine GlobTxtBuf, "int some_var;"
```

### Control Flow Tracking

Nested control structures are tracked with:

```basic
DIM controllevel              ' Nesting depth
REDIM controltype(1000)       ' Type at each level:
                              '   0 = (cleared)
                              '   1 = IF/THEN/ELSE
                              '   2 = FOR/NEXT
                              '   3 = DO...LOOP WHILE/UNTIL (condition at LOOP)
                              '   4 = DO WHILE/UNTIL...LOOP (condition at DO)
                              '   5 = WHILE/WEND
                              '   6 = $IF directive
                              '   10-17 = SELECT CASE (various types)
                              '   18 = CASE
                              '   19 = CASE ELSE
                              '   32 = SUB/FUNCTION
```

### Label System

Labels for GOTO/GOSUB and DATA statements:

```basic
TYPE Label_Type
    State AS _UNSIGNED _BYTE   ' 0=referenced, 1=created
    cn AS STRING * 256         ' C++ label name
    Scope AS LONG              ' SUB/FUNCTION scope
    Data_Offset AS _INTEGER64  ' DATA statement offset
    Data_Referenced AS _BYTE   ' Referenced in DATA
    Error_Line AS LONG         ' For error reporting
    SourceLineNumber AS LONG   ' Where defined
END TYPE

REDIM Labels(1 TO Labels_Ubound) AS Label_Type
```

### Key Compiler State Variables

```basic
DIM prepass                   ' TRUE during preprocessing pass
DIM linenumber                ' Current source line
DIM wholeline$                ' Current tokenized line (uppercase)
DIM cwholeline$               ' Case-sensitive version
DIM subfunc$, subfuncn        ' Current SUB/FUNCTION name and ID
DIM sublevel                  ' SUB/FUNCTION nesting level
DIM dimstatic, dimshared      ' Declaration scope flags
DIM Error_Happened            ' Compilation error flag
DIM ideerrorline, idemessage  ' Error reporting for IDE
```

## IDE: source/ide/

### ide_global.bas

Global IDE state (~243 lines):
- `IdeSystem` - Current input context (1=main, 2=search, 3=help)
- `IdeDebugMode` - Debugger state
- `callstacklist$`, `callStackLength` - Debug call stack
- `IdeBmk()` - Bookmark array (line, x position)
- `IdeBreakpoints()`, `IdeSkipLines()` - Breakpoint management
- Codepage support for 27 character encodings

### ide_methods.bas

Main IDE implementation (~21,000 lines):
- `ide(ignore)` - Brief coordination with compiler
- `ide2(ignore)` - Full IDE with UI rendering

Features:
- Syntax highlighting with configurable colors
- Auto-indentation and code formatting
- Find/replace functionality
- Bookmarks and quick navigation
- Line numbers
- Real-time compilation progress
- Context-sensitive help (wiki integration)

### config/

Configuration management:
- `cfg_global.bas` - Config data structures
- `cfg_methods.bas` - Read/write .cfg files

Stores: fonts, colors, window size, recent files, editor preferences.

### wiki/

Integrated help system:
- `wiki_global.bas` - Help database structure
- `wiki_methods.bas` - Help rendering and search

## Built-in Functions: source/subs_functions/

### subs_functions.bas

Registers ~250+ built-in functions and SUBs via `reginternal`:

**Categories:**
- String: `Mid$`, `Left$`, `Right$`, `Trim$`, `Ucase$`, `Lcase$`
- Math: `Abs`, `Int`, `Sin`, `Cos`, `Tan`, `Sqr`, `Exp`, `Log`, `Rnd`
- I/O: `Print`, `Input`, `Open`, `Close`, `Line Input`
- Array: `ReDim`, `Erase`, `LBound`, `UBound`
- Type conversion: `Val`, `Str$`, `Hex$`, `Oct$`, `Bin$`
- QB64 Extensions: `_Resize`, `_MemGet`, `_MemPut`, `_Cast`, `_IIf`
- Graphics: `_PutImage`, `_GetImage`, `_LoadImage`, `_FreeImage`

**Special argument codes:**
```
-1  = Any numeric type
-2  = Offset+Size (CALL INTERRUPT)
-3  = Offset+Size (graphics GET/PUT)
-4  = Offset+Size (file GET/PUT)
-5  = Offset only
-6  = Size only
-7  = _MEM structure reference
```

### extensions/opengl/

OpenGL support:
- `opengl_global.bas` - OpenGL command/constant tables
- `opengl_methods.bas` - OpenGL function registration
- `GL_COMMANDS()` - ~2000 OpenGL functions
- `GL_DEFINES()` - OpenGL constants

A `_GL` SUB declaration triggers the OpenGL dependency.

## Utilities: source/utilities/

| File | Purpose |
|------|---------|
| `hash.bas` | Symbol table hash functions |
| `type.bas` | Type system utilities |
| `elements.bas` | String element parsing (tokenization helpers) |
| `const_eval.bas` | Compile-time constant evaluation for CONST and $IF |
| `strings.bas` | String manipulation (StrRemove$, StrReplace$, quotes) |
| `file.bas` | File operations (CopyFile, path handling, extension handling) |
| `format.bas` | Code formatting and indentation |
| `give_error.bas` | Error reporting |
| `statevars.bas` | State variable tracking with undo |
| `build.bas` | C++ compilation integration |
| `terminal.bas` | Console output and colors |
| `ini-manager/` | INI file parsing |
| `s-buffer/` | Simple buffer management |

## Global Definitions: source/global/

| File | Purpose |
|------|---------|
| `version.bas` | Version string (`Version$ = "4.4.0"`) and CI detection |
| `constants.bas` | Separator constants (sp, sp2, sp3), line endings, OS detection |
| `settings.bas` | Debug mode flag |

## Code Emission: source/emit/

### logging.bas

Generates logging statements:
- `EmitLoggingStatement()` - Generate `_LogTrace`, `_LogInfo`, `_LogWarn`, `_LogError`
- Integrates source file, function name, and line number into log output

## Compilation Flow

```
Initialize
├── Clear hash table
├── Register reserved words (keywords, types, operators)
├── Create temp folder
└── Open output buffers

Pre-pass (prepass=1)
├── Read source lines
├── Process $INCLUDE, $EMBED, $USELIBRARY
├── Handle $IF/$IFDEF conditionals
├── Parse TYPE definitions
└── Build initial symbol table

Main Pass (prepass=0)
├── FOR each source line:
│   ├── Tokenize to space-delimited elements
│   ├── Identify statement type
│   ├── Resolve identifiers via hash table
│   ├── Parse expressions
│   ├── Type check
│   ├── Generate C++ code
│   └── Write to appropriate buffer
└── Handle SUB/FUNCTION definitions

Post-processing
├── Combine output buffers
├── Generate C++ prototypes
└── Write files to internal/temp/

Invoke C++ Compiler
├── Call Makefile with OS and DEP_* flags
└── Link with libqb and dependencies
```
