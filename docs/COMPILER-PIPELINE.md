# COMPILER-PIPELINE.md

Documentation of the QB64pe compiler pipeline.

## Overview

QB64pe is a self-hosted transpiler that converts QB64 BASIC source code to C++, which is then compiled by a platform-specific C++ compiler.

## Compiler entry and phase flow

Single diagram for onboarding: from entry (IDE or CLI) through compiler phases to temp files and Make.

```
    Source (.bas)          Source (.bas)
           │                     │
    ┌──────┴──────┐        ┌──────┴──────┐
    │     IDE     │        │     CLI     │   (-x, -c, -z, etc.)
    └──────┬──────┘        └──────┬──────┘
           └──────────┬───────────┘
                      ▼
              ┌───────────────┐
              │   Prepass     │   Declarations & structure (qb64pe.bas ~L1615–2873)
              └───────┬───────┘
                      ▼
              ┌───────────────┐
              │  Main pass    │   C++ code generation (qb64pe.bas ~L2967–11604)
              └───────┬───────┘
                      ▼
              ┌───────────────┐
              │ Finalization  │   Verify & prepare (qb64pe.bas ~L11614–12025)
              └───────┬───────┘
                      ▼
              ┌─────────────────────────────┐
              │  internal/temp/*.txt        │   global.txt, main*.txt, maindata.txt, etc.
              └──────────────┬──────────────┘
                      ▼
              ┌───────────────┐
              │     Make      │   qbx.cpp + includes → native executable
              └───────┬───────┘
                      ▼
              Native executable
```

## Compilation Flow (summary)

```
Source File (.bas)
    ↓
IDE/Command-Line Entry
    ↓
Prepass → Main Pass → Finalization
    ↓
C++ Output (internal/temp/*.txt)
    ↓
Make → Native Executable
```

## Entry Points

### IDE Mode
- Real-time error feedback
- Line-by-line compilation
- Visual Watch debugging

### Command-Line Mode
- Headless compilation
- Batch processing
- CI/CD integration

## Output Buffers

| Buffer | File | Purpose |
|--------|------|---------|
| `GlobTxtBuf` | global.txt | Global declarations |
| `MainTxtBuf` | main0.txt | Main program code |
| `DataTxtBuf` | maindata.txt | DATA statements |
| `ErrTxtBuf` | mainerr.txt | Error handlers |
| `FreeTxtBuf` | mainfree.txt | Memory cleanup |
| `RetTxtBuf` | ret0.txt | RETURN handlers |

## Temp Files

```
internal/temp/
├── global.txt       ' Global variables
├── main.txt         ' Main code
├── maindata.txt     ' DATA section
├── mainerr.txt      ' Error handlers
├── regsf.txt        ' Registration
└── compilelog.txt   ' C++ output
```

## Self-Hosting (Bootstrap)

```
Phase 1: Build Bootstrap
    internal/source/*.cpp → qb64pe_bootstrap

Phase 2: Generate C++
    qb64pe_bootstrap source/qb64pe.bas → internal/temp/

Phase 3: Build Final
    internal/temp/*.txt → qb64pe
```

## Command-Line Flags

| Flag | Purpose |
|------|---------|
| `-c` | GUI progress |
| `-x` | Console output |
| `-z` | C++ only |
| `-y` | Format source |
| `-q` | Quiet mode |

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | Main compiler |
| Lines 1615-2873 | Prepass |
| Lines 2967-11604 | Main pass |
| Lines 11614-12025 | Finalization |
