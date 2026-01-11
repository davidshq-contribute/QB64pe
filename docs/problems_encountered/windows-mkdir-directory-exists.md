# Windows MKDIR Directory Already Exists Error

## Problem

When implementing out-of-source builds, we encountered an issue where `mkdir` on Windows fails if the directory already exists, causing build failures with errors like:

```
A subdirectory or file build-win\obj\internal\c\parts\gui\ already exists.
mingw32-make: *** [internal\c/parts/gui/build.mk:17: build-win/obj/internal/c/parts/gui/gui.o] Error 1
```

## Root Cause

On Windows, the `mkdir` command (without flags) fails if the target directory already exists. This is different from Unix systems where `mkdir -p` succeeds even if the directory exists.

In our build system, when multiple source files in the same directory are compiled, Make may try to create the same directory multiple times in parallel, causing the second attempt to fail.

## Solution

Created a `MKDIR_SAFE` helper function that handles Windows directory creation safely:

```makefile
# Helper function to create directory (Windows-safe)
# Usage: $(call MKDIR_SAFE,path)
ifeq ($(OS),win)
MKDIR_SAFE = @if not exist $(call FIXPATH,$1) mkdir $(call FIXPATH,$1)
else
MKDIR_SAFE = @$(MKDIR) $(call FIXPATH,$1)
endif
```

This uses Windows batch syntax `if not exist ... mkdir` which only creates the directory if it doesn't exist, preventing the error.

## Implementation

Updated all pattern rules and library creation rules to use `MKDIR_SAFE` instead of `@$(MKDIR)`:

**Before:**
```makefile
$(BUILD_OBJ_DIR)/internal/c/parts/gui/%.o: $(PATH_INTERNAL_C)/parts/gui/%.cpp | $(BUILD_OBJ_DIR)
	@$(MKDIR) $(call FIXPATH,$(dir $@))
	$(CXX) ... $< -c -o $@
```

**After:**
```makefile
$(BUILD_OBJ_DIR)/internal/c/parts/gui/%.o: $(PATH_INTERNAL_C)/parts/gui/%.cpp | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(CXX) ... $< -c -o $@
```

## Files Updated

- `Makefile` - Added `MKDIR_SAFE` helper and updated pattern rules
- `internal/c/libqb/build.mk` - Updated pattern rules
- `internal/c/parts/*/build.mk` - Updated all component build files (10 files)
- `tests/build.mk` - Updated test build rules

## Testing

Verified the fix works by:
1. Building a test executable: `make OS=win EXE=test_build.exe DEP_CONSOLE_ONLY=y`
2. Confirming build directory structure is created correctly
3. Testing clean target removes build directory

## Key Learnings

1. **Windows vs Unix differences**: Always consider platform differences when implementing cross-platform build systems
2. **Parallel builds**: Directory creation can happen in parallel, so must be idempotent
3. **Helper functions**: Creating platform-specific helper functions (`MKDIR_SAFE`) makes the code cleaner and more maintainable
4. **Pattern rules**: All pattern rules that create directories need to use the safe version

## Best Practices

1. Always use `MKDIR_SAFE` for directory creation in pattern rules
2. Use prerequisites (`| $(BUILD_OBJ_DIR)`) for base directories, but still create subdirectories safely
3. Test on Windows early to catch platform-specific issues
