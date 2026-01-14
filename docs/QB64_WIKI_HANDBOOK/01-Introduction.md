# Introduction to QB64-PE

## What is QB64-PE?

QB64-PE (Phoenix Edition) is a modern 64-bit BASIC compiler and IDE that is compatible with traditional QBasic syntax while adding many modern features and capabilities. It allows you to write BASIC programs that can run on modern Windows, Linux, and macOS systems.

## Key Features

### **QBasic Compatibility**
- Almost 100% compatible with QBasic 4.5 and QuickBASIC
- Existing QBasic code can be compiled and run with minimal changes
- Maintains the familiar BASIC syntax and structure

### **Modern Enhancements**
- **64-bit support** for accessing modern system resources
- **OpenGL integration** for advanced 3D graphics
- **Network capabilities** including TCP/IP and HTTP functions
- **Advanced graphics** with 32-bit color and alpha blending
- **Sound system** with MIDI and digital audio support
- **File handling** with modern file system access

### **Cross-Platform**
- Runs on Windows, Linux, and macOS
- Creates native executables for each platform
- Consistent behavior across operating systems

### **Modern Development Features**
- **Integrated Development Environment** with syntax highlighting
- **Built-in help system** with complete keyword reference
- **Debugging tools** including breakpoints and watch variables
- **Error handling** with detailed error messages
- **Memory management** with dynamic allocation

## The Help System

QB64-PE includes a comprehensive built-in help system that provides:

- **Complete keyword reference** for all commands and functions
- **Syntax examples** with practical code samples
- **Cross-references** between related commands
- **Search capabilities** to quickly find information
- **Wiki integration** with online content updates

### Accessing Help

You can access the help system in several ways:

1. **Help Menu**: Use the Help menu in the IDE
2. **F1 Key**: Press F1 while the cursor is on a keyword
3. **Shift+F1**: Open the help window with the last viewed article
4. **Context Help**: Right-click on keywords for context-sensitive help

### Help Menu Options

The Help menu provides quick access to important reference materials:

- **View** (Shift+F1): Opens help with last viewed article
- **Contents Page**: Main help contents and navigation
- **Keywords Index**: Alphabetical listing of all keywords
- **Keywords by Usage**: Keywords organized by functionality
- **Metacommands**: QB64-specific compiler directives
- **Variable Types**: Data type reference
- **Update Current Page**: Refresh current help page from wiki
- **Update All Pages**: Download latest wiki content
- **View Current Page On Wiki**: Open current page in web browser

## Getting Started

### Basic Program Structure

```basic
' This is a comment
PRINT "Hello, World!"
END
```

### Variables and Data Types

```basic
DIM name AS STRING
DIM age AS INTEGER
DIM balance AS SINGLE

name = "John Doe"
age = 25
balance = 1234.56

PRINT "Name: "; name
PRINT "Age: "; age
PRINT "Balance: "; balance
```

### Control Structures

```basic
' IF...THEN statement
IF age >= 18 THEN
    PRINT "You are an adult"
ELSE
    PRINT "You are a minor"
END IF

' FOR...NEXT loop
FOR i = 1 TO 10
    PRINT "Count: "; i
NEXT i

' DO...LOOP loop
DO WHILE age < 30
    age = age + 1
    PRINT "Age: "; age
LOOP
```

## QB64-Specific Features

### Modern Data Types

QB64-PE introduces several new data types:

- **_BIT**: Single bit values (0 or -1)
- **_BYTE**: 8-bit signed integers (-128 to 127)
- **_UNSIGNED**: Unsigned variants of integer types
- **_INTEGER64**: 64-bit integers
- **_FLOAT**: High-precision floating point

### Enhanced Graphics

```basic
SCREEN _NEWIMAGE(800, 600, 32)  ' 32-bit color screen
_COLOR _RGB(255, 0, 0)          ' Red color
PSET (100, 100)                 ' Draw a pixel
CIRCLE (400, 300), 50           ' Draw a circle
```

### Network Functions

```basic
' HTTP request
response$ = _OPENCLIENT("HTTP://example.com")
PRINT response$

' TCP connection
handle = _OPENHOST("TCP", "localhost", 8080)
IF _CONNECTED(handle) THEN
    PRINT "Connected successfully"
END IF
```

## Community Resources

### Online Resources

- **QB64 Phoenix Edition Wiki**: https://qb64phoenix.com/qb64wiki
- **QB64 Phoenix Edition Forum**: https://qb64phoenix.com/forum
- **QB64 Phoenix Edition Discord**: https://discord.gg/D2M7hepTSx
- **QB64 Phoenix Edition Reddit**: https://www.reddit.com/r/QB64pe/

### Contributing

The QB64-PE project is community-driven. You can contribute by:

- **Reporting bugs** on the forum or GitHub
- **Suggesting features** for future versions
- **Sharing code** and examples with the community
- **Helping other users** on the forum and Discord
- **Improving documentation** on the wiki

## System Requirements

### Minimum Requirements
- **Windows**: Windows 7 or later (64-bit)
- **Linux**: Most modern distributions (64-bit)
- **macOS**: macOS 10.12 or later (64-bit)
- **Memory**: 512 MB RAM minimum
- **Storage**: 100 MB free space

### Recommended Requirements
- **Memory**: 2 GB RAM or more
- **Storage**: 500 MB free space for development
- **Graphics**: OpenGL 2.0+ compatible for advanced graphics

## Installation

### Windows
1. Download the QB64-PE package from the official website
2. Extract the ZIP file to a folder of your choice
3. Run `qb64pe.exe` to start the IDE

### Linux
1. Download the Linux package
2. Extract the archive: `tar -xzf qb64pe-linux.tar.gz`
3. Run the setup script: `./setup_lnx.sh`
4. Start QB64-PE: `./qb64pe`

### macOS
1. Download the macOS package
2. Extract the archive
3. Run the setup script: `./setup_osx.command`
4. Start QB64-PE from the Applications folder

## Next Steps

This handbook contains comprehensive documentation for all QB64-PE features. You can:

1. **Browse the keyword reference** to learn about specific commands
2. **Read the FAQ** for common questions and answers
3. **Explore the examples** throughout the documentation
4. **Join the community** to get help and share your projects

Welcome to QB64-PE! Happy programming!
