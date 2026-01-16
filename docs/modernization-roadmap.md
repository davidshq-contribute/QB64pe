# QB64 Phoenix Edition Modernization Roadmap

## Current State

The libqb modularization effort has reduced `libqb.cpp` from ~31,111 to **14,274 lines (54.1% reduction)**. The state accessor layer (`libqb_state.h`) enables controlled access to shared global state.

### Completed Milestones
- **54.1% code reduction** in libqb.cpp (16,837 lines extracted)
- **Test coverage**: 15/15 modules (100%)
- **API documentation**: 15/15 modules (100%)
- **State accessor layer**: 30+ accessors in libqb_state.h
- **Hardware textures**: All functions consolidated in graphics.cpp
- **Print formatting**: Extracted to print_using.cpp
- **Static data**: Icons, palettes, charset data in separate modules

### Remaining Challenges
- **Global state coupling**: Image system (`img[]`, `pages[]`), font arrays still accessed via externs
- **Complex interdependencies**: Display/render loop, input state machine, graphics mode management are tightly coupled
- **Practical ceiling reached**: Remaining ~14K lines are core systems requiring architectural changes to extract

---

## ROI Analysis Framework

**Effort Levels:** Low (hours), Medium (days), High (weeks)
**Impact:** How much it improves maintainability, comprehension, or velocity
**Risk:** Likelihood of introducing regressions

**Tiers:**
- **Tier 1**: High impact, low effort (do first)
- **Tier 2**: Medium impact, medium effort (do next)
- **Tier 3**: High impact, high effort (plan carefully)
- **Tier 4**: Low impact or high risk (defer or avoid)

---

## Quick Wins (Tier 1)

These tasks require minimal context and provide immediate value.

| Task | Priority | Effort | Status |
|------|----------|--------|--------|
| Standardize comment style | Medium | 2-4 hours | NOT STARTED |
| Fix inconsistent variable naming | Medium | 1-2 hours | NOT STARTED |
| Extract magic numbers to constants | Low | 30 min - 1 hour | NOT STARTED |
| Simplify complex conditionals | Medium | 30 min - 1 hour | NOT STARTED |
| Organize utility functions | Low | 1-2 hours | NOT STARTED |

### Comment Style Guidelines
- BASIC: `' Note:` (capitalized)
- C++: Doxygen-style `/** */` for function docs, `//` for inline

### Variable Naming
- C++: `snake_case` for functions/variables, `PascalCase` for classes
- BASIC: Follow QB64 conventions per CONTRIBUTING.md

---

## Medium ROI Improvements (Tier 2)

### Reduce Extern Declaration Count

**Current State:** ~160 extern declarations across 19 modules

| Module | Current Externs | Action |
|--------|-----------------|--------|
| text.cpp | 51 | Migrate incrementally to accessors |
| graphics.cpp | 24 | Defer (performance critical) |
| fileio.cpp | 15 | Good candidate for migration |

**Effort:** Medium (4-8 hours per module)
**Risk:** Low-Medium

---

## Strategic Investments (Tier 3)

### Platform Code Consolidation

**Current State:** Platform-specific code scattered throughout libqb.cpp and modules

**Proposal:** Create platform-specific files:
- `libqb/src/platform_win.cpp`
- `libqb/src/platform_linux.cpp`
- `libqb/src/platform_macos.cpp`

**Benefits:** Clearer boundaries, easier maintenance, platform-specific optimizations
**Effort:** High (1-2 weeks) | **Risk:** Medium

### BASIC Compiler Source Organization

**Current State:** `source/qb64pe.bas` is 24,330 lines

See `docs/basic-modernization-plan.md` for the detailed extraction plan.

**Effort:** High | **Risk:** High - compiler changes have wide blast radius

---

## Defer or Avoid (Tier 4)

### Further libqb.cpp Extractions

The remaining code is deeply interdependent:
- Display/render loop (1,371 lines) - core orchestrator
- Key input infrastructure (1,200 lines) - complex state machine
- Graphics mode management (1,050 lines) - deep integration
- INPUT statement (830 lines) - cross-cuts 4+ systems

**Recommendation:** Accept 54% as the practical ceiling. Focus on improving existing module interfaces and documentation.

### Class-Based Refactoring

Converting to ImageSystem, FontSystem, InputSystem classes would require:
- Extensive testing infrastructure
- Careful state migration
- Potential performance impact

**Recommendation:** Defer until accessor layer migration is complete and clear need emerges.

---

## Long-term Architecture Vision

These are larger changes for future consideration:

### Image System Abstraction
Replace direct `img[]` array access with managed `ImageSystem` class.

### Font System Abstraction
Replace font array access with `FontSystem` class for resource management.

### Input System Refactoring
Replace message queue with event-driven architecture for unified input handling.

---

## Metrics

| Metric | Current | Target | Status |
|--------|---------|--------|--------|
| libqb.cpp reduction | 54.1% | ~55% | NEAR CEILING |
| Module test coverage | 15/15 | 15/15 | COMPLETE |
| API documentation | 15/15 | 15/15 | COMPLETE |
| Extern declarations | ~160 | <150 | IN PROGRESS |
| State accessor functions | 30+ | Maintain | COMPLETE |

---

## Related Documentation

- `libqb-modularization-plan.md` - Detailed module extraction status and patterns
- `basic-modernization-plan.md` - BASIC source code refactoring plan
- `libqb-extraction-complexity.md` - Function-level complexity ratings

---

*Last Updated: January 2026*
