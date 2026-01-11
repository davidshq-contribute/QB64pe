@echo off
REM Windows wrapper to run QB64 tests in WSL (no GUI dialogs)
REM This avoids Windows error dialog boxes that require user interaction

REM Change to QB64 root directory
cd /d "%~dp0..\.."

REM Convert Windows path to WSL path
set WSL_PATH=/mnt/c/code/qb64contain/QB64pe

REM Run the WSL test script
wsl bash -c "cd %WSL_PATH% && bash tests/unit/run_tests_wsl.sh"

REM Exit with the same code as the WSL script
exit /b %ERRORLEVEL%
