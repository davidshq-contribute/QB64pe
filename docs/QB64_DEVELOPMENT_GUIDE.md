# QB64-PE Development Guide

## Overview
This guide documents the QB64-PE development workflow, build system, and common procedures based on hands-on experience with the codebase.

## Architecture Overview

### Key Components
- **Compiler Source**: `source/qb64pe.bas` - Main QB64 compiler source code
- **Runtime Library**: `internal/c/libqb/` - C++ runtime library
- **Bootstrapped Sources**: `internal/source/` - Generated C++ sources used by running compiler
- **Generated Temp Files**: `internal/temp/` - Generated during compilation of user programs
- **Build System**: `Makefile` + platform-specific scripts

### Compilation Flow
1. **Bootstrap Process**: `qb64pe_bootstrap.exe` compiles `source/qb64pe.bas` → `internal/source/`
2. **Main Build**: Uses bootstrapped sources to build `qb64pe.exe`
3. **User Compilation**: `qb64pe.exe` uses `internal/source/` to compile user programs

## Build Procedures

### Environment Setup
```bash
# Windows setup
setup_win.cmd    # Sets up environment and PATH

# Key environment variables:
# - PATH includes internal/c/c_compiler/bin
# - Build tools: mingw32-make, c++.exe, llvm-rc.exe
```

### Bootstrap Process
```bash
# Create bootstrap compiler
.ci/bootstrap.bat

# Regenerate sources from updated qb64pe.bas
qb64pe_bootstrap.exe -x -w source\qb64pe.bas

# Build final compiler
.ci/compile.sh
```

### Direct Build Commands
```bash
# Clean build
mingw32-make clean

# Build specific executable
mingw32-make EXE=qb64pe.exe

# Build with debug symbols
mingw32-make EXE=qb64pe.exe DEBUG=1
```

## File System Structure

### Critical Directories
```
QB64pe/
├── source/                    # Original QB64 source code
│   ├── qb64pe.bas           # Main compiler source
│   └── ide/                 # IDE-related sources
├── internal/
│   ├── source/               # Bootstrapped C++ sources (USED BY RUNNING COMPILER)
│   ├── temp/                 # Generated during user program compilation
│   └── c/                   # C++ runtime library
├── build-win/                # Build output directory
└── tests/                    # Test suites
```

### Key Files
- `source/qb64pe.bas` - Master compiler source
- `internal/source/main.txt` - Bootstrapped main program template
- `internal/source/mainerr.txt` - Bootstrapped error handling template
- `internal/temp/*.txt` - Generated per user compilation (overwritten each run)

## Development Workflows

### Modifying Compiler Behavior
1. **Edit Source**: Modify `source/qb64pe.bas`
2. **Bootstrap**: Run bootstrap process to regenerate `internal/source/`
3. **Rebuild**: Build new `qb64pe.exe` using updated sources
4. **Test**: Verify changes work with test programs

### Common Pitfalls
- **Direct temp file editing**: Changes to `internal/temp/` are overwritten each compilation
- **Source vs Bootstrapped**: Running compiler uses `internal/source/`, not `source/`
- **CONST placement**: CONST declarations must come before executable statements in QB64

### Error Handling Refactoring Example

#### Problem
Deprecated global variables in error handling:
```cpp
// Old patterns (deprecated)
error_handling = 1;
error_occurred = 0;
error_goto_line = label;
```

#### Solution Steps
1. **API Implementation**: Add new functions in `internal/c/libqb/`
2. **Source Updates**: Modify `source/qb64pe.bas` code generation patterns
3. **Bootstrap**: Regenerate `internal/source/` with new patterns
4. **Rebuild**: Create new compiler binary
5. **Verify**: Test with error handling test cases

#### Code Changes Made
```cpp
// New API functions
void set_error_handling(bool state);
bool is_error_handling();
void set_error_goto_line(uint32_t line);
uint32_t get_error_goto_line();
```

```basic
' Updated code generation in qb64pe.bas
WriteBufLine MainTxtBuf, "if (!is_error_handling()){error(20);}else{set_error_handling(false); set_error_err(0); return;}"
```

## Build System Details

### Makefile Structure
- Platform detection (Windows/Linux/macOS)
- Compiler configuration
- Dependency management
- Resource compilation (icons, etc.)

### Common Build Issues
1. **Missing llvm-rc.exe**: Fixed by adding to PATH in setup_win.cmd
2. **Missing output directories**: Fixed with $(call MKDIR_SAFE,$(dir $@))
3. **Interactive prompts**: Use echo "n" | make for non-interactive builds

### Resource Compilation
```makefile
# Windows resource compilation
$(ICON_OBJ): $(PATH_INTERNAL_TEMP)\icon.rc | $(BUILD_OBJ_DIR)
	$(call MKDIR_SAFE,$(dir $@))
	$(LLVMRC) /FO $@ $<
```

## Testing Procedures

### Test Categories
1. **Unit Tests**: `tests/unit/` - Component testing
2. **Integration Tests**: `tests/integration/` - End-to-end testing
3. **Runtime Tests**: `tests/c/` - C++ library testing
4. **Compile Tests**: `tests/compile_tests/` - Compilation verification

### Running Tests
```bash
# All tests
./tests/run_tests.sh

# With discovery system
./tests/run_tests_with_discovery.sh

# Continuous testing
./tests/continuous_test.sh --watch
```

### Test File Creation
```basic
' Example test structure
'$INCLUDE: 'test_framework.bi'
'$INCLUDE: 'test_state_manager.bi'

TestInitialize()
TestResult = AssertEquals(expected, actual)
TestCleanup()
```

## Debugging Procedures

### Common Issues
1. **Generated Code Problems**: Check `internal/temp/main.txt` and `mainerr.txt`
2. **Compilation Failures**: Check `internal/temp/compilelog.txt`
3. **Runtime Errors**: Use logging framework in `internal/c/libqb/src/logging/`

### Debugging Tools
- **Compile Log**: `internal/temp/compilelog.txt`
- **Generated Code**: `internal/temp/*.txt`
- **Build Output**: Makefile output shows compilation steps
- **Logging Framework**: Runtime logging for debugging

## Code Quality Procedures

### Code Style
- Follow existing patterns in source files
- Use consistent indentation and naming
- Add comments for complex logic
- Update documentation for API changes

### Review Process
1. **Code Review**: Manual inspection of changes
2. **Grep Search**: Verify no deprecated patterns remain
3. **Compilation Test**: Ensure code builds without errors
4. **Functional Test**: Verify behavior with test cases

## Specific Commands Reference

### File Operations
```bash
# Copy files (PowerShell)
Copy-Item source.txt destination.txt -Force

# Search for patterns
grep -r "pattern" directory/
Select-String -Pattern "pattern" -Recurse
```

### Build Commands
```bash
# Environment setup
.\setup_win.cmd

# Bootstrap
.\ci\bootstrap.bat

# Build
mingw32-make EXE=qb64pe.exe

# Clean
mingw32-make clean
```

### Testing Commands
```bash
# Run specific test
qb64pe.exe -x tests\test_file.bas

# Check generated code
Get-Content internal\temp\main.txt
```

## Lessons Learned

### Critical Insights
1. **Bootstrapping is Key**: The running compiler uses `internal/source/`, not `source/`
2. **Temp Files are Ephemeral**: `internal/temp/` is regenerated each compilation
3. **CONST Rules Matter**: QB64 syntax requires CONST before executable statements
4. **Build Order Matters**: Bootstrap → Build → Test cycle is essential

### Common Mistakes to Avoid
1. **Editing temp files directly** - changes are lost
2. **Forgetting to bootstrap** - changes in source/ won't take effect
3. **Ignoring build errors** - fix issues immediately
4. **Not testing after changes** - verification is crucial

### Best Practices
1. **Document changes** - update relevant documentation
2. **Test incrementally** - test small changes frequently
3. **Use version control** - commit working states
4. **Understand the flow** - know which files are used when

## Quick Reference

### Error Handling Refactoring Checklist
- [ ] Add API functions to `internal/c/libqb/src/error_handle.cpp`
- [ ] Add declarations to `internal/c/libqb/include/error_handle.h`
- [ ] Update code generation in `source/qb64pe.bas`
- [ ] Run bootstrap to update `internal/source/`
- [ ] Rebuild `qb64pe.exe`
- [ ] Test with error handling test cases
- [ ] Verify no deprecated patterns remain

### Build Troubleshooting
- **Path Issues**: Check `setup_win.cmd` PATH modifications
- **Missing Tools**: Verify `internal/c/c_compiler/bin/` contents
- **Resource Errors**: Check Makefile LLVMRC configuration
- **Permission Issues**: Run as administrator if needed

### Development Workflow Summary
1. Make changes to source code
2. Bootstrap to regenerate internal sources
3. Build new compiler binary
4. Test changes with appropriate test cases
5. Verify no regressions
6. Update documentation
7. Commit changes
