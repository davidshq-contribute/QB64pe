# QB64 Phoenix Edition - Function-Level Documentation Project

## Project Overview

This document outlines a comprehensive plan to add function-level documentation to all non-third-party code files in the QB64 Phoenix Edition repository. The goal is to improve code maintainability, developer onboarding, and overall code quality.

## Scope Definition

### Non-Third-Party Code Files

Based on the license analysis, the following directories contain QB64-PE's own code:

**Core QB64-PE Code (First-Party):**
- `source/` - Main QB64 compiler and IDE written in BASIC
- `internal/c/libqb/` - Core runtime library (C++)
- `internal/c/parts/` - QB64-specific C++ components
- `tests/` - Test files (excluding third-party test data)

**Third-Party Code (Excluded from Documentation):**
- `internal/c/parts/video/font/freetype/` - FreeTYPE library
- `internal/c/parts/video/image/stb/` - stb_image libraries
- `internal/c/parts/video/image/nanosvg/` - NanoSVG library
- `internal/c/parts/video/image/qoi/` - QOI library
- `internal/c/parts/video/image/sg_pcx/` - dr_pcx library
- `internal/c/parts/video/image/jo_gif/` - jo_gif library
- `internal/c/parts/video/image/pixelscalers/` - Various pixel scaler libraries
- `internal/c/parts/data/miniz.h` - miniz compression
- `internal/c/parts/data/modp_b64.h` - MODP_B64 encoding
- `internal/c/parts/audio/miniaudio/` - miniaudio library
- `internal/c/parts/audio/extras/` - All audio codec libraries
- `internal/c/parts/input/game_controller/` - libstem_gamepad
- `internal/c/parts/gui/` - tiny file dialogs
- `internal/c/parts/os/clipboard/clip/` - Clip library
- `internal/c/parts/network/http/curl/` - libcurl
- `internal/c/parts/core/glew/` - GLEW library
- `internal/c/parts/core/freeglut/` - FreeGLUT library

## Complete File Inventory

### BASIC Source Files (.bas)

#### Core Compiler/IDE
1. `source/qb64pe.bas` - Main compiler entry point (24,636 lines)
2. `source/ide/ide_global.bas` - IDE global variables and declarations
3. `source/ide/ide_methods.bas` - IDE core functionality and UI methods
4. `source/ide/ide_converters.bas` - IDE code conversion utilities
5. `source/ide/ide_export.bas` - IDE export functionality
6. `source/ide/config/cfg_global.bas` - Configuration system globals
7. `source/ide/config/cfg_methods.bas` - Configuration management methods
8. `source/ide/wiki/wiki_global.bas` - Wiki system globals
9. `source/ide/wiki/wiki_methods.bas` - Wiki functionality methods

#### Global Systems
10. `source/global/constants.bas` - Global constants
11. `source/global/constants_ide.bas` - IDE-specific constants
12. `source/global/settings.bas` - Global settings management
13. `source/global/version.bas` - Version information

#### Code Generation & Emission
14. `source/emit/logging.bas` - Code generation logging

#### Subroutines & Functions
15. `source/subs_functions/subs_functions.bas` - Core subroutine and function handling
16. `source/subs_functions/syntax_highlighter_list.bas` - Syntax highlighting definitions
17. `source/subs_functions/extensions/opengl/opengl_global.bas` - OpenGL extension globals
18. `source/subs_functions/extensions/opengl/opengl_methods.bas` - OpenGL extension methods

#### Utility Modules
19. `source/utilities/build.bas` - Build system utilities
20. `source/utilities/const_eval.bas` - Constant evaluation
21. `source/utilities/elements.bas` - Code element management
22. `source/utilities/file.bas` - File operations utilities
23. `source/utilities/format.bas` - Code formatting utilities
24. `source/utilities/give_error.bas` - Error reporting system
25. `source/utilities/hash.bas` - Hashing utilities
26. `source/utilities/hash_init.bas` - Hash system initialization
27. `source/utilities/include_provider.bas` - Include file management
28. `source/utilities/parser_utils.bas` - Parser utilities
29. `source/utilities/s-buffer/simplebuffer_init.bas` - Simple buffer initialization
30. `source/utilities/statevars.bas` - State variable management
31. `source/utilities/strings.bas` - String manipulation utilities
32. `source/utilities/terminal.bas` - Terminal handling
33. `source/utilities/type.bas` - Type system utilities
34. `source/utilities/type_init.bas` - Type system initialization

#### Internal Support Files
35. `internal/support/converter/AddPREFIX.bas` - Prefix addition utility
36. `internal/support/converter/QB45BIN.bas` - QB45 binary converter
37. `internal/support/include/aftermain.bas` - Post-main include handling

#### Test Files
38. `tests/basic_test.bas` - Basic functionality tests
39. `tests/compile_tests/*/` - Various compile test files (approximately 50+ files)

### BASIC Header Files (.bi)

#### Utility Headers
40. `source/utilities/const_eval.bi` - Constant evaluation declarations
41. `source/utilities/give_error.bi` - Error system declarations
42. `source/utilities/hash.bi` - Hash system declarations
43. `source/utilities/hash_declarations.bi` - Hash type declarations
44. `source/utilities/include_provider.bi` - Include provider declarations
45. `source/utilities/ini-manager/ini.bi` - INI file management declarations
46. `source/utilities/s-buffer/sb_qb64pe_extension.bi` - Simple buffer extensions
47. `source/utilities/s-buffer/simplebuffer.bi` - Simple buffer declarations
48. `source/utilities/s-buffer/simplebuffer_declarations.bi` - Simple buffer types
49. `source/utilities/statevars.bi` - State variable declarations
50. `source/utilities/type.bi` - Type system declarations
51. `source/utilities/type_declarations.bi` - Type system type definitions

#### Internal Support Headers
52. `internal/support/color/color0.bi` - Color system declarations (8-bit)
53. `internal/support/color/color32.bi` - Color system declarations (32-bit)
54. `internal/support/include/beforefirstline.bi` - Pre-include declarations
55. `internal/support/vwatch/vwatch.bi` - Variable watcher declarations

#### Test Headers
56. `tests/unit/*.bi` - Test framework declarations (approximately 10 files)
57. `tests/compile_tests/extra/*.bi` - Test support headers

### C++ Source Files (.cpp)

#### Core Runtime Library
58. `internal/c/libqb.cpp` - Main libqb entry point (31,114 lines)
59. `internal/c/libqb/src/bitops.cpp` - Bit operations
60. `internal/c/libqb/src/buffer.cpp` - Buffer management
61. `internal/c/libqb/src/command.cpp` - Command processing
62. `internal/c/libqb/src/console-only-main-thread.cpp` - Console-only main thread
63. `internal/c/libqb/src/datetime.cpp` - Date/time functions
64. `internal/c/libqb/src/environ.cpp` - Environment variables
65. `internal/c/libqb/src/error_handle.cpp` - Error handling
66. `internal/c/libqb/src/file-fields.cpp` - File field operations
67. `internal/c/libqb/src/filepath.cpp` - File path utilities
68. `internal/c/libqb/src/filesystem.cpp` - File system operations
69. `internal/c/libqb/src/gfs.cpp` - General file system
70. `internal/c/libqb/src/glut-main-thread.cpp` - GLUT main thread handling
71. `internal/c/libqb/src/glut-message.cpp` - GLUT message handling
72. `internal/c/libqb/src/glut-msg-queue.cpp` - GLUT message queue
73. `internal/c/libqb/src/graphics.cpp` - Graphics primitives
74. `internal/c/libqb/src/hexoctbin.cpp` - Hex/octal/binary conversion
75. `internal/c/libqb/src/http-stub.cpp` - HTTP functionality stub
76. `internal/c/libqb/src/http.cpp` - HTTP client functionality
77. `internal/c/libqb/src/mac-mouse-support.cpp` - macOS mouse support
78. `internal/c/libqb/src/mem.cpp` - Memory management
79. `internal/c/libqb/src/qblist.cpp` - QB64 list implementation
80. `internal/c/libqb/src/qbs.cpp` - QB64 string implementation
81. `internal/c/libqb/src/qbs__tostr.cpp` - String to string conversion
82. `internal/c/libqb/src/qbs_cmem.cpp` - String memory management
83. `internal/c/libqb/src/qbs_mk_cv.cpp` - String make/convert
84. `internal/c/libqb/src/qbs_str.cpp` - String operations
85. `internal/c/libqb/src/qbs_val.cpp` - String value operations
86. `internal/c/libqb/src/shell.cpp` - Shell command execution
87. `internal/c/libqb/src/string_functions.cpp` - String utility functions
88. `internal/c/libqb/src/threading-posix.cpp` - POSIX threading
89. `internal/c/libqb/src/threading-windows.cpp` - Windows threading
90. `internal/c/libqb/src/threading.cpp` - Threading abstraction

#### Logging System
91. `internal/c/libqb/src/logging/logging.cpp` - Core logging system
92. `internal/c/libqb/src/logging/handlers/fp_handler.cpp` - File pointer log handler
93. `internal/c/libqb/src/logging/mingw/file.cpp` - MinGW file logging
94. `internal/c/libqb/src/logging/mingw/pe.cpp` - PE file logging
95. `internal/c/libqb/src/logging/mingw/pe_symtab.cpp` - PE symbol table logging
96. `internal/c/libqb/src/logging/qb64pe_symbol.cpp` - QB64-PE symbol logging
97. `internal/c/libqb/src/logging/stacktrace.cpp` - Stack trace logging
98. `internal/c/libqb/src/logging/unix/symbol.cpp` - Unix symbol logging

#### QB64-Specific Components
99. `internal/c/parts/audio/audio.cpp` - Audio system core
100. `internal/c/parts/core/gl_header_for_parsing/temp/gl_kit.bas` - OpenGL parsing utilities

### C++ Header Files (.h)

#### Core Headers
101. `internal/c/libqb.h` - Main libqb header
102. `internal/c/common.h` - Common definitions
103. `internal/c/os.h` - OS abstraction header

#### Component Headers
104. `internal/c/parts/audio/framework.h` - Audio framework
105. `internal/c/parts/core/gl_header_for_parsing/gl.h` - OpenGL parsing header
106. `internal/c/parts/core/gl_header_for_parsing/temp/gl_helper_code.h` - OpenGL helper code

#### Logging Headers
107. `internal/c/libqb/src/logging/logging_private.h` - Private logging definitions
108. `internal/c/libqb/src/glut-message.h` - GLUT message definitions

#### Platform-Specific Headers
109. `internal/c/libqb/src/mac-key-monitor.h` - macOS key monitoring

## Recommended Documentation Approach Order

### Phase 1: Foundation Systems (Week 1-2)
**Priority: HIGH** - These files contain core abstractions used throughout the codebase.

1. **Core Types and Globals** 
   - `source/global/constants.bas`
   - `source/global/constants_ide.bas`
   - `source/global/settings.bas`
   - `source/global/version.bas`

2. **Core Data Structures**
   - `source/utilities/statevars.bas`
   - `source/utilities/type.bas`
   - `source/utilities/type_init.bas`
   - `source/utilities/hash.bas`
   - `source/utilities/hash_init.bas`

3. **Error and Logging Systems**
   - `source/utilities/give_error.bas`
   - `source/emit/logging.bas`
   - `internal/c/libqb/src/logging/logging.cpp`

### Phase 2: Core Compiler Infrastructure (Week 3-4)
**Priority: HIGH** - Main compilation pipeline components.

4. **Main Compiler Entry**
   - `source/qb64pe.bas` (Focus on main functions first)

5. **Parsing and Utilities**
   - `source/utilities/parser_utils.bas`
   - `source/utilities/elements.bas`
   - `source/utilities/const_eval.bas`
   - `source/utilities/strings.bas`

6. **File and Include Management**
   - `source/utilities/file.bas`
   - `source/utilities/include_provider.bas`

### Phase 3: Runtime Library Core (Week 5-6)
**Priority: HIGH** - Essential runtime functionality.

7. **Main Runtime Library**
   - `internal/c/libqb.cpp` (Focus on core functions)
   - `internal/c/libqb.h`
   - `internal/c/common.h`

8. **Memory and String Management**
   - `internal/c/libqb/src/mem.cpp`
   - `internal/c/libqb/src/qbs.cpp`
   - `internal/c/libqb/src/qbs_*.cpp` (All qbs variants)

9. **Threading and Platform**
   - `internal/c/libqb/src/threading.cpp`
   - `internal/c/libqb/src/threading-posix.cpp`
   - `internal/c/libqb/src/threading-windows.cpp`

### Phase 4: IDE and User Interface (Week 7-8)
**Priority: MEDIUM** - Development environment components.

10. **IDE Core**
    - `source/ide/ide_global.bas`
    - `source/ide/ide_methods.bas`
    - `source/ide/config/cfg_global.bas`
    - `source/ide/config/cfg_methods.bas`

11. **IDE Supporting Systems**
    - `source/ide/ide_converters.bas`
    - `source/ide/ide_export.bas`
    - `source/ide/wiki/wiki_global.bas`
    - `source/ide/wiki/wiki_methods.bas`

### Phase 5: Language Features (Week 9-10)
**Priority: MEDIUM** - Specific language feature implementations.

12. **Subroutines and Functions**
    - `source/subs_functions/subs_functions.bas`
    - `source/subs_functions/syntax_highlighter_list.bas`

13. **Extensions**
    - `source/subs_functions/extensions/opengl/opengl_global.bas`
    - `source/subs_functions/extensions/opengl/opengl_methods.bas`

### Phase 6: Systems Integration (Week 11-12)
**Priority: MEDIUM** - System-level integration components.

14. **File System and I/O**
    - `internal/c/libqb/src/filesystem.cpp`
    - `internal/c/libqb/src/filepath.cpp`
    - `internal/c/libqb/src/file-fields.cpp`

15. **Graphics and Display**
    - `internal/c/libqb/src/graphics.cpp`
    - `internal/c/libqb/src/glut-main-thread.cpp`
    - `internal/c/libqb/src/glut-message.cpp`

16. **Audio System**
    - `internal/c/parts/audio/audio.cpp`
    - `internal/c/parts/audio/framework.h`

### Phase 7: Utilities and Support (Week 13-14)
**Priority: LOW** - Supporting utilities and tools.

17. **Remaining Utilities**
    - `source/utilities/build.bas`
    - `source/utilities/format.bas`
    - `source/utilities/terminal.bas`
    - `source/utilities/buffer.bas`
    - `source/utilities/s-buffer/simplebuffer_init.bas`

18. **Command and Shell**
    - `internal/c/libqb/src/command.cpp`
    - `internal/c/libqb/src/shell.cpp`

19. **Math and Conversion**
    - `internal/c/libqb/src/hexoctbin.cpp`
    - `internal/c/libqb/src/string_functions.cpp`

### Phase 8: Testing and Validation (Week 15-16)
**Priority: LOW** - Test infrastructure and validation.

20. **Test Framework**
    - `tests/basic_test.bas`
    - All test framework headers in `tests/unit/`

21. **Compile Tests**
    - All compile test files (document test purposes)

## Documentation Standards

### Function Documentation Template

```basic
'======================================================================
' FUNCTION/SUB: [Name]
'======================================================================
' PURPOSE: Brief description of what the function does
' PARAMETERS:
'   paramName (type) - Description of parameter
'   paramName (type) - Description of parameter
' RETURNS: Description of return value (for functions)
' NOTES: 
'   - Important implementation details
'   - Dependencies on other functions
'   - Platform-specific behavior
' EXAMPLE:
'   result = functionName(param1, param2)
'======================================================================
```

### C++ Documentation Template

```cpp
//======================================================================
// Function: [Name]
//======================================================================
// Purpose: Brief description of what the function does
// Parameters:
//   paramType paramName - Description of parameter
//   paramType paramName - Description of parameter
// Returns: Description of return value
// Notes:
//   - Important implementation details
//   - Thread safety considerations
//   - Memory management responsibilities
//   - Platform-specific behavior
//======================================================================
```

## Implementation Guidelines

### Documentation Quality Standards
1. **Clarity**: Use clear, concise language
2. **Completeness**: Document all parameters, return values, and side effects
3. **Consistency**: Use consistent formatting throughout
4. **Accuracy**: Ensure documentation matches actual implementation
5. **Examples**: Provide usage examples where helpful

### Process Guidelines
1. **Read First**: Always read the entire file before documenting
2. **Understand Context**: Understand how the function fits into the larger system
3. **Update Existing**: Improve existing documentation rather than replacing it entirely
4. **Cross-Reference**: Reference related functions and modules
5. **Version Control**: Commit documentation changes separately when possible

### Tools and Automation
1. **Validation**: Consider using documentation validation tools
2. **Generation**: Explore auto-generation of function lists
3. **Consistency**: Use scripts to check documentation consistency
4. **Coverage**: Track documentation coverage metrics

## Success Metrics

### Quantitative Metrics
- **Documentation Coverage**: % of functions with documentation
- **Documentation Quality**: Average documentation score (1-10)
- **Completion Rate**: % of files completed per phase

### Qualitative Metrics
- **Developer Feedback**: Ease of understanding documented code
- **Onboarding Time**: Time for new developers to become productive
- **Maintenance Efficiency**: Reduced time for bug fixes and feature additions

## Risk Mitigation

### Potential Challenges
1. **Large Codebase**: 100+ files with varying complexity
2. **Time Investment**: Significant documentation effort required
3. **Consistency**: Maintaining consistent documentation style
4. **Keeping Current**: Documentation may become outdated

### Mitigation Strategies
1. **Phased Approach**: Break into manageable phases
2. **Template Usage**: Standardized templates for consistency
3. **Regular Reviews**: Periodic documentation audits
4. **Automation**: Tools to validate and maintain documentation

## Next Steps

1. **Review and Approve**: Review this plan with the development team
2. **Set Up Tools**: Prepare documentation templates and validation tools
3. **Begin Phase 1**: Start with foundation systems
4. **Regular Progress Reviews**: Weekly progress check-ins
5. **Adjust as Needed**: Modify approach based on lessons learned

---

**Estimated Timeline**: 16 weeks (4 months)
**Estimated Effort**: 200-300 hours of documentation work
**Team Size**: 1-2 documentation specialists
**Review Cadence**: Weekly progress reviews
