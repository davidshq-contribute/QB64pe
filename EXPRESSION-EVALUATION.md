# EXPRESSION-EVALUATION.md

Documentation of the QB64pe expression evaluation system.

## Overview

The expression evaluation system converts QB64 BASIC expressions into C++ code with proper type handling, operator precedence, and scope resolution. The core functions span ~5,000 lines in `source/qb64pe.bas`.

## Core Functions

| Function | Lines | Purpose |
|----------|-------|---------|
| `evaluate$()` | 15783-16616 | Main expression parser |
| `evaluatefunc$()` | 16621-17000+ | Function call evaluation |
| `evaluatetotyp$()` | 18381-18799 | Type conversion |
| `fixoperationorder$()` | 19158-20100+ | Operator precedence |
| `isoperator()` | 20142-20175 | Operator identification |
| `operatorusage()` | 21183-21265 | Operator code generation |

## evaluate$() Function

The main expression evaluator that converts QB64 expressions to C++ code.

### Algorithm Overview

1. **Block Parsing**: Tokenize into variables, operators, literals, functions
2. **Element Classification**: Identify type of each block
3. **Scope Resolution**: Use try_method 1-4 for variable lookup
4. **Type Identification**: Determine types from suffixes and context
5. **Operator Application**: Apply operators with type conversions
6. **Code Generation**: Return C++ expression string

### Block Parsing Phase

Elements are separated by `sp` (CHR$(13)) and processed sequentially:

```basic
n = numelements(a$)
FOR i = 1 TO n
    e$ = getelement$(a$, i)
    ' Process element...
NEXT
```

Bracket depth (`b`) tracks nested parentheses and function calls.

### Element Classification

Each non-operator block is classified as:
- Numeric literal (integer, float, hex, binary)
- String literal
- Variable reference
- Array access
- UDT member access
- Function call

## try_method 1-4 Lookup Strategy

Variable resolution uses a 4-stage scope search:

### try_method = 1: Exact Local Match
- Searches only current function scope
- Most restrictive lookup
- Condition: `subfuncn = id.insubfuncn AND try_method <= 2`

### try_method = 2: Add Type Suffix
- If no type suffix provided, adds implied suffix
- Uses `defineextaz(v)` for default type by first letter
- Retries search with suffix added

```basic
IF try_method = 2 OR try_method = 4 THEN
    dtyp$ = removesymbol(l$)
    IF LEN(dtyp$) = 0 THEN
        IF isvalidvariable(l$) THEN
            IF LEFT$(l$, 1) = "_" THEN v = 27 ELSE v = ASC(UCASE$(l$)) - 64
            l$ = l$ + defineextaz(v)
        END IF
    END IF
END IF
```

### try_method = 3: Broader Scope
- Searches parent functions and modules
- Condition: `try_method >= 3`

### try_method = 4: Global/Shared
- Most permissive search
- Finds shared variables and global declarations

This allows local variables to shadow globals while still finding globals when needed.

## evaluatefunc$() Function

Evaluates function calls and generates C++ code.

### Argument Processing

1. **Count Arguments**: Use `countFunctionElements(a$)` (counts commas at nesting level 1)
2. **Validate Count**: Check against `id2.args` and `id2.minargs`
3. **Format Validation**: Check `id2.specialformat` for optional arguments
4. **Evaluate Each**: Call `evaluatetotyp$()` for type conversion

### Special Format Syntax

```
"?,?,?"         - Three required arguments
"[?],?,[?]"     - Middle required, first and last optional
```

### Special Function Handling

| Function | Special Handling |
|----------|------------------|
| `VAL()` | Returns templated type (int64_t, uint64_t, long double) |
| `_CAST()` | Type casting syntax |
| `_CV()` | Conversion functions (CVI, CVL, CVS, CVD) |
| `_MK()` | Binary value creation |
| `UBOUND/LBOUND` | Array dimension functions |
| `ASC()` | Optional second argument for position |

## evaluatetotyp$() Function

Converts expression result to a specific target type.

### Special Target Types

| targettyp | Meaning |
|-----------|---------|
| -4 | byte_element (offset, size) for _MEM |
| -5 | Size only |
| -6 | Offset only |
| -8 | _MEM structure helper |

### Type Conversion Logic

```basic
IF targettyp = -4 OR targettyp = -5 OR targettyp = -6 THEN
    IF (sourcetyp AND ISREFERENCE) = 0 THEN
        Give_Error "Expected variable name/array element"
    END IF
    ' Handle UDT member access, extract offset/size
END IF
```

## fixoperationorder$() Function

Applies operator precedence by adding `{}` brackets.

### Processing Stages

**A. Bracket Validation**
- Quick check for mismatched parentheses

**B. Operator Simplification**
```
++ → +     (double plus)
-+ → -     (minus-plus)
-- →       (double minus after operator)
```

**C. Negation Localization**
- Changes `operator - variable` to `operator CHR$(241) variable`
- CHR$(241) = special negation marker
- Direct negation of numbers stays as literal

**D. Power-Negation Bracketing**
- Handles `^ CHR$(241)` (power of negation)
- Example: `2^-1` becomes `{2^{-1}}`

**E. Operator Level Detection**
- Finds lowest (lco) and highest (hco) precedence at bracket level 0

**F. Bracketing Application**
- Wraps lowest precedence operators with `{}`
- Recursively processes nested brackets

**G-H. Element Processing**
- Convert numbers to include type suffix
- Evaluate compile-time constants
- Process quoted strings with escapes

## Operator Precedence Levels

The `isoperator()` function returns precedence (0 = not operator):

| Level | Operators |
|-------|-----------|
| 1 | `_ORELSE` |
| 2 | `_ANDALSO` |
| 3 | `IMP` |
| 4 | `EQV` |
| 5 | `XOR` |
| 6 | `OR` |
| 7 | `AND` |
| 8 | `_NEGATE` |
| 9 | `NOT` |
| 10 | `=`, `<>`, `>`, `<`, `>=`, `<=` |
| 11-12 | `+`, `-` |
| 13-14 | `MOD`, `\` |
| 15-16 | `*`, `/` |
| 17 | CHR$(241) (unary negation) |
| 18 | `^` |

Higher level = higher precedence (evaluated first).

## operatorusage() Function

Determines how operators are rendered in C++ code.

### Return Values

| Value | Meaning | Example |
|-------|---------|---------|
| 1 | Binary operator | `lhs + rhs` |
| 2 | Function call | `qbs_add(lhs, rhs)` |
| 3 | Bracket with negation | Change operator |
| 4 | Binary NOT on LHS | `~lhs op rhs` |
| 5 | Unary on RHS | `op(rhs)` |
| 6 | Unary with negation | `-(op(rhs))` |

### Examples

```basic
' String concatenation
IF operator$ = "+" AND (typ AND ISSTRING) THEN
    info$ = "qbs_add": operatorusage = 2    ' → qbs_add(lhs, rhs)

' Numeric addition
ELSEIF operator$ = "+" THEN
    info$ = "+": operatorusage = 1          ' → lhs + rhs

' Power
IF operator$ = "^" THEN
    info$ = "pow2": operatorusage = 2       ' → pow2(lhs, rhs)

' Bitwise NOT
IF operator$ = "NOT" THEN
    info$ = "~": operatorusage = 5          ' → -(~rhs)
```

## Type Resolution

### Type Markup Algorithm

When combining two values with an operator:

```basic
IF (oldtyp AND ISSTRING) = 0 AND (newtyp AND ISSTRING) = 0 THEN
    IF (oldtyp AND ISFLOAT) OR (newtyp AND ISFLOAT) THEN
        ' Both sides float? Use larger size
        b = oldtyp AND 511
        IF (newtyp AND ISFLOAT) THEN
            b2 = newtyp AND 511: IF b2 > b THEN b = b2
        END IF
        typ = ISFLOAT + b
    ELSE
        ' Both integer? Use 64-bit as ideal
        b = oldtyp AND 511: b2 = newtyp AND 511
        IF b2 > b THEN b = b2
        typ = 64&
        IF b = 64 AND (oldtyp AND ISUNSIGNED) AND (newtyp AND ISUNSIGNED) THEN
            typ = 64& + ISUNSIGNED
        END IF
    END IF
END IF
```

### Conversion Rules

| From | To | Method |
|------|-----|--------|
| String | Number | ERROR |
| Number | String | ERROR (except with `+`) |
| Float | Int | `qbr()` rounding |
| Int | Float | `((long double)(...))` |

### Result Override Flags

| Flag | Effect |
|------|--------|
| `result = 1` | Force integer result |
| `result = 2` | Force float result |
| `result = 4` | Force string result |
| `result = 8` | Force boolean (comparisons) |

## Symbol Resolution (findid)

The `findid()` function searches the symbol table.

### Return Values

| Value | Meaning |
|-------|---------|
| 0 | Not found |
| 1 | Found, no more to search |
| 2 | Found, more matches possible (overloads) |

### Scope Checking

```basic
' In scope?
IF ids(i).subfunc = 0 AND ids(i).share = 0 THEN
    IF ids(i).insubfunc <> insf$ THEN GOTO findidnomatch
END IF
' Shared variables and functions are always in scope
' Local variables must match current subfunction context
```

## Temporary Variable Generation

### Auto-Variables

When an undefined variable is used in an expression:
- Type derived from first letter (DEF* commands)
- Created via `dim2(x$, typ$, 1, "")`

### Type Suffixes by Letter

| Letters | Default Type |
|---------|--------------|
| A-I | Integer `%` (or DEFINT-specified) |
| J-Z | Single `!` (or DEFSNG-specified) |
| _ | Same as following letter, default Long `&` |

### Auto-Arrays

When `variable()` syntax detected:
- Dimension syntax: `fakee$ = "10" sp "," sp "10"`
- Creates temp array indexed from 1

## Evaluation Pipeline Summary

| Stage | Function | Input | Output |
|-------|----------|-------|--------|
| 1 | `lineformat$()` | QB64 source | Tokenized elements |
| 2 | `evaluate$()` | Expression | C++ code + type |
| 3 | `fixoperationorder$()` | Expression | Bracketed expression |
| 4 | `isoperator()` | Token | Precedence level |
| 5 | `operatorusage()` | Operator + types | C++ operator/function |
| 6 | `evaluatefunc$()` | Function call | C++ function call |
| 7 | `evaluatetotyp$()` | Expression + target | Converted C++ |
| 8 | `findid()` | Name | Variable/function ID |
| 9 | `refer$()` | ID | C++ access code |

## Common Patterns

### Function Call Generation

```cpp
// QB64: PRINT LEN(a$)
// C++:  qbs_print(func_len(a$));
```

### Array Access

```cpp
// QB64: arr(i, j)
// C++:  *(int32*)(((char*)__ARRAY_INTEGER_ARR[0])+((i-__ARRAY_INTEGER_ARR[4])*__ARRAY_INTEGER_ARR[5]+(j-__ARRAY_INTEGER_ARR[6])*__ARRAY_INTEGER_ARR[7])*2)
```

### Type Conversion

```cpp
// QB64: x% = y#
// C++:  *__INTEGER_X = qbr(*__DOUBLE_Y);
```

### String Concatenation

```cpp
// QB64: a$ = b$ + c$
// C++:  qbs_set(__STRING_A, qbs_add(__STRING_B, __STRING_C));
```
