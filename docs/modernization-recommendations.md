# QB64-PE Modernization Recommendations

## Executive Summary

This document provides ROI-prioritized recommendations for the ongoing QB64-PE modernization effort. After reviewing the entire codebase, documentation, and current progress, the following key findings emerge:

**Current State:**
- libqb.cpp reduced from 31,111 to 17,492 lines (43.8% reduction)
- 47 modules exist in `libqb/src/`, with 15 major functional extractions completed
- State accessor layer (`libqb_state.h`) enables further modularization
- Build system and CI/CD are mature and well-documented
- Test infrastructure exists but has gaps

**Key Opportunities:**
1. The remaining ~19K lines in libqb.cpp are increasingly interdependent
2. Testing coverage for extracted modules is incomplete (4 of 15 modules have tests)
3. Documentation coverage is partial (3 of 15 modules have API docs)
4. Quick wins remain in the codebase (deprecated features, dead code)

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

### 1.1 Complete Module Test Coverage

**Current State:** Only 4 modules have C++ unit tests (color, fileio, graphics, screen)

**Missing Tests For:**
| Module | Lines | Complexity | Test Priority |
|--------|-------|------------|---------------|
| text.cpp | 2,121 | High | Critical |
| networking.cpp | 894 | Medium | High |
| platform.cpp | 870 | Medium | High |
| keyboard.cpp | 177 | Low | Medium |
| mouse.cpp | 355 | Low | Medium |
| port_io.cpp | 249 | Low | Medium |
| utility.cpp | 173 | Low | Low |
| console.cpp | 171 | Low | Low |
| window.cpp | 77 | Low | Low |
| mem_legacy.cpp | 61 | Low | Low |
| libqb_state.cpp | 168 | Low | Low |

**Recommendation:** Prioritize tests for text.cpp, networking.cpp, and platform.cpp. These are the largest extracted modules and most likely to have regressions during future work.

**Effort:** Medium (1-2 days per module)
**Impact:** High - enables confident refactoring
**Risk:** None

### 1.4 Remove Deprecated Features

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

### Phase 1: Foundation (Immediate)

1. **Update stale documentation** - 1-2 hours
   - longest-code-files.md
   - modernization-roadmap.md
   - low-hanging-fruit.md

2. **Add tests for text.cpp** - 1-2 days
   - Largest untested module
   - High complexity, high regression risk

3. **Document remaining module APIs** - 1 week
   - Focus on text.h, networking.h, graphics.h first

### Phase 2: Quick Wins (Near-term)

4. **Remove deprecated features** - 2-3 hours
   - $NOPREFIX, $MIDISOUNDFONT

5. **Add CI formatting check** - 1-2 hours

6. **Add tests for networking.cpp, platform.cpp** - 2-3 days

### Phase 3: Module Improvements (Medium-term)

7. **Expand window module** - 1-2 days

8. **Extract hardware texture functions** - 2-3 days

9. **Migrate screen.cpp, fileio.cpp to use accessors** - 1 week

### Phase 4: Strategic Work (Long-term)

10. **Add input buffer accessors** - 1-2 weeks

11. **Platform code consolidation** - 1-2 weeks

---

## Metrics to Track

| Metric | Current | Target |
|--------|---------|--------|
| libqb.cpp line count | 17,492 | ~17,500 (maintain current level) |
| Module test coverage | 4/15 (27%) | 15/15 (100%) |
| API documentation coverage | 3/15 (20%) | 15/15 (100%) |
| Extern declarations | 183 | <150 |
| CI test platforms | 3 | 3 (maintain) |

---

## Documentation Gaps Summary

| Document | Gap | Priority |
|----------|-----|----------|
| module-interfaces.md | Missing 12 modules | High |
| longest-code-files.md | Outdated numbers | Medium |
| modernization-roadmap.md | Outdated progress | Medium |
| low-hanging-fruit.md | No status tracking | Low |
| NEW: Architecture overview | Doesn't exist | Medium |
| NEW: Contributing guide for libqb | Doesn't exist | Medium |

---

## Conclusion

The QB64-PE modernization effort has made significant progress (43.8% of libqb.cpp extracted). The remaining code is increasingly interdependent, making further extraction yield diminishing returns.

**Focus Areas for Best ROI:**
1. **Testing** - Add tests for extracted modules to enable confident future changes
2. **Documentation** - Complete API docs to reduce contributor onboarding time
3. **Quick wins** - Remove deprecated features, add CI checks
4. **Incremental extraction** - Window module, hardware textures (~700 more lines)

**Avoid:**
- Major architectural changes without comprehensive test coverage
- Extracting tightly-coupled systems (display loop, input state machine)
- Class-based refactoring before accessor layer is complete

The path forward is pragmatic: shore up the foundation with tests and documentation, capture remaining quick wins, then evaluate whether strategic investments like input buffer accessors are worth the effort based on actual contributor needs.
