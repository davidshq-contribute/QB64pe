# QB64 Formatting and Linting Extraction Proposal

## Overview

This document outlines a proposal to extract QB64 IDE's built-in formatting and linting functionality into a standalone application that can be used independently and integrated back into the IDE as a modular component.

## Current State Analysis

### Built-in Formatting Features

The QB64 IDE currently includes comprehensive formatting capabilities:

#### Auto-Layout System (`IDEAutoLayout`)
- **Location**: `source/utilities/format.bas`, `source/ide/config/cfg_methods.bas`
- **Function**: `apply_layout_indent$()`
- **Features**:
  - Automatic spacing around operators and keywords
  - Single-spacing of code elements
  - Keyword case conversion (UPPERCASE, lowercase, CaMeLCase)
  - String quote completion
  - Question mark to PRINT conversion (`?` → `PRINT`)

#### Auto-Indent System (`IDEAutoIndent`)
- **Configuration**: `IDEAutoIndentSize` (1-64 spaces)
- **Features**:
  - Structure-based indentation
  - SUB/FUNCTION indentation (`IDEIndentSubs`)
  - Smart indent level detection
  - Tab-to-space conversion

#### Keyword Case Management
- **Styles**: UPPERCASE, lowercase, CaMeLCase
- **Function**: `SCase$()`, `SCase2$()`
- **Configuration**: `IDEAutoLayoutKwStyle`

### Built-in Linting Features

#### Warning System
- **Location**: `source/qb64pe.bas` lines 24476-24539
- **Function**: `addWarning()`
- **Features**:
  - Duplicate constant detection
  - Invalid metacommand validation
  - Version info field validation
  - Include file tracking
  - Line number and file context

#### Warning Categories
1. **Duplicate Constants**: Same constant name with different values
2. **Metacommand Issues**: Invalid `$DEBUG`, `$CHECKING:OFF` usage
3. **Version Info**: Missing or invalid `$VERSIONINFO` fields
4. **Include File Problems**: Issues with `$INCLUDE` statements

## Proposed Standalone Application

### Architecture

```
qb64-formatter/
├── src/
│   ├── formatter.bas          # Core formatting engine
│   ├── linter.bas             # Linting and validation
│   ├── config.bas             # Configuration management
│   ├── parser.bas             # BASIC code parsing
│   └── output.bas             # Result formatting
├── config/
│   ├── default.ini             # Default formatting rules
│   └── schemas/                # Configuration schemas
├── tests/
│   ├── test_formatter.bas     # Formatting tests
│   └── test_linter.bas         # Linting tests
└── tools/
    ├── cli.bas                 # Command-line interface
    └── ide-integration.bas     # IDE integration layer
```

### Core Components

#### 1. Formatter Engine (`formatter.bas`)

**Extracted Functions**:
- `apply_layout_indent$()` → `FormatLine$()`
- `SCase$()`, `SCase2$()` → `FormatKeyword$()`
- `converttabs$()` → `ConvertTabs$()`

**New Features**:
- Batch file processing
- Multiple output formats
- Plugin system for custom rules

#### 2. Linter Engine (`linter.bas`)

**Extracted Functions**:
- `addWarning()` → `AddIssue()`
- Warning detection logic
- Validation routines

**Enhanced Features**:
- Configurable rule sets
- Custom rule definitions
- Severity levels (error, warning, info)
- Auto-fix suggestions

#### 3. Configuration System (`config.bas`)

**Settings**:
```ini
[Formatting]
AutoLayout=true
AutoIndent=true
IndentSize=4
IndentSubs=true
KeywordCase=camel

[Linting]
EnableWarnings=true
WarningLevel=standard
IgnoreRules=duplicate_const

[Output]
FormatInPlace=false
BackupOriginal=true
OutputFormat=diff
```

#### 4. Command-Line Interface

```bash
# Format files
qb64-formatter format file.bas --indent-size=4 --keyword-case=upper

# Lint files
qb64-formatter lint file.bas --warnings=all --output=json

# Both operations
qb64-formatter process file.bas --format --lint --fix

# Configuration
qb64-formatter config --set Formatting.IndentSize=2
qb64-formatter config --show
```

### Integration with IDE

#### 1. Library Integration
- Compile formatter as standalone library
- IDE calls formatter functions directly
- Real-time formatting as user types
- Shared configuration system

#### 2. Process Integration
- IDE spawns formatter process for batch operations
- Communication via stdin/stdout or temporary files
- Non-blocking operation for large files

#### 3. Configuration Synchronization
- IDE and formatter share configuration files
- Real-time setting updates
- Per-project configurations

## Implementation Plan

### Phase 1: Core Extraction
1. **Extract formatting functions** from `source/utilities/format.bas`
2. **Extract linting functions** from `source/qb64pe.bas`
3. **Create configuration system** based on IDE settings
4. **Implement basic CLI** interface

### Phase 2: Enhancement
1. **Add batch processing** capabilities
2. **Implement plugin system** for custom rules
3. **Create comprehensive test suite**
4. **Add multiple output formats**

### Phase 3: IDE Integration
1. **Modify IDE** to use external formatter
2. **Implement real-time integration**
3. **Add configuration UI** for formatter settings
4. **Performance optimization**

### Phase 4: Advanced Features
1. **Language Server Protocol** implementation
2. **IDE plugin development** (VS Code, etc.)
3. **Cloud-based formatting** service
4. **AI-assisted formatting** suggestions

## Benefits

### For Developers
- **Consistent formatting** across IDE and command line
- **Automated linting** in CI/CD pipelines
- **Customizable rules** per project
- **Faster IDE** (less built-in processing)

### For Project
- **Modular architecture** easier to maintain
- **Separate testing** of formatting logic
- **Community contributions** to formatter
- **Cross-platform compatibility**

### For Users
- **Choice of tools** (IDE vs. CLI)
- **Integration with editors** beyond QB64 IDE
- **Automated workflows** (pre-commit hooks, CI)
- **Consistent code style** across teams

## Technical Considerations

### Dependencies
- **QB64 runtime** for BASIC execution
- **Configuration files** (INI format)
- **File I/O operations**
- **String processing** libraries

### Performance
- **Line-by-line processing** for large files
- **Memory-efficient** parsing
- **Parallel processing** for multiple files
- **Caching** of parsed results

### Compatibility
- **Backward compatibility** with existing IDE settings
- **Cross-platform** (Windows, Linux, macOS)
- **QB64 version** compatibility
- **File encoding** support (UTF-8, codepages)

## Migration Strategy

### Step 1: Parallel Development
- Develop standalone formatter alongside IDE
- Maintain existing IDE functionality
- Test compatibility and performance

### Step 2: Gradual Migration
- Replace IDE functions one by one
- Add configuration options to choose implementation
- Gather user feedback

### Step 3: Full Replacement
- Remove built-in formatting from IDE
- Make formatter dependency for IDE
- Update documentation and tutorials

## Conclusion

Extracting the formatting and linting functionality from QB64 IDE into a standalone application offers significant benefits for maintainability, extensibility, and user experience. The modular approach allows for independent development, testing, and community contributions while maintaining seamless integration with the existing IDE.

The proposed architecture leverages the existing robust codebase while adding modern features like CLI interfaces, configuration management, and extensibility. This approach positions QB64's formatting capabilities to compete with modern language tooling while preserving the unique characteristics of BASIC code formatting.
