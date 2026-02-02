# ARCHITECTURE.md

High-level architecture overview of QB64 Phoenix Edition.

## What is QB64pe?

QB64pe is a **self-hosted transpiler** that converts QB64 BASIC source code to C++, then compiles to native executables. It maintains compatibility with QBasic/QB4.5 while adding modern features (OpenGL, networking, audio).

## Compilation Pipeline

```
QB64 Source (.bas)
       │
       ▼
┌──────────────────┐
│   qb64pe.bas     │  ← QB64 compiler (written in QB64)
│   (Transpiler)   │
└──────────────────┘
       │
       ▼
C++ Source (internal/temp/*.txt)
       │
       ▼
┌──────────────────┐
│   Makefile       │  ← Orchestrates C++ compilation
│   + g++/clang    │
└──────────────────┘
       │
       ▼
┌──────────────────┐
│   libqb + deps   │  ← Runtime library + dependencies
└──────────────────┘
       │
       ▼
Native Executable
```

## Bootstrap Architecture

QB64pe compiles itself (self-hosted). The bootstrap process:

1. **internal/source/** contains pre-generated C++ from a previous QB64pe version
2. Build `qb64pe_bootstrap` from this C++ using only a C++ compiler
3. Use bootstrap to compile `source/qb64pe.bas` into the real `qb64pe`
4. CI auto-updates `internal/source/` when main branch changes

This allows building QB64pe on any system with just a C++ compiler.

## Major Components

| Component | Location | Language | Purpose |
|-----------|----------|----------|---------|
| Compiler | `source/qb64pe.bas` | QB64 | Transpiles QB64 → C++ |
| IDE | `source/ide/` | QB64 | Integrated development environment |
| Runtime | `internal/c/libqb/` | C++ | QB64 standard library functions |
| Dependencies | `internal/c/parts/` | C/C++ | Audio, graphics, networking, etc. |
| Build System | `Makefile` | Make | Compiles C++ to executables |

See [ARCHITECTURE-BASIC.md](ARCHITECTURE-BASIC.md) for compiler/IDE details.
See [ARCHITECTURE-C.md](ARCHITECTURE-C.md) for runtime/dependencies details.

## Directory Structure

```
QB64pe/
├── source/                 # QB64 source code (the compiler itself)
│   ├── qb64pe.bas         # Main compiler (~24K lines)
│   ├── ide/               # IDE implementation
│   ├── subs_functions/    # Built-in function definitions
│   ├── utilities/         # Compiler utilities
│   └── global/            # Global constants and version
│
├── internal/
│   ├── c/                 # C/C++ runtime code
│   │   ├── libqb/        # QB64 runtime library
│   │   ├── parts/        # Third-party dependencies
│   │   ├── qbx.cpp       # Generated code entry point
│   │   └── libqb.cpp     # Runtime compilation unit
│   ├── source/           # Pre-generated C++ for bootstrap
│   ├── support/          # Auto-included support files
│   │   ├── include/      # beforefirstline.bi, aftermain.bas, afterlastline.bm
│   │   ├── color/        # color0.bi, color32.bi
│   │   ├── vwatch/       # vwatch.bi, vwatch.bm, vwatch_stub.bm ($DEBUG)
│   │   └── converter/    # AddPREFIX.bas, QB45BIN.bas
│   └── temp/             # Generated C++ during compilation (temp2, ... for multi-IDE)
│
├── tests/                 # Test suites
│   ├── compile_tests/    # QB64 language tests
│   ├── c/                # C++ unit tests
│   └── qbasic_testcases/ # Sample program compilation
│
├── .ci/                   # CI/CD scripts
├── Makefile              # Build orchestration
└── setup_*.sh/.cmd       # Platform setup scripts
```

## Compiler Phases

The QB64 compiler (`qb64pe.bas`) operates in two main passes:

### Pre-pass (prepass=1)
- Parse metacommands (`$INCLUDE`, `$COLOR`, `$DEBUG`, `$VERSIONINFO`)
- Handle auto-includes (inject support code transparently)
- Resolve `$USELIBRARY` dependencies
- Build initial symbol table

### Main Pass (prepass=0)
- Tokenize source lines into space-delimited elements
- Parse statements and expressions
- Resolve variable types and array dimensions
- Generate C++ code to output buffers
- Handle control flow (IF/FOR/DO/SELECT)

## Auto-Include System

QB64pe automatically injects support code at three positions:

```
┌─────────────────────────────────────┐
│ AtTop (firstLine)                   │  ← Constants, type definitions
│   beforefirstline.bi                │
│   color0.bi / color32.bi            │
│   Library .bi files                 │
│   vwatch.bi (if $DEBUG)             │
├─────────────────────────────────────┤
│ User's Main Program Code            │
│   CONST, DIM, TYPE...               │
│   executable code...                │
│   DATA statements...                │
│   SUB/FUNCTION definitions...       │
├─────────────────────────────────────┤
│ AfterMain (mainEndLine)             │  ← Implicit END, library code
│   aftermain.bas                     │
│   Library .bas files                │
├─────────────────────────────────────┤
│ AtBottom (lastLine)                 │  ← Support functions
│   vwatch.bm (if $DEBUG)             │
│   Library .bm files                 │
│   afterlastline.bm                  │
└─────────────────────────────────────┘
```

Control variables (`firstLine`, `mainEndLine`, `lastLine`) use states:
- 0 = inactive
- 1 = triggered
- 2 = in progress
- 3 = done

## IDE/Compiler Communication

When running in IDE mode, the compiler and IDE communicate via a command protocol:

| Command | Direction | Purpose |
|---------|-----------|---------|
| 0 | IDE→Compiler | No IDE present (array returns 0) |
| 1 | Compiler→IDE | Open file (first call only) |
| 2 | IDE→Compiler | Begin new compilation |
| 3 | Compiler→IDE | Request next line |
| 4 | IDE→Compiler | Return next line |
| 5 | IDE→Compiler | No more lines |
| 6 | Compiler→IDE | Compilation OK |
| 7 | Compiler→IDE | Rewind for repass |
| 8 | Compiler→IDE | Error occurred |
| 9 | IDE→Compiler | C++ compile and run |
| 10 | Compiler→IDE | Request specific line |
| 11 | Compiler→IDE | ".EXE file created" message |
| 12 | Compiler→IDE | Report exe name |
| 100 | Compiler→IDE | Simplified next line request |
| 254 | Compiler→IDE | Launch debug interface |
| 255 | Compiler→IDE | QB error in IDE |

## Dependency System

Runtime features are controlled by `DEP_*` Makefile flags that:
1. Set C preprocessor defines (`-DDEPENDENCY_*`)
2. Include/exclude source files in compilation
3. Link appropriate libraries

The `libqb` object file is named with a bit pattern encoding enabled features (e.g., `libqb_make_00010100.o`) allowing multiple configurations to coexist.

## Platform Support

| Platform | Compiler | Graphics | Audio | Notes |
|----------|----------|----------|-------|-------|
| Windows | MinGW (bundled) | FreeGLUT + GLEW | miniaudio | Ships pre-compiled |
| Linux | System GCC | FreeGLUT + GLEW | miniaudio | Requires setup script |
| macOS | Xcode Clang | FreeGLUT + GLEW | miniaudio | Requires Xcode CLI tools |

## Versioning

- Release versions: `X.Y.Z` (SemVer)
- CI versions: `X.Y.Z-NN-XXXXXXXX` (commits since release + commit hash)
- Local builds: `X.Y.Z-UNKNOWN`

Version is defined in `source/global/version.bas` and embedded via `$VERSIONINFO`.
