# ENVIRONMENT.md

Documentation of the QB64pe environment and system interaction functions.

## Overview

QB64pe provides cross-platform access to environment variables, command line arguments, shell execution, file dialogs, and system information.

## ENVIRON$ Function

```basic
value$ = ENVIRON$("VARNAME")
value$ = ENVIRON$(n%)
```

Uses C `getenv()` function.

## ENVIRON Statement

```basic
ENVIRON "NAME=VALUE"
ENVIRON "NAME="        ' Remove
```

Platform-specific: `setenv()`/`_putenv()`.

## COMMAND$ Function

```basic
all$ = COMMAND$
arg$ = COMMAND$(n%)
count% = _COMMANDCOUNT
```

- `COMMAND$(0)` = Program name
- `COMMAND$(1)` = First argument

## SHELL Statement

| Syntax | Description |
|--------|-------------|
| `SHELL cmd$` | Execute and wait |
| `SHELL _HIDE cmd$` | Hidden |
| `SHELL _DONTWAIT cmd$` | Background |
| `exitcode% = SHELL(cmd$)` | Get exit code |

## _STARTDIR$ / _CWD$

```basic
start$ = _STARTDIR$  ' Initial directory
curr$ = _CWD$        ' Current directory
```

## _DIR$

```basic
path$ = _DIR$("documents")
path$ = _DIR$("temp")
path$ = _DIR$("home")
```

Returns system directories.

## _OS$

```basic
os$ = _OS$
' "[WINDOWS][64BIT]"
' "[LINUX][64BIT][ARM]"
' "[MACOSX][LINUX][64BIT]"
```

## File Dialogs

```basic
file$ = _OPENFILEDIALOG$(title$, path$, filter$, desc$, multi%)
file$ = _SAVEFILEDIALOG$(title$, path$, filter$, desc$)
folder$ = _SELECTFOLDERDIALOG$(title$, path$)
```

Uses tinyfiledialogs library.

## END Statement

```basic
END
END exitcode%
```

Closes files, shows "Press any key" prompt.

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/src/environ.cpp` | ENVIRON |
| `internal/c/libqb/src/command.cpp` | COMMAND$ |
| `internal/c/libqb/src/shell.cpp` | SHELL |
| `internal/c/libqb/src/filesystem.cpp` | Directories |
| `internal/c/parts/gui/gui.cpp` | Dialogs |

## Example Usage

```basic
' Environment
PRINT ENVIRON$("PATH")
ENVIRON "MYVAR=Hello"

' Command line
FOR i = 1 TO _COMMANDCOUNT
    PRINT COMMAND$(i)
NEXT

' Shell
exitcode% = SHELL("myprogram.exe")
SHELL _DONTWAIT "notepad.exe"

' Dialogs
file$ = _OPENFILEDIALOG$("Open", "", "*.txt", "Text")
```
