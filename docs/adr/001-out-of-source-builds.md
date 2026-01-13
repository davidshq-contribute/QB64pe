# ADR-001: Out-of-Source Builds Implementation

## Status
Accepted

## Context
QB64pe previously built all C/C++ artifacts in the source tree, mixing generated code with source code and making cleanup difficult. This created several problems:
- Generated object files (.o) and libraries (.a) polluted the source directory
- Clean builds required manual cleanup of generated files
- Multiple concurrent builds could interfere with each other
- No separation between source and build artifacts

## Decision
Implement out-of-source builds that move all C/C++ build artifacts to dedicated build directories outside the source tree.

### Key Changes
1. **Build Directory Structure**: Create `build-$(OS)$(TEMP_ID)/` directories with `obj/` and `lib/` subdirectories
2. **Path Handling**: Add `BUILD_OBJ_DIR` and `BUILD_OBJ` helper functions for consistent cross-platform path handling
3. **Makefile Updates**: Update main Makefile and 12 component build.mk files to use build directory structure
4. **Windows Path Fix**: Resolve ICON_OBJ path mixing issue between backslashes and forward slashes

### Technical Implementation
```makefile
# Helper functions for consistent path handling
BUILD_OBJ_DIR = build-$(OS)$(TEMP_ID)
BUILD_OBJ = $(BUILD_OBJ_DIR)/obj
BUILD_LIB = $(BUILD_OBJ_DIR)/lib

# Example usage in component makefiles
$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
```

## Consequences
### Positive
- **Clean Source Tree**: Source directory remains pristine with only source files
- **Parallel Builds**: Multiple builds can run simultaneously using different TEMP_ID values
- **Easy Cleanup**: Entire build can be removed by deleting build directory
- **Better Organization**: Clear separation between source and generated artifacts
- **Cross-platform Consistency**: Unified path handling across Windows, Linux, and macOS

### Negative
- **Increased Complexity**: More complex build system with additional path handling
- **Learning Curve**: Developers need to understand new build directory structure
- **Migration Effort**: Existing build scripts and tools needed updates

### Risks
- **Windows Path Handling**: Potential issues with mixed path separators (mitigated with helper functions)
- **Build Directory Cleanup**: Need to ensure clean targets work properly on all platforms
- **IDE Integration**: IDEs may need configuration updates to find build artifacts

## Implementation Details
- **Files Changed**: 25 files modified, including Makefile and 12 component build.mk files
- **Backwards Compatibility**: Maintained compatibility with existing build processes
- **Platform Support**: Verified on Windows, Linux, and macOS
- **Testing**: Comprehensive testing of build scenarios across platforms

## Alternatives Considered
1. **In-source builds with .gitignore**: Continue current approach with better gitignore rules
   - Rejected: Doesn't solve core pollution and parallel build issues
2. **CMake migration**: Switch to CMake for build system
   - Rejected: Too large a change for current needs; existing makefile system works well
3. **Separate build directory per component**: Each component gets its own build directory
   - Rejected: Over-complicated; single build directory per OS/TEMP_ID is sufficient

## References
- Commit: 3f9eb92f3 - "feat: implement out-of-source builds and add Phase 1 testing suite"
- Documentation: docs/build-system.md
- Issue Tracking: docs/problems_encountered/out-of-source-builds-path-handling.md

## Date
2026-01-11
