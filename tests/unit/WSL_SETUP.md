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

**Important:** Always run `apt-get update` first to refresh package lists before installing.

The `setup_lnx.sh` script requires these packages for Ubuntu/Debian-based distributions:

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

**Note:** `libglu1-mesa-dev` is critical - it provides the GLU (OpenGL Utility Library) headers needed for compilation. Without it, you'll get `GL/glu.h: No such file or directory` errors.

### Step 3: Build QB64-PE for Linux

From your Windows directory (WSL can access Windows files at `/mnt/c/`):

```bash
# Inside WSL:
cd /mnt/c/code/qb64contain/QB64pe

# Build QB64-PE for Linux
./setup_lnx.sh
```

This will create the `qb64pe` Linux binary. The build process takes 5-10 minutes.

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

## Benefits of WSL Testing

✅ **No GUI Dialogs** - Tests run in pure terminal environment
✅ **Fully Automated** - No user interaction required
✅ **CI/CD Ready** - Can run in automated pipelines
✅ **Faster** - No window creation overhead
✅ **True Cross-Platform** - Tests run in actual Linux environment

## Troubleshooting

### "qb64pe: command not found" or "qb64pe executable not found"

QB64-PE hasn't been built for Linux yet. Run:
```bash
cd /mnt/c/code/qb64contain/QB64pe
./setup_lnx.sh
```

**Note:** Make sure you're running the test scripts from the project root directory, or the scripts will not be able to find the `qb64pe` executable.

### "Permission denied" when running scripts

Make scripts executable:
```bash
chmod +x tests/unit/run_tests_wsl.sh
chmod +x tests/run_tests.sh
```

### "No such file or directory" when running tests/run_tests.sh

If you see an error like `./tests/test_utils.sh: No such file or directory`:

1. **Make sure you're in the project root directory** (where `qb64pe` or `qb64pe.exe` is located)
2. The script should work from either the root or the `tests/` directory - if it doesn't, try running from the root:
   ```bash
   cd /mnt/c/code/qb64contain/QB64pe  # or your project root
   bash tests/run_tests.sh
   ```

### "Cannot access '/mnt/c/...'"

Your Windows drive may be mounted differently. Check with:
```bash
ls /mnt/
```

Adjust the path in `run_tests_wsl.bat` accordingly.

### Build Errors During setup_lnx.sh

#### "GL/glu.h: No such file or directory" Error

If you see this error:
```
fatal error: GL/glu.h: No such file or directory
```

**Solution:** Install the missing `libglu1-mesa-dev` package:

```bash
sudo apt-get update
sudo apt-get install -y libglu1-mesa-dev
```

Then retry the build:
```bash
./setup_lnx.sh
```

#### Other Build Errors

If you encounter other compilation errors, ensure all required dependencies are installed:

```bash
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

**Note:** The `setup_lnx.sh` script should automatically install these packages, but if it doesn't (or if you're on an unsupported distribution), install them manually.

### "E: Unable to fetch some archives" Error

If you see this error when running `sudo apt install`:

```
E: Unable to fetch some archives, maybe run apt-get update or try with --fix-missing?
```

**Solution:** The package lists are outdated. Run `apt-get update` first:

```bash
# Update package lists
sudo apt-get update

# Then retry the installation
sudo apt-get install -y build-essential git libgl1-mesa-dev libasound2-dev libgtk-3-dev
```

If the error persists after updating:

1. **Try with --fix-missing flag:**
   ```bash
   sudo apt-get install -y --fix-missing build-essential git libgl1-mesa-dev libasound2-dev libgtk-3-dev
   ```

2. **Check your internet connection** - WSL needs network access to download packages

3. **Try cleaning apt cache:**
   ```bash
   sudo apt-get clean
   sudo apt-get update
   sudo apt-get install -y build-essential git libgl1-mesa-dev libasound2-dev libgtk-3-dev
   ```

4. **If using a proxy or VPN**, ensure WSL can access it. Check `/etc/apt/apt.conf` for proxy settings.

### "Hash Sum mismatch" Error

If you see an error like:

```
E: Failed to fetch ... Hash Sum mismatch
Hashes of expected file:
  - SHA512:...
Hashes of received file:
  - SHA512:...
```

This means the downloaded package file is corrupted. **Solution:**

1. **Clean apt cache and retry** (most common fix):
   ```bash
   sudo apt-get clean
   sudo apt-get update
   sudo apt-get install -y build-essential git libgl1-mesa-dev libasound2-dev libgtk-3-dev
   ```

2. **If that doesn't work, clean all caches more thoroughly:**
   ```bash
   sudo rm -rf /var/lib/apt/lists/*
   sudo apt-get clean
   sudo apt-get update
   sudo apt-get install -y build-essential git libgl1-mesa-dev libasound2-dev libgtk-3-dev
   ```

3. **Try a different Ubuntu mirror** (if the repository is having issues):
   ```bash
   # Backup current sources
   sudo cp /etc/apt/sources.list /etc/apt/sources.list.backup
   
   # Edit sources.list to use a different mirror (e.g., us.archive.ubuntu.com)
   sudo sed -i 's/archive.ubuntu.com/us.archive.ubuntu.com/g' /etc/apt/sources.list
   sudo sed -i 's/security.ubuntu.com/us.archive.ubuntu.com/g' /etc/apt/sources.list
   
   # Update and retry
   sudo apt-get update
   sudo apt-get install -y build-essential git libgl1-mesa-dev libasound2-dev libgtk-3-dev
   ```

4. **Check for network issues** - Hash mismatches can be caused by:
   - Unstable internet connection
   - VPN or proxy interfering with downloads
   - Firewall blocking or modifying packets

5. **If specific packages fail repeatedly**, try installing them individually:
   ```bash
   sudo apt-get install -y build-essential
   sudo apt-get install -y git
   sudo apt-get install -y libgl1-mesa-dev
   sudo apt-get install -y libasound2-dev
   sudo apt-get install -y libgtk-3-dev
   ```

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
