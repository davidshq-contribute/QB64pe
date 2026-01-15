@echo off
REM Run QB64 BASIC unit tests
REM Compiles and runs all test_*.bas files in tests\unit\

setlocal enabledelayedexpansion

set SCRIPT_DIR=%~dp0
set PROJECT_DIR=%SCRIPT_DIR%..
set QB64PE=%PROJECT_DIR%\qb64pe.exe

if not exist "%QB64PE%" (
    echo Error: qb64pe.exe not found at %QB64PE%
    echo Please build QB64-PE first
    exit /b 1
)

echo Running QB64 BASIC Unit Tests
echo ==============================
echo.

set TOTAL_TESTS=0
set FAILED_TESTS=0

REM Find and run compiler unit tests
for %%f in ("%SCRIPT_DIR%unit\compiler\test_*.bas") do (
    set /a TOTAL_TESTS+=1
    set "testfile=%%f"
    set "testname=%%~nf"

    echo Running: !testname!

    set "outfile=%SCRIPT_DIR%unit\!testname!.exe"

    REM Compile the test
    "%QB64PE%" -x "!testfile!" -o "!outfile!" >nul 2>&1
    if !errorlevel! equ 0 (
        REM Run the test
        "!outfile!"
        if !errorlevel! equ 0 (
            echo   PASSED
        ) else (
            echo   FAILED ^(runtime error^)
            set /a FAILED_TESTS+=1
        )
        del "!outfile!" 2>nul
    ) else (
        echo   FAILED ^(compilation error^)
        set /a FAILED_TESTS+=1
    )
    echo.
)

REM Find and run IDE unit tests
for %%f in ("%SCRIPT_DIR%unit\ide\test_*.bas") do (
    set /a TOTAL_TESTS+=1
    set "testfile=%%f"
    set "testname=%%~nf"

    echo Running: !testname!

    set "outfile=%SCRIPT_DIR%unit\!testname!.exe"

    "%QB64PE%" -x "!testfile!" -o "!outfile!" >nul 2>&1
    if !errorlevel! equ 0 (
        "!outfile!"
        if !errorlevel! equ 0 (
            echo   PASSED
        ) else (
            echo   FAILED ^(runtime error^)
            set /a FAILED_TESTS+=1
        )
        del "!outfile!" 2>nul
    ) else (
        echo   FAILED ^(compilation error^)
        set /a FAILED_TESTS+=1
    )
    echo.
)

echo ==============================
echo Total: %TOTAL_TESTS% tests
echo Failed: %FAILED_TESTS% tests

if %FAILED_TESTS% gtr 0 (
    exit /b 1
)
