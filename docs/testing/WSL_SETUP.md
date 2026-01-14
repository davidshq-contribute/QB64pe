# Running QB64 Tests in WSL (Windows Subsystem for Linux)

## Why Use WSL for Testing?

On Windows, QB64 console applications open in a separate console window and may show error dialogs that **require user interaction**. This makes automated testing difficult because:

1. A new console window pops up
2. Error dialogs appear (even for expected test errors)
3. User must click "OK" or "Cancel" to continue
4. Tests cannot run unattended

**Solution:** Run tests in WSL (Linux environment) where there are no GUI dialogs.

## Prerequisites

- Windows 10/11 with WSL enabled
- Ubuntu (or other Linux distribution) installed in WSL
- QB64-PE source code accessible from WSL

## One-Time Setup

### Step 1: Verify WSL Installation

```bash
wsl --status
```

If WSL is not installed, follow [Microsoft's WSL installation guide](https://docs.microsoft.com/en-us/windows/wsl/install).

### Step 2: Install Build Dependencies in WSL

Open WSL and install required packages:

```bash
wsl

# Inside WSL:
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    x11-utils \
    mesa-common-dev \
    libglu1-mesa-dev \
    libasound2-dev \
    libpng-dev \
    libcurl4-openssl-dev \
    git
```

**Note:** `libglu1-mesa-dev` is critical - it provides the GLU (OpenGL Utility Library) headers needed for compilation.

### Step 3: Build QB64-PE for Linux

From your Windows directory (WSL can access Windows files at `/mnt/c/`):

```bash
# Inside WSL:
cd /mnt/c/code/qb64contain/QB64pe

# Build QB64-PE for Linux
./setup_lnx.sh
```

This will create a `qb64pe` Linux binary. The build process takes 5-10 minutes.

### Step 4: Compile Test Runner

```bash
# Inside WSL:
cd /mnt/c/code/qb64contain/QB64pe

# Compile the test runner for Linux
./qb64pe -x tests/unit/test_runner.bas -o test_runner
```

## Running Tests

### Running Unit Tests (WSL)

The unit test runner is located at `tests/unit/run_tests_wsl.sh`. This script runs only the unit tests.

#### Method 1: From Windows (Recommended)

**From the project root directory:**

**Git Bash:**
```bash
wsl bash tests/unit/run_tests_wsl.sh
```

**Windows Command Prompt:**
```cmd
wsl bash tests/unit/run_tests_wsl.sh
```

**Or use the batch wrapper:**
```cmd
tests\unit\run_tests_wsl.bat
```

#### Method 2: From Inside WSL

```bash
wsl
cd /mnt/c/code/qb64contain/QB64pe
bash tests/unit/run_tests_wsl.sh
```

**Note:** The script automatically detects the project root, so you can run it from anywhere within the project.

### Running Full Test Suite

The main test runner (`tests/run_tests.sh`) runs all tests including compiler tests, format tests, integration tests, etc.

**From the project root:**
```bash
# From Windows (Git Bash or PowerShell)
wsl bash tests/run_tests.sh

# Or from inside WSL
cd /mnt/c/code/qb64contain/QB64pe
bash tests/run_tests.sh
```

**Note:** The `tests/run_tests.sh` script can be run from either the project root or from within the `tests/` directory - it will automatically detect the correct paths.

## Expected Output

```
=== QB64-PE Unit Test Runner (WSL) ===

Running tests...

=== Test Results ===

=== Test Summary ===
Total tests: 73
Passed: 73
Failed: 0
Skipped: 0

Total assertions: 73
Passed: 73
Failed: 0

ALL TESTS PASSED

✓ All tests passed!
```

## Troubleshooting

### Common Issues

* **"qb64pe: command not found" or "qb64pe executable not found"**: QB64-PE hasn't been built for Linux yet. Run `./setup_lnx.sh` to build it.
* **"Permission denied" when running scripts**: Make scripts executable with `chmod +x tests/unit/run_tests_wsl.sh` and `chmod +x tests/run_tests.sh`.
* **Build Errors During setup_lnx.sh**: Ensure all required dependencies are installed. If you see a "GL/glu.h: No such file or directory" error, install `libglu1-mesa-dev` with `sudo apt-get install -y libglu1-mesa-dev`.

## Comparison: Windows vs WSL Testing

| Feature | Windows Native | WSL Linux |
|---------|---------------|-----------|
| Separate console window | ✅ Yes (pops up) | ❌ No |
| Error dialogs | ✅ Yes (requires clicks) | ❌ No |
| User interaction needed | ✅ Yes | ❌ No |
| Automated testing | ⚠️ Limited | ✅ Full |
| CI/CD compatible | ⚠️ Limited | ✅ Yes |
| Setup required | ❌ None | ✅ One-time build |
| Speed | Normal | Faster |

## Recommendation

- **Unit Tests (Development):** Use WSL (`tests/unit/run_tests_wsl.sh`) for fast, automated unit testing
- **Full Test Suite:** Use WSL (`tests/run_tests.sh`) to run all tests including compiler, format, and integration tests
- **Quick Checks:** Windows native is fine if you don't mind clicking through dialogs
- **CI/CD:** Always use WSL or native Linux

**Script Locations:**
- Unit tests only: `tests/unit/run_tests_wsl.sh`
- Full test suite: `tests/run_tests.sh` (works from project root or tests/ directory)

## Alternative: Native Linux

If you have a native Linux machine or VM, you can run tests there directly:

```bash
cd /path/to/QB64pe
./setup_lnx.sh
./qb64pe -x tests/unit/test_runner.bas -o test_runner
./test_runner
cat test_results.txt
```

No special setup needed!
