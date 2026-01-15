# QB64 BASIC Source Code Modernization Plan

## Goals
- **Maintainability**: Make the codebase easier to understand, navigate, and modify
- **Testability**: Enable unit testing of compiler components
- **Risk Tolerance**: Moderate (larger refactors acceptable if well-tested)

## Current State
- ~59,000 lines of BASIC code across 28 files
- Two monolithic files: `qb64pe.bas` (24,330 lines), `ide_methods.bas` (20,864 lines)
- 200+ global `DIM SHARED` variables scattered across files
- Uses `$INCLUDE` system for modularity (.bas, .bi, .bm files)

---

## Implementation Progress

### Completed (Phase 1)
- [x] Test harness framework (`tests/unit/test_harness.bi`, `tests/unit/test_harness.bas`)
- [x] State container types (`source/compiler/compiler_state.bi`)
- [x] State accessor functions (`source/compiler/compiler_state.bas`)
- [x] Parser module with pure utilities (`source/compiler/parser.bi`, `source/compiler/parser.bas`)
- [x] Unit test for parser (`tests/unit/compiler/test_parser.bas`)
- [x] Test runner scripts (`tests/run_unit_tests.sh`, `tests/run_unit_tests.bat`)

### In Progress
- [ ] Integration of new modules into main compiler
- [ ] Expression system extraction

### Pending
- [ ] IDE module extraction
- [ ] State consolidation
- [ ] Full migration from scattered globals

---

## Phase 1: Foundation Setup

### 1.1 Create Test Harness [COMPLETED]
Unit testing framework for BASIC code.

**Files created:**
- `tests/unit/test_harness.bi` - Test assertion declarations
- `tests/unit/test_harness.bas` - Test runner implementation

```basic
' Test harness pattern
TYPE TestResult
    name AS STRING
    passed AS _BYTE
    message AS STRING
END TYPE

SUB AssertEquals(expected$, actual$, testName$)
SUB AssertTrue(condition%%, testName$)
SUB RunTests
```

### 1.2 Create State Container Types
Define TYPE structures to consolidate related globals.

**File:** `source/compiler/compiler_state.bi`
```basic
TYPE CompilerState
    sourcefile AS STRING
    linenumber AS LONG
    prepass AS _BYTE
    subfuncn AS LONG
    controllevel AS INTEGER
END TYPE

TYPE TypeSystemState
    lasttype AS LONG
    lasttypeelement AS LONG
    definingtype AS LONG
END TYPE
```

### 1.3 Create Accessor Functions
Following the C++ libqb_state.cpp pattern.

**File:** `source/compiler/compiler_state.bas`
```basic
FUNCTION GetCurrentLineNumber&()
SUB SetCurrentLineNumber(n AS LONG)
FUNCTION IsPrepass%%()
FUNCTION GetCurrentScope$()
```

---

## Phase 2: Extract Pure Functions

Start with functions that have minimal global dependencies.

### 2.1 Create Parser Module
Extract from `qb64pe.bas` lines ~20000+

**File:** `source/compiler/parser.bas` (~1,500 lines)

Functions to extract:
- `lineformat$` - Line tokenization
- `getelement$`, `getelements$` - Element extraction
- `numelements`, `countelements` - Element counting
- `validname` - Name validation
- `uniquenumber&` - Unique ID generation
- `str_nth$` - String utilities

### 2.2 Add Unit Tests
**File:** `tests/unit/compiler/test_parser.bas`

---

## Phase 3: Extract Expression System

Higher complexity, requires comprehensive testing.

### 3.1 Create Expressions Module
**File:** `source/compiler/expressions.bas` (~3,500 lines)

Functions to extract:
- `fixoperationorder$` - Operator precedence
- `evaluate$` - Expression evaluation (~800 lines)
- `evaluatefunc$` - Function evaluation (~1,700 lines)
- `evaluatetotyp$` - Type coercion
- `operatorusage` - Operator validation

### 3.2 Add Integration Tests
Test compilation of programs with complex expressions.

---

## Phase 4: Extract Type System

### 4.1 Create Types Module
**File:** `source/compiler/types.bas` (~1,500 lines)

Functions to extract:
- `typname2typ` - Type name parsing
- `typ2ctyp$` - C type conversion
- UDT handling functions
- Type flag operations (ISSTRING, ISFLOAT, etc.)

---

## Phase 5: IDE Module Extraction

Lower risk - IDE failures don't affect compilation.

### 5.1 Extract Dialog Functions
**File:** `source/ide/ide_dialogs.bas` (~3,500 lines)

Functions to extract:
- `idemessagebox`
- `ideinputbox$`
- `idefiledialog$`
- `ideDisplayBox`
- `idechoosecolorsbox`
- `ideLayoutBox`
- `ideCompilerSettingsBox`

### 5.2 Extract Search Functions
**File:** `source/ide/ide_search.bas` (~1,200 lines)

Functions to extract:
- `idefind$`
- `idechange$`
- `idefindagain`
- `multiSearch`

### 5.3 Extract Display Functions
**File:** `source/ide/ide_display.bas` (~2,000 lines)

Functions to extract:
- `ideshowtext`
- `idebox`, `ideboxshadow`
- `idehbar`, `idevbar`
- `idedrawobj`, `idedrawpar`

### 5.4 Extract Editor Functions
**File:** `source/ide/ide_editor.bas` (~2,000 lines)

Functions to extract:
- `idegetline$`, `idesetline`
- `ideinsline`, `idedelline`
- `insertAtCursor`
- `delselect`
- Selection handling

### 5.5 Refactor ide2() Main Loop
Break the 6,800+ line function into smaller pieces using dispatcher pattern.

---

## Phase 6: State Consolidation

### 6.1 Migrate to State Containers
Replace direct global access with accessor function calls.

### 6.2 Remove Deprecated Globals
Once all references use accessors, migrate data to new containers.

---

## Proposed Directory Structure

```
source/
  qb64pe.bas                    # Main entry (~500 lines after refactor)

  compiler/                     # NEW
    compiler_state.bi           # State TYPE definitions
    compiler_state.bas          # Accessor functions
    parser.bas                  # Tokenization
    expressions.bas             # Expression evaluation
    types.bas                   # Type system
    codegen.bas                 # C++ generation
    control_flow.bas            # IF/FOR/DO/SELECT
    declarations.bas            # SUB/FUNCTION
    metacommands.bas            # $INCLUDE, $DEFINE

  ide/                          # Reorganized
    ide_global.bas              # Keep
    ide_main.bas                # Core loop
    ide_editor.bas              # Text editing
    ide_dialogs.bas             # Dialog boxes
    ide_search.bas              # Find/Replace
    ide_display.bas             # Rendering
    ide_file.bas                # File operations

  utilities/                    # Keep existing
  global/                       # Keep existing
  emit/                         # Keep existing
  subs_functions/               # Keep existing

tests/
  unit/
    test_harness.bi
    test_harness.bas
    compiler/
      test_parser.bas
      test_expressions.bas
      test_types.bas
    ide/
      test_editor.bas
      test_search.bas
```

---

## Critical Files

| File | Action | Lines |
|------|--------|-------|
| `source/qb64pe.bas` | Primary extraction target | 24,330 |
| `source/ide/ide_methods.bas` | Secondary extraction target | 20,864 |
| `source/utilities/type.bi` | Pattern for state containers | 76 |
| `source/utilities/hash.bi` | Example of clean module interface | 93 |

---

## Success Metrics

| Metric | Current | Target |
|--------|---------|--------|
| qb64pe.bas lines | 24,330 | < 1,500 |
| ide_methods.bas lines | 20,864 | < 2,500 |
| Largest file | 24,330 | < 4,000 |
| Global DIM SHARED count | 200+ | < 100 |
| Unit test coverage | ~0% | > 60% |

---

## Verification

After each extraction phase:
1. Run `./tests/run_tests.sh` - All existing tests pass
2. Run bootstrap compilation - QB64-PE can compile itself
3. Compile test programs - Output matches original compiler
4. Run new unit tests - Extracted modules work correctly

---

## Risk Mitigation

- Use git branches for each phase
- Extract one module at a time
- Validate self-hosting after each extraction
- Keep ability to rollback to monolithic version

---

## Running Unit Tests

### Requirements
- QB64-PE must be built first (requires graphical environment on Linux: use `xvfb-run`)
- Windows: Tests require the graphical subsystem (OpenGL/DX) to be available

### Commands
```bash
# Linux/macOS
./tests/run_unit_tests.sh

# Windows
tests\run_unit_tests.bat
```

### Test Structure
Tests use the test harness pattern:
```basic
'$INCLUDE:'../test_harness.bi'

SUB TestMyFunction
    AssertEquals "expected", actual$, "Test description"
    AssertTrue condition%%, "Test description"
END SUB

InitTestSuite "Module Name Tests"
TestMyFunction
RunTests

'$INCLUDE:'../test_harness.bas'
```

---

## Files Created

| File | Purpose |
|------|---------|
| `source/compiler/compiler_state.bi` | State container TYPE definitions |
| `source/compiler/compiler_state.bas` | State accessor functions |
| `source/compiler/parser.bi` | Parser module declarations |
| `source/compiler/parser.bas` | Parser utility implementations |
| `tests/unit/test_harness.bi` | Test framework declarations |
| `tests/unit/test_harness.bas` | Test runner implementation |
| `tests/unit/compiler/test_parser.bas` | Parser module unit tests |
| `tests/run_unit_tests.sh` | Linux/macOS test runner |
| `tests/run_unit_tests.bat` | Windows test runner |
