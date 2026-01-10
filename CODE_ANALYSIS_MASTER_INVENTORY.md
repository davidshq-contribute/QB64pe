# QB64-PE Code Analysis - Master File Inventory

This document catalogs all project-owned files to be analyzed, excluding third-party libraries and generated files.

## File Categories

### 1. QB64 Compiler Source Files (source/)

#### Main Compiler
- `source/qb64pe.bas` - Main compiler entry point

#### Global Files
- `source/global/constants.bas` - Global constants
- `source/global/settings.bas` - Settings management
- `source/global/version.bas` - Version information

#### Utilities
- `source/utilities/build.bas` - Build system integration
- `source/utilities/const_eval.bas` - Constant evaluation
- `source/utilities/const_eval.bi` - Constant evaluation interface
- `source/utilities/elements.bas` - Element handling
- `source/utilities/file.bas` - File operations
- `source/utilities/format.bas` - Code formatting
- `source/utilities/give_error.bas` - Error reporting
- `source/utilities/give_error.bi` - Error reporting interface
- `source/utilities/hash.bas` - Hash table implementation
- `source/utilities/hash.bi` - Hash table interface
- `source/utilities/statevars.bas` - State variable management
- `source/utilities/statevars.bi` - State variable interface
- `source/utilities/strings.bas` - String utilities
- `source/utilities/terminal.bas` - Terminal operations
- `source/utilities/type.bas` - Type system
- `source/utilities/type.bi` - Type system interface

#### Utility Modules
- `source/utilities/ini-manager/ini.bi` - INI file interface
- `source/utilities/ini-manager/ini.bm` - INI file implementation
- `source/utilities/s-buffer/simplebuffer.bi` - Simple buffer interface
- `source/utilities/s-buffer/simplebuffer.bm` - Simple buffer implementation
- `source/utilities/s-buffer/sb_qb64pe_extension.bi` - Buffer extension interface
- `source/utilities/s-buffer/sb_qb64pe_extension.bm` - Buffer extension implementation

#### Parser and Functions
- `source/subs_functions/subs_functions.bas` - Main parser and function handlers
- `source/subs_functions/syntax_highlighter_list.bas` - Syntax highlighting

#### Extensions
- `source/subs_functions/extensions/opengl/opengl_global.bas` - OpenGL global definitions
- `source/subs_functions/extensions/opengl/opengl_methods.bas` - OpenGL method handlers

#### Code Emission
- `source/emit/logging.bas` - Logging emission

#### IDE Components
- `source/ide/ide_global.bas` - IDE global definitions
- `source/ide/ide_methods.bas` - IDE method handlers
- `source/ide/ide_converters.bas` - IDE converters
- `source/ide/ide_export.bas` - IDE export functionality
- `source/ide/config/cfg_global.bas` - Config global definitions
- `source/ide/config/cfg_methods.bas` - Config method handlers
- `source/ide/wiki/wiki_global.bas` - Wiki global definitions
- `source/ide/wiki/wiki_methods.bas` - Wiki method handlers

**Total QB64 Source Files: 28 .bas + 8 .bi + 3 .bm = 39 files**

### 2. Runtime Library Files (internal/c/libqb/)

#### Source Files (src/)
- `internal/c/libqb/src/bitops.cpp` - Bit operations
- `internal/c/libqb/src/buffer.cpp` - Buffer management
- `internal/c/libqb/src/command.cpp` - Command-line parsing
- `internal/c/libqb/src/console-only-main-thread.cpp` - Console-only main thread
- `internal/c/libqb/src/datetime.cpp` - Date/time functions
- `internal/c/libqb/src/environ.cpp` - Environment variables
- `internal/c/libqb/src/error_handle.cpp` - Error handling
- `internal/c/libqb/src/file-fields.cpp` - File field operations
- `internal/c/libqb/src/filepath.cpp` - File path operations
- `internal/c/libqb/src/filesystem.cpp` - File system operations
- `internal/c/libqb/src/gfs.cpp` - Graphics file system
- `internal/c/libqb/src/graphics.cpp` - Graphics operations
- `internal/c/libqb/src/hexoctbin.cpp` - Hex/oct/bin conversion
- `internal/c/libqb/src/http.cpp` - HTTP client
- `internal/c/libqb/src/http-stub.cpp` - HTTP stub
- `internal/c/libqb/src/mem.cpp` - Memory management
- `internal/c/libqb/src/qblist.cpp` - List data structure
- `internal/c/libqb/src/qbs.cpp` - QB64 string core
- `internal/c/libqb/src/qbs__tostr.cpp` - String conversion
- `internal/c/libqb/src/qbs_cmem.cpp` - Common memory strings
- `internal/c/libqb/src/qbs_mk_cv.cpp` - String make/convert
- `internal/c/libqb/src/qbs_str.cpp` - String operations
- `internal/c/libqb/src/qbs_val.cpp` - String value operations
- `internal/c/libqb/src/shell.cpp` - Shell operations
- `internal/c/libqb/src/string_functions.cpp` - String utilities
- `internal/c/libqb/src/threading.cpp` - Threading core
- `internal/c/libqb/src/threading-posix.cpp` - POSIX threading
- `internal/c/libqb/src/threading-windows.cpp` - Windows threading
- `internal/c/libqb/src/glut-main-thread.cpp` - GLUT main thread
- `internal/c/libqb/src/glut-message.cpp` - GLUT messaging
- `internal/c/libqb/src/glut-msg-queue.cpp` - GLUT message queue
- `internal/c/libqb/src/mac-mouse-support.cpp` - macOS mouse support

#### Logging System
- `internal/c/libqb/src/logging/logging.cpp` - Logging core
- `internal/c/libqb/src/logging/stacktrace.cpp` - Stack trace generation
- `internal/c/libqb/src/logging/qb64pe_symbol.cpp` - Symbol resolution
- `internal/c/libqb/src/logging/handlers/fp_handler.cpp` - File pointer handler
- `internal/c/libqb/src/logging/mingw/file.cpp` - MinGW file operations
- `internal/c/libqb/src/logging/mingw/pe.cpp` - PE parsing
- `internal/c/libqb/src/logging/mingw/pe_symtab.cpp` - PE symbol table
- `internal/c/libqb/src/logging/mingw/symbol.cpp` - MinGW symbol resolution
- `internal/c/libqb/src/logging/unix/symbol.cpp` - Unix symbol resolution

#### Header Files (include/)
- `internal/c/libqb/include/audio.h` - Audio interface
- `internal/c/libqb/include/bitops.h` - Bit operations interface
- `internal/c/libqb/include/buffer.h` - Buffer interface
- `internal/c/libqb/include/clipboard.h` - Clipboard interface
- `internal/c/libqb/include/cmem.h` - Common memory interface
- `internal/c/libqb/include/command.h` - Command interface
- `internal/c/libqb/include/compression.h` - Compression interface
- `internal/c/libqb/include/completion.h` - Completion interface
- `internal/c/libqb/include/condvar.h` - Condition variable interface
- `internal/c/libqb/include/datetime.h` - Date/time interface
- `internal/c/libqb/include/encoding.h` - Encoding interface
- `internal/c/libqb/include/environ.h` - Environment interface
- `internal/c/libqb/include/error_handle.h` - Error handling interface
- `internal/c/libqb/include/event.h` - Event interface
- `internal/c/libqb/include/extended_math.h` - Extended math interface
- `internal/c/libqb/include/file-fields.h` - File fields interface
- `internal/c/libqb/include/filepath.h` - File path interface
- `internal/c/libqb/include/filesystem.h` - File system interface
- `internal/c/libqb/include/font.h` - Font interface
- `internal/c/libqb/include/game_controller.h` - Game controller interface
- `internal/c/libqb/include/gfs.h` - Graphics file system interface
- `internal/c/libqb/include/graphics.h` - Graphics interface
- `internal/c/libqb/include/gui.h` - GUI interface
- `internal/c/libqb/include/hashing.h` - Hashing interface
- `internal/c/libqb/include/hexoctbin.h` - Hex/oct/bin interface
- `internal/c/libqb/include/http.h` - HTTP interface
- `internal/c/libqb/include/image.h` - Image interface
- `internal/c/libqb/include/keyhandler.h` - Key handler interface
- `internal/c/libqb/include/libqb-common.h` - Common definitions
- `internal/c/libqb/include/logging.h` - Logging interface
- `internal/c/libqb/include/mac-mouse-support.h` - macOS mouse support interface
- `internal/c/libqb/include/mem.h` - Memory interface
- `internal/c/libqb/include/mutex.h` - Mutex interface
- `internal/c/libqb/include/qblist.h` - List interface
- `internal/c/libqb/include/qbmath.h` - Math interface
- `internal/c/libqb/include/qbs-mk-cv.h` - String make/convert interface
- `internal/c/libqb/include/qbs.h` - String interface
- `internal/c/libqb/include/rounding.h` - Rounding interface
- `internal/c/libqb/include/shell.h` - Shell interface
- `internal/c/libqb/include/thread.h` - Thread interface

#### Private Headers
- `internal/c/libqb/src/logging/logging_private.h` - Logging private definitions
- `internal/c/libqb/src/glut-message.h` - GLUT message definitions
- `internal/c/libqb/src/mac-key-monitor.h` - macOS key monitor
- `internal/c/libqb/src/logging/mingw/file.hpp` - MinGW file C++ interface
- `internal/c/libqb/src/logging/mingw/pe.hpp` - PE C++ interface
- `internal/c/libqb/src/logging/mingw/pe_symtab.hpp` - PE symbol table C++ interface

**Total Runtime Library Files: 41 .cpp + 44 .h + 3 .hpp = 88 files**

### 3. Core C++ Files (internal/c/)

- `internal/c/qbx.cpp` - Main program entry point (includes generated code)
- `internal/c/libqb.cpp` - Runtime library entry point
- `internal/c/libqb.h` - Runtime library header
- `internal/c/common.h` - Common definitions
- `internal/c/os.h` - OS abstraction

**Total Core C++ Files: 5 files**

### 4. Integration Files (internal/c/parts/)

#### Audio Integration
- `internal/c/parts/audio/audio.cpp` - Audio system integration
- `internal/c/parts/audio/framework.h` - Audio framework
- `internal/c/parts/audio/stub_audio.cpp` - Audio stub

#### Video/Font Integration
- `internal/c/parts/video/font/font.cpp` - Font system integration
- `internal/c/parts/video/font/hashing.cpp` - Font hashing
- `internal/c/parts/video/font/stub_font.cpp` - Font stub

#### Video/Image Integration
- `internal/c/parts/video/image/image.cpp` - Image system integration

#### GUI Integration
- `internal/c/parts/gui/gui.cpp` - GUI system integration

#### Data Integration
- `internal/c/parts/data/compression.cpp` - Compression wrapper
- `internal/c/parts/data/encoding.cpp` - Encoding wrapper
- `internal/c/parts/data/modp_b64.cpp` - Base64 wrapper (project-owned wrapper, not third-party)

#### OS Integration
- `internal/c/parts/os/clipboard/clipboard.cpp` - Clipboard integration

#### Input Integration
- `internal/c/parts/input/game_controller/game_controller.cpp` - Game controller integration

**Total Integration Files: 11 files**

### 5. Build System Files

- `Makefile` - Main build system
- `setup_lnx.sh` - Linux setup script
- `setup_win.cmd` - Windows setup script
- `setup_mingw.cmd` - MinGW setup script
- `setup_osx.command` - macOS setup script
- `internal/c/purge_all_precompiled_content.bat` - Windows cleanup
- `internal/c/purge_all_precompiled_content.sh` - Unix cleanup
- `internal/c/purge_all_precompiled_content.command` - macOS cleanup
- `internal/c/libqb/build.mk` - Runtime library build config
- `tests/build.mk` - Test build config

**Total Build System Files: 10 files**

### 6. Documentation Files (docs/)

- `docs/ARCHITECTURE.md` - Architecture documentation
- `docs/GETTING_STARTED.md` - Getting started guide
- `docs/build-system.md` - Build system documentation
- `docs/testing.md` - Testing documentation
- `docs/auto-including.md` - Auto-include documentation
- `docs/DEAD_CODE.md` - Dead code analysis
- `docs/IMPROVEMENTS.md` - Improvement suggestions
- `README.md` - Main README

**Total Documentation Files: 8 files**

### 7. Support Files (internal/support/)

#### Auto-Include Files
- `internal/support/include/beforefirstline.bi` - Pre-include definitions
- `internal/support/include/aftermain.bas` - Post-main code
- `internal/support/include/afterlastline.bm` - Post-include code

#### Color Definitions
- `internal/support/color/color0.bi` - Color 0 definitions
- `internal/support/color/color32.bi` - Color 32 definitions

#### Debug Watch System
- `internal/support/vwatch/vwatch.bi` - Watch interface
- `internal/support/vwatch/vwatch.bm` - Watch implementation
- `internal/support/vwatch/vwatch_stub.bm` - Watch stub

#### Converter Tools
- `internal/support/converter/AddPREFIX.bas` - Prefix adder
- `internal/support/converter/QB45BIN.bas` - QB45 binary converter

**Total Support Files: 9 files**

### 8. Test Infrastructure (tests/)

#### Test Scripts
- `tests/compile_tests.sh` - Compile test runner
- `tests/format_tests.sh` - Format test runner
- `tests/qbasic_tests.sh` - QBASIC test runner
- `tests/dist_tests.sh` - Distribution test runner
- `tests/run_tests.sh` - Main test runner
- `tests/run_c_tests.sh` - C test runner
- `tests/run_dist_tests.sh` - Distribution test runner
- `tests/assert.sh` - Test assertion library
- `tests/colors.sh` - Color output utilities
- `tests/add_prefix_test.sh` - Prefix test

#### C Test Framework
- `tests/c/test.h` - Test framework header
- `tests/c/test.cpp` - Test framework implementation
- `tests/c/buffer.cpp` - Buffer tests
- `tests/c/http.cpp` - HTTP tests

**Total Test Infrastructure Files: 13 files**

## Summary

| Category | File Count |
|----------|------------|
| QB64 Compiler Source | 39 |
| Runtime Library | 88 |
| Core C++ Files | 5 |
| Integration Files | 11 |
| Build System | 10 |
| Documentation | 8 |
| Support Files | 9 |
| Test Infrastructure | 13 |
| **TOTAL** | **183 files** |

## Files Excluded (Third-Party/Generated)

- All files in `internal/c/parts/*/` subdirectories that are third-party libraries:
  - `internal/c/parts/audio/extras/*` (except integration files)
  - `internal/c/parts/audio/miniaudio/*`
  - `internal/c/parts/core/freeglut/*`
  - `internal/c/parts/core/glew/*`
  - `internal/c/parts/data/miniz.*`, `modp_b64.*` (third-party sources)
  - `internal/c/parts/gui/tinyfiledialogs.*`
  - `internal/c/parts/network/http/curl/*`
  - `internal/c/parts/video/font/freetype/*`
  - `internal/c/parts/video/image/*/` (except image.cpp)
  - `internal/c/parts/input/game_controller/libstem_gamepad/*`
- Generated files: `internal/source/*.txt`
- Build outputs: `out/`, `*.o`, `*.exe`, `*.sym`
- Embedded compiler: `internal/c/c_compiler/`
- License files: `licenses/*.txt`
- Individual test cases (only infrastructure analyzed)
