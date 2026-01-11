# QB64 Wiki Guide - Advanced Topics

This guide covers important QB64 topics beyond the fundamentals, compiled from the QB64 Phoenix Edition Wiki.

## Table of Contents

1. [Main Program Code Structure](#main-program-code-structure)
2. [Includes and Code Organization](#includes-and-code-organization)
3. [Advanced Data Types](#advanced-data-types)
4. [Metacommands](#metacommands)
5. [Memory Management](#memory-management)
6. [Graphics and Images](#graphics-and-images)
7. [Audio System](#audio-system)
8. [Networking and TCP/IP](#networking-and-tcpip)
9. [External Libraries and DLLs](#external-libraries-and-dlls)
10. [Console Windows](#console-windows)
11. [File Operations](#file-operations)
12. [String Manipulation](#string-manipulation)
13. [Mathematical Functions](#mathematical-functions)
14. [Bitwise Operations](#bitwise-operations)
15. [Error Handling and Debugging](#error-handling-and-debugging)
16. [Cross-Platform Considerations](#cross-platform-considerations)
17. [OpenGL Integration](#opengl-integration)

---

## Main Program Code Structure

QB64 has a unique program structure that differs from languages with explicit `MAIN` functions. Understanding this structure is crucial to avoid common issues.

### QB64 Program Structure

Unlike languages like C/C++ or Python, QB64 does **not** use an explicit `MAIN` function. Instead:

1. **Main Program Code**: Code at the top level (before any SUB/FUNCTION definitions) executes first
2. **SUB/FUNCTION Definitions**: Procedures are defined after the main code
3. **Implicit END**: An implicit `END` is automatically injected before the first SUB/FUNCTION

### Program Flow

```
Program Start
    ↓
AtTop Auto-Includes (constants, types, declarations)
    ↓
Your Main Program Code (executes first)
    ↓
AfterMain Auto-Includes (DATA, GOSUB, error handlers)
    ↓
Implicit END (automatically injected)
    ↓
SUB/FUNCTION Definitions (available to be called)
    ↓
AtBottom Auto-Includes (support code)
```

### Key Concepts

#### Main Program Code
- **Definition**: All code before the first SUB/FUNCTION definition
- **Execution**: Runs automatically when program starts
- **Scope**: Global scope - can access SHARED variables
- **Ends When**: First SUB/FUNCTION is encountered OR end of file

#### Implicit END Injection
QB64 automatically injects an `END` statement:
- **Location**: Right before the first SUB/FUNCTION definition
- **Purpose**: Separates main program code from procedure definitions
- **Effect**: Main code cannot continue executing into SUB/FUNCTION definitions

### Common Issues and Solutions

#### Issue 1: Code After SUB/FUNCTION Doesn't Execute

**Problem**: Code placed after SUB/FUNCTION definitions never runs because of implicit END.

```basic
' This code runs
PRINT "This executes"

SUB MySub
    PRINT "In sub"
END SUB

' This code NEVER runs!
PRINT "This never executes"
```

**Solution**: Put all executable code before SUB/FUNCTION definitions, or call it from within procedures.

#### Issue 2: Forward Declaration Confusion

**Problem**: Trying to call SUB/FUNCTION before it's defined in main code.

```basic
' Main code
MySub  ' ERROR: SUB not yet defined

SUB MySub
    PRINT "Hello"
END SUB
```

**Solution**: Use `DECLARE SUB` for forward declarations:

```basic
' Forward declaration
DECLARE SUB MySub

' Main code
MySub  ' OK: Forward declared

SUB MySub
    PRINT "Hello"
END SUB
```

#### Issue 3: Auto-Include Restrictions

**Problem**: SUB/FUNCTION definitions in AtTop or AfterMain auto-include files cause errors.

**Restrictions**:
- **AtTop files** (`.bi`): Can only contain CONST, TYPE, DIM [SHARED], DECLARE
- **AfterMain files** (`.bas`): Can only contain DATA, GOSUB routines, error handlers
- **AtBottom files** (`.bm`): Can contain SUB/FUNCTION definitions

**Solution**: Follow the restrictions - use `.bm` files for procedures in libraries.

#### Issue 4: Main Code in Included Files

**Problem**: Main program code in `$INCLUDE`d files can cause unexpected behavior.

**Behavior**:
- Main code in included files executes when included
- If included before first SUB/FUNCTION, it's part of main program
- If included after, it may not execute (due to implicit END)

**Solution**: 
- Put main code in the main program file
- Use SUB/FUNCTION in included files for reusable code
- Use `$INCLUDE` for declarations, not executable code

#### Issue 5: Variable Scope in Main vs SUB/FUNCTION

**Problem**: Variables declared in main code are not automatically accessible in SUB/FUNCTION.

**Solution**: Use `SHARED` keyword:

```basic
' Main code
DIM SHARED myVar AS INTEGER
myVar = 10

SUB MySub
    ' Can access myVar because it's SHARED
    PRINT myVar
END SUB
```

**Note**: Variables in SUB/FUNCTION are local by default. Use `SHARED` to access main scope variables.

#### Issue 6: DATA Statement Placement

**Problem**: DATA statements must be in main program scope, not inside SUB/FUNCTION.

**Correct**:
```basic
' Main code
DATA 1, 2, 3
DATA 4, 5, 6

SUB MySub
    READ x, y, z  ' Can READ from main scope DATA
END SUB
```

**Incorrect**:
```basic
SUB MySub
    DATA 1, 2, 3  ' ERROR: DATA not allowed in SUB
END SUB
```

#### Issue 7: GOSUB/RETURN in Main Code

**Problem**: GOSUB labels must be in main program scope.

**Correct**:
```basic
' Main code
GOSUB MyLabel
END

MyLabel:
    PRINT "In GOSUB"
    RETURN

SUB MySub
    ' SUB definitions after main code
END SUB
```

**Note**: GOSUB cannot jump into SUB/FUNCTION definitions.

### Auto-Include System

QB64-PE automatically includes support files at three positions:

#### AtTop (Before First Line)
- **Files**: `beforefirstline.bi`, color constants, library AtTop files
- **Content**: CONST, TYPE, DIM [SHARED], DECLARE statements only
- **Restriction**: No SUB/FUNCTION definitions
- **State Variable**: `firstLine` (0=inactive, 1=triggered, 2=in progress, 3=done)
- **Trigger**: Automatically triggered at start of program compilation

#### AfterMain (Before First SUB/FUNCTION)
- **Files**: `aftermain.bas`, library AfterMain files
- **Content**: DATA statements, GOSUB routines, error handlers
- **Restriction**: No SUB/FUNCTION definitions
- **Special**: Implicit END is injected here
- **State Variable**: `mainEndLine` (0=inactive, 1=triggered, 2=in progress, 3=done)
- **Trigger**: Automatically triggered when first SUB/FUNCTION is detected OR when end of file is reached

#### AtBottom (After Last Line)
- **Files**: `afterlastline.bm`, `vwatch.bm`, library AtBottom files
- **Content**: SUB/FUNCTION definitions, support code
- **No Restrictions**: Can contain any code
- **State Variable**: `lastLine` (0=inactive, 1=triggered, 2=in progress, 3=done)
- **Trigger**: Automatically triggered when end of source file is reached

#### State Variable Lifecycle

The compiler uses state variables to track auto-include processing:

1. **Triggered (1)**: Position detected, ready to process auto-includes
2. **In Progress (2)**: Auto-include files are being processed
3. **Done (3)**: Auto-includes completed for this position

This ensures auto-includes happen at the correct positions and prevents duplicate processing.

### Best Practices

1. **Structure Your Code**:
   ```basic
   ' 1. Constants and types
   CONST PI = 3.14159
   TYPE MyType
       x AS INTEGER
   END TYPE
   
   ' 2. Variable declarations
   DIM SHARED myVar AS INTEGER
   
   ' 3. Forward declarations (if needed)
   DECLARE SUB MySub
   
   ' 4. Main program code
   myVar = 10
   MySub
   END
   
   ' 5. SUB/FUNCTION definitions
   SUB MySub
       PRINT myVar
   END SUB
   ```

2. **Use Explicit END**: Even though implicit END exists, using explicit `END` makes code clearer

3. **Avoid Main Code in Includes**: Keep executable main code in the main file

4. **Understand Auto-Includes**: Know what gets included where to avoid conflicts

5. **Use SHARED for Global Access**: Explicitly declare SHARED variables for clarity

### Main Program Detection Internals

The compiler uses state variables to detect main program structure:

**Detection Logic:**
- **firstLine**: Triggered at start of compilation (state 1), processes AtTop auto-includes
- **mainEndLine**: Triggered when first SUB/FUNCTION is detected OR when end of file is reached (state 1), processes AfterMain auto-includes
- **lastLine**: Triggered when end of source file is reached (state 1), processes AtBottom auto-includes

**Detection Conditions:**
- Main end is detected when:
  1. First `SUB` or `FUNCTION` keyword is found at ExecLevel 0 (not inside `$IF` blocks)
  2. Not inside `DECLARE LIBRARY` blocks
  3. `mainEndLine` hasn't been triggered yet
- Detection only happens during prepass (preprocessor phase)

**State Transitions:**
- `0` = inactive (not yet triggered)
- `1` = triggered (position detected, ready to process)
- `2` = in progress (auto-includes being processed)
- `3` = done (auto-includes completed)

### Debugging Main Code Issues

- **Check for Implicit END**: Remember that END is injected before first SUB/FUNCTION
- **Verify Include Order**: Auto-includes happen at specific positions (AtTop, AfterMain, AtBottom)
- **Test Code Placement**: Move code around to see if placement is the issue
- **Use $DEBUG**: Enable debugger to see execution flow
- **Check Error Messages**: Compiler errors often indicate main code issues
- **Verify State Variables**: If auto-includes aren't working, check that state variables are being triggered correctly
- **Check Detection Logic**: Ensure SUB/FUNCTION detection isn't being blocked by `$IF` blocks or library declarations

### Related Topics

- See [Includes and Code Organization](#includes-and-code-organization) for detailed include system
- See [Metacommands](#metacommands) for `$INCLUDE` and `$USELIBRARY`
- See [Error Handling](#error-handling-and-debugging) for error handler placement

---

## Includes and Code Organization

QB64 provides multiple ways to organize and include code from external files. Understanding the include system is essential for writing modular, maintainable programs.

### Manual Includes

#### $INCLUDE

The `$INCLUDE` metacommand includes the contents of an external file at the point where it appears in your code.

**Syntax:**
```basic
$INCLUDE: 'filename'
```

**Example:**
```basic
' Main program
$INCLUDE: 'constants.bi'
$INCLUDE: 'utilities.bas'

PRINT "Using included code"
```

**Behavior:**
- File contents are inserted exactly where `$INCLUDE` appears
- Included files can contain any valid QB64 code
- If included before first SUB/FUNCTION, code becomes part of main program
- If included after first SUB/FUNCTION, code may not execute (due to implicit END)
- Paths can be relative or absolute
- Supports nested includes (included files can include other files)
- Maximum nesting depth: 100 levels

**Path Resolution:**
- **Relative paths**: Resolved relative to the directory of the file containing the `$INCLUDE` directive
- **Absolute paths**: Used as-is (Unix `/` or Windows `C:`)
- **Auto-includes**: Use absolute paths when including from root
- Leading `.\` or `./` are automatically removed

**Include Level Tracking:**
- Each nested include increments the include level (0 = main file, 1 = first include, etc.)
- File handles are allocated as `#(199 + inclevel + 1)` to ensure uniqueness
- Error messages show full include chain for nested includes: `"in line X of file.bas included (through parent.bas, grandparent.bas)"`

**Best Practices:**
- Use `$INCLUDE` for declarations, constants, and types at the top of your program
- Avoid executable main code in included files
- Use forward declarations (`DECLARE SUB`) if needed before including SUB/FUNCTION definitions

#### $INCLUDEONCE

The `$INCLUDEONCE` metacommand prevents a file from being included multiple times, even if multiple `$INCLUDE` statements reference it.

**Syntax:**
```basic
$INCLUDEONCE: 'filename'
```

**Example:**
```basic
' File: common.bi
$INCLUDEONCE: 'common.bi'  ' Prevents re-inclusion

CONST PI = 3.14159
```

**Use Cases:**
- Header files that might be included from multiple places
- Preventing duplicate declarations
- Circular dependency protection

**Implementation Details:**
- `$INCLUDEONCE` must appear at the start of the file or after a newline
- The compiler reads the entire file to check for `$INCLUDEONCE` directive
- Uses a buffer (`IncOneBuf`) to track already-included files by full path
- If file was already included, it's skipped (file is closed without processing)
- If not previously included, file path is added to buffer and inclusion proceeds

**Note:** `$INCLUDEONCE` checks the exact file path, so different paths to the same file are treated as different files. Use `_FULLPATH$` to normalize paths if needed.

### Auto-Include System

QB64-PE automatically includes support files and library code at three strategic positions in your program. This system was introduced in QB64-PE v4.0.0 for support files and extended in v4.3.0 for libraries.

#### Auto-Include Positions

The auto-include system operates at three positions:

1. **AtTop** - Before the first line of your program
2. **AfterMain** - Before the first SUB/FUNCTION definition
3. **AtBottom** - After the last line of your program

#### AtTop Auto-Includes

**Position:** Before the first line of your program (before any user code)

**Files Included:**
- `beforefirstline.bi` - General QB64-PE constants and declarations
- `color0.bi` or `color32.bi` - Color constants (if `$COLOR:0` or `$COLOR:32` is used)
- Library AtTop files (if `$USELIBRARY` is used)
- `vwatch.bi` - Debug support (if `$DEBUG` is used)

**Allowed Content:**
- `CONST` declarations
- `TYPE` definitions
- `DIM [SHARED]` variable declarations
- `DECLARE SUB/FUNCTION` forward declarations
- `$INCLUDE` directives (to other files with same restrictions)

**Restrictions:**
- **NO** SUB/FUNCTION definitions
- **NO** executable code
- **NO** DATA statements
- **NO** GOSUB labels

**Example:**
```basic
' This is what AtTop files can contain
CONST MAX_SIZE = 100
TYPE Point
    x AS INTEGER
    y AS INTEGER
END TYPE
DIM SHARED globalVar AS INTEGER
DECLARE SUB MySub
```

#### AfterMain Auto-Includes

**Position:** Right before the first SUB/FUNCTION definition (or after last line if no SUB/FUNCTION exists)

**Files Included:**
- `aftermain.bas` - QB64-PE support code (injects implicit END)
- Library AfterMain files (if `$USELIBRARY` is used)

**Allowed Content:**
- `DATA` statements
- GOSUB routines and labels
- Error handlers (`ON ERROR GOTO`)
- `$INCLUDE` directives (to other files with same restrictions)

**Restrictions:**
- **NO** SUB/FUNCTION definitions
- **NO** CONST, TYPE, DIM declarations (use AtTop for these)

**Special Note:** The implicit `END` statement is injected here, separating main program code from SUB/FUNCTION definitions.

**Example:**
```basic
' This is what AfterMain files can contain
MyData:
DATA 1, 2, 3, 4, 5

MyGosub:
    PRINT "In GOSUB"
    RETURN

MyErrorHandler:
    PRINT "Error occurred"
    RESUME NEXT
```

#### AtBottom Auto-Includes

**Position:** After the last line of your program

**Files Included:**
- `vwatch.bm` or `vwatch_stub.bm` - Debug support (if `$DEBUG` is used)
- Library AtBottom files (if `$USELIBRARY` is used)
- `afterlastline.bm` - QB64-PE support functions

**Allowed Content:**
- **ANY** valid QB64 code
- SUB/FUNCTION definitions
- Support code and utilities
- No restrictions

**Example:**
```basic
' This is what AtBottom files can contain
SUB MySub
    PRINT "In sub"
END SUB

FUNCTION MyFunction AS INTEGER
    MyFunction = 42
END FUNCTION
```

### File Extensions and Their Meanings

QB64 uses file extensions to indicate where code should be placed:

#### .bi Files (Before Include)
- **Purpose:** Header files with declarations
- **Content:** CONST, TYPE, DIM [SHARED], DECLARE
- **Use:** AtTop includes, library AtTop files
- **Restriction:** No SUB/FUNCTION definitions

#### .bas Files (BASic)
- **Purpose:** Main program code or AfterMain includes
- **Content:** Main program code, DATA, GOSUB, error handlers
- **Use:** Main program files, library AfterMain files
- **Restriction:** No SUB/FUNCTION definitions in AfterMain context

#### .bm Files (Before Module)
- **Purpose:** Module files with SUB/FUNCTION definitions
- **Content:** SUB/FUNCTION definitions, support code
- **Use:** Library AtBottom files, support modules
- **No Restrictions:** Can contain any code

### Library Includes with $USELIBRARY

The `$USELIBRARY` metacommand automatically includes libraries from the QB64-PE Libraries Pack.

**Syntax:**
```basic
$USELIBRARY: 'author/libraryname'
```

**Example:**
```basic
$USELIBRARY: 'qb64pe/json'
$USELIBRARY: 'qb64pe/http'
```

**How It Works:**
1. QB64-PE searches for the library in `libraries/includes/<author>/<libraryname>/`
2. Automatically includes three files if they exist:
   - `<libraryname>.bi` - AtTop file (declarations)
   - `<libraryname>.bas` - AfterMain file (DATA, GOSUB, error handlers)
   - `<libraryname>.bm` - AtBottom file (SUB/FUNCTION definitions)

**Include Order:**
- **AtTop files:** Included in **reverse order** of `$USELIBRARY` declarations (dependencies first)
- **AfterMain files:** Included in **same order** as `$USELIBRARY` declarations
- **AtBottom files:** Included in **same order** as `$USELIBRARY` declarations

**Library Dependencies:**
Libraries can declare dependencies by placing `$USELIBRARY` directives in their AtTop (`.bi`) file:

```basic
' File: mylib.bi (AtTop file of mylib)
$USELIBRARY: 'qb64pe/dependency'  ' Declares dependency
CONST MY_CONST = 100
```

This ensures dependencies are included before the library that needs them.

### Common Issues and Solutions

#### Issue 1: SUB/FUNCTION in Wrong Include Position

**Problem:** Placing SUB/FUNCTION definitions in AtTop or AfterMain files causes compiler errors.

**Error Message:**
```
SUB/FUNCTION definitions not allowed in AtTop/AfterMain includes
```

**Solution:** Move SUB/FUNCTION definitions to:
- AtBottom files (`.bm`)
- Main program file (after main code)
- Files included after the first SUB/FUNCTION

#### Issue 2: Main Code in Included Files

**Problem:** Executable code in included files may not run as expected.

**Behavior:**
- If included before first SUB/FUNCTION: Code executes as part of main program
- If included after first SUB/FUNCTION: Code may not execute (due to implicit END)

**Solution:**
- Put main executable code in the main program file
- Use SUB/FUNCTION in included files for reusable code
- Use `$INCLUDE` for declarations, not executable code

#### Issue 3: Circular Dependencies

**Problem:** Two files include each other, causing infinite recursion.

**Example:**
```basic
' File A includes File B
' File B includes File A
```

**Solution:**
- Use `$INCLUDEONCE` to prevent re-inclusion
- Restructure code to eliminate circular dependencies
- Move shared declarations to a common header file

#### Issue 4: Variable Scope in Includes

**Problem:** Variables declared in included files may not be accessible where expected.

**Solution:**
- Use `DIM SHARED` for global variables
- Declare variables in AtTop files (`.bi`) for global access
- Use `SHARED` keyword in SUB/FUNCTION to access global variables

#### Issue 5: Library Include Order

**Problem:** Libraries are included in wrong order, causing dependency issues.

**Solution:**
- Declare dependencies in library AtTop files
- QB64-PE automatically handles dependency order (reverse order for AtTop)
- Manually order `$USELIBRARY` declarations if needed

### Best Practices

1. **Organize by Purpose:**
   ```basic
   ' constants.bi - All constants
   CONST PI = 3.14159
   CONST MAX_SIZE = 100
   
   ' types.bi - All type definitions
   TYPE Point
       x AS INTEGER
       y AS INTEGER
   END TYPE
   
   ' utilities.bm - All utility functions
   SUB PrintPoint(p AS Point)
       PRINT p.x, p.y
   END SUB
   ```

2. **Use Appropriate File Extensions:**
   - `.bi` for declarations (AtTop)
   - `.bas` for main code or AfterMain content
   - `.bm` for SUB/FUNCTION definitions (AtBottom)

3. **Prevent Multiple Inclusions:**
   ```basic
   ' At top of header file
   $INCLUDEONCE: 'myheader.bi'
   ```

4. **Structure Main Program:**
   ```basic
   ' 1. Includes for declarations
   $INCLUDE: 'constants.bi'
   $INCLUDE: 'types.bi'
   
   ' 2. Variable declarations
   DIM SHARED myVar AS INTEGER
   
   ' 3. Forward declarations (if needed)
   DECLARE SUB MySub
   
   ' 4. Main program code
   myVar = 10
   MySub
   END
   
   ' 5. SUB/FUNCTION definitions
   SUB MySub
       PRINT myVar
   END SUB
   ```

5. **Use Libraries for Reusable Code:**
   - Create libraries for code used across multiple projects
   - Follow library structure (`.bi`, `.bas`, `.bm` files)
   - Document library dependencies

6. **Avoid Executable Code in Includes:**
   - Keep executable code in main program file
   - Use includes for declarations and definitions
   - Use SUB/FUNCTION for reusable code

### Testing with Includes

QB64-PE provides special features for testing code that uses includes.

#### Include Provider System

The include provider system is an abstraction layer that allows different implementations for include operations. This system enables testability and flexibility in how includes are processed.

**Provider Types:**
- **Filesystem Provider** (default): Normal file system includes
  - Uses QB64 file handles `#(199 + level + 1)` for each include level
  - Handles file opening, reading, and closing via standard QB64 I/O
  - Resolves relative paths based on the including file's location
- **Memory Provider**: In-memory include content for testing
  - Stores file content in memory array (`memoryFiles()`)
  - Simulates file operations without actual file I/O
  - Useful for unit tests that need to test include logic without filesystem
- **Test Provider**: Specialized provider for unit testing with call tracking
  - Extends memory provider with call tracking
  - Supports error injection for testing error handling
  - Provides path mapping for testing include resolution
  - Tracks all provider calls for test verification

**State Management:**
- Each include level has its own state tracked in `includeProviderStates()`
- State includes: provider type, file handle, content, current line, file name, open status
- Level parameter (0-based) indicates which include file is being accessed
- Level 0 is the main source file

**Usage in Compiler:**
The compiler uses this system when processing `$INCLUDE` and `$INCLUDEONCE` directives. All include operations go through the abstracted interface functions:
- `IncludeProvider_FileExists&()` - Check if file exists
- `IncludeProvider_Open&()` - Open file for reading
- `IncludeProvider_ReadLine$()` - Read next line from include
- `IncludeProvider_EOF&()` - Check if end of file reached
- `IncludeProvider_Close()` - Close include file
- `IncludeProvider_ResolvePath$()` - Resolve relative/absolute paths
- `IncludeProvider_ReadAll$()` - Read entire file (for `$INCLUDEONCE` checking)

#### Skip Includes Mode

For unit testing, you can enable "skip includes" mode to ignore all `$INCLUDE` directives. This is useful when testing individual functions without processing all their include dependencies.

**Usage:**
```basic
' Enable skip includes mode
TestFramework_SkipIncludes -1  ' or IncludeProvider_SkipIncludes(-1)

' Now compile your file - all $INCLUDE directives will be ignored
' This allows you to test individual functions without worrying about
' all the include dependencies at the top of the file

' When done, disable skip mode
TestFramework_SkipIncludes 0  ' or IncludeProvider_SkipIncludes(0)
```

**Important Notes:**
- Skip includes mode should **only** be used for unit testing
- It will cause compilation errors if the code being tested actually depends on content from the includes
- Use this feature when you want to test a function in isolation without its dependencies
- Always disable skip mode after testing to restore normal behavior

**Example:**
```basic
' Test a function that has many includes at the top
TestFramework_SkipIncludes -1  ' Skip all includes

' Test the function
result = MyFunction(42)
Test_AssertEqual result, 100, "Function should return 100"

TestFramework_SkipIncludes 0  ' Restore normal includes
```

#### Memory Provider for Testing

You can also use the memory provider to inject test content instead of reading from files:

```basic
' Switch to memory provider
IncludeProvider_SetType INCLUDE_PROVIDER_MEMORY

' Add test content
IncludeProvider_Memory_AddFile "test.bas", "PRINT ""Hello, World!"""

' Now includes will use in-memory content instead of file system
' Switch back when done
IncludeProvider_SetType INCLUDE_PROVIDER_FILESYSTEM
```

### Include Processing Internals

Understanding how the compiler processes includes can help debug issues:

**Processing Flow:**
1. Preprocessor detects `$INCLUDE` directive and sets `addmetainclude$`
2. Include Manager checks if skip includes mode is enabled (for unit testing)
3. File path is resolved (relative to including file or absolute)
4. For `$INCLUDEONCE`: File content is read and checked for `$INCLUDEONCE` directive
5. If `$INCLUDEONCE` found and file already included, skip inclusion
6. File is opened using IncludeProvider system
7. Include level is incremented and file information is tracked
8. Lines are read from include file and fed to compiler
9. When include file reaches EOF, file is closed and level is decremented
10. Process continues until all nested includes are processed

**Nested Include Handling:**
- Each nested include increments `inclevel` (0 = main file, 1 = first include, etc.)
- File information is stored in arrays: `incname$(level)`, `inclinenumber(level)`
- Maximum nesting depth is 100 levels
- Error messages show full include chain for debugging

**File Handle Allocation:**
- Filesystem provider uses file handles starting at `#200` (`199 + level + 1`)
- Each nested include gets a unique file handle
- Handles are automatically closed when include file is finished

### Debugging Include Issues

- **Check Include Order:** Verify files are included at correct positions
- **Verify File Extensions:** Ensure `.bi`, `.bas`, `.bm` are used correctly
- **Check Restrictions:** Ensure AtTop/AfterMain files follow restrictions
- **Use Error Messages:** Compiler errors indicate include problems and show include chain
- **Test Incrementally:** Include files one at a time to isolate issues
- **Verify Paths:** Ensure include file paths are correct (relative or absolute)
- **Check Nesting Depth:** If you hit 100 include levels, you may have circular dependencies
- **Verify $INCLUDEONCE:** Check that files with `$INCLUDEONCE` aren't being included multiple times
- **Use Skip Includes Mode:** For unit testing, use skip includes mode to test code without dependencies

### Related Topics

- See [Main Program Code Structure](#main-program-code-structure) for program flow
- See [Metacommands](#metacommands) for `$INCLUDE` and `$USELIBRARY` details
- See [Auto-Including Documentation](../auto-including.md) for technical details
- See [Architecture Documentation](../ARCHITECTURE.md) for compiler internals
- See source code: `source/utilities/include_provider.bi` and `include_provider.bas` for include provider implementation
- See source code: `source/qb64pe.bas` (Include Manager sections) for include processing logic

---

## Advanced Data Types

QB64 extends QBasic's type system with several new data types:

### _BIT (`)
- Single bit type: 0 or 1 (signed) or 0 to 1 (unsigned)
- Most memory-efficient type
- Useful for boolean flags and bit arrays

### _BYTE (%%)
- 8-bit unsigned integer: 0 to 255
- Signed range: -128 to 127
- Useful for raw byte manipulation

### _INTEGER64 (&&)
- 64-bit signed integer
- Range: -9,223,372,036,854,775,808 to 9,223,372,036,854,775,807
- For very large integer values

### _FLOAT (##)
- 32-byte floating-point type
- Maximum precision available in QB64
- For high-precision calculations

### _OFFSET (%&)
- Memory pointer type
- Used with `DECLARE LIBRARY` and `_MEM` operations
- Platform-dependent size

### _UNSIGNED (~)
- Modifier for INTEGER, LONG, or _INTEGER64
- Doubles positive range (e.g., INTEGER: -32768 to 32767 becomes 0 to 65535)
- Use with: `~%`, `~&`, `~&&`

---

## Metacommands

Metacommands are compiler directives that control compilation behavior. They start with `$`.

### Code Organization
- **`$INCLUDE`**: Includes external code files
- **`$INCLUDEONCE`**: Prevents multiple inclusions of the same file
- **`$DYNAMIC`**: Makes all arrays dynamic (resizable with REDIM)
- **`$STATIC`**: Makes all arrays static (fixed size, faster)

### Debugging
- **`$DEBUG`**: Enables debugging features (breakpoints, variable inspection)
- **`$ASSERTS`**: Enables `_ASSERT` macro for debug tests

### Conditional Compilation
- **`$IF`**: Precompiler conditional
- **`$ELSE`**: Precompiler else clause
- **`$ELSEIF`**: Precompiler else-if clause
- **`$END IF`**: Ends precompiler conditional block
- **`$ECHO`**: Outputs text during compilation

### Error Handling
- **`$ERROR`**: Triggers compiler errors from precompiler
- **`$CHECKING`**: Controls event and error checking (ON/OFF)

### File Embedding
- **`$EMBED`**: Embeds file contents into compiled executable
- Access with `_EMBEDDED$` function

### Executable Customization
- **`$EXEICON`**: Embeds .ICO file as program icon
- **`$VERSIONINFO`**: Adds metadata to Windows binaries

### Code Formatting
- **`$FORMAT`**: Controls code formatting behavior in QB64-PE

### Other Metacommands
- **`$RESIZE`**: Allows user window resizing (ON/OFF)
- **`$SCREENHIDE`**: Hides program window at startup
- **`$SCREENSHOW`**: Shows program window after hiding
- **`$CONSOLE`**: Creates console window for program use
- **`$COLOR`**: Includes named color constants
- **`$LET`**: Sets precompiler variables
- **`$NOPREFIX`**: (Deprecated) Allows QB64 keywords without underscore
- **`$UNSTABLE`**: Enables unstable/experimental features
- **`$USELIBRARY`**: Includes library from QB64-PE Libraries Pack

---

## Memory Management

QB64 provides direct memory access through the `_MEM` system.

### _MEM Type
The `_MEM` type contains:
- **OFFSET**: Memory address offset
- **SIZE**: Size of memory block
- **TYPE**: Type of data
- **ELEMENTSIZE**: Size of each element

### Memory Functions
- **`_MEMNEW(size)`**: Allocates new memory block, returns `_MEM` handle
- **`_MEMFREE(mem)`**: Frees allocated memory block
- **`_MEM(mem)`**: Returns `_MEM` block for largest continuous region
- **`_MEMELEMENT(mem)`**: Returns `_MEM` block for variable's memory
- **`_MEMIMAGE(handle)`**: Returns `_MEM` block for image handle
- **`_MEMSOUND(handle)`**: Returns `_MEM` block for sound handle
- **`_MEMEXISTS(name$)`**: Verifies memory block exists

### Memory Operations
- **`_MEMGET(mem, offset, variable)`**: Reads value from memory
- **`_MEMPUT(mem, offset, value)`**: Writes value to memory
- **`_MEMCOPY(source, sourceOffset, size, dest, destOffset)`**: Copies memory
- **`_MEMFILL(mem, offset, size, type, value)`**: Fills memory with value

### Memory Safety
- Always free memory blocks with `_MEMFREE`
- Only free each block once
- Validate memory exists before use
- Use `_MEMEXISTS` to check before accessing

---

## Graphics and Images

QB64 provides modern graphics capabilities beyond QBasic.

### Screen Creation
- **`SCREEN _NEWIMAGE(width, height, mode)`**: Creates new screen/page
  - Mode: 0 (text), 1-13 (legacy), 32 (32-bit color), 256 (8-bit palette)
- **`_SCREENEXISTS`**: Returns -1 if screen created

### Image Loading and Display
- **`_LOADIMAGE(filename$)`**: Loads image file (BMP, PNG, JPG), returns handle
- **`_PUTIMAGE(x, y), handle`**: Displays image at coordinates
- **`_PUTIMAGE(x1, y1)-(x2, y2), handle, dest`**: Displays scaled image
- **`_FREEIMAGE(handle)`**: Frees image from memory
- **`_COPYIMAGE(handle)`**: Copies image to new handle

### Image Information
- **`_WIDTH(handle)`**: Returns image width
- **`_HEIGHT(handle)`**: Returns image height
- **`_PIXELSIZE(handle)`**: Returns pixel mode (bits per pixel)

### Color Functions
- **`_RGB(red, green, blue)`**: Returns color value (palette or 32-bit)
- **`_RGB32(red, green, blue)`**: Returns 32-bit color value
- **`_RGBA(red, green, blue, alpha)`**: Returns color with alpha
- **`_RGBA32(red, green, blue, alpha)`**: Returns 32-bit color with alpha
- **`_RED32(color)`**: Extracts red component
- **`_GREEN32(color)`**: Extracts green component
- **`_BLUE32(color)`**: Extracts blue component
- **`_ALPHA32(color)`**: Extracts alpha component

### HSB Color Space
- **`_HSB32(hue, saturation, brightness)`**: Creates color from HSB
- **`_HSBA32(hue, saturation, brightness, alpha)`**: HSB with alpha
- **`_HUE32(color)`**: Gets hue value
- **`_SATURATION32(color)`**: Gets saturation value
- **`_BRIGHTNESS32(color)`**: Gets brightness value

### Transparency and Blending
- **`_BLEND`**: Enables 32-bit alpha blending (default)
- **`_DONTBLEND`**: Disables alpha blending
- **`_CLEARCOLOR(handle, color)`**: Sets transparent color
- **`_SETALPHA(handle, alpha, x1, y1, x2, y2)`**: Sets alpha channel

### Display Control
- **`_DISPLAY`**: Disables auto-display, shows on call
- **`_AUTODISPLAY`**: Re-enables auto-display
- **`_DEST(handle)`**: Sets destination for graphics operations
- **`_SOURCE(handle)`**: Sets source image for operations

### Screen Management
- **`_FULLSCREEN`**: Sets fullscreen mode
- **`_SCREENMOVE(x, y)`**: Moves window position
- **`_SCREENMOVE _MIDDLE`**: Centers window
- **`_DESKTOPWIDTH`**: Returns desktop width
- **`_DESKTOPHEIGHT`**: Returns desktop height

### Image Saving
- **`_SAVEIMAGE(filename$, handle)`**: Saves image to file

---

## Audio System

QB64 provides modern audio capabilities.

### Sound Loading
- **`_SNDOPEN(filename$)`**: Opens audio file (WAV, OGG, MP3, MIDI), returns handle
- **`_SNDNEW(duration, sampleRate, channels, bitsPerSample)`**: Creates empty sound
- **`_SNDOPENRAW(sampleRate, channels, bitsPerSample)`**: Opens raw audio stream
- **`_SNDCOPY(handle)`**: Copies sound handle

### Playback Control
- **`_SNDPLAY(handle)`**: Plays sound
- **`_SNDPLAYCOPY(handle)`**: Plays copy and auto-closes
- **`_SNDPLAYFILE(filename$)`**: Directly plays file
- **`_SNDSTOP(handle)`**: Stops playback
- **`_SNDPAUSE(handle)`**: Pauses playback
- **`_SNDPLAYING(handle)`**: Returns -1 if playing
- **`_SNDPAUSED(handle)`**: Returns -1 if paused

### Sound Information
- **`_SNDLEN(handle)`**: Returns length in seconds
- **`_SNDGETPOS(handle)`**: Returns current position
- **`_SNDSETPOS(handle, position)`**: Sets playback position

### Sound Control
- **`_SNDVOL(handle, volume)`**: Sets volume (0.0 to 1.0)
- **`_SNDBAL(handle, balance)`**: Sets balance/3D position
- **`_SNDLOOP(handle)`**: Loops sound
- **`_SNDLIMIT(handle, seconds)`**: Stops after duration

### Raw Audio
- **`_SNDRAW(data, sampleRate, channels, format)`**: Plays raw audio data
- **`_SNDRAWBATCH(array, count, sampleRate, channels, format)`**: Batch playback
- **`_SNDRAWDONE`**: Pads final buffer
- **`_SNDRAWLEN`**: Returns buffer length
- **`_SNDRATE`**: Returns sound card sample rate

### Cleanup
- **`_SNDCLOSE(handle)`**: Closes and frees sound

### MIDI Support
- **`_MIDISOUNDBANK(filename$)`**: Sets MIDI sound bank/SoundFont
- **`$MIDISOUNDFONT`**: (Deprecated) Sets MIDI sound font

---

## Networking and TCP/IP

QB64 provides TCP/IP networking capabilities.

### Connection Types
- **`_OPENHOST(port$)`**: Opens server on port, returns host handle
- **`_OPENCLIENT(host$:port$)`**: Connects to server, returns client handle
- **`_OPENCONNECTION(hostHandle)`**: Accepts client connection, returns connection handle

### Connection Status
- **`_CONNECTED(handle)`**: Returns -1 if connected
- **`_CONNECTIONADDRESS$(handle)`**: Returns client IP address
- **`_STATUSCODE(handle)`**: Returns HTTP status code (for HTTP requests)

### Data Transfer
- **`GET #handle, , data$`**: Reads data from connection
- **`PUT #handle, , data$`**: Sends data to connection
- **`EOF(handle)`**: Checks for end of connection

### HTTP Operations
Use `_OPENCLIENT` with HTTP URLs:
```basic
handle = _OPENCLIENT("https://example.com/page")
GET #handle, , response$
```

### URL Encoding
- **`_ENCODEURL$(url$)`**: Encodes URL string
- **`_DECODEURL$(url$)`**: Decodes URL string

---

## External Libraries and DLLs

QB64 can interface with external C++ libraries and Windows DLLs.

### DECLARE LIBRARY
```basic
DECLARE LIBRARY "libraryname"
    FUNCTION FunctionName& (BYVAL param AS LONG)
    SUB SubName (BYVAL param AS LONG)
END DECLARE
```

### Windows API Example
```basic
DECLARE LIBRARY "user32"
    FUNCTION MessageBoxA& (BYVAL hWnd AS LONG, BYVAL lpText AS STRING, _
                            BYVAL lpCaption AS STRING, BYVAL uType AS LONG)
END DECLARE

MessageBoxA 0, "Hello", "Title", 0
```

### Parameter Passing
- **`BYVAL`**: Pass by value (default for numbers)
- **`BYREF`**: Pass by reference (default for strings)
- **`_OFFSET`**: For pointer parameters

### ALIAS
Use `ALIAS` to rename imported functions:
```basic
DECLARE LIBRARY "mylib"
    FUNCTION OriginalName ALIAS "ActualName" (param AS LONG)
END DECLARE
```

### Library Types
- **C++ Headers**: Include C++ header files
- **Windows DLLs**: Access Windows API functions
- **SDL Libraries**: Use SDL functions
- **Custom Libraries**: Link custom C++ code

### $USELIBRARY
Simplifies including libraries from QB64-PE Libraries Pack:
```basic
$USELIBRARY "libraryname"
```

---

## Console Windows

QB64 provides console window support for text I/O.

### Console Creation
- **`$CONSOLE`**: Creates console window at program start
- **`_CONSOLE ON/OFF`**: Controls console visibility
- **`_CONSOLE`**: Sets console as `_DEST` for output

### Console Input
- **`_CONSOLEINPUT`**: Fetches input from console (mouse and keyboard)
- **`_CINP`**: Returns key code from console input

### Console Appearance
- **`_CONSOLETITLE(title$)`**: Sets console window title
- **`_CONSOLEFONT(fontName$, size)`**: Sets console font
- **`_CONSOLECURSOR ON/OFF`**: Shows/hides cursor
- **`_CONSOLECURSOR size`**: Sets cursor size

---

## File Operations

QB64 extends file operations beyond QBasic.

### File Existence
- **`_FILEEXISTS(filename$)`**: Returns -1 if file exists
- **`_DIREXISTS(path$)`**: Returns -1 if directory exists

### File Reading/Writing
- **`_READFILE$(filename$)`**: Reads entire file into string
- **`_WRITEFILE(filename$, data$)`**: Writes string to file

### File Listing
- **`_FILES$(pattern$)`**: Returns matching file/directory name
- Use in loop to get all matches

### Path Operations
- **`_FULLPATH$(path$)`**: Converts relative to absolute path
- **`_CWD$`**: Returns current working directory
- **`_STARTDIR$`**: Returns program's starting directory
- **`_DIR$(type)`**: Returns Windows special paths (My Documents, etc.)

### File Dialogs
- **`_OPENFILEDIALOG$(title$, filter$, defaultPath$)`**: Shows open dialog
- **`_SAVEFILEDIALOG$(title$, filter$, defaultPath$)`**: Shows save dialog
- **`_SELECTFOLDERDIALOG$(title$, defaultPath$)`**: Shows folder selection

### File Dropping
- **`_ACCEPTFILEDROP`**: Enables file drop on window
- **`_DROPPEDFILE(index)`**: Returns dropped file path
- **`_TOTALDROPPEDFILES`**: Returns count of dropped files
- **`_FINISHDROP`**: Clears drop list

---

## String Manipulation

QB64 provides enhanced string functions.

### String Comparison
- **`_STRCMP(string1$, string2$)`**: Case-sensitive comparison
- **`_STRICMP(string1$, string2$)`**: Case-insensitive comparison
- Returns: <0 (less), 0 (equal), >0 (greater)

### String Search
- **`_INSTRREV(string$, substring$)`**: Finds last occurrence (reverse INSTR)

### String Conversion
- **`_TOSTR$(number)`**: Converts number to string (replaces STR$)
- **`_TRIM$(string$)`**: Trims both ends (LTRIM$ + RTRIM$)

### Base64 Encoding
- **`_BASE64ENCODE$(string$)`**: Encodes string to Base64
- **`_BASE64DECODE$(string$)`**: Decodes Base64 string

### Compression
- **`_DEFLATE$(string$)`**: Compresses string
- **`_INFLATE$(string$)`**: Decompresses string

### Checksums
- **`_MD5$(string$)`**: Returns MD5 hash
- **`_CRC32(string$)`**: Returns CRC-32 checksum
- **`_ADLER32(string$)`**: Returns Adler-32 checksum

### Clipboard
- **`_CLIPBOARD$`**: Sets clipboard text
- **`_CLIPBOARD$`**: (function) Gets clipboard text
- **`_CLIPBOARDIMAGE`**: Sets clipboard image
- **`_CLIPBOARDIMAGE`**: (function) Gets clipboard image

---

## Mathematical Functions

QB64 provides extensive mathematical functions.

### Trigonometric Functions
- **`_ACOS(x)`**: Arc cosine
- **`_ASIN(x)`**: Arc sine
- **`_ATAN2(y, x)`**: Arc tangent of y/x (handles all quadrants)
- **`_COT(x)`**: Cotangent (1/TAN)
- **`_CSC(x)`**: Cosecant (1/SIN)
- **`_SEC(x)`**: Secant (1/COS)

### Hyperbolic Functions
- **`_COSH(x)`**: Hyperbolic cosine
- **`_SINH(x)`**: Hyperbolic sine
- **`_TANH(x)`**: Hyperbolic tangent
- **`_ACOSH(x)`**: Inverse hyperbolic cosine
- **`_ASINH(x)`**: Inverse hyperbolic sine
- **`_ATANH(x)`**: Inverse hyperbolic tangent
- **`_COTH(x)`**: Hyperbolic cotangent
- **`_CSCH(x)`**: Hyperbolic cosecant
- **`_SECH(x)`**: Hyperbolic secant

### Inverse Trigonometric
- **`_ARCCOT(x)`**: Inverse cotangent
- **`_ARCCSC(x)`**: Inverse cosecant
- **`_ARCSEC(x)`**: Inverse secant

### Angle Conversion
- **`_D2R(degrees)`**: Degrees to radians
- **`_R2D(radians)`**: Radians to degrees
- **`_D2G(degrees)`**: Degrees to gradients
- **`_G2D(gradients)`**: Gradients to degrees
- **`_G2R(gradients)`**: Gradients to radians
- **`_R2G(radians)`**: Radians to gradients

### Other Functions
- **`_PI(multiplier)`**: Returns π or multiples
- **`_CEIL(x)`**: Rounds up
- **`_ROUND(x)`**: Rounds to nearest integer
- **`_CLAMP(value, min, max)`**: Clamps value to range
- **`_MIN(a, b)`**: Returns minimum
- **`_MAX(a, b)`**: Returns maximum
- **`_HYPOT(x, y)`**: Returns hypotenuse

---

## Bitwise Operations

QB64 provides bit manipulation functions.

### Bit Reading/Setting
- **`_READBIT(value, bit)`**: Reads bit state (0 or 1)
- **`_SETBIT(value, bit)`**: Sets bit to 1
- **`_RESETBIT(value, bit)`**: Sets bit to 0
- **`_TOGGLEBIT(value, bit)`**: Toggles bit

### Bit Shifting
- **`_SHL(value, bits)`**: Shift left
- **`_SHR(value, bits)`**: Shift right (logical)
- **`_ROL(value, bits)`**: Rotate left
- **`_ROR(value, bits)`**: Rotate right

### Number Base Conversion
- **`_BIN$(number)`**: Converts to binary string
- **`HEX$(number)`**: Converts to hexadecimal string
- **`OCT$(number)`**: Converts to octal string

### Type Casting
- **`_CAST(type, value)`**: C-like type casting

---

## Error Handling and Debugging

QB64 provides enhanced error handling.

### Error Information
- **`_ERRORLINE`**: Returns line number of error
- **`_ERRORMESSAGE$`**: Returns error description
- **`_INCLERRORFILE$`**: Returns include file with error
- **`_INCLERRORLINE`**: Returns line in include file

### Assertions
- **`_ASSERT(condition)`**: Debug assertion (requires `$ASSERTS`)
- Stops program if condition is false

### Debugging
- **`$DEBUG`**: Enables debugger
- Set breakpoints, step through code, inspect variables

---

## Cross-Platform Considerations

QB64 supports Windows, Linux, and macOS.

### Platform Detection
- **`_OS$`**: Returns platform string: `[WINDOWS]`, `[LINUX]`, `[MACOSX]` and `[32BIT]` or `[64BIT]`

### Platform-Specific Features
- **Windows**: DLL support, Windows API, Registry access
- **Linux/macOS**: Some Windows-specific features unavailable
- Check `_OS$` before using platform-specific code

### Path Separators
- Use `\` for Windows, `/` for Linux/macOS
- QB64 handles this automatically in most cases

### Case Sensitivity
- Windows: Case-insensitive file system
- Linux/macOS: Case-sensitive file system
- Use consistent casing in filenames

---

## OpenGL Integration

QB64 provides OpenGL support for 3D graphics.

### OpenGL Setup
- All OpenGL functions prefixed with `_gl` or `_glu`
- Must use `SUB _GL` for OpenGL code
- Requires `SCREEN _NEWIMAGE` with appropriate mode

### Key OpenGL Functions
- **`_glBegin(mode)` / `_glEnd`**: Delimit primitive drawing
- **`_glVertex2f/3f/4f(x, y, z, w)`**: Specify vertex coordinates
- **`_glColor3f/4f(r, g, b, a)`**: Set color
- **`_glClear(mask)`**: Clear buffers
- **`_glMatrixMode(mode)`**: Set matrix mode
- **`_glLoadIdentity`**: Load identity matrix
- **`_glRotatef(angle, x, y, z)`**: Rotate
- **`_glTranslatef(x, y, z)`**: Translate
- **`_glScalef(x, y, z)`**: Scale

### OpenGL Rendering
- **`_GLRENDER`**: Controls OpenGL rendering order
- **`_DISPLAYORDER`**: Sets rendering order (software/hardware/custom)

### See Also
- Full OpenGL keyword reference in wiki
- OpenGL tutorials and examples

---

## Additional Important Topics

### Controller Devices
- **`_DEVICES`**: Returns number of input devices
- **`_DEVICE$(number)`**: Returns device information
- **`_AXIS(device, axis)`**: Gets axis value (-1 to 1)
- **`_BUTTON(device, button)`**: Gets button state
- **`_WHEEL(device, wheel)`**: Gets wheel scroll
- **`_DEVICEINPUT`**: Returns device with input event

### Mouse Functions
- **`_MOUSEX`**, **`_MOUSEY`**: Mouse position
- **`_MOUSEBUTTON(button)`**: Button state
- **`_MOUSEWHEEL`**: Scroll wheel
- **`_MOUSEINPUT`**: Input event occurred
- **`_MOUSEMOVE(x, y)`**: Move mouse cursor
- **`_MOUSEHIDE`**, **`_MOUSESHOW`**: Hide/show cursor

### Keyboard Functions
- **`_KEYHIT`**: Returns key code
- **`_KEYDOWN(code)`**: Checks if key pressed
- **`_KEYCLEAR`**: Clears keyboard buffer

### Fonts
- **`_LOADFONT(filename$, size, style)`**: Loads TTF font
- **`_FONT(handle)`**: Sets current font
- **`_FONTHEIGHT`**: Returns font height
- **`_FONTWIDTH`**: Returns font width
- **`_FREEFONT(handle)`**: Frees font

### Text Printing
- **`_PRINTSTRING(x, y, text$)`**: Prints text at coordinates
- **`_PRINTWIDTH(text$)`**: Returns text width
- **`_PRINTMODE(mode)`**: Sets text printing mode
- **`_UPRINTSTRING(x, y, text$)`**: Unicode text printing
- **`_UPRINTWIDTH(text$)`**: Unicode text width

### Unicode Support
- **`_MAPUNICODE(unicode, ascii)`**: Maps Unicode to ASCII
- **`_MAPUNICODE(ascii)`**: (function) Gets Unicode from ASCII
- **`_UCHARPOS(text$, index)`**: Gets character positions

### Timers
- **`ON TIMER(seconds) GOSUB`**: Sets timer event
- **`_FREETIMER`**: Returns unused timer number
- **`FREE TIMER(number)`**: Frees timer

### Dialog Boxes
- **`_MESSAGEBOX(title$, message$, buttons$, icon$)`**: Shows message box
- **`_MESSAGEBOX(title$, message$, buttons$, icon$)`**: (function) Returns button clicked
- **`_INPUTBOX$(prompt$, title$, default$)`**: Shows input dialog
- **`_COLORCHOOSERDIALOG`**: Shows color picker

### System Integration
- **`_SHELLHIDE(command$)`**: Executes shell command, returns exit code
- **`_NOTIFYPOPUP(title$, message$)`**: Shows system notification
- **`_SCREENCLICK(x, y)`**: Simulates screen click
- **`_SCREENPRINT(text$)`**: Simulates typing

### Logical Operators
- **`_ANDALSO`**: Short-circuit AND
- **`_ORELSE`**: Short-circuit OR
- **`_NEGATE`**: Logical NOT

### Conditional Evaluation
- **`_IIF(condition, trueValue, falseValue)`**: Ternary operator with short-circuiting

### Program Control
- **`_CONTINUE`**: Skips to next loop iteration
- **`_EXIT`**: (function) Prevents user exit, checks close button

### Window Management
- **`_TITLE(title$)`**: Sets window title
- **`_TITLE$`**: (function) Gets window title
- **`_SCREENHIDE`**: Hides window
- **`_SCREENSHOW`**: Shows window
- **`_SCREENICON`**: Minimizes window
- **`_SCREENICON`**: (function) Checks if minimized
- **`_WINDOWHANDLE`**: Returns OS window handle (Windows only)
- **`_WINDOWHASFOCUS`**: Returns -1 if window has focus (Windows only)

### Environment Variables
- **`ENVIRON$`**: Gets environment variable
- **`ENVIRON`**: Sets environment variable
- **`_ENVIRONCOUNT`**: Returns count of environment variables

### Command Line
- **`COMMAND$`**: Returns command line arguments
- **`_COMMANDCOUNT`**: Returns number of arguments

### Line Numbers
- QB64 supports line numbers for compatibility
- Can remove with utilities
- See wiki articles on line number handling

---

## Best Practices

1. **Memory Management**: Always free resources (`_FREEIMAGE`, `_FREEFONT`, `_MEMFREE`, `_SNDCLOSE`)
2. **Error Checking**: Check return values and use `_FILEEXISTS`, `_DIREXISTS` before file operations
3. **Platform Awareness**: Use `_OS$` to detect platform before platform-specific code
4. **Type Safety**: Use appropriate data types for efficiency
5. **Resource Cleanup**: Free all resources before program end
6. **Debugging**: Use `$DEBUG` during development
7. **Code Organization**: Use `$INCLUDE` for modular code
8. **Documentation**: Comment complex code sections

---

## Reference Links

- **QB64 Wiki**: https://qb64phoenix.com/qb64wiki
- **Keyword Reference - Alphabetical**: Complete list of all keywords
- **Keyword Reference - By Usage**: Keywords organized by purpose
- **Metacommands**: Complete metacommand reference
- **Forum**: https://qb64phoenix.com/forum

---

*This guide was compiled from the QB64 Phoenix Edition Wiki. For the most up-to-date information, refer to the official wiki.*
