# libqb.cpp Modularization Plan

## Overview

This document tracks the modularization of `internal/c/libqb.cpp`, the core runtime library for QB64 Phoenix Edition.

**Current status:** 23,359 lines (reduced from 31,111 - a 24.9% reduction)

---

## Completed Modules

| Module | Target File | Lines | Key Functions |
|--------|-------------|-------|---------------|
| **Graphics** | `libqb/src/graphics.cpp` | 7,589 | `sub_line`, `sub_circle`, `sub_pset`, `sub_paint`, `sub_draw`, `sub__putimage`, GET/PUT, image management |
| **File I/O** | `libqb/src/fileio.cpp` | 1,680 | `func_freefile`, generic I/O, number parsing |
| **Color** | `libqb/src/color.cpp` | 621 | `func__rgb`, `func__rgba`, channel extraction, palette ops |
| **Mouse** | `libqb/src/mouse.cpp` | 355 | Mouse position and button functions |
| **Screen** | `libqb/src/screen.cpp` | 195 | `sub__display`, `sub__fullscreen`, `sub__resize`, dimension queries |
| **Utility** | `libqb/src/utility.cpp` | 173 | `sub_randomize`, `func_rnd`, `sub__fps`, `sub__blink`, `sub__controlchr` |
| **Console** | `libqb/src/console.cpp` | 171 | Console output functions |
| **State Accessor** | `libqb/src/libqb_state.cpp` | 168 | Image/font/page accessors (enables further modularization) |
| **Keyboard** | `libqb/src/keyboard.cpp` | 102 | Lock key functions |
| **Window** | `libqb/src/window.cpp` | 77 | `func__handle`, `func__title`, `func__hasfocus` |
| **Legacy Memory** | `libqb/src/mem_legacy.cpp` | 61 | `func_peek`, `sub_poke`, `sub_defseg` |

**Build system:** All modules added to `libqb/build.mk`

---

## State Accessor Layer (NEW)

The `libqb_state` module provides controlled access to global state, enabling further modularization:

```cpp
// Image system
img_struct* libqb_get_write_page();
img_struct* libqb_get_read_page();
img_struct* libqb_get_display_page();
img_struct* libqb_get_image(int32_t handle);

// Page indexes
int32_t libqb_get_write_page_index();
void libqb_set_write_page_index(int32_t index);

// Font system
int32_t libqb_get_font_width(int32_t font_handle);
int32_t libqb_get_font_height(int32_t font_handle);

// Environment 2D
int32_t libqb_get_screen_width();
int32_t libqb_get_screen_height();
```

**Usage:** Include `libqb_state.h` instead of using extern declarations for globals.

---

## Ready for Extraction (with State Accessors)

These modules can now be extracted using the state accessor layer:

| Module | Est. Lines | Required Accessors |
|--------|------------|-------------------|
| **Text & Font** | ~2,000 | `libqb_get_write_page()`, `libqb_get_font_*()` |
| **Tab/Spc** | ~200 | `libqb_get_write_page()`, `libqb_get_font_width()` |
| **Port I/O** | ~150 | `libqb_get_write_page()` (for palette access) |
| **Window Queries** | ~80 | Platform APIs only (no accessor needed) |

---

## Deferred Modules

These modules have complex dependencies beyond what the accessor layer provides:

| Module | Est. Lines | Blocking Dependencies |
|--------|------------|----------------------|
| **Input** | ~3,000 | Mouse message queue, key event system, display page structures |
| **Window/UI** | ~1,200 | Platform APIs (Windows, X11, macOS), GLUT thread, console handles |
| **Control Flow** | ~1,500 | Display, fonts, QBS string system (cross-cutting) |
| **x86 Emulator** | ~1,100 | CMEM memory management (deeply interleaved) |

**Path forward:** Extend the accessor layer as needed, or extract with platform-specific wrappers.

---

## Critical Global State

These globals are accessed across modules. Many now have accessors via `libqb_state.h`:

### Image System
- `img[]`, `nextimg`, `pages[]`, `nimg`, `fimg[]`
- **Accessors:** `libqb_get_image()`, `libqb_get_write_page()`, `libqb_get_read_page()`, `libqb_get_display_page()`

### Font System
- `font[]`, `fontheight[]`, `fontwidth[]`, `fontflags[]`
- **Accessors:** `libqb_get_font_width()`, `libqb_get_font_height()`, `libqb_get_font_flags()`

### Page/View State
- `display_page_index`, `write_page_index`, `read_page_index`
- **Accessors:** `libqb_get_*_page_index()`, `libqb_set_*_page_index()`

### Display Settings
- `environment_2d_*` variables (screen scaling/positioning)
- **Accessors:** `libqb_get_screen_width()`, `libqb_get_screen_height()`, etc.

### Input Buffers (no accessors yet)
- `keyhit[]`, `port60h_event[]`, key state arrays

---

## Existing Headers

libqb.cpp includes 33 modularized headers:

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

## Lessons Learned

### Patterns That Extract Well

1. **Minimal global state dependencies** - Only rely on a few extern variables
2. **Clear input/output interfaces** - Well-defined parameters and return values
3. **Self-contained logic** - Don't call many internal helper functions
4. **Standard conditional compilation** - Platform-specific code works with `#ifdef` guards

### Patterns That Block Extraction

1. **Deep image system integration** - Functions using `write_page`, `display_page`, `img[]`
2. **Font array dependencies** - Functions accessing `fontwidth[]`, `fontheight[]`, `font[]`
3. **Environment 2D variables** - Functions using `environment_2d__screen_*` scaling
4. **Message queue structures** - Input functions accessing `mouse_message_queue`
5. **Cross-cutting helpers** - Functions like `pset_and_clip`, `lineclip` used by multiple operations

### Recommended Approach for Future Extraction

1. **Create accessor functions first** - Before extracting, create accessors for required globals
2. **Extract in dependency order** - Lower-level utilities before higher-level functions
3. **Create shared state header** - `libqb_state.h` with extern declarations for all globals
4. **Batch related functions** - Extract tightly-coupled function groups together
5. **Test incrementally** - Compile after each function extraction

---

## Files Created

| File | Purpose |
|------|---------|
| `libqb/src/graphics.cpp` | Drawing primitives, DRAW, _PUTIMAGE, GET/PUT, image management |
| `libqb/include/graphics.h` | Graphics declarations |
| `libqb/src/fileio.cpp` | File I/O operations |
| `libqb/include/fileio.h` | File I/O declarations |
| `libqb/include/handles.h` | File handle type definitions |
| `libqb/src/color.cpp` | Color creation and extraction |
| `libqb/include/color.h` | Color declarations |
| `libqb/src/mouse.cpp` | Mouse functions |
| `libqb/include/mouse.h` | Mouse declarations |
| `libqb/src/screen.cpp` | Screen/display management |
| `libqb/include/screen.h` | Screen declarations |
| `libqb/src/console.cpp` | Console output functions |
| `libqb/include/console.h` | Console declarations |
| `libqb/src/libqb_state.cpp` | Global state accessor layer |
| `libqb/include/libqb_state.h` | State accessor declarations |
| `libqb/src/utility.cpp` | RND, RANDOMIZE, _FPS, _BLINK, _CONTROLCHR functions |
| `libqb/include/utility.h` | Utility function declarations |
| `libqb/src/window.cpp` | Window handle, title, focus functions |
| `libqb/include/window.h` | Window function declarations |
| `libqb/src/keyboard.cpp` | Keyboard lock key functions |
| `libqb/include/keyboard.h` | Keyboard declarations |
| `libqb/src/mem_legacy.cpp` | Legacy memory access (peek/poke/defseg) |
| `libqb/include/mem_legacy.h` | Legacy memory declarations |

### Build System

New modules use this pattern in `libqb/build.mk`:
```makefile
libqb-objs-y += $(PATH_LIBQB)/src/module.o
```
