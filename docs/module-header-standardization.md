# Module Header Standardization

## Overview

All libqb module headers have been standardized to follow a consistent structure and format. This improves maintainability, readability, and makes it easier for contributors to understand the codebase organization.

## Standard Header Template

Every module header should follow this structure:

```cpp
//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE [Module Name] Module
//  [Brief description of module purpose]
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_[MODULE_NAME]_H
#define INCLUDE_LIBQB_[MODULE_NAME]_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>  // Standard integer types
// Add other system headers here

// Forward declarations
struct qbs;
struct byte_element_struct;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// [Group 1: Primary functionality]
// Function declarations with brief comments describing purpose

// [Group 2: Secondary functionality]
// Related function declarations

// [Group 3: Internal helpers exposed for other modules]
// Functions needed by other modules but not part of public API

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// Any important notes about implementation, dependencies, or future plans
// Include TODO items for functions still in libqb.cpp

#endif // INCLUDE_LIBQB_[MODULE_NAME]_H
```

## Key Standardization Rules

### 1. Header Format
- **Use include guards** (`#ifndef`/`#define`/`#endif`) instead of `#pragma once`
- **Include guard naming**: `INCLUDE_LIBQB_[MODULE_NAME]_H`
- **ASCII art banner**: Consistent QB64-PE branding in all headers
- **Module description**: Clear, concise description of purpose

### 2. Section Organization
- **Dependencies**: All includes and forward declarations
- **Public API**: Function declarations organized by functionality
- **Implementation Notes**: Documentation and TODO items

### 3. Naming Conventions
- **File names**: Lowercase with hyphens (e.g., `fileio.h`, `color.h`)
- **Include guards**: Uppercase with underscores
- **Function grouping**: Logical grouping with descriptive comments

### 4. Documentation
- **Module purpose**: Brief description in header banner
- **Function groups**: Descriptive comments for each functional group
- **Implementation notes**: Detailed module documentation at the end

## Standardized Modules

The following modules have been standardized:

### Completed Modules
- `color.h` - Color & Palette Module
- `screen.h` - Screen Management Module  
- `fileio.h` - File I/O Module
- `audio.h` - Audio Engine Module
- `mem.h` - Memory Management Module

### Key Changes Made

1. **Consistent Headers**: All modules now use the same header format
2. **Include Guards**: Replaced `#pragma once` with standard include guards
3. **Section Organization**: Clear separation of dependencies, API, and documentation
4. **Function Grouping**: Logical organization with descriptive comments
5. **Implementation Notes**: Added comprehensive module documentation

## Benefits

1. **Improved Readability**: Consistent structure makes headers easier to scan
2. **Better Documentation**: Clear module descriptions and implementation notes
3. **Easier Maintenance**: Standardized format reduces cognitive load
4. **Contributor Friendly**: Clear template for new modules
5. **Professional Appearance**: Consistent branding and formatting

## Future Work

- Apply standardization to remaining module headers
- Create automated validation for header compliance
- Update module interface documentation to reflect new structure
- Consider adding Doxygen-compatible documentation format

## Template Usage

When creating new modules:
1. Copy the template from `internal/c/libqb/include/module-template.h`
2. Replace `[MODULE_NAME]` with the actual module name
3. Add module-specific description in header banner
4. Organize functions into logical groups
5. Add comprehensive implementation notes

This ensures consistency across all libqb modules and maintains the professional appearance of the codebase.
