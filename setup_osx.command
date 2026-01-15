# Argument 1: If not blank, qb64pe will not be started after compilation

# Function to print error messages
print_error() {
    echo ""
    echo "========================================"
    echo "ERROR: $1"
    echo "========================================"
    echo ""
    [ -n "$2" ] && echo "$2"
    echo ""
}

# Function to print warning messages
print_warning() {
    echo ""
    echo "WARNING: $1"
    echo ""
    [ -n "$2" ] && echo "$2"
    echo ""
}

# Change to the directory where the script is located
cd "$(dirname "$0")" || {
    print_error "Failed to change to script directory" \
        "Unable to navigate to the script's directory. Please ensure you have proper permissions."
    exit 1
}

dont_run="$1"

Pause() {
  OLDCONFIG=`stty -g`
  stty -icanon -echo min 1 time 0
  dd count=1 2>/dev/null
  stty $OLDCONFIG
}

echo "QB64-PE Setup"
echo ""

# Make command files executable
find . -name "*.command" -exec chmod +x {} \; 2>/dev/null

# Check for required tools
if ! command -v clang++ >/dev/null 2>&1; then
  print_error "C++ compiler (clang++) not found" \
      "Apple's C++ compiler (clang++) is required but not found in your PATH."
  echo ""
  echo "Attempting to install Apple's Command Line Tools for Xcode..."
  echo "This will open a dialog window. Please follow the instructions to install."
  echo ""
  
  if ! command -v xcode-select >/dev/null 2>&1; then
    print_error "xcode-select not found" \
        "The xcode-select command is not available.\nThis is unusual on macOS. Please ensure you're running this on a Mac system."
    [ -z "$dont_run" ] && Pause
    exit 1
  fi
  
  # xcode-select --install returns 0 if dialog opens, non-zero if already installed or error
  if xcode-select --install 2>&1; then
    echo "Installation dialog should have appeared. After installation completes, run this setup script again."
  else
    # Check if tools are already installed by trying to find clang
    if command -v clang >/dev/null 2>&1; then
      print_warning "Command Line Tools may already be installed" \
          "The installation dialog did not appear, but clang was found.\nThe tools may need to be configured. Try running:\n  xcode-select --reset"
      echo ""
      echo "If clang++ is still not available, you may need to install Command Line Tools manually."
    else
      print_warning "Command Line Tools installation failed or dialog already open" \
          "The installation dialog did not appear.\nIf a dialog is already open, please complete that installation.\nOtherwise, you may need to install Command Line Tools manually from:\n  https://developer.apple.com/xcode/"
    fi
    echo ""
    echo "After installation completes, run this setup script again."
  fi
  
  [ -z "$dont_run" ] && Pause
  exit 1
fi

# Verify make is available
if ! command -v make >/dev/null 2>&1; then
  print_error "Make utility not found" \
      "The 'make' command is required but not found in your PATH.\nThis should be included with Xcode Command Line Tools.\nPlease ensure Command Line Tools are properly installed."
  [ -z "$dont_run" ] && Pause
  exit 1
fi

# Check if Makefile exists
if [ ! -f "Makefile" ] && [ ! -f "makefile" ]; then
  print_error "Makefile not found" \
      "The Makefile is missing from the QB64-PE directory.\nPlease ensure you're running this script from the correct directory\nand that you have downloaded the complete QB64-PE source code."
  [ -z "$dont_run" ] && Pause
  exit 1
fi

echo "Building 'QB64-PE'"
echo "This may take several minutes, please be patient..."
echo ""

# Run make clean
echo "Cleaning previous build..."
if ! make OS=osx clean 2>&1; then
  print_warning "Clean step had issues" \
      "The 'make clean' command encountered some issues, but continuing with build..."
fi

# Build QB64-PE
echo ""
echo "Compiling QB64-PE..."
if ! make OS=osx BUILD_QB64=y -j3; then
  echo ""
  print_error "Build failed" \
      "The compilation process encountered errors.\nPlease review the error messages above for details.\n\nCommon issues:\n  - Missing Xcode Command Line Tools\n  - Insufficient disk space\n  - Compiler errors in the source code\n  - Missing system libraries\n\nTroubleshooting:\n  1. Ensure Xcode Command Line Tools are fully installed:\n     Run: xcode-select --install\n  2. Check available disk space\n  3. Review the compilation output above for specific errors\n\nIf you need help, please report the issue at:\n  https://github.com/QB64-Phoenix-Edition/QB64pe/issues"
  [ -z "$dont_run" ] && Pause
  exit 1
fi

echo ""
if [ -f ./qb64pe ]; then
  echo "========================================"
  echo "Build successful!"
  echo "========================================"
  echo ""
  
  if [ -z "$dont_run" ]; then
    echo "Launching 'QB64-PE'..."
    if ! ./qb64pe; then
      print_warning "QB64-PE launched but exited with an error" \
          "QB64-PE was built successfully but encountered a runtime error.\nThis may indicate a system compatibility issue."
    fi
  fi
  
  echo ""
  echo "Note: 'qb64pe' is located in the same folder as this setup program."
  echo "You can run it by executing: ./qb64pe"
  echo ""
  [ -z "$dont_run" ] && echo "Press any key to continue..." && Pause
  exit 0
else
  print_error "Build completed but executable not found" \
      "The build process completed, but the qb64pe executable was not created.\nThis usually indicates a linker error or missing dependencies."
  echo ""
  echo "Troubleshooting steps:"
  echo "  1. Review the compilation output above for error messages"
  echo "  2. Ensure Xcode Command Line Tools are fully installed"
  echo "  3. Check that you have sufficient disk space"
  echo "  4. Try running 'make clean OS=osx' and then rebuild"
  echo "  5. Verify your macOS version is supported"
  echo ""
  echo "If you need help, please report the issue at:"
  echo "  https://github.com/QB64-Phoenix-Edition/QB64pe/issues"
  echo ""
  echo "Please include:"
  echo "  - Your macOS version (run: sw_vers)"
  echo "  - The complete build output"
  echo "  - Any error messages shown during compilation"
  [ -z "$dont_run" ] && Pause
  exit 1
fi
