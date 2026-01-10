# Dead Code Analysis Report

This document identifies dead code found throughout the QB64-PE codebase. Dead code includes:
- Commented-out functions and code blocks
- Functions/variables marked for removal (FIXME)
- Unused or deprecated code paths
- Disabled features

## Summary

- **Commented-out functions**: 1 major function
- **FIXME-marked code for removal**: Multiple instances
- **Commented-out code blocks**: Hundreds of lines
- **Deprecated error handling variables**: 7 variables marked for removal

---

## 1. Commented-Out Functions

### SIGSEGV_handler (qbx.cpp)

**Location**: `internal/c/qbx.cpp:1693-1695`

```1693:1695:internal/c/qbx.cpp
// void SIGSEGV_handler(int ignore){
//    error(256);//assume stack overflow? (the most likely cause)
//}
```

**Status**: Completely commented out, but still referenced in a commented signal registration:
- Line 1701: `// signal(SIGSEGV, SIGSEGV_handler);`

**Recommendation**: Remove both the function definition and the commented signal registration if this handler is no longer needed.

---

## 2. Code Marked for Removal (FIXME)

### 2.1 Error Handling Variables (error_handle.h)

**Location**: `internal/c/libqb/include/error_handle.h:10-20`

```10:20:internal/c/libqb/include/error_handle.h
// FIXME: Should be removed in the future, use `is_error_pending()`.
//
// Some spots edit this directly to clear/restore an error, those sites should
// be examined for the best solution.
extern uint32_t new_error;
extern uint32_t error_err;
extern uint32_t error_occurred;
extern uint32_t error_goto_line;
extern qbs *error_handler_history;
extern uint32_t error_handling;
extern uint32_t error_retry;
```

**Status**: These 7 global variables are marked for removal. A replacement function `is_error_pending()` exists (line 22-24).

**Usage**: Found in 6 files:
- `internal/c/qbx.cpp` (12 references)
- `internal/c/parts/video/image/image.cpp` (2 references)
- `internal/c/libqb/src/datetime.cpp` (3 references)
- `internal/c/libqb/include/error_handle.h` (9 references - definitions)
- `internal/c/libqb/src/error_handle.cpp` (23 references)
- `internal/c/libqb/include/event.h` (1 reference)

**Recommendation**: 
1. Audit all usages to replace with `is_error_pending()` or appropriate accessor functions
2. Remove direct access to these variables
3. Remove the extern declarations once all usages are migrated

### 2.2 qbs_tmp_list Variables (qbs.h)

**Location**: `internal/c/libqb/include/qbs.h:92-94`

```92:94:internal/c/libqb/include/qbs.h
// FIXME: Usages of these outside of qbx.c (and qbs_cleanup()) need to be removed.
extern intptr_t *qbs_tmp_list;
extern uint32_t qbs_tmp_list_lasti;
```

**Status**: Marked for removal, should only be used in `qbx.c` and `qbs_cleanup()`.

**Usage**: Found in 5 files:
- `internal/c/qbx.cpp`
- `internal/c/libqb/include/qbs.h` (declaration)
- `internal/c/libqb.cpp`
- `internal/c/libqb/src/qbs.cpp`
- `internal/c/libqb/src/qbs_cmem.cpp`

**Recommendation**: 
1. Verify all usages outside `qbx.cpp` and `qbs_cleanup()` are necessary
2. Refactor to use proper accessor functions if needed
3. Remove extern declarations once cleaned up

### 2.3 Conventional Memory Variables (qbs_cmem.cpp)

**Location**: `internal/c/libqb/src/qbs_cmem.cpp:11`

```11:11:internal/c/libqb/src/qbs_cmem.cpp
// FIXME: conventional memory should be consolidated into libqb source and headers
```

**Status**: Code organization issue - conventional memory handling should be moved to proper libqb location.

**Recommendation**: Consolidate conventional memory code into libqb source and headers.

### 2.4 MAIN_LOOP Declaration (console-only-main-thread.cpp)

**Location**: `internal/c/libqb/src/console-only-main-thread.cpp:14`

```14:15:internal/c/libqb/src/console-only-main-thread.cpp
// FIXME: PUt this definition somewhere else
void MAIN_LOOP(void *);
```

**Status**: Declaration location needs to be moved to a proper header file.

**Recommendation**: Move `MAIN_LOOP` declaration to appropriate header file.

---

## 3. Commented-Out Code Blocks

### 3.1 Extensive Commented Code in libqb.cpp

**Location**: `internal/c/libqb.cpp`

Hundreds of lines of commented-out code throughout the file, including:
- Debug alert statements (lines 582-586, 4039-4040, etc.)
- Error handling code (lines 14095-14098, 20341-20392, etc.)
- Graphics-related code (lines 4461, 11039-11041, etc.)
- Keyboard/mouse handling (lines 25839-25844, 28087-28090, etc.)
- Memory operations (lines 28480, 29651, etc.)

**Examples**:
```cpp
// alert("HARDWARE_GRAPHICS_COMMAND__FREEIMAGE_REQUEST");
// alert(old_hgc->src_img);
```

```cpp
// if (mode==1) img[i].print_mode=3;//fill
// if (mode==2) img[i].print_mode=1;//keep
// if (mode==3) img[i].print_mode=2;//only
```

**Recommendation**: 
1. Review each commented block to determine if it's:
   - Debug code that can be removed
   - Alternative implementation that should be kept
   - Code that needs to be restored
2. Remove debug-only commented code
3. Document why alternative implementations are kept if they're needed for reference

### 3.2 Commented Code in Image/Font Files

**Location**: `internal/c/parts/video/font/font.cpp`

Multiple commented-out debug trace statements:
- `image_log_trace()` calls (lines 178, 203, 213, 224, etc.)
- `IMAGE_DEBUG_CHECK()` calls (lines 189, 560, 832, etc.)

**Recommendation**: Remove debug-only code or wrap in `#ifdef DEBUG` blocks.

### 3.3 Disabled Function in FreeType (ftsdf.c)

**Location**: `internal/c/parts/video/font/freetype/ftsdf.c:2950-2987`

```2950:2956:internal/c/parts/video/font/freetype/ftsdf.c
  /* `sdf_generate' is not used at the moment */
#if 0

  #error "DO NOT USE THIS!"
  #error "The function still outputs 16-bit data, which might cause memory"
  #error "corruption.  If required I will add this later."
```

**Status**: Function intentionally disabled with `#if 0` and error messages preventing compilation.

**Recommendation**: Keep as-is (intentionally disabled for safety reasons).

---

## 4. Unused/Commented Functions in Third-Party Libraries

### 4.1 Simplebuffer System

**Location**: `source/utilities/s-buffer/readme.txt:10-11`

According to the readme:
> "Functions currently not used in qb64pe were commented out in order to avoid unnecessary bloat of the qb64pe executable."

**Status**: Intentionally commented out to reduce executable size.

**Recommendation**: Keep as-is (intentional optimization).

---

## 5. Deprecated Third-Party Code

### 5.1 libcurl Deprecated Functions

**Location**: `internal/c/parts/network/http/curl/include/curl/curl.h:2557-2584`

Multiple `CURLFORM_*` enum values marked as `CURL_DEPRECATED(7.56.0, ...)`.

**Status**: Part of third-party library, not QB64-PE code.

**Recommendation**: Monitor for removal in future libcurl updates.

---

## 6. TODO/FIXME Comments Indicating Incomplete Code

### 6.1 Const Evaluation (const_eval.bas)

**Location**: `source/utilities/const_eval.bas:1104`

```1104:1104:source/utilities/const_eval.bas
        'FIXME: This doesn't account for `x ^ NOT y + 2`, where it evaluates as `x ^ (NOT y) + 2`
```

**Status**: Known bug/limitation, not dead code but incomplete implementation.

**Recommendation**: Fix the operator precedence issue.

---

## 7. Test Code with Disabled Features

### 7.1 QBasic Test Cases

Several test files contain code for features marked as "not ready yet" or "not implemented":
- `tests/qbasic_testcases/pete/simpire/ssimpire.bas`: Features marked as not ready
- `tests/qbasic_testcases/pete/tor/readme.txt`: Disabled vsync code

**Status**: Test code, not production dead code.

**Recommendation**: Keep as-is (test cases may need these for future testing).

---

## Recommendations Summary

### High Priority
1. **Remove commented-out SIGSEGV_handler** if no longer needed
2. **Migrate error handling variables** from direct access to `is_error_pending()` API
3. **Clean up qbs_tmp_list usage** outside of allowed files
4. **Remove debug-only commented code** in `libqb.cpp` and font/image files

### Medium Priority
1. **Consolidate conventional memory code** into proper libqb location
2. **Move MAIN_LOOP declaration** to appropriate header
3. **Review and document** why alternative implementations are kept as comments

### Low Priority
1. **Monitor third-party deprecated code** for future updates
2. **Fix const evaluation operator precedence** bug

---

## Notes

- Some commented code may be kept intentionally for:
  - Reference implementations
  - Debugging purposes
  - Future restoration
- Always verify that commented code is truly unused before removal
- Consider using version control history to understand why code was commented out
- Test thoroughly after removing any code marked for removal

---

*Generated: Dead code analysis of QB64-PE codebase*
