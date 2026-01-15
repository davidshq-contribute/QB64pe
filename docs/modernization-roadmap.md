# QB64 Phoenix Edition Modernization Roadmap

## Current State

The libqb modularization effort has reduced `libqb.cpp` from ~31,111 to **18,947 lines (39.1% reduction)**. Forty-seven modules have been extracted to `libqb/src/`, with 15 major functional modules and many supporting infrastructure modules. The state accessor layer (`libqb_state.h`) enables controlled access to shared global state.

### Completed Extractions
- Graphics (7,830 lines), Text/Font (2,127 lines), File I/O (1,916 lines)
- Shell (1,540 lines), GFS (1,191 lines), Filesystem (1,044 lines)
- Networking (894 lines), Platform (894 lines), Color (795 lines)
- QBS string handling (725 lines), HTTP (530 lines), Memory (411 lines)
- State Accessor layer (401 lines) - 30+ accessors for display/fullscreen/resize/filedrop
- Window (347 lines) - desktop dimensions, window state, file drop, _SCREENMOVE
- Mouse, Port I/O, Screen, Keyboard, Utility, Console, Legacy Memory
- Total: 27,028+ lines in 47 modules

### Recent Progress
- Screen.cpp fully migrated to use state accessors (17 externs removed)
- Shell.cpp, text.cpp, glut-main-thread.cpp migrated to use accessors
- State accessor layer expanded with display control, fullscreen, resize, and file drop accessors

### Remaining Challenges
- **Global state coupling**: Image system (`img[]`, `pages[]`), font arrays, environment 2D variables still accessed via externs in some modules
- **Complex interdependencies**: Display/render loop, input state machine, graphics mode management are tightly coupled
- **Diminishing returns**: Remaining ~19K lines are increasingly interdependent; practical ceiling is ~40% reduction

---

## Phase 1: Enable Further Modularization

### 1.1 Global State Accessor Layer - COMPLETED

Created `libqb/include/libqb_state.h` with accessor functions for commonly-accessed globals:

```cpp
// Image system accessors
img_struct* libqb_get_write_page();
img_struct* libqb_get_display_page();
img_struct* libqb_get_image(int32_t handle);

// Font system accessors
int32_t libqb_get_font_width(int32_t font_handle);
int32_t libqb_get_font_height(int32_t font_handle);

// Display state accessors
int32_t libqb_get_display_page_index();
void libqb_set_display_page_index(int32_t index);
```

Modules migrated to use accessors: color.cpp, mouse.cpp, text.cpp (partial).

### 1.2 Extract Remaining Modules - PARTIALLY COMPLETED

| Module | Status | Notes |
|--------|--------|-------|
| Text/Font | DONE | 2,121 lines extracted |
| Networking | DONE | 894 lines extracted with stub for non-socket builds |
| Platform | DONE | 870 lines extracted (_SCREENPRINT) |
| Input | BLOCKED | Needs input buffer accessors (keyhit[], keyheld[]) |
| Window/UI | PARTIAL | 77 lines done, ~270 more extractable |

### 1.3 Consolidate Platform Code - PARTIAL

Platform-specific code exists in `platform.cpp` for _SCREENPRINT. Further consolidation possible but lower priority than testing and documentation.

---

## Phase 2: Testing Infrastructure

### Current State
- QB64 compiler tests exist (`tests/compile_tests/`) - 177 test files
- C++ unit tests exist (`tests/c/`) - 18 test files
- **Progress:** 15 of 15 extracted modules have unit tests (100%) - COMPLETE

### Existing Tests
| Module | Test File | Status |
|--------|-----------|--------|
| color.cpp | test_color.cpp | EXISTS |
| fileio.cpp | test_fileio.cpp | EXISTS |
| graphics.cpp | test_graphics.cpp | EXISTS |
| screen.cpp | test_screen.cpp | EXISTS |
| text.cpp | test_text.cpp | EXISTS |
| networking.cpp | test_networking.cpp | EXISTS |
| platform.cpp | test_platform.cpp | EXISTS |
| keyboard.cpp | test_keyboard.cpp | EXISTS |
| mouse.cpp | test_mouse.cpp | EXISTS |
| utility.cpp | test_utility.cpp | EXISTS |
| console.cpp | test_console.cpp | EXISTS |
| window.cpp | test_window.cpp | EXISTS |
| port_io.cpp | test_port_io.cpp | EXISTS |
| mem_legacy.cpp | test_mem_legacy.cpp | EXISTS |
| libqb_state.cpp | test_libqb_state.cpp | EXISTS |

### Completed Test Coverage

1. ~~**Add tests for text.cpp**~~ - DONE (font metrics, control chars, LPOS)
2. ~~**Add tests for networking.cpp**~~ - DONE (initialization, handle conversion, stubs)
3. ~~**Add tests for platform.cpp**~~ - DONE (linkage, edge cases, platform detection)
4. ~~**Add tests for keyboard.cpp**~~ - DONE (lock keys, key codes, unicode mapping)
5. ~~**Add tests for mouse.cpp**~~ - DONE (buttons, wheel, cursor, queue structure)
6. ~~**Add tests for utility.cpp**~~ - DONE (RND, FPS, blink, control chars)
7. ~~**Add tests for console.cpp**~~ - DONE (console window, input, cursor)
8. ~~**Add tests for window.cpp**~~ - DONE (handle, title, focus)
9. ~~**Add tests for port_io.cpp**~~ - DONE (VGA palette, keyboard port, status register)
10. ~~**Add tests for mem_legacy.cpp**~~ - DONE (DEF SEG, PEEK, POKE)
11. ~~**Add tests for libqb_state.cpp**~~ - DONE (all state accessors)
12. **CI test coverage** - All platforms already tested; ensure new tests run everywhere

---

## Phase 3: Architecture Improvements (Long-term)

These are larger changes that require significant refactoring:

### 3.1 Image System Abstraction

Replace direct `img[]` array access with an `ImageSystem` class:
- Managed image lifecycle
- Controlled access to image data
- Cleaner interface for graphics operations

### 3.2 Font System Abstraction

Replace font array access with `FontSystem` class:
- Font resource management
- Cached metrics
- Platform-independent interface

### 3.3 Input System Refactoring

Replace message queue with event-driven architecture:
- Abstract input device layer
- Unified mouse/keyboard/gamepad handling
- Cleaner separation from display code

---

## Practical Next Steps

1. ~~**Create `libqb_state.h`**~~ - DONE
2. ~~**Extract Text module**~~ - DONE (2,121 lines)
3. ~~**Add unit tests for extracted modules**~~ - DONE (15/15, 100%)
4. ~~**Document remaining module APIs**~~ - DONE (15/15 in module-interfaces.md)
5. ~~**Expand Window module**~~ - DONE (347 lines, includes _SCREENMOVE, file drop)
6. ~~**Expand State Accessor layer**~~ - DONE (401 lines, 30+ accessors)
7. ~~**Migrate screen.cpp to accessors**~~ - DONE (17 externs removed)
8. ~~**Migrate shell.cpp, glut-main-thread.cpp**~~ - DONE (fullscreen/screen_hide)
9. **Extract Hardware Texture functions** - ~766 lines, complex OpenGL code (next target)
10. **Add input buffer accessors** - Unblocks Input module extraction

See `docs/modernization-recommendations.md` for detailed ROI analysis.

---

## Success Criteria

- All QB64 test programs compile and run correctly
- No performance regression in common operations
- Each module is self-contained with clear interfaces
- New contributors can understand module boundaries
