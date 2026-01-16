# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

QB64 Phoenix Edition (QB64-PE) is a modern BASIC compiler that extends QB4.5/QBasic with OpenGL support. It compiles native binaries for Windows, Linux, and macOS.

**Key insight:** QB64-PE is written in QB64 BASIC itself and bootstraps via pre-compiled C++ source stored in `internal/source/`.

## Build Commands

### Build QB64-PE from source

```bash
# Windows
setup_win.cmd

# Linux
./setup_lnx.sh

# macOS
./setup_osx.command
```

### Manual build (if needed)

```bash
# Windows
internal\c\c_compiler\bin\mingw32-make.exe OS=win BUILD_QB64=y

# Linux
make OS=lnx BUILD_QB64=y

# macOS
make OS=osx BUILD_QB64=y
```

### Compile a QB64 program

```bash
qb64pe -c yourfile.bas -o output.exe    # Opens compiler window
qb64pe -x yourfile.bas -o output.exe    # Silent compilation
```

## Testing

```bash
# Run all tests
./tests/run_tests.sh

# Individual test suites
./tests/compile_tests.sh      # QB64 compiler tests
./tests/run_c_tests.sh        # C++ unit tests for libqb
./tests/qbasic_tests.sh       # QBasic compatibility tests
```

### Test structure
- `tests/compile_tests/*.bas` - Compiler tests with matching `.output` or `.err` files
- `tests/c/*.cpp` - C++ unit tests using `test.h` framework
- `tests/qbasic_testcases/` - Sample programs that must compile successfully

## Architecture

### Two-Stage Compilation
1. QB64-PE compiles `.bas` source to C++ (output in `internal/temp/`)
2. Make compiles C++ to native binary with dependencies based on `DEP_*` flags

### Key Directories
- `source/` - QB64-PE compiler source (qb64pe.bas is the main file)
- `internal/c/libqb/` - Modularized core runtime library (48 modules)
- `internal/c/libqb.cpp` - Monolithic core runtime (~23K lines, being modularized)
- `internal/c/parts/` - Third-party dependencies (audio, fonts, networking, etc.)
- `internal/source/` - Pre-compiled C++ of QB64-PE (for bootstrapping)
- `internal/temp/` - Compilation workspace (generated files)
- `.ci/` - CI/CD build scripts

### libqb Modularization
The runtime library is being modularized from `libqb.cpp` into `libqb/src/*.cpp`. Completed modules:
- `fileio.cpp` - File I/O operations
- `graphics.cpp` - Drawing primitives, DRAW, _PUTIMAGE, GET/PUT
- `screen.cpp` - Display management, fullscreen, resize
- `color.cpp` - RGB/RGBA, palette operations
- `keyboard.cpp`, `mouse.cpp`, `console.cpp`, `mem_legacy.cpp`

Deferred modules (complex global state dependencies): Text/Font, Input, Window/UI, Control Flow

### Dependency Flags
The Makefile uses `DEP_*` flags to conditionally compile features:
- `DEP_GL` - OpenGL support
- `DEP_AUDIO_MINIAUDIO` - Sound support
- `DEP_IMAGE_CODEC` - Image loading
- `DEP_FONT` - Font rendering
- `DEP_SOCKETS`, `DEP_HTTP` - Networking
- `DEP_CONSOLE`, `DEP_CONSOLE_ONLY` - Console mode

### Global State
Key globals shared across modules (defined in `libqb.cpp`):
- `img[]`, `nextimg`, `pages[]` - Image system
- `font[]`, `fontwidth[]`, `fontheight[]` - Font system
- `display_page_index`, `write_page_index` - Page state
- `environment_2d_*` - Display scaling/positioning

## CI/CD Process

1. `bootstrap.bat/sh` - Build `qb64pe_bootstrap` from `internal/source/`
2. `compile.bat/sh` - Use bootstrap to compile `source/qb64pe.bas`
3. Tests run against compiled QB64-PE
4. `make-dist.sh` - Create distribution packages
5. `internal/source/` updated if changed (auto-committed on main)

## Versioning

Format: `X.Y.Z` for releases, `X.Y.Z-NN-XXXXXXXX` for CI builds (commits since release + hash)
- Version defined in `source/global/version.bas`
- CI builds write to `internal/version.txt`

## Documentation

- `docs/build-system.md` - Detailed build process and Makefile parameters
- `docs/testing.md` - Test framework documentation
- `docs/modernization-roadmap.md` - Comprehensive modernization roadmap including libqb modularization plan, extraction progress, and lessons learned
- `docs/auto-including.md` - Auto-include feature for $USELIBRARY
