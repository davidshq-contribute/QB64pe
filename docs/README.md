# QB64-PE Documentation

This folder contains all developer and reference documentation for QB64 Phoenix Edition.

## Master Index

See [INDEX.md](INDEX.md) for the complete categorized documentation index.

## Repository Root Files

| Document | Purpose |
|----------|---------|
| [../README.md](../README.md) | User-facing: installation, usage, links to wiki/forum |
| [../CLAUDE.md](../CLAUDE.md) | AI assistant guidance: build, test, architecture summary |

## Architecture

| Document | Purpose |
|----------|---------|
| [ARCHITECTURE.md](ARCHITECTURE.md) | High-level architecture: pipeline, bootstrap, auto-include, IDE protocol, DEP_* |
| [ARCHITECTURE-BASIC.md](ARCHITECTURE-BASIC.md) | QB64 compiler/IDE source: tokenization, hash, symbol table, type system, buffers |
| [ARCHITECTURE-C.md](ARCHITECTURE-C.md) | C/C++ runtime: libqb layout, qbs/img_struct, parts/*, DEP_* table |
| [GLOBALS.md](GLOBALS.md) | Global variable reference: pass control, labels, ids, buffers, RCStateVar |
| [GLOSSARY.md](GLOSSARY.md) | Naming conventions and abbreviations |

## Build & Test

| Document | Purpose |
|----------|---------|
| [BUILD-SYSTEM.md](BUILD-SYSTEM.md) | Build process, CI flow, repository layout, Makefile parameters, versioning, quick reference |
| [SETUP-LNX.md](SETUP-LNX.md) | Step-by-step: what happens when you run `./setup_lnx.sh` on Linux |
| [LINUX-DISPLAY-TROUBLESHOOTING.md](LINUX-DISPLAY-TROUBLESHOOTING.md) | Linux: icon but no window (Wayland, XWayland, VMs) |
| [testing.md](testing.md) | Test suites: run_tests.sh, compile_tests, format_tests, C++ tests |

## Compiler Internals

| Document | Purpose |
|----------|---------|
| [COMPILER-PIPELINE.md](COMPILER-PIPELINE.md) | Compilation phases and code generation |
| [CODEGEN-PATTERNS.md](CODEGEN-PATTERNS.md) | How QB64 constructs map to C++ |
| [EXPRESSION-EVALUATION.md](EXPRESSION-EVALUATION.md) | Expression evaluator: evaluate$(), try_method |
| [CONST-EVAL.md](CONST-EVAL.md) | Compile-time constant evaluation |
| [PREPROCESSOR.md](PREPROCESSOR.md) | $IF, $ELSE conditional compilation |
| [METACOMMANDS.md](METACOMMANDS.md) | All `$` metacommands |
| [auto-including.md](auto-including.md) | Auto-include positions and states |
| [support-files.md](support-files.md) | internal/support/ file reference |

## Type System & Data

| Document | Purpose |
|----------|---------|
| [TYPE-SYSTEM.md](TYPE-SYSTEM.md) | Type encoding: bit flags, size, suffixes, UDTs |
| [STRING-INTERNALS.md](STRING-INTERNALS.md) | qbs struct, pools, temporary strings |
| [ARRAYS.md](ARRAYS.md) | Array declaration and usage |
| [ARRAY-IMPLEMENTATION.md](ARRAY-IMPLEMENTATION.md) | Internal array representation |
| [DATA-STATEMENTS.md](DATA-STATEMENTS.md) | DATA/READ/RESTORE syntax |
| [DATA-RESTORE.md](DATA-RESTORE.md) | DATA implementation details |

## Error Handling

| Document | Purpose |
|----------|---------|
| [ERROR-HANDLING.md](ERROR-HANDLING.md) | ON ERROR, RESUME, error trapping |
| [ERROR-CODES.md](ERROR-CODES.md) | Error code reference |
| [error-handling.md](error-handling.md) | Compiler error flow internals |
| [COMMON-PITFALLS.md](COMMON-PITFALLS.md) | Known gotchas and issues |

## IDE & Debugging

| Document | Purpose |
|----------|---------|
| [IDE-ARCHITECTURE.md](IDE-ARCHITECTURE.md) | IDE implementation |
| [IDE-PROTOCOL.md](IDE-PROTOCOL.md) | IDE/compiler communication |
| [DEBUGGING.md](DEBUGGING.md) | Debugging overview |
| [DEBUG-VWATCH.md](DEBUG-VWATCH.md) | Variable watch system implementation |
| [ASSERTIONS.md](ASSERTIONS.md) | $ASSERTS system |

## Libraries

| Document | Purpose |
|----------|---------|
| [LIBRARY-SYSTEM.md](LIBRARY-SYSTEM.md) | $USELIBRARY implementation |
| [DECLARE-LIBRARY.md](DECLARE-LIBRARY.md) | External library bindings |

## Other References

| Location | Purpose |
|---------|---------|
| [../licenses/README.md](../licenses/README.md) | Third-party licenses |
| [../tests/qbasic_testcases/README.md](../tests/qbasic_testcases/README.md) | QBasic sample programs |
| [../tests/compile_tests/glut/README.md](../tests/compile_tests/glut/README.md) | GLUT-specific tests |
| [DOCUMENTATION-CONSOLIDATION.md](DOCUMENTATION-CONSOLIDATION.md) | Documentation improvement plan |

## Quick Links by Role

- **New contributor:** Start with [ARCHITECTURE.md](ARCHITECTURE.md), [build-system.md](build-system.md), [testing.md](testing.md)
- **Compiler changes:** [ARCHITECTURE-BASIC.md](ARCHITECTURE-BASIC.md), [GLOBALS.md](GLOBALS.md), [CODEGEN-PATTERNS.md](CODEGEN-PATTERNS.md)
- **Runtime changes:** [ARCHITECTURE-C.md](ARCHITECTURE-C.md), [STRING-INTERNALS.md](STRING-INTERNALS.md)
- **IDE/debug:** [IDE-ARCHITECTURE.md](IDE-ARCHITECTURE.md), [DEBUG-VWATCH.md](DEBUG-VWATCH.md), [LIBRARY-SYSTEM.md](LIBRARY-SYSTEM.md)
