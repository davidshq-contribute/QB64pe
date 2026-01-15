# QB64-PE Modernization Recommendations

## Executive Summary

This document provides ROI-prioritized recommendations for the ongoing QB64-PE modernization effort. After reviewing the entire codebase, documentation, and current progress, the following key findings emerge:

**Current State:**
- libqb.cpp reduced from 31,111 to 18,947 lines (39.1% reduction)
- 47 modules exist in `libqb/src/`, with 15 major functional extractions completed
- State accessor layer (`libqb_state.h`) expanded to 401 lines with 30+ accessors
- Build system and CI/CD are mature and well-documented
- Test coverage complete: 15/15 modules have unit tests
- Documentation complete: 15/15 modules have API docs in module-interfaces.md

**Key Opportunities:**
1. The remaining ~19K lines in libqb.cpp are increasingly interdependent
2. Quick wins remain in the codebase (deprecated features, dead code)
3. ✅ Window module expanded (77→347 lines) - DONE
4. Hardware texture functions can be extracted (~766 lines)

---

## ROI Analysis Framework

Each recommendation is scored on:
- **Effort**: Low (hours), Medium (days), High (weeks)
- **Impact**: How much it improves maintainability, comprehension, or velocity
- **Risk**: Likelihood of introducing regressions

**ROI Categories:**
- **Tier 1**: High impact, low effort (do first)
- **Tier 2**: Medium impact, medium effort (do next)
- **Tier 3**: High impact, high effort (plan carefully)
- **Tier 4**: Low impact or high risk (defer or avoid)

---

## Tier 1: High ROI Quick Wins

### ~~1.2 Remove Deprecated Features~~ - NOT RECOMMENDED

**Analysis:** These features are already properly deprecated and should NOT be removed:

| Feature | Location | Current Behavior |
|---------|----------|------------------|
| `$NOPREFIX` | qb64pe.bas:1833 | Error with IDE conversion offer |
| `$MIDISOUNDFONT` | qb64pe.bas:1902 | Error pointing to `_MIDISOUNDBANK` |

**Why keep them:**
- `$NOPREFIX`: IDE offers automatic code conversion via `AddPREFIX.bas` utility
- Both provide clear error messages helping users update old code
- Removing them would make old programs fail with confusing errors

**Status:** Already properly deprecated - no action needed

---

## Tier 2: Medium ROI Improvements

### ~~2.2 Extract Image Functions~~ - ✅ COMPLETED

**Status:** All image functions are now in graphics.cpp:

**Hardware texture functions:**
- `new_hardware_img()`, `free_hardware_img()` - Hardware texture lifecycle
- `NPO2_texture_generate()` - Non-power-of-2 texture handling
- `hardware_img_put()`, `hardware_img_tri2d()`, `hardware_img_tri3d()` - Hardware rendering

**Software image functions (extracted from libqb.cpp):**
- `restorepalette()` - Palette restoration
- `pset()` - Pixel setting with alpha blending
- `newimg()`, `freeimg()` - Image allocation/deallocation
- `imgrevert()`, `imgframe()`, `imgnew()` - Image lifecycle management

**Result:** ~482 lines moved to graphics.cpp, libqb.cpp reduced to 17,563 lines.

### 2.3 Add CI Formatting Check

**Current State:** `.clang-format` exists but no CI enforcement

**Action:**
1. Add `clang-format --dry-run --Werror` step to CI
2. Provide instructions for auto-fixing
3. Consider pre-commit hook setup documentation

**Effort:** Low (1-2 hours for CI, 2-3 hours for pre-commit docs)
**Impact:** Medium - prevents style drift
**Risk:** Low

### ~~2.4 Reduce Extern Declaration Count~~ - PARTIAL

**Current State:** ~168 extern declarations across 19 modules (down from 183)

**Completed Migrations:**
| Module | Status | Notes |
|--------|--------|-------|
| screen.cpp | ✅ DONE | All 17 externs migrated to accessors |
| shell.cpp | ✅ DONE | Fullscreen externs migrated |
| glut-main-thread.cpp | ✅ DONE | screen_hide migrated |
| text.cpp | Partial | autodisplay migrated, write_page kept as extern |

**Remaining High-Value Targets:**
| Module | Current Externs | Accessors Available | Action |
|--------|-----------------|---------------------|--------|
| graphics.cpp | 24 | Many | Defer (performance critical) |
| text.cpp | 51 | Some | Migrate incrementally |
| fileio.cpp | 15 | Some | Good candidate |

**Effort:** Medium (4-8 hours per module)
**Impact:** Medium - cleaner interfaces
**Risk:** Low-Medium

---

## Tier 3: Strategic Investments

### ~~3.1 Input Buffer Accessors~~ - ✅ COMPLETED

**Status:** All input buffer accessors have been added to `libqb_state.h`:

**Keyhit buffer accessors:**
- `libqb_keyhit_pop()` - Pop next key event
- `libqb_keyhit_push()` - Push key event
- `libqb_keyhit_pending()` - Count pending events
- `libqb_keyhit_clear()` - Clear all events

**Keyheld accessor:**
- `libqb_keyheld()` - Check if key is held (wrapper around existing function)

**Port60h buffer accessors:**
- `libqb_port60h_pop()` - Pop scancode
- `libqb_port60h_peek()` - Peek without removing
- `libqb_port60h_push()` - Push scancode
- `libqb_port60h_push_release()` - Push release scancode
- `libqb_port60h_events_count()` - Get event count

**Mouse accessors:**
- `libqb_mouse_get_current()` - Get current mouse state
- `libqb_mouse_input_next()` - Advance to next event
- `libqb_mouse_has_pending()` - Check for pending events
- `libqb_get/set_mouse_hidden()` - Mouse visibility
- `libqb_get/set_mouse_cursor_style()` - Cursor style

**Codepage accessor:**
- `libqb_get/set_codepage_mapping()` - Unicode conversion table

**Modules migrated:** keyboard.cpp, mouse.cpp, port_io.cpp (8 externs removed)

### 3.2 Platform Code Consolidation

**Current State:** Platform-specific code scattered throughout libqb.cpp and modules

**Proposal:** Create platform-specific files:
- `libqb/src/platform_win.cpp`
- `libqb/src/platform_linux.cpp`
- `libqb/src/platform_macos.cpp`

**Benefits:**
- Clearer platform-specific code boundaries
- Easier to find and maintain platform bugs
- Potential for platform-specific optimizations

**Effort:** High (1-2 weeks)
**Impact:** Medium-High
**Risk:** Medium

### 3.3 Compiler Source Organization

**Current State:** `source/qb64pe.bas` is 24,330 lines

**Observation:** The BASIC source is organized into subdirectories but the main file remains monolithic.

**Potential Actions:**
1. Extract more functions to `source/utilities/`
2. Create additional include files for logical sections
3. Add section markers and documentation

**Recommendation:** Document the compiler architecture before attempting to refactor. The compiler works and regressions would be costly.

**Effort:** High
**Impact:** Medium
**Risk:** High - compiler changes have wide blast radius

---

## Tier 4: Defer or Avoid

### 4.1 Further libqb.cpp Extractions Beyond 45%

**Analysis:** The remaining code is deeply interdependent:
- Display/render loop (1,371 lines) - core orchestrator
- Key input infrastructure (1,200 lines) - complex state machine
- Graphics mode management (1,050 lines) - deep integration
- INPUT statement (830 lines) - cross-cuts 4+ systems

**Recommendation:** Accept ~42-45% as the practical ceiling without major architectural changes. Focus instead on:
- Improving existing module interfaces
- Adding accessors for cleaner boundaries
- Documentation and testing

### 4.2 Class-Based Refactoring

**Proposal from modernization-roadmap.md:** Convert to ImageSystem, FontSystem, InputSystem classes

**Assessment:** This would require:
- Extensive testing infrastructure first
- Careful state migration
- Potential performance impact
- Risk of subtle behavior changes

**Recommendation:** Defer until:
1. Module test coverage is complete
2. Accessor layer is comprehensive
3. Clear performance baseline established

---

## Recommended Action Sequence

### ~~Phase 1: Foundation~~ - COMPLETE

1. ~~**Update stale documentation**~~ - DONE
2. ~~**Add tests for all modules**~~ - DONE (15/15)
3. ~~**Document remaining module APIs**~~ - DONE (15/15)

### Phase 2: Quick Wins (Current Priority)

4. ~~**Remove deprecated features**~~ - NOT APPLICABLE
   - Already properly deprecated with helpful error messages and conversion tools

5. **Add CI formatting check** - 1-2 hours

6. **Update longest-code-files.md** - 1 hour

### Phase 3: Module Improvements (Near-term)

7. ~~**Expand window module**~~ - ✅ COMPLETED
   - Added _SCREENMOVE, _FILEDROP, desktop/window functions (77→347 lines)
   - State accessor layer expanded (168→401 lines)

8. ~~**Extract hardware texture functions**~~ - ALREADY DONE
   - Hardware functions already in graphics.cpp
   - Remaining: software image functions (~470 lines) - lower priority

9. ~~**Migrate screen.cpp to use accessors**~~ - ✅ COMPLETED
   - All 17 externs migrated; shell.cpp/glut-main-thread.cpp also migrated

10. ~~**Migrate input modules to use accessors**~~ - ✅ COMPLETED
    - keyboard.cpp: 4 externs removed (keyhit buffer, keyheld, codepage)
    - port_io.cpp: 2 externs removed (port60h buffer)
    - mouse.cpp: 2 externs removed (mouse_hidden, cursor_style)
    - fileio.cpp: Analyzed - not suitable (function dependencies, not state)

### Phase 4: Strategic Work (Long-term)

11. ~~**Add input buffer accessors**~~ - ✅ COMPLETED
    - Added keyhit, keyheld, port60h, mouse, codepage accessors to libqb_state
    - Input modules migrated (see Phase 3 item 10)

12. **Platform code consolidation** - 1-2 weeks

---

## Metrics to Track

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| libqb.cpp line count | 17,563 | ~17,500 | ✅ TARGET MET |
| Module test coverage | 15/15 (100%) | 15/15 (100%) | COMPLETE |
| API documentation coverage | 15/15 (100%) | 15/15 (100%) | COMPLETE |
| Extern declarations | ~160 | <150 | IN PROGRESS (8 removed via input module migration) |
| State accessor functions | 30+ | Maintain | EXPANDED |
| CI test platforms | 3 | 3 (maintain) | COMPLETE |

---

## Documentation Gaps Summary

| Document | Gap | Priority | Status |
|----------|-----|----------|--------|
| module-interfaces.md | ~~Missing 12 modules~~ | ~~High~~ | COMPLETE |
| longest-code-files.md | ~~Needs line count update~~ | ~~Medium~~ | COMPLETE |
| modernization-roadmap.md | ~~Outdated progress~~ | ~~Medium~~ | COMPLETE |
| libqb-modularization-plan.md | ~~Needs status update~~ | ~~Medium~~ | COMPLETE |
| low-hanging-fruit.md | Status tracking update | Low | PENDING |
| NEW: Architecture overview | Doesn't exist | Medium | DEFERRED |
| NEW: Contributing guide for libqb | Doesn't exist | Medium | DEFERRED |

---

## Conclusion

The QB64-PE modernization effort has made significant progress (43.5% of libqb.cpp extracted, from 31,111 to 17,563 lines). The remaining code is increasingly interdependent, making further extraction yield diminishing returns.

**Completed Milestones:**
- Test coverage: 15/15 modules (100%)
- API documentation: 15/15 modules (100%)
- State accessor layer expanded (30+ accessors, 401 lines)
- Window module expanded (77→347 lines)
- Screen.cpp fully migrated to accessors (17 externs removed)
- Input modules migrated to accessors (keyboard.cpp, mouse.cpp, port_io.cpp - 8 externs removed)
- Hardware texture functions already in graphics.cpp
- Software image functions extracted to graphics.cpp (~482 lines)

**Current Focus Areas:**
1. **Quick wins** - Add CI formatting checks
2. **Deprecated features** - Already properly deprecated with helpful error messages (no action needed)

**Avoid:**
- Major architectural changes without clear need
- Extracting tightly-coupled systems (display loop, input state machine)
- Class-based refactoring before accessor layer migration is complete

The foundation is now solid with comprehensive tests and documentation. Future work should focus on incremental improvements and strategic investments based on actual contributor needs.
