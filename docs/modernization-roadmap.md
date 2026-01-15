# QB64 Phoenix Edition Modernization Roadmap

## Current State

The libqb modularization effort has reduced `libqb.cpp` from ~31,111 to ~23,442 lines (24.6% reduction). Eight modules have been extracted successfully. However, further modularization is blocked by deep architectural coupling.

### Blocking Issues
- **Global state coupling**: Image system (`img[]`, `pages[]`), font arrays, environment 2D variables accessed directly across modules
- **Cross-cutting dependencies**: Text/font rendering interleaved with image and display systems
- **Platform-specific code**: Windows API, X11, macOS code mixed with core logic

---

## Phase 1: Enable Further Modularization

### 1.1 Global State Accessor Layer

Create `libqb/include/libqb_state.h` with accessor functions for commonly-accessed globals:

```cpp
// Image system accessors
img_struct* get_write_page();
img_struct* get_display_page();
img_struct* get_image(int32_t handle);

// Font system accessors
int32_t get_font_width(int32_t font_handle);
int32_t get_font_height(int32_t font_handle);

// Display state accessors
int32_t get_display_page_index();
void set_display_page_index(int32_t index);
```

This allows deferred modules (text, input, window) to be extracted without exposing raw global arrays.

### 1.2 Extract Remaining Modules

With accessor layer in place, extract:

| Module | Blocker Resolved By |
|--------|---------------------|
| Text/Font | Image and font accessors |
| Input | Mouse queue + environment 2D accessors |
| Window/UI | Display state accessors |

### 1.3 Consolidate Platform Code

Move platform-specific code to dedicated files:
- `libqb/src/platform_win.cpp`
- `libqb/src/platform_linux.cpp`
- `libqb/src/platform_macos.cpp`

Use `#ifdef` guards or separate compilation units.

---

## Phase 2: Testing Infrastructure

### Current State
- QB64 compiler tests exist (`tests/compile_tests/`)
- C++ unit tests exist (`tests/c/`)
- No integration tests for cross-module interactions

### Improvements

1. **Add libqb module tests** - Unit tests for each extracted module
2. **Add regression tests** - Capture current behavior before refactoring
3. **CI test coverage** - Ensure all modules are tested on all platforms

Test structure:
```
tests/c/
├── test_graphics.cpp
├── test_fileio.cpp
├── test_screen.cpp
├── test_color.cpp
└── ...
```

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

1. **Create `libqb_state.h`** with accessors for `write_page`, `display_page`, font arrays
2. **Extract Text module** using new accessors
3. **Add unit tests** for extracted modules
4. **Extract Input module** once text is done
5. **Consolidate platform code** into separate files

---

## Success Criteria

- All QB64 test programs compile and run correctly
- No performance regression in common operations
- Each module is self-contained with clear interfaces
- New contributors can understand module boundaries
