# QB64-PE Documentation Index

This folder and the repository root contain developer and reference documentation for QB64 Phoenix Edition. Use this index to find the right doc for each topic.

## Root-level docs (repository root)

| Document | Purpose |
|----------|---------|
| [../README.md](../README.md) | User-facing: installation, usage, links to wiki/forum |
| [../CLAUDE.md](../CLAUDE.md) | AI assistant guidance: build, test, architecture summary |
| [../ARCHITECTURE.md](../ARCHITECTURE.md) | High-level architecture: pipeline, bootstrap, auto-include, IDE protocol, DEP_* |
| [../ARCHITECTURE-BASIC.md](../ARCHITECTURE-BASIC.md) | QB64 compiler/IDE source: tokenization, hash, symbol table, type system, buffers, control flow, emit |
| [../ARCHITECTURE-C.md](../ARCHITECTURE-C.md) | C/C++ runtime: libqb layout, qbs/img_struct, parts/*, DEP_* table, tests/c |
| [../GLOBALS.md](../GLOBALS.md) | Global variable reference: pass control, labels, ids, buffers, RCStateVar, command-line |
| [../GLOSSARY.md](../GLOSSARY.md) | Naming conventions and abbreviations (sp/sp2/sp3, idstruct, buffers, etc.) |
| [../TYPE-SYSTEM.md](../TYPE-SYSTEM.md) | Type encoding: bit flags, size, suffixes, UDT arrays |
| [../METACOMMANDS.md](../METACOMMANDS.md) | All `$` metacommands: $IF, $INCLUDE, $COLOR, $DEBUG, $USELIBRARY, etc. |
| [../ERROR-CODES.md](../ERROR-CODES.md) | Runtime and compiler error code reference |
| [../CODEGEN-PATTERNS.md](../CODEGEN-PATTERNS.md) | How QB64 constructs map to C++: IF/FOR/DO/SELECT, ERROR/RESUME, buffers |
| [../EXPRESSION-EVALUATION.md](../EXPRESSION-EVALUATION.md) | Expression evaluator: evaluate$(), try_method 1–4, block parsing |
| [../COMMON-PITFALLS.md](../COMMON-PITFALLS.md) | Gotchas: operator precedence, qbs_tmp_list index 0, auto-include order, etc. |
| [../STRING-INTERNALS.md](../STRING-INTERNALS.md) | qbs struct, pools, temporary strings, cmem |
| [../DEBUG-VWATCH.md](../DEBUG-VWATCH.md) | $DEBUG and vwatch: vwatch.bi/.bm, auto-include, codegen |
| [../LIBRARY-SYSTEM.md](../LIBRARY-SYSTEM.md) | $USELIBRARY and DECLARE LIBRARY: descriptors, injection, external bindings |
| [../IDE-ARCHITECTURE.md](../IDE-ARCHITECTURE.md) | IDE: ide2 loop, screen buffers, menus, wiki, config |

## Docs in this folder (docs/)

| Document | Purpose |
|----------|---------|
| [build-system.md](build-system.md) | Build process, CI flow, repository layout, Makefile parameters, versioning, release |
| [testing.md](testing.md) | Test suites: run_tests.sh, compile_tests, format_tests, add_prefix, C++ tests, dist tests |
| [auto-including.md](auto-including.md) | Auto-include positions (AtTop, AfterMain, AtBottom), states, $USELIBRARY order |
| [support-files.md](support-files.md) | internal/support/ layout and purpose of each file (include, color, vwatch, converter) |
| [error-handling.md](error-handling.md) | Compiler error flow: Give_Error, errmes, IDE command 8, recompile triggers |

## Other references

| Location | Purpose |
|---------|---------|
| [../licenses/README.md](../licenses/README.md) | Third-party licenses and when they apply to compiled programs |
| [../tests/qbasic_testcases/README.md](../tests/qbasic_testcases/README.md) | QBasic sample programs used for compile tests |
| [../tests/compile_tests/glut/README.md](../tests/compile_tests/glut/README.md) | GLUT-specific compile tests |

## Quick links by role

- **New contributor:** Start with [ARCHITECTURE.md](../ARCHITECTURE.md), [build-system.md](build-system.md), [testing.md](testing.md), then [ARCHITECTURE-BASIC.md](../ARCHITECTURE-BASIC.md) or [ARCHITECTURE-C.md](../ARCHITECTURE-C.md) depending on where you work.
- **Compiler (QB64) changes:** [ARCHITECTURE-BASIC.md](../ARCHITECTURE-BASIC.md), [GLOBALS.md](../GLOBALS.md), [CODEGEN-PATTERNS.md](../CODEGEN-PATTERNS.md), [EXPRESSION-EVALUATION.md](../EXPRESSION-EVALUATION.md).
- **Runtime (C++) changes:** [ARCHITECTURE-C.md](../ARCHITECTURE-C.md), [STRING-INTERNALS.md](../STRING-INTERNALS.md).
- **IDE / debug / libraries:** [IDE-ARCHITECTURE.md](../IDE-ARCHITECTURE.md), [DEBUG-VWATCH.md](../DEBUG-VWATCH.md), [LIBRARY-SYSTEM.md](../LIBRARY-SYSTEM.md), [support-files.md](support-files.md).
