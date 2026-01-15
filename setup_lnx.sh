#!/bin/bash
# QB64-PE Installer
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

#Change to the directory where the script is located
cd "$(dirname "$0")" || {
    print_error "Failed to change to script directory" \
        "Unable to navigate to the script's directory. Please ensure you have proper permissions."
    exit 1
}

dont_run="$1"

#This checks the currently installed packages for the one's QB64-PE needs
#And runs the package manager to install them if that is the case
pkg_install() {
  # Check if package manager command exists
  # Extract the actual package manager command (skip 'sudo' and flags)
  local cmd_name
  for word in $installer_command; do
    # Skip sudo, common flags, and the word "install"
    case "$word" in
      sudo|install)
        continue
        ;;
      -*)
        # Skip any flag (starts with -)
        continue
        ;;
      *)
        cmd_name="$word"
        break
        ;;
    esac
  done
  if [ -z "$cmd_name" ] || ! command -v "$cmd_name" >/dev/null 2>&1; then
    print_error "Package manager not found" \
        "The package manager '$cmd_name' is not available on your system.\nPlease install it manually or use a different package manager."
    return 1
  fi

  #Search
  packages_to_install=
  for pkg in $pkg_list; do
    if [ -z "$(echo "$installed_packages" | grep -w "$pkg")" ]; then
      packages_to_install="$packages_to_install $pkg"
    fi
  done
  if [ -n "$packages_to_install" ]; then
    echo "Installing required packages: $packages_to_install"
    echo "If prompted, please enter your password."
    if ! $installer_command $packages_to_install; then
      print_error "Package installation failed" \
          "Failed to install required packages. Please check the error messages above.\nYou may need to run: $installer_command $packages_to_install"
      return 1
    fi
  else
    echo "All required packages are already installed."
  fi
  return 0
}

#Make sure we're not running as root
if [ "$EUID" -eq 0 ]; then
  print_error "Running as root is not recommended" \
      "This script should not be run as root.\nIt will prompt you for your sudo password if needed to install packages.\nPlease run this script as a regular user."
  exit 1
fi

# Check for required basic tools
if ! command -v make >/dev/null 2>&1; then
  print_error "Make utility not found" \
      "The 'make' command is required but not found in your PATH.\nPlease install 'make' using your package manager before running this script."
  exit 1
fi

if ! command -v g++ >/dev/null 2>&1 && ! command -v gcc >/dev/null 2>&1; then
  print_warning "C++ compiler not found" \
      "Neither 'g++' nor 'gcc' was found in your PATH.\nThe script will attempt to install the required compiler packages."
fi

GET_WGET=
#Path to Icon
#Relative Path to icon -- Don't include beginning or trailing '/'
QB64_ICON_PATH="source"

#Name of the Icon picture
QB64_ICON_NAME="qb64pe.png"

DISTRO=

lsb_command=`which lsb_release 2> /dev/null`

#Outputs from lsb_command:

#Arch Linux  = arch
#Debian      = debian
#Fedora      = Fedora
#KUbuntu     = ubuntu
#LUbuntu     = ubuntu
#Linux Mint  = linuxmint
#Ubuntu      = ubuntu
#Slackware   = slackware
#VoidLinux   = voidlinux
#XUbuntu     = ubuntu
#Zorin       = Zorin
if [ -n "$lsb_command" ]; then
  DISTRO=`$lsb_command -si | tr '[:upper:]' '[:lower:]'`
elif [ -e /etc/arch-release ]; then
  DISTRO=arch
elif [ -e /etc/debian_version ] || [ -e /etc/debian_release ]; then
  DISTRO=debian
elif [ -e /etc/fedora-release ]; then
  DISTRO=fedora
elif [ -e /etc/redhat-release ]; then
  DISTRO=redhat
elif [ -e /etc/centos-release ]; then
  DISTRO=centos
fi

#Find and install packages
if [ "$DISTRO" == "arch" ]; then
  echo "ArchLinux detected."
  pkg_list="gcc make zlib curl $GET_WGET"
  if ! command -v pacman >/dev/null 2>&1; then
    print_error "pacman package manager not found" \
        "This script detected Arch Linux but pacman is not available.\nPlease ensure you're running this on a proper Arch Linux system."
    exit 1
  fi
  installed_packages=`pacman -Q 2>/dev/null`
  installer_command="sudo pacman -S --needed "
  if ! pkg_install; then
    print_error "Failed to install required packages" \
        "Please install the following packages manually:\n  $pkg_list"
    exit 1
  fi
elif [ "$DISTRO" == "linuxmint" ] || [ "$DISTRO" == "ubuntu" ] || [ "$DISTRO" == "debian" ] || [ "$DISTRO" == "zorin" ]; then
  echo "Debian based distro detected."
  pkg_list="build-essential x11-utils mesa-common-dev libglu1-mesa-dev libasound2-dev libpng-dev libcurl4-openssl-dev $GET_WGET"
  if ! command -v apt-get >/dev/null 2>&1; then
    print_error "apt-get package manager not found" \
        "This script detected a Debian-based distro but apt-get is not available.\nPlease ensure apt-get is installed and accessible."
    exit 1
  fi
  installed_packages=`dpkg -l 2>/dev/null`
  installer_command="sudo apt-get -y install "
  if ! pkg_install; then
    print_error "Failed to install required packages" \
        "Please install the following packages manually:\n  $pkg_list\n\nOr run: sudo apt-get update && sudo apt-get install $pkg_list"
    exit 1
  fi
elif [ "$DISTRO" == "fedora" ] || [ "$DISTRO" == "redhat" ] || [ "$DISTRO" == "centos" ]; then
  echo "Fedora/Redhat based distro detected."
  pkg_list="gcc-c++ make mesa-libGLU-devel alsa-lib-devel libpng-devel libcurl-devel $GET_WGET"
  # Try dnf first (Fedora 22+), fall back to yum
  if command -v dnf >/dev/null 2>&1; then
    installed_packages=`dnf list installed 2>/dev/null`
    installer_command="sudo dnf install -y "
  elif command -v yum >/dev/null 2>&1; then
    installed_packages=`yum list installed 2>/dev/null`
    installer_command="sudo yum install -y "
  else
    print_error "Package manager not found" \
        "Neither 'dnf' nor 'yum' is available on your system.\nPlease install one of these package managers."
    exit 1
  fi
  if ! pkg_install; then
    print_error "Failed to install required packages" \
        "Please install the following packages manually:\n  $pkg_list"
    exit 1
  fi
elif [ "$DISTRO" == "voidlinux" ]; then
  echo "VoidLinux detected."
  pkg_list="gcc make glu-devel libpng-devel alsa-lib-devel libcurl-devel $GET_WGET"
  if ! command -v xbps-install >/dev/null 2>&1; then
    print_error "xbps-install package manager not found" \
        "This script detected Void Linux but xbps-install is not available.\nPlease ensure you're running this on a proper Void Linux system."
    exit 1
  fi
  installed_packages=`xbps-query -l 2>/dev/null | grep -v libgcc`
  installer_command="sudo xbps-install -Sy "
  if ! pkg_install; then
    print_error "Failed to install required packages" \
        "Please install the following packages manually:\n  $pkg_list"
    exit 1
  fi

elif [ -z "$DISTRO" ]; then
  print_warning "Unable to detect Linux distribution" \
      "This script could not automatically detect your Linux distribution.\nPackage installation will be skipped."
  echo ""
  echo "For QB64-PE to compile, you will need the following installed:"
  echo "  - GNU C++ Compiler (g++)"
  echo "  - Make utility"
  echo "  - OpenGL development libraries (Mesa or equivalent)"
  echo "  - ALSA development libraries"
  echo "  - libpng development libraries"
  echo "  - libcurl development libraries"
  echo ""
  echo "Please install these packages using your distribution's package manager."
  echo ""
  # Skip interactive prompt in CI environments
  if [ -z "$CI" ] && [ -z "$CONTINUOUS_INTEGRATION" ] && [ -z "$GITHUB_ACTIONS" ] && [ -z "$GITLAB_CI" ] && [ -z "$JENKINS_URL" ] && [ -z "$TRAVIS" ] && [ -z "$CIRCLECI" ]; then
    read -p "Press Enter to continue with compilation (or Ctrl+C to exit)..."
  else
    echo "CI environment detected, continuing automatically..."
  fi
fi

# Verify critical tools are available after package installation
if ! command -v g++ >/dev/null 2>&1; then
  print_error "C++ compiler (g++) not found" \
      "The g++ compiler is required but not found in your PATH.\nPlease install g++ using your package manager:\n  - Debian/Ubuntu: sudo apt-get install g++\n  - Fedora/RHEL: sudo dnf install gcc-c++\n  - Arch: sudo pacman -S gcc"
  exit 1
fi

if ! command -v make >/dev/null 2>&1; then
  print_error "Make utility not found" \
      "The 'make' command is required but not found in your PATH.\nPlease install make using your package manager."
  exit 1
fi

echo "Compiling and installing QB64-PE..."
echo "This may take several minutes, please be patient..."
echo ""

# Check if Makefile exists
if [ ! -f "Makefile" ] && [ ! -f "makefile" ]; then
  print_error "Makefile not found" \
      "The Makefile is missing from the QB64-PE directory.\nPlease ensure you're running this script from the correct directory\nand that you have downloaded the complete QB64-PE source code."
  exit 1
fi

# Run make clean
echo "Cleaning previous build..."
if ! make clean OS=lnx >/dev/null 2>&1; then
  print_warning "Clean step had issues" \
      "The 'make clean' command encountered some issues, but continuing with build..."
fi

# Build QB64-PE
echo ""
echo "Building QB64-PE..."
if ! make OS=lnx BUILD_QB64=y -j3; then
  echo ""
  print_error "Build failed" \
      "The compilation process encountered errors.\nPlease review the error messages above for details.\n\nCommon issues:\n  - Missing development libraries\n  - Insufficient disk space\n  - Compiler errors in the source code\n\nIf you need help, please report the issue at:\n  https://github.com/QB64-Phoenix-Edition/QB64pe/issues\n\nInclude your distribution name: $DISTRO"
  exit 1
fi

if [ -e "./qb64pe" ]; then
  echo "Done compiling!!"

  echo "Creating ./run_qb64pe.sh script..."
  _pwd=`pwd`
  echo "#!/bin/sh" > ./run_qb64pe.sh
  echo "cd $_pwd" >> ./run_qb64pe.sh
  echo "./qb64pe &" >> ./run_qb64pe.sh

  chmod +x ./run_qb64pe.sh
  #chmod -R 777 ./
  echo "Adding QB64-PE menu entry..."
  # Ensure the applications directory exists
  mkdir -p ~/.local/share/applications 2>/dev/null
  cat > ~/.local/share/applications/qb64pe.desktop <<EOF
[Desktop Entry]
Name=QB64-PE Programming IDE
GenericName=QB64-PE Programming IDE
Exec=$_pwd/run_qb64pe.sh
Icon=$_pwd/$QB64_ICON_PATH/$QB64_ICON_NAME
Terminal=false
Type=Application
Categories=Development;IDE;
Path=$_pwd
StartupNotify=false
EOF

  if [ -z "$dont_run" ]; then
    echo "Running QB64-PE..."
    ./qb64pe &
  fi

  echo "QB64-PE is located in this folder:"
  echo "`pwd`"
  echo "There is a ./run_qb64pe.sh script in this folder that should let you run qb64pe if using the executable directly isn't working."
  echo
  echo "You should also find a QB64-PE option in the Programming/Development section of your menu you can use."
else
  ### QB64-PE didn't compile
  print_error "Build completed but executable not found" \
      "The build process completed, but the qb64pe executable was not created.\nThis usually indicates a linker error or missing dependencies."
  echo ""
  echo "Troubleshooting steps:"
  echo "  1. Review the compilation output above for error messages"
  echo "  2. Check that all required development libraries are installed:"
  echo "     - OpenGL/Mesa development libraries"
  echo "     - ALSA development libraries"
  echo "     - libpng development libraries"
  echo "     - libcurl development libraries"
  echo "  3. Ensure you have sufficient disk space"
  echo "  4. Try running 'make clean OS=lnx' and then rebuild"
  echo ""
  if [ -z "$DISTRO" ]; then
    echo "Note: Your distribution was not automatically detected."
    echo "You may need to manually install the required packages for your distribution."
  else
    echo "Detected distribution: $DISTRO"
    echo "If you're using a distribution not fully supported by this script,"
    echo "you may need to install equivalent packages manually."
  fi
  echo ""
  echo "If you need help, please report the issue at:"
  echo "  https://github.com/QB64-Phoenix-Edition/QB64pe/issues"
  echo ""
  echo "Please include:"
  echo "  - Your Linux distribution: $DISTRO"
  echo "  - The complete build output"
  echo "  - Any error messages shown during compilation"
  exit 1
fi
echo
echo "Thank you for using the QB64-PE installer."
