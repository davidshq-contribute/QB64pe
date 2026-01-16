# QB64 Phoenix Edition

**IMPORTANT: I was interested in refactoring QB64 to use a more modular, modern featureset but this proved difficult. I've abandoned this project and am experimenting with implementing a 100% compatible implementation of the language with backwards QBasic compatibility built in. Find it here: [https://github.com/davidshq/QB64Fresh](https://github.com/davidshq/QB64Fresh)**

# Table of Contents

1. [Installation](#installation)
    1. [Windows](#windows)
    2. [macOS](#macos)
    3. [Linux](#linux)

2. [Usage](#usage)
3. [Additional Information](#additional-information)

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

# Usage

Run the ```qb64pe``` executable to launch the IDE, which you can use to edit your .BAS files. From there, hit F5 to compile and run your code.

To generate a binary without running it, hit F11.

Additionally, if you do not wish to use the integrated IDE and to only compile your program, you can use the following command-line calls:

```qb64pe -c yourfile.bas```

```qb64pe -c yourfile.bas -o outputname.exe```

Replacing `-c` with `-x` will compile without opening a separate compiler window.

# Additional Information

More about QB64-PE at our wiki: <https://qb64phoenix.com/qb64wiki>

We have a community forum at: <https://qb64phoenix.com/forum>

Find us on Discord: <https://discord.gg/D2M7hepTSx>

Join us on Reddit: <https://www.reddit.com/r/QB64pe/>
