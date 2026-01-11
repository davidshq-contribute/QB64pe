# PowerShell script to lint C/C++ code using clang-tidy
# Usage: .\scripts\lint-code.ps1 [--fix] [--files <file1> <file2> ...]

param(
    [switch]$Fix,
    [string[]]$Files
)

$ErrorActionPreference = "Stop"

# Check if clang-tidy is available
$clangTidy = Get-Command clang-tidy -ErrorAction SilentlyContinue
if (-not $clangTidy) {
    Write-Error "clang-tidy not found. Please install LLVM/Clang tools."
    exit 1
}

# Get project root
$projectRoot = Split-Path -Parent $PSScriptRoot

# Define source directories
$sourceDirs = @(
    "$projectRoot\internal\c",
    "$projectRoot\tests\c"
)

# Get files to lint
if ($Files) {
    $filesToLint = $Files | Where-Object { Test-Path $_ }
} else {
    # Find all C/C++ source files (exclude third-party code)
    $filesToLint = Get-ChildItem -Path $sourceDirs -Include *.c,*.cpp -Recurse -File | 
        Where-Object { 
            $_.FullName -notmatch '\\freetype\\' -and 
            $_.FullName -notmatch '\\stb\\' -and
            $_.FullName -notmatch '\\nanosvg\\' -and
            $_.FullName -notmatch '\\qoi\\' -and
            $_.FullName -notmatch '\\jo_gif\\' -and
            $_.FullName -notmatch '\\modp_b64\\' -and
            $_.FullName -notmatch '\\libmidi\\' -and
            $_.FullName -notmatch '\\radv2\\' -and
            $_.FullName -notmatch '\\primesynth\\' -and
            $_.FullName -notmatch '\\ymfmidi\\' -and
            $_.FullName -notmatch '\\hqx\\' -and
            $_.FullName -notmatch '\\mmpx\\' -and
            $_.FullName -notmatch '\\sxbr\\'
        }
}

if (-not $filesToLint) {
    Write-Warning "No C/C++ files found to lint."
    exit 0
}

$totalFiles = $filesToLint.Count
$errors = @()
$warnings = @()

Write-Host "Linting $totalFiles file(s)..." -ForegroundColor Cyan

# Build compile commands if compile_commands.json doesn't exist
$compileCommands = "$projectRoot\compile_commands.json"
if (-not (Test-Path $compileCommands)) {
    Write-Host "Note: compile_commands.json not found. Some checks may be limited." -ForegroundColor Yellow
    Write-Host "      Consider generating it with: bear -- make ..." -ForegroundColor Yellow
}

foreach ($file in $filesToLint) {
    $relativePath = $file.FullName.Replace($projectRoot, "").TrimStart('\', '/')
    Write-Host "`nChecking: $relativePath" -ForegroundColor Cyan
    
    $fixFlag = if ($Fix) { "-fix" } else { "" }
    
    try {
        $output = & clang-tidy $fixFlag $file.FullName 2>&1
        
        # Parse output
        $fileErrors = $output | Where-Object { $_ -match "error:" }
        $fileWarnings = $output | Where-Object { $_ -match "warning:" }
        
        if ($fileErrors) {
            $errors += $fileErrors
            $fileErrors | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
        }
        
        if ($fileWarnings) {
            $warnings += $fileWarnings
            $fileWarnings | ForEach-Object { Write-Host "  $_" -ForegroundColor Yellow }
        }
        
        if (-not $fileErrors -and -not $fileWarnings) {
            Write-Host "  [OK] No issues found" -ForegroundColor Green
        }
    } catch {
        $errors += "Error linting $relativePath : $_"
        Write-Host "  [ERROR] Failed to lint" -ForegroundColor Red
    }
}

Write-Host ("`n" + ("=" * 60)) -ForegroundColor Cyan
Write-Host "Linting Summary:" -ForegroundColor Cyan
Write-Host "  Files checked: $totalFiles" -ForegroundColor White
Write-Host "  Errors: $($errors.Count)" -ForegroundColor $(if ($errors.Count -gt 0) { "Red" } else { "Green" })
Write-Host "  Warnings: $($warnings.Count)" -ForegroundColor $(if ($warnings.Count -gt 0) { "Yellow" } else { "Green" })

if ($errors.Count -gt 0) {
    Write-Host "`nLinting failed with errors." -ForegroundColor Red
    exit 1
} elseif ($warnings.Count -gt 0) {
    Write-Host "`nLinting completed with warnings." -ForegroundColor Yellow
    exit 0
} else {
    Write-Host "`nLinting passed!" -ForegroundColor Green
    exit 0
}
