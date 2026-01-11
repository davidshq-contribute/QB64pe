# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

QB64 Phoenix Edition (QB64-PE) is a modern BASIC compiler that transpiles QB64/QBasic source code to C++ and then compiles it to native executables. It maintains compatibility with QB4.5/QBasic while adding modern features.

**Architecture**: BASIC → C++ (transpiler) → Native Binary
**Platforms**: Windows (7+), Linux, macOS (Catalina+)
**Language**: The compiler itself is written in QB64/BASIC

## Build Commands

### Building QB64-PE from Source

**Linux/macOS:**
```bash
./setup_lnx.sh    # Linux
./setup_osx.command   # macOS
```

**Windows:**
```cmd
setup_win.cmd
```

### Compiling Programs with QB64-PE

```bash
# Launch IDE
./qb64pe

# Compile without IDE (show progress window)
./qb64pe -c program.bas

# Compile to console (no progress window)
./qb64pe -x program.bas -o output_name

# Format code
./qb64pe -y source.bas -o formatted.bas

# Generate C++ without compiling (for debugging)
./qb64pe -z program.bas
```

### Running Tests

```bash
# All tests
./tests/run_tests.sh

# Specific test categories
./tests/compile_tests.sh ./qb64pe        # Compiler tests
./tests/run_c_tests.sh                   # C++ runtime tests
./tests/qbasic_tests.sh ./qb64pe         # QBasic compatibility

# Unit tests (must build first)
./qb64pe -x tests/unit/test_runner.bas
./test_runner

# Test discovery and filtering
./tests/run_tests_with_discovery.sh --category unit
./tests/run_tests_with_discovery.sh --tag error

# Continuous testing during development
./tests/continuous_test.sh --watch --parallel 4
```

### Build System (for development)

The Makefile is called internally by QB64-PE but can be used directly:

```bash
# Build QB64-PE bootstrap from internal/source/
make OS=lnx BUILD_QB64=y EXE=qb64pe_bootstrap

# Build with specific dependencies
make OS=lnx EXE=myprogram DEP_GL=y DEP_AUDIO_MINIAUDIO=y

# Build C++ runtime tests
make build-tests OS=lnx
```

## High-Level Architecture

### Compilation Pipeline

```
QB64 Source (.bas)
    ↓
QB64 Compiler (source/qb64pe.bas)
    ↓
Generated C++ (.txt files in internal/temp/)
    ↓
Makefile + C++ Compiler
    ↓
Native Executable
```

### Key Components

1. **QB64 Compiler** (`source/qb64pe.bas`)
   - Written in QB64 itself (self-hosting)
   - Performs lexical analysis, parsing, semantic analysis, and C++ code generation
   - Main parser in `source/subs_functions/subs_functions.bas` (~3,865 lines)

2. **Runtime Library** (`internal/c/libqb/`)
   - C++ runtime providing core execution environment
   - ~41 header files, ~48 implementation files
   - Includes: string handling (qbs), memory management, graphics, I/O, threading

3. **Third-Party Dependencies** (`internal/c/parts/`)
   - Organized by category: audio, core (OpenGL), data, gui, input, network, video
   - Each has its own `build.mk` file
   - Compiled conditionally based on `DEP_*` flags

4. **Build System** (`Makefile`)
   - Platform-specific (Windows/Linux/macOS)
   - Conditional dependency compilation
   - Links everything into final executable

### Bootstrap Process

QB64-PE is self-hosting - it compiles itself:

1. `internal/source/` contains pre-generated C++ from previous QB64-PE build (~1,200 .txt files)
2. C++ compiler builds `qb64pe_bootstrap` from these files
3. `qb64pe_bootstrap` compiles `source/qb64pe.bas` → final `qb64pe` executable
4. CI auto-updates `internal/source/` when compiler changes

**Why .txt extension?** Generated C++ files use `.txt` extension by convention to distinguish them from hand-written source. The C++ preprocessor includes them via `#include` directives in `qbx.cpp`.

### Multi-Pass Compilation

The compiler uses multiple passes to handle forward references and feature activation:

- **fullrecompile:** Complete reset of compiler state
- **recompile:** Incremental recompilation with preserved state
- **RCStateVar System** (`statevars.bas`): Tracks feature activation (color sets, OPTION _EXPLICIT, assertions, console mode, debugging) and triggers recompiles when state changes

### Auto-Include System

QB64-PE automatically injects support files at three positions:

1. **AtTop** (before first user line): Constants, types, library headers
   - `beforefirstline.bi`: Core constants (_TRUE, _FALSE, etc.)
   - Color constants (`color0.bi` or `color32.bi`)
   - Library "AtTop" files (from `$USELIBRARY`)

2. **AfterMain** (before first SUB/FUNCTION): Global data, GOSUB routines, error handlers
   - `aftermain.bas`: Implicit END injection
   - Library "AfterMain" files

3. **AtBottom** (after last user line): SUB/FUNCTION implementations
   - `vwatch.bm`: Debug support (if `$DEBUG` used)
   - Library "AtBottom" files
   - `afterlastline.bm`: QB64-PE support functions

## Code Organization

### Compiler Source (`source/`)

```
source/
├── qb64pe.bas              # Main compiler entry point
├── global/                 # Global constants, settings, version
├── utilities/              # Compiler utilities
│   ├── hash.bas/bi         # Symbol table (hash-based lookup)
│   ├── type.bas/bi         # Type system
│   ├── const_eval.bas/bi   # Compile-time constant evaluation
│   ├── statevars.bas/bi    # Feature activation & recompile system
│   ├── give_error.bas/bi   # Error reporting
│   ├── include_provider.bas/bi  # Abstract include file system (testability)
│   ├── build.bas           # Build system integration
│   ├── strings.bas         # String utilities
│   ├── format.bas          # Code formatting
│   └── parser_utils.bas    # Parser helpers
├── subs_functions/
│   ├── subs_functions.bas  # Main parser (~3,865 lines)
│   └── extensions/         # Extensible function systems (e.g., OpenGL)
├── ide/                    # IDE component (optional)
│   ├── ide_methods.bas     # IDE main logic
│   ├── config/             # IDE settings
│   └── wiki/               # Built-in help
└── emit/
    └── logging.bas         # C++ code generation
```

### Runtime Library (`internal/c/libqb/`)

**Core modules:**
- **String System** (`qbs*.cpp`): QB64 string type with reference counting, copy-on-write
- **Memory Management** (`mem.cpp`): Dynamic allocation, memory locks, common memory (DBLOCK)
- **Graphics** (`graphics.cpp`): Screen/image operations, OpenGL integration
- **File System** (`filesystem.cpp`, `filepath.cpp`): Cross-platform file I/O
- **Threading** (`threading.cpp`): Platform-specific implementations
- **Error Handling** (`error_handle.cpp`): ON ERROR GOTO support
- **Utilities**: datetime, environ, command, shell, string functions, etc.

### Testing Infrastructure (`tests/`)

```
tests/
├── compile_tests/          # Language feature tests (.bas + .output/.err)
├── unit/                   # Component unit tests (type system, parser, etc.)
├── integration/            # End-to-end compiler tests
├── c/                      # C++ runtime tests (test.h framework)
├── qbasic_testcases/       # QBasic compatibility tests
├── dist/                   # Distribution tests
└── *.sh                    # Test runners and utilities
```

**Test Discovery System**: `test_discovery.sh` auto-discovers, categorizes, and filters tests. Supports filtering by category, tag, pattern, or path.

**Continuous Testing**: `continuous_test.sh` provides watch mode, incremental testing, and parallel execution.

## Development Patterns

### QB64 Language Quirks

**GOTO Labels in SUB/FUNCTION:**
- GOTO labels are NOT allowed inside SUB/FUNCTION definitions
- Error: "Common label within a SUB/FUNCTION"
- Solution: Refactor to structured control flow (IF/ELSEIF/ELSE, DO...LOOP)
- See `docs/problems_encountered/qb64_goto_labels_in_included_functions.md`

**Implicit END Injection:**
- QB64 injects implicit END before first SUB/FUNCTION
- Affects file organization in test code
- Solution: Three-phase include structure (declarations → main code → implementations)

**Include Files:**
- `.bi` files: CONST, TYPE, DIM SHARED, DECLARE only (like C headers)
- `.bas` files: Main program code
- `.bm` files: SUB/FUNCTION definitions only
- `$INCLUDEONCE` prevents multiple inclusion

### Testing Guidelines

1. **Unit Tests**: Test compiler components in isolation using `tests/unit/test_framework.bi`
2. **Component Test Harness**: Use Test State Manager for state isolation
3. **Include Provider**: Abstract include file system enables testing without filesystem
4. **C++ Runtime Tests**: Use `tests/c/test.h` framework for libqb testing

### Symbol Table System

- **Hash-based lookup**: Fast O(1) symbol access using 64MB lookup table
- **Symbol types**: Variables, functions, subroutines, constants, UDTs, labels
- **Symbol flags**: HASHFLAG_VARIABLE, HASHFLAG_FUNCTION, etc.
- Implementation in `source/utilities/hash.bas`

### Dependency System

The compiler detects required features and sets `DEP_*` flags:

- `DEP_GL`: OpenGL support
- `DEP_AUDIO_MINIAUDIO`: Audio playback
- `DEP_SOCKETS`: Networking
- `DEP_HTTP`: HTTP client (requires DEP_SOCKETS)
- `DEP_IMAGE_CODEC`: Image loading/saving
- `DEP_FONT`: Font rendering
- And more...

Build system compiles only needed dependencies, reducing executable size.

### Code Review Process

From `.cursor/commands/code-review.md`:
- Review all code changes for bugs, bad practices, lost functionality
- Document challenging problems in `docs/problems_encountered/` for future reference

## Important Files

- **docs/ARCHITECTURE.md**: Comprehensive architecture documentation
- **docs/build-system.md**: Build system details
- **docs/auto-including.md**: Auto-include mechanism
- **docs/testing/testing.md**: Testing infrastructure
- **Makefile**: Build orchestration (called by qb64pe internally)
- **internal/source/**: Bootstrap files (pre-generated C++ of QB64-PE)
- **internal/temp/**: Generated C++ for current compilation (not in git)

## CI/CD Process

1. Build `qb64pe_bootstrap` from `internal/source/`
2. Use bootstrap to compile `source/qb64pe.bas` → final `qb64pe`
3. Run test suite
4. Update `internal/source/` if compiler changed
5. Create platform-specific distributions
6. Auto-commit `internal/source/` changes to git

## Platform Differences

- **Windows**: Bundled MinGW compiler in `internal/c/c_compiler/` (not in git, downloaded during setup)
- **Linux**: Uses system GCC/Clang, X11 for windowing, ALSA for audio
- **macOS**: Uses system Clang, Cocoa/AppKit for GUI, CoreAudio for audio

## Adding New Features

**New Compiler Functions:**
1. Implement in `source/subs_functions/subs_functions.bas` or create extension
2. Register with `regid` system
3. Generate C++ calls
4. Implement runtime in `libqb` if needed

**New Features with State Management:**
1. Define `RCStateVar` variable in `source/qb64pe.bas`
2. Add state management in `fullrecompile:` and `recompile:` sections
3. Use `SetRCStateVar()` when feature activates
4. Check with `GetRCStateVar()` when needed

**New Dependencies:**
1. Add source to `internal/c/parts/[category]/[name]/`
2. Create `build.mk` for compilation
3. Add `DEP_[NAME]` flag to Makefile
4. Update compiler dependency detection

## Common Gotchas

- **Generated files**: `internal/temp/*.txt` are regenerated each compilation (not in git)
- **Bootstrap files**: `internal/source/*.txt` ARE in git (required for bootstrap)
- **Multiple passes**: Compiler may recompile multiple times due to RCStateVar system
- **Self-hosting**: Changes to compiler require recompiling QB64-PE itself
- **Platform-specific code**: Check `OS` detection in Makefile and source code
