# Code Formatting and Linting

This document describes how to use the code formatting and linting tools for the C/C++ code in QB64-PE.

## Overview

The project uses:
- **clang-format** for automatic code formatting
- **clang-tidy** for static analysis and linting

Both tools are configured via:
- `.clang-format` - Formatting style configuration
- `.clang-tidy` - Linting rules and checks

## Prerequisites

### Installing LLVM/Clang Tools

**Windows:**
1. Download LLVM from https://github.com/llvm/llvm-project/releases
2. Install and ensure `clang-format.exe` and `clang-tidy.exe` are in your PATH
3. Or use a package manager:
   ```powershell
   # Using Chocolatey
   choco install llvm
   
   # Using Scoop
   scoop install llvm
   ```

**Linux:**
```bash
# Ubuntu/Debian
sudo apt-get install clang-format clang-tidy

# Fedora
sudo dnf install clang-tools-extra

# Arch Linux
sudo pacman -S clang
```

**macOS:**
```bash
# Using Homebrew
brew install llvm

# Add to PATH (if needed)
echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
```

### Verify Installation

```bash
clang-format --version
clang-tidy --version
```

## Code Formatting

### Using the Helper Scripts

**Windows (PowerShell):**
```powershell
# Format all C/C++ files
.\scripts\format-code.ps1

# Check which files need formatting (without modifying them)
.\scripts\format-code.ps1 --check

# Format specific files
.\scripts\format-code.ps1 --files internal/c/qbx.cpp internal/c/qbx2.cpp
```

**Linux/macOS:**
```bash
# Format all C/C++ files
./scripts/format-code.sh

# Check which files need formatting (without modifying them)
./scripts/format-code.sh --check

# Format specific files
./scripts/format-code.sh --files internal/c/qbx.cpp internal/c/qbx2.cpp
```

### Using clang-format Directly

```bash
# Format a single file
clang-format -i path/to/file.cpp

# Check if a file is formatted correctly
clang-format path/to/file.cpp | diff path/to/file.cpp -

# Format multiple files
clang-format -i file1.cpp file2.cpp file3.cpp
```

### Formatting Style

The formatting style is defined in `.clang-format`. Key characteristics:
- **Indentation**: 4 spaces (no tabs)
- **Column Limit**: 160 characters
- **Brace Style**: Attached (K&R style)
- **Pointer Alignment**: Right (`int* ptr`)
- **Includes**: Sorted and grouped

## Code Linting

### Using the Helper Scripts

**Windows (PowerShell):**
```powershell
# Lint all C/C++ files
.\scripts\lint-code.ps1

# Lint and automatically fix issues where possible
.\scripts\lint-code.ps1 --fix

# Lint specific files
.\scripts\lint-code.ps1 --files internal/c/qbx.cpp
```

**Linux/macOS:**
```bash
# Lint all C/C++ files
./scripts/lint-code.sh

# Lint and automatically fix issues where possible
./scripts/lint-code.sh --fix

# Lint specific files
./scripts/lint-code.sh --files internal/c/qbx.cpp
```

### Using clang-tidy Directly

```bash
# Lint a single file
clang-tidy path/to/file.cpp

# Lint and fix issues automatically
clang-tidy -fix path/to/file.cpp

# Lint with specific checks
clang-tidy -checks='-*,readability-*' path/to/file.cpp
```

### Linting Configuration

The linting rules are defined in `.clang-tidy`. Enabled check categories:
- **bugprone-*** - Bug detection
- **cert-*** - CERT secure coding guidelines
- **cppcoreguidelines-*** - C++ Core Guidelines
- **performance-*** - Performance optimizations
- **readability-*** - Code readability
- **modernize-*** - Modern C++ features

Some checks are disabled for compatibility with the existing codebase.

### Compile Commands

For best results, clang-tidy benefits from a `compile_commands.json` file that describes how files are compiled. This can be generated using:

**Linux/macOS:**
```bash
# Using bear (install: sudo apt-get install bear)
bear -- make OS=lnx

# Or using intercept-build (from scan-build)
intercept-build make OS=lnx
```

**Windows:**
```powershell
# Using bear (if available via WSL or similar)
# Or manually create compile_commands.json based on Makefile
```

## Integration with IDEs

### Visual Studio Code

Install the extensions:
- **C/C++** (Microsoft)
- **clang-format** (xaver.clang-format)
- **clangd** (llvm-vs-code-extensions.vscode-clangd)

The `.clangd` file is already configured for the project.

### CLion

CLion automatically detects `.clang-format` and `.clang-tidy` files. Configure in:
- **Settings → Editor → Code Style → C/C++** (for formatting)
- **Settings → Editor → Inspections → C/C++** (for linting)

### Visual Studio

1. Install the **ClangFormat** extension
2. Configure in **Tools → Options → ClangFormat**
3. Set "Use clang-format file" to enabled

## Pre-commit Hooks (Optional)

You can set up pre-commit hooks to automatically format code before commits:

**Windows (PowerShell):**
```powershell
# Create pre-commit hook
$hookContent = @"
#!/bin/sh
.\scripts\format-code.ps1 --check
if (`$LASTEXITCODE -ne 0) {
    echo "Code formatting check failed. Run .\scripts\format-code.ps1 to fix."
    exit 1
}
"@
$hookContent | Out-File -FilePath .git\hooks\pre-commit -Encoding utf8
```

**Linux/macOS:**
```bash
# Create pre-commit hook
cat > .git/hooks/pre-commit << 'EOF'
#!/bin/bash
./scripts/format-code.sh --check
if [ $? -ne 0 ]; then
    echo "Code formatting check failed. Run ./scripts/format-code.sh to fix."
    exit 1
fi
EOF
chmod +x .git/hooks/pre-commit
```

## Continuous Integration

Example GitHub Actions workflow:

```yaml
name: Code Quality

on: [push, pull_request]

jobs:
  format:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - uses: DoozyX/clang-format-lint-action@v0.14
        with:
          clangFormatVersion: 15
          checkPath: internal/c
          exclude: '(freetype|stb|nanosvg)'

  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Install clang-tidy
        run: sudo apt-get install -y clang-tidy
      - name: Run clang-tidy
        run: ./scripts/lint-code.sh
```

## Excluded Files

The following directories are excluded from formatting/linting (third-party code):
- `internal/c/parts/video/font/freetype/` - FreeType library
- `internal/c/parts/video/image/stb/` - STB libraries
- `internal/c/parts/video/image/nanosvg/` - NanoSVG library
- `internal/c/parts/video/image/qoi/` - QOI library
- `internal/c/parts/video/image/jo_gif/` - jo_gif library
- Other third-party dependencies

## Troubleshooting

### clang-format not found
- Ensure LLVM/Clang tools are installed and in your PATH
- On Windows, you may need to restart your terminal after installation

### clang-tidy shows many false positives
- Some checks may need to be disabled in `.clang-tidy` for legacy code
- Use `// NOLINT` comments to suppress specific warnings:
  ```cpp
  int* ptr = malloc(sizeof(int)); // NOLINT
  ```

### Formatting conflicts with existing code
- The `.clang-format` file can be adjusted to match existing style
- Consider formatting incrementally, one directory at a time

### Performance issues with large codebase
- Use the `--files` option to format/lint specific files
- Consider using parallel execution for large batches

## Additional Resources

- [clang-format Documentation](https://clang.llvm.org/docs/ClangFormat.html)
- [clang-tidy Documentation](https://clang.llvm.org/extra/clang-tidy/)
- [ClangFormat Style Options](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
- [C++ Core Guidelines](https://isocpp.github.io/CppCoreGuidelines/)
