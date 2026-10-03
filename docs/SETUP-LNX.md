# Linux Setup Script (setup_lnx.sh)

This document describes step by step what happens when you run `./setup_lnx.sh` on Linux. The script installs build dependencies, compiles QB64-PE from the pre-generated C++ in `internal/source/`, and optionally launches the IDE.

**Usage:**

```bash
./setup_lnx.sh           # Build and then start QB64-PE
./setup_lnx.sh <any>     # Build only; do not start QB64-PE
```

---

## Step-by-step flow

### 1. Change to script directory

The script changes to the directory where it lives (the repository root):

```bash
cd "$(dirname "$0")"
```

So all later commands run from the QB64-PE repo root.

### 2. Root check

If the script is run as root (`EUID == 0`), it exits with an error. It is intended to be run as a normal user; it will use `sudo` only when installing packages.

### 3. Distro detection

The script detects the Linux distribution to choose the right package manager and package names:

| Detection method | Resulting `DISTRO` |
|------------------|--------------------|
| `lsb_release -si` (lowercased) | arch, debian, Fedora, ubuntu, linuxmint, zorin, slackware, voidlinux, Zorin |
| `/etc/arch-release` exists | arch |
| `/etc/debian_version` or `/etc/debian_release` | debian |
| `/etc/fedora-release` | fedora |
| `/etc/redhat-release` | redhat |
| `/etc/centos-release` | centos |

If none match, `DISTRO` is left empty and package installation is skipped (see step 5).

### 4. Package installation helper

A function `pkg_install()` is defined (but not yet run). It:

- Compares a list of required packages (`pkg_list`) against currently installed packages.
- For any package not found in the installed list, adds it to `packages_to_install`.
- If there are packages to install, runs the distro’s installer command (with `sudo` if needed). The user may be prompted for a password.

Package lists and installer commands are set per distro in the next step.

### 5. Install dependencies by distro

Depending on `DISTRO`, the script sets `pkg_list` and the installer command, then calls `pkg_install()`:

| Distro | Packages installed | Installer |
|--------|--------------------|-----------|
| **Arch** | gcc, make, zlib, curl | `sudo pacman -S` |
| **Debian/Ubuntu/Mint/Zorin** | build-essential, x11-utils, mesa-common-dev, libglu1-mesa-dev, libasound2-dev, libpng-dev, libcurl4-openssl-dev | `sudo apt-get -y install` |
| **Fedora/Red Hat/CentOS** | gcc-c++, make, mesa-libGLU-devel, alsa-lib-devel, libpng-devel, libcurl-devel | `sudo yum install` |
| **Void** | gcc, make, glu-devel, libpng-devel, alsa-lib-devel, libcurl-devel | `sudo xbps-install -Sy` |
| **Unknown** | (none) | — |

If `DISTRO` is empty, the script prints that it could not detect the distro, skips package installation, and lists what you need manually: OpenGL dev libs, ALSA dev libs, GNU C++ (g++), and libpng.

### 6. Clean previous build

The script runs:

```bash
make clean OS=lnx
```

This removes build artifacts (e.g. object files, `internal/temp` contents) so the next build is from a clean state. See [BUILD-SYSTEM.md](BUILD-SYSTEM.md) for how `clean` and the Makefile work.

### 7. Build QB64-PE

The script runs:

```bash
make OS=lnx BUILD_QB64=y -j3
```

- **`OS=lnx`** — Linux target (system GCC, paths, and options).
- **`BUILD_QB64=y`** — Build the QB64-PE binary from `internal/source/` (pre-generated C++), not from a user `.bas` file. The Makefile copies `internal/source/` into `internal/temp/` and builds the executable (default name `qb64pe`). Dependencies such as fonts, icon, sockets, HTTP, console, and zlib are enabled automatically.
- **`-j3`** — Up to 3 parallel jobs.

The result is the `qb64pe` executable in the repo root, if the build succeeds.

### 8. If the build succeeded (`./qb64pe` exists)

#### 8a. Create `run_qb64pe.sh`

A small launcher script is written to `./run_qb64pe.sh`:

- Shebang: `#!/bin/sh`
- `cd` to the current directory (repo root)
- Run `./qb64pe &` (background)

The file is made executable (`chmod +x`).

#### 8b. Add desktop menu entry

A desktop entry is created at:

`~/.local/share/applications/qb64pe.desktop`

It registers “QB64-PE Programming IDE” in the application menu (Development/IDE), with:

- **Exec:** `run_qb64pe.sh` in the repo directory
- **Icon:** `source/qb64pe.png`
- **Path:** repo root
- **Terminal:** false

So you can start QB64-PE from the system menu without opening a terminal.

#### 8c. Optionally run QB64-PE

If the script was invoked **without** an argument (`dont_run` is empty), it starts QB64-PE in the background:

```bash
./qb64pe &
```

If you passed any argument (e.g. `./setup_lnx.sh no`), this step is skipped.

#### 8d. Final message

The script prints where QB64-PE is installed (current directory), mentions `run_qb64pe.sh`, and that a menu entry was added under Programming/Development.

### 9. If the build failed (no `./qb64pe`)

The script prints that the executable was not created (likely compile errors), suggests missing packages or unsupported distro, and asks you to report the issue (including the line `DISTRO: $DISTRO`) on the QB64 Phoenix Edition Forums.

### 10. Closing message

In all cases, the script ends with:

```text
Thank you for using the QB64-PE installer.
```

---

## Summary diagram

```
./setup_lnx.sh [dont_run?]
       │
       ├─ cd to repo root
       ├─ Exit if root
       ├─ Detect DISTRO (lsb_release / /etc/*-release)
       ├─ pkg_install() for that distro (or skip if unknown)
       ├─ make clean OS=lnx
       ├─ make OS=lnx BUILD_QB64=y -j3
       │
       ├─ If ./qb64pe exists:
       │     ├─ Create run_qb64pe.sh (cd + ./qb64pe &)
       │     ├─ Add ~/.local/share/applications/qb64pe.desktop
       │     ├─ ./qb64pe &  (unless dont_run was set)
       │     └─ Print success and paths
       │
       └─ Else: Print failure message and DISTRO
```

---

## Related documentation

- [BUILD-SYSTEM.md](BUILD-SYSTEM.md) — Makefile, `BUILD_QB64`, `OS`, and how QB64-PE is built from `internal/source/`.
- [CLAUDE.md](../CLAUDE.md) — Project overview and other setup/build commands.
