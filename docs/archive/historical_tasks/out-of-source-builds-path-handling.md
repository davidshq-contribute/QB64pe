# Out-of-Source Builds: Path Handling Challenges

## Problem

When implementing out-of-source builds, we encountered several path handling challenges due to the mixed use of forward slashes (Unix-style) and backslashes (Windows-style) in the Makefile.

## Issues Identified

### 1. Windows ICON_OBJ Path Mixing

**Location**: `Makefile` line 96

**Problem**: On Windows, `ICON_OBJ` is defined as:
```makefile
ICON_OBJ := $(BUILD_OBJ_DIR)\icon.o
```

But `BUILD_OBJ_DIR` is defined with forward slashes (`$(BUILD_DIR)/obj`), creating a mixed path like `build-win/obj\icon.o` which is invalid.

**Solution**: Use the `BUILD_OBJ` helper function which normalizes paths:
```makefile
ICON_OBJ := $(call BUILD_OBJ,$(PATH_INTERNAL_TEMP)/icon.rc)
```

### 2. Pattern Rule Path Matching on Windows

**Location**: Pattern rules throughout build.mk files

**Problem**: Pattern rules like `$(BUILD_OBJ_DIR)/%.o: %.cpp` use forward slashes, but on Windows, source paths may use backslashes (e.g., `internal\c\libqb\src\file.cpp`). Make's pattern matching may not work correctly with mixed separators.

**Solution**: The `BUILD_OBJ` helper function normalizes all paths to use forward slashes before pattern matching. Pattern rules should use the normalized paths from `BUILD_OBJ` rather than trying to match raw source paths.

### 3. Windows PATH_INTERNAL_TEMP Backslash Usage

**Location**: `Makefile` line 78, 421

**Problem**: On Windows, `PATH_INTERNAL_TEMP` uses backslashes (`internal\temp$(TEMP_ID)`), but when calling `BUILD_OBJ`, we sometimes use forward slashes in the path.

**Solution**: Always use the path variables as-is, and let `BUILD_OBJ` normalize them. The helper function handles both forward and backslashes correctly.

### 4. Test Build Source-to-Object Conversion

**Location**: `tests/build.mk`

**Problem**: The original test build system linked source files directly (compile-and-link in one step). Converting to out-of-source builds requires compiling sources to objects first, but libqb sources should use objects from `libqb/build.mk`, not compile them again.

**Solution**: The `test_src_to_obj` helper converts all source files to object file paths. The pattern rules in `libqb/build.mk` will handle compilation of libqb sources, and the test pattern rule handles test sources. Make's dependency system ensures objects are built before linking.

## Key Learnings

1. **Always use helper functions**: The `BUILD_OBJ` and `BUILD_LIB` helper functions handle path normalization automatically. Use them consistently.

2. **Pattern rules need normalized paths**: Make's pattern matching works best with consistent path separators. Normalize paths before pattern matching.

3. **Windows path handling**: On Windows, the Makefile uses backslashes for some paths (like `PATH_INTERNAL_TEMP`) but forward slashes work in most contexts. The `BUILD_OBJ` function normalizes this.

4. **FIXPATH function**: The existing `FIXPATH` function converts forward slashes to backslashes for Windows commands, but build directory paths should use forward slashes for Make's pattern matching.

## Best Practices

1. Always use `$(call BUILD_OBJ,<source-path>)` instead of manually constructing object paths
2. Always use `$(call BUILD_LIB,<name>)` for library paths
3. Let helper functions handle path normalization
4. Use `$(call FIXPATH,<path>)` only when passing paths to shell commands that require Windows-style paths
5. Pattern rules should match normalized paths (forward slashes)
