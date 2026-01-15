# Contributing to QB64 Phoenix Edition

Thank you for your interest in contributing to QB64 Phoenix Edition! This document provides guidelines and information to help you contribute effectively to the project.

## Table of Contents

1. [Getting Started](#getting-started)
2. [Development Setup](#development-setup)
3. [Contribution Process](#contribution-process)
4. [Coding Standards](#coding-standards)
5. [Testing](#testing)
6. [Review Criteria](#review-criteria)
7. [Community Guidelines](#community-guidelines)

## Getting Started

### Prerequisites

- Git
- C++ compiler (GCC, Clang, or MSVC)
- Basic understanding of BASIC programming language
- Familiarity with C/C++ (for core contributions)

### Project Structure

- `source/` - BASIC source code and IDE
- `internal/c/` - C++ runtime and libqb core
- `internal/source/` - Generated C++ code from BASIC
- `tests/` - Test suites and test programs
- `docs/` - Documentation and project planning

## Development Setup

### Building from Source

1. Clone the repository:
   ```bash
   git clone https://github.com/QB64-Phoenix-Edition/QB64pe.git
   cd QB64pe
   ```

2. Build for your platform:
   - **Windows**: Run `setup_win.cmd`
   - **Linux**: Run `./setup_lnx.sh`
   - **macOS**: Run `./setup_osx.command`

3. Verify the build:
   ```bash
   ./qb64pe -c tests/compile_tests/hello_world.bas
   ```

### Development Tools

- **Code Formatting**: Use the provided `.clang-format` for C++ code
- **Testing**: Run test suites with `make test` or platform-specific scripts
- **Debugging**: Use `-d` flag for debug builds

## Contribution Process

### 1. Fork and Clone

1. Fork the repository on GitHub
2. Clone your fork locally
3. Add the upstream repository:
   ```bash
   git remote add upstream https://github.com/QB64-Phoenix-Edition/QB64pe.git
   ```

### 2. Create a Branch

Use descriptive branch names:
```bash
git checkout -b feature/add-new-function
git checkout -b fix/memory-leak-issue
git checkout -b docs/update-api-documentation
```

### 3. Make Changes

- Follow the coding standards outlined below
- Write tests for new functionality
- Update documentation as needed
- Keep changes focused and minimal

### 4. Test Your Changes

- Run existing tests to ensure no regressions
- Add new tests for your changes
- Test on multiple platforms if possible

### 5. Submit a Pull Request

1. Push your branch to your fork
2. Create a pull request with:
   - Clear title and description
   - Reference to related issues
   - Testing instructions
   - Screenshots if applicable

### 6. Code Review

- Address reviewer feedback promptly
- Keep discussions constructive and focused
- Update your branch as needed

## Coding Standards

### C++ Code

#### Formatting

- Use the provided `.clang-format` configuration
- 4-space indentation (no tabs)
- 160-character line limit
- LLVM-based style with modifications

#### Naming Conventions

- **Functions**: `snake_case` (e.g., `load_image_data`)
- **Variables**: `snake_case` (e.g., `image_width`)
- **Constants**: `UPPER_SNAKE_CASE` (e.g., `MAX_BUFFER_SIZE`)
- **Classes**: `PascalCase` (e.g., `ImageLoader`)
- **Member variables**: `snake_case_` (e.g., `width_`)

#### Documentation

```cpp
/**
 * Brief description of the function
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 * @throws ExceptionType Description of when exception is thrown
 */
int calculateSum(int param1, int param2);
```

#### Error Handling

- Use appropriate error codes and return values
- Log errors with descriptive messages
- Clean up resources in error paths
- Avoid memory leaks

### BASIC Code

#### Style Guidelines

- Use descriptive variable and function names
- Add comments for complex logic
- Follow QB64 naming conventions
- Maintain backward compatibility

#### Function Documentation

```basic
' Calculates the factorial of a number
' @param n The number to calculate factorial for
' @return The factorial result
FUNCTION factorial& (n AS LONG)
```

## Testing

### Test Categories

1. **Unit Tests**: Test individual functions and modules
2. **Integration Tests**: Test component interactions
3. **Regression Tests**: Ensure fixes don't break existing functionality
4. **Platform Tests**: Verify cross-platform compatibility

### Running Tests

```bash
# Run all tests
make test

# Run specific test category
make test-unit
make test-integration

# Run tests on specific platform
bash tests/run_tests.sh
```

### Writing Tests

- Place tests in appropriate `tests/` subdirectory
- Use descriptive test names
- Test both success and failure cases
- Include setup and cleanup code

## Review Criteria

### Code Quality

- **Correctness**: Code works as intended
- **Efficiency**: No unnecessary complexity or performance issues
- **Maintainability**: Clear, readable, and well-structured
- **Documentation**: Adequate comments and documentation

### Technical Requirements

- **No Regressions**: Existing functionality remains intact
- **Cross-Platform**: Works on supported platforms (Windows, Linux, macOS)
- **Memory Safety**: No memory leaks or buffer overflows
- **Thread Safety**: Safe for multi-threaded environments where applicable

### Project Standards

- **Coding Standards**: Follows project formatting and naming conventions
- **Testing**: Includes appropriate tests for new functionality
- **Documentation**: Updates relevant documentation
- **License**: Compatible with project licensing (MIT for core code)

### Review Process

1. **Automated Checks**: CI/CD pipeline runs tests and formatting checks
2. **Peer Review**: At least one maintainer reviews the changes
3. **Testing**: Changes are tested on multiple platforms
4. **Integration**: Verified that changes integrate properly

## Community Guidelines

### Communication

- Be respectful and constructive in all interactions
- Welcome newcomers and help them learn
- Focus on what is best for the community
- Show empathy toward other community members

### Issue Reporting

- Use the provided issue templates
- Search existing issues before creating new ones
- Provide clear, reproducible bug reports
- Include system information and error details

### Feature Requests

- Explain the problem you're trying to solve
- Describe your proposed solution
- Consider alternative approaches
- Be open to feedback and suggestions

## Getting Help

- **Forum**: [QB64 Phoenix Edition Forum](https://qb64phoenix.com/forum)
- **Discord**: [QB64 PE Discord Server](https://discord.gg/D2M7hepTSx)
- **Wiki**: [QB64 Wiki](https://qb64phoenix.com/qb64wiki)
- **GitHub Issues**: For bug reports and feature requests

## License

By contributing to QB64 Phoenix Edition, you agree that your contributions will be licensed under the same license as the project (MIT for core code, with various licenses for third-party dependencies).

## Recognition

Contributors are recognized in:
- Release notes
- Contributor documentation
- Project acknowledgments

Thank you for contributing to QB64 Phoenix Edition!
