# Module Consistency Audit

## Overview

This document tracks consistency issues across libqb modules and their fixes.

**Status:** Audit completed January 2026

---

## Header Guard Status

All 55 headers have include guards:
- **15 headers:** Traditional `#ifndef INCLUDE_LIBQB_*_H` guards
- **13 headers:** Modern `#pragma once` directive
- **Remaining headers:** Have guards within file (after comments)

### Minor Inconsistency (1 header)

| Header | Current | Should Be |
|--------|---------|-----------|
| keyhandler.h | `INCLUDE_LIBQB_KEYHANDLER_h` | `INCLUDE_LIBQB_KEYHANDLER_H` |

---

## Source File Include Order Issues

### Standard Pattern
```cpp
#include "libqb-common.h"   // Platform macros first

#include "module.h"          // Own header second
#include "dependency.h"      // Other libqb headers

#include "../../os.h"        // Type definitions (if using int32 types)

#include <stdint.h>          // Standard headers
#include <string.h>

#ifdef QB64_WINDOWS          // Platform-specific (guarded)
#include <windows.h>
#endif
```

### Files Fixed (January 2026)

| File | Change |
|------|--------|
| color.cpp | Added `libqb-common.h` first, reordered includes |
| fileio.cpp | Added `libqb-common.h` first, reordered includes |
| libqb_state.cpp | Added `libqb-common.h` first |
| mouse.cpp | Added `libqb-common.h` first, reordered includes |
| screen.cpp | Added `libqb-common.h` first, reordered includes |
| graphics.cpp | Moved `libqb-common.h` to first position |
| mem_legacy.cpp | Added `libqb-common.h` first, added `<stdint.h>` |
| utility.cpp | Added `libqb-common.h` first, added `<stdint.h>` |

### Files Still Using Legacy Pattern (Low Priority)

| File | Notes |
|------|-------|
| http.cpp | Uses curl, has own include structure |
| http-stub.cpp | Stub implementation |

### Platform Includes Status

All platform-specific includes (`<windows.h>`) are properly guarded with `#ifdef QB64_WINDOWS`.

---

## Type Consistency Issues

### Recommended Types

| Use | Instead Of |
|-----|------------|
| `int32_t` | `int32` |
| `uint32_t` | `uint32` |
| `int64_t` | `int64` |
| `uint8_t` | `uint8` |

### Files Using Legacy Types

| File | Legacy Types Used |
|------|-------------------|
| libqb_state.cpp | `int32`, `uint32` (via os.h) |

---

## Fix Status

### Completed (January 2026)

- [x] Platform includes already properly guarded
- [x] Added `libqb-common.h` to 8 source files
- [x] Standardized include order in 8 source files
- [x] All headers have include guards (verified)

### Remaining (Low Priority)

- [ ] Fix keyhandler.h guard naming (`_h` -> `_H`)
- [ ] Migrate legacy `int32` types to `int32_t` (160+ usages across 5 files)
- [ ] Standardize http.cpp/http-stub.cpp includes
