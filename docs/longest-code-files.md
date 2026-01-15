# Longest Code Files (Excluding Third-Party Libraries)

This document lists the longest code files in the QB64pe repository, ranked by line count. Only source code files (`.bas`, `.c`, `.cpp`, `.h`, `.hpp`) are included, and third-party libraries have been excluded.

## Top QB64 Code Files by Line Count

| Rank | File | Lines | Extension |
|------|------|-------|-----------|
| 1 | `internal/c/libqb.cpp` | 23,442 | .cpp |
| 2 | `source/qb64pe.bas` | 21,218 | .bas |
| 3 | `source/ide/ide_methods.bas` | 18,697 | .bas |
| 4 | `internal/c/libqb/src/graphics.cpp` | 7,587 | .cpp |
| 5 | `source/subs_functions/subs_functions.bas` | 3,865 | .bas |
| 6 | `internal/c/parts/audio/audio.cpp` | 3,190 | .cpp |
| 7 | `internal/support/converter/QB45BIN.bas` | 2,378 | .bas |
| 8 | `internal/c/qbx.cpp` | 1,925 | .cpp |
| 9 | `internal/c/libqb/src/shell.cpp` | 1,448 | .cpp |
| 10 | `internal/c/parts/video/font/font.cpp` | 1,332 | .cpp |
| 11 | `source/ide/wiki/wiki_methods.bas` | 1,253 | .bas |
| 12 | `internal/c/libqb/src/gfs.cpp` | 1,100 | .cpp |

## Modularization Progress

The `libqb.cpp` file has been reduced from ~31,111 lines to 23,442 lines through modularization:

| Module Extracted | Lines | Target File |
|-----------------|-------|-------------|
| Graphics/Drawing | ~7,587 | `libqb/src/graphics.cpp` |
| File I/O | ~1,685 | `libqb/src/fileio.cpp` |
| Screen | ~2,500 | `libqb/src/screen.cpp` |
| Color | ~1,200 | `libqb/src/color.cpp` |
| Keyboard | ~200 | `libqb/src/keyboard.cpp` |
| Mouse | ~300 | `libqb/src/mouse.cpp` |
| Console | ~170 | `libqb/src/console.cpp` |
| Legacy Memory | ~100 | `libqb/src/mem_legacy.cpp` |

**Total reduction:** ~7,669 lines (24.6%)
