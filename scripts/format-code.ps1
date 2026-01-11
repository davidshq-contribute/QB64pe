# PowerShell script to format C/C++ code using clang-format
# Usage: .\scripts\format-code.ps1 [--check] [--files <file1> <file2> ...]

param(
    [switch]$Check,
    [string[]]$Files
)

$ErrorActionPreference = "Stop"

# Check if clang-format is available
$clangFormat = Get-Command clang-format -ErrorAction SilentlyContinue
if (-not $clangFormat) {
    Write-Error "clang-format not found. Please install LLVM/Clang tools."
    exit 1
}

# Get project root (parent of scripts directory)
$projectRoot = Split-Path -Parent $PSScriptRoot

# Define source directories
$sourceDirs = @(
    "$projectRoot\internal\c",
    "$projectRoot\tests\c"
)

# Get files to format
if ($Files) {
    $filesToFormat = $Files | Where-Object { Test-Path $_ }
} else {
    # Find all C/C++ source files
    $filesToFormat = Get-ChildItem -Path $sourceDirs -Include *.c,*.cpp,*.h,*.hpp -Recurse -File | 
        Where-Object { $_.FullName -notmatch '\\freetype\\' -and $_.FullName -notmatch '\\stb\\' }
}

if (-not $filesToFormat) {
    Write-Warning "No C/C++ files found to format."
    exit 0
}

$totalFiles = $filesToFormat.Count
$modifiedFiles = @()
$errors = @()

Write-Host "Formatting $totalFiles file(s)..." -ForegroundColor Cyan

foreach ($file in $filesToFormat) {
    $relativePath = $file.FullName.Replace($projectRoot, "").TrimStart('\', '/')
    
    if ($Check) {
        # Check if file is formatted correctly
        try {
            $original = Get-Content $file.FullName -Raw
            $formatted = & clang-format $file.FullName 2>&1
            if ($LASTEXITCODE -ne 0) {
                $errors += "Error checking format for $relativePath : $formatted"
                Write-Host "  [ERROR] $relativePath" -ForegroundColor Red
            } elseif ($original -ne $formatted) {
                $modifiedFiles += $relativePath
                Write-Host "  [NEEDS FORMAT] $relativePath" -ForegroundColor Yellow
            } else {
                Write-Host "  [OK] $relativePath" -ForegroundColor Green
            }
        } catch {
            $errors += "Error checking format for $relativePath : $_"
            Write-Host "  [ERROR] $relativePath" -ForegroundColor Red
        }
    } else {
        # Format the file
        try {
            & clang-format -i $file.FullName
            Write-Host "  [FORMATTED] $relativePath" -ForegroundColor Green
        } catch {
            $errors += "Error formatting $relativePath : $_"
            Write-Host "  [ERROR] $relativePath" -ForegroundColor Red
        }
    }
}

if ($Check) {
    if ($modifiedFiles.Count -gt 0) {
        Write-Host "`n$($modifiedFiles.Count) file(s) need formatting:" -ForegroundColor Yellow
        $modifiedFiles | ForEach-Object { Write-Host "  - $_" }
        Write-Host "`nRun without --check to format these files." -ForegroundColor Yellow
        exit 1
    } else {
        Write-Host "`nAll files are properly formatted!" -ForegroundColor Green
        exit 0
    }
} else {
    if ($errors.Count -gt 0) {
        Write-Host "`nErrors occurred:" -ForegroundColor Red
        $errors | ForEach-Object { Write-Host "  $_" }
        exit 1
    } else {
        Write-Host "`nFormatting complete! Formatted $totalFiles file(s)." -ForegroundColor Green
        exit 0
    }
}
