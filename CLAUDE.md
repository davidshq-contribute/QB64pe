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

For comprehensive architecture documentation, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

**Quick Overview:**
- QB64 Source → QB64 Compiler → Generated C++ → Makefile + C++ Compiler → Native Executable
- Self-hosting compiler written in QB64
- Runtime library (`libqb`) provides core execution environment
- Conditional dependency compilation via `DEP_*` flags

**Key Documentation:**
- **Architecture Details**: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) - Complete system architecture, components, and data flow
- **Build System**: [docs/build-system.md](docs/build-system.md) - Build process, Makefile parameters, and CI process
- **Auto-Include System**: [docs/auto-including.md](docs/auto-including.md) - Auto-include mechanism details

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

### Testing Infrastructure

For comprehensive testing documentation, see [docs/testing/TESTING_IMPLEMENTATION.md](docs/testing/TESTING_IMPLEMENTATION.md).

**Quick Overview:**
- **Compile Tests**: Language feature tests
- **Unit Tests**: Component unit tests (type system, parser, etc.)
- **Integration Tests**: End-to-end compiler tests
- **Runtime Tests**: C++ runtime tests
- **Test Discovery**: Automatic test discovery and filtering
- **Continuous Testing**: Watch mode, incremental testing, parallel execution

**Key Documentation:**
- **Testing Implementation**: [docs/testing/TESTING_IMPLEMENTATION.md](docs/testing/TESTING_IMPLEMENTATION.md) - Complete testing strategy and implementation
- **Test Discovery**: [docs/testing/TEST_DISCOVERY.md](docs/testing/TEST_DISCOVERY.md) - Test discovery system details
- **Continuous Testing**: [docs/testing/CONTINUOUS_TESTING.md](docs/testing/CONTINUOUS_TESTING.md) - Continuous testing features
- **Component Testing**: [docs/testing/COMPONENT_TESTING_STRATEGY.md](docs/testing/COMPONENT_TESTING_STRATEGY.md) - Component testing strategy
- **Testing Overview**: [docs/testing/testing.md](docs/testing/testing.md) - Testing framework overview

## Development Patterns

### QB64 Language Quirks

**GOTO Labels in SUB/FUNCTION:**
- GOTO labels are NOT allowed inside SUB/FUNCTION definitions
- Error: "Common label within a SUB/FUNCTION"
- Solution: Refactor to structured control flow (IF/ELSEIF/ELSE, DO...LOOP)
- See `docs/problems_encountered/qb64_goto_labels_in_included_functions.md`

**GOTO Label Refactoring (2026-01-10):**
- All GOTO labels have been eliminated from utility files to enable test compilation
- **Files Refactored:**
  - `source/utilities/hash.bas` - 6 GOTO labels replaced with DO...LOOP
  - `source/utilities/include_provider.bas` - 2 error handler GOTOs replaced with pre-validation
  - `source/utilities/elements.bas` - 4 GOTO labels replaced with DO...LOOP
- **Total:** 12 GOTO labels eliminated across 3 files
- **Pattern:** All refactored to structured control flow (DO WHILE loops, IF/ELSEIF/ELSE)
- **Result:** Test compilation now succeeds at 100%, all 73 unit tests pass
- See `docs/REFACTORING_LOG.md` for detailed refactoring notes

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
- **docs/testing/TESTING_IMPLEMENTATION.md**: Complete testing strategy and implementation
- **docs/testing/testing.md**: Testing framework overview
- **docs/GETTING_STARTED.md**: Getting started guide for developers
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
