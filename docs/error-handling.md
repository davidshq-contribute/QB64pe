# Compiler Error Handling Flow

How QB64-PE reports compilation errors, when recompilation is triggered, and how the IDE receives error messages. For runtime error codes, see [ERROR-CODES.md](../ERROR-CODES.md).

## Overview

Compiler errors flow through:

1. **Detection** – Parser/codegen sets error message (via `Give_Error` or direct `a$ = "..."` then `GOTO errmes`).
2. **Reporting** – `errmes` sets `ideerrorline` and `idemessage$` (IDE mode) or prints to console (CLI).
3. **IDE delivery** – Label `ideerror` sends command 8 (error) to the IDE: `CHR$(8) + idemessage$ + MKL$(ideerrorline)`.
4. **Recompilation** – Flag `recompile = 1` causes a jump to `do_recompile` at safe points; `do_recompile` closes buffers and `GOTO recompile` to start a fresh pass.

## Error detection and message setup

### Give_Error (utilities/give_error.bas)

```basic
SUB Give_Error (a$)
    Error_Happened = 1
    Error_Message = a$
END SUB
```

- Used when a utility or helper needs to signal an error without knowing the current line.
- Caller sets the message in `a$`, then calls `Give_Error(a$)`. Execution continues until code reaches `errmes`.

### errmes (qb64pe.bas)

Code that detects an error either:

- Calls `Give_Error(a$)`, then later execution hits `errmes`, or  
- Sets `a$` to the message and does `GOTO errmes` directly.

At **errmes**:

1. If `Error_Happened` is set, `a$` is replaced with `Error_Message` and `Error_Happened` is cleared.
2. Layout is invalidated (`layout$ = ""`, `layoutok = 0`).
3. If the error occurred while auto-including (e.g. vwatch, library), the message may be rewritten and `erldiff` adjusted so the reported line is correct.
4. If `inclevel > 0`, include context is appended to `a$` via `incerror$`.
5. **IDE mode:** `ideerrorline = linenumber + erldiff`, `idemessage$ = a$`, then `GOTO ideerror`.
6. **CLI mode:** Error is printed to the console and the compiler typically stops.

So: **Give_Error** only stores message and a flag; **errmes** is the single place that assigns `idemessage$`/`ideerrorline` or prints and decides IDE vs CLI.

## IDE error delivery

When `idemode` is set and execution reaches **ideerror**:

1. If `idemessage$` contains the internal separator `sp$` (e.g. from an unexpected runtime error), the message is replaced with a generic “Compiler error (check for syntax errors)” line and `_ERRORLINE` / `_INCLERRORLINE` are appended.
2. The IDE is sent a single block: **command byte 8** (error), then the message string, then the 4-byte line number:  
   `sendc$ = CHR$(8) + idemessage$ + MKL$(ideerrorline)`.
3. Control goes to `sendcommand`, which sends `sendc$` to the IDE. The IDE displays the message and line number.

See [ARCHITECTURE.md](../ARCHITECTURE.md) for the full IDE command list (e.g. 8 = error).

## qberror* variables (runtime / C++ compile errors)

These are used when the **compiler itself** is running (e.g. ON ERROR in the QB64 compiler, or C++ compile failure), not for normal “syntax error” reporting:

| Variable        | Purpose |
|----------------|---------|
| `qberrorhappened` | Flag: error occurred in compiler or C++ build (values like -3, -2, -1, 1 used in different paths). |
| `qberrorcode`  | Error code (e.g. from `ERR`). |
| `qberrorline`  | Line number associated with the error. |

The IDE also has:

- `ideerror`, `ideerrorline`, `idemessage` – set when the C++ compiler or other external step fails; `ideerrorline = 0` is used for C++ compile errors so the IDE can show its own mapping.

So: **normal compile errors** go through `errmes` → `ideerrorline`/`idemessage$` → command 8. **Compiler/C++ failures** use `qberror*` and/or `ideerror`/`ideerrorline`/`idemessage` in separate paths.

## When recompilation is triggered

The global flag **`recompile`** (integer) is set to 1 when the compiler must run another pass (e.g. after a metacommand or RCStateVar change). It is **not** set for ordinary syntax errors; those report and exit (or stop the current compile).

Typical triggers for `recompile = 1`:

- **RCStateVar change** – e.g. `$COLOR:0` / `$COLOR:32`, `$DEBUG`, `$USELIBRARY`, `$CONSOLE`, `$ASSERTS`. See [GLOBALS.md](../GLOBALS.md) “RCStateVar and recompilation”.
- **Metacommands** that alter auto-include or feature state (the same ones that use `SetRCStateVar` or otherwise request another pass).

At certain points in the main loop the compiler checks `IF recompile GOTO do_recompile`. In **do_recompile**:

1. `recompile = 0`.
2. If `idemode`, `iderecompile = 1` so the IDE knows a recompile was requested.
3. Buffers are closed and temp files re-locked.
4. Control does `GOTO recompile` (the label after `recompile: recompile = 0`), so a new pass starts with a clean state (e.g. `ExecuteRCStateVar` for each RCStateVar, hash cleared, etc.).

So: **syntax/semantic errors** → `errmes` → IDE message 8 or console print; **feature/metacommand changes** → `recompile = 1` → later `do_recompile` → `recompile` label and a new pass.

## fullrecompile vs recompile

- **fullrecompile** – Used when a completely fresh compile is needed (e.g. new source file, or after IDE “new compilation” command). Resets library list, clears all RCStateVars, then falls into the same flow that eventually hits **recompile**.
- **recompile** – Start of a new pass without changing source file or full reset; `recompile` flag is cleared, RCStateVars are executed, then compilation continues.

So fullrecompile does a heavier reset; recompile is “run another pass with current source and state.”
