# INDEX.md

Master index to QB64 Phoenix Edition documentation.

## Getting Started

| Document | Description |
|----------|-------------|
| [README.md](README.md) | Project overview and quick start |
| [CLAUDE.md](CLAUDE.md) | Build commands and project summary |
| [MIGRATION.md](MIGRATION.md) | Guide for QBasic/QB45 users |
| [QUICK-REFERENCE.md](QUICK-REFERENCE.md) | One-page cheat sheet |
| [GLOSSARY.md](GLOSSARY.md) | Terminology definitions |

## Architecture

| Document | Description |
|----------|-------------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | High-level system overview |
| [ARCHITECTURE-BASIC.md](ARCHITECTURE-BASIC.md) | QB64 compiler and IDE internals |
| [ARCHITECTURE-C.md](ARCHITECTURE-C.md) | C++ runtime library internals |
| [COMPILER-PIPELINE.md](COMPILER-PIPELINE.md) | Compilation phases and code generation |
| [BUILD-SYSTEM.md](BUILD-SYSTEM.md) | Makefile and dependency system |
| [GLOBALS.md](GLOBALS.md) | Global variables reference |

## Language Features

### Type System
| Document | Description |
|----------|-------------|
| [TYPE-SYSTEM.md](TYPE-SYSTEM.md) | Type encoding, suffixes, UDTs |
| [ARRAYS.md](ARRAYS.md) | Array declaration and storage |
| [ARRAY-IMPLEMENTATION.md](ARRAY-IMPLEMENTATION.md) | Internal array representation |
| [STRING-INTERNALS.md](STRING-INTERNALS.md) | String management (qbs structure) |

### Control Flow
| Document | Description |
|----------|-------------|
| [CONTROL-FLOW.md](CONTROL-FLOW.md) | IF, FOR, DO, SELECT statements |
| [SUBROUTINES.md](SUBROUTINES.md) | SUB/FUNCTION definitions |
| [EVENTS.md](EVENTS.md) | Event-driven programming |

### Expressions
| Document | Description |
|----------|-------------|
| [EXPRESSION-EVALUATION.md](EXPRESSION-EVALUATION.md) | Operator precedence, parsing |
| [CONST-EVAL.md](CONST-EVAL.md) | Compile-time constant evaluation |
| [BINARY-OPS.md](BINARY-OPS.md) | Bitwise operations |
| [MATH-FUNCTIONS.md](MATH-FUNCTIONS.md) | Mathematical functions |

### Data
| Document | Description |
|----------|-------------|
| [DATA-STATEMENTS.md](DATA-STATEMENTS.md) | DATA statement syntax |
| [DATA-RESTORE.md](DATA-RESTORE.md) | DATA/READ/RESTORE implementation |

## Built-in Functions

| Document | Description |
|----------|-------------|
| [BUILTIN-FUNCTIONS.md](BUILTIN-FUNCTIONS.md) | Function registration system |
| [DATETIME.md](DATETIME.md) | Date, time, and timing functions |
| [ENVIRONMENT.md](ENVIRONMENT.md) | Environment variables, paths |

## Graphics

| Document | Description |
|----------|-------------|
| [GRAPHICS.md](GRAPHICS.md) | Graphics system overview |
| [SCREEN-MODES.md](SCREEN-MODES.md) | SCREEN statement and modes |
| [COLOR-SYSTEM.md](COLOR-SYSTEM.md) | Color handling and palettes |
| [COORDINATE-SYSTEMS.md](COORDINATE-SYSTEMS.md) | VIEW, WINDOW, coordinate mapping |
| [IMAGE-FORMATS.md](IMAGE-FORMATS.md) | Image loading and saving |
| [FONTS.md](FONTS.md) | Font loading and rendering |
| [OPENGL.md](OPENGL.md) | OpenGL integration |
| [WINDOW-MANAGEMENT.md](WINDOW-MANAGEMENT.md) | Window control functions |

## Audio

| Document | Description |
|----------|-------------|
| [AUDIO.md](AUDIO.md) | Modern audio system (miniaudio) |
| [SOUND-LEGACY.md](SOUND-LEGACY.md) | Legacy SOUND/PLAY/BEEP |

## Input/Output

| Document | Description |
|----------|-------------|
| [FILE-IO.md](FILE-IO.md) | File operations |
| [INPUT-DEVICES.md](INPUT-DEVICES.md) | Keyboard, mouse, controllers |
| [USER-INPUT.md](USER-INPUT.md) | INPUT, LINE INPUT, INKEY$ |
| [PRINT-FORMATTING.md](PRINT-FORMATTING.md) | PRINT USING formatting |
| [CONSOLE.md](CONSOLE.md) | Console mode operations |
| [CLIPBOARD.md](CLIPBOARD.md) | Clipboard access |
| [NETWORKING.md](NETWORKING.md) | TCP/UDP networking |
| [PRINTING.md](PRINTING.md) | Printer output |

## Memory

| Document | Description |
|----------|-------------|
| [MEMORY-BLOCKS.md](MEMORY-BLOCKS.md) | _MEM system |
| [LEGACY-COMPAT.md](LEGACY-COMPAT.md) | PEEK/POKE, DEF SEG, CMEM |

## Compiler Features

| Document | Description |
|----------|-------------|
| [METACOMMANDS.md](METACOMMANDS.md) | $INCLUDE, $DEBUG, etc. |
| [PREPROCESSOR.md](PREPROCESSOR.md) | $IF, $ELSE conditional compilation |
| [CODEGEN-PATTERNS.md](CODEGEN-PATTERNS.md) | C++ code generation patterns |
| [DECLARE-LIBRARY.md](DECLARE-LIBRARY.md) | External library bindings |
| [LIBRARY-SYSTEM.md](LIBRARY-SYSTEM.md) | $USELIBRARY implementation |

## Development Tools

| Document | Description |
|----------|-------------|
| [IDE-ARCHITECTURE.md](IDE-ARCHITECTURE.md) | IDE implementation |
| [IDE-PROTOCOL.md](IDE-PROTOCOL.md) | IDE/compiler communication |
| [DEBUGGING.md](DEBUGGING.md) | Debugging techniques |
| [DEBUG-VWATCH.md](DEBUG-VWATCH.md) | Variable watch system |
| [ASSERTIONS.md](ASSERTIONS.md) | $ASSERTS system |

## Error Handling

| Document | Description |
|----------|-------------|
| [ERROR-HANDLING.md](ERROR-HANDLING.md) | ON ERROR, error trapping |
| [ERROR-CODES.md](ERROR-CODES.md) | Error code reference |
| [COMMON-PITFALLS.md](COMMON-PITFALLS.md) | Known gotchas and issues |

## Additional Documentation

| Document | Description |
|----------|-------------|
| [docs/README.md](docs/README.md) | Developer documentation index |
| [docs/auto-including.md](docs/auto-including.md) | Auto-include system details |
| [docs/build-system.md](docs/build-system.md) | Build system internals |
| [docs/testing.md](docs/testing.md) | Test suite documentation |
| [docs/support-files.md](docs/support-files.md) | Support file reference |
| [docs/error-handling.md](docs/error-handling.md) | Error handling internals |

## By Topic

### For Users
- Start with [MIGRATION.md](MIGRATION.md) if coming from QBasic
- [QUICK-REFERENCE.md](QUICK-REFERENCE.md) for common patterns
- [ERROR-CODES.md](ERROR-CODES.md) for troubleshooting

### For Contributors
- [ARCHITECTURE.md](ARCHITECTURE.md) for system overview
- [COMPILER-PIPELINE.md](COMPILER-PIPELINE.md) for code generation
- [COMMON-PITFALLS.md](COMMON-PITFALLS.md) for known issues
- [docs/testing.md](docs/testing.md) for test suite

### For Runtime Developers
- [ARCHITECTURE-C.md](ARCHITECTURE-C.md) for C++ internals
- [STRING-INTERNALS.md](STRING-INTERNALS.md) for string handling
- [MEMORY-BLOCKS.md](MEMORY-BLOCKS.md) for _MEM system
