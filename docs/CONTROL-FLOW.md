# CONTROL-FLOW.md

Documentation of the QB64pe control flow implementation.

## Overview

QB64pe uses a stack-based control tracking system to manage nested control structures during compilation, generating appropriate C code for each construct.

## Control Tracking Infrastructure

### Stack Variables (qb64pe.bas lines 703-725)

```basic
DIM SHARED controllevel AS INTEGER    ' Nesting depth (0=not in block)
DIM SHARED controltype(1000) AS INTEGER
DIM controlid(1000) AS LONG           ' Unique ID for labels
DIM controlvalue(1000) AS LONG        ' Extra state info
DIM controlstate(1000) AS INTEGER     ' Current state
DIM SHARED controlref(1000) AS LONG   ' Line number created
```

### Control Type Values

| Type | Constant | Structure |
|------|----------|-----------|
| 1 | IF | IF/THEN/ELSE |
| 2 | FOR | FOR/NEXT |
| 3 | DO | DO (plain) |
| 4 | DO WHILE/UNTIL | DO with condition |
| 5 | WHILE | WHILE/WEND |
| 6 | $IF | Preprocessor conditional |
| 10-17 | SELECT | SELECT CASE (various types) |
| 18 | CASE | CASE block |
| 19 | CASE ELSE | CASE ELSE block |
| 32 | SUB/FUNCTION | Procedure |

## IF/THEN/ELSE/ELSEIF/END IF

### Syntax

```basic
IF condition THEN
    statements
[ELSEIF condition THEN
    statements]
[ELSE
    statements]
END IF
```

### Single-Line Form

```basic
IF condition THEN statement
IF condition GOTO label
```

### Code Generation

**IF:**
```c
if ((<expression>)||is_error_pending()){
```

**ELSE:**
```c
}else{
```

**ELSEIF:**
```c
}else{
if (<expression>){
```

**END IF:**
```c
}  // Close all nested braces from ELSEIF
```

## SELECT CASE

### Syntax

```basic
SELECT CASE expression
    CASE value1
        statements
    CASE value2 TO value3
        statements
    CASE IS > value4
        statements
    CASE ELSE
        statements
END SELECT
```

### SELECT EVERYCASE Variant

```basic
SELECT EVERYCASE expression
    ' All matching CASE blocks execute
END SELECT
```

### Code Generation

**SELECT CASE:**
```c
int64 sc_<id> = expression;  // Store for comparison
```

**CASE:**
```c
if (sc_<id> == value) goto sc_<id>_<n>;
// or for ranges:
if (sc_<id> >= low && sc_<id> <= high) goto sc_<id>_<n>;
```

**END SELECT:**
```c
sc_<id>_end:;
```

## FOR/NEXT Loops

### Syntax

```basic
FOR variable = start TO end [STEP increment]
    statements
NEXT [variable]
```

### Code Generation

```c
static int64 fornext_value<id>;
static int64 fornext_finalvalue<id>;
static int64 fornext_step<id>;
static uint8 fornext_step_negative<id>;

fornext_value<id> = start;
fornext_finalvalue<id> = end;
fornext_step<id> = step;

goto fornext_entrylabel<id>;
while(1){
    fornext_value<id> = fornext_step<id> + (variable);
    fornext_entrylabel<id>:
    variable = fornext_value<id>;
    if (fornext_step_negative<id>){
        if (fornext_value<id> < fornext_finalvalue<id>) break;
    }else{
        if (fornext_value<id> > fornext_finalvalue<id>) break;
    }
    // loop body
}
fornext_exit_<id>:;
```

## DO/LOOP Variants

### Top-Condition Forms

```basic
DO WHILE condition    ' while((condition)||is_error_pending()){
DO UNTIL condition    ' while((!condition)||is_error_pending()){
DO                    ' do{
```

### Bottom-Condition Forms

```basic
LOOP                  ' }while(1);
LOOP WHILE condition  ' }while((condition)&&(!is_error_pending()));
LOOP UNTIL condition  ' }while((!condition)&&(!is_error_pending()));
```

## WHILE/WEND Loops

### Syntax

```basic
WHILE condition
    statements
WEND
```

### Code Generation

```c
while((expression)||is_error_pending()){
    // body
    ww_continue_<id>:;
}
ww_exit_<id>:;
```

## GOTO and Labels

### Label Types

**Numeric Labels:**
```basic
100 PRINT "Line 100"
    GOTO 100
```

**Alphanumeric Labels:**
```basic
start:
    PRINT "At start"
    GOTO start
```

### Code Generation

```c
LABEL_<name>:;
goto LABEL_<label>;
```

## GOSUB/RETURN

### Syntax

```basic
GOSUB label
' ...
label:
    statements
    RETURN
```

### Code Generation

**GOSUB:**
```c
return_point[next_return_point++] = <gosubid>;
if (next_return_point >= return_points) more_return_points();
goto LABEL_<label>;
RETURN_<gosubid>:;
```

**RETURN:**
```c
switch(return_point[--next_return_point]) {
    case <gosubid>: goto RETURN_<gosubid>; break;
}
```

## EXIT Statements

| Statement | Generated Code |
|-----------|----------------|
| EXIT FOR | `goto fornext_exit_<id>;` |
| EXIT DO | `goto dl_exit_<id>;` |
| EXIT WHILE | `goto ww_exit_<id>;` |
| EXIT SELECT | `goto sc_<id>_end;` |
| EXIT SUB/FUNCTION | `goto exit_subfunc;` |

## CONTINUE Statements

| Statement | Generated Code |
|-----------|----------------|
| CONTINUE FOR | `goto fornext_continue_<id>;` |
| CONTINUE DO | `goto dl_continue_<id>;` |
| CONTINUE WHILE | `goto ww_continue_<id>;` |

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | Control flow parsing |
| Lines 5703-7015 | Loop and branch handling |
| Lines 6100-6275 | IF/THEN/ELSE |
| Lines 6281-6650 | SELECT CASE |

## Example Usage

### Nested Loops

```basic
FOR i = 1 TO 10
    FOR j = 1 TO 10
        IF i * j > 50 THEN EXIT FOR
        PRINT i * j;
    NEXT j
NEXT i
```

### SELECT CASE

```basic
SELECT CASE grade$
    CASE "A"
        PRINT "Excellent"
    CASE "B", "C"
        PRINT "Good"
    CASE "D" TO "F"
        PRINT "Needs improvement"
    CASE ELSE
        PRINT "Invalid grade"
END SELECT
```

### DO/LOOP

```basic
DO
    INPUT "Enter number (0 to quit): ", n
    IF n = 0 THEN EXIT DO
    total = total + n
LOOP
```
