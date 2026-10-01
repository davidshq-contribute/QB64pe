# CONST-EVAL.md

Documentation of the QB64pe compile-time constant evaluation system.

## Overview

The constant evaluation system provides compile-time evaluation of expressions for CONST declarations and `$IF` preprocessor conditions. It uses recursive descent parsing with proper operator precedence.

## Core Files

| File | Purpose |
|------|---------|
| `source/utilities/const_eval.bas` | Main evaluation engine |
| `source/utilities/const_eval.bi` | Header definitions |
| `source/utilities/elements.bas` | Element tokenization |

## Operator Precedence

Precedence hierarchy (lowest to highest):

| Level | Operators | Notes |
|-------|-----------|-------|
| 1 | `,` | Comma expression |
| 2 | `IMP` | Bitwise implication |
| 3 | `EQV` | Bitwise equivalence |
| 4 | `XOR` | Bitwise exclusive or |
| 5 | `OR` | Bitwise or |
| 6 | `AND` | Bitwise and |
| 7 | `NOT` | Bitwise not (unary) |
| 8 | `=`, `<>`, `<`, `>`, `<=`, `>=` | Comparisons |
| 9 | `+`, `-` | Addition/subtraction |
| 10 | `MOD` | Modulo |
| 11 | `\` | Integer division |
| 12 | `*`, `/` | Multiplication/division |
| 13 | `-` | Unary negation |
| 14 | `^`, `ROOT` | Exponentiation (right-associative) |

**Important Notes:**
- Exponentiation is right-associative: `2^3^2` = `2^(3^2)` = 512
- Division `/` always produces floats, even with integer operands
- `NOT` has special handling via parenthesis insertion

## Constant Storage

Constants are stored in parallel arrays:

| Array | Purpose |
|-------|---------|
| `constname()` | Original constant name |
| `constcname()` | C-compatible name |
| `constnamesymbol()` | Type suffix symbol |
| `consttype()` | Variable type identifier |
| `constinteger()` | Stored _INTEGER64 value |
| `constuinteger()` | Stored _UNSIGNED _INTEGER64 value |
| `constfloat()` | Stored _FLOAT value |
| `conststring()` | Stored STRING value |
| `constsubfunc()` | Scope (0=global, >0=subfunc) |
| `constdefined()` | Definition status flag |

## CONST Definition Flow

1. **Expression Parsing**: CONST statement parsed, expression extracted
2. **Compile-Time Evaluation**: `Evaluate_Expression$()` called
3. **Value Storage**: Result stored in appropriate const array
4. **Hash Registration**: Added to hash table via `HashAdd`

```basic
temp1$ = _TRIM$(Evaluate_Expression$(e$, tempNum))
' tempNum contains type information
' temp1$ contains evaluated value
```

## Evaluable Expressions

### Numeric Operations

- Arithmetic: `+`, `-`, `*`, `/`, `\`, `MOD`, `^`, `ROOT`
- Bitwise: `AND`, `OR`, `XOR`, `EQV`, `IMP`, `NOT`
- Comparisons: `=`, `<>`, `<`, `>`, `<=`, `>=`

### Mathematical Functions

| Category | Functions |
|----------|-----------|
| Trigonometric | `SIN`, `COS`, `TAN`, `ATN`, `_ACOS`, `_ASIN` |
| Inverse Trig | `_SEC`, `_CSC`, `_COT`, `_ARCSEC`, `_ARCCSC`, `_ARCCOT` |
| Hyperbolic | `_SECH`, `_CSCH`, `_COTH` |
| Utilities | `ABS`, `SGN`, `INT`, `FIX`, `_ROUND`, `_CEIL` |
| Exponential | `EXP`, `LOG`, `SQR` |
| Conversion | `_D2R`, `_D2G`, `_R2D`, `_R2G`, `_G2D`, `_G2R` |
| Constants | `_PI` |

### Color Functions

- `_RGB32()`, `_RGBA32()`, `_RGB()`, `_RGBA()`
- `_RED32()`, `_GREEN32()`, `_BLUE32()`, `_ALPHA32()`
- `_RED()`, `_GREEN()`, `_BLUE()`, `_ALPHA()`

### String Functions

- `CHR$()` - Convert ASCII code to character (0-255)
- `ASC()` - Get ASCII code from character

### Complex Expressions

```basic
CONST x = (1 + 2) * (3 + 4)     ' Nested parentheses
CONST y = x + 3                  ' Constant references
CONST z = 2^3 + 4*5 - 1          ' Multiple operations
CONST s$ = "hello" + "world"     ' String concatenation
```

### Non-Evaluable

- Variable references (runtime values)
- Non-const function calls
- Array operations
- I/O operations

## $IF Preprocessor Conditions

### Structure

```basic
$IF condition THEN
    [code]
$ELSEIF condition2 THEN
    [code]
$ELSE
    [code]
$END IF
```

### Built-in Variables

| Variable | True When |
|----------|-----------|
| `WINDOWS`, `WIN` | Windows OS |
| `LINUX` | Linux OS |
| `MAC`, `MACOSX` | macOS |
| `32BIT` | 32-bit system |
| `64BIT` | 64-bit system |
| `_ARM_` | ARM architecture |
| `VERSION` | QB64 version string |

### Evaluation Function (EvalPreIF)

**Phase 1 - Preprocessor Directives:**
- Evaluates comparison operators
- Handles version comparison: `VERSION = "3.0"`
- Compares against built-in defines

**Phase 2 - Logical Operations:**
- Processes: `AND`, `OR`, `XOR`
- Combines phase 1 comparisons

**Result:**
- Non-zero/non-empty = TRUE
- Zero/"0"/"" = FALSE

### Execution Level Tracking

| Variable | Purpose |
|----------|---------|
| `ExecLevel()` | Stack tracking which blocks execute |
| `ExecCounter` | Nesting depth |
| `DefineElse()` | Flags for $IF/$ELSE/$END IF |
| `InvalidLine()` | Marks lines to skip |

## Compile-Time vs Runtime Differences

### Compile-Time

- Expression must be completely resolvable
- No runtime variables allowed
- Constant folding performed
- Type determined by suffixes
- Deterministic results
- Element tokenization by spaces

### Runtime

- Variables allowed
- Dynamic evaluation
- Automatic type coercion
- Floating-point precision
- Array access allowed

### Division Difference

- **Compile-time**: `/` always produces FLOAT
- **Runtime**: `/` produces FLOAT, `\` produces INTEGER

## Type Suffixes

| Suffix | Type |
|--------|------|
| `%` | INTEGER |
| `~%` | UNSIGNED INTEGER |
| `&` | LONG |
| `~&` | UNSIGNED LONG |
| `&&` | INTEGER64 |
| `~&&` | UNSIGNED INTEGER64 |
| `!` | SINGLE |
| `#` | DOUBLE |
| `##` | _FLOAT |

## Error Handling

### Parsing Errors

| Error | Cause |
|-------|-------|
| `ERROR - BAD () Count` | Mismatched parentheses |
| `ERROR - Invalid characters` | Invalid characters in expression |
| `ERROR - Expected variable/value` | Missing operand |
| `ERROR - Unexpected element` | Unknown element |
| `ERROR - Wrong number of arguments` | Function argument count |

### Type Errors

| Error | Cause |
|-------|-------|
| `ERROR - String can not be in numeric operation` | Type mismatch |
| `ERROR - Expected {type} value` | Wrong type |
| `ERROR - Invalid constant type` | Invalid const type |

### Mathematical Errors

| Error | Cause |
|-------|-------|
| `ERROR - ABS(_ARCSEC) value < 1` | Domain error |
| `ERROR - Invalid argument to CHR$` | Range 0-255 |

### Preprocessor Errors

| Error | Cause |
|-------|-------|
| `$IF without THEN` | Missing THEN |
| `$ELSE without $IF` | Unmatched $ELSE |
| `$IF block already has $ELSE` | Duplicate $ELSE |
| `$END IF without $IF` | Unmatched $END IF |

## Error Detection

Errors always start with `"ERROR - "` prefix:

```basic
IF LEFT$(result$, 8) = "ERROR - " THEN
    errorMessage$ = MID$(result$, 9)
END IF
```

## Debug Support

```basic
CONST CONST_EVAL_DEBUG = 0  ' In const_eval.bi
```

Set to non-zero to enable detailed `_ECHO` debug output tracing each parsing stage.

## Scoped Constants

- **Global constants**: `constsubfunc() = 0`
- **Local constants**: `constsubfunc() = subfuncn`
- Lookup respects scope during evaluation

## Element Tokenization

Elements separated by single space (`sp`):

| Function | Purpose |
|----------|---------|
| `getelement$(string, index)` | Get nth element |
| `getnextelement$()` | Iterate forward |
| `getprevelement$()` | Iterate backward |
| `numelements(string)` | Count elements |
| `pushelement()` | Add element |

**Example:**
```
"5 + 3 * 2" → Elements: "5", "+", "3", "*", "2"
```

## Negative Constants

- Handled via `Unary()` function
- `DWD()` normalizes duplicate operators:
  - `--` becomes `+`
  - `-+` becomes `-`

## String Concatenation

```basic
CONST greeting$ = "Hello, " + "World!"
```

- Uses `+` operator between strings
- Evaluated at compile-time
- Result stored in `conststring()`
