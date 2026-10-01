# GLOBALS.md

Documentation of global variables and their interactions in the QB64pe compiler.

## Compilation State Variables

### Pass Control

| Variable | Type | Values | Purpose |
|----------|------|--------|---------|
| `prepass` | LONG | 0, 1, -1 | 0=main pass, 1=prepass, -1=preprocessing |
| `recompile` | INTEGER | 0, 1 | Flag to force recompilation |

### Line Tracking

| Variable | Type | Purpose |
|----------|------|---------|
| `linenumber` | LONG | Current line being compiled |
| `reallinenumber` | LONG | Actual source file line |
| `totallinenumber` | LONG | Total lines processed |
| `wholeline$` | STRING | Current tokenized line |
| `cwholeline$` | STRING | Case-sensitive version |
| `linefragment$` | STRING | Partial line during parsing |

### Include File State

| Variable | Type | Purpose |
|----------|------|---------|
| `inclevel` | LONG | Nesting depth (0=main, 1+=included) |
| `incname$(100)` | STRING | File paths at each level |
| `inclinenumber(100)` | LONG | Line number in each file |
| `incerror` | STRING | Include-related error message |

## Control Structure Tracking

All indexed by `controllevel` (current nesting depth):

| Variable | Type | Purpose |
|----------|------|---------|
| `controllevel` | INTEGER | Current nesting depth (0-1000) |
| `controltype(1000)` | INTEGER | Type of control structure |
| `controlid(1000)` | LONG | ID reference for structure |
| `controlvalue(1000)` | LONG | Value info (loop bounds, etc.) |
| `controlstate(1000)` | INTEGER | State during compilation |
| `controlref(1000)` | LONG | Source line where created |

### Control Type Values

| Value | Structure |
|-------|-----------|
| 1 | IF/THEN/ELSE |
| 2 | FOR/NEXT |
| 3 | DO...LOOP WHILE/UNTIL (condition at LOOP) |
| 4 | DO WHILE/UNTIL...LOOP (condition at DO) |
| 5 | WHILE/WEND |
| 6 | $IF directive |
| 10-17 | SELECT CASE (various types) |
| 18 | CASE |
| 19 | CASE ELSE |
| 32 | SUB/FUNCTION |

## Identifier Registry

| Variable | Type | Purpose |
|----------|------|---------|
| `id` | idstruct | Currently loaded identifier |
| `ids(ids_max)` | idstruct | Array of all identifiers |
| `idn` | LONG | Count of registered identifiers |
| `ids_max` | LONG | Array capacity (default 1024) |
| `currentid` | LONG | Index of last accessed ID |
| `cleariddata` | idstruct | Template for clearing entries |

### Identifier Lookup State

| Variable | Type | Purpose |
|----------|------|---------|
| `findidsecondarg$` | STRING | Second argument for overload matching |
| `findanotherid` | INTEGER | 0=find first, 1=find next match |
| `findidinternal` | LONG | Internal lookup state |

## Type System Globals

### UDT (User-Defined Type) Tracking

| Variable | Type | Purpose |
|----------|------|---------|
| `lasttype` | LONG | Index of last TYPE definition |
| `lasttypeelement` | LONG | Index of last element in TYPE |
| `typeDefinitions$` | STRING | Serialized TYPE information |
| `backupTypeDefinitions$` | STRING | Backup for rollback |

### UDT Definition Arrays (indexed by UDT ID)

| Array | Purpose |
|-------|---------|
| `udtxname(1000)` | UDT name (uppercase) |
| `udtxcname(1000)` | UDT name (case-sensitive) |
| `udtxsize(1000)` | Size in bytes |
| `udtxnext(1000)` | First element index |
| `udtxvariable(1000)` | Has variable-length fields? |

### UDT Element Arrays (indexed by element ID)

| Array | Purpose |
|-------|---------|
| `udtename(1000)` | Element name (uppercase) |
| `udtecname(1000)` | Element name (case-sensitive) |
| `udtesize(1000)` | Element size in bytes |
| `udtetype(1000)` | Element type value |
| `udtetypesize(1000)` | Element type size |
| `udtearrayelements(1000)` | Array elements (-1 if not array) |
| `udtenext(1000)` | Next element index |

## SUB/FUNCTION Tracking

| Variable | Type | Purpose |
|----------|------|---------|
| `subfunc$` | STRING | Current SUB/FUNCTION name |
| `subfuncn` | LONG | Current SUB/FUNCTION index |
| `subfuncnlast` | LONG | Previous SUB/FUNCTION index |
| `subfuncid` | LONG | Unique ID for current scope |

## Declaration State

| Variable | Type | Purpose |
|----------|------|---------|
| `dimstatic` | LONG | Declaring STATIC variable |
| `dimshared` | INTEGER | Declaring SHARED variable |
| `dimoption` | INTEGER | DIM options flags |
| `redimoption` | INTEGER | REDIM options flags |
| `dimsfarray` | INTEGER | 0=var, 1=string, 2=array |
| `optionbase` | INTEGER | OPTION BASE value (0 or 1) |
| `DynamicMode` | INTEGER | 0=static arrays, 1=dynamic |

### Static/Common Array Lists

| Variable | Type | Purpose |
|----------|------|---------|
| `staticarraylist$` | STRING | Space-separated static array names |
| `staticarraylistn` | LONG | Count of static arrays |
| `commonarraylist$` | STRING | COMMON array names |
| `commonarraylistn` | LONG | Count of COMMON arrays |

## Label and GOTO Tracking

| Variable | Type | Purpose |
|----------|------|---------|
| `nLabels` | LONG | Count of labels |
| `Labels_Ubound` | LONG | Array capacity |
| `Labels(Labels_Ubound)` | Label_Type | Label registry |
| `Empty_Label` | Label_Type | Template for new labels |
| `PossibleSubNameLabels` | STRING | Potential label/sub conflicts |
| `SubNameLabels` | STRING | Confirmed sub labels |

### Label_Type Structure

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
```

## Error Handling

| Variable | Type | Purpose |
|----------|------|---------|
| `qberrorhappened` | INTEGER | Error occurred flag |
| `qberrorcode` | INTEGER | Error code |
| `qberrorline` | INTEGER | Error line number |
| `errorLineInInclude` | LONG | Line in include file |
| `IDEStartAtLine` | LONG | Resume compilation line |

## IDE Communication

| Variable | Type | Purpose |
|----------|------|---------|
| `idemode` | LONG | 0=CLI, 1=IDE mode |
| `idecommand$` | STRING | Command from IDE |
| `idereturn$` | STRING | Response to IDE |
| `ideerror` | LONG | Error flag |
| `idecompiled` | LONG | Compilation complete flag |
| `ideerrorline` | LONG | Error line for IDE |
| `idemessage$` | STRING | Error message text |

## Output Buffers

All are buffer handles (LONG) for C++ code generation:

| Buffer | Output File | Purpose |
|--------|-------------|---------|
| `GlobTxtBuf` | global.txt | Global declarations |
| `MainTxtBuf` | main0.txt | Main code |
| `DataTxtBuf` | maindata.txt | DATA initialization |
| `DataBinBuf` | data.bin | Binary data |
| `RegTxtBuf` | regsf.txt | SUB/FUNCTION registration |
| `FreeTxtBuf` | mainfree.txt | Cleanup code |
| `ErrTxtBuf` | mainerr.txt | Error handling |
| `RunTxtBuf` | runline.txt | Runtime execution |
| `ChainTxtBuf` | chain.txt | CHAIN statement |
| `InpChainTxtBuf` | inpchain.txt | CHAIN input |
| `TimeTxtBuf` | ontimer.txt | ON TIMER |
| `TimejTxtBuf` | ontimerj.txt | ON TIMER (j variant) |
| `KeyTxtBuf` | onkey.txt | ON KEY |
| `KeyjTxtBuf` | onkeyj.txt | ON KEY (j variant) |
| `StrigTxtBuf` | onstrig.txt | ON STRIG |
| `StrigjTxtBuf` | onstrigj.txt | ON STRIG (j variant) |
| `RetTxtBuf` | ret0.txt | RETURN handling |
| `VWatchMainDispatchBuf` | vw_main_dispatch.txt | $DEBUG variable watch dispatch |
| `VWatchMainSkipBuf` | vw_main_skip.txt | $DEBUG variable watch skip |
| `ExtDepBuf` | extdep.txt | External deps: ICON: (icon path), DECL: (DECLARE LIBRARY files), INCL: ($INCLUDE paths). Read by IDE/build. |
| `IncOneBuf` | incone.txt | $INCLUDEONCE guard (paths already included) |
| `FormatBuf` | format.out | Formatter output |
| `autoIncludeBuffer` | (internal) | Auto-include file list (beforefirstline.bi, color*.bi, vwatch, aftermain, afterlastline) |
| `mainincbuf` | main.txt | Aggregator: `#include "main0.txt"`, `#include "main1.txt"`, ... (written at end of codegen; qbx.cpp includes main.txt) |

## Layout/Formatting

| Variable | Type | Purpose |
|----------|------|---------|
| `layout$` | STRING | Formatted line for IDE |
| `layoutok` | LONG | Formatting complete flag |
| `layoutcomment` | STRING | Comment portion |
| `layoutcontinuations` | STRING | Line continuation text |
| `tlayout` | STRING | Temporary layout buffer |

## Character Classification (ASCII lookup tables)

| Array | Purpose |
|-------|---------|
| `isalpha(255)` | 1 if alphabetic or underscore |
| `isnumeric(255)` | 1 if digit 0-9 |
| `alphanumeric(255)` | 1 if alphanumeric or underscore |
| `lfsinglechar(255)` | 1 if single-char operator |

## RCStateVar Configuration Variables

These use the RCStateVar TYPE (wanted, actual, locked, forced) and trigger recompilation when changed:

| Variable | Purpose |
|----------|---------|
| `ColorSet` | Color mode ($COLOR:0/32) |
| `OptExpl` | Explicit optimization |
| `OptExplArr` | Explicit array optimization |
| `AssertsOn` | Assertions enabled |
| `ConsoleOn` | Console mode |
| `vWatchOn` | Variable watch/debug |
| `SockDepOn` | Socket dependency |

### RCStateVar state machine and recompilation

**TYPE RCStateVar** (source/utilities/statevars.bi):

| Field | Purpose |
|-------|---------|
| `wanted` | Value requested by the user (e.g. metacommand). |
| `actual` | Value currently in effect for this compile pass. |
| `locked` | After a successful pass, non-zero means “value applied”; next pass will keep it unless forced. |
| `forced` | Override applied at start of each (re)compile (e.g. ForceOptExpl); takes precedence over wanted/actual when set. |

**Routines** (source/utilities/statevars.bas):

- **ClearRCStateVar** – Used between `fullrecompile:` and `recompile:` only. Zeros wanted, actual, locked, forced.
- **SetRCStateVar(stVar, setVal)** – Sets `wanted = setVal`. If `actual <> wanted` and `locked = 0`, sets `recompile = 1` so a recompile is triggered at the next check.
- **ForceRCStateVar(stVar, forceVal)** – Sets `forced = forceVal`; at execute time this overrides the current value.
- **ExecuteRCStateVar(stVar)** – Called right after the `recompile:` label. If not locked, sets `actual = wanted` and, if non-zero, locks. If `forced <> 0`, sets `actual = forced`.
- **GetRCStateVar(stVar)** – Returns `actual` (the value in effect for the current pass).

**When recompile is triggered:** Any call to `SetRCStateVar` that makes `wanted <> actual` while `locked = 0` sets `recompile = 1`. The main loop checks `IF recompile GOTO do_recompile` at safe points; `do_recompile` closes buffers, sets `iderecompile = 1` in IDE mode, then `GOTO recompile`. At `recompile`, the flag is cleared and `ExecuteRCStateVar` is run for each RCStateVar, so the new pass runs with the updated feature state (e.g. $COLOR, $DEBUG, $USELIBRARY, $CONSOLE, $ASSERTS). See docs/error-handling.md for the full recompile flow.

## Command-Line Options

| Variable | Type | Purpose |
|----------|------|---------|
| `CMDLineSrcFile$` | STRING | Source file path |
| `CMDLineOutFile$` | STRING | Output file path |
| `NoIDEMode` | _BYTE | CLI-only mode |
| `ConsoleMode` | _BYTE | Console compilation |
| `FormatMode` | _BYTE | Code formatting mode |
| `NoCCompileMode` | _BYTE | Skip C++ compilation |
| `QuietMode` | _BYTE | Suppress output |
| `ShowWarnings` | _BYTE | Show all warnings |

## Variable Interactions

### Control Flow Chain
```
controllevel → controltype/id/value/state/ref arrays
```
All control structures nest together using these coordinated arrays.

### Parser State Chain
```
wholeline$ + linefragment$ + linenumber + reallinenumber
```
Updated together for each source line. Include level tracked separately.

### Identifier Chain
```
idn → ids() array ← currentid (cache)
```
All identifiers stored in `ids()`, accessed via `currentid` for performance.

### Type System Chain
```
typeDefinitions$ ↔ lasttype/lasttypeelement
udtxname/cname/size/next/variable (types)
udtename/cname/size/type/typesize/arrayelements/next (elements)
```

### Compilation Phase Chain
```
prepass → main compilation → code generation
RCStateVar changes trigger recompile
```

### Buffer Output Chain
```
All *TxtBuf buffers → assembled by Makefile → final executable
```
