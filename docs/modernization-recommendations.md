# QB64-PE Modernization Recommendations

## Executive Summary

This document provides ROI-prioritized recommendations for the ongoing QB64-PE modernization effort. After reviewing the entire codebase, documentation, and current progress, the following key findings emerge:

**Current State:**
- libqb.cpp reduced from 31,111 to 19,119 lines (38.5% reduction)
- 47 modules exist in `libqb/src/`, with 15 major functional extractions completed
- State accessor layer (`libqb_state.h`) enables further modularization
- Build system and CI/CD are mature and well-documented
- Test coverage complete: 15/15 modules have unit tests
- Documentation complete: 15/15 modules have API docs in module-interfaces.md

**Key Opportunities:**
1. The remaining ~19K lines in libqb.cpp are increasingly interdependent
2. Quick wins remain in the codebase (deprecated features, dead code)
3. Window module can be expanded (~270 more lines)
4. Hardware texture functions can be extracted (~400 lines)

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

### ~~1.1 Complete Module Test Coverage~~ - DONE

**Status:** COMPLETED - All 15 modules now have unit tests

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

### 1.2 Remove Deprecated Features

**Identified Deprecations:**

| Feature | Location | Lines | Status |
|---------|----------|-------|--------|
| `$NOPREFIX` | qb64pe.bas:1020 | ~50 | Deprecated with warning |
| `$MIDISOUNDFONT` | qb64pe.bas:1900 | ~20 | Replaced by `_MIDISOUNDBANK` |

**Recommendation:** Remove both deprecated features. They emit warnings and add maintenance burden.

**Effort:** Low (2-3 hours)
**Impact:** Low-Medium - reduces codebase complexity
**Risk:** Low - features already emit deprecation warnings

---

## Tier 2: Medium ROI Improvements

### 2.1 Expand Window Module

**Current State:** window.cpp is only 77 lines

**Candidates for Extraction:**
- `sub__screenmove` (~80 lines)
- `sub__icon` (~50 lines)
- `sub__filedrop` / `func__filedrop` (~100 lines)
- `func__desktopwidth` / `func__desktopheight` (~40 lines)

**Total:** ~270 additional lines extractable

**Effort:** Medium (1-2 days)
**Impact:** Medium - cleaner module boundaries
**Risk:** Low - functions are relatively self-contained

### 2.2 Extract Hardware Texture Functions

**Candidates:** (~400 lines)
- `newimg()` - Image creation
- `freeimg()` - Image cleanup
- `imgrevert()` - Image reversion
- `new_hardware_img()` - Hardware texture creation
- Related texture upload functions

**Decision Point:** Create new `hardware.cpp` or merge into existing `graphics.cpp`?

**Recommendation:** Merge into graphics.cpp (already 7,589 lines but logically related)

**Effort:** Medium (2-3 days)
**Impact:** Medium - clearer graphics subsystem
**Risk:** Medium - performance-critical code

### 2.3 Add CI Formatting Check

**Current State:** `.clang-format` exists but no CI enforcement

**Action:**
1. Add `clang-format --dry-run --Werror` step to CI
2. Provide instructions for auto-fixing
3. Consider pre-commit hook setup documentation

**Effort:** Low (1-2 hours for CI, 2-3 hours for pre-commit docs)
**Impact:** Medium - prevents style drift
**Risk:** Low

### 2.4 Reduce Extern Declaration Count

**Current State:** 183 extern declarations across 19 modules

**High-Value Migrations:**
| Module | Current Externs | Accessors Available | Action |
|--------|-----------------|---------------------|--------|
| graphics.cpp | 24 | Many | Defer (performance critical) |
| text.cpp | 52 | Some | Migrate incrementally |
| screen.cpp | 16 | Many | Good candidate |
| fileio.cpp | 15 | Some | Good candidate |

**Recommendation:** Focus on screen.cpp and fileio.cpp first - they have accessors available and are not performance-critical.

**Effort:** Medium (4-8 hours per module)
**Impact:** Medium - cleaner interfaces
**Risk:** Low-Medium

---

## Tier 3: Strategic Investments

### 3.1 Input Buffer Accessors

**Blocker:** Input system modularization is blocked by lack of accessors for:
- `keyhit[]` buffer
- `keyheld[]` array
- `port60h_event[]` buffer
- Mouse message queue structures

**Action:** Add these accessors to `libqb_state.h`:
```cpp
// Proposed additions
int32_t libqb_get_keyhit(int32_t index);
void libqb_set_keyhit(int32_t index, int32_t value);
bool libqb_get_keyheld(int32_t keycode);
// etc.
```

**Effort:** High (1-2 weeks including testing)
**Impact:** High - unblocks input system modularization
**Risk:** Medium - input is complex state machine

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

4. **Remove deprecated features** - 2-3 hours
   - $NOPREFIX, $MIDISOUNDFONT

5. **Add CI formatting check** - 1-2 hours

6. **Update longest-code-files.md** - 1 hour

### Phase 3: Module Improvements (Near-term)

7. **Expand window module** - 1-2 days
   - Add _SCREENMOVE, _ICON, _FILEDROP (~270 lines)

8. **Extract hardware texture functions** - 2-3 days
   - newimg(), freeimg(), imgrevert() (~400 lines)

9. **Migrate screen.cpp, fileio.cpp to use accessors** - 1 week

### Phase 4: Strategic Work (Long-term)

10. **Add input buffer accessors** - 1-2 weeks
    - Unblocks input system modularization

11. **Platform code consolidation** - 1-2 weeks

---

## Metrics to Track

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| libqb.cpp line count | 19,119 | ~19,000 (maintain) | ON TRACK |
| Module test coverage | 15/15 (100%) | 15/15 (100%) | COMPLETE |
| API documentation coverage | 15/15 (100%) | 15/15 (100%) | COMPLETE |
| Extern declarations | 183 | <150 | IN PROGRESS |
| CI test platforms | 3 | 3 (maintain) | COMPLETE |

---

## Documentation Gaps Summary

| Document | Gap | Priority | Status |
|----------|-----|----------|--------|
| module-interfaces.md | ~~Missing 12 modules~~ | ~~High~~ | COMPLETE |
| longest-code-files.md | Needs line count update | Medium | PENDING |
| modernization-roadmap.md | ~~Outdated progress~~ | ~~Medium~~ | COMPLETE |
| low-hanging-fruit.md | Status tracking update | Low | PENDING |
| NEW: Architecture overview | Doesn't exist | Medium | DEFERRED |
| NEW: Contributing guide for libqb | Doesn't exist | Medium | DEFERRED |

---

## Conclusion

The QB64-PE modernization effort has made significant progress (38.5% of libqb.cpp extracted, from 31,111 to 19,119 lines). The remaining code is increasingly interdependent, making further extraction yield diminishing returns.

**Completed Milestones:**
- Test coverage: 15/15 modules (100%)
- API documentation: 15/15 modules (100%)
- State accessor layer fully implemented

**Current Focus Areas:**
1. **Quick wins** - Remove deprecated features, add CI formatting checks
2. **Incremental extraction** - Window module (~270 lines), hardware textures (~400 lines)
3. **Accessor migration** - Move screen.cpp, fileio.cpp to use state accessors

**Avoid:**
- Major architectural changes without clear need
- Extracting tightly-coupled systems (display loop, input state machine)
- Class-based refactoring before accessor layer migration is complete

The foundation is now solid with comprehensive tests and documentation. Future work should focus on incremental improvements and strategic investments based on actual contributor needs.
