# Adding a Built-in: Step-by-Step Guide

Short checklist for adding a new built-in **function** or **sub** to QB64pe. For type encoding, overloading, and feature dependencies, see [BUILTIN-FUNCTIONS.md](BUILTIN-FUNCTIONS.md).

---

## 1. Implement in C/C++

**Where:** `internal/c/libqb/src/<module>.cpp` and `internal/c/libqb/include/<module>.h` (e.g. `filesystem.cpp` / `filesystem.h`, `datetime.cpp` / `datetime.h`).

**Naming:**  
- **FUNCTION** → C name `func__<name>` (e.g. `func__fileexists`).  
- **SUB** → C name `sub__<name>` (e.g. `sub__delay`).

**Signatures:**  
- String argument: `qbs *arg`  
- Numeric (LONG, INTEGER, etc.): `int32_t` / `int64_t` / `double` as appropriate  
- Return: functions return a value; subs return `void`  
- Call `is_error_pending()` at the start if the routine can trigger errors.

**Example – FUNCTION (LONG return, one string arg):**

```c
// In internal/c/libqb/include/filesystem.h
int32_t func__fileexists(qbs *path);

// In internal/c/libqb/src/filesystem.cpp
int32_t func__fileexists(qbs *path) {
    if (is_error_pending()) return 0;
    // ... implementation ...
    return result;  // 0 or 1, etc.
}
```

**Example – SUB (one double arg):**

```c
// In internal/c/libqb/include/datetime.h
void sub__delay(double seconds);

// In internal/c/libqb/src/datetime.cpp
void sub__delay(double seconds) {
    // ... implementation ...
}
```

---

## 2. Register in the compiler

**File:** `source/subs_functions/subs_functions.bas`  
**Where:** Inside `SUB reginternal` (the big block that starts at line 1). Add a new block **before** the `reginternalsubfunc = 0` at the end of the SUB (or in a logical place with similar built-ins).

**Pattern:** For each overload/signature use: `clearid` → set `id.*` fields → `regid`.

**Example – FUNCTION `_FileExists(fileName$)` returning LONG:**

```basic
clearid
id.n = "_FileExists"
id.subfunc = 1
id.callname = "func__fileexists"
id.args = 1
id.arg = MKL$(STRINGTYPE - ISPOINTER)
id.ret = LONGTYPE - ISPOINTER
id.hr_syntax = "_FILEEXISTS(fileName$)"
regid
```

**Example – SUB `_Delay seconds` (one double):**

```basic
clearid
id.n = "_Delay"
id.subfunc = 2
id.callname = "sub__delay"
id.args = 1
id.arg = MKL$(DOUBLETYPE - ISPOINTER)
id.hr_syntax = "_DELAY seconds!"
regid
```

**Fields you’ll usually set:**

| Field | Meaning |
|-------|--------|
| `id.n` | QB64 name (e.g. `"_FileExists"`) |
| `id.subfunc` | `1` = FUNCTION, `2` = SUB |
| `id.callname` | C name (`func__*` or `sub__*`) |
| `id.args` | Number of arguments |
| `id.arg` | Encoded arg types: `MKL$(TYPE1) + MKL$(TYPE2) + ...` |
| `id.ret` | Return type (functions only) |
| `id.hr_syntax` | Human-readable signature for help/errors |
| `id.musthave` | e.g. `"$"` for string suffix (optional) |
| `id.specialformat` | e.g. `"?,?[,?]"` for optional args (optional) |
| `id.Dependency` | e.g. `DEPENDENCY_MINIAUDIO` (optional) |

Type constants: `LONGTYPE`, `INTEGERTYPE`, `DOUBLETYPE`, `SINGLETYPE`, `STRINGTYPE`, `ULONGTYPE`, etc. Use `- ISPOINTER` for non-pointer types. See [BUILTIN-FUNCTIONS.md](BUILTIN-FUNCTIONS.md) for the full list and special codes (-1, -2, …).

---

## 3. Add to the syntax highlighter

**File:** `source/subs_functions/syntax_highlighter_list.bas`  

Add the keyword to the right alphabetical block. Format: `_NAME@` (with `@` as separator). String-returning functions: `_NAME$@`.

Example for `_FileExists` (in the [F] block):

```basic
listOfKeywords$ = listOfKeywords$ + "..._FILEEXISTS@..."
```

If your built-in returns a string (e.g. `_ReadFile$`), include the `$`: `_READFILE$@`.

---

## 4. Rebuild

1. Recompile QB64pe (e.g. run `./qb64pe -x source/qb64pe.bas` or use your usual build).
2. Rebuild libqb if you changed C++ (the normal QB64pe build process compiles `internal/c/libqb` when building a program).

---

## Checklist summary

| Step | File(s) | Action |
|------|--------|--------|
| 1 | `internal/c/libqb/include/*.h`, `internal/c/libqb/src/*.cpp` | Declare and define `func__*` or `sub__*` with correct C++ signature |
| 2 | `source/subs_functions/subs_functions.bas` | In `reginternal`, add `clearid` … `regid` block with `id.n`, `id.callname`, `id.args`, `id.arg`, `id.ret` (if function), `id.hr_syntax` |
| 3 | `source/subs_functions/syntax_highlighter_list.bas` | Add keyword `_NAME@` or `_NAME$@` in the correct letter block |
| 4 | — | Rebuild QB64pe and (if needed) libqb |

---

## See also

- [BUILTIN-FUNCTIONS.md](BUILTIN-FUNCTIONS.md) — Full registration pattern, type encoding, overloading, optional args, feature dependencies, and C implementation locations.
