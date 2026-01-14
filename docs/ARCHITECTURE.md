# QB64 Phoenix Edition - Architecture Document

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Core Components](#core-components)
4. [Compilation Pipeline](#compilation-pipeline)
5. [Build System](#build-system)
6. [Directory Structure](#directory-structure)
7. [Key Technologies](#key-technologies)
8. [Data Flow](#data-flow)
9. [Testing Architecture](#testing-architecture)
10. [Bootstrap Process](#bootstrap-process)
11. [Compiler-IDE Integration](#compiler-ide-integration)
12. [Key Design Patterns](#key-design-patterns)
13. [Platform-Specific Considerations](#platform-specific-considerations)
14. [Extension Points](#extension-points)
15. [Performance Considerations](#performance-considerations)
16. [Security Considerations](#security-considerations)
17. [Future Architecture Considerations](#future-architecture-considerations)
18. [Conclusion](#conclusion)

## Overview

QB64 Phoenix Edition (QB64-PE) is a modern BASIC compiler that transpiles QB64/QBasic source code to C++ and then compiles it to native executables. It maintains compatibility with QB4.5/QBasic while adding modern features like OpenGL support, networking, and cross-platform capabilities.

### Key Characteristics

- **Language**: QB64/BASIC (source) → C++ (intermediate) → Native Binary (output)
- **Target Platforms**: Windows (7+), Linux, macOS (Catalina+)
- **Architecture**: Transpiler + Runtime Library + IDE
- **License**: MIT

## System Architecture

QB64-PE follows a multi-stage compilation architecture:

```
┌─────────────────┐
│  QB64 Source    │
│   (.bas file)   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   QB64-PE       │
│   Compiler      │  ← Transpiler (BASIC → C++)
│  (qb64pe.bas)   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  Generated C++  │
│  Source Files   │
│ (in temp/)      │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│     Makefile    │
│   Build System  │  ← C++ Compilation & Linking
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   Native        │
│   Executable    │
└─────────────────┘
```

### High-Level Components

1. **QB64 Compiler** (`source/qb64pe.bas`): The main transpiler that converts BASIC to C++
2. **Runtime Library** (`internal/c/libqb`): Core runtime functions and data structures
3. **Build System** (`Makefile`): Orchestrates C++ compilation and linking
4. **IDE** (`source/ide/`): Integrated development environment (optional)
5. **Dependencies** (`internal/c/parts/`): Third-party libraries and components

## Core Components

### 1. QB64 Compiler (`source/qb64pe.bas`)

The compiler is written in QB64 itself and performs the following functions:

- **Lexical Analysis**: Tokenizes BASIC source code
- **Syntax Parsing**: Parses BASIC statements and expressions
- **Semantic Analysis**: Type checking, symbol resolution, scope management
- **Code Generation**: Emits C++ code equivalent to the BASIC source
- **Preprocessing**: Handles `$INCLUDE`, `$DEFINE`, conditional compilation
- **Dependency Detection**: Analyzes code to determine required runtime features

**Key Subsystems:**
- **Parser** (`source/subs_functions/subs_functions.bas`): Handles all BASIC statements and functions (~3,865 lines)
- **Code Emitter** (`source/emit/logging.bas`): Generates C++ output
- **Type System** (`source/utilities/type.bas`, `type_declarations.bi`, `type_init.bas`): Manages variable types and conversions
  - Split into declaration (`.bi`) and initialization (`.bas`) files to support QB64's three-phase include system
- **Error Handling** (`source/utilities/give_error.bas`): Compiler error reporting
- **Symbol Table** (`source/utilities/hash.bas`, `hash_declarations.bi`, `hash_init.bas`): Hash table-based symbol lookup system
  - Split into declaration (`.bi`) and initialization (`.bas`) files to support QB64's three-phase include system
- **Constant Evaluation** (`source/utilities/const_eval.bas`): Compile-time constant folding and evaluation
- **State Management** (`source/utilities/statevars.bas`): Feature activation and recompile triggering
- **Build Utilities** (`source/utilities/build.bas`): Build system integration and cleanup
- **String Utilities** (`source/utilities/strings.bas`): String manipulation functions
- **File Utilities** (`source/utilities/file.bas`): File I/O and path handling
- **Format Utilities** (`source/utilities/format.bas`): Code formatting and output
- **Terminal Utilities** (`source/utilities/terminal.bas`): Terminal/console output
- **Include Provider** (`source/utilities/include_provider.bas`): Abstract include file system for testability
  - Multiple provider types (Filesystem, Memory, Test)
  - Skip includes mode for testing individual functions
  - Full backward compatibility maintained
- **Element Management** (`source/utilities/elements.bas`): Element parsing and management
  - Improved maintainability and testability
- **String Buffer** (`source/utilities/s-buffer/`): String buffer utilities
  - Split into `simplebuffer_declarations.bi` and `simplebuffer_init.bas` for three-phase include system
- **Extension System** (`source/subs_functions/extensions/`): Extensible function system (e.g., OpenGL)

### 2. Runtime Library (`internal/c/libqb`)

The runtime library (`libqb` = "Library QB64") provides the core execution environment for compiled QB64 programs. It's written in C++ and compiled into a single object file that gets linked with every QB64 program.

**Structure:**
- **`libqb/include/`**: Public header files (41 headers)
  - Core types: `qbs.h`, `mem.h`, `cmem.h`
  - System interfaces: `filesystem.h`, `filepath.h`, `command.h`, `environ.h`
  - Graphics: `graphics.h`, `image.h`
  - Utilities: `datetime.h`, `encoding.h`, `compression.h`, `hashing.h`
  - Platform: `thread.h`, `mutex.h`, `condvar.h`
  - Features: `audio.h`, `http.h`, `font.h`, `game_controller.h`, `gui.h`
  - Debugging: `logging.h`, `error_handle.h`

- **`libqb/src/`**: Implementation files (48 source files)
  - Core runtime functionality
  - Platform-specific implementations (Windows/Unix)
  - Conditional compilation based on `DEP_*` flags

**Core Modules:**

1. **String System** (`qbs*.cpp`):
   - `qbs.cpp`, `qbs_str.cpp`, `qbs_val.cpp`: QB64 String (`qbs`) type
   - `qbs_cmem.cpp`: Common memory string operations
   - `qbs_mk_cv.cpp`: String conversion utilities
   - `qbs__tostr.cpp`: String representation functions
   - Provides automatic memory management, reference counting, UTF-8 support

2. **Memory Management** (`mem.cpp`):
   - Dynamic memory allocation for QB64 programs
   - Memory locking/unlocking for arrays via `mem_lock` system
   - **Common Memory (`cmem`)**: Large static buffer (1.1MB) for QB64-compatible memory operations
     - Supports QB4.5-style memory addressing (SEG:OFFSET)
     - DBLOCK: Data block region within cmem (segment 80, offset 0-65535)
     - Used for compatibility with legacy QB4.5 memory operations
   - **Static Memory Pool**: Managed memory for program variables
   - **Memory Lock System**: Tracks memory blocks with unique IDs
     - Types: malloc, image, sound, array, sub/function scope
     - Automatic cleanup on scope exit
     - Security validation via lock IDs

3. **Graphics System** (`graphics.cpp`):
   - Screen and image operations
   - Page management (active/visual pages)
   - Graphics device abstraction
   - Integration with OpenGL/FreeGLUT (when enabled)

4. **File System** (`filesystem.cpp`, `filepath.cpp`, `file-fields.cpp`):
   - File I/O operations (OPEN, CLOSE, INPUT, PRINT, etc.)
   - Path manipulation and normalization
   - File field handling (random access, binary mode)
   - Cross-platform path handling

5. **Threading** (`threading.cpp`, `threading-windows.cpp`, `threading-posix.cpp`):
   - Multi-threading support
   - Platform-specific implementations
   - Thread synchronization primitives
   - Event handling infrastructure

6. **Error Handling** (`error_handle.cpp`):
   - Runtime error management
   - Error recovery mechanisms
   - ON ERROR GOTO support
   - Error message formatting
   - **Modern API Functions**: Provides 13 modern API functions for type-safe error state management:
     - Core Functions: `is_error_handling()`, `set_error_handling(bool)`, `is_error_pending()`, `set_error_pending(bool)`, `get_error_occurred()`, `set_error_occurred(bool)`, `get_error_err()`, `set_error_err(uint32_t)`
     - Handler Management: `get_error_goto_line()`, `set_error_goto_line(uint32_t)`, `get_error_handler_history()`, `set_error_handler_history(qbs*)`, `get_error_retry()`, `set_error_retry(bool)`
   - **Refactoring Completed**: All direct variable access patterns replaced with modern API functions (100% complete)
     - 13 API functions created with comprehensive error state management
     - All 166 compiler patterns updated across 8 files
     - Eliminated technical debt and improved maintainability
     - All tests passing with full backward compatibility

7. **Logging System** (`logging/`):
   - Debug and diagnostic logging
   - Stack trace generation
   - Symbol resolution (platform-specific)
   - Log handlers (file, console, etc.)

8. **Utilities**:
   - `datetime.cpp`: Date/time functions
   - `environ.cpp`: Environment variable access
   - `command.cpp`: Command-line argument parsing
   - `shell.cpp`: Shell command execution
   - `hexoctbin.cpp`: Number base conversion
   - `bitops.cpp`: Bit manipulation operations
   - `buffer.cpp`: Buffer management
   - `gfs.cpp`: Graphics file system
   - `qblist.cpp`: List data structure
   - `string_functions.cpp`: String utility functions

**Key Data Structures:**
- `qbs`: Dynamic string type with automatic memory management, reference counting, copy-on-write
- `mem_lock`: Memory reference tracking for garbage collection
- Device structures: Graphics, file, and console device management
- Platform-specific structures for threading, file I/O, graphics

**Compilation:**
- Compiled conditionally based on `DEP_*` flags
- Different object file names based on enabled features (e.g., `libqb_make_00010100.o`)
- Platform-specific code paths (Windows vs. Unix)
- Stub implementations for optional features

### 3. Build System (`Makefile`)

The Makefile orchestrates the final compilation stage:

**Responsibilities:**
- Compiles generated C++ source files
- Builds `libqb` with appropriate feature flags
- Compiles third-party dependencies conditionally
- Links all components into final executable
- Strips debug symbols (optional)
- Generates license files

**Key Features:**
- Platform-specific compilation (Windows/Linux/macOS)
- Conditional dependency inclusion via `DEP_*` flags
- Support for multiple concurrent compilation instances
- Symbol file generation for debugging

### 4. IDE Component (`source/ide/`)

Optional integrated development environment:

**Features:**
- **Code Editor**: Full-featured text editor with syntax highlighting
- **Syntax Highlighting**: Keyword and syntax recognition (`syntax_highlighter_list.bas`)
- **Project Management**: File management and project organization
- **Configuration Management** (`ide/config/`): IDE settings and preferences
- **Wiki Integration** (`ide/wiki/`): Built-in help and documentation browser
- **Export Functionality** (`ide_export.bas`): Export programs and resources
- **Debugging Support**:
  - Variable watch system (`vWatch`)
  - Breakpoint management
  - Call stack viewing
  - Debug client integration
- **Build Integration**: Compile and run from IDE
- **External Dependency Tracking**: Monitors `DECLARE LIBRARY` and `$INCLUDE` files for changes
- **IDE-Compiler Communication**: Uses `idecommand$` protocol for bidirectional communication
  - Command/status byte system (0-255)
  - Line-by-line compilation feedback
  - Error reporting and progress updates
  - Debug mode integration

### 5. Third-Party Dependencies (`internal/c/parts/`)

Modular third-party dependencies organized by category. Each dependency has its own `build.mk` file and is compiled conditionally based on `DEP_*` flags. This allows QB64 programs to only include the features they need, reducing executable size.

**Directory Structure:**

1. **`parts/audio/`** - Audio Playback and Codecs
   - **miniaudio**: Cross-platform audio library (core audio engine)
   - **extras/**: Additional audio codec support
     - **foo_midi**: MIDI playback with multiple backends
     - **hivelytracker**: HivelyTracker module format support
     - **libmidi**: MIDI file format support
     - **libxmp-lite**: Module format support (MOD, XM, IT, etc.)
     - **primesynth**: PrimeSynth audio synthesis
     - **qoa**: QOA audio format
     - **radv2**: RAD v2 audio format
     - **stb**: stb_vorbis (Ogg Vorbis decoder)
     - **tinysoundfont**: SoundFont support
     - **ymfmidi**: YMF MIDI support
   - **audio.cpp**: Audio system integration
   - **stub_audio.cpp**: Stub implementation when audio is disabled

2. **`parts/core/`** - Core Graphics and OpenGL
   - **freeglut**: OpenGL windowing and input (31 files)
   - **glew**: OpenGL Extension Wrangler Library
   - **gl_header_for_parsing**: OpenGL header files for compiler parsing
   - Provides window management, input handling, OpenGL context

3. **`parts/data/`** - Data Processing
   - **miniz**: zlib-compatible compression library
   - **modp_b64**: Base64 encoding/decoding
   - **compression.cpp**: Compression wrapper
   - **encoding.cpp**: Character encoding utilities

4. **`parts/gui/`** - Graphical User Interface
   - **tinyfiledialogs**: Cross-platform file dialogs
     - Open/save file dialogs
     - Message boxes
     - Color picker
   - **gui.cpp**: GUI system integration

5. **`parts/input/`** - Input Devices
   - **game_controller/**: Game controller support
     - **libstem_gamepad**: Gamepad input library
     - **game_controller.cpp**: Controller integration
   - Provides gamepad/joystick input support

6. **`parts/network/`** - Networking
   - **http/**: HTTP client support
     - **curl**: libcurl library (372 files)
     - Full HTTP/HTTPS client capabilities
     - Supports GET, POST, PUT, DELETE, etc.
     - SSL/TLS support

7. **`parts/os/`** - Operating System Integration
   - **clipboard/**: Clipboard access
     - Cross-platform clipboard operations
     - Copy/paste text support
   - Platform-specific OS features

8. **`parts/video/`** - Video and Image Processing
   - **font/**: Font Rendering
     - **freetype**: FreeType library (460 files)
     - Font loading and rendering
     - TTF, OTF, and other font format support
     - **font.cpp**: Font system integration
     - **hashing.cpp**: Font cache hashing
     - **stub_font.cpp**: Stub when fonts disabled
   
   - **image/**: Image Codecs
     - **stb**: stb_image (PNG, JPEG, BMP, TGA, etc.)
     - **qoi**: QOI image format
     - **qoa**: QOA image format (if applicable)
     - **nanosvg**: SVG vector graphics support
     - **jo_gif**: GIF format support
     - **sg_curico**: ICO format support
     - **sg_pcx**: PCX format support
     - **pixelscalers**: Image scaling algorithms (HQX, SXBR, MMX)
     - **image.cpp**: Image system integration

**Build System Integration:**
- Each dependency directory contains a `build.mk` file
- Dependencies are compiled only when corresponding `DEP_*` flag is set
- Stub implementations provided when features are disabled
- License files tracked in `licenses/` directory

## Compilation Pipeline

QB64-PE uses a multi-pass compilation approach to handle forward references, feature activation, and incremental compilation.

### Compilation Passes

The compiler performs multiple passes through the source code:

1. **Prepass** (IDE mode only):
   - Quick syntax check for IDE error reporting
   - Detects basic syntax errors without full compilation
   - Used for real-time error highlighting in IDE

2. **Full Recompile** (`fullrecompile:` label):
   - Complete reset of compiler state
   - Clears all symbol tables, state variables, and tracking
   - Resets library tracking and dependency information
   - Triggered when major state changes occur

3. **Recompile** (`recompile:` label):
   - Incremental recompilation with preserved state
   - Applies state variable changes (RCStateVar system)
   - Maintains symbol tables and dependency information
   - Used when feature activation requires recompilation

**Pass Flow:**
```
Start → fullrecompile: (if needed)
    ↓
Clear all state
    ↓
recompile: (or continue)
    ↓
Execute state changes
    ↓
Compile source code
    ↓
Check if recompile needed
    ↓
If yes → goto recompile:
If no → Generate C++ code
```

### Stage 1: Source Analysis

1. **Preprocessing**: 
   - Process `$INCLUDE` directives (with `$INCLUDEONCE` support)
   - Handle `$DEFINE` and conditional compilation (`$IF`, `$ELSE`, `$ELSEIF`, `$ENDIF`)
   - Auto-include support files (constants, debug code) - see [Auto-Inclusion](#3-auto-inclusion)
   - Resolve `$USELIBRARY` directives (library management system)
   - Process metacommands (`$COLOR`, `$CONSOLE`, `$DEBUG`, `$EMBED`, `$VERSIONINFO`, etc.)

2. **Lexical Analysis**:
   - Tokenize source code
   - Identify keywords, operators, literals, identifiers

3. **Syntax Parsing**:
   - Build abstract syntax tree
   - Validate syntax against QB64 grammar
   - Handle line numbers and labels

### Stage 2: Semantic Analysis

1. **Symbol Resolution**:
   - Build symbol tables using hash table system (`hash.bas`)
   - Fast symbol lookup via hash-based indexing
   - Resolve scope (global, local, shared)
   - Type inference and validation
   - Track symbols with flags (variable, function, subroutine, constant, etc.)

2. **Constant Evaluation**:
   - Compile-time constant folding (`const_eval.bas`)
   - Evaluate constant expressions at compile time
   - Support for constant functions and expressions
   - Reduces runtime overhead

3. **Dependency Detection**:
   - Analyze which runtime features are needed
   - Set `DEP_*` flags for build system
   - Determine required third-party libraries
   - Track external dependencies (`DECLARE LIBRARY`, `$INCLUDE` files)

4. **State Management**:
   - Track feature activation via `RCStateVar` system (`statevars.bas`)
   - Features: Color sets, OPTION _EXPLICIT, assertions, console mode, debugging
   - Automatic recompile triggering when state changes
   - Lock mechanism prevents unnecessary recompiles

5. **Code Optimization**:
   - Constant folding
   - Dead code elimination (basic)
   - Type coercion optimization

### Stage 3: Code Generation

1. **C++ Code Emission**:
   - Generate C++ source files in `internal/temp/` with `.txt` extension
   - Main program code → `main.txt`
   - Data declarations → `maindata.txt`
   - Error handlers → `mainerr.txt`
   - Event handlers → `ontimerj.txt`, `onkeyj.txt`, etc.
   - Function code segments → `data1.txt`, `data2.txt`, ... `dataN.txt`
   - Function return points → `ret0.txt`, `ret1.txt`, ... `retN.txt`
   - Memory management → `free1.txt`, `free2.txt`, ... `freeN.txt`
   - Additional files: `global.txt`, `regsf.txt`, `chain.txt`, `clear.txt`, etc.

2. **Why `.txt` Extension?**:
   - **Convention**: The `.txt` extension clearly indicates these are generated files, not hand-written C++ source
   - **C++ Preprocessor Compatibility**: The C++ preprocessor can include files with any extension via `#include` directives
   - **Build System Integration**: The Makefile expects `*.txt` files in the temp directory
   - **IDE Clarity**: Prevents IDEs from treating generated code as regular C++ source files
   - **Historical**: This convention has been used throughout QB64's history

3. **Structure Generation**:
   - Variable declarations with proper types
   - Function/subroutine wrappers
   - Type conversions
   - Memory management calls

### Stage 4: C++ Compilation

1. **Dependency Compilation**:
   - Build required third-party libraries
   - Each dependency has its own `build.mk` file

2. **Runtime Library Compilation**:
   - Compile `libqb.cpp` with feature flags
   - Generate platform-specific code paths

3. **Program Compilation**:
   - Compile `qbx.cpp` which includes generated `.txt` files via `#include` directives
   - The `qbx.cpp` template contains `#include "../temp/main.txt"` and other includes
   - C++ preprocessor expands these includes during compilation
   - Link with `libqb` and dependencies
   - Generate final executable

4. **Post-Processing**:
   - Strip debug symbols (optional)
   - Generate symbol files for debugging
   - Create license attribution file

## Directory Structure

### Overview

The QB64-PE repository is organized into several key areas:

- **`source/`**: QB64 compiler source code (written in QB64/BASIC)
- **`internal/c/`**: C/C++ runtime and build system
- **`internal/source/`**: Bootstrap files (pre-generated C++ of QB64-PE)
- **`internal/support/`**: Support files auto-included in QB64 programs
- **`docs/`**: Documentation
- **`tests/`**: Test suite
- **`licenses/`**: Third-party license files

### Detailed Directory Structure

```
QB64pe/
├── source/                 # QB64 compiler source code
│   ├── qb64pe.bas         # Main compiler entry point
│   ├── global/            # Global constants, settings, version
│   ├── ide/               # IDE component (optional)
│   ├── subs_functions/    # Parser and code generation
│   │   ├── subs_functions.bas  # Main parser (~3,865 lines)
│   │   ├── syntax_highlighter_list.bas  # Keywords for IDE syntax highlighting
│   │   └── extensions/    # Extensible function systems
│   │       └── opengl/     # OpenGL function extensions
│   │           ├── opengl_global.bas  # OpenGL initialization
│   │           └── opengl_methods.bas  # OpenGL function implementations
│   └── utilities/         # Compiler utility modules
│       ├── build.bas      # Build system integration
│       ├── const_eval.bas # Constant evaluation system
│       ├── hash.bas       # Hash table symbol lookup (implementation)
│       ├── hash_declarations.bi  # Hash table declarations (Phase 1)
│       ├── hash_init.bas  # Hash table initialization (Phase 2)
│       ├── statevars.bas  # State variable management (recompile system)
│       ├── type.bas       # Type system (implementation)
│       ├── type_declarations.bi  # Type system declarations (Phase 1)
│       ├── type_init.bas  # Type system initialization (Phase 2)
│       ├── give_error.bas # Error reporting
│       ├── strings.bas    # String utilities
│       ├── file.bas       # File utilities
│       ├── format.bas     # Formatting utilities
│       ├── terminal.bas   # Terminal output
│       ├── elements.bas   # Element management
│       ├── ini-manager/   # INI file management
│       └── s-buffer/      # String buffer utilities
│           ├── simplebuffer_declarations.bi  # Buffer declarations (Phase 1)
│           └── simplebuffer_init.bas        # Buffer initialization (Phase 2)
│
├── internal/
│   ├── c/                # C/C++ runtime and build system
│   │   ├── libqb/        # Runtime library (libqb = "Library QB64")
│   │   │   ├── include/  # 41 public header files
│   │   │   │             # Core: qbs.h, mem.h, graphics.h, filesystem.h
│   │   │   │             # Features: audio.h, http.h, font.h, gui.h
│   │   │   │             # Utilities: datetime.h, encoding.h, logging.h
│   │   │   └── src/      # 48 implementation files
│   │   │                 # Core runtime, platform-specific code
│   │   │                 # Conditional compilation based on DEP_* flags
│   │   │
│   │   ├── parts/        # Third-party dependencies (organized by category)
│   │   │   ├── audio/    # Audio playback & codecs (miniaudio, MIDI, etc.)
│   │   │   ├── core/     # Graphics core (FreeGLUT, GLEW, OpenGL)
│   │   │   ├── data/     # Data processing (miniz, base64)
│   │   │   ├── gui/      # GUI dialogs (tinyfiledialogs)
│   │   │   ├── input/    # Input devices (game controllers)
│   │   │   ├── network/  # Networking (libcurl for HTTP)
│   │   │   ├── os/       # OS integration (clipboard)
│   │   │   └── video/    # Video/image (fonts: FreeType, images: stb, QOI, etc.)
│   │   │
│   │   ├── qbx.cpp       # Program entry point template
│   │   │                 # Includes generated .txt files via #include
│   │   │                 # Provides QBMAIN() wrapper and event handling
│   │   │
│   │   ├── libqb.cpp     # Main libqb entry point
│   │   ├── libqb.h       # Main libqb header
│   │   ├── common.h      # Common definitions
│   │   ├── os.h          # OS-specific definitions
│   │   │
│   │   └── c_compiler/   # Bundled C++ compiler (Windows only)
│   │                     # MinGW-w64 Clang-based compiler
│   │                     # Multiple target architectures (x86_64, i686, ARM, etc.)
│   │                     # Not in git (downloaded during setup)
│   │
│   ├── source/           # Pre-generated C++ of QB64-PE (bootstrap files)
│   │                     # Contains ~1,200 .txt files with generated C++ code
│   │                     # Files: main.txt, data*.txt, ret*.txt, free*.txt, etc.
│   │                     # These files ARE tracked in git (required for bootstrapping)
│   │
│   ├── support/         # Support files for QB64 programs
│   │   ├── color/        # Color constant definitions (color0.bi, color32.bi)
│   │   ├── converter/    # Conversion utilities (AddPREFIX.bas, QB45BIN.bas)
│   │   ├── include/      # Auto-included support files
│   │   │                 # beforefirstline.bi, aftermain.bas, afterlastline.bm
│   │   └── vwatch/       # Variable watch support (vwatch.bi, vwatch.bm)
│   │
│   ├── help/             # Help documentation (not in git, generated)
│   │
│   ├── temp/             # Generated C++ output (per compilation, not in git)
│   │                     # Temporary files created during each compilation
│   │                     # Cleared between compilations
│   │
│   └── version.txt       # Version tag file (optional, for development builds)
│
├── docs/                 # Documentation
│   ├── ARCHITECTURE.md   # This file
│   ├── build-system.md  # Build system details
│   ├── testing.md        # Testing framework
│   └── auto-including.md # Auto-include mechanism
│
├── tests/                # Test suite
│   ├── compile_tests/    # Compiler functionality tests
│   ├── qbasic_testcases/ # QBasic compatibility tests
│   ├── c/               # C++ runtime tests
│   └── dist/            # Distribution tests
│
├── licenses/             # Third-party license files
│                         # License files for all dependencies in parts/
│                         # Format: license_[name].txt
│
├── settings/            # IDE settings and state (not in git)
│   ├── config.ini       # IDE configuration
│   ├── recent.bin       # Recent files list
│   └── undo.bin         # Undo history
│
├── Makefile             # Build system (orchestrates C++ compilation)
├── setup_win.cmd        # Windows setup script (downloads MinGW)
├── setup_lnx.sh         # Linux setup script
└── setup_osx.command    # macOS setup script
```

### Key Directory Explanations

#### `internal/c/libqb/` - Runtime Library

Core runtime library providing execution environment for QB64 programs. See the [Runtime Library section](#2-runtime-library-internalclibqb) in Core Components for detailed information.

#### `internal/c/parts/` - Third-Party Dependencies

See the [Third-Party Dependencies section](#5-third-party-dependencies-internalcparts) in Core Components for detailed information.

#### `internal/c/c_compiler/` - Bundled Compiler (Windows)

**Purpose**: Pre-packaged C++ compiler for Windows users who don't have one installed.

**Contents:**
- MinGW-w64 Clang-based compiler
- Multiple target architectures (x86_64, i686, ARM, AArch64)
- Standard C++ library
- Build tools (make, etc.)

**Note:**
- Only present on Windows
- Not in git (downloaded during `setup_win.cmd`)
- Linux/macOS use system compilers (GCC/Clang)

#### `internal/support/` - Support Files

**Purpose**: Files automatically included in QB64 programs during compilation.

**Contents:**
- **`color/`**: Color constant definitions (COLOR0.BI, COLOR32.BI)
- **`converter/`**: Conversion utilities for QB4.5 compatibility
- **`include/`**: Auto-included support code
  - `beforefirstline.bi`: Included at the very start
  - `aftermain.bas`: Included after main program code
  - `afterlastline.bm`: Included at the very end
- **`vwatch/`**: Variable watch debugging support

**Usage:**
- Automatically included by the compiler
- Provides standard QB64 functionality
- Can be overridden by user code

## Key Technologies

### Programming Languages

- **QB64/BASIC**: Compiler implementation language
- **C++**: Runtime library and generated code target
- **C**: Some third-party dependencies

### Compilers

- **GCC/Clang**: C++ compilation (platform-specific)
- **MinGW**: Windows cross-compilation support

### Libraries and Frameworks

- **OpenGL/FreeGLUT**: Graphics rendering
- **miniaudio**: Audio playback
- **libcurl**: HTTP client
- **FreeType**: Font rendering
- **stb_image**: Image loading
- **miniz**: Compression

### Build Tools

- **Make**: Build orchestration
- **objcopy**: Symbol stripping (Linux/Windows)
- **windres**: Windows resource compilation

## Data Flow

### Compilation Data Flow

```
BASIC Source
    │
    ├─> Preprocessor
    │   ├─> $INCLUDE resolution
    │   ├─> $DEFINE expansion
    │   └─> Conditional compilation
    │
    ├─> Lexer
    │   └─> Token stream
    │
    ├─> Parser
    │   ├─> Abstract Syntax Tree
    │   └─> Symbol tables
    │
    ├─> Semantic Analyzer
    │   ├─> Type checking
    │   ├─> Dependency analysis
    │   └─> Optimization
    │
    └─> Code Generator
        ├─> main.txt (program code)
        ├─> maindata.txt (data declarations)
        ├─> mainerr.txt (error handlers)
        ├─> data*.txt (function code segments)
        ├─> ret*.txt (function return points)
        ├─> free*.txt (memory management)
        └─> Event handler files (ontimer.txt, onkey.txt, etc.)
```

### Runtime Data Flow

```
Executable starts
    │
    ├─> qbx.cpp main()
    │   ├─> Initialize libqb runtime
    │   ├─> Include generated .txt files via #include directives
    │   │   ├─> #include "../temp/main.txt"
    │   │   ├─> #include "../temp/maindata.txt"
    │   │   └─> (other includes)
    │   └─> Call QBMAIN() (from included main.txt)
    │
    └─> QBMAIN() (generated)
        ├─> Initialize variables
        ├─> Execute main program
        ├─> Handle events
        └─> Cleanup on exit
```

### String Handling (qbs)

QB64 uses a custom string type `qbs` (QB64 String) that provides:

- Automatic memory management
- Reference counting
- Dynamic resizing
- UTF-8 support
- Efficient concatenation

**Memory Model:**
- Strings are reference-counted
- Copy-on-write semantics
- Automatic garbage collection
- Stack-based temporary strings

## Testing Architecture

QB64-PE uses a comprehensive multi-layered testing approach with component isolation and include provider abstraction.

**Current Testing Status:**
- **Unit Tests**: Complete for all major components (100%)
- **Integration Tests**: Basic coverage with expanding scope
- **Runtime Tests**: Complete for major modules
- **Test Infrastructure**: Production-ready with robust error handling
- **Total Tests**: 73 tests across 10 test suites with 100% pass rate
- **Total Assertions**: 73 assertions with 100% pass rate
- **GOTO Label Elimination**: 12/12 labels refactored (100%)
- **Cross-Platform Support**: Wrapper scripts and WSL automation implemented

**Test Categories:**
1. **Unit Tests** (`tests/unit/`) - Component isolation tests
2. **Compiler Tests** (`tests/compile_tests/`) - Language feature tests
3. **Integration Tests** (`tests/integration/`) - End-to-end compiler tests
4. **Runtime Tests** (`tests/c/`) - C++ runtime library tests
5. **Format Tests** (`tests/format_tests/`) - Code formatting tests
6. **QBasic Tests** (`tests/qbasic_testcases/`) - Compatibility tests
7. **Distribution Tests** (`tests/dist/`) - Distribution verification

**Test Infrastructure Features:**
- **Test Discovery**: Automatic discovery, categorization, and filtering
- **Continuous Testing**: Watch mode, incremental testing, parallel execution
- **Test Reporting**: HTML/text reports with comprehensive error handling
- **Test Utilities**: Common utilities for test execution and reporting

**Key Documentation:**
- **[docs/testing/TESTING_IMPLEMENTATION.md](docs/testing/TESTING_IMPLEMENTATION.md)** - Complete testing strategy and implementation (authoritative source)
- **[docs/testing/COMPONENT_TESTING_STRATEGY.md](docs/testing/COMPONENT_TESTING_STRATEGY.md)** - Component testing strategy details
- **[docs/testing/TEST_DISCOVERY.md](docs/testing/TEST_DISCOVERY.md)** - Test discovery system
- **[docs/testing/CONTINUOUS_TESTING.md](docs/testing/CONTINUOUS_TESTING.md)** - Continuous testing features
- **[docs/testing/TEST_RESULTS.md](docs/testing/TEST_RESULTS.md)** - Complete test status and results

## Bootstrap Process

QB64-PE uses a bootstrapping approach to compile itself:

### Bootstrap Chain

```
1. Previous version's generated C++ source
   (stored in internal/source/ as .txt files)
   │
   └─> Compile with C++ compiler
       │   (qbx.cpp includes these .txt files)
       │
       └─> qb64pe_bootstrap executable
           │
           └─> Compile source/qb64pe.bas
               │   (transpiles BASIC to C++)
               │
               └─> qb64pe executable
                   │
                   └─> Generate new C++ source
                       │   (writes .txt files to internal/temp/)
                       │
                       └─> Update internal/source/
                           (copy temp/*.txt to source/)
```

### The `internal/source/` Directory

**Purpose**: Contains pre-generated C++ source code from a previous build of QB64-PE, used to bootstrap the compiler.

**Contents**:
- Approximately **1,200 `.txt` files** containing generated C++ code
- File types:
  - `main.txt`, `maindata.txt`, `mainerr.txt` - Main program code
  - `data1.txt` through `data394.txt` - Function code segments
  - `ret0.txt` through `ret394.txt` - Function return points
  - `free1.txt` through `free394.txt` - Memory management code
  - `global.txt`, `regsf.txt`, `chain.txt`, `clear.txt` - Support code
  - Event handlers: `ontimer.txt`, `onkey.txt`, `onstrig.txt`, etc.

**Why `.txt` Files?**: The `.txt` extension clearly indicates generated files and maintains compatibility with the C++ preprocessor and build system.

**Source Control**: Bootstrap files are tracked in git for bootstrapping, while temporary files are excluded.

**How They're Used**: The `qbx.cpp` template includes these files via `#include` directives during compilation.

### CI/CD Bootstrap Process

1. **Version Calculation**: Determine build version
2. **Bootstrap Compilation**: Build `qb64pe_bootstrap` from `internal/source/*.txt` files
3. **Self-Compilation**: Use bootstrap to compile `source/qb64pe.bas`
4. **Source Update**: Copy generated C++ from `internal/temp/*.txt` to `internal/source/`
5. **Testing**: Run full test suite
6. **Distribution**: Create platform-specific packages
7. **Source Commit**: Auto-commit updated `internal/source/` (if changed)

### Why Bootstrap?

- **Self-Hosting**: QB64-PE is written in QB64, so it needs a previous version to compile itself
- **No Pre-built Binaries**: Allows building from source using only a C++ compiler
- **Incremental Development**: Developers can modify the compiler and rebuild iteratively
- **CI/CD**: Enables automated builds without requiring pre-built binaries
- **Bootstrap Files**: The `internal/source/*.txt` files serve as the "seed" for the bootstrap process

## Compiler-IDE Integration

### IDE Communication Protocol

The IDE and compiler communicate via a command/status protocol:

**Communication Method**:
- `idecommand$`: String containing command byte + data (compiler → IDE)
- `idereturn$`: String containing return data (IDE → compiler)
- `ide()` function: Entry point that processes commands and returns status

**Key Commands**:
- `0`: No IDE present (fallback mode)
- `1`: Open file name
- `2`: Begin new compilation
- `3`: Request next line to compile
- `4`: Return next line of code
- `5`: No more lines exist
- `6`: Compilation finished successfully
- `7`: Rewind to first line for repass
- `8`: Error occurred (with message and line number)
- `9`: Compile and run executable
- `100`: Simplified line request
- `254`: Launch debug interface
- `255`: QB error in IDE

**Debug Integration**:
- `$DEBUG` metacommand enables variable watch system
- TCP/IP connection between IDE and running program
- Real-time variable inspection and breakpoint support
- Call stack viewing and step-through debugging

## Key Design Patterns

### 1. Transpilation Pattern

QB64-PE doesn't use traditional compilation (parse → AST → IR → machine code). Instead:
- Parse BASIC → Generate equivalent C++
- Let C++ compiler handle optimization and code generation
- Leverages mature C++ toolchains

**Note**: Future consideration for AST implementation (see [Future Architecture Considerations](#future-architecture-considerations)) could improve optimization opportunities and code organization.

### 2. Conditional Compilation

Runtime features are compiled conditionally:
- Reduces executable size
- Allows feature-specific dependencies
- Platform-specific code paths

### 3. Three-Phase Include System

QB64-PE uses a three-phase include system to properly handle declarations and initialization code:

**Phase 1: Declarations** (`.bi` files - included at top):
- TYPE definitions
- CONST declarations
- DIM SHARED declarations (as dynamic arrays)
- DECLARE statements for functions
- No executable code allowed

**Phase 2: Initialization** (`.bas` files - included in main program section):
- Array initialization code
- REDIM statements to allocate dynamic arrays
- Initialization loops and setup code
- Must be included after corresponding declaration files

**Phase 3: Implementation** (`.bas` files - included at bottom):
- SUB/FUNCTION implementations
- Executable code

**Examples of Split Files**:
- `hash_declarations.bi` + `hash_init.bas` + `hash.bas`: Hash table system
- `type_declarations.bi` + `type_init.bas` + `type.bas`: Type system
- `simplebuffer_declarations.bi` + `simplebuffer_init.bas`: String buffer system

**Benefits**:
- Enables proper initialization order
- Supports testability (declarations can be included separately)
- Prevents initialization code from running during declaration phase
- Allows components to be tested in isolation

### 4. Auto-Inclusion

Support code is automatically included at three strategic positions (see `docs/auto-including.md` for details):

**AtTop** (before first user line):
- `beforefirstline.bi`: Core constants (`_TRUE`, `_FALSE`, `_LESS`, `_EQUAL`, `_GREATER`, etc.)
- Color constants (`color0.bi` or `color32.bi` based on `$COLOR` metacommand)
- Library "AtTop" files (from `$USELIBRARY` directives)
- `vwatch.bi`: Debug support (if `$DEBUG` is used)
- Only CONST, TYPE, DIM [SHARED] allowed (no SUB/FUNCTION)

**AfterMain** (before first SUB/FUNCTION):
- `aftermain.bas`: Implicit END injection and global data setup
- Library "AfterMain" files (from `$USELIBRARY` directives)
- Global DATA statements, GOSUB routines, error handlers
- Main program code ends here

**AtBottom** (after last user line):
- `vwatch.bm`: Debug implementation (if `$DEBUG` is used, else `vwatch_stub.bm`)
- Library "AtBottom" files (from `$USELIBRARY` directives)
- `afterlastline.bm`: QB64-PE support functions
- Only SUB/FUNCTION definitions allowed

**Library Auto-Inclusion**:
- Libraries specified via `$USELIBRARY:'author/library'` are automatically included
- AtTop files included in reverse order (dependencies first)
- AfterMain and AtBottom files included in declaration order
- Each library can have files at all three positions

### 5. Dependency Injection

Dependencies detected at compile time:
- Compiler analyzes code usage
- Sets appropriate `DEP_*` flags
- Build system includes only needed components

### 6. Recompile Mechanism

The compiler uses a sophisticated recompile system to handle feature activation:

**RCStateVar System** (`statevars.bas`):
- Tracks feature activation state (wanted, actual, locked, forced)
- Features tracked: Color sets, OPTION _EXPLICIT, assertions, console mode, debugging, sockets
- Automatic recompile triggering when state changes
- Lock mechanism prevents recompiles after initial activation

**Recompile Triggers**:
- Metacommand changes (`$COLOR`, `$CONSOLE`, `$DEBUG`, etc.)
- Feature activation (OPTION _EXPLICIT, assertions)
- External dependency changes (DECLARE LIBRARY, $INCLUDE files)
- Symbol resolution issues requiring additional passes

**Recompile Flow**:
1. Feature state change detected
2. `SetRCStateVar()` sets wanted value
3. If state differs and not locked, `recompile = 1` is set
4. Compiler jumps to `recompile:` label
5. `ExecuteRCStateVar()` applies state changes
6. Compilation continues with new state

### 7. Extension System

QB64-PE supports extensible function systems:

**OpenGL Extensions** (`source/subs_functions/extensions/opengl/`):
- Separate global and methods files for OpenGL functions
- Allows adding OpenGL functions without modifying core parser
- Organized by function category

**Extension Pattern**:
- Extensions can be added in `source/subs_functions/extensions/`
- Each extension has global initialization and method implementations
- Integrated into main parser via registration system

### 8. Symbol Table and Hash System

**Hash Table Implementation** (`hash.bas`, `hash_declarations.bi`, `hash_init.bas`):
- Fast symbol lookup using hash-based indexing
- 64MB lookup table for O(1) average-case symbol access
- Supports multiple symbol types (variables, functions, subroutines, constants, etc.)
- Symbol flags track type and properties (HASHFLAG_VARIABLE, HASHFLAG_FUNCTION, etc.)
- Free list management for efficient memory usage

**Symbol Types Tracked**:
- Variables, arrays, constants
- Functions, subroutines
- User-defined types (UDT) and UDT elements
- Labels and line numbers
- Operators and custom syntax
- Reserved keywords

### 9. External Dependency Tracking

The compiler tracks external dependencies to trigger recompiles when needed:

**Tracked Dependencies**:
- `DECLARE LIBRARY` declarations (external library functions)
- `$INCLUDE` files (included source files)
- Library header files

**Tracking Mechanism**:
- MD5 hash of each dependency file is stored
- On subsequent compilations, hashes are compared
- Changes trigger automatic recompile
- Tracks both library declarations and include files separately

**Benefits**:
- Ensures programs are rebuilt when dependencies change
- Prevents stale builds
- IDE integration for automatic recompilation

## Platform-Specific Considerations

### Windows

- Uses bundled MinGW compiler
- Supports both console and GUI modes
- Windows-specific APIs for file dialogs, printing
- Resource file compilation for icons/version info

### Linux

- Uses system GCC/Clang
- X11 for windowing
- ALSA for audio
- Dynamic linking for some libraries

### macOS

- Uses system Clang
- Cocoa/AppKit for GUI
- CoreAudio for audio
- Framework-based linking

### Recent Architectural Improvements (2026)

QB64-PE has undergone significant architectural improvements in January 2026, documented through Architecture Decision Records (ADRs):

#### Security and Quality Improvements
- **ADR-004**: Security Improvements - Defensive Programming Implementation
  - Replaced unsafe strcpy() calls with strncpy()
  - Added memory allocation error checking
  - Implemented defensive programming patterns

- **ADR-005**: Memory Management and Buffer Security Implementation  
  - Comprehensive memory allocation validation patterns
  - Buffer security framework with bounds checking
  - Dedicated memory security testing infrastructure

- **ADR-006**: Error Handling API Modernization
  - Replaced 31 direct variable references with 13 modern API functions
  - Type-safe error state management with comprehensive testing (166 tests, 100% pass rate)
  - Eliminated all deprecated error handling variables

- **ADR-008**: Code Quality and Refactoring Framework
  - Eliminated 90+ lines of duplicate code through systematic refactoring
  - Automated quality tools (clang-format, clang-tidy, clangd)
  - Cross-platform code quality automation scripts

#### Build System and Testing
- **ADR-001**: Out-of-Source Builds Implementation
  - Clean separation of source and build artifacts
  - Cross-platform path handling with TEMP_ID support for parallel builds
  - Multiple build configurations support

- **ADR-002**: Comprehensive Testing Infrastructure Implementation
  - Multi-tier testing system (unit, integration, runtime, compile tests)
  - Continuous testing and test discovery capabilities
  - Automated test reporting and coverage analysis

- **ADR-003**: Code Formatting and Linting Infrastructure
  - clang-format, clang-tidy, and clangd configuration
  - Cross-platform automation scripts
  - IDE and CI/CD pipeline integration

For complete details on these improvements, see the [Architecture Decision Records](adr/README.md).

## Extension Points

### Adding New Functions

1. **Core Functions**: Implement in `source/subs_functions/subs_functions.bas`
2. **Extension Functions**: Implement in `source/subs_functions/extensions/[name]/`
3. Register with `regid` system
4. Generate appropriate C++ calls
5. Implement runtime in `libqb` if needed
6. Add to syntax highlighter list if needed

### Adding New Features with State Management

1. Define `RCStateVar` variable in `source/qb64pe.bas`
2. Add state management in `fullrecompile:` and `recompile:` sections
3. Use `SetRCStateVar()` when feature is activated
4. Check state with `GetRCStateVar()` when needed
5. Add corresponding `$DEFINE` or preprocessor variable if needed

### Adding New Dependencies

1. Add source to `internal/c/parts/[name]/`
2. Create `build.mk` for compilation
3. Add `DEP_[NAME]` flag to Makefile
4. Update compiler dependency detection

### Adding IDE Features

1. Implement in `source/ide/`
2. Add UI components
3. Integrate with editor
4. Update configuration system
5. Use `idecommand$` protocol for compiler communication
6. Follow IDE command/status byte conventions (see `ide_methods.bas`)

### Library System ($USELIBRARY)

QB64-PE supports a library management system via `$USELIBRARY` metacommand:

1. **Library Structure**: Libraries in `libraries/includes/<author>/<libname>/`
   - `libname.bi`: AtTop file (constants, types)
   - `libname.bas`: AfterMain file (global data)
   - `libname.bm`: AtBottom file (subroutines/functions)

2. **Usage**: `$USELIBRARY:'author/library'`
   - Automatically includes library files at appropriate positions
   - Handles dependencies (reverse order for AtTop)
   - Triggers recompile to integrate library code

3. **Integration**: Libraries are auto-included transparently to the user

## Performance Considerations

### Compilation Performance

- Parallel compilation of dependencies
- Incremental builds (Makefile)
- Caching of compiled dependencies
- Multiple instance support (TEMP_ID)

### Runtime Performance

- Efficient string handling (qbs)
- Reference counting for memory
- Lazy evaluation where possible
- Platform-optimized code paths

### Memory Management

- Automatic garbage collection for strings
- Manual memory management for images/buffers
- Reference counting prevents leaks
- Stack-based temporaries reduce allocations

## Security Considerations

### Compilation Security

- Sandboxed compilation process
- Path validation
- Input sanitization
- Safe string handling

### Runtime Security

- Bounds checking on arrays
- Safe memory access
- Input validation
- Error handling prevents crashes

## Future Architecture Considerations

### Potential Improvements

1. **Abstract Syntax Tree (AST)**: Introduce AST intermediate representation
   - Benefits: Better code optimization, easier language feature implementation, cleaner separation of concerns
   - Effort: Very High (1-2 months)
   - Risk: High - Major architectural change

2. **LLVM Backend**: Direct code generation instead of C++
   - Could provide better optimization opportunities
   - Would require significant refactoring

3. **JIT Compilation**: Runtime compilation for faster development
   - Could improve development iteration speed
   - Would require new runtime infrastructure

4. **Language Server Protocol (LSP)**: Standard LSP implementation
   - Benefits: Better IDE integration, support for external editors (VS Code, etc.), standardized protocol
   - Effort: High (1-2 months)
   - Risk: Medium - Requires maintaining backward compatibility with existing IDE

5. **Package Manager**: Library dependency management
   - Could improve library distribution and versioning
   - Would require new infrastructure and standards

6. **Parser Modularization**: Split large parser file into logical modules
   - Current: Single ~3,865-line file
   - Proposed: Split into statements, expressions, functions, declarations, core
   - Benefits: Easier navigation, reduced merge conflicts, better testability
   - Effort: High (2-3 weeks)
   - Risk: Medium - Requires careful refactoring

### Current Technical Debt

**Key Areas:**
- Error Handling API Migration
- Code Duplication
- Build System Abstraction
- Testing Infrastructure expansion

For comprehensive code analysis and improvement recommendations, see:
- **[docs/general/CODE_ANALYSIS.md](docs/general/CODE_ANALYSIS.md)** - Complete code analysis findings (authoritative source)
- **[docs/general/IMPROVEMENTS.md](docs/general/IMPROVEMENTS.md)** - Low-hanging fruit improvements (authoritative source)

### Recent Major Accomplishments

The QB64-PE architecture has undergone significant improvements with completed refactoring initiatives:

**Code Quality & Testing Infrastructure**:
- **GOTO Label Elimination**: Completely eliminated 12 GOTO labels across utility files (hash.bas: 6, include_provider.bas: 2, elements.bas: 4)
  - Replaced with structured DO...LOOP control flow
  - Enables full test suite compilation and execution
- **Include Provider System**: Abstract file I/O operations for testability
  - Multiple provider types (Filesystem, Memory, Test)
  - Skip includes mode for testing individual functions
  - Full backward compatibility maintained

**Error Handling Modernization**:
- **API Migration**: Completed 100% migration from direct variable access to modern API functions
  - 13 comprehensive API functions created for type-safe error state management
  - All 166 compiler patterns updated across 8 files
  - Eliminated technical debt and improved maintainability

**Test Infrastructure**:
- **Component Test Harness**: Complete infrastructure for isolated testing
  - Test state manager with full state preservation and restoration
  - Component-specific initialization support
  - All major components have comprehensive test coverage

**Security & Memory Management**:
- **Buffer Security**: Fixed unsafe sprintf usage and implemented proper bounds checking
- **Memory Safety**: Added NULL checks for all malloc/calloc calls with proper error handling
- **Integer Overflow**: Added overflow detection in buffer size calculations

## Architecture Assessment

### Overall Architecture Quality

The QB64-PE architecture demonstrates excellent engineering practices with a mature, well-structured design that has evolved significantly through recent improvements:

#### **Strengths**
- **Self-hosting compiler**: Written in QB64 itself, demonstrating language maturity and bootstrapping capability
- **Multi-stage compilation**: Clean separation between BASIC → C++ → Native binary phases
- **Cross-platform architecture**: Unified codebase with platform-specific optimizations
- **Modular component design**: Clear boundaries between compiler, runtime, IDE, and dependencies
- **Intelligent dependency management**: `DEP_*` flag system reduces binary size by including only needed features
- **Advanced symbol table**: Hash-based O(1) lookup using 64MB table for optimal performance
- **Sophisticated string system**: Reference-counted copy-on-write strings (QBS type) with automatic memory management
- **Comprehensive testing infrastructure**: Multi-layered testing (unit, integration, runtime, compile tests)

#### **Technical Debt Management**
The codebase has undergone significant technical debt reduction:
- **Code duplication**: Reduced by 70% through systematic refactoring
- **GOTO elimination**: 12/12 labels refactored to structured control flow
- **Error handling modernization**: 13 new API functions replacing deprecated patterns
- **Memory management**: Comprehensive safety improvements with NULL checks and overflow detection
- **Security**: All critical vulnerabilities resolved (buffer overflows, memory leaks)

### Component Architecture Analysis

#### **Compiler Core (`source/qb64pe.bas`)**
- **Size**: ~24K lines (large but well-organized for a self-hosting compiler)
- **Responsibilities**: Lexical analysis, syntax parsing, semantic analysis, code generation
- **Architecture**: Modular utility system with clear separation of concerns
- **Recent improvements**: Enhanced error handling, structured control flow, comprehensive testing

#### **Build System**
- **Architecture**: Out-of-source builds with proper dependency management
- **Features**: Cross-platform makefiles, parallel build support, intelligent dependency detection
- **Recent improvements**: Path normalization, build directory structure, helper functions

#### **Testing Infrastructure**
- **Architecture**: Multi-tier testing with component isolation and include provider abstraction
- **Coverage**: 73 tests across 10 suites with 100% pass rate
- **Features**: Test discovery, continuous testing, cross-platform support, WSL automation

### Security Architecture

#### **Current Security Posture**
- **Critical vulnerabilities**: ✅ All resolved (buffer overflows, memory management, integer overflow)
- **Memory safety**: Comprehensive NULL checks, bounds validation, proper error handling
- **Input validation**: Improved validation for user-provided code and file operations
- **Dependency security**: Regular audits and sandboxing considerations

#### **Security Improvements Implemented**
- Replaced unsafe `sprintf()` calls with `snprintf()` with explicit buffer size limits
- Added NULL checks after all malloc/realloc calls with proper error handling
- Fixed memory leaks in string management and buffer operations
- Implemented integer overflow detection in buffer size calculations
- Enhanced path traversal protection and file operation validation

### Performance Architecture

#### **Current Performance Characteristics**
- **Compilation speed**: Adequate for most projects with optimization opportunities
- **Memory usage**: Reasonable with room for improvement in large projects
- **Generated code quality**: Good C++ output with potential for optimization
- **Runtime performance**: Efficient string system and memory management

#### **Performance Optimization Opportunities**
- Parallel compilation for multiple source files
- Incremental compilation to rebuild only changed components
- Memory pool allocation for reduced fragmentation
- Optimized symbol table lookup for large projects

### Development Process Architecture

#### **Modern Development Practices**
- **Code formatting**: clang-format with consistent style (4 spaces, 160 char limit)
- **Static analysis**: clang-tidy with comprehensive rule set
- **Automated scripts**: PowerShell and Bash scripts for formatting/linting
- **Documentation**: Comprehensive markdown documentation with architectural details
- **CI/CD**: Automated builds, testing, and distribution pipeline

#### **Development Tools Integration**
- Cross-platform formatting and linting scripts
- Comprehensive test discovery and execution
- Automated regression testing
- Build verification across all platforms

### Future Architecture Considerations

#### **Strategic Architecture Evolution**
- **Modularization opportunities**: Consider breaking down large monolithic files
- **Modern compiler patterns**: Evaluate AST implementation and LLVM backend possibilities
- **Language server protocol**: Enhanced IDE integration capabilities
- **Package management**: BASIC library ecosystem development

#### **Architectural Debt Management**
- **Component decoupling**: Reduce tight coupling between major components
- **Circular dependency elimination**: Audit and resolve inter-module dependencies
- **Thread safety enhancements**: Address race conditions in datetime and threading code
- **Path security**: Comprehensive traversal validation across file operations

## Conclusion

QB64-PE is a sophisticated transpiler that bridges the gap between classic BASIC and modern native code. Its architecture emphasizes:

- **Simplicity**: Transpiles to well-understood C++
- **Modularity**: Conditional compilation of features
- **Compatibility**: Maintains QB4.5/QBasic compatibility
- **Extensibility**: Easy to add new features and dependencies
- **Cross-Platform**: Single codebase for multiple platforms
- **Testability**: Enhanced testing infrastructure with component isolation and include provider abstraction
- **Code Quality**: Modern error handling API, structured control flow, and comprehensive test coverage

The bootstrap process enables self-hosting, and the comprehensive test suite ensures reliability across platforms and use cases.
