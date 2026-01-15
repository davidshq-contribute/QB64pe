# libqb.cpp Modularization Plan

## Overview

This document outlines a comprehensive plan to modularize `internal/c/libqb.cpp`, a 31,111-line monolithic file containing the core runtime library for QB64 Phoenix Edition.

---

## Progress Summary

### Completed Modules ✅

| Module | Target File | Lines Extracted | Status |
|--------|-------------|-----------------|---------|
| **File I/O** | `libqb/src/fileio.cpp` | ~1,685 | COMPLETE |
| **Graphics Drawing** | `libqb/src/graphics.cpp` | ~7,587 | COMPLETE |
| **Screen Management** | `libqb/src/screen.cpp` | ~2,500 | COMPLETE |
| **Color & Palette** | `libqb/src/color.cpp` | ~1,200 | COMPLETE |
| **Legacy Memory** | `libqb/src/mem_legacy.cpp` | ~100 | COMPLETE |
| **Keyboard Functions** | `libqb/src/keyboard.cpp` | ~200 | COMPLETE |
| **Mouse Functions** | `libqb/src/mouse.cpp` | ~300 | COMPLETE |
| **Console Functions** | `libqb/src/console.cpp` | ~170 | COMPLETE |

### Deferred Modules ⏸️

| Module | Reason |
|--------|--------|
| **Text & Font** | Deep integration with image system (`img[]`, `write_page`, `display_page`), font arrays (`fontwidth[]`, `fontheight[]`), and rendering pipeline |
| **Input** | Complex dependencies on mouse message queue, environment 2D variables, display page structures |
| **Window/UI** | Heavy platform-specific code (Windows API, X11, macOS), GLUT thread integration, console handle management |
| **Control Flow** | Cross-cutting dependencies on display, fonts, QBS string system; key functions already extracted to datetime.cpp |
| **x86 Emulator** | ~1,100 lines of CPU emulation code deeply interleaved with CMEM memory management; would require significant refactoring |
| **File Print/Input** | ~2,000 lines of file print/input functions heavily interleaved with DATA/READ functions and complex qbs dependencies |

### Build System Updates
- `libqb/build.mk` updated with: `fileio.o`, `color.o`, `screen.o`, `mem_legacy.o`, `keyboard.o`, `mouse.o`, `console.o`, `graphics.o`

### libqb.cpp Size Reduction
- Original: ~31,111 lines
- Current: ~23,442 lines (reduced by ~7,669 lines through modularization)

## Current State Analysis

### File Statistics
- **Location:** `internal/c/libqb.cpp`
- **Total Lines:** ~23,442 (as of latest modularization)
- **QB Functions:** ~194 function implementations (`func_*` and `sub_*`)

### Existing Modularization

The file already includes 33 modularized headers:

| Category | Headers |
|----------|---------|
| Audio | `audio.h` |
| Data/Encoding | `bitops.h`, `compression.h`, `encoding.h` |
| Files/System | `filesystem.h`, `filepath.h`, `file-fields.h`, `gfs.h`, `shell.h`, `command.h` |
| Graphics | `graphics.h`, `image.h`, `font.h`, `gui.h` |
| Input | `keyhandler.h`, `game_controller.h` |
| I/O | `http.h` |
| Memory | `cmem.h`, `mem.h` |
| String | `qbs.h` |
| System | `datetime.h`, `error_handle.h`, `event.h`, `logging.h`, `thread.h`, `mutex.h` |
| UI/Threading | `glut-thread.h`, `mac-mouse-support.h` |
| Utilities | `extended_math.h`, `hashing.h`, `completion.h`, `rounding.h`, `qblist.h` |

---

---

## Module Details

### ✅ COMPLETED MODULES

#### Module 1: File I/O 
**Target:** `libqb/src/fileio.cpp` (~3,000 lines)
- **Functions:** `sub_open`, `sub_close`, `func_freefile`, file read/write operations, `sub_get`, `sub_put`, `sub_bload`, `sub_bsave`, `sub_seek`, `func_eof`, `func_lpos`
- **Dependencies:** `qbs.h`, `error_handle.h`, `filesystem.h`
- **Files Created:** `libqb/src/fileio.cpp`, `libqb/include/fileio.h`, `libqb/include/handles.h`

#### Module 2: Graphics Drawing 
**Target:** `libqb/src/graphics.cpp` (~7,587 lines)
- **Functions:** `sub_line`, `sub_circle`, `sub_pset`, `sub_preset`, `sub_paint` (all variants), `func_point`, `point`, `sub_draw`, `sub__putimage`, `sub_graphics_get`, `sub_graphics_put`, image management (`func__newimage`, `func__copyimage`, `sub__freeimage`, `func__source`, `func__dest`, etc.)
- **Dependencies:** `graphics.h`, `rounding.h`, `qbs.h`, `os.h`, `common.h`
- **Status:** All drawing primitives, DRAW command, _PUTIMAGE, GET/PUT graphics, and image management functions extracted. Helper functions (`pset`, `pset_and_clip`, `fast_line`, `fast_boxfill`, `qb32_line`, `qb32_boxfill`) remain in libqb.cpp and are accessed via extern declarations.

#### Module 3: Screen Management 
**Target:** `libqb/src/screen.cpp` (~2,500 lines)
- **Functions:** Screen initialization, display control (`sub__display`, `sub__autodisplay`), fullscreen and resize handling (`sub__fullscreen`, `func__fullscreen`, `func__fullscreensmooth`, `sub__resize`, `func__resize`, `func__resizewidth`, `func__resizeheight`), screen dimension queries (`func__scaledwidth`, `func__scaledheight`, `func__screenx`, `func__screeny`)
- **Dependencies:** `graphics.h`, `font.h`, `gui.h`, `glut-thread.h`

#### Module 4: Color & Palette 
**Target:** `libqb/src/color.cpp` (~1,200 lines)
- **Functions:** RGB/RGBA creation (`func__rgb`, `func__rgba`), color channel extraction (`func__red`, `func__green`, `func__blue`, `func__alpha`), color matching (`matchcol` overloads), palette operations (`func__palettecolor`, `sub__palettecolor`, `sub__copypalette`), color defaults and clearing (`func__defaultcolor`, `func__backgroundcolor`, `sub__clearcolor`, `func__clearcolor`)
- **Dependencies:** `graphics.h`, `image.h`, `error_handle.h`

#### Module 5: Legacy Memory 
**Target:** `libqb/src/mem_legacy.cpp` (~100 lines)
- **Functions:** Legacy memory access (`func_peek`, `sub_poke`, `sub_defseg`)
- **Dependencies:** `cmem.h`, `error_handle.h`

#### Module 6: Keyboard Functions 
**Target:** `libqb/src/keyboard.cpp` (~200 lines)
- **Functions:** Lock key functions (`func__capslock`, `func__scrolllock`, `func__numlock`, `sub__capslock`, `sub__scrolllock`, `sub__numlock`)
- **Dependencies:** `keyhandler.h`

###  DEFERRED MODULES

#### Module 7: Text & Font 
**Target:** `libqb/src/text.cpp` (~2,000 lines)
- **Functions:** Font operations, text rendering, printing, font metrics
- **Dependencies:** `font.h`, `image.h`, `graphics.h`
- **Deferral Reason:** Deep integration with image system (`img[]`, `write_page`, `display_page`), font arrays (`fontwidth[]`, `fontheight[]`, `font[]`), and rendering pipeline

#### Module 8: Input 
**Target:** `libqb/src/input.cpp` (~3,000 lines)
- **Functions:** Mouse, keyboard, game controller, file drop
- **Dependencies:** `keyhandler.h`, `game_controller.h`, `event.h`
- **Deferral Reason:** Complex dependencies on mouse message queue structures, environment 2D variables, display page structures and font metrics

#### Module 9: Window/UI 
**Target:** `libqb/src/window.cpp` (~1,200 lines)
- **Functions:** Window properties, management, console operations
- **Dependencies:** `gui.h`, `graphics.h`, Windows API, X11
- **Deferral Reason:** Platform-specific APIs, console handle management, GLUT thread integration

#### Module 10: Control Flow 
**Target:** `libqb/src/control.cpp` (~1,500 lines)
- **Functions:** Program flow, timing, random numbers, drawing commands
- **Dependencies:** Various system headers, QBS string system, image system
- **Deferral Reason:** Cross-cutting dependencies, key functions already in datetime.cpp

---

## Critical Global State

These global variables must be carefully managed during extraction:

### Image System
- `img[]`, `nextimg`, `pages[]`, `nimg`, `fimg[]`

### Palette System
- `palette_256[]`, `palette_64[]`, `pal[]`

### Page/View State
- `display_page_index`, `write_page_index`, `read_page_index`

### Font System
- `font[]`, `fontheight[]`, `fontwidth[]`, `fontflags[]`

### Display Settings
- `environment_2d_*` variables, screen scaling

### Input Buffers
- `keyhit[]`, `port60h_event[]`, key state arrays

### Rendering State
- Blend modes, alpha settings, hardware commands

---

## Dependency Graph

```
                    ┌─────────────────┐
                    │   libqb.cpp     │
                    │   (core/glue)   │
                    └────────┬────────┘
                             │
        ┌────────────────────┼────────────────────┐
        │                    │                    │
        ▼                    ▼                    ▼
┌───────────────┐   ┌───────────────┐   ┌───────────────┐
│   File I/O    │   │   Graphics    │   │    Input      │
│   fileio.cpp  │   │   Drawing     │   │   input.cpp   │
└───────────────┘   └───────┬───────┘   └───────────────┘
                            │
              ┌─────────────┼─────────────┐
              │             │             │
              ▼             ▼             ▼
      ┌───────────┐  ┌───────────┐  ┌───────────┐
      │  Screen   │  │   Color   │  │   Text    │
      │screen.cpp │  │ color.cpp │  │ text.cpp  │
      └───────────┘  └───────────┘  └───────────┘
              │             │             │
              └─────────────┼─────────────┘
                            │
                            ▼
                    ┌───────────────┐
                    │  Shared State │
                    │   (globals)   │
                    └───────────────┘
```

### Key Dependencies

| Module | Depends On |
|--------|------------|
| Graphics Drawing | Screen, Color, Palette, Font |
| File I/O | String (qbs), Filesystem, Error Handling |
| Text | Font, Color, Graphics |
| Input | Event System, Keyhandler |
| Screen | Graphics, Color, Font, Palette |

---

## Implementation Strategy

### Step 1: Preparation
1. Create directory structure: `libqb/src/` and `libqb/include/`
2. Set up build system to compile new modules
3. Create shared header for global state access

### Step 2: Module Extraction Process
For each module:
1. Create new `.cpp` file with appropriate includes
2. Create corresponding `.h` header with public interface
3. Move function implementations (keep signatures identical)
4. Extract related global variables or create accessor functions
5. Add `#include` for new header in `libqb.cpp`
6. Build and test incrementally

### Step 3: Global State Management
Options for handling global state:
- **Option A:** Keep globals in `libqb.cpp`, expose via accessor functions
- **Option B:** Move globals to relevant modules, expose via headers
- **Option C:** Create dedicated state management module

**Recommended:** Option B for module-specific state, Option A for cross-cutting state

### Step 4: Testing
1. Build after each module extraction
2. Run existing test suite
3. Test QB64 programs using affected functions
4. Verify no regressions in functionality

---

## Expected Outcomes

### Before Modularization
- Single 31,111-line file
- Difficult to navigate and maintain
- Hard to understand dependencies
- Challenging for new contributors

### After Modularization
- Core `libqb.cpp` reduced to ~23,581 lines (coordination and remaining utilities)
- 6 completed modules + 4 deferred modules
- Clear separation of concerns
- Easier to maintain and extend
- Better code organization for contributors

### File Size Estimates

| Module | Status | Lines |
|--------|--------|-------|
| `libqb.cpp` (remaining) | Active | ~23,581 |
| `fileio.cpp` | ✅ Complete | ~3,000 |
| `graphics.cpp` | ✅ Complete | ~7,587 |
| `screen.cpp` | ✅ Complete | ~2,500 |
| `color.cpp` | ✅ Complete | ~1,200 |
| `mem_legacy.cpp` | ✅ Complete | ~100 |
| `keyboard.cpp` | ✅ Complete | ~200 |
| `text.cpp` | ⏸️ Deferred | ~2,000 |
| `input.cpp` | ⏸️ Deferred | ~3,000 |
| `window.cpp` | ⏸️ Deferred | ~1,200 |
| `control.cpp` | ⏸️ Deferred | ~1,500 |

---

## Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| Breaking existing functionality | Incremental extraction with testing after each module |
| Global state access issues | Careful analysis of dependencies before extraction |
| Build system complexity | Update CMake/Makefiles incrementally |
| Circular dependencies | Design clear interfaces; use forward declarations |
| Performance regression | Profile critical paths; avoid unnecessary indirection |

---

## Success Criteria

1. ✅ All QB64 test programs compile and run correctly (for completed modules)
2. ✅ No performance regression in common operations
3. ✅ Build system updated to compile all modules
4. ✅ Each module is self-contained with clear interfaces
5. ✅ Documentation updated to reflect new structure

---

## Lessons Learned

### Successfully Extracted Function Patterns

Functions that were successfully extracted share these characteristics:
1. **Minimal global state dependencies** - Only rely on a few extern variables
2. **Clear input/output interfaces** - Well-defined parameters and return values
3. **Self-contained logic** - Don't call many internal helper functions
4. **Conditional compilation handled** - Platform-specific code works with standard `#ifdef` guards

### Challenging Patterns (Deferred)

Functions that proved difficult to extract:
1. **Deep image system integration** - Functions using `write_page`, `display_page`, `img[]` array
2. **Font array dependencies** - Functions accessing `fontwidth[]`, `fontheight[]`, `font[]`
3. **Environment 2D variables** - Functions using `environment_2d__screen_*` scaling/positioning
4. **Message queue structures** - Input functions accessing `mouse_message_queue`
5. **Cross-cutting helpers** - Functions like `pset_and_clip`, `lineclip` used by multiple drawing operations

### Recommended Approach for Future Extraction

1. **Create accessor functions first** - Before extracting a module, create accessor functions for required globals in libqb.cpp
2. **Extract in dependency order** - Lower-level utilities before higher-level functions
3. **Consider a shared state header** - Create `libqb_state.h` with extern declarations for all commonly-accessed globals
4. **Batch related functions** - Extract tightly-coupled function groups together
5. **Test incrementally** - Compile after each function extraction

## Files Created/Modified

| File | Purpose | Status |
|------|---------|--------|
| `libqb/src/fileio.cpp` | File I/O operations | ✅ Complete |
| `libqb/include/fileio.h` | File I/O declarations | ✅ Complete |
| `libqb/include/handles.h` | File handle type definitions | ✅ Complete |
| `libqb/src/color.cpp` | Color creation and extraction | ✅ Complete |
| `libqb/include/color.h` | Color function declarations | ✅ Complete |
| `libqb/src/screen.cpp` | Screen/display management | ✅ Complete |
| `libqb/include/screen.h` | Screen function declarations | ✅ Complete |
| `libqb/src/mem_legacy.cpp` | Legacy memory access (peek/poke/defseg) | ✅ Complete |
| `libqb/include/mem_legacy.h` | Legacy memory function declarations | ✅ Complete |
| `libqb/src/keyboard.cpp` | Keyboard lock key functions | ✅ Complete |
| `libqb/include/keyboard.h` | Keyboard function declarations | ✅ Complete |
| `libqb/src/graphics.cpp` | Drawing primitives (extended existing file) | ✅ Complete |
| `libqb/include/graphics.h` | Drawing primitive declarations (extended) | ✅ Complete |

### Build System Notes

The `libqb/build.mk` file uses the pattern:
```makefile
libqb-objs-y += $(PATH_LIBQB)/src/module.o
```

New modules must be added to this file to be compiled and linked.
