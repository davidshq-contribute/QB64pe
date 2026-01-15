# Longest Code Files (Excluding Third-Party Libraries)

This document lists the longest code files in the QB64pe repository, ranked by line count. Only source code files (`.bas`, `.c`, `.cpp`, `.h`, `.hpp`) are included, and third-party libraries have been excluded.

## Top QB64 Code Files by Line Count

| Rank | File | Lines | Extension |
|------|------|-------|-----------|
| 1 | `source/qb64pe.bas` | 24,330 | .bas |
| 2 | `source/ide/ide_methods.bas` | 20,864 | .bas |
| 3 | `internal/c/libqb.cpp` | 19,119 | .cpp |
| 4 | `internal/c/libqb/src/graphics.cpp` | 7,830 | .cpp |
| 5 | `source/subs_functions/subs_functions.bas` | 4,309 | .bas |
| 6 | `internal/c/parts/audio/audio.cpp` | 3,911 | .cpp |
| 7 | `internal/support/converter/QB45BIN.bas` | 3,231 | .bas |
| 8 | `internal/c/libqb/src/text.cpp` | 2,127 | .cpp |
| 9 | `internal/c/libqb/src/fileio.cpp` | 1,916 | .cpp |
| 10 | `internal/c/qbx.cpp` | 1,743 | .cpp |
| 11 | `internal/c/parts/video/font/font.cpp` | 1,604 | .cpp |
| 12 | `internal/c/libqb/src/shell.cpp` | 1,540 | .cpp |

## Modularization Progress

The `libqb.cpp` file has been reduced from ~31,111 lines to **19,119 lines** through modularization:

### Major Extracted Modules (>500 lines)

| Module Extracted | Lines | Target File |
|-----------------|-------|-------------|
| Graphics/Drawing | 7,830 | `libqb/src/graphics.cpp` |
| Text & Font | 2,127 | `libqb/src/text.cpp` |
| File I/O | 1,916 | `libqb/src/fileio.cpp` |
| Shell | 1,540 | `libqb/src/shell.cpp` |
| GFS (General File System) | 1,191 | `libqb/src/gfs.cpp` |
| Filesystem | 1,044 | `libqb/src/filesystem.cpp` |
| Networking | 894 | `libqb/src/networking.cpp` |
| Platform | 894 | `libqb/src/platform.cpp` |
| Color | 795 | `libqb/src/color.cpp` |
| QBS (String) | 725 | `libqb/src/qbs.cpp` |
| QBS MK/CV | 535 | `libqb/src/qbs_mk_cv.cpp` |
| HTTP | 530 | `libqb/src/http.cpp` |

### Medium Extracted Modules (100-500 lines)

| Module Extracted | Lines | Target File |
|-----------------|-------|-------------|
| Memory | 411 | `libqb/src/mem.cpp` |
| String Functions | 368 | `libqb/src/string_functions.cpp` |
| Mouse | 363 | `libqb/src/mouse.cpp` |
| DateTime | 357 | `libqb/src/datetime.cpp` |
| File Fields | 351 | `libqb/src/file-fields.cpp` |
| Error Handle | 346 | `libqb/src/error_handle.cpp` |
| Screen | 298 | `libqb/src/screen.cpp` |
| QBS Val | 294 | `libqb/src/qbs_val.cpp` |
| Hex/Oct/Bin | 290 | `libqb/src/hexoctbin.cpp` |
| QBS Str | 260 | `libqb/src/qbs_str.cpp` |
| Port I/O | 249 | `libqb/src/port_io.cpp` |
| QBS CMem | 235 | `libqb/src/qbs_cmem.cpp` |
| Window | 347 | `libqb/src/window.cpp` |
| State Accessor | 401 | `libqb/src/libqb_state.cpp` |
| QBS ToStr | 193 | `libqb/src/qbs__tostr.cpp` |
| GLUT Main Thread | 189 | `libqb/src/glut-main-thread.cpp` |
| Utility | 176 | `libqb/src/utility.cpp` |
| Keyboard | 176 | `libqb/src/keyboard.cpp` |
| Console | 173 | `libqb/src/console.cpp` |
| Filepath | 165 | `libqb/src/filepath.cpp` |
| QBList | 153 | `libqb/src/qblist.cpp` |
| GLUT Msg Queue | 145 | `libqb/src/glut-msg-queue.cpp` |
| Mac Mouse Support | 126 | `libqb/src/mac-mouse-support.cpp` |
| Networking Stub | 113 | `libqb/src/networking-stub.cpp` |
| Environ | 109 | `libqb/src/environ.cpp` |
| Threading Windows | 108 | `libqb/src/threading-windows.cpp` |
| Threading POSIX | 104 | `libqb/src/threading-posix.cpp` |

### Small Extracted Modules (<100 lines)

| Module Extracted | Lines | Target File |
|-----------------|-------|-------------|
| Buffer | 94 | `libqb/src/buffer.cpp` |
| Console-Only Main | 79 | `libqb/src/console-only-main-thread.cpp` |
| Command | 69 | `libqb/src/command.cpp` |
| HTTP Stub | 68 | `libqb/src/http-stub.cpp` |
| Legacy Memory | 64 | `libqb/src/mem_legacy.cpp` |
| GLUT Message | 59 | `libqb/src/glut-message.cpp` |
| Bitops | 38 | `libqb/src/bitops.cpp` |
| Threading | 36 | `libqb/src/threading.cpp` |

### Summary

| Metric | Value |
|--------|-------|
| Original libqb.cpp | 31,111 lines |
| Current libqb.cpp | 18,947 lines |
| Total extracted | 27,028 lines (47 modules) |
| Reduction | 12,164 lines (39.1%) |

*Last updated: January 2026*
