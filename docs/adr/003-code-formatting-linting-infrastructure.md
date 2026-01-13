# ADR-003: Code Formatting and Linting Infrastructure

## Status
Accepted

## Context
QB64pe lacked automated code formatting and linting tools, leading to inconsistent code style across the codebase and potential quality issues. Manual code style enforcement was error-prone and time-consuming, with no systematic approach to catching common coding issues early in development.

### Problems Addressed
- Inconsistent code formatting across C++ and BASIC source files
- No automated detection of common coding issues and potential bugs
- Manual code review burden for style violations
- Difficulty maintaining consistent style across multiple contributors
- No integration with modern IDE features like clangd

## Decision
Implement a comprehensive code formatting and linting infrastructure using clang-format, clang-tidy, and clangd with cross-platform automation scripts.

### Key Components Implemented

#### 1. Code Formatting with clang-format
- **Configuration**: `.clang-format` with QB64pe-specific style rules
- **Language Detection**: Automatic detection of C++, BASIC, and other file types
- **IDE Integration**: clangd integration for real-time formatting
- **Cross-Platform**: Consistent formatting across Windows, Linux, and macOS

#### 2. Static Analysis with clang-tidy
- **Configuration**: `.clang-tidy` with comprehensive rule set
- **Bug Detection**: Automated detection of common coding issues
- **Performance Analysis**: Identification of performance bottlenecks
- **Modern C++**: Enforcement of modern C++ best practices

#### 3. Language Server Protocol (LSP) Support
- **clangd Configuration**: `.clangd` for IDE integration
- **Real-time Analysis**: Live error detection and suggestions
- **Code Completion**: Enhanced autocomplete and navigation
- **Refactoring Support**: Automated refactoring capabilities

#### 4. Automation Scripts
- **PowerShell Scripts**: Windows-native automation (`format-code.ps1`, `lint-code.ps1`)
- **Bash Scripts**: Linux/macOS automation (`format-code.sh`, `lint-code.sh`)
- **CI/CD Integration**: Automated formatting and linting in build pipeline
- **Pre-commit Hooks**: Optional pre-commit validation

## Technical Implementation

### clang-format Configuration
```yaml
# .clang-format key settings
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 160
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
```

### clang-tidy Configuration
```yaml
# .clang-tidy key checks
Checks: >
  modernize-*,
  performance-*,
  bugprone-*,
  readability-*,
  -modernize-use-trailing-return-type
```

### Automation Scripts
```bash
# Format all source files
./scripts/format-code.sh

# Run linting with error reporting
./scripts/lint-code.sh

# Windows PowerShell equivalents
.\scripts\format-code.ps1
.\scripts\lint-code.ps1
```

## Consequences
### Positive
- **Consistent Code Style**: Automated formatting ensures consistent style across all files
- **Early Bug Detection**: clang-tidy catches potential issues before they become problems
- **Improved Developer Experience**: IDE integration provides real-time feedback
- **Reduced Code Review Burden**: Style issues automatically handled
- **Better Code Quality**: Static analysis enforces best practices
- **Cross-Platform Consistency**: Same formatting rules on all platforms

### Negative
- **Learning Curve**: Developers need to learn clang-tidy warnings and fixes
- **Initial Formatting Effort**: Large codebase requires initial formatting pass
- **Tool Dependency**: Development now depends on clang toolchain
- **Potential Conflicts**: Automated formatting might conflict with developer preferences

### Risks
- **Tool Version Compatibility**: Different clang versions might format differently
- **Performance Impact**: Large codebases might be slow to format/lint
- **False Positives**: clang-tidy might flag acceptable code patterns
- **Integration Issues**: IDE integration might require configuration

## Implementation Statistics
- **Files Added**: 8 configuration and script files
- **Lines of Code**: 1,353 lines of configuration and automation
- **Code Formatted**: 365 lines in qbx.cpp as initial formatting example
- **Platform Support**: Windows (PowerShell), Linux/macOS (Bash)
- **IDE Integration**: clangd configuration for VS Code, CLion, etc.

## Usage Examples

### Command Line Usage
```bash
# Format all C++ files
find . -name "*.cpp" -o -name "*.h" | xargs clang-format -i

# Run clang-tidy checks
clang-tidy -checks='-*,modernize-*' source/file.cpp

# Use automation scripts
./scripts/format-code.sh --check  # Check formatting without modifying
./scripts/lint-code.sh --fix      # Auto-fix linting issues
```

### IDE Integration
```bash
# Install clangd for language server support
# VS Code: C/C++ extension with clangd
# CLion: Built-in clangd support
# Vim/Neovim: coc-clangd or vim-clangd
```

## Alternatives Considered
1. **Manual Style Guidelines**: Continue with manual style enforcement
   - Rejected: Too error-prone and inconsistent
2. **Different Formatting Tools**: Use astyle, uncrustify, or other formatters
   - Rejected: clang-format has best IDE integration and C++ support
3. **No Linting**: Focus only on formatting without static analysis
   - Rejected: Linting provides significant quality benefits
4. **Custom Scripts**: Develop custom formatting and linting tools
   - Rejected: Existing tools are mature and well-maintained

## Future Enhancements
- **Pre-commit Integration**: Git hooks for automatic formatting
- **CI/CD Enforcement**: Fail builds on formatting/linting violations
- **Custom Rules**: Project-specific clang-tidy rules
- **Performance Benchmarks**: Track formatting/linting performance
- **Editor Integration**: Enhanced integration with more editors

## References
- Commit: ee95a5c70 - "Add code formatting and linting infrastructure"
- Documentation: docs/CODE_FORMATTING_AND_LINTING.md
- Configuration: .clang-format, .clang-tidy, .clangd
- Scripts: scripts/format-code.sh, scripts/lint-code.sh

## Date
2026-01-11
