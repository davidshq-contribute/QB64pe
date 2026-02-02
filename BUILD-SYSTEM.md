# BUILD-SYSTEM.md

Documentation of the QB64pe build system.

## Overview

QB64pe uses a unified Makefile with platform detection to build from generated C++ code on Windows, Linux, and macOS.

## Build Flow

```
QB64 Source (.bas)
    ↓
QB64pe Compiler (transpiler)
    ↓
Generated C++ (internal/temp/*.txt)
    ↓
qbx.cpp (includes generated files)
    ↓
C++ Compiler
    ↓
Native Executable
```

## Platform Selection

```makefile
OS = lnx | win | osx
```

| Platform | Compiler | Shell |
|----------|----------|-------|
| Linux | System GCC | bash |
| Windows | MinGW | cmd |
| macOS | Xcode Clang | bash |

## Directory Structure

```
internal/c/
├── libqb/           # Runtime library
├── parts/           # Third-party deps
│   ├── audio/      # miniaudio
│   ├── core/       # FreeGLUT, GLEW
│   ├── video/      # fonts, images
│   └── ...
├── qbx.cpp         # Entry point
└── c_compiler/     # Windows MinGW
```

## Dependency Flags

| Flag | Component |
|------|-----------|
| DEP_GL | Graphics/OpenGL |
| DEP_SOCKETS | Network sockets |
| DEP_FONT | TrueType fonts |
| DEP_AUDIO_MINIAUDIO | Sound |
| DEP_ZLIB | Compression |

## Setup Scripts

### Linux (setup_lnx.sh)
- Detects distribution
- Installs packages
- Compiles QB64pe

### macOS (setup_osx.command)
- Checks for Xcode
- Compiles QB64pe

### Windows (setup_mingw.cmd)
- Downloads LLVM-MinGW
- Extracts to internal/c/c_compiler/

## Build Commands

```bash
# Linux
make OS=lnx

# Windows
mingw32-make OS=win

# macOS
make OS=osx

# Build QB64pe itself
make OS=lnx BUILD_QB64=y
```

## Key Files

| File | Purpose |
|------|---------|
| `Makefile` | Main build |
| `internal/c/libqb/build.mk` | Runtime |
| `internal/c/parts/*/build.mk` | Components |
| `setup_*.sh/cmd` | Setup scripts |
