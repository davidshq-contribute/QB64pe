@rem QB64-PE Windows setup script
@rem
@rem This NT command script calls setup_mingw.cmd which downloads and installs MINGW if required
@rem It then proceeds to build QB64-PE
@rem
@rem If argument 1 is not blank, then qb64pe will not be started after compilation
@rem
@echo off

rem Enable cmd extensions and exit if not present
setlocal enableextensions
if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Command Prompt extensions not available!
    echo ========================================
    echo.
    echo This script requires command extensions to be enabled.
    echo Please run this script in a standard Windows Command Prompt.
    echo.
    goto report_error
)

echo QB64-PE Setup
echo.

rem Change to the correct drive & path
cd /d %~dp0
if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Failed to change to script directory!
    echo ========================================
    echo.
    echo Unable to navigate to the script's directory.
    echo Please ensure you have proper permissions and try again.
    echo.
    goto report_error
)

rem Check if the C++ compiler is there and skip MINGW setup if it exists
rem Also verify make.exe exists if compiler is present
if exist "internal\c\c_compiler\bin\c++.exe" (
    if not exist "internal\c\c_compiler\bin\mingw32-make.exe" (
        echo.
        echo ========================================
        echo ERROR: Make utility not found!
        echo ========================================
        echo.
        echo Expected location: internal\c\c_compiler\bin\mingw32-make.exe
        echo.
        echo The make utility is required to build QB64-PE.
        echo Please ensure MinGW was installed completely.
        echo.
        goto report_error
    )
    goto build_qb64pe
)

rem Check if setup_mingw.cmd exists before trying to use it
if not exist "setup_mingw.cmd" (
    echo.
    echo ========================================
    echo ERROR: Required file not found!
    echo ========================================
    echo.
    echo The setup_mingw.cmd script is missing from this directory.
    echo This file is required to download and install MinGW.
    echo.
    echo Please ensure you have downloaded the complete QB64-PE package.
    echo.
    goto report_error
)

rem Check if PowerShell is available for architecture detection
where powershell >nul 2>nul
if errorlevel 1 (
    echo.
    echo ========================================
    echo WARNING: PowerShell not found!
    echo ========================================
    echo.
    echo Unable to detect system architecture automatically.
    echo Defaulting to 64-bit MinGW installation.
    echo.
    set BITS=64
    goto setup_mingw
)

rem Check the processor type and then set the BITS variable
powershell -c "(Get-WmiObject Win32_OperatingSystem).OsArchitecture" 2>nul | find /i "64-bit" > nul && set BITS=64 || set BITS=32
if errorlevel 1 (
    echo.
    echo WARNING: Could not detect system architecture, defaulting to 64-bit.
    set BITS=64
)

rem If the OS is 32-bit then proceed to download right away
if %BITS% == 32 goto setup_mingw

rem Check if the user wants to use 32-bit MINGW on a 64-bit system. Default to 64-bit after 60 seconds
choice /t 60 /c 12 /d 1 /m "Do you prefer to download MinGW [1] 64-bit (default) or [2] 32-bit"
if %errorlevel% == 2 set BITS=32

:setup_mingw

rem Call the MINGW setup script using the BITS variable
echo Checking for MinGW %BITS%-bit installation...
pushd .
call setup_mingw.cmd %BITS%
popd

rem Note: setup_mingw.cmd uses 'goto end' on errors which doesn't set errorlevel,
rem so we check for the compiler file existence instead (more reliable)

rem Finally check if the C++ compiler is there now
if not exist "internal\c\c_compiler\bin\c++.exe" (
    echo.
    echo ========================================
    echo ERROR: C++ compiler not found after MinGW setup!
    echo ========================================
    echo.
    echo Expected location: internal\c\c_compiler\bin\c++.exe
    echo.
    echo The MinGW setup may have completed, but the C++ compiler
    echo was not found in the expected location.
    echo.
    echo Please try running setup_mingw.cmd manually or check if:
    echo   - The download completed successfully
    echo   - Files were extracted to the correct location
    echo   - Antivirus software didn't quarantine any files
    echo.
    goto report_error
)

rem Check if make.exe is available
if not exist "internal\c\c_compiler\bin\mingw32-make.exe" (
    echo.
    echo ========================================
    echo ERROR: Make utility not found!
    echo ========================================
    echo.
    echo Expected location: internal\c\c_compiler\bin\mingw32-make.exe
    echo.
    echo The make utility is required to build QB64-PE.
    echo Please ensure MinGW was installed completely.
    echo.
    goto report_error
)

:build_qb64pe

rem Run make clean
echo Cleaning previous build...
internal\c\c_compiler\bin\mingw32-make.exe OS=win clean > nul 2> nul
if errorlevel 1 (
    echo WARNING: Clean step had issues, but continuing with build...
)

rem Now build QB64-PE
echo Building QB64-PE...
echo This may take several minutes, please be patient...
echo.
internal\c\c_compiler\bin\mingw32-make.exe OS=win BUILD_QB64=y
if errorlevel 1 (
    echo.
    echo ========================================
    echo ERROR: Build failed!
    echo ========================================
    echo.
    echo The compilation process encountered errors.
    echo Please review the error messages above for details.
    echo.
    goto report_error
)

rem Verify that qb64pe.exe was created
if not exist "qb64pe.exe" (
    echo.
    echo ========================================
    echo ERROR: Build completed but executable not found!
    echo ========================================
    echo.
    echo The build process completed, but qb64pe.exe was not created.
    echo This usually indicates a linker error or missing dependencies.
    echo.
    echo Please check the build output above for warnings or errors.
    echo.
    goto report_error
)

echo.
echo ========================================
echo Build successful!
echo ========================================
echo.

rem Execute QB64-PE only if there are no parameters
if "%~1" == "" (
    echo Launching QB64-PE...
    qb64pe.exe
    if errorlevel 1 (
        echo.
        echo WARNING: QB64-PE launched but exited with an error code.
        echo This may indicate a runtime issue.
    )
) else (
    echo.
    echo QB64-PE has been built successfully.
    echo You can run it by executing: qb64pe.exe
    echo.
    pause
)

rem Jump to the end of the script
goto end

rem This is only executed if something on top fails
:report_error
echo.
echo ========================================
echo BUILD FAILED
echo ========================================
echo.
echo QB64-PE compilation was unsuccessful.
echo.
echo Troubleshooting steps:
echo   1. Review the error messages above
echo   2. Ensure you have sufficient disk space
echo   3. Check that antivirus software isn't interfering
echo   4. Try running the script as Administrator
echo   5. Ensure you have a stable internet connection (for MinGW download)
echo.
echo If the problem persists, please report the issue at:
echo   https://github.com/QB64-Phoenix-Edition/QB64pe/issues
echo.
echo Please include:
echo   - Your Windows version
echo   - The complete error output from this script
echo   - Any error messages shown during compilation
echo.
pause

rem The End!
:end
endlocal
