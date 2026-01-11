@echo off
REM Wrapper script to run QB64 unit tests and display results
REM This script handles the console window issue by:
REM 1. Running test_runner.exe in the current console
REM 2. Waiting for completion
REM 3. Reading and displaying test_results.txt

setlocal enabledelayedexpansion

REM Determine the root directory (two levels up from this script)
set SCRIPT_DIR=%~dp0
cd /d "%SCRIPT_DIR%..\.."

echo === QB64-PE Unit Test Runner ===
echo.

REM Check if test_runner.exe exists
if not exist "test_runner.exe" (
    echo test_runner.exe not found. Compiling tests...

    REM Compile the test runner
    call qb64pe.exe -x tests\unit\test_runner.bas
    if errorlevel 1 (
        echo Error: Failed to compile tests
        exit /b 1
    )
    echo Tests compiled successfully
    echo.
)

REM Remove old test results if they exist
if exist test_results.txt del /f /q test_results.txt

echo Running tests...
echo.

REM Run the test executable in the current console window
REM Note: This will still open in a new window on Windows, but will close automatically
start /wait test_runner.exe

REM Wait a moment for the file to be written
timeout /t 2 /nobreak >nul 2>&1

REM Check if test results file was created
if not exist test_results.txt (
    echo Error: test_results.txt not found
    echo Tests may have crashed or failed to run.
    exit /b 1
)

echo === Test Results ===
echo.

REM Display the test results
type test_results.txt
echo.

REM Parse results to determine exit code
findstr /C:"ALL TESTS PASSED" test_results.txt >nul 2>&1
if %errorlevel% equ 0 (
    echo [92mAll tests passed![0m
    exit /b 0
)

findstr /C:"SOME TESTS FAILED" test_results.txt >nul 2>&1
if %errorlevel% equ 0 (
    echo [91mSome tests failed[0m
    exit /b 1
)

findstr /C:"Failed: 0" test_results.txt >nul 2>&1
if %errorlevel% equ 0 (
    echo [92mAll tests passed![0m
    exit /b 0
)

REM Default: assume failure if we can't determine status
echo Tests completed
exit /b 1
