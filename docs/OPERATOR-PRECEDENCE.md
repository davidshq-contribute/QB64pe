# OPERATOR-PRECEDENCE.md

Single reference for QB64pe operator precedence and associativity. The compiler applies this via `fixoperationorder$()` using levels from `isoperator()` in `source/qb64pe.bas`.

## Precedence table

**Higher level = higher precedence (evaluated first).** Level 0 means “not an operator”.

| Level | Operators | Associativity |
|-------|-----------|---------------|
| 1 | `_ORELSE` | Left |
| 2 | `_ANDALSO` | Left |
| 3 | `IMP` | Left |
| 4 | `EQV` | Left |
| 5 | `XOR` | Left |
| 6 | `OR` | Left |
| 7 | `AND` | Left |
| 8 | `_NEGATE` | — |
| 9 | `NOT` | Right (unary) |
| 10 | `=`, `<>`, `>`, `<`, `>=`, `<=` | Left |
| 11 | `+`, `-` (binary) | Left |
| 12 | `MOD` | Left |
| 13 | `\` (integer divide) | Left |
| 14 | `*`, `/` | Left |
| 15 | unary negation (CHR$(241) in expression stream) | Right (unary) |
| 16 | `^` (power) | Right |

Source: `isoperator()` in `source/qb64pe.bas` (lines 20142–20175). Application: `fixoperationorder$()` (lines 19158–20100+).

## Associativity

- **Same precedence, binary:** Left-to-right (e.g. `a - b - c` → `(a - b) - c`, `a * b / c` → `(a * b) / c`). The compiler wraps the *lowest*-precedence operator at bracket level 0 first, then recurses; when multiple operators share the same level, effective grouping is left-to-right.
- **Power `^`:** Right-associative (e.g. `2^3^4` → `2^(3^4)`). Standard for exponentiation.
- **Unary:** `NOT` and unary negation bind to the immediate right operand.

## Edge cases

### AND vs = in expressions

`AND` has higher precedence than `=`, so:

- `a AND b = c` is parsed as `a AND (b = c)` — the comparison is the right operand of AND.
- Use parentheses when you mean `(a AND b) = c`.

### Type checks: AND vs = (compiler internals)

For *type* checks (e.g. “is this type string?”), use bitwise AND, not equality. See [COMMON-PITFALLS.md — Type Checking with AND vs =](COMMON-PITFALLS.md#7-type-checking-with-and-vs-).

### Functions and precedence

Function calls are not automatically parenthesized with following operators; complex expressions with functions can have surprising precedence. See [COMMON-PITFALLS.md — Operator Precedence with Functions](COMMON-PITFALLS.md#1-operator-precedence-with-functions).

## Related docs

| Doc | Content |
|-----|---------|
| [EXPRESSION-EVALUATION.md](EXPRESSION-EVALUATION.md) | How `evaluate$()`, `fixoperationorder$()`, and `isoperator()` fit into expression handling |
| [COMMON-PITFALLS.md](COMMON-PITFALLS.md) | Precedence gotchas (#1 functions, #7 type AND vs =) |
| [CODEGEN-PATTERNS.md](CODEGEN-PATTERNS.md) | Use of `fixoperationorder$()` in code generation |
