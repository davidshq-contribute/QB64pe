# libqb.cpp Modularization Plan

## Overview

This document tracks the modularization of `internal/c/libqb.cpp`, the core runtime library for QB64 Phoenix Edition.

**Current status:** 19,982 lines (reduced from 31,111 - a 35.8% reduction)

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
| **Keyboard** | `libqb/src/keyboard.cpp` | 177 | Lock key functions, `func__keyhit`, `func__keydown`, `sub__mapunicode`, `func__mapunicode` |
| **Window** | `libqb/src/window.cpp` | 77 | `func__handle`, `func__title`, `func__hasfocus` |
| **Legacy Memory** | `libqb/src/mem_legacy.cpp` | 61 | `func_peek`, `sub_poke`, `sub_defseg` |
| **Text & Font** | `libqb/src/text.cpp` | 2,121 | `printchr`, `qbs_print`, `qbg_sub_locate`, `sub_cls`, `func_csrlin`, `func_pos`, `func_tab`, `func_spc`, font management (`_LOADFONT`, `_FONT`, `_FREEFONT`), print modes |
| **Port I/O** | `libqb/src/port_io.cpp` | 249 | `sub_out`, `func_inp`, `sub_wait` - VGA palette emulation, keyboard scancode, retrace timing |
| **Platform** | `libqb/src/platform.cpp` | 870 | `sub__screenprint` - keyboard input simulation (Windows SendInput, macOS CGEvent) |

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

## Remaining Code Analysis

The ~20,000 remaining lines break down into these functional areas:

| Area | Lines | Location | Self-Contained? |
|------|-------|----------|-----------------|
| Global state & data structures | ~1,700 | 61-1750 | N/A (declarations) |
| Display/render loop | ~1,371 | 16396-18797 | No (core orchestrator) |
| Key input infrastructure | ~1,200 | 14289-19899 | No (state machine) |
| Graphics mode management | ~1,050 | 5845-6866 | No (deep integration) |
| INPUT statement | ~830 | 8513-9342 | No (cross-cutting) |
| Networking/TCP | ~650 | 12700-13600 | **Yes** |
| PRINT USING | ~600 | 11450-12600 | Moderate |
| Control flow/runtime | ~600 | scattered | No |
| Hardware texture mgmt | ~400 | 3600-4100 | **Yes** |
| Window/GUI operations | ~300 | scattered | **Yes** |
| Mouse GLUT callbacks | ~215 | 16946-17192 | Moderate |

---

## ROI-Prioritized Extraction Roadmap

### Tier 1: High ROI (Self-contained, minimal dependencies)

| Module | Lines | Effort | Impact | Notes |
|--------|-------|--------|--------|-------|
| **Networking** | ~650 | Low | High | TCP/socket functions are isolated. Only needs GFS and error_handle externs. |
| **Hardware Textures** | ~400 | Low | Medium | `newimg()`, `freeimg()`, `new_hardware_img()`, texture upload functions. |
| **Window Operations** | ~300 | Low | Medium | Expand existing window.cpp with `sub__screenmove()`, `sub__icon()`, file drop handlers. |

**Recommendation:** Extract networking first - highest line count with lowest complexity.

### Tier 2: Medium ROI (Needs accessor layer expansion)

| Module | Lines | Effort | Impact | Blockers |
|--------|-------|--------|--------|----------|
| **Mouse GLUT Callbacks** | ~215 | Medium | Low | Needs mouse state accessors, display offset calculation |
| **PRINT USING** | ~600 | Medium | Medium | Coupled to text rendering, cursor state |
| **Printer** | ~100 | Medium | Low | Deep img_struct access |

### Tier 3: Low ROI (Architectural changes required)

| Module | Lines | Effort | Impact | Why Difficult |
|--------|-------|--------|--------|---------------|
| **Key Input** | ~1,200 | High | Medium | Complex state machine with keyhit buffer, keyheld arrays, modifier tracking |
| **Graphics Mode** | ~1,050 | High | Medium | `qbg_screen()` alone is 607 lines touching fonts, palettes, pages |
| **INPUT Statement** | ~830 | Very High | Low | Cross-cuts graphics, text, input, cursor - 4+ major systems |

### Tier 4: Core System (Not Extractable)

| Area | Lines | Why |
|------|-------|-----|
| **Display/Render Loop** | ~1,371 | Central orchestrator - `display()` and `GLUT_DISPLAY_REQUEST()` coordinate everything |
| **Global State** | ~1,700 | Foundation data structures - can only add accessors, not move |
| **Control Flow** | ~600 | GOSUB/RETURN, CHAIN, exit handlers - deeply integrated |

---

## Recommended Next Steps

### Immediate (High ROI)

1. **Extract Networking Module** (~650 lines)
   - Functions: `tcp_init`, `tcp_done`, `tcp_host_open`, `tcp_client_open`, `connection_new`, stream operations
   - Dependencies: GFS module, error_handle (both already have headers)
   - Estimated effort: 2-3 hours

2. **Expand Window Module** (~300 lines)
   - Add: `sub__screenmove`, `sub__icon`, `sub__filedrop`, `func__filedrop`
   - Current window.cpp is only 77 lines - room to grow
   - Platform wrappers already exist in the file

3. **Extract Hardware Texture Functions** (~400 lines)
   - Functions: `newimg`, `freeimg`, `imgrevert`, `new_hardware_img`, texture management
   - Could merge into existing graphics.cpp or create new hardware.cpp
   - Low coupling to other systems

### Deferred (Low ROI or blocking)

- **Key Input Infrastructure** - Would require event system redesign
- **Graphics Mode Setup** - `qbg_screen()` is too entangled with fonts/palettes/pages
- **INPUT Statement** - Cross-cuts too many systems; leave in libqb.cpp
- **Display Loop** - Core orchestrator must remain

### Strategic Considerations

**Diminishing Returns:** We've extracted 35.8% of the code. The remaining ~20K lines are increasingly interdependent. Further extraction yields smaller modules with higher effort.

**Practical Ceiling:** Realistically, we can reach ~40-45% reduction (extract networking + window + hardware = ~1,350 more lines). Beyond that requires architectural changes to the accessor layer.

**Alternative Focus:** Instead of more extractions, consider:
- Improving existing module interfaces (reduce extern declarations)
- Adding more accessors to libqb_state.h for cleaner module boundaries
- Documentation and code cleanup within libqb.cpp itself

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

## Extern Declaration Audit

**Total:** 183 extern declarations across 19 module files (down from 194 after migration).

### By Module (sorted by count)

| Module | Count | Notes |
|--------|-------|-------|
| text.cpp | 52 | Largest - many font/image/lprint dependencies (migrated 1) |
| graphics.cpp | 24 | Image system, blend tables |
| libqb_state.cpp | 21 | Expected - implements accessors |
| mouse.cpp | 14 | Console state (migrated 2 to accessors) |
| screen.cpp | 16 | Image system, display state |
| fileio.cpp | 15 | GFS system, error handling |
| console.cpp | 8 | Console state variables |
| color.cpp | 4 | Migrated 3 to accessors |
| port_io.cpp | 6 | Retrace flags, keyboard buffer |
| keyboard.cpp | 5 | Keyhit buffer, keyheld function, codepage array |
| Others | 18 | Various smaller dependencies |

### Categories of Extern Usage

1. **Function declarations** (~60%) - Forward declarations for functions in other modules. These are acceptable and follow C++ patterns.

2. **Global state with accessors** (~25%) - Variables like `write_page`, `write_page_index` that have accessors in `libqb_state.h` but modules still use extern. Could be migrated.

3. **Global state without accessors** (~15%) - Variables like `lpos`, `autodisplay`, `lock_display` that would need new accessors.

### Completed Migrations

The following modules have been migrated to use accessor functions:

| Module | Migrated Externs | Notes |
|--------|------------------|-------|
| **color.cpp** | `write_page`, `write_page_index`, `read_page_index` | Fully migrated to accessors |
| **mouse.cpp** | `display_page`, `read_page` | Fully migrated to accessors |
| **text.cpp** | `write_page_index` | Partial - `write_page` kept as extern (250+ usages) |

### Remaining Migration Opportunities

| Current Extern | Accessor Available | Used In |
|---------------|-------------------|---------|
| `extern img_struct *write_page` | `libqb_get_write_page()` | text.cpp (250+ uses), graphics.cpp |
| `extern img_struct *read_page` | `libqb_get_read_page()` | graphics.cpp |
| `extern img_struct *display_page` | `libqb_get_display_page()` | graphics.cpp |
| Page index externs | `libqb_get_*_page_index()` | graphics.cpp |

**Note:** graphics.cpp migration deferred - it has 24 externs and is performance-critical code.

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
| `libqb/src/text.cpp` | Text output, cursor control, font management (fully implemented) |
| `libqb/include/text.h` | Text/font function declarations (~25 functions) |
| `libqb/src/port_io.cpp` | Port I/O module - VGA palette emulation, keyboard scancode, retrace timing |
| `libqb/include/port_io.h` | Port I/O function declarations (INP, OUT, WAIT) |
| `libqb/src/platform.cpp` | Platform module - keyboard input simulation (_SCREENPRINT) |
| `libqb/include/platform.h` | Platform function declarations |

### Build System

New modules use this pattern in `libqb/build.mk`:
```makefile
libqb-objs-y += $(PATH_LIBQB)/src/module.o
```

---

## Recent Changes

### January 2026

- **Platform module extracted** - Created new platform.cpp with `sub__screenprint` (870 lines) for keyboard input simulation (Windows SendInput, macOS CGEvent)
- **Accessor migration completed** - Migrated color.cpp (3 externs), mouse.cpp (2 externs), text.cpp (1 extern) to use `libqb_state.h` accessor functions instead of extern declarations
- **Keyboard input functions extracted** - Added `func__keyhit`, `func__keydown`, `sub__mapunicode`, `func__mapunicode` to keyboard.cpp (now 177 lines)
- **Port I/O module completed** - Full implementation of `sub_out`, `func_inp`, `sub_wait` with VGA palette emulation and keyboard scancode support (249 lines)
- **Dead code removal** - Removed 118 lines of unused MACVK_* global constants (were shadowed by local statics)
- **Module include fixes** - Added `libqb-common.h` to console.cpp, keyboard.cpp, text.cpp for proper platform macro definitions
- **Extern declaration audit** - Documented 183 extern declarations across 19 modules (down from 194 after migration)
- **libqb.cpp reduced to 19,982 lines** (35.8% reduction from original 31,111)
