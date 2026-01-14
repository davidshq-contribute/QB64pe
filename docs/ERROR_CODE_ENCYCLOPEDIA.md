# QB64-PE Error Code Encyclopedia

**Version**: 1.0  
**Last Updated**: 2026-01-14  
**Target Audience**: All developers  

---

## Overview

This comprehensive reference provides detailed information about all error codes in QB64-PE, including their causes, solutions, troubleshooting guides, and prevention strategies. QB64-PE uses a sophisticated error handling system that provides both runtime and compile-time error detection.

---

## Error Handling System Architecture

### Core Components

- **Error Detection**: Runtime and compile-time error identification
- **Error Reporting**: Human-readable error messages with line numbers
- **Error Recovery**: ON ERROR GOTO handlers and error recovery mechanisms
- **Error State Management**: Global error state tracking and retrieval functions

### Key Files

- `internal/c/libqb/include/error_handle.h` - Error handling API and constants
- `internal/c/libqb/src/error_handle.cpp` - Error handling implementation
- `source/utilities/give_error.bas` - BASIC-level error reporting
- `source/qb64pe.bas` - Main compiler error handling

---

## Error Code Categories

### 1. Control Flow Errors (1-30)
These errors occur when program flow control structures are malformed or mismatched.

### 2. Type and Data Errors (5-14, 37-40)
Related to data types, conversions, and variable usage.

### 3. File I/O Errors (52-76)
Comprehensive file system and device I/O error handling.

### 4. Memory Management Errors (7, 256-257, 300-318)
Memory allocation, deallocation, and memory region management.

### 5. System and Library Errors (258-271)
Dynamic library loading, OpenGL, and system-level errors.

---

## Complete Error Code Reference

## Control Flow Errors

### Error 1: NEXT without FOR
**Message**: "NEXT without FOR"  
**Category**: Control Flow  
**Severity**: Critical

**Causes**:
- Missing FOR statement for a NEXT
- Mismatched FOR/NEXT pairs
- Nested loop structure errors

**Example**:
```basic
' INCORRECT
NEXT i

' CORRECT
FOR i = 1 TO 10
    ' code
NEXT i
```

**Solutions**:
- Ensure every NEXT has a corresponding FOR
- Check loop variable names match
- Verify proper nesting

**Prevention**:
- Use consistent indentation
- Test loop structures incrementally

---

### Error 2: Syntax Error
**Message**: "Syntax error"  
**Category**: Language Syntax  
**Severity**: Critical

**Causes**:
- Invalid BASIC syntax
- Missing keywords or operators
- Incorrect statement structure

**Common Examples**:
```basic
' INCORRECT
IF x > 0 THEN
    PRINT "Positive"
' Missing END IF

' CORRECT
IF x > 0 THEN
    PRINT "Positive"
END IF
```

**Solutions**:
- Check statement syntax in reference
- Verify all keywords are spelled correctly
- Ensure proper statement termination

**Prevention**:
- Use IDE syntax highlighting
- Test code in small segments
- Follow coding conventions

---

### Error 3: RETURN without GOSUB
**Message**: "RETURN without GOSUB"  
**Category**: Control Flow  
**Severity**: Critical

**Causes**:
- RETURN statement without matching GOSUB
- Multiple RETURN statements
- Incorrect subroutine structure

**Example**:
```basic
' INCORRECT
RETURN

' CORRECT
GOSUB MySub
END

MySub:
    ' subroutine code
RETURN
```

**Solutions**:
- Ensure every RETURN has a corresponding GOSUB
- Check subroutine flow
- Use modern FUNCTION/SUB instead of GOSUB/RETURN

---

### Error 4: Out of DATA
**Message**: "Out of DATA"  
**Category**: Data Handling  
**Severity**: Critical

**Causes**:
- READ statements exceed available DATA statements
- Missing DATA statements
- Incorrect DATA indexing

**Example**:
```basic
' INCORRECT
READ a, b, c
DATA 1, 2

' CORRECT
READ a, b, c
DATA 1, 2, 3
```

**Solutions**:
- Count READ variables vs DATA items
- Add missing DATA statements
- Use RESTORE if needed

---

### Error 5: Illegal Function Call
**Message**: "Illegal function call"  
**Category**: Function Call  
**Severity**: Critical

**Causes**:
- Invalid function parameters
- Out-of-range function arguments
- Incorrect function usage

**Common Examples**:
```basic
' INCORRECT
SQR(-1)        ' Negative square root
MID$("abc", 0) ' Zero-based index
LEFT$("", 5)   ' Empty string

' CORRECT
IF x >= 0 THEN SQR(x)
MID$("abc", 1)
IF LEN(s$) > 0 THEN LEFT$(s$, 5)
```

**Solutions**:
- Validate function parameters
- Check function documentation
- Add parameter validation

---

### Error 6: Overflow
**Message**: "Overflow"  
**Category**: Numeric  
**Severity**: Critical

**Causes**:
- Numeric calculations exceed type limits
- Integer overflow
- String concatenation exceeding limits

**Example**:
```basic
' INCORRECT
DIM x AS INTEGER
x = 32767 + 1  ' Integer overflow

' CORRECT
DIM x AS LONG
x = 32767 + 1
```

**Solutions**:
- Use larger data types (LONG, _INTEGER64)
- Add range checking
- Use appropriate numeric types

---

### Error 7: Out of Memory
**Message**: "Out of memory"  
**Category**: Memory Management  
**Severity**: Critical

**Causes**:
- Insufficient system memory
- Memory leaks
- Large array allocations
- Excessive string operations

**Solutions**:
- Free unused variables
- Use smaller data types
- Implement memory management
- Check for memory leaks

**Prevention**:
- Monitor memory usage
- Use dynamic allocation carefully
- Implement proper cleanup

---

### Error 8: Label Not Defined
**Message**: "Label not defined"  
**Category**: Control Flow  
**Severity**: Critical

**Causes**:
- GOTO/GOSUB to undefined label
- Label name typos
- Missing label definitions

**Example**:
```basic
' INCORRECT
GOTO MyLabel
END

' CORRECT
GOTO MyLabel
END

MyLabel:
    ' code
```

**Solutions**:
- Verify label spelling
- Ensure all labels are defined
- Check label scope

---

### Error 9: Subscript Out of Range
**Message**: "Subscript out of range"  
**Category**: Array Handling  
**Severity**: Critical

**Causes**:
- Array index outside bounds
- Incorrect array dimensions
- Uninitialized arrays

**Example**:
```basic
' INCORRECT
DIM arr(5)
arr(6) = 10  ' Index out of range

' CORRECT
DIM arr(5)
arr(5) = 10
```

**Solutions**:
- Check array bounds before access
- Use LBOUND/UBOUND functions
- Validate array dimensions

---

### Error 10: Duplicate Definition
**Message**: "Duplicate definition"  
**Category**: Declaration  
**Severity**: Critical

**Causes**:
- Multiple variable definitions
- Duplicate function/sub definitions
- Repeated constant definitions

**Example**:
```basic
' INCORRECT
DIM x AS INTEGER
DIM x AS INTEGER  ' Duplicate

' CORRECT
DIM x AS INTEGER
DIM y AS INTEGER
```

**Solutions**:
- Remove duplicate definitions
- Use unique variable names
- Check include files for conflicts

---

### Error 11: Division by Zero
**Message**: "Division by zero"  
**Category**: Numeric  
**Severity**: Critical

**Causes**:
- Division by zero
- MOD operation with zero
- Floating-point division by zero

**Example**:
```basic
' INCORRECT
x = 10 / 0
y = 10 MOD 0

' CORRECT
IF divisor != 0 THEN
    x = 10 / divisor
END IF
```

**Solutions**:
- Check denominators before division
- Add zero-checking logic
- Use error handling

---

### Error 12: Illegal in Direct Mode
**Message**: "Illegal in direct mode"  
**Category**: Environment  
**Severity**: Medium

**Causes**:
- Using statements not allowed in immediate mode
- Invalid direct mode commands

**Solutions**:
- Use proper program mode
- Check statement compatibility
- Use appropriate commands

---

### Error 13: Type Mismatch
**Message**: "Type mismatch"  
**Category**: Type System  
**Severity**: Critical

**Causes**:
- Incompatible type assignments
- Incorrect function parameter types
- Array type conflicts

**Example**:
```basic
' INCORRECT
DIM x AS INTEGER
x = "Hello"  ' String to integer

' CORRECT
DIM x AS INTEGER
DIM s AS STRING
x = 123
s = "Hello"
```

**Solutions**:
- Use compatible types
- Add type conversion functions
- Check function signatures

---

### Error 14: Out of String Space
**Message**: "Out of string space"  
**Category**: Memory Management  
**Severity**: Critical

**Causes**:
- Excessive string concatenation
- String memory exhaustion
- Large string operations

**Solutions**:
- Free unused strings
- Use string space efficiently
- Implement string pooling

---

### Error 16: String Formula Too Complex
**Message**: "String formula too complex"  
**Category**: String Operations  
**Severity**: Medium

**Causes**:
- Complex string expressions
- Nested string operations
- Deep string concatenation

**Solutions**:
- Simplify string expressions
- Break into multiple statements
- Use temporary variables

---

### Error 17: Cannot Continue
**Message**: "Cannot continue"  
**Category**: Program Control  
**Severity**: Medium

**Causes**:
- Attempting to continue after certain errors
- Invalid program state

**Solutions**:
- Restart program
- Fix underlying error
- Use proper error handling

---

### Error 18: Function Not Defined
**Message**: "Function not defined"  
**Category**: Function Call  
**Severity**: Critical

**Causes**:
- Calling undefined functions
- Missing function declarations
- Incorrect function names

**Example**:
```basic
' INCORRECT
x = MyFunction()  ' Function not defined

' CORRECT
FUNCTION MyFunction()
    MyFunction = 42
END FUNCTION

x = MyFunction()
```

**Solutions**:
- Define missing functions
- Check function spelling
- Include necessary libraries

---

### Error 19: No RESUME
**Message**: "No RESUME"  
**Category**: Error Handling  
**Severity**: Critical

**Causes**:
- Error handler without RESUME
- Incomplete error handling

**Solutions**:
- Add RESUME statement
- Use proper error handling structure
- Check error handler flow

---

### Error 20: RESUME without Error
**Message**: "RESUME without error"  
**Category**: Error Handling  
**Severity**: Critical

**Causes**:
- RESUME outside error handler
- Incorrect error handling

**Solutions**:
- Place RESUME in error handler
- Check error handling logic
- Use proper ON ERROR structure

---

## File I/O Errors (52-76)

### Error 52: Bad File Name or Number
**Message**: "Bad file name or number"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Invalid file numbers
- Incorrect file names
- File number conflicts

**Example**:
```basic
' INCORRECT
OPEN "data.txt" FOR INPUT AS #999  ' Invalid file number

' CORRECT
OPEN "data.txt" FOR INPUT AS #1
```

**Solutions**:
- Use valid file numbers (1-255)
- Check file name validity
- Close unused files

---

### Error 53: File Not Found
**Message**: "File not found"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- File doesn't exist
- Incorrect file path
- File name typos

**Solutions**:
- Verify file existence
- Check file path
- Use full paths if needed

---

### Error 54: Bad File Mode
**Message**: "Bad file mode"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Incompatible file operations
- Wrong file access mode
- Invalid file operations

**Example**:
```basic
' INCORRECT
OPEN "data.txt" FOR INPUT AS #1
PRINT #1, "Hello"  ' Can't write to INPUT file

' CORRECT
OPEN "data.txt" FOR OUTPUT AS #1
PRINT #1, "Hello"
```

**Solutions**:
- Use correct file modes
- Check operation compatibility
- Close and reopen with correct mode

---

### Error 55: File Already Open
**Message**: "File already open"  
**Category**: File I/O  
**Severity**: Medium

**Causes**:
- Opening already open file
- File number conflicts

**Solutions**:
- Close file before reopening
- Use different file numbers
- Check file status

---

### Error 57: Device I/O Error
**Message**: "Device I/O error"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Hardware I/O failures
- Device not ready
- Communication errors

**Solutions**:
- Check device status
- Retry operations
- Handle hardware errors

---

### Error 58: File Already Exists
**Message**: "File already exists"  
**Category**: File I/O  
**Severity**: Medium

**Causes**:
- Creating existing files
- Name conflicts

**Solutions**:
- Check file existence
- Use different names
- Delete existing files if appropriate

---

### Error 61: Disk Full
**Message**: "Disk full"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Insufficient disk space
- Disk quota exceeded

**Solutions**:
- Free disk space
- Use different drive
- Compress files

---

### Error 62: Input Past End of File
**Message**: "Input past end of file"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Reading beyond file end
- EOF not checked

**Example**:
```basic
' INCORRECT
OPEN "data.txt" FOR INPUT AS #1
INPUT #1, x  ' May read past EOF

' CORRECT
OPEN "data.txt" FOR INPUT AS #1
IF NOT EOF(1) THEN
    INPUT #1, x
END IF
```

**Solutions**:
- Check EOF before reading
- Use proper file reading loops
- Handle end-of-file conditions

---

### Error 64: Bad File Name
**Message**: "Bad file name"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Invalid characters in file name
- Reserved file names
- Path too long

**Solutions**:
- Use valid file names
- Avoid reserved names
- Check path length

---

### Error 67: Too Many Files
**Message**: "Too many files"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Exceeding file handle limits
- Too many open files

**Solutions**:
- Close unused files
- Use file handles efficiently
- Increase system limits if possible

---

### Error 68: Device Unavailable
**Message**: "Device unavailable"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Device not connected
- Device offline
- Hardware issues

**Solutions**:
- Check device connection
- Restart device
- Use alternative devices

---

### Error 70: Permission Denied
**Message**: "Permission denied"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Insufficient permissions
- Read-only files
- Access restrictions

**Solutions**:
- Check file permissions
- Run with appropriate privileges
- Use different file locations

---

### Error 71: Disk Not Ready
**Message**: "Disk not ready"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Disk not inserted
- Drive not ready
- Media issues

**Solutions**:
- Insert disk
- Wait for drive ready
- Check media

---

### Error 72: Disk Media Error
**Message**: "Disk-media error"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Disk corruption
- Media failure
- Bad sectors

**Solutions**:
- Check disk integrity
- Use different media
- Run disk repair utilities

---

### Error 73: Feature Unavailable
**Message**: "Feature unavailable"  
**Category**: System  
**Severity**: Medium

**Causes**:
- Unsupported features
- Platform limitations
- Missing dependencies

**Solutions**:
- Check platform compatibility
- Use alternative features
- Install required components

---

### Error 74: Rename Across Disks
**Message**: "Rename across disks"  
**Category**: File I/O  
**Severity**: Medium

**Causes**:
- Renaming files across drives
- Cross-device operations

**Solutions**:
- Copy then delete
- Use same drive
- Implement cross-device rename

---

### Error 75: Path/File Access Error
**Message**: "Path/File access error"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Invalid paths
- Access violations
- Path not found

**Solutions**:
- Verify path validity
- Check access rights
- Use correct path format

---

### Error 76: Path Not Found
**Message**: "Path not found"  
**Category**: File I/O  
**Severity**: Critical

**Causes**:
- Non-existent directories
- Invalid paths
- Missing folders

**Solutions**:
- Create missing directories
- Use correct paths
- Verify directory structure

---

## Memory Management Errors

### Error 256: Out of Stack Space
**Message**: "Out of stack space"  
**Category**: Memory Management  
**Severity**: Critical

**Causes**:
- Deep recursion
- Excessive function calls
- Large local variables

**Example**:
```basic
' INCORRECT - Infinite recursion
FUNCTION Recursive(x)
    Recursive = Recursive(x + 1)
END FUNCTION

' CORRECT - With base case
FUNCTION Recursive(x)
    IF x > 1000 THEN
        Recursive = x
    ELSE
        Recursive = Recursive(x + 1)
    END IF
END FUNCTION
```

**Solutions**:
- Add recursion limits
- Use iteration instead of recursion
- Reduce local variable sizes
- Increase stack size if possible

---

### Error 257: Out of Memory (Fatal)
**Message**: "Out of memory"  
**Category**: Memory Management  
**Severity**: Fatal

**Causes**:
- System memory exhaustion
- Critical allocation failure
- Memory leaks

**Solutions**:
- Close other applications
- Free unused memory
- Restart system
- Use more efficient data structures

---

### Error 300-318: Memory Region Errors
**Category**: Memory Management  
**Severity**: Critical

These errors relate to the QB64-PE advanced memory management system:

- **300**: Memory region out of range
- **301**: Invalid size
- **302**: Source memory region out of range
- **303**: Destination memory region out of range
- **304**: Both memory regions out of range
- **305**: Source memory has been freed
- **306**: Destination memory has been freed
- **307**: Memory already freed
- **308**: Memory has been freed
- **309**: Memory not initialized
- **310**: Source memory not initialized
- **311**: Destination memory not initialized
- **312**: Both memory not initialized
- **313**: Both memory freed
- **314**: _ASSERT failed
- **315**: _ASSERT failed (check console for description)

**Causes**:
- Invalid memory operations
- Use-after-free errors
- Double-free errors
- Uninitialized memory access

**Solutions**:
- Initialize all memory before use
- Track memory allocation status
- Use memory debugging tools
- Implement proper memory management

---

## System and Library Errors

### Error 258: Invalid Handle
**Message**: "Invalid handle"  
**Category**: System  
**Severity**: Critical

**Causes**:
- Invalid file handles
- Closed handle usage
- Handle corruption

**Solutions**:
- Validate handles before use
- Check handle status
- Use proper handle management

---

### Error 259: Cannot Find Dynamic Library File
**Message**: "Cannot find dynamic library file"  
**Category**: System  
**Severity**: Critical

**Causes**:
- Missing DLL files
- Incorrect library paths
- Library dependencies

**Solutions**:
- Install required libraries
- Check library paths
- Verify library dependencies

---

### Error 260: Function Not Found in Dynamic Library
**Message**: "Function not found in dynamic library"  
**Category**: System  
**Severity**: Critical

**Causes**:
- Missing function exports
- Incorrect function names
- Library version mismatch

**Solutions**:
- Check library exports
- Verify function names
- Use correct library version

---

### Error 270: GL Command Outside Sub GL Scope
**Message**: "GL command outside sub gl scope"  
**Category**: Graphics  
**Severity**: Critical

**Causes**:
- OpenGL commands outside proper scope
- Invalid OpenGL context

**Solutions**:
- Use OpenGL commands in proper context
- Initialize OpenGL correctly
- Check OpenGL setup

---

### Error 271: End System in Sub GL Scope
**Message**: "End system in sub gl scope"  
**Category**: Graphics  
**Severity**: Critical

**Causes**:
- Improper OpenGL termination
- Context management errors

**Solutions**:
- Properly terminate OpenGL context
- Follow OpenGL lifecycle

---

## Error Handling Best Practices

### 1. Preventive Programming

```basic
' Example of robust error handling
FUNCTION SafeDivision(a, b)
    IF b = 0 THEN
        SafeDivision = 0  ' or raise error
        EXIT FUNCTION
    END IF
    SafeDivision = a / b
END FUNCTION

' Array bounds checking
SUB SafeArrayAccess(arr(), index, value)
    IF index >= LBOUND(arr) AND index <= UBOUND(arr) THEN
        arr(index) = value
    ELSE
        PRINT "Array index out of range"
    END IF
END SUB
```

### 2. Error Handling Structure

```basic
' ON ERROR GOTO example
ON ERROR GOTO ErrorHandler

' Main program code
OPEN "data.txt" FOR INPUT AS #1
INPUT #1, data
CLOSE #1

END

ErrorHandler:
    PRINT "Error"; ERR; "occurred at line"; ERL
    PRINT "Error message:"; _ERRORMESSAGE$
    RESUME NEXT
```

### 3. Modern Error Handling

```basic
' Using functions that return error codes
FUNCTION FileRead(filename$, content$)
    DIM result AS INTEGER
    result = 0  ' Success
    
    ON ERROR GOTO FileReadError
    
    OPEN filename$ FOR INPUT AS #1
    content$ = INPUT$(LOF(1), 1)
    CLOSE #1
    
    GOTO FileReadDone
    
FileReadError:
    result = ERR  ' Return error code
    
FileReadDone:
    FileRead = result
END FUNCTION
```

---

## Error Detection and Debugging

### Error Information Functions

```basic
' Get current error information
errorCode = ERR
errorLine = ERL
errorMessage$ = _ERRORMESSAGE$

' Include file error information
includeErrorLine = _INCLERRORLINE
includeFile$ = _INCLERRORFILE$
```

### Debugging Techniques

1. **Use Error Handlers**: Implement comprehensive error handling
2. **Log Errors**: Record errors for later analysis
3. **Validate Inputs**: Check parameters before use
4. **Test Edge Cases**: Test boundary conditions
5. **Use Debug Prints**: Add debugging output

---

## Common Error Patterns and Solutions

### Pattern 1: Array Index Errors
**Problem**: Frequent subscript out of range errors

**Solution**:
```basic
' Always check bounds
IF index >= LBOUND(array) AND index <= UBOUND(array) THEN
    value = array(index)
ELSE
    PRINT "Index out of range"
END IF
```

### Pattern 2: File Handle Conflicts
**Problem**: Bad file name or number errors

**Solution**:
```basic
' Track file handles
DIM fileHandles(10) AS INTEGER
DIM nextHandle AS INTEGER
nextHandle = 1

FUNCTION GetFreeHandle()
    GetFreeHandle = nextHandle
    nextHandle = nextHandle + 1
END FUNCTION
```

### Pattern 3: Memory Leaks
**Problem**: Out of memory errors over time

**Solution**:
```basic
' Explicit cleanup
SUB Cleanup()
    ' Close files
    IF fileOpen THEN CLOSE #fileHandle
    
    ' Free arrays
    ERASE dynamicArray
    
    ' Reset variables
    largeString$ = ""
END SUB
```

---

## Error Prevention Strategies

### 1. Code Review Checklist
- [ ] All arrays have bounds checking
- [ ] File operations include error handling
- [ ] Division operations check for zero
- [ ] String operations validate inputs
- [ ] Memory allocations are checked

### 2. Testing Strategies
- Test with valid and invalid inputs
- Test boundary conditions
- Test error recovery paths
- Test memory stress conditions
- Test file system edge cases

### 3. Defensive Programming
- Assume inputs may be invalid
- Validate all external data
- Use meaningful variable names
- Add comments for complex logic
- Implement logging

---

## Platform-Specific Considerations

### Windows
- File paths use backslashes
- Case-insensitive file names
- Drive letters in paths
- Windows-specific error codes

### Linux
- File paths use forward slashes
- Case-sensitive file names
- Unix-style permissions
- POSIX error codes

### macOS
- Similar to Linux
- App sandboxing restrictions
- Bundle-specific paths
- macOS-specific features

---

## Advanced Topics

### Custom Error Handling
```basic
' Define custom error codes
CONST CUSTOM_ERROR_BASE = 1000
CONST ERROR_INVALID_INPUT = CUSTOM_ERROR_BASE + 1
CONST ERROR_NETWORK_FAILURE = CUSTOM_ERROR_BASE + 2

' Custom error handler
SUB CustomErrorHandler(errorCode, description$)
    PRINT "Custom Error"; errorCode; ": "; description$
    ' Log error
    ' Send notification
    ' Attempt recovery
END SUB
```

### Error Recovery Strategies
1. **Retry Logic**: Attempt operation multiple times
2. **Fallback Options**: Use alternative methods
3. **Graceful Degradation**: Reduce functionality
4. **User Notification**: Inform user of issues
5. **Automatic Recovery**: Self-healing mechanisms

---

## Resources and References

### Internal Documentation
- `ARCHITECTURE.md` - System architecture
- `GETTING_STARTED.md` - Development setup
- Source code comments and documentation

### External Resources
- QB64-PE Wiki
- Community forums
- Bug tracking system
- Example code repositories

---

## Conclusion

Understanding QB64-PE's error handling system is crucial for developing robust applications. This encyclopedia provides comprehensive coverage of all error codes, their causes, and solutions. By following best practices and implementing proper error handling, developers can create more reliable and user-friendly applications.

Remember that errors are not just problems to be solved—they're opportunities to improve code quality and user experience. Use this reference as a guide to better error handling in your QB64-PE projects.

---

*This document is part of the QB64-PE documentation series. For additional information, see the Documentation Index.*
