# The QB64-PE Auto-Include Logic

QB64-PE features an automatic include system that transparently injects support files and library code into compiled programs at the correct positions. This system ensures that necessary support code (constants, debug utilities, library functions) is available without requiring explicit `$INCLUDE` statements from the user.

## Overview

The auto-include feature was introduced in **QB64-PE v4.0.0** for support files (general and color CONSTs, vwatch code ($DEBUG), and other support code). Since **QB64-PE v4.3.0**, the auto-include system also handles the `$USELIBRARY` metacommand to automatically include 3rd party libraries.

This document details the auto-include mechanism for QB64-PE developers and contributors.

## Related Documentation

- [Build System](build-system.md) - How QB64-PE compiles programs
- [Architecture](ARCHITECTURE.md) - Overall compiler architecture
- [Testing](testing.md) - Testing infrastructure

## Auto-Include Positions

The auto-include system operates at three strategic positions in the compiled program:
1. **AtTop** - This auto-including happens right at the beginning of the program as the compiler will see it, but is actually done before the first line the user has written in his program i.e. what you see as first line in the IDE respectively. In this place only initializing stuff CONST, (RE)DIM [SHARED], TYPE etc. can be done just like in an `*.bi` file of a library. The auto-include logic in the QB64-PE sources uses the **firstLine** variable to trigger and control including at this position.
2. **AfterMain** - This auto-including happens right before the first SUB or FUNCTION definition appears in the code, regardless if it appears in the main program file or inside of an $INCLUDE'd file. If no SUB/FUNCTION is used in the program at all, then auto-including happens after the last program line but still before the **AtBottom** auto-includes. In this place only main scope (global) DATAs, GOSUB routines and ERROR handlers can be done. If libraries use this, then respective include files should simply use the `*bas` extension just like the main program file. The auto-include logic in the QB64-PE sources uses the **mainEndLine** variable to trigger and control including at this position.
3. **AtBottom** - This auto-including happens right at the end of the program as the compiler will see it, but is actually done after the last line the user has written in his program i.e. what you see as last line in the IDE respectively. In this place only SUB and FUNCTION definitions can be done just like in an `*.bm` file of a library. The auto-include logic in the QB64-PE sources uses the **lastLine** variable to trigger and control including at this position.

### Control Variables

The auto-include system uses three control variables in the compiler source code:
- **`firstLine`** - Controls AtTop includes
- **`mainEndLine`** - Controls AfterMain includes  
- **`lastLine`** - Controls AtBottom includes

Each control variable can have four different states during each compile pass:
- **0** = inactive
- **1** = triggered
- **2** = in progress
- **3** = done

These states are used throughout the compiler to control the include flow and generate appropriate errors when necessary.

### Restrictions

**AtTop** and **AfterMain** files cannot contain any SUB or FUNCTION definitions. This restriction prevents interleaved main program code with SUB/FUNCTION definitions when multiple files are auto-included (such as when multiple libraries are used). A compiler error was specifically implemented to catch such cases and will be generated if SUB or FUNCTION definitions are found in these positions.

Auto-included files can themselves use `$INCLUDE` directives, but those included files must also follow the restrictions of the current auto-include level (i.e., AtTop files can only include other files that contain constants, types, and declarations, not SUB/FUNCTION definitions).

### Library Dependencies

If library **ABC** depends on another library **XYZ**, the dependency can be declared by placing a `$USELIBRARY:'author/xyz'` directive inside the **AtTop** file of library **ABC**. Each `$USELIBRARY` directive found during compilation triggers a recompile to allow the auto-include logic to adapt the program structure.

**Include Order:**
- **AtTop** library files are included in **reverse order** of their `$USELIBRARY` declarations. This ensures dependencies are included before the libraries that depend on them.
- **AfterMain** and **AtBottom** library files are included in the **same order** as their `$USELIBRARY` declarations appear in the code.

## Program Structure

The following diagram shows the complete program structure as it appears to the compiler after all auto-include processing:

```
    +---------------------------------------------------+
    | beforefirstline.bi (always, except the file is in |---> from internal/support/include
    | the IDE for editing (general CONST values))       |
    +---------------------------------------------------+
    | color0.bi or color32.bi (if $COLOR:0/32 is used   |---> from internal/support/color
    | and none of both is in the IDE for editing)       |
    \---------------------------------------------------/
     >> from here all general/color CONST can be used <<
    /---------------------------------------------------\
    | "AtTop" library files (if $USELIBRARY is used)    |---> from libraries/includes/<author>/<libname>
    | The order of libraries is reversed to the order of|
    | their appearance in code to serve dependencies.   |
    +---------------------------------------------------+
    | vwatch.bi (if $DEBUG is used or vwatch.bm is in   |---> from internal/support/vwatch
    | the IDE for editing)                              |
    +---------------------------------------------------+---  ---  ---  ---  ---  ---  ---  ---  ---  ---  --- --- ---
    | This is the user's main program currently in the  |          /--------------------------------------------------+
    | IDE or passed to the command line compiler.       |         / aftermain.bas (always, inject an implicit END)    |
    |      CONST, DIM, TYPE etc.                        |   +----<  The include happens right before the first SUB or |
    |      code...                                      |   |     \ FUNCTION, i.e. in the last line, if the main code |
    |      END                                          |   |      \ doesn't define any SUBs or FUNCTIONs.            |--+
    |      labelMyData:                                 |   |       \-----------+-------------------------------------+  |
    |      DATA ... ... ... ... ...                     |   |                   |   from internal/support/include <------+
    |      DATA ... ... ... ... ...                     |   |                   |
    |      labelMyGosub:                                |   |      /------------+-------------------------------------+
    |      code...                                      |   |     / "AfterMain" library files (if $USELIBRARY is used)|
    |      RETURN                                       |   |  +-<  The order of libraries is as they appear in code. |-----------+
    |      labelMyErrorHandler:                         |   |  |  \-------------+-------------------------------------+           |
    |      code...                                      |   |  |                |   from libraries/includes/<author>/<libname> <--+
    |      RESUME NEXT                                  |   |  |                ^
    |       ===============     <<<-------------------------+  |              NOTE:
    |       ===============     <<<----------------------------+    Works also if the main end (i.e. the 1st SUB/FUNC)
    |      SUB/FUNCTION MySubFunc                       |           is inside of a included file. Then the include
    |      code...                                      |           file is regularly continued (with 1st S/F) after
    |      END SUB/FUNCTION                             |           all "AfterMain" files are auto-included.
    +---------------------------------------------------+---  ---  ---  ---  ---  ---  ---  ---  ---  ---  --- --- ---
    | vwatch.bm (if $DEBUG is used, else vwatch_stub.bm)|
    | Only if none of the vwatch*.* files is in the IDE.|---> from internal/support/vwatch
    +---------------------------------------------------+
    | "AtBottom" library files (if $USELIBRARY is used) |
    | The order of libraries is as they appear in code. |---> from libraries/includes/<author>/<libname>
    +---------------------------------------------------+
    | afterlastline.bm (always, except the file is in   |
    | the IDE for editing (QB64-PE support functions))  |---> from internal/support/include
    +---------------------------------------------------+
```

## Testing Auto-Include Functionality

The auto-include system is tested through the compiler test suite:

- **Compile Tests** (`tests/compile_tests/`) - Verify that auto-included files are properly injected
- **Integration Tests** (`tests/integration/`) - Test compiler state and dependency detection
- **Unit Tests** (`tests/unit/`) - Test include processing logic

For more information about testing, see [Testing Documentation](testing.md).

## Implementation Details

The auto-include logic is implemented in the QB64-PE compiler source code. Key files:
- Include processing: `source/qb64pe.bas`
- Support files: `internal/support/include/`, `internal/support/color/`, `internal/support/vwatch/`
- Library system: Handled through `$USELIBRARY` processing

## See Also

- [Build System Documentation](build-system.md) - How programs are compiled
- [Architecture Documentation](ARCHITECTURE.md) - Compiler architecture overview
- [Testing Documentation](testing.md) - Testing infrastructure