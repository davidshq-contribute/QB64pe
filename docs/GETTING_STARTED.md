# Getting Started with QB64-PE Development

Welcome to the QB64 Phoenix Edition (QB64-PE) project! This guide will help you get started as a new developer contributing to this open-source BASIC compiler.

## Table of Contents

1. [Welcome and Project Overview](#welcome-and-project-overview)
2. [Quick Start](#quick-start)
3. [Project Architecture Overview](#project-architecture-overview)
4. [Directory Structure](#directory-structure)
5. [Development Workflow](#development-workflow)
6. [Testing](#testing)
7. [Key Concepts for Developers](#key-concepts-for-developers)
8. [Common Development Tasks](#common-development-tasks)
9. [Resources and Next Steps](#resources-and-next-steps)
10. [Troubleshooting](#troubleshooting)

## Welcome and Project Overview

QB64 Phoenix Edition is a modern BASIC compiler that transpiles QB64/QBasic source code to C++ and then compiles it to native executables. It maintains compatibility with QB4.5/QBasic while adding modern features like OpenGL support, networking, and cross-platform capabilities.

### Key Characteristics

- **Self-Hosting Compiler**: The QB64-PE compiler is written in QB64 itself, meaning it compiles itself
- **Transpiler Architecture**: Converts BASIC source code to C++ intermediate code, then compiles to native binaries
- **Cross-Platform**: Supports Windows (7+), Linux, and macOS (Catalina+)
- **QB4.5/QBasic Compatible**: Maintains backward compatibility with classic QBasic programs
- **Modern Extensions**: Adds OpenGL, networking, audio, and other modern features
- **License**: MIT (with third-party dependencies having their own licenses - see [licenses/README.md](../licenses/README.md))

### What Makes QB64-PE Unique?

QB64-PE is a **self-hosting compiler** - the compiler that translates BASIC to C++ is itself written in BASIC. This creates an interesting bootstrap process where a pre-compiled version of the compiler (stored as C++ source in `internal/source/`) is used to compile the current version of the compiler.

## Quick Start

### Prerequisites

Before you can build QB64-PE, you'll need:

**All Platforms:**
- Git (to clone the repository)
- Make (GNU Make)
- A C++ compiler:
  - **Windows**: MinGW-w64 (automatically downloaded by setup script)
  - **Linux**: GCC (g++) - typically `build-essential` package
  - **macOS**: Xcode Command Line Tools (clang++)

**Platform-Specific:**
- **Linux**: OpenGL libraries, ALSA (audio), and development headers
- **macOS**: Xcode Command Line Tools (install with `xcode-select --install`)

### Initial Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/QB64-Phoenix-Edition/QB64pe.git
   cd QB64pe
   ```

2. **Run the setup script for your platform:**
   - **Windows**: Run `setup_win.cmd` (downloads MinGW if needed, then builds QB64-PE)
   - **Linux**: Run `./setup_lnx.sh` (installs dependencies, then builds QB64-PE)
   - **macOS**: Run `./setup_osx.command` (checks for Xcode tools, then builds QB64-PE)

3. **Wait for the build to complete** - The setup script will:
   - Install any missing dependencies
   - Compile the bootstrap version of QB64-PE from `internal/source/`
   - Use the bootstrap to compile `source/qb64pe.bas` into the final `qb64pe` executable

4. **Verify the installation:**
   ```bash
   # Windows
   qb64pe.exe -h

   # Linux/macOS
   ./qb64pe -h
   ```

### Running the IDE

After building, you can launch the integrated development environment:

```bash
# Windows
qb64pe.exe

# Linux/macOS
./qb64pe
```

From the IDE, you can:
- Edit `.bas` files with syntax highlighting
- Press **F5** to compile and run your program
- Press **F11** to compile without running (generate executable only)

### Compiling a Simple Program

Create a test file `hello.bas`:

```basic
PRINT "Hello, QB64-PE!"
END
```

Compile it from the command line:

```bash
# Windows
qb64pe.exe -c hello.bas

# Linux/macOS
./qb64pe -c hello.bas
```

Or use the `-x` flag to compile without opening a separate compiler window:

```bash
qb64pe -x hello.bas
```

> **Note**: For end-user installation instructions, see the main [README.md](../README.md).

## Project Architecture Overview

For comprehensive architecture documentation, see [ARCHITECTURE.md](ARCHITECTURE.md).

**Quick Overview:**
- QB64 Source → QB64-PE Compiler → Generated C++ → Makefile → Native Executable
- Self-hosting transpiler written in QB64
- Runtime library (`libqb`) provides core execution environment
- Conditional dependency compilation via `DEP_*` flags
- Optional IDE component for development

> **For detailed architecture information**, see [ARCHITECTURE.md](ARCHITECTURE.md).

## Directory Structure

Understanding the project layout will help you navigate the codebase:

```
QB64pe/
├── source/                 # QB64 compiler source code
│   ├── qb64pe.bas         # Main compiler entry point
│   ├── subs_functions/    # Parser and function implementations
│   │   ├── subs_functions.bas    # Core parser (large file)
│   │   ├── syntax_highlighter_list.bas  # IDE syntax highlighting
│   │   └── extensions/    # Extension system (e.g., OpenGL)
│   ├── utilities/         # Utility modules
│   │   ├── hash.bas       # Symbol table (hash-based)
│   │   ├── type.bas       # Type system
│   │   ├── const_eval.bas # Constant evaluation
│   │   ├── give_error.bas # Error reporting
│   │   └── ...            # Other utilities
│   ├── emit/              # Code generation
│   │   └── logging.bas    # Code emission
│   ├── ide/               # IDE component (optional)
│   │   ├── ide_global.bas  # IDE initialization
│   │   ├── ide_methods.bas # IDE functionality
│   │   └── ...            # IDE modules
│   └── global/            # Global constants and settings
│       ├── version.bas    # Version information
│       ├── constants.bas  # Compiler constants
│       └── settings.bas   # Compiler settings
│
├── internal/
│   ├── c/                 # C++ runtime and dependencies
│   │   ├── libqb/         # Runtime library source
│   │   │   ├── include/   # Header files (41 headers)
│   │   │   └── src/       # Implementation (48 source files)
│   │   ├── parts/         # Third-party dependencies
│   │   │   ├── audio/     # Audio libraries
│   │   │   ├── video/     # Graphics/image libraries
│   │   │   ├── network/   # Networking libraries
│   │   │   └── ...        # Other dependencies
│   │   ├── qbx.cpp        # Generated code entry point
│   │   ├── libqb.cpp      # Main runtime library file
│   │   └── c_compiler/    # MinGW (Windows only, downloaded by setup)
│   ├── source/            # Pre-compiled C++ source of QB64-PE
│   │                       # (used for bootstrap compilation)
│   └── temp/              # Temporary files during compilation
│       └── *.cpp         # Generated C++ source files
│
├── docs/                  # Documentation
│   ├── ARCHITECTURE.md    # Detailed system architecture
│   ├── build-system.md    # Build process documentation
│   ├── testing.md         # Testing framework details
│   ├── auto-including.md  # Auto-include system
│   └── GETTING_STARTED.md # This file
│
├── tests/                 # Test suites
│   ├── compile_tests/     # QB64-based compiler tests
│   │   └── [category]/   # Test categories (audio, graphics, etc.)
│   ├── c/                 # C++ runtime tests
│   │   ├── test.h         # Test framework
│   │   └── *.cpp         # Individual test files
│   ├── qbasic_testcases/  # QBasic compatibility tests
│   ├── compile_tests.sh   # Run compile tests
│   ├── run_c_tests.sh     # Run C++ tests
│   └── run_tests.sh       # Run all tests
│
├── licenses/              # Third-party license information
│   └── README.md          # License summary
│
├── .ci/                   # CI build scripts (for releases)
│   ├── bootstrap.bat      # Windows bootstrap build script
│   ├── bootstrap.sh        # Linux/macOS bootstrap build script
│   ├── compile.bat         # Windows compilation script
│   ├── compile.sh          # Linux/macOS compilation script
│   ├── calculate_version.sh # Version calculation script
│   ├── make-dist.sh        # Distribution creation script
│   └── push-internal-source.sh # Updates internal/source in CI
│
├── Makefile               # Build system for C++ compilation
├── setup_win.cmd          # Windows setup script
├── setup_lnx.sh           # Linux setup script
└── setup_osx.command      # macOS setup script
```

### Key Directories Explained

- **`source/`**: Contains all QB64 source code for the compiler itself. This is where you'll make most changes when modifying compiler behavior.

- **`internal/c/libqb/`**: The C++ runtime library. Changes here affect how compiled QB64 programs execute at runtime.

- **`internal/c/parts/`**: Third-party dependencies. Each subdirectory contains a library that can be conditionally included in compiled programs.

- **`internal/source/`**: Pre-generated C++ source of a previous version of QB64-PE, stored as `.txt` files (approximately 1,200 files). This is used to bootstrap the compiler (compile the compiler with itself). These files are automatically updated by the CI process.

- **`internal/temp/`**: Temporary directory where generated C++ code is placed during compilation of user programs. The compiler generates multiple `.txt` files containing C++ code segments that are included by `qbx.cpp` during compilation.

- **`tests/compile_tests/`**: Test cases for the compiler. Each test has a `.bas` file and either a `.output` or `.err` file showing expected results.

## Development Workflow

### Bootstrap Process

For detailed bootstrap process information, see [ARCHITECTURE.md](ARCHITECTURE.md#bootstrap-process).

**Quick Overview:**
1. Bootstrap compilation from `internal/source/` (pre-generated C++ files)
2. Self-compilation using the bootstrap compiler
3. CI updates `internal/source/` for future bootstraps

This ensures QB64-PE can always compile itself. See [ARCHITECTURE.md](ARCHITECTURE.md#bootstrap-process) for complete details.

### Making Changes to the Compiler

When modifying the QB64 compiler source code:

1. **Edit QB64 source files** in `source/` directory
2. **Rebuild QB64-PE**:
   ```bash
   # Windows
   setup_win.cmd

   # Linux
   ./setup_lnx.sh

   # macOS
   ./setup_osx.command
   ```
   
   The rebuild process will:
   - Compile the bootstrap compiler from `internal/source/` (if needed)
   - Use the bootstrap to compile your modified `source/qb64pe.bas`
   - Generate the new `qb64pe` executable
   
   **Note**: After a successful rebuild, the generated C++ source files (`.txt` files) in `internal/temp/` could be copied to `internal/source/` for future bootstraps, but this is typically only done in CI builds. The bootstrap source in the repository is sufficient for local development.

3. **Test your changes** using the test suite (see [Testing](#testing))
4. **Verify** that QB64-PE can still compile itself and compile other programs correctly

### Making Changes to the Runtime Library

When modifying the C++ runtime (`internal/c/libqb/`):

1. **Edit C++ source files** in `internal/c/libqb/src/` or headers in `internal/c/libqb/include/`
2. **Recompile a test program** - The Makefile will automatically rebuild `libqb` with your changes
3. **Run C++ tests** to verify runtime changes (see [Testing](#testing))
4. **Test with QB64 programs** that use the modified functionality

### Build Process Overview

For detailed build process information, see [build-system.md](build-system.md).

**Quick Overview:**
1. Transpilation: QB64-PE generates C++ source code
2. Dependency Detection: Compiler determines required `DEP_*` flags
3. C++ Compilation: Makefile compiles dependencies, libqb, and generated code
4. Linking: All components linked into final executable
5. Symbol Stripping: Debug symbols removed (saved to `.sym` file)

The Makefile is invoked automatically by QB64-PE. See [build-system.md](build-system.md) for complete details including Makefile parameters and CI process.

## Testing

QB64-PE has two separate testing frameworks to ensure code quality:

### QB64-Based Tests

Located in `tests/compile_tests/`, these test the compiler itself:

- **Structure**: Each test folder contains `.bas` files with corresponding `.output` or `.err` files
- **`.output` files**: Contain the expected output when the program runs
- **`.err` files**: Contain the expected compiler error message
- **`.flags` files**: Optional command-line arguments for the compiler

**Running QB64 tests:**
```bash
./tests/compile_tests.sh
```

**Writing a new test:**
1. Create a `.bas` file in an appropriate test category folder (e.g., `tests/compile_tests/audio_test/`)
2. Create a corresponding `.output` or `.err` file with the same base name
   - `.output` files contain the expected program output
   - `.err` files contain the expected compiler error message
3. Ensure output files don't have trailing spaces or empty lines at the end (these cause test failures)
4. Optionally create a `.flags` file to pass command-line arguments to the compiler

### C++-Based Tests

Located in `tests/c/`, these test the runtime library (`libqb`):

- Each test is a standalone `.cpp` file with its own `main()`
- Tests use `test.h` for test framework functionality
- Tests are compiled using `tests/build.mk`

**Running C++ tests:**
```bash
cd tests
make OS=win build-tests    # Windows: OS=win
make OS=lnx build-tests    # Linux: OS=lnx
make OS=osx build-tests   # macOS: OS=osx
./run_c_tests.sh
```

Note: Replace `OS=win`, `OS=lnx`, or `OS=osx` with the appropriate value for your platform.

**Writing a new C++ test:**
1. Create a `.cpp` file in `tests/c/`
2. Add the test executable to the list in `run_c_tests.sh`
3. Update `tests/build.mk` to build your test

### Running All Tests

```bash
./tests/run_tests.sh
```

This runs all test suites including:
- Compile tests
- C++ tests
- QBasic compatibility tests
- Distribution tests

> **For detailed testing information**, see [testing.md](testing.md).

## Key Concepts for Developers

Understanding these core concepts will help you work effectively with the QB64-PE codebase:

### Self-Hosting Compiler

QB64-PE is a **self-hosting compiler** - it's written in the language it compiles. This means:
- The compiler source is in QB64 (`source/qb64pe.bas`)
- It compiles itself using a bootstrap process
- Changes to the compiler require recompiling the compiler itself

### Transpilation Process

The compiler performs several stages to convert BASIC to C++:

1. **Lexical Analysis**: Tokenizes BASIC source code into tokens (keywords, identifiers, operators, literals)
2. **Syntax Parsing**: Builds an abstract syntax tree (AST) from tokens using recursive descent parsing
3. **Semantic Analysis**: 
   - Type checking and type inference
   - Symbol resolution (variables, functions, subroutines)
   - Scope management (global vs. local variables)
   - Constant evaluation and folding
4. **Code Generation**: Emits C++ code equivalent to the BASIC source
   - Generates function calls to `libqb` runtime functions
   - Handles type conversions automatically
   - Manages variable declarations and memory allocation

The generated C++ code is placed in `internal/temp/` and then compiled by the Makefile.

### Conditional Compilation and Dependency Flags

QB64-PE uses `DEP_*` flags to conditionally include features. These flags are automatically detected by the compiler or can be manually specified.

**Common flags:** `DEP_GL`, `DEP_IMAGE_CODEC`, `DEP_AUDIO_MINIAUDIO`, `DEP_SOCKETS`, `DEP_HTTP`, `DEP_FONT`, `DEP_ZLIB`, `DEP_CONSOLE`, `DEP_CONSOLE_ONLY`, and many more.

These flags control which parts of `libqb` are compiled, which dependencies are included, and what functions are available.

> **For a complete list of dependency flags and details**, see [build-system.md](build-system.md#makefile-usage-and-parameters).

### Symbol Table and Hash-Based Lookup

The compiler uses a hash table-based symbol lookup system (`source/utilities/hash.bas`) for:
- Variable names
- Function names
- Subroutine names
- Type definitions

This provides fast O(1) average-case lookup for symbols during compilation.

### Type System and Constant Evaluation

- **Type System** (`source/utilities/type.bas`): Manages variable types, type conversions, and type checking
- **Constant Evaluation** (`source/utilities/const_eval.bas`): Performs compile-time constant folding and evaluation
- Types include: `_BYTE`, `INTEGER`, `LONG`, `_INTEGER64`, `SINGLE`, `DOUBLE`, `STRING`, and unsigned variants

### Extension System

New functions can be added through the extension system (`source/subs_functions/extensions/`):
- OpenGL functions are implemented as extensions (see `source/subs_functions/extensions/opengl/`)
- Each extension can define new keywords, functions, and statements
- Extensions integrate with the parser (`subs_functions.bas`) and code generator
- Extensions allow modular addition of features without modifying core compiler code

## Common Development Tasks

### Adding a New BASIC Statement or Function

1. **Add parser support** in `source/subs_functions/subs_functions.bas`:
   - Add keyword recognition
   - Implement parsing logic
   - Handle syntax validation

2. **Add code generation** in the appropriate emitter:
   - Generate C++ code that calls `libqb` functions
   - Handle type conversions and validation

3. **Add runtime support** (if needed) in `internal/c/libqb/`:
   - Implement the actual functionality in C++
   - Add function declarations to headers

4. **Add tests** in `tests/compile_tests/`:
   - Create test cases for the new feature
   - Test edge cases and error conditions

5. **Update documentation**:
   - Add the new feature to the wiki or help system
   - Document syntax and usage

### Modifying the Runtime Library

1. **Edit C++ source** in `internal/c/libqb/src/` or headers in `internal/c/libqb/include/`
2. **Update feature flags** if adding new conditional compilation
3. **Add/update C++ tests** in `tests/c/`
4. **Test with QB64 programs** that use the modified functionality
5. **Rebuild** - The Makefile will automatically recompile `libqb` when needed

### Adding a New Third-Party Dependency

1. **Create directory** in `internal/c/parts/` for the new dependency
2. **Add source files** and create a `build.mk` file:
   - Define compilation rules
   - Set up conditional compilation based on `DEP_*` flags
3. **Update main Makefile** to include the new dependency
4. **Add license file** to `licenses/` directory
5. **Update** `licenses/README.md` with licensing information
6. **Add compiler support** to detect when the dependency is needed
7. **Add tests** to verify the dependency works correctly

### Debugging Compilation Issues

1. **Check compiler output**: Look for error messages and warnings
2. **Examine generated C++**: Check `internal/temp/` for the generated code
3. **Use verbose mode**: Some compiler options provide more detailed output
4. **Test incrementally**: Break down complex changes into smaller testable pieces
5. **Run relevant tests**: Use the test suite to isolate the problem
6. **Check symbol resolution**: Verify that all symbols are properly resolved

### Understanding Error Messages

**Compiler errors** (when compiling QB64 source):
- **Line number**: Where the error occurred in the source file
- **Error type**: Syntax error, type mismatch, undefined symbol, etc.
- **Context**: What the compiler was expecting vs. what it found
- **File information**: Which file contains the error (useful with `$INCLUDE`)

**Runtime errors** (from compiled QB64 programs):
- Handled by `libqb` error handling system
- Error codes and descriptive messages
- Stack traces (if available and enabled)
- File and line information (if debug symbols are present)
- ON ERROR GOTO support for error recovery

**Build errors** (C++ compilation/linking):
- Standard C++ compiler error messages
- Check `internal/temp/` for generated C++ source if needed
- Verify `DEP_*` flags are set correctly
- Check that all required dependencies are available

## Resources and Next Steps

### Documentation

- **[ARCHITECTURE.md](ARCHITECTURE.md)**: Detailed system architecture, component descriptions, and data flow
- **[build-system.md](build-system.md)**: Complete build process documentation, Makefile parameters, and CI process
- **[testing.md](testing.md)**: Testing framework details, writing tests, and test execution
- **[auto-including.md](auto-including.md)**: Automatic include file handling
- **[README.md](../README.md)**: User-focused installation and usage guide

### Community Resources

- **Forum**: [https://qb64phoenix.com/forum](https://qb64phoenix.com/forum) - Community discussions and support
- **Wiki**: [https://qb64phoenix.com/qb64wiki](https://qb64phoenix.com/qb64wiki) - Language documentation and tutorials
- **Discord**: [https://discord.gg/D2M7hepTSx](https://discord.gg/D2M7hepTSx) - Real-time chat with developers
- **Reddit**: [https://www.reddit.com/r/QB64pe/](https://www.reddit.com/r/QB64pe/) - Community discussions

### Contributing Guidelines

When contributing to QB64-PE:

1. **Test your changes**: Always run the test suite before submitting
2. **Follow code style**: Match the existing code style in the file you're editing
3. **Document changes**: Update relevant documentation when adding features
4. **Consider compatibility**: Ensure changes don't break existing QB64/QBasic programs
5. **Submit pull requests**: Use GitHub pull requests for code contributions

### Code Style and Conventions

- **QB64 code** (in `source/`):
  - Use `DEFLNG A-Z` for default variable types (long integers)
  - Comment complex logic and non-obvious code
  - Use descriptive variable names
  - Follow the existing indentation and formatting style
  - Use `$INCLUDE` for modular code organization
  
- **C++ code** (in `internal/c/libqb/`):
  - Follow standard C++ conventions
  - Use meaningful names that match existing patterns
  - Add comments for non-obvious code, especially platform-specific workarounds
  - Follow the existing structure and organization in `libqb`
  - Use conditional compilation (`#ifdef`) for platform-specific code

### Next Steps

Now that you're familiar with the project:

1. **Explore the codebase**: 
   - Start with `source/qb64pe.bas` to see the main compiler entry point
   - Browse `source/subs_functions/subs_functions.bas` to understand how statements are parsed
   - Look at `source/utilities/` to see core compiler utilities
2. **Run the tests**: Execute the test suite to see what's covered and verify your build works
3. **Try a small change**: 
   - Make a minor modification (e.g., add a comment, change an error message)
   - Rebuild and verify it still works
4. **Read the detailed docs**: Review [ARCHITECTURE.md](ARCHITECTURE.md) for deep technical details
5. **Join the community**: Participate in forum discussions or Discord to ask questions
6. **Pick an issue**: Look for beginner-friendly issues labeled "good first issue" on GitHub

## Troubleshooting

### Common Setup Issues

**"Command not found" or "Compiler not found"**
- **Windows**: Ensure MinGW was downloaded correctly by `setup_win.cmd`
- **Linux**: Install build-essential: `sudo apt-get install build-essential` (Debian/Ubuntu) or equivalent
- **macOS**: Run `xcode-select --install` to install command line tools

**"Permission denied" on setup scripts**
- **Linux/macOS**: Make scripts executable: `chmod +x setup_lnx.sh setup_osx.command`

**Antivirus blocking compilation**
- **Windows**: Whitelist the `qb64pe` folder in your antivirus software
- The compiler generates executables, which some antivirus software flags

### Build Failures

**Bootstrap compilation fails**
- Check that `internal/source/` contains the pre-generated C++ source files (`.txt` files, approximately 1,200 files)
- Verify your C++ compiler is working: `g++ --version` or `clang++ --version`
- Check for disk space issues
- On Windows, ensure MinGW was properly downloaded and extracted
- Try running `make clean OS=[your-os]` and rebuilding
- Verify the Makefile can find the files in `internal/source/`

**Self-compilation fails after making changes**
- Review your changes for syntax errors in the QB64 source
- Check compiler error messages carefully - they point to the problematic code
- Verify that you haven't broken the bootstrap process (the bootstrap should still work)
- Try compiling a simple test program to see if the issue is specific to self-compilation
- Check `internal/temp/` for generated C++ `.txt` files to see if the transpilation succeeded
- Try reverting recent changes to isolate the problem
- Ensure you're using the correct bootstrap (the one in `internal/source/` should work)

**"Cannot find libqb" or linking errors**
- Ensure `internal/c/libqb/` is present
- Check that the Makefile can find required files
- Verify `DEP_*` flags are set correctly
- The `libqb` object file name depends on the `DEP_*` flags used - ensure consistency
- Check that the C++ compiler can find all required libraries

### Platform-Specific Considerations

**Windows:**
- Use `setup_win.cmd` which handles MinGW installation automatically
- Ensure you have write permissions in the installation directory
- Paths with spaces may cause issues - avoid them if possible

**Linux:**
- Different distributions may need different packages
- Check `setup_lnx.sh` for package names specific to your distribution
- Some distributions may need additional development packages
- Common requirements: `build-essential`, `libgl1-mesa-dev`, `libasound2-dev`
- The setup script attempts to auto-detect your package manager and install dependencies

**macOS:**
- Xcode Command Line Tools are required (install with `xcode-select --install`)
- macOS Catalina (10.15) or later is required
- Some versions may need specific compiler flags
- The setup script will check for and prompt to install command line tools if missing

### Getting Help

If you encounter issues not covered here:

1. **Check existing issues**: Search [GitHub issues](https://github.com/QB64-Phoenix-Edition/QB64pe/issues) for similar problems
2. **Ask on the forum**: Post your question with:
   - Error messages (full output)
   - Your operating system and version
   - Steps you've already tried
   - Relevant code or configuration
3. **Join Discord**: Get real-time help from the community
4. **Review documentation**: Check the detailed docs for your specific area:
   - [ARCHITECTURE.md](ARCHITECTURE.md) for system internals
   - [build-system.md](build-system.md) for build issues
   - [testing.md](testing.md) for test-related questions

---

**Welcome to QB64-PE development!** We're glad to have you here. Don't hesitate to ask questions and contribute to making QB64-PE better for everyone.
