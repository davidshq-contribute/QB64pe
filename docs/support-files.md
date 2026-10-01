# Support Files (internal/support/)

The `internal/support/` directory holds QB64-PE auto-included support files. They are injected at specific positions during compilation; see [auto-including.md](auto-including.md) for the overall flow and state machine.

**Important:** These files are reserved for QB64-PE itself. Do not add personal or third-party code here; use `$USELIBRARY` and the library pack for extensions.

## Directory layout

```
internal/support/
├── include/     # AtTop / AfterMain / AtBottom includes
├── color/       # $COLOR:0 and $COLOR:32 palette files
├── vwatch/      # $DEBUG variable watch (see DEBUG-VWATCH.md)
└── converter/   # Add-prefix and QB45 conversion utilities (not auto-included)
```

## include/

| File | Include position | Purpose |
|------|------------------|---------|
| `beforefirstline.bi` | **AtTop** (before first user line) | Common constants and type definitions. Implements language-level features that require underscore-prefixed names. Must work with OPTION _EXPLICIT; all CONST/SUB/FUNCTION/TYPE/variable names must begin with `_` (except $LET vars, UDT elements, labels). Becomes “line zero” of each program. |
| `aftermain.bas` | **AfterMain** (after main-level code) | Injects an implicit `END` so execution does not fall through into library or AtBottom code if the user omits END/SYSTEM. Should contain nothing else. |
| `afterlastline.bm` | **AtBottom** (after last user line) | New commands or helper SUBs/FUNCTIONs that are implemented in QB64 rather than C/C++. Same naming rules as beforefirstline.bi (underscore prefix). Becomes the final line of each program. |

When the user is editing one of these files in the IDE, that file is not auto-included for that compile so the user’s edits are used.

## color/

| File | When included | Purpose |
|------|----------------|--------|
| `color0.bi` | When `$COLOR:0` is used and not being edited in IDE | 8-color (legacy) palette constants. |
| `color32.bi` | When `$COLOR:32` is used and not being edited in IDE | 32-bit color mode constants. |

Only one of `$COLOR:0` or `$COLOR:32` may be used in a program.

## vwatch/

Used when `$DEBUG` is set. See [DEBUG-VWATCH.md](../DEBUG-VWATCH.md) for full details.

| File | Include position | Purpose |
|------|------------------|---------|
| `vwatch.bi` | AtTop (after color) | Variable declarations for the debug/watch system (e.g. `vwatch_linenumber`, `vwatch_breakpoints()`). |
| `vwatch.bm` | AtBottom (before afterlastline.bm) | Debug runtime SUB: breakpoints, variable watch, stepping, call stack. Not included when the compiler is building vwatch itself. |
| `vwatch_stub.bm` | AtBottom (when building vwatch or when $DEBUG not used) | No-op stub so that code that references vwatch still compiles. |

## converter/

These are **not** auto-included into user programs. They are standalone utilities used for repository/maintenance tasks.

| File | Purpose |
|------|---------|
| `AddPREFIX.bas` | Converts QB64 keywords to prefixed forms (e.g. `_IKW_`). Used for compatibility and testing; see `tests/add_prefix_test.sh` and `tests/converter_tests/`. |
| `QB45BIN.bas` | QB45 binary/conversion utility. |

Tests that cover converter behavior live under `tests/converter_tests/` and are run via `tests/add_prefix_test.sh` (see [testing.md](testing.md)).
