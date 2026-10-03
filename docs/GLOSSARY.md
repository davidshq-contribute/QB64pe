# GLOSSARY.md

Naming conventions and common abbreviations used throughout the QB64pe codebase.

## String Separator Constants

These characters delimit tokenized elements in the compiler's internal representation. Defined in `source/global/constants.bas` using `_CHR_CR`, `_CHR_LF`, `_CHR_SUB` (equivalent to CHR$(13), CHR$(10), CHR$(26)).

| Constant | Value | ASCII | Purpose |
|----------|-------|-------|---------|
| `sp` | `CHR$(13)` | CR | Primary element separator |
| `sp2` | `CHR$(10)` | LF | Secondary delimiter |
| `sp3` | `CHR$(26)` | SUB | Tertiary delimiter |
| `sp_asc` | 13 | - | ASCII value of sp |
| `sp2_asc` | 10 | - | ASCII value of sp2 |
| `sp3_asc` | 26 | - | ASCII value of sp3 |

In debug mode (`Debug` true), sp/sp2/sp3 are replaced with visible characters (CHR$(250), CHR$(249), CHR$(179)) in `constants.bas`.

**Usage:** Functions like `getelement$()`, `numelements()` use `sp` to iterate through space-separated token lists.

## Identifier Structure Fields

The `idstruct` type (in `source/qb64pe.bas`) stores all identifier information. Key fields:

| Field | Meaning |
|-------|---------|
| `n` | Name (uppercase, for case-insensitive lookups) |
| `cn` | Case-sensitive name (preserves original source) |
| `t` | Type value (bit-encoded flags + size) |
| `tsize` | Size in bytes (for fixed-length strings) |
| `arraytype` | Array element type (for arrays; similar to t) |
| `arrayelements` | Array dimension count |
| `subfunc` | 0=variable, 1=function, 2=sub |
| `callname` | C++ function/sub name |
| `args` | Number of arguments (for functions/subs) |
| `minargs` | Minimum required arguments |
| `arg` | Serialized argument types |
| `ret` | Return type (for functions) |
| `hr_syntax` | Human-readable syntax for IDE |
| `sfid` | Parent sub/function ID (if this is a parameter) |
| `sfarg` | Argument position in parent (1=first) |
| `share` | SHARED flag |
| `staticscope` | STATIC flag |

Additional fields in source: `Dependency`, `internal_subfunc`, `ccall`, `overloaded`, `argsize`, `specialformat`, `secondargmustbe`, `secondargcantbe`, `insubfunc`, `insubfuncn`, `nele`, `nelereq`, `linkid`, `linkarg`, `staticarray`, `mayhave`, `musthave`.

## Common Variable Name Patterns

### Single-Letter Variables

| Name | Typical Meaning |
|------|-----------------|
| `i`, `j` | Loop counters |
| `a$`, `b$` | Temporary/working strings |
| `n` | Counter or count value |
| `p` | Position/pointer in string |
| `t` | Type value |
| `c` | Character (from ASC()) |
| `e$` | Error message or element string |
| `l$` | Current token/element from loop |

### Two-Letter Abbreviations

| Name | Meaning |
|------|---------|
| `a2$`, `l2$` | Backup/alternate copy |
| `i2` | Secondary counter |
| `n$` | Identifier name |
| `sc$` | Symbol/character suffix (`$`, `%`, `&`) |
| `sf` | Sub/function (prefix in arrays) |

## ID Management Functions

| Function | Purpose |
|----------|---------|
| `regid` | Register new identifier in `ids()` array |
| `findid&(n$)` | Search for identifier by name, returns 0/1/2 |
| `getid(i)` | Load `ids(i)` into global `id` variable |
| `clearid` | Reset `id` to blank state |
| `currentid` | Index of last accessed identifier |

**findid return values:**
- 0 = Not found
- 1 = Found, no more matches
- 2 = Found, more matches possible (for overloads)

## Element Parsing Functions

Functions for working with tokenized strings (space-delimited by `sp`):

| Function | Purpose |
|----------|---------|
| `getelement$(a$, n)` | Get nth element from string |
| `numelements(a$)` | Count total elements |
| `getnextelement$()` | Iterator (forward) |
| `getprevelement$()` | Iterator (backward) |
| `peeknextelement$()` | Look ahead without consuming |
| `pushelement(a$, b$)` | Append element to list |
| `getelements$(a$, i1, i2)` | Get range of elements |
| `removeelements(a$, first, last)` | Remove range |

## Expression Evaluation Functions

| Function | Purpose |
|----------|---------|
| `evaluate$(a$, typ)` | Evaluate expression, return C++ code and type |
| `evaluatefunc$(a$, args, typ)` | Evaluate function call |
| `evaluatetotyp$(a$, targettyp)` | Evaluate and convert to target type |
| `fixoperationorder$(a$)` | Apply operator precedence |

## Buffer Names

Pattern: `[Purpose]TxtBuf` (or `*Buf`) - each writes to a temp file during C++ generation:

| Buffer | Output File | Contents |
|--------|-------------|----------|
| `GlobTxtBuf` | global.txt | Global declarations |
| `MainTxtBuf` | main0.txt | Main program code |
| `DataTxtBuf` | maindata.txt | DATA statement contents |
| `DataBinBuf` | data.bin | Binary data |
| `RegTxtBuf` | regsf.txt | SUB/FUNCTION registration |
| `FreeTxtBuf` | mainfree.txt | Cleanup code |
| `ErrTxtBuf` | mainerr.txt | Error handling |
| `RunTxtBuf` | runline.txt | Line-by-line execution |
| `TimeTxtBuf` | ontimer.txt | ON TIMER handlers |
| `TimejTxtBuf` | ontimerj.txt | ON TIMER (j variant) |
| `KeyTxtBuf` | onkey.txt | ON KEY handlers |
| `KeyjTxtBuf` | onkeyj.txt | ON KEY (j variant) |
| `StrigTxtBuf` | onstrig.txt | ON STRIG handlers |
| `StrigjTxtBuf` | onstrigj.txt | ON STRIG (j variant) |
| `ChainTxtBuf` | chain.txt | CHAIN statement |
| `InpChainTxtBuf` | inpchain.txt | CHAIN input |
| `RetTxtBuf` | ret0.txt | RETURN handling |
| `VWatchMainDispatchBuf` | vw_main_dispatch.txt | $DEBUG variable watch |
| `VWatchMainSkipBuf` | vw_main_skip.txt | $DEBUG variable watch skip |
| `ExtDepBuf` | extdep.txt | ICON:, DECL:, INCL: (external deps for IDE/build) |
| `IncOneBuf` | incone.txt | $INCLUDEONCE guard |
| `FormatBuf` | format.out | Formatter output |
| `mainincbuf` | main.txt | Aggregator: `#include "main0.txt"`, `#include "main1.txt"`, ... (qbx includes main.txt) |

## Type-Related Naming

| Term | Meaning |
|------|---------|
| `typ` | Encoded type value (bit flags + size) |
| `bits` | Size in bits (`typ AND 511`) |
| `UDT` | User-Defined Type (TYPE...END TYPE) |
| `udtx*` | UDT definition arrays (udtxname, udtxsize, etc.) |
| `udte*` | UDT element arrays (udtename, udtetype, etc.) |

## Scope and Control Variables

| Variable | Meaning |
|----------|---------|
| `subfunc$` | Current SUB/FUNCTION name |
| `subfuncn` | Current SUB/FUNCTION index in `ids()` |
| `sublevel` | SUB/FUNCTION nesting depth |
| `controllevel` | Control structure nesting depth |
| `controltype()` | Type of each nested control structure |
| `dimstatic` | Currently declaring STATIC variable |
| `dimshared` | Currently declaring SHARED variable |

## Compilation State

| Variable | Meaning |
|----------|---------|
| `prepass` | 0=main pass, 1=preprocessing pass |
| `wholeline$` | Current source line (tokenized) |
| `cwholeline$` | Case-sensitive version |
| `linenumber` | Current line number |
| `reallinenumber` | Actual source file line |
| `inclevel` | Include file nesting depth |
| `incname$()` | Include file paths by level |

## IDE Communication

| Variable | Meaning |
|----------|---------|
| `idemode` | 1=IDE mode, 0=command-line |
| `idecommand$` | Message from IDE (command byte + data) |
| `idereturn$` | Response to IDE |
| `ideerror` | Error flag |
| `ideerrorline` | Line number of error |
| `idemessage$` | Error message text |

## Common Prefixes

| Prefix | Meaning |
|--------|---------|
| `ide*` | IDE-related variables |
| `dim*` | Declaration context flags |
| `control*` | Control structure tracking |
| `inc*` | Include file handling |
| `udt*` | User-defined type handling |
| `sf*` | Sub/function related |
| `qbs*` | QB string operations (C++) |
| `func_*` | C++ function implementations |
| `sub_*` | C++ sub implementations |
