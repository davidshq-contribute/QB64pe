# QB64 Phoenix Edition

![QB64-PE](source/peLogo.png)

QB64 is a modern extended BASIC+OpenGL language that retains QB4.5/QBasic compatibility and compiles native binaries for Windows (7 and up), Linux and macOS (Catalina and up).

The [Phoenix Edition](https://www.qb64phoenix.com) is one of the new offshoots created when the old project related pages (QB64Team/www.qb64.org) went offline, but it's still the same programming language. For the whole story visit our new [Forum](https://qb64phoenix.com/forum/showthread.php?tid=259).

# Table of Contents

1. [Installation](#installation)
    1. [Windows](#windows)
    2. [macOS](#macos)
    3. [Linux](#linux)

2. [Usage](#usage)
3. [Troubleshooting](#troubleshooting)
4. [Additional Information](#additional-information)

# Installation

Download the appropriate package for your operating system over at <https://github.com/QB64-Phoenix-Edition/QB64pe/releases/latest>

## Windows

Make sure to extract the package contents to a folder with full write permissions (failing to do so may result in IDE or compilation errors).

* It is advisable to whitelist the 'qb64pe' folder in your antivirus/antimalware software *

## macOS

Before using QB64-PE make sure to install the Xcode command line tools with:

```bash
xcode-select --install
```

Run ```./setup_osx.command``` to compile QB64-PE for your OS version.

## Linux

Compile QB64-PE with ```./setup_lnx.sh```.

Dependencies should be automatically installed. Required packages include OpenGL, ALSA and the GNU C++ Compiler.

For detailed information about build dependencies on all platforms, see [Build Dependencies](docs/build-dependencies.md).

# Usage

Run the ```qb64pe``` executable to launch the IDE, which you can use to edit your .BAS files. From there, hit F5 to compile and run your code.

To generate a binary without running it, hit F11.

Additionally, if you do not wish to use the integrated IDE and to only compile your program, you can use the following command-line calls:

```qb64pe -c yourfile.bas```

```qb64pe -c yourfile.bas -o outputname.exe```

Replacing `-c` with `-x` will compile without opening a separate compiler window.

# Troubleshooting

This section covers common issues and their solutions.

## Installation Issues

### Windows

#### "IDE or compilation errors" after extraction
**Problem**: You extracted QB64-PE to a folder without write permissions (e.g., `C:\Program Files`).

**Solution**: Extract QB64-PE to a folder with full write permissions, such as:
- `C:\Users\YourName\Documents\QB64pe`
- `C:\QB64pe`
- Any folder in your user directory

#### Antivirus software blocking QB64-PE
**Problem**: Your antivirus or antimalware software quarantines or blocks QB64-PE files.

**Solution**: 
- Whitelist the entire `qb64pe` folder in your antivirus/antimalware software
- Add exceptions for `qb64pe.exe`, `qb64pe_bootstrap.exe`, and the `internal\c\c_compiler\` directory
- If files were quarantined, restore them and add to exclusions

#### MinGW setup fails during installation
**Problem**: The setup script fails when downloading or installing MinGW.

**Common causes and solutions**:
- **Network connectivity**: Check your internet connection and try again
- **Insufficient disk space**: Ensure you have at least 500MB free space
- **Antivirus blocking**: Temporarily disable antivirus or add QB64-PE folder to exclusions
- **Insufficient permissions**: Run the setup script as administrator if needed
- **Firewall blocking**: Allow the setup script through your firewall

#### "C++ compiler not found" error
**Problem**: After running setup, the compiler is not found at `internal\c\c_compiler\bin\c++.exe`.

**Solution**:
- Verify that MinGW setup completed successfully
- Check if antivirus software quarantined any files in `internal\c\c_compiler\`
- Try running `setup_mingw.cmd` manually
- Re-download and extract the complete QB64-PE package

#### "Command Prompt extensions not available" error
**Problem**: Setup script reports that command extensions are not available.

**Solution**: Run the setup script in a standard Windows Command Prompt (cmd.exe), not PowerShell or a restricted environment.

### macOS

#### "Apple's C++ compiler not found" error
**Problem**: Setup script cannot find `clang++`.

**Solution**: Install Xcode Command Line Tools:
```bash
xcode-select --install
```
After installation completes, run the setup script again.

#### Setup script fails to compile
**Problem**: Compilation fails with errors during setup.

**Solution**:
- Ensure Xcode Command Line Tools are fully installed (run `xcode-select --install` again)
- Check that you have sufficient disk space
- Verify you're running macOS Catalina (10.15) or later
- Try running `make OS=osx clean` followed by `make OS=osx BUILD_QB64=y`

### Linux

#### "Unable to detect distro" message
**Problem**: Setup script cannot detect your Linux distribution.

**Solution**: The script will skip automatic package installation. You need to manually install:
- OpenGL development libraries (mesa-common-dev, libglu1-mesa-dev)
- ALSA development libraries (libasound2-dev)
- GNU C++ Compiler (g++ or gcc-c++)
- libpng development libraries (libpng-dev)
- libcurl development libraries (libcurl4-openssl-dev or libcurl-devel)
- Build tools (build-essential or equivalent)

Install these using your distribution's package manager before running the setup script.

#### Setup script fails to compile
**Problem**: Compilation fails with missing dependency errors.

**Solution**:
- Ensure all required packages are installed (see above)
- For Debian/Ubuntu: `sudo apt-get install build-essential mesa-common-dev libglu1-mesa-dev libasound2-dev libpng-dev libcurl4-openssl-dev`
- For Fedora/RHEL: `sudo yum install gcc-c++ make mesa-libGLU-devel alsa-lib-devel libpng-devel libcurl-devel`
- For Arch: `sudo pacman -S gcc make zlib curl`
- Check the error messages for specific missing libraries

#### "You are trying to run this script as root" error
**Problem**: Setup script detects you're running as root.

**Solution**: Do not run the setup script as root. Run it as a regular user; the script will prompt for your sudo password when needed to install packages.

## Usage Issues

### IDE won't launch
**Problem**: Double-clicking `qb64pe` (or `qb64pe.exe` on Windows) doesn't start the IDE.

**Solutions**:
- **Windows**: Check if antivirus software is blocking execution
- **Linux**: Ensure the file has execute permissions: `chmod +x qb64pe`
- **Linux**: Try running from terminal: `./qb64pe` or use the provided `./run_qb64pe.sh` script
- Check that all required files are present in the QB64-PE directory
- Try running from a terminal/command prompt to see error messages

### Compilation fails with errors
**Problem**: Your QB64 program fails to compile.

**Solutions**:
- Check the error messages in the compiler window for specific issues
- Verify your code syntax is correct
- Ensure all required files (images, data files, etc.) are in the correct locations
- Check that you're using valid QB64 keywords and functions
- Try compiling a simple test program (e.g., `PRINT "Hello, World!"`) to verify QB64-PE is working

### Compiled program won't run
**Problem**: Your program compiles successfully but crashes or won't execute.

**Solutions**:
- Check for runtime errors in the console output
- Verify all required files (data files, images, etc.) are accessible
- On Windows, check if antivirus is blocking the executable
- Try running the executable from a command prompt/terminal to see error messages
- Ensure you have the necessary system libraries installed

### Command-line compilation issues
**Problem**: Using `qb64pe -c` or `qb64pe -x` doesn't work as expected.

**Solutions**:
- Ensure you're in the correct directory or provide full paths
- Check that the `.bas` file exists and is readable
- Verify QB64-PE is in your PATH, or use the full path to `qb64pe`
- On Windows, use `qb64pe.exe` instead of `qb64pe`
- Check file permissions on the source file

## Getting More Help

If you continue to experience issues:

1. **Check the documentation**: Visit the [QB64 Wiki](https://qb64phoenix.com/qb64wiki) for detailed information
2. **Search the forum**: Many common issues are discussed on the [QB64 Phoenix Edition Forum](https://qb64phoenix.com/forum)
3. **Ask for help**: Post on the forum or Discord with:
   - Your operating system and version
   - The exact error messages you're seeing
   - Steps you've already tried
   - What you were trying to do when the error occurred
4. **Report bugs**: If you believe you've found a bug, report it on [GitHub Issues](https://github.com/QB64-Phoenix-Edition/QB64pe/issues)

# Additional Information

More about QB64-PE at our wiki: <https://qb64phoenix.com/qb64wiki>

We have a community forum at: <https://qb64phoenix.com/forum>

Find us on Discord: <https://discord.gg/D2M7hepTSx>

Join us on Reddit: <https://www.reddit.com/r/QB64pe/>
