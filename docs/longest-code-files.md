# Longest Code Files (Excluding Third-Party Libraries)

This document lists the longest code files in the QB64pe repository, ranked by line count. Only source code files (`.bas`, `.c`, `.cpp`, `.h`, `.hpp`) are included, and third-party libraries have been excluded.

## Top 50 QB64 Code Files by Line Count

| Rank | File | Lines | Size (bytes) | Extension |
|------|------|-------|--------------|-----------|
| 1 | `internal/c/libqb.cpp` | 27,261 | 1,158,572 | .cpp |
| 2 | `source/qb64pe.bas` | 21,218 | 1,087,350 | .bas |
| 3 | `source/ide/ide_methods.bas` | 18,697 | 861,236 | .bas |
| 4 | `internal/c/libqb/src/graphics.cpp` | 4,321 | 151,868 | .cpp |
| 7 | `source/subs_functions/subs_functions.bas` | 3,865 | 123,123 | .bas |
| 9 | `internal/c/parts/audio/audio.cpp` | 3,190 | 176,671 | .cpp |
| 13 | `internal/support/converter/QB45BIN.bas` | 2,378 | 90,764 | .bas |
| 17 | `internal/c/parts/audio/extras/libxmp-lite/player.c` | 1,975 | 57,554 | .c |
| 18 | `internal/c/qbx.cpp` | 1,925 | 72,827 | .cpp |
| 20 | `internal/c/parts/audio/extras/tinysoundfont/tsf.h` | 1,862 | 90,640 | .h |
| 24 | `internal/c/parts/audio/extras/hivelytracker/hvl_replay.c` | 1,717 | 87,390 | .c |
| 27 | `internal/c/parts/audio/extras/primesynth/primesynth.cpp` | 1,585 | 66,072 | .cpp |
| 28 | `tests/qbasic_testcases/misc/temple.bas` | 1,510 | 61,310 | .bas |
| 29 | `internal/c/parts/audio/extras/libxmp-lite/read_event.c` | 1,452 | 41,455 | .c |
| 30 | `internal/c/libqb/src/shell.cpp` | 1,448 | 58,025 | .cpp |
| 34 | `internal/c/parts/video/font/font.cpp` | 1,332 | 66,804 | .cpp |
| 35 | `internal/c/parts/audio/extras/libxmp-lite/it_load.c` | 1,328 | 36,159 | .c |
| 36 | `tests/qbasic_testcases/misc/carols.bas` | 1,327 | 70,174 | .bas |
| 38 | `internal/c/parts/core/gl_header_for_parsing/temp/gl_helper_code.h` | 1,296 | 40,725 | .h |
| 40 | `source/ide/wiki/wiki_methods.bas` | 1,253 | 64,579 | .bas |
| 41 | `internal/c/parts/audio/extras/radv2/player20.hpp` | 1,208 | 44,538 | .hpp |
| 46 | `internal/c/parts/audio/extras/libmidi/Recomposer/RCP.cpp` | 1,140 | 52,748 | .cpp |
| 47 | `tests/qbasic_testcases/pete/invader1/invader.bas` | 1,128 | 29,628 | .bas |
| 48 | `internal/c/libqb/src/gfs.cpp` | 1,100 | 35,897 | .cpp |

## Summary

- **Total files analyzed**: All tracked `.bas`, `.c`, `.cpp`, `.h`, `.hpp` files (excluding third-party libraries)
- **Largest QB64 file**: `internal/c/libqb.cpp` (27,261 lines)
- **Main QB64 source**: `source/qb64pe.bas` (21,218 lines)
- **IDE methods**: `source/ide/ide_methods.bas` (18,697 lines)
- **Graphics module**: `internal/c/libqb/src/graphics.cpp` (4,321 lines) - now 4th largest after modularization

The list now focuses on QB64's own codebase, excluding major third-party libraries like miniaudio, GLEW, FreeType, cURL, and others. Test programs and sample code still contribute significantly to the line count totals.

## Modularization Progress

The `libqb.cpp` file has been reduced from ~31,111 lines to 27,261 lines through modularization. Drawing primitives were moved to `graphics.cpp`, which grew from ~2,632 to 4,321 lines.
