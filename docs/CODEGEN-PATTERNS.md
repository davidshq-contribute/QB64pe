# CODEGEN-PATTERNS.md

How QB64 constructs are translated to C++ code.

## Control Structures

### IF/THEN/ELSE

**QB64:**
```basic
IF x > 5 THEN
    y = 10
ELSEIF x = 5 THEN
    y = 5
ELSE
    y = 0
END IF
```

**Generated C++:**
```cpp
if (((x) > 5)||is_error_pending()){
    y = 10;
}else{
    if ((x) == 5){
        y = 5;
    }else{
        y = 0;
    }
}
```

**Notes:**
- All conditions include `||is_error_pending()` for error handling
- ELSEIF becomes nested `else { if ... }`

### FOR/NEXT

**QB64:**
```basic
FOR i = 1 TO 10 STEP 2
    PRINT i
NEXT i
```

**Generated C++:**
```cpp
fornext_step0 = 2;
if (fornext_step0<0) fornext_step_negative0=1; else fornext_step_negative0=0;
if (is_error_pending()) goto fornext_error0;
fornext_value0 = 1;
fornext_finalvalue0 = 10;
goto fornext_entrylabel0;
while(1){
    fornext_value0=fornext_step0+(i);
    fornext_entrylabel0:
    i=fornext_value0;
    if (fornext_step_negative0){
        if (fornext_value0<fornext_finalvalue0) break;
    }else{
        if (fornext_value0>fornext_finalvalue0) break;
    }
    // loop body
    fornext_error0:;
}
```

**Notes:**
- Uses `while(1)` with explicit break conditions
- Static variables track step, final value, and direction
- Handles both positive and negative step values

### DO/LOOP

**QB64:**
```basic
DO WHILE x > 0
    x = x - 1
LOOP
```

**Generated C++:**
```cpp
while((x>0)||is_error_pending()){
    x = x - 1;
}
```

**Infinite loops (DO...LOOP):**
```cpp
do{
    // body
}while(1);
```

### WHILE/WEND

**QB64:**
```basic
WHILE x > 0
    x = x - 1
WEND
```

**Generated C++:**
```cpp
while((x>0)||is_error_pending()){
    x = x - 1;
}
```

### SELECT CASE

**QB64:**
```basic
SELECT CASE x
    CASE 1
        PRINT "One"
    CASE 2
        PRINT "Two"
    CASE ELSE
        PRINT "Other"
END SELECT
```

**Generated C++:**
```cpp
// Static variables for case comparison
static int32 sc_var0;
sc_var0 = x;
if (sc_var0==1){
    // CASE 1 body
    goto sc_0_end;
}
if (sc_var0==2){
    // CASE 2 body
    goto sc_0_end;
}
// CASE ELSE body
sc_0_end:;
```

**Notes:**
- Uses if/else chains, not C++ switch
- Each CASE jumps to end after execution

## Variable Declarations

### Scalar Variables

**QB64:**
```basic
DIM x AS INTEGER
DIM y AS LONG
DIM z AS DOUBLE
```

**Generated C++:**
```cpp
int16 x;
int32 y;
double z;
```

### Type Mapping

| QB64 Type | C++ Type |
|-----------|----------|
| _BYTE | int8 |
| _UNSIGNED _BYTE | uint8 |
| INTEGER | int16 |
| _UNSIGNED INTEGER | uint16 |
| LONG | int32 |
| _UNSIGNED LONG | uint32 |
| _INTEGER64 | int64 |
| _UNSIGNED _INTEGER64 | uint64 |
| SINGLE | float |
| DOUBLE | double |
| _FLOAT | long double |
| STRING | qbs* |
| _OFFSET | ptrszint |

### Arrays

**QB64:**
```basic
DIM arr(10) AS LONG
```

**Generated C++:**
```cpp
ptrszint *arr;
// Allocation in initialization
arr = (ptrszint*)mem_static_malloc(4*11+4+1)*sizeof(ptrszint));
```

**Dynamic arrays (REDIM):**
```cpp
arr = (ptrszint*)mem_static_malloc(elements * elementsize + descriptor);
```

### Strings

**QB64:**
```basic
DIM s AS STRING
s = "Hello"
```

**Generated C++:**
```cpp
qbs *s;
s = qbs_new(0,0);
qbs_set(s, qbs_new_txt_len("Hello", 5));
```

**String functions:**
- `qbs_new()` - Create string
- `qbs_set()` - Assign string
- `qbs_free()` - Free string
- `qbs_cleanup()` - Cleanup temporaries
- `qbs_add()` - Concatenation
- `qbs_new_txt_len()` - String literal

### Fixed-Length Strings

**QB64:**
```basic
DIM s AS STRING * 10
```

**Generated C++:**
```cpp
static qbs *s = (qbs*)mem_static_malloc(sizeof(qbs) + 10);
s->len = 10;
s->fixed = 1;
```

## SUB/FUNCTION Declarations

### FUNCTION

**QB64:**
```basic
FUNCTION Add(a AS INTEGER, b AS INTEGER) AS INTEGER
    Add = a + b
END FUNCTION
```

**Generated C++:**
```cpp
int64 FUNC_ADD(int64 a, int64 b){
    int64 __FUNC_ADD_RETURN;
    __FUNC_ADD_RETURN = a + b;
    return __FUNC_ADD_RETURN;
}
```

### SUB

**QB64:**
```basic
SUB PrintX(x AS INTEGER)
    PRINT x
END SUB
```

**Generated C++:**
```cpp
void SUB_PRINTX(int64 x){
    // body
}
```

### Parameter Passing

- Default: Pass by value (copy)
- Arrays: Pass by pointer
- BYREF: Pass by reference (pointer)

## Expressions

### Arithmetic

**QB64:**
```basic
y = x + 5 * 2
```

**Generated C++:**
```cpp
y = (x) + ((5) * (2));
```

All operands are parenthesized to preserve precedence.

### String Concatenation

**QB64:**
```basic
s = "Hello " + "World"
```

**Generated C++:**
```cpp
qbs_set(s, qbs_add(
    qbs_new_txt_len("Hello ", 6),
    qbs_new_txt_len("World", 5)
));
```

### Comparisons

**QB64:**
```basic
IF x > 5 THEN...
```

**Generated C++:**
```cpp
if (((x) > 5)||is_error_pending()){
```

## I/O Statements

### PRINT

**QB64:**
```basic
PRINT "Hello"; x; "World"
```

**Generated C++:**
```cpp
tqbs = qbs_new(0,0);
qbs_print(tqbs, 0);  // output to screen
```

**PRINT USING:**
```cpp
tmp_long = print_using(format_str, tmp_long, tqbs, expression);
tmp_long = print_using_single(format_str, expression, tmp_long, tqbs);
tmp_long = print_using_double(format_str, expression, tmp_long, tqbs);
```

### INPUT

**QB64:**
```basic
INPUT x, s$
```

**Generated C++:**
```cpp
sub_read_string(data, &data_offset, data_size, s);
x = func_read_int64(data, &data_offset, data_size);
```

### File I/O

**QB64:**
```basic
PRINT #1, "Hello"
INPUT #1, x
```

**Generated C++:**
```cpp
tab_fileno = tmp_fileno = filenumber;
sub_file_print(tmp_fileno, qbs_string, newline, 0, 0);
```

## Error Handling

### ON ERROR GOTO

**QB64:**
```basic
ON ERROR GOTO ErrorHandler
x = 1/0
ErrorHandler:
    PRINT "Error"
RESUME
```

**Generated C++:**
```cpp
// Set error handler
error_goto_line = 0;
qbs_set(error_handler_history, qbs_new_txt_len(..., 0));

// Later in error checking:
error_goto_line = 1;
if (error_goto_line==1){error_handling=1; goto LABEL_ErrorHandler;}
```

### RESUME

**RESUME (retry):**
```cpp
if (!error_handling){error(20);}else{
    error_retry=1; qbevent=1;
    error_handling=0; error_err=0; return;
}
```

**RESUME NEXT:**
```cpp
if (!error_handling){error(20);}else{
    error_handling=0; error_err=0; return;
}
```

**RESUME label:**
```cpp
if (!error_handling){error(20);}else{
    error_handling=0; error_err=0;
    goto LABEL_labelname;
}
```

## DATA, READ, and RESTORE

### Buffers and files

- **DataTxtBuf** → `maindata.txt` (main program) or `dataN.txt` (SUB/FUNCTION N). Holds C++ declarations and initialization for DATA-backed variables and the READ pointer state.
- **DataBinBuf** → `data.bin`. Holds binary DATA (e.g. for MKI$/MKL$/MKS$/MKD$ style or raw bytes).

Main-level DATA and READ use `maindata.txt`. When compiling a SUB or FUNCTION, the compiler switches to `data` + `subfuncn` + `.txt` (e.g. `data1.txt`) so each procedure has its own DATA region. At the end of the procedure it appends back to `maindata.txt` for the main program.

### DATA layout

DATA statements are compiled into static variables and/or initialization in the appropriate `maindata.txt` or `dataN.txt`. Labels for DATA (used by RESTORE) are tracked in the label system with `Data_Offset` and `Data_Referenced`. String DATA may emit code that copies into a qbs (e.g. `qbs_new`, `memcpy`); numeric DATA initializes C scalars or arrays.

### READ and RESTORE

READ generates code that advances an internal “data pointer” and assigns the next value(s) to the listed variables. RESTORE generates code that resets that pointer to a label (or the start of DATA). RESTORE has no scope: only one instance of a given RESTORE target label may exist. The compiler emits the pointer state and assignments into the same DataTxtBuf so the runtime reads in order.

## CHAIN

The CHAIN statement runs another program and can pass COMMON data and state. Codegen uses two buffer families:

### chain.txt and inpchain.txt

- **ChainTxtBuf** → `chain.txt`. During CHAIN, the compiler redirects main output into `chain.txt` (or per-call files `chain1.txt`, `chain2.txt`, …). The generated code is included from `qbx.cpp` when the runtime executes CHAIN: it writes the current directory and state to a `.tmp` file, then `#include "../temp/chain.txt"` runs the emitted “save state / launch next program” code.
- **InpChainTxtBuf** → `inpchain.txt`. Code run when the *chained-to* program starts: it reads COMMON and other state from the `.tmp` file. `qbx.cpp` calls `chain_input()` early; `chain_input()` checks COMMAND$ for the chain tag and, if present, opens the `.tmp` file, restores state, then `#include "../temp/inpchain.txt"` runs the emitted “read COMMON / restore state” code.

So: **chain.txt** = “what to do when we CHAIN out”; **inpchain.txt** = “what to do when we were CHAINed into”. Multiple CHAIN targets use separate `chainN.txt` and `inpchainN.txt`; the main `chain.txt` / `inpchain.txt` aggregate them via `#include` of those files.

## Code Generation Buffers

Code is written to different buffers using `WriteBufLine`:

```basic
WriteBufLine MainTxtBuf, "C++ statement;"
WriteBufLine GlobTxtBuf, "int some_var;"
```

| Buffer | Purpose |
|--------|---------|
| `MainTxtBuf` | Main executable code |
| `GlobTxtBuf` | Global declarations |
| `DataTxtBuf` | DATA statements |
| `RegTxtBuf` | SUB/FUNCTION registration |
| `FreeTxtBuf` | Cleanup code |
| `ErrTxtBuf` | Error handling |

## Memory Management Patterns

### Temporary Strings

```cpp
// Create temporary
qbs *tqbs = qbs_new(0, 0);

// Use temporary
qbs_set(result, qbs_add(a, b));

// Cleanup happens automatically at statement end
qbs_cleanup(qbs_tmp_list_nexti);
```

### Static Variables

```cpp
static int64 static_var = 0;
```

### SHARED Variables

```cpp
// Global scope
static int64 shared_var;
```

## Common Patterns

### Expression Evaluation

The `evaluate$()` function returns C++ code for any QB64 expression:

```basic
e$ = evaluate$(expression$, typ)
' e$ now contains C++ code
' typ contains the result type
```

### Operator Precedence

`fixoperationorder$()` ensures correct precedence:

```basic
a$ = fixoperationorder$(expression$)
```

### Type Conversion

```basic
e$ = evaluatetotyp$(expression$, targetType)
```

## Error Checking

All generated code includes error checking:

```cpp
// Condition checks
if ((condition)||is_error_pending()){

// After operations
if (is_error_pending()) goto error_handler;

// Array bounds
if (index < lbound || index > ubound) error(9);
```
