# QB64 Syntax Rules Reference

## Metacommands
- **$INCLUDE**: Must be prefixed with single quote: `'$INCLUDE:'filename'`
  - The single quote is NOT a comment - it's part of the metacommand syntax
  - Used to include other .bas files during compilation

## Variable Declarations
- **DIM**: Creates variables/arrays
- **REDIM**: Resizes existing arrays
- **SHARED**: Makes variables accessible across subs/functions
- **AS**: Specifies data type (INTEGER, LONG, STRING, etc.)

## Function/Subroutine Definitions
- **FUNCTION name$ (params)**: Returns string value
- **FUNCTION name (params)**: Returns numeric value
- **SUB name (params)**: Subroutine (no return value)
- **END FUNCTION/SUB**: Required terminator

## String Operations
- String literals use double quotes: `"text"`
- String variables end with `$`: `name$`
- String concatenation: `+` or `;`
- String functions: `LEFT$`, `RIGHT$`, `MID$`, `LEN$`

## Arrays
- Arrays must be DIMensioned before use
- String arrays: `DIM array$(1 TO 10) AS STRING`
- Numeric arrays: `DIM array(1 TO 10) AS INTEGER`

## Line Continuation
- Use underscore `_` for line continuation
- Useful for long lines

## Comments
- Single quote `'` for comments (except in metacommands)
- REM keyword also works: `REM comment`

## Data Types
- INTEGER: 16-bit signed integer
- LONG: 32-bit signed integer  
- SINGLE: single-precision float
- DOUBLE: double-precision float
- STRING: text data

## Control Structures
- IF...THEN...ELSE...END IF
- FOR...NEXT loops
- DO...LOOP
- WHILE...WEND
- SELECT CASE...END SELECT

## Error Handling
- ON ERROR GOTO label
- ON ERROR RESUME NEXT
- ERR function for error codes

## File I/O
- OPEN filename FOR mode AS #filenum
- INPUT #, PRINT #, GET #, PUT #
- CLOSE #

## Common Pitfalls
- Don't remove the single quote from $INCLUDE metacommands
- Ensure arrays are dimensioned before use
- String parameters need proper handling in function calls
- Check for circular dependencies in includes
