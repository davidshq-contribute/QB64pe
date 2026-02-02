# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

QB64 Phoenix Edition is a self-hosted BASIC compiler that translates QB64 source code to C++, then compiles to native executables. It maintains QB4.5/QBasic compatibility while adding modern features like OpenGL support. The compiler is written in QB64 itself (bootstrapped).

## Build Commands

**Initial setup (compiles QB64pe from pre-generated C++ in `internal/source/`):**
```bash
./setup_lnx.sh      # Linux - installs dependencies and builds
./setup_osx.command  # macOS - requires Xcode CLI tools first
./setup_win.cmd      # Windows
```

**Compile a QB64 program:**
```bash
./qb64pe -x yourfile.bas                    # Compile without IDE window
./qb64pe -c yourfile.bas -o output.exe      # Compile with output name
```

**Build QB64pe itself (after bootstrap exists):**
```bash
./qb64pe -x ./source/qb64pe.bas
```

**Makefile usage (called by QB64pe, not typically run directly):**
```bash
make OS=lnx EXE=program BUILD_QB64=y    # Build QB64pe from internal/source
make OS=lnx EXE=myprogram DEP_GL=y      # Build with OpenGL support
```

Key Makefile parameters: `OS` (win/lnx/osx), `EXE`, `BUILD_QB64`, `DEP_*` flags for dependencies.

## Running Tests

```bash
tests/run_tests.sh          # QB64 + format + add-prefix tests (not C++ tests)
tests/compile_tests.sh      # QB64 compiler/language tests
tests/qbasic_tests.sh       # Compile sample QB64 programs
tests/format_tests.sh       # Formatter output tests
tests/add_prefix_test.sh    # Add-prefix (e.g. _IKW_) test
tests/run_c_tests.sh        # C++ unit tests for libqb (requires make build-tests; run separately in CI)
tests/run_dist_tests.sh     # Distribution validation
```

**Test structure in `tests/compile_tests/`:**
- `*.bas` - test source file
- `*.output` - expected stdout (for success tests)
- `*.err` - expected compiler error message (for error tests)
- `*.flags` - optional command-line arguments
- `*.compile-from-base` - marker to compile from repo root instead of test directory

## Architecture

### Compilation Pipeline

```
QB64 Source (.bas) → qb64pe.bas → C++ Source (internal/temp/) → make → Executable
```

The compiler (`source/qb64pe.bas`, ~24K lines) performs:
1. **Prepass** (prepass=1): Parse metacommands, resolve dependencies, handle auto-includes
2. **Main pass** (prepass=0): Tokenize, parse, build symbol table, generate C++ code

### Key Source Directories

- `source/qb64pe.bas` - Main compiler implementation
- `source/ide/` - Integrated Development Environment (ide_global.bas, ide_methods.bas)
- `source/subs_functions/` - QB64 built-in function implementations
- `source/utilities/` - Compiler utilities (const_eval, hash, type system)
- `internal/c/libqb/` - C++ runtime library source
- `internal/c/parts/` - Third-party dependencies (audio, graphics, networking)
- `internal/source/` - Pre-generated C++ of QB64pe (for bootstrapping)
- `internal/temp/` - Generated C++ during compilation

### Bootstrap Process

QB64pe is self-hosted. CI builds work as follows:
1. Build `qb64pe_bootstrap` from pre-generated C++ in `internal/source/`
2. Use bootstrap to compile `source/qb64pe.bas` into final `qb64pe`
3. Update `internal/source/` with newly generated C++ (auto-committed on main)

### Auto-Include System

The compiler automatically includes support files at three positions:
- **AtTop** (firstLine): Constants, type definitions - before user code
- **AfterMain** (mainEndLine): After main-level code, before SUB/FUNCTIONs
- **AtBottom** (lastLine): Support functions - after all user code

Control variables use states: 0=inactive, 1=triggered, 2=in progress, 3=done

### Key Metacommands

`$INCLUDE`, `$COLOR:0/32`, `$DEBUG`, `$VERSIONINFO`, `$USELIBRARY`, `$EMBED`, `$EXEICON`

## Runtime Dependencies

Dependencies are controlled via `DEP_*` Makefile flags:
- `DEP_GL` - OpenGL support
- `DEP_AUDIO_MINIAUDIO` - Sound via miniaudio
- `DEP_SOCKETS`, `DEP_HTTP` - Networking
- `DEP_IMAGE_CODEC` - Image loading (stb_image)
- `DEP_ZLIB` - Compression

## CI/CD

GitHub Actions builds for Linux x64, macOS x64, Windows x86/x64/ARM64. Scripts in `.ci/`:
- `bootstrap.sh/.bat` - Build bootstrap compiler
- `compile.sh/.bat` - Build final QB64pe
- `make-dist.sh` - Create distribution packages
