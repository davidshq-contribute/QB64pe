# QB64-PE Development Guidelines for Claude

This document captures rules, patterns, and lessons learned from working on the QB64 Phoenix Edition codebase.

---

## 1. C++ Module Extraction Rules

### Before Extracting Code

1. **Read before modifying** - Always read the full file/function before suggesting changes
2. **Create accessors first** - Before extracting functions that use globals, add accessors to `libqb_state.h/cpp`
3. **Check dependencies** - Use grep to find all usages of a function before moving it
4. **Extract in dependency order** - Lower-level utilities before higher-level functions

### Extraction Checklist

```
[ ] Identify all extern declarations the code needs
[ ] Check if accessors exist in libqb_state.h
[ ] Create missing accessors BEFORE extraction
[ ] Extract tightly-coupled functions together
[ ] Update build.mk with new .o file
[ ] Compile and test after EACH significant change
```

### What Extracts Well

- Functions with minimal global state dependencies
- Clear input/output interfaces (well-defined parameters)
- Self-contained logic that doesn't call many internal helpers
- Platform-specific code behind `#ifdef` guards

### What Blocks Extraction (Don't Try These)

- Functions heavily using `write_page`, `display_page`, `img[]`
- Functions accessing `fontwidth[]`, `fontheight[]`, `font[]` arrays
- Functions using `environment_2d__screen_*` scaling variables
- Input functions accessing `mouse_message_queue`
- Cross-cutting helpers like `pset_and_clip`, `lineclip`

### Diminishing Returns

The practical extraction ceiling is ~46-48% of libqb.cpp. The remaining ~17K lines are increasingly interdependent. Don't force extraction of tightly coupled code.

---

## 2. Header File Standards

### Include Guard Format

```cpp
#ifndef INCLUDE_LIBQB_MODULE_NAME_H
#define INCLUDE_LIBQB_MODULE_NAME_H
// ...
#endif
```

### Section Order in Headers

```cpp
// 1. DEPENDENCIES (what this header needs)
// 2. PUBLIC API (function declarations)
// 3. IMPLEMENTATION NOTES (for maintainers)
```

### Include Order in Source Files

```cpp
#include "libqb-common.h"      // 1. Platform macros FIRST
#include "module.h"            // 2. Own module header
#include "other-libqb.h"       // 3. Other libqb headers
#include "../../os.h"          // 4. Type definitions
#include <stdint.h>            // 5. Standard headers
#ifdef QB64_WINDOWS            // 6. Platform-specific (guarded)
#include <windows.h>
#endif
```

### File Naming

- Use lowercase with hyphens: `file-io.h`, NOT `fileio.h`
- Exception: Legacy files may keep original naming for compatibility

---

## 3. Build System Patterns

### Adding New Modules

In `libqb/build.mk`:

```makefile
# Always-compiled module
libqb-objs-y += $(PATH_LIBQB)/src/module.o

# Conditional module (with stub fallback)
libqb-objs-$(DEP_FEATURE) += $(PATH_LIBQB)/src/module.o
libqb-objs-y$(DEP_FEATURE) += $(PATH_LIBQB)/src/module-stub.o
```

### Build Commands

```bash
# Windows
internal\c\c_compiler\bin\mingw32-make.exe OS=win BUILD_QB64=y

# Linux
make OS=lnx BUILD_QB64=y

# macOS
make OS=osx BUILD_QB64=y
```

### Test Builds Often

Compile after each extraction step. Don't batch multiple changes before testing.

---

## 4. Testing Requirements

### C++ Unit Tests (`tests/c/test_*.cpp`)

```cpp
#include <stdio.h>
#include <stdint.h>
#include "test.h"
#include "../../internal/c/os.h"

// Forward declarations for functions being tested
extern "C" {
    int32 func_to_test();
};

TEST(ModuleTests) {
    int32 result = func_to_test();
    ASSERT(result >= 0);
}
```

### BASIC Unit Tests (`tests/unit/*/test_*.bas`)

```basic
'$INCLUDE:'../test_harness.bi'

InitTestSuite "Module Name Tests"
TestMyFunction
RunTests

SUB TestMyFunction
    AssertEquals "expected", actual$, "Description"
    AssertTrue condition%%, "Description"
END SUB

'$INCLUDE:'../test_harness.bas'
```

### Test Documentation

Every test file should include:
- What functions are being tested
- Known limitations (e.g., "requires graphics window")
- How to run the tests
- Expected behavior

### Test Coverage Goal

100% of extracted modules should have unit tests.

---

## 5. State Accessor Pattern

### Using Accessors (Preferred)

```cpp
// Include the state header
#include "libqb_state.h"

// Use accessors instead of extern
img_struct* page = libqb_get_write_page();
int32_t index = libqb_get_write_page_index();
```

### Available Accessor Categories

| Category | Functions |
|----------|-----------|
| Image system | `libqb_get_write_page()`, `libqb_get_read_page()`, `libqb_get_display_page()`, `libqb_get_image()` |
| Page indexes | `libqb_get_*_page_index()`, `libqb_set_*_page_index()` |
| Font system | `libqb_get_font_width()`, `libqb_get_font_height()`, `libqb_get_font_flags()` |
| Display | `libqb_get_screen_width()`, `libqb_get_screen_height()` |
| File drop | `libqb_get_total_dropped_files()`, `libqb_get_dropped_file()` |

### When to Create New Accessors

Create new accessors when:
1. Multiple modules need the same global
2. You're extracting code that uses a global
3. The global is part of a logical "system" (image, font, input, etc.)

---

## 6. Conditional Compilation

### Stub Pattern for Optional Features

Create two files:
- `module.cpp` - Full implementation
- `module-stub.cpp` - Empty stubs returning safe defaults

```cpp
// module-stub.cpp
int32_t func__feature() {
    return 0;  // Safe default when feature disabled
}
```

### Common Feature Flags

| Flag | Feature |
|------|---------|
| `DEP_GL` | OpenGL support |
| `DEP_AUDIO_MINIAUDIO` | Sound support |
| `DEP_SOCKETS` | Networking |
| `DEP_HTTP` | HTTP support |
| `DEP_CONSOLE_ONLY` | Console-only mode |

---

## 7. Platform-Specific Code

### Guard Pattern

```cpp
#ifdef QB64_WINDOWS
    // Windows-specific code
#elif defined(QB64_MACOSX)
    // macOS-specific code
#elif defined(QB64_LINUX)
    // Linux-specific code
#endif
```

### Platform Macros

Always include `libqb-common.h` first to ensure platform macros are defined.

---

## 8. Git and Commits

### Commit Message Style

Follow the repository's existing style:
- Start with verb: "Add", "Fix", "Update", "Remove"
- Be specific about what changed
- Reference modules/files when relevant

### Branch Naming

- Feature branches: `feature-name`
- Bug fixes: `fix-description`
- Modularization: `modular` (current branch)

---

## 9. Documentation Standards

### Code Comments

- Don't add comments unless the logic isn't self-evident
- Don't add docstrings to code you didn't change
- Implementation notes go in header files under "IMPLEMENTATION NOTES" section

### Module Documentation

Each extracted module should be documented in:
1. `modernization-roadmap.md` - Add to completed modules table
2. Header file - Brief description of purpose
3. Test file - Coverage and limitations

---

## 10. Common Mistakes to Avoid

### DON'T

1. Extract code without reading it first
2. Create accessors after extraction (creates broken intermediate state)
3. Batch multiple extractions before compiling
4. Force extraction of tightly-coupled code
5. Add "improvements" beyond what was requested
6. Create new files when editing existing ones would work
7. Use extern declarations when accessors exist

### DO

1. Read the full function before moving it
2. Check all call sites with grep
3. Compile after each significant change
4. Accept diminishing returns - not everything needs to be extracted
5. Document lessons learned in the modularization plan
6. Use the state accessor layer for global access

---

## 11. Complexity Ratings Reference

When assessing extraction difficulty:

| Rating | Description | Action |
|--------|-------------|--------|
| 1 | Pure data, no dependencies | Extract freely |
| 2 | Few globals, minimal coupling | Good candidate |
| 3 | Some global state, manageable | Extract with accessors |
| 4 | Significant state, threading, platform-specific | Consider carefully |
| 5 | Deeply intertwined, GL/GLUT/threading | Don't extract |

---

## 12. Quick Reference: Key Files

| File | Purpose |
|------|---------|
| `internal/c/libqb.cpp` | Monolithic runtime (being modularized) |
| `internal/c/libqb/src/*.cpp` | Extracted modules |
| `internal/c/libqb/include/*.h` | Module headers |
| `internal/c/libqb/include/libqb_state.h` | State accessor declarations |
| `internal/c/libqb/build.mk` | Build configuration |
| `docs/modernization-roadmap.md` | Comprehensive roadmap including modularization progress and complexity analysis |

---

## 13. BASIC Source Modernization

### Current State

- ~59,000 lines across 28 files
- Two monolithic files: `qb64pe.bas` (24K lines), `ide_methods.bas` (21K lines)
- 200+ global `DIM SHARED` variables

### Modernization Pattern

1. Create state container types in `.bi` files
2. Extract related functions into modules
3. Create unit tests with the test harness
4. Integrate back into main compiler

### Test Harness Usage

```basic
'$INCLUDE:'../test_harness.bi'
InitTestSuite "Suite Name"
TestSomething
RunTests

SUB TestSomething
    AssertEquals expected$, actual$, "description"
    AssertTrue condition%%, "description"
END SUB
'$INCLUDE:'../test_harness.bas'
```

---

*Last updated: Based on project state as of modular branch*
