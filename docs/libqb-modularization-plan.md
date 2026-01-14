# libqb.cpp Modularization Plan

## Overview

This document outlines a comprehensive plan to modularize `internal/c/libqb.cpp`, a 31,111-line monolithic file containing the core runtime library for QB64 Phoenix Edition.

---

## Progress Summary

### Completed Modules

| Module | Files | Status | Functions Extracted |
|--------|-------|--------|---------------------|
| **File I/O** | `fileio.cpp`, `fileio.h`, `handles.h` | ✅ Complete | `func_freefile`, generic I/O operations, number parsing |
| **Color** | `color.cpp`, `color.h` | ✅ Complete | `matchcol`, `func__rgb`, `func__rgba`, `func__red`, `func__green`, `func__blue`, `func__alpha`, `sub__clearcolor`, `func__clearcolor`, `func__defaultcolor`, `func__backgroundcolor`, `func__palettecolor`, `sub__palettecolor`, `sub__copypalette` |
| **Screen** | `screen.cpp`, `screen.h` | ✅ Complete | `sub__display`, `sub__autodisplay`, `sub__fullscreen`, `func__fullscreen`, `func__fullscreensmooth`, `sub__allowfullscreen`, `sub__resize`, `func__resize`, `func__resizewidth`, `func__resizeheight`, `func__scaledwidth`, `func__scaledheight`, `func__screenx`, `func__screeny` |
| **Memory Legacy** | `mem_legacy.cpp`, `mem_legacy.h` | ✅ Complete | `sub_defseg`, `func_peek`, `sub_poke` |
| **Keyboard** | `keyboard.cpp`, `keyboard.h` | ✅ Complete | `func__capslock`, `func__scrolllock`, `func__numlock`, `sub__capslock`, `sub__scrolllock`, `sub__numlock` |
| **Drawing** | `graphics.cpp`, `graphics.h` | ✅ Complete | `sub_line`, `sub_circle`, `sub_paint` (all variants), `sub_pset`, `sub_preset`, `func_point`, `point` |

### Deferred Modules

| Module | Reason |
|--------|--------|
| **Text & Font** | Deep integration with image system (`img[]`, `write_page`, `display_page`), font arrays (`fontwidth[]`, `fontheight[]`), and rendering pipeline |
| **Input** | Complex dependencies on mouse message queue, environment 2D variables, display page structures |
| **Window/UI** | Heavy platform-specific code (Windows API, X11, macOS), GLUT thread integration, console handle management |
| **Control Flow** | Cross-cutting dependencies on display, fonts, QBS string system; key functions already extracted to datetime.cpp |

### Build System Updates
- `libqb/build.mk` updated with: `fileio.o`, `color.o`, `screen.o`, `mem_legacy.o`, `keyboard.o`

### libqb.cpp Size Reduction
- Original: ~31,111 lines
- Current: ~27,261 lines (reduced by ~3,850 lines through modularization)

## Current State Analysis

### File Statistics
- **Location:** `internal/c/libqb.cpp`
- **Total Lines:** 31,111
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

## Functional Categories Identified

### 1. Graphics & Image Functions (~3,500 lines)
**Functions:**
- Image management: `func__newimage`, `func__copyimage`, `sub__freeimage`
- Image rendering: `sub__putimage`, `sub_graphics_put`, `sub_graphics_get`
- Drawing primitives: `sub_line`, `sub_circle`, `sub_pset`, `sub_preset`
- Fill operations: `sub_paint`, `sub_paint32`, `sub_paint32x`
- Page operations: `sub_pcopy`, `sub_paletteusing`
- Image queries: `func__width`, `func__height`, `func__pixelsize`

### 2. Screen & Display Functions (~2,500 lines)
**Functions:**
- Display control: `sub__display`, `func__display`, `sub__autodisplay`
- Screen dimensions: `func__screenwidth`, `func__screenheight`
- Clear screen: `sub_cls`, `sub_clsDest`
- Window management: `qbg_sub_window`, `func__screenx`, `func__screeny`
- Full-screen: `sub__fullscreen`, `func__fullscreen`, `func__fullscreensmooth`
- Resize handling: `sub__resize`, `func__resize`, `func__resizewidth`
- Display ordering: `sub__displayorder`, `sub__glrender`

### 3. Color & Palette Functions (~1,200 lines)
**Functions:**
- Color creation: `func__rgb`, `func__rgba`
- Channel extraction: `func__red`, `func__green`, `func__blue`, `func__alpha`
- Palette operations: `func__palettecolor`, `sub__palettecolor`, `sub__copypalette`
- Color defaults: `func__defaultcolor`, `func__backgroundcolor`
- Color clearing: `sub__clearcolor`, `func__clearcolor`

### 4. Text & Font Functions (~2,000 lines)
**Functions:**
- Font management: `sub__font`, `func__font`, `sub__freefont`
- Font metrics: `func__fontwidth`, `func__fontheight`
- Text rendering: `sub__printstring`, `sub__printimage`, `qbs_print`
- Text measurement: `func__printwidth`
- Print mode: `sub__printmode`, `func__printmode`
- Console font: `sub__consolefont`, `sub__console_cursor`

### 5. File I/O Functions (~3,000 lines)
**Functions:**
- File operations: `sub_open`, `sub_close`, `func_freefile`
- File reading: `sub_file_input_string`, `sub_file_line_input_string`
- File writing: `sub_file_print`
- Binary I/O: `sub_get`, `sub_get2`, `sub_put`, `sub_put2`, `sub_bload`, `sub_bsave`
- File positioning: `sub_seek`, `func_eof`, `func_lpos`
- Port I/O: `func_inp`, `sub_out`, `sub_wait`

### 6. Mouse/Input Functions (~1,500 lines)
**Functions:**
- Mouse position: `func__mousex`, `func__mousey`, `func__mousemovementx`, `func__mousemovementy`
- Mouse control: `sub__mousemove`, `sub__mousehide`, `sub__mouseshow`
- Mouse events: `func__mouseinput`, `func__mousebutton`, `func__mousewheel`
- Game controller: `func_stick`, `func_strig`
- File drop: `func__filedrop`, `sub__filedrop`, `sub__finishdrop`, `func__totaldroppedfiles`

### 7. Keyboard Functions (~1,200 lines)
**Functions:**
- Key state: `func__keyhit`, `func__keydown`
- Key management: `sub__keyclear`, `sub_lock`, `sub_unlock`
- Unicode mapping: `sub__mapunicode`, `func__mapunicode`
- Lock keys: `func__capslock`, `func__scrolllock`, `func__numlock`
- Lock control: `sub__capslock`, `sub__scrolllock`, `sub__numlock`

### 8. Memory & Debugging (~1,000 lines)
**Functions:**
- Memory access: `func_peek`, `sub_poke`, `sub_defseg`
- Debugging: `sub__assert`, `sub__echo`, `sub__writefile`, `func__readfile`
- Memory allocation: `mem_static_malloc`, `mem_static_restore`
- Dynamic memory: `cmem_dynamic_malloc`, `cmem_dynamic_free`
- Variable info: `func_varptr_helper`

### 9. Window & System Functions (~1,200 lines)
**Functions:**
- Window properties: `func_windowexists`, `func__hasfocus`
- Window management: `func__title`, `sub__title`, `sub__icon`
- Console operations: `sub__console`, `func__console`, `sub__consoletitle`, `func__getconsoleinput`
- Screen queries: `func_screenwidth`, `func_screenheight`, `func__screenimage`
- Screen visibility: `func__screenhide`, `sub__screenhide`, `sub__screenshow`
- Interaction: `sub__screenclick`, `sub__screenprint`

### 10. Network/HTTP Functions (~500 lines)
**Functions:**
- Connection management: `func__openclient`, `func__openhost`, `func__openconnection`
- Status queries: `func__connected`, `func__statusCode`
- Address lookup: `func__connectionaddress`

### 11. Graphics Blending (~800 lines)
**Functions:**
- Source/destination control: `sub__source`, `sub__dest`, `func__source`, `func__dest`
- Blending modes: `sub__blend`, `sub__dontblend`, `func__blend`
- Alpha operations: `sub__setalpha`, `func__alpha`

### 12. Control/System (~1,500 lines)
**Functions:**
- Program flow: `sub_end`, `sub_run`, `sub_run_init`, `func__exit`
- Timing: `func_timer`, `sub_sleep`, `sub__fps`
- Random numbers: `sub_randomize`, `func_rnd`
- Drawing: `sub_draw`

### 13. Utility Functions (~800 lines)
**Functions:**
- String builders: `func_space`, `func_spc`, `func_tab`
- Input: `func_input`
- Cursor: `func_pos`, `func_csrlin`, `func_pmap`, `func_point`
- System info: `func__os`

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

## Modularization Plan

### Phase 1: High Priority (Large, Self-Contained)

#### Module 1: File I/O ✅ COMPLETE
**Target:** `libqb/src/fileio.cpp` (~3,000+ lines)

| Item | Description |
|------|-------------|
| **Functions** | `sub_open`, `sub_close`, `func_freefile`, file read/write operations, `sub_get`, `sub_put`, `sub_bload`, `sub_bsave` |
| **Dependencies** | `qbs.h`, `error_handle.h`, `filesystem.h` |
| **Rationale** | File I/O is largely self-contained with clear boundaries. Large reduction in main file size. |

**Tasks:**
1. ✅ Create `libqb/src/fileio.cpp` and `libqb/include/fileio.h`
2. ✅ Extract `func_freefile` and generic I/O operations
3. ⏳ Extract binary I/O functions (`sub_get`, `sub_put`, `sub_bload`, `sub_bsave`) - partial
4. ⏳ Extract file positioning functions (`sub_seek`, `func_eof`, `func_lpos`) - partial
5. ✅ Created `handles.h` for file handle definitions
6. ✅ Update include statements in `libqb.cpp`

#### Module 2: Graphics Drawing ✅ COMPLETE
**Target:** `libqb/src/graphics.cpp` (~1,420 lines added)

| Item | Description |
|------|-------------|
| **Functions** | `sub_line`, `sub_circle`, `sub_pset`, `sub_preset`, `sub_paint` (all variants), `func_point`, `point` |
| **Dependencies** | `graphics.h`, `rounding.h`, `qbs.h`, `os.h` |
| **Rationale** | Drawing operations form a cohesive unit. High impact on code organization. |

**Status:** Drawing primitives extracted to existing `graphics.cpp`. Helper functions (`pset`, `pset_and_clip`, `fast_line`, `fast_boxfill`, `qb32_line`, `qb32_boxfill`) remain in libqb.cpp and are accessed via extern declarations.

**Tasks:**
1. ✅ Added declarations to `libqb/include/graphics.h`
2. ✅ Extract line drawing functions (`sub_line`)
3. ✅ Extract circle drawing functions (`sub_circle`)
4. ✅ Extract point operations (`point`, `func_point`)
5. ✅ Extract fill/paint operations (`sub_paint`, `sub_paint32`, `sub_paint32x`)
6. ✅ Extract pset/preset operations (`sub_pset`, `sub_preset`)
7. ✅ Added extern declarations for helper functions in graphics.cpp

---

### Phase 2: Medium Priority (Substantial, Well-Defined)

#### Module 3: Screen Management ✅ COMPLETE
**Target:** `libqb/src/screen.cpp` (~2,500+ lines)

| Item | Description |
|------|-------------|
| **Functions** | Screen initialization, `sub_cls`, display management, window operations, resize handling |
| **Dependencies** | `graphics.h`, `font.h`, `gui.h`, `glut-thread.h` |

**Tasks:**
1. ✅ Create `libqb/src/screen.cpp` and `libqb/include/screen.h`
2. ⏳ Extract screen initialization and mode setting - partial
3. ⏳ Extract `sub_cls` and clear operations - not yet
4. ✅ Extract display control (`sub__display`, `sub__autodisplay`)
5. ✅ Extract fullscreen and resize handling (`sub__fullscreen`, `func__fullscreen`, `func__fullscreensmooth`, `sub__allowfullscreen`, `sub__resize`, `func__resize`, `func__resizewidth`, `func__resizeheight`)
6. ✅ Extract screen dimension queries (`func__scaledwidth`, `func__scaledheight`, `func__screenx`, `func__screeny`)

#### Module 4: Color & Palette ✅ COMPLETE
**Target:** `libqb/src/color.cpp` (~1,200+ lines)

| Item | Description |
|------|-------------|
| **Functions** | RGB operations, palette manipulation, color queries |
| **Dependencies** | `graphics.h`, `image.h`, `error_handle.h` |

**Tasks:**
1. ✅ Create `libqb/src/color.cpp` and `libqb/include/color.h`
2. ✅ Extract RGB/RGBA creation functions (`func__rgb`, `func__rgba`)
3. ✅ Extract color channel extraction functions (`func__red`, `func__green`, `func__blue`, `func__alpha`)
4. ✅ Extract color matching (`matchcol` overloads)
5. ✅ Extract palette operations (`func__palettecolor`, `sub__palettecolor`, `sub__copypalette`)
6. ✅ Extract color default and clear functions (`func__defaultcolor`, `func__backgroundcolor`, `sub__clearcolor`, `func__clearcolor`)

#### Module 5: Text & Font ⏸️ DEFERRED
**Target:** `libqb/src/text.cpp` (~2,000+ lines)

| Item | Description |
|------|-------------|
| **Functions** | Font operations, text rendering, printing, font metrics |
| **Dependencies** | `font.h`, `image.h`, `graphics.h` |

**Status:** DEFERRED - Deep integration with:
- Image system (`img[]`, `write_page`, `display_page`)
- Font arrays (`fontwidth[]`, `fontheight[]`, `font[]`)
- Rendering pipeline and QBS string system

**Tasks:**
1. ⏸️ Create `libqb/src/text.cpp` and `libqb/include/text.h` - DEFERRED
2. ⏸️ Extract font management functions - DEFERRED
3. ⏸️ Extract text rendering functions (`qbs_print`, `sub__printstring`) - DEFERRED
4. ⏸️ Extract font metric queries - DEFERRED
5. ⏸️ Extract print mode functions - DEFERRED

#### Module 6: Input ⏸️ DEFERRED
**Target:** `libqb/src/input.cpp` (~3,000+ lines)

| Item | Description |
|------|-------------|
| **Functions** | Mouse, keyboard, game controller, file drop |
| **Dependencies** | `keyhandler.h`, `game_controller.h`, `event.h` |

**Status:** DEFERRED - Complex dependencies on:
- Mouse message queue structures
- Environment 2D variables (`environment_2d__screen_*`)
- Display page structures and font metrics
- Console input handling

**Tasks:**
1. ⏸️ Create `libqb/src/input.cpp` and `libqb/include/input.h` - DEFERRED
2. ⏸️ Extract mouse functions (position, control, events) - DEFERRED
3. ⏸️ Extract keyboard functions (keyhit, keydown, lock keys) - DEFERRED
4. ⏸️ Extract game controller functions (stick, strig) - DEFERRED
5. ⏸️ Extract file drop handling - DEFERRED
6. ⏸️ Move input buffer globals to module - DEFERRED

---

### Phase 3: Lower Priority (Cross-Cutting)

#### Module 7: Window/UI ⏸️ DEFERRED
**Target:** `libqb/src/window.cpp` (~1,200+ lines)

| Item | Description |
|------|-------------|
| **Functions** | Window properties, management, console operations |
| **Dependencies** | `gui.h`, `graphics.h`, Windows API, X11 |

**Status:** DEFERRED - Complex dependencies on:
- Platform-specific APIs (Windows API, X11, macOS)
- Console handle management
- GLUT thread integration
- QBS string system for title handling

#### Module 8: Memory/Debug ✅ PARTIAL (Legacy Memory Complete)
**Target:** `libqb/src/mem_legacy.cpp` (~100 lines extracted)

| Item | Description |
|------|-------------|
| **Functions** | Legacy memory access (peek/poke/defseg) |
| **Dependencies** | `cmem.h`, `error_handle.h` |

**Status:** Legacy memory functions extracted. Remaining debug functions (sub__assert, sub__echo, etc.) and dynamic memory allocation functions remain in libqb.cpp due to complex dependencies.

#### Module 9: Control Flow ⏸️ DEFERRED
**Target:** `libqb/src/control.cpp` (~1,500+ lines)

| Item | Description |
|------|-------------|
| **Functions** | Program flow, timing, random numbers, drawing commands |
| **Dependencies** | Various system headers, QBS string system, image system |

**Status:** DEFERRED - Complex dependencies on:
- func_timer already in datetime.cpp
- sub_end/sub_system require extensive cross-cutting access (display, fonts, QBS)
- sub_randomize uses QBS input functions for interactive mode
- Random number globals (rnd_seed) tightly coupled to initialization code

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
- Core `libqb.cpp` reduced to ~10,000 lines (coordination and remaining utilities)
- 7-9 focused modules averaging 2,000-3,000 lines each
- Clear separation of concerns
- Easier to maintain and extend
- Better code organization for contributors

### File Size Estimates

| Module | Estimated Lines |
|--------|-----------------|
| `libqb.cpp` (remaining) | ~10,000 |
| `fileio.cpp` | ~3,000 |
| `graphics_drawing.cpp` | ~3,500 |
| `screen.cpp` | ~2,500 |
| `color.cpp` | ~1,200 |
| `text.cpp` | ~2,000 |
| `input.cpp` | ~3,000 |
| `window.cpp` | ~1,200 |
| `memory_debug.cpp` | ~1,000 |
| `control.cpp` | ~1,500 |

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

1. All QB64 test programs compile and run correctly
2. No performance regression in common operations
3. Build system updated to compile all modules
4. Each module is self-contained with clear interfaces
5. Documentation updated to reflect new structure

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

### Files Created/Modified

| File | Purpose |
|------|---------|
| `libqb/src/fileio.cpp` | File I/O operations |
| `libqb/include/fileio.h` | File I/O declarations |
| `libqb/include/handles.h` | File handle type definitions |
| `libqb/src/color.cpp` | Color creation and extraction |
| `libqb/include/color.h` | Color function declarations |
| `libqb/src/screen.cpp` | Screen/display management |
| `libqb/include/screen.h` | Screen function declarations |
| `libqb/src/mem_legacy.cpp` | Legacy memory access (peek/poke/defseg) |
| `libqb/include/mem_legacy.h` | Legacy memory function declarations |
| `libqb/src/keyboard.cpp` | Keyboard lock key functions |
| `libqb/include/keyboard.h` | Keyboard function declarations |
| `libqb/src/graphics.cpp` | Drawing primitives (extended existing file) |
| `libqb/include/graphics.h` | Drawing primitive declarations (extended) |

### Build System Notes

The `libqb/build.mk` file uses the pattern:
```makefile
libqb-objs-y += $(PATH_LIBQB)/src/module.o
```

New modules must be added to this file to be compiled and linked.
