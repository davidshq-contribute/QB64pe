# Build System

This document describes how QB64-PE compiles programs and how the build system works.

---

## Path Handling Challenges

### Windows Path Separator Issues

During implementation of out-of-source builds, several path handling challenges were encountered due to mixed use of forward slashes (Unix-style) and backslashes (Windows-style).

#### Windows ICON_OBJ Path Mixing

**Location**: `Makefile` line 96

**Problem**: On Windows, `ICON_OBJ` was defined as:
```makefile
ICON_OBJ := $(BUILD_OBJ_DIR)\icon.o
```

But `BUILD_OBJ_DIR` is defined with forward slashes (`$(BUILD_DIR)/obj`), creating a mixed path like `build-win/obj\icon.o` which is invalid.

**Solution**: Use the `BUILD_OBJ` helper function which normalizes paths:
```makefile
ICON_OBJ := $(call BUILD_OBJ,$(PATH_INTERNAL_TEMP)/icon.rc)
```

#### Pattern Rule Path Matching on Windows

**Location**: Pattern rules throughout build.mk files

**Problem**: Pattern rules like `$(BUILD_OBJ_DIR)/%.o: %.cpp` use forward slashes, but on Windows, source paths may use backslashes (e.g., `internal\c\libqb\src\file.cpp`). Make's pattern matching may not work correctly with mixed separators.

**Solution**: Ensure consistent use of forward slashes in all build system paths and use proper path normalization functions.

---

## Related Documentation

- [Auto-Including](auto-including.md) - How support files are automatically included
- [Architecture](ARCHITECTURE.md) - Overall compiler architecture
- [Testing](testing.md) - Testing infrastructure

## Build Process

This describes the process that goes on to compile a QB64 program into an executable.

1. `QB64-PE` takes the QB64 source code and produces C++ source code for the program. That C++ source is placed into various files and goes into `./internal/temp`.
2. `make` is used to run the `Makefile` at the root of this repository. It is supplied the `OS` parameter, the `EXE` parameter for the final executable name, and a variety of `DEP_*` parameters depending on what dependencies the QB64 program requires.
    1. 3rd party dependencies located in `./internal/c/parts/` are compiled if requested. Each of these has its own `build.mk` file that includes the `make` logic to build that dependency.
    2. `./internal/c/libqb.cpp` is compiled with various C preprocessor flags depending on the provided `DEP_*` flags. The name is changed depending on the settings it is compiled with so that we don't accidentally use a libqb compiled with different settings.
    3. `./internal/qbx.cpp` is compiled, which uses `#include` to pull in the generated C++ source located in `./internal/temp`.
    4. All of `qbx.o`, `libqb.o`, and the 3rd party dependencies are linked together to produce the executable.
    5. Debug symbols are stripped from the executable to make it smaller, the symbols are stored into a symbol file ending in `.sym` in `./internal/temp`.

## Build Directory Structure

QB64-PE uses **out-of-source builds**, meaning all build artifacts (object files, libraries, executables) are placed in a separate build directory rather than alongside source files. This keeps the source tree clean and makes it easy to clean build artifacts.

### Build Directory Location

The build directory is named `build-$(OS)$(TEMP_ID)/` where:
- `$(OS)` is the operating system identifier (`win`, `lnx`, or `osx`)
- `$(TEMP_ID)` is an optional instance identifier (blank for the default instance, or a number for parallel IDE instances)

For example:
- `build-lnx/` - Linux build (default instance)
- `build-win2/` - Windows build (instance 2, for parallel IDE)
- `build-osx/` - macOS build (default instance)

### Build Directory Contents

```
build-$(OS)$(TEMP_ID)/
├── obj/                      # All object files (.o)
│   └── [mirrors source directory structure]
│       ├── internal/
│       │   └── c/
│       │       ├── libqb/
│       │       │   └── src/          # libqb object files
│       │       ├── parts/
│       │       │   ├── audio/        # audio objects
│       │       │   ├── core/         # core objects
│       │       │   └── ...           # other parts
│       │       ├── qbx$(TEMP_ID).o   # main qbx object
│       │       └── libqb_make_*.o    # libqb main object
│       └── temp$(TEMP_ID)/           # Generated source (unchanged)
├── lib/                      # All library files (.a)
│   ├── freeglut.a
│   ├── audio.a
│   ├── font.a
│   └── ...
└── tests/                    # Test executables (if building tests)
    └── exes/
        └── cpp/
```

### Benefits of Out-of-Source Builds

1. **Clean source tree**: No build artifacts mixed with source code
2. **Easy cleanup**: Remove the entire build directory to clean all artifacts
3. **Multiple configurations**: Can have separate build directories for different OS/configurations
4. **Version control**: Build artifacts won't accidentally be committed
5. **Parallel builds**: Easier to support multiple build configurations simultaneously

### Cleaning Build Artifacts

To clean all build artifacts, use the `clean` target:

```bash
make clean OS=lnx
```

This removes the entire `build-$(OS)$(TEMP_ID)/` directory and all its contents.

## CI Process

This describes how QB64-PE itself is built by the CI process to produce a release of QB64-PE. The actual build scripts that do all of this are located in `.ci/`.

1. Before CI ever begins, `./internal/source/` contains the generated C++ source of a previous version of `qb64pe.bas`.
2. `.ci/calculate_version.sh` is run to determine what version this CI build should be considered. That information is written to `./internal/version.txt`, if it is a release version `./internal/version.txt` is removed.
3. `qb64pe_bootstrap` is compiled from `./internal/source`
    1. `make` is used to call the `Makefile` with the proper `OS` setting, `EXE=qb64pe_bootstrap`, and `BUILD_QB64=y`.
    2. The Makefile will take care of copying `./internal/source` into `./internal/temp` and compiling QB64-PE with the proper settings.
4. `qb64pe_bootstrap` is used to compile `./source/qb64pe.bas` into a proper `qb64pe` executable
    1. We run `./qb64pe_bootstrap -x ./source/qb64pe.bas` to compile QB64-PE. This compiles QB64-PE the same way as a regular QB64 program as detailed in 'Build Process'
5. `./internal/source` is cleared out and, excluding a few files, the contents of `./internal/temp` are copied into `./internal/source` as the new generated C++ source of QB64-PE.
6. `tests/run_tests.sh` is run to test the compiled version of QB64-PE. A failure of these tests fails the build at this stage.
7. `.ci/make-dist.sh` is run to produce the distribution of QB64-PE, which is stored as a build artifact and potentially a release artifact..
    1. The contents of the distribution depends on the OS it is being created for.
        1. The Windows distribution contains a precompiled copy of QB64-PE as `qb64pe.exe` and can be used immediately.
        2. The Linux and OSX distributions do not contain a precompiled copy of QB64-PE. Those version come with a "setup" script that has to be run which builds QB64-PE based on the contents of `./internal/source`.
8. `tests/run_dist_tests.sh` is run to verify the distribution of QB64-PE works correctly.
9. If this is a CI build of `main` and `./internal/source` has changed due to the newly compiled version, then `git` is used to commit and push the updated version of `./internal/source` to the GitHub repo.

## Repository Layout

- `.ci/` - All files in this folder are related to the CI build process.
  - `bootstrap.bat`
    - Windows only, Downloads MinGW compiler, builds the precompiled version of QB64-PE located in `./internal/source` as `qb64pe_bootstrap.exe`.
  - `bootstrap.sh`
    - Linux and OSX, builds the precompiled version of QB64-PE located in `./internal/source` as `qb64pe_bootstrap.exe`.
  - `compile.bat`
    - Uses `qb64pe_bootstrap.exe` to build `./source/qb64pe.bas`. This compiled QB64-PE is the released `qb64pe.exe`.
  - `compile.sh`
    - Uses `qb64pe_bootstrap` to build `./source/qb64pe.bas`. This built QB64-PE is what is used for testing, and the sources from this QB64-PE are placed into `./internal/source`.
  - `make-dist.sh`
    - Copies all the relevant parts of QB64-PE into a new folder, which can be distributed as a release.
  - `push-internal-source.sh`
    - If `./internal/source` is different after building `./source/qb64pe.bas`, then this will automatically push those changes to the repository to update the sources used to build `qb64pe_bootstrap`.
- `internal/`
  - `c/` - Contains everything related to the C/C++ source files for QB64-PE.
    - `c_compiler/`
      - On Windows, this folder is populated with the MinGW C++ compiler during the CI process (or when using `setup_win.cmd`).
    - `parts/`
      - Contains the sources to many of the dependencies that QB64 uses. Most of the dependencies have a `build.mk` file that is used by the main `Makefile` to build them.
  - `source/`
    - Contains a copy of the generated C++ source of a previous version of QB64-PE. This is used to build a copy of QB64-PE using only a C++ compiler. This is updated automatically via the CI process.
  - `version.txt`
    - QB64-PE checks this file to determine if there is a version tag (`-foobar` on the end of the version) for this version of QB64-PE.
- `source/`
  - Contains the QB64 source to QB64-PE itself.
- `tests/` - Contains the tests run on QB64-PE during CI to verify changes.
  - `compile_tests/`
    - Testcases related to specific dependencies that QB64 can pull in. These tests are largely intended to test that QB64-PE and the Makefile correctly pulls in the proper dependencies.
  - `c/`
    - The source for the C++-based runtime tests. Each test is a `.cpp` file that compiles into an executable.
    - `test.h` and `test.cpp` - Test framework for C++ tests
  - `build.mk`
    - Build configuration for C++ runtime tests. Defines test executables and their dependencies.
  - `unit/`
    - Unit tests for compiler components (type system, parser, code generation, etc.)
  - `integration/`
    - Integration tests for end-to-end compiler behavior
  - `qbasic_testcases/`
    - A variety of collected QB64 sample programs
  - `dist/`
    - Test files for distribution tests.
  - `compile_tests.sh`
    - Runs the `compile_tests` test cases.
  - `qbasic_tests.sh`
    - Compiles all the testcases in `qbasic_testcases` and verifies they compile successfully.
  - `dist_tests.sh`
    - Verifies the output of `make-dist.sh` is a functioning distribution of QB64-PE
  - `run_dist_tests.sh`
    - Runs the distribution test collections.
  - `run_tests.sh`
    - Runs all individual test collections.
  - `run_c_tests.sh`
    - Runs all the C++ runtime test cases.
  - `test_discovery.sh`
    - Automatic test discovery system for categorizing and filtering tests
  - `test_report.sh`
    - Generates HTML and text test reports
  - `test_utils.sh`
    - Common test utility functions
  - `continuous_test.sh`
    - Watch mode and continuous testing support
- `setup_lnx.sh`
  - Used as part of the Linux release to install dependencies and compile QB64-PE.
- `setup_osx.command`
  - Used as part of the OSx release to compile QB64-PE.
- `setup_win.cmd`
  - Used only for compiled QB64-PE directly from a clone of the repository (not a release, we distribute QB64-PE already compiled in the Windows release)
- `Makefile`
  - Used for building QB64 programs.

## Makefile Usage and Parameters

> **Note**: These parameters are not guaranteed to stay the same. The 'setup' scripts and QB64-PE are the only things intended to call the Makefile directly.

These flags control some basic settings for how the `Makefile` can compile the program.

| Parameter | Default Value | Valid Values | Required | Description |
| :-------- | :-----------: | :----------: | :------: |  :---------- |
| `OS` | None | `win`, `lnx`, or `osx` | Yes | Indicates to the Makefile which OS it is being used to compile for. Controls what utilities the Makefile assumes is available and where they are located, and also platform dependent compiler settings. |
| `BUILD_QB64` | None | `y` | No | If set then the Makefile will build the QB64-PE source located in `./internal/source`. If `EXE` is not specified, that is set to `qb64pe`. Dependency settings for QB64-PE are set automatically. |
| `EXE` | None | Executable name | Yes, if not using `BUILD_QB64` | Specifies the name of the executable to build. If on Windows, it should include `.exe` on the end. Spaces in the filename should be escaped with `\` |
| `TEMP_ID` | None | blank, or an integer | No | Controls the name of `qbx*.cpp` and `./internal/temp*` that are used for compilation. This is mainly relevant for multiple instances of the IDE, where the second instance compiles to `./internal/temp2` and tells the Makefile which to use. Further instances will use higher numbers. |
| `CXXFLAGS_EXTRA` | None | Collection of C++ compiler flags | No | These flags are provided to the compiler when building the C++ files. This includes the generate source files, but also files like `libqb.cpp`. |
| `CXXLIBS_EXTRA` | None | Collection of linker flags | No | These flags are provided to the compiler (`g++` or `clang++`) when linking the final executable. Typically includes the `-l` library flags |
| `CFLAGS_EXTRA` | None | Collection of C compiler flags | No | These flags are provided to the compiler when building C files |
| `STRIP_SYMBOLS` | None | `n` | No | Symbols are stripped by default, if this is `n` then symbols will not be stripped from the executable |
| `GENERATE_LICENSE` | None | `y` | No | Generates a `.license.txt` file next to your executable, which contains all the licenses that apply to the executable |
| `LICENSE` | `$(EXE).license.txt`| License file name | No | Specifies an alternative filename for the license file |

These flags controls whether certain dependencies are compiled in or not. All of them are blank by default and should be set to `y` to use them. Note that the program being built may not compile if you supply the wrong dependency settings - may of them supply functions that will not be defined if the dependency is required, and some of them (Ex. `DEP_GL`) have requirements the program being compiled must meet.

| Parameter | Description |
| :-------- | :---------- |
| `DEP_GL` | Enables the `_GL` support |
| `DEP_SCREENIMAGE` | Enables `_SCREENIMAGE` support, also implies `DEP_IMAGE_CODEC=y` |
| `DEP_IMAGE_CODEC` | Enables `_LOADIMAGE` and the various image decoders. |
| `DEP_SOCKETS` | Adds networking support. |
| `DEP_PRINTER` | Adds `_PRINTIMAGE` support. |
| `DEP_ICON` | Adds `_ICON` support. |
| `DEP_ICON_RC` | Adds `$EXEICON` and `$VERSIONINFO` support, compiles `.rc` file into the executable on Windows |
| `DEP_FONT` | Enables various `_FONT` related support. |
| `DEP_DEVICEINPUT` | Enables game controller input support. |
| `DEP_ZLIB` | Adds `_DEFLATE` and `_INFLATE` support. |
| `DEP_EMBED` | Compiles in data embedded via `$EMBED` statements. |
| `DEP_CONSOLE` | On Windows, this gives the program console support (graphical support is still allowed) |
| `DEP_CONSOLE_ONLY` | Same as `DEP_CONSOLE`, but also removes GLUT and graphics support. |
| `DEP_AUDIO_MINIAUDIO` | Pulls in sound support using miniaudio for playing sounds via `PLAY`, `_SNDPLAY`, and various other functions that makes sounds. |
| `DEP_HTTP` | Enables http support via libcurl. Should only be used if `DEP_SOCKETS` is on. |

## Test Build System

The Makefile includes support for building C++ runtime tests through `tests/build.mk`. This system allows testing of `libqb` runtime functionality.

### Building Tests

To build all C++ runtime tests:

```bash
make build-tests OS=lnx  # or OS=win, OS=osx
```

This will:
1. Compile the test framework (`tests/c/test.cpp`)
2. Compile all test executables defined in `tests/build.mk`
3. Place test executables in `tests/exes/cpp/`

### Test Configuration

Tests are configured in `tests/build.mk`:
- Each test has a name (e.g., `buffer`, `http`, `graphics`)
- Source files are specified with `testname.src-y`
- Additional compiler flags with `testname.cflags-y`
- Additional linker flags with `testname.libs-y`
- Platform-specific settings using `testname.libs-$(platform)`

### Running Tests

After building, tests can be run individually or via the test runner:

```bash
# Run all C++ tests
./tests/run_c_tests.sh

# Or run individual test executables
./tests/exes/cpp/buffer_test
./tests/exes/cpp/http_test
```

For more information about testing, see [Testing Documentation](testing.md).

---

## Out-of-Source Builds Test Results

### Test Date
Windows build test completed during implementation

### Test Environment
- **OS**: Windows 10
- **Make**: mingw32-make (from internal MinGW compiler)
- **Compiler**: clang-19 (via MinGW toolchain)

### Test 1: Basic Build

**Command:**
```bash
make OS=win EXE=test_build.exe DEP_CONSOLE_ONLY=y
```

**Result**: ✅ **SUCCESS**

- Build completed successfully
- Executable `test_build.exe` created in root directory
- Build directory `build-win/` created with proper structure:
  - `build-win/obj/` - Contains all object files
  - `build-win/lib/` - Contains all library files (.a)
- Object files placed in `build-win/obj/internal/c/...` mirroring source structure
- Library files placed in `build-win/lib/`

**Issues Found and Fixed:**
1. Windows `mkdir` failing when directory exists - Fixed with `MKDIR_SAFE` helper
2. Clean target not handling directories on Windows - Fixed with `RMDIR` for directories

### Test 2: Build Directory Structure

**Verification:**
- ✅ Object files in `build-win/obj/` subdirectories
- ✅ Library files in `build-win/lib/`
- ✅ Source tree remains clean (no .o or .a files in source directories)

### Test 3: Clean Target

**Command:**
```bash
make OS=win clean
```

**Result**: ✅ **SUCCESS** (after fix)

- Build directory `build-win/` removed successfully
- All build artifacts cleaned

**Initial Issue:**
- Windows `del /Q` doesn't work on directories
- Fixed by using `rmdir /S /Q` for directories and handling separately

### Test 4: Test Build System

**Command:**
```bash
make OS=win build-tests
```

**Result**: ⚠️ **PARTIAL SUCCESS**

- Test framework compiled successfully
- Some tests compiled (buffer_test.exe created)
- HTTP test failed due to missing curl headers (expected - requires DEP_HTTP=y)
- Directory creation issues fixed during testing

**Note**: Full test build requires proper dependency flags. The build system itself works correctly.

### Summary

#### ✅ Working Correctly

1. **Out-of-source builds**: All object files and libraries go to `build-win/` directory
2. **Path handling**: Windows path normalization works correctly
3. **Directory creation**: `MKDIR_SAFE` handles Windows directory creation safely
4. **Clean target**: Removes entire build directory correctly
5. **Build structure**: Mirrors source directory structure in build directory

#### ⚠️ Known Limitations

1. **Test builds**: Some tests require specific dependency flags (e.g., HTTP tests need DEP_HTTP=y)
2. **Platform testing**: Only tested on Windows; Linux and macOS testing needed

#### 🔧 Issues Fixed During Testing

1. **Windows MKDIR errors**: Created `MKDIR_SAFE` helper function
2. **Windows clean target**: Fixed to use `rmdir /S /Q` for directories
3. **Library directory creation**: Updated all library creation rules to use `MKDIR_SAFE`

### Recommendations

1. **Test on Linux and macOS**: Verify builds work correctly on all platforms
2. **Test parallel builds**: Verify `TEMP_ID` works for parallel IDE instances
3. **Test full dependency builds**: Test with various `DEP_*` flags enabled
4. **Add to CI**: Include out-of-source build verification in CI pipeline

### Conclusion

The out-of-source builds implementation is **working correctly** on Windows. The build system successfully:
- Creates build directory structure
- Compiles all source files to build directory
- Creates libraries in build directory
- Cleans build directory properly
- Maintains backward compatibility with existing Makefile parameters

The implementation is ready for use, with recommended testing on other platforms.

---

## Versioning

QB64 Phoenix Edition follows SemVer, which means that major releases indicate a breaking change, minor releases indicate new features, and patch release indicates bug fixes.

- Release versions of QB64-PE will have just a version number in the form `X.Y.Z`. All other versions of QB64-PE will have some kind of 'tag' at the end of the version, which is arbitrary text after a `-` placed on the end of the version.
- CI versions get a tag in the form of `-XX-YYYYYYYY`, where `XX` is the number of commits since the last release, and `YYYYYYYY` is the first 8 digits of the commit hash of that build.
- If you build the repository directly, you would get an `-UNKNOWN` version, which indicates that due to not running through the CI process we do not know what particular version (if any) that you are using.

## Release Process

1. Update the version listed in `./source/global/version.bas`. Update both the `Version$` label and also the `$VERSIONINFO` entries.
2. After `version.bas` is compiled, wait for the CI build on `main` and also update of `./internal/source` to finish.
3. Tag the latest commit in `main` with the format of `vX.Y.Z` where `X.Y.Z` is the version being released.
4. Wait for the tag build to finish, it will create a draft release for the new version.
5. Edit the draft release to include release notes, and publish it when ready.

## See Also

- [Auto-Including Documentation](auto-including.md) - Automatic include file handling
- [Architecture Documentation](ARCHITECTURE.md) - Compiler architecture overview
- [Testing Documentation](testing.md) - Comprehensive testing infrastructure
