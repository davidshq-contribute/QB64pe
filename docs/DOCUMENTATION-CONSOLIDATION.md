# Documentation Consolidation Plan

This document identifies duplication across QB64pe documentation and provides recommendations for establishing authoritative sources with cross-linking.

## Guiding Principles

1. **Single source of truth** - Each topic should have one authoritative document
2. **Cross-link, don't duplicate** - Other docs should reference the authoritative source with brief context
3. **Separate concerns** - User-facing syntax vs implementation internals
4. **Brief blurbs are OK** - A 2-3 sentence summary before "See X for details" is helpful

---

## 1. Build System (Major Duplication) — DONE

### Current State (as of consolidation)

| File | Status |
|------|--------|
| `docs/build-system.md` | **Single authoritative doc.** Now includes: build flow diagram, platform table, quick build commands, full CI/repository/Makefile/versioning/release content, and key files table. |
| `BUILD-SYSTEM.md` | **Removed.** Content merged into `build-system.md`. |

### Action taken

`BUILD-SYSTEM.md` was merged into `docs/build-system.md` (diagram, platform selection, quick build commands, key files table added at the top). `BUILD-SYSTEM.md` was deleted. README.md and INDEX.md now reference only `build-system.md`.

---

## 2. Error Handling (Major Duplication)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `docs/error-handling.md` | 97 | Compiler error flow: Give_Error, errmes, IDE delivery, recompile triggers |
| `ERROR-HANDLING.md` | 226 | Runtime: ON ERROR GOTO, RESUME, error state variables, examples |
| `ERROR-CODES.md` | 383 | Error code tables + **duplicates ON ERROR/RESUME content** |

### Overlap Analysis

`ERROR-CODES.md` duplicates from `ERROR-HANDLING.md`:
- Lines 111-159: ON ERROR GOTO implementation and generated C++ code
- Lines 162-209: RESUME variants and comparison table
- Lines 211-264: Error state variables and functions

### Recommendation

**Authoritative sources:**
- `ERROR-HANDLING.md` - Runtime error handling (ON ERROR, RESUME, _ERRORMESSAGE$)
- `ERROR-CODES.md` - Error code reference tables only
- `docs/error-handling.md` - Compiler error flow (rename to `docs/compiler-error-flow.md`)

**Action for `ERROR-CODES.md`:**
1. Keep lines 1-110 (error code tables)
2. Remove lines 111-295 (ON ERROR/RESUME/error state sections)
3. Add cross-reference:

```markdown
## Error Handling

For error handling syntax (ON ERROR GOTO, RESUME, _ERRORMESSAGE$),
see [ERROR-HANDLING.md](ERROR-HANDLING.md).

For compiler error flow and IDE error delivery,
see [error-handling.md](error-handling.md).
```

**Action for `docs/error-handling.md`:**
Rename to `docs/compiler-error-flow.md` to clarify its scope (compiler internals vs runtime).

---

## 3. Debugging (Significant Duplication)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `DEBUGGING.md` | 270 | User overview: $DEBUG, breakpoints, stepping, protocol summary |
| `DEBUG-VWATCH.md` | 486 | Implementation: vwatch.bi, TCP protocol details, variable resolution |

### Overlap Analysis

Both documents cover:
- $DEBUG metacommand effects
- Breakpoint implementation
- IDE-Program protocol commands (tables)
- Generated code examples
- Special line numbers

### Recommendation

**Authoritative:** `DEBUG-VWATCH.md` (implementation details)

**Action for `DEBUGGING.md`:**
Keep as user-focused overview. Remove duplicate sections:

1. Remove lines 122-148 (IDE to Program / Program to IDE command tables) - replace with:
   ```markdown
   For the complete protocol specification, see [DEBUG-VWATCH.md](DEBUG-VWATCH.md#tcpip-communication).
   ```

2. Remove lines 162-184 (Generated Code section) - replace with:
   ```markdown
   For generated code details, see [DEBUG-VWATCH.md](DEBUG-VWATCH.md#code-generation).
   ```

3. Keep: Overview, basic usage examples, architecture diagram

---

## 4. DATA Statements (Significant Duplication)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `DATA-STATEMENTS.md` | 237 | Syntax, type-specific functions, examples |
| `DATA-RESTORE.md` | 292 | Implementation: compile-time processing, number parsing, buffers |

### Overlap Analysis

Both documents cover:
- Type-specific READ functions table
- Label_Type structure
- Error codes table
- Generated C code examples
- RESTORE implementation

### Recommendation

**Option A: Merge into single document**

Create `DATA-STATEMENTS.md` with sections:
1. Syntax and Usage (user-facing)
2. Implementation Details (for contributors)

Delete `DATA-RESTORE.md`.

**Option B: Keep separate with clear boundaries**

**Authoritative:**
- `DATA-STATEMENTS.md` - Syntax, usage, examples (user-facing)
- `DATA-RESTORE.md` - Implementation internals (contributor-facing)

**Action for `DATA-STATEMENTS.md`:**
1. Remove lines 42-49 (Type-Specific Functions table) - exists in DATA-RESTORE.md
2. Remove lines 134-168 (Compiler Implementation section)
3. Add: "For implementation details, see [DATA-RESTORE.md](DATA-RESTORE.md)"

---

## 5. Arrays (Moderate Duplication)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `ARRAYS.md` | 368 | User guide: DIM/REDIM syntax, examples, passing arrays |
| `ARRAY-IMPLEMENTATION.md` | 347 | Internals: descriptor format, memory layout, bounds checking |

### Overlap Analysis

Both documents have identical content for:
- Array descriptor structure (layout table)
- Flag bits table
- ERASE behavior
- Error codes table

### Recommendation

**Authoritative:**
- `ARRAYS.md` - User syntax and examples
- `ARRAY-IMPLEMENTATION.md` - Internal representation

**Action for `ARRAYS.md`:**
1. Remove lines 11-37 (Array Descriptor Structure section)
2. Remove lines 29-37 (Flag Bits table)
3. Add near top:
   ```markdown
   For internal array representation and memory layout,
   see [ARRAY-IMPLEMENTATION.md](ARRAY-IMPLEMENTATION.md).
   ```

---

## 6. Audio (Moderate Duplication)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `AUDIO.md` | 405 | Modern miniaudio system + legacy BEEP/SOUND/PLAY |
| `SOUND-LEGACY.md` | 290 | Legacy only: BEEP, SOUND, PLAY, MML syntax |

### Overlap Analysis

`AUDIO.md` lines 194-264 duplicate `SOUND-LEGACY.md`:
- BEEP statement
- SOUND statement parameters
- PLAY command codes

### Recommendation

**Authoritative:**
- `AUDIO.md` - Modern audio (_SNDOPEN, _SNDPLAY, _SNDRAW)
- `SOUND-LEGACY.md` - Legacy sound (BEEP, SOUND, PLAY)

**Action for `AUDIO.md`:**
Replace lines 194-264 (Legacy Sound section) with:

```markdown
## Legacy Sound (BEEP, SOUND, PLAY)

QB64pe provides QBasic-compatible BEEP, SOUND, and PLAY statements
for simple tone generation and music playback.

For complete documentation of legacy sound including MML (Music Macro Language),
waveform types, and PSG architecture, see [SOUND-LEGACY.md](SOUND-LEGACY.md).
```

---

## 7. Compiler Pipeline (Subsumes Parent)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `ARCHITECTURE.md` | 196 | Complete overview including pipeline, bootstrap, auto-include |
| `COMPILER-PIPELINE.md` | 95 | Pipeline only (subset of ARCHITECTURE.md) |

### Overlap Analysis

`COMPILER-PIPELINE.md` content is largely contained within `ARCHITECTURE.md`:
- Compilation flow diagram
- Output buffers table
- Self-hosting description

### Recommendation

**Option A: Delete `COMPILER-PIPELINE.md`**

All content exists in `ARCHITECTURE.md`. Update any cross-references.

**Option B: Expand `COMPILER-PIPELINE.md`**

Make it the deep-dive into compilation phases:
- Detailed prepass documentation
- Main pass line processing
- Symbol table operations during compilation
- Buffer management

Then `ARCHITECTURE.md` references it for details.

**Recommended: Option A** (less maintenance burden)

---

## 8. Input System (Minor Overlap)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `INPUT-DEVICES.md` | 450 | Hardware: keyboard, mouse, controllers |
| `USER-INPUT.md` | 313 | Text: INPUT, LINE INPUT, INPUT$, LOCATE |
| `PRINT-FORMATTING.md` | 494 | Output: PRINT, LOCATE, COLOR, TAB |

### Overlap Analysis

LOCATE statement documented in both:
- `USER-INPUT.md` lines 99-129
- `PRINT-FORMATTING.md` lines 110-138

### Recommendation

**Authoritative:** `PRINT-FORMATTING.md` for LOCATE (cursor positioning relates to output)

**Action for `USER-INPUT.md`:**
Replace LOCATE section (lines 99-129) with:

```markdown
## Cursor Positioning

Use LOCATE to position the cursor before INPUT statements.

For complete LOCATE documentation, see [PRINT-FORMATTING.md](PRINT-FORMATTING.md#locate-statement).

### Quick Reference

```basic
LOCATE row, column          ' Position cursor
LOCATE , , 1                ' Show cursor
row% = CSRLIN               ' Get current row
col% = POS(0)               ' Get current column
```
```

---

## 9. Documentation Indexes (Minor Overlap)

### Current State

| File | Lines | Content |
|------|-------|---------|
| `INDEX.md` | 161 | Master index for all documentation |
| `docs/README.md` | 51 | Index for docs/ folder + root docs |

### Recommendation

**Authoritative:** `INDEX.md` for complete index

**Action for `docs/README.md`:**
Focus only on docs/ folder contents. Remove root-level doc listings (lines 8-26).
Add: "For the complete documentation index, see [INDEX.md](../INDEX.md)"

---

## Implementation Checklist

### High Priority

- [ ] `ERROR-CODES.md`: Remove lines 111-295, add cross-references
- [ ] `DEBUGGING.md`: Remove protocol tables and generated code sections
- [x] `BUILD-SYSTEM.md`: Merged into build-system.md; file removed

### Medium Priority

- [ ] `DATA-STATEMENTS.md`: Remove implementation sections OR merge with DATA-RESTORE.md
- [ ] `ARRAYS.md`: Remove descriptor structure section
- [ ] `AUDIO.md`: Replace legacy section with cross-link

### Low Priority

- [ ] Delete or expand `COMPILER-PIPELINE.md`
- [ ] `USER-INPUT.md`: Consolidate LOCATE to PRINT-FORMATTING.md
- [ ] `docs/README.md`: Reduce scope to docs/ folder only
- [ ] Rename `docs/error-handling.md` to `docs/compiler-error-flow.md`

---

## Cross-Reference Pattern

When removing duplicated content, use this pattern:

```markdown
## [Topic Name]

[1-3 sentence summary of what this covers]

For [detailed documentation | implementation details | complete reference],
see [Document Name](relative/path/to/document.md#optional-anchor).

### Quick Reference (optional)

[Brief table or code block with most common usage]
```

---

## Validation

After implementing changes:

1. Search for broken internal links
2. Verify each topic has exactly one authoritative source
3. Check that cross-references use consistent anchor names
4. Update `INDEX.md` if document purposes changed
