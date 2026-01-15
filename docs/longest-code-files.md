# Longest Code Files (Excluding Third-Party Libraries)

This document lists the longest code files in the QB64pe repository, ranked by line count. Only source code files (`.bas`, `.c`, `.cpp`, `.h`, `.hpp`) are included, and third-party libraries have been excluded.

## Top QB64 Code Files by Line Count

| Rank | File | Lines | Extension |
|------|------|-------|-----------|
| 1 | `source/qb64pe.bas` | 24,330 | .bas |
| 2 | `internal/c/libqb.cpp` | 17,492 | .cpp |
| 3 | `source/ide/ide_methods.bas` | 18,697 | .bas |
| 4 | `internal/c/libqb/src/graphics.cpp` | 7,589 | .cpp |
| 5 | `source/subs_functions/subs_functions.bas` | 3,865 | .bas |
| 6 | `internal/c/parts/audio/audio.cpp` | 3,190 | .cpp |
| 7 | `internal/c/libqb/src/text.cpp` | 2,121 | .cpp |
| 8 | `internal/support/converter/QB45BIN.bas` | 2,378 | .bas |
| 9 | `internal/c/qbx.cpp` | 1,925 | .cpp |
| 10 | `internal/c/libqb/src/fileio.cpp` | 1,680 | .cpp |
| 11 | `internal/c/libqb/src/shell.cpp` | 1,448 | .cpp |
| 12 | `internal/c/parts/video/font/font.cpp` | 1,332 | .cpp |

## Modularization Progress

The `libqb.cpp` file has been reduced from ~31,111 lines to **17,492 lines** through modularization:

| Module Extracted | Lines | Target File |
|-----------------|-------|-------------|
| Graphics/Drawing | 7,589 | `libqb/src/graphics.cpp` |
| Text & Font | 2,121 | `libqb/src/text.cpp` |
| File I/O | 1,680 | `libqb/src/fileio.cpp` |
| Networking | 894 | `libqb/src/networking.cpp` |
| Platform | 870 | `libqb/src/platform.cpp` |
| Color | 621 | `libqb/src/color.cpp` |
| Mouse | 355 | `libqb/src/mouse.cpp` |
| Port I/O | 249 | `libqb/src/port_io.cpp` |
| Screen | 195 | `libqb/src/screen.cpp` |
| Keyboard | 177 | `libqb/src/keyboard.cpp` |
| Utility | 173 | `libqb/src/utility.cpp` |
| Console | 171 | `libqb/src/console.cpp` |
| State Accessor | 168 | `libqb/src/libqb_state.cpp` |
| Window | 77 | `libqb/src/window.cpp` |
| Legacy Memory | 61 | `libqb/src/mem_legacy.cpp` |

**Total reduction:** ~13,619 lines (43.8%)

*Last updated: January 2026*
