# ARRAYS.md

Documentation of the QB64pe array implementation.

## Overview

QB64pe arrays use a descriptor-based system with flexible dimensioning, bounds tracking, and efficient memory management supporting both static and dynamic allocation.

## Array Descriptor Structure

```c
// Descriptor layout (as ptrszint* array)
Index 0     = Memory pointer to data
Index 1     = Reserved
Index 2     = Flags (bit 0: defined, bit 1: static, bit 2: cmem)
Index 3     = Reserved
Index 4*N   = Lower bound of dimension N
Index 4*N+1 = Element count in dimension N
Index 4*N+2 = Stride multiplier for dimension N
Index 4*N+3 = Reserved
Last        = Pointer to mem_lock structure
```

### Descriptor Size

```c
size = (4 * num_dimensions + 5) * sizeof(ptrszint)
```

### Flag Bits

| Bit | Value | Meaning |
|-----|-------|---------|
| 0 | 1 | Array initialized/defined |
| 1 | 2 | Array is static |
| 2 | 4 | Array in conventional memory (cmem) |

## DIM Statement

```basic
DIM arr(10) AS INTEGER           ' 0 TO 10 (11 elements)
DIM arr(1 TO 100) AS LONG        ' Explicit bounds
DIM arr(10, 20) AS SINGLE        ' Multi-dimensional
DIM SHARED globalArr(50) AS STRING  ' Global scope
```

### Generated Code

```c
if (__ARRAY_LONG_ARR[2]&2) {
    error(10);  // Cannot redefine static
} else {
    // Update memory lock
    ((mem_lock*)((ptrszint*)__ARRAY_LONG_ARR)[8])->id = (++mem_lock_id);

    // Free previous if defined
    if (__ARRAY_LONG_ARR[2]&1) {
        if (__ARRAY_LONG_ARR[2]&4)
            cmem_dynamic_free((uint8*)(__ARRAY_LONG_ARR[0]));
        else
            free((void*)(__ARRAY_LONG_ARR[0]));
    }

    // Set dimensions
    __ARRAY_LONG_ARR[4] = 0;              // Lower bound
    __ARRAY_LONG_ARR[5] = 11;             // Count (0-10)
    __ARRAY_LONG_ARR[6] = 1;              // Stride

    // Allocate
    __ARRAY_LONG_ARR[0] = (ptrszint)calloc(11 * 4, 1);
    if (!__ARRAY_LONG_ARR[0]) error(257);

    // Mark defined
    __ARRAY_LONG_ARR[2] |= 1;
}
```

## REDIM Statement

```basic
REDIM arr(newSize) AS INTEGER
REDIM _PRESERVE arr(newSize) AS INTEGER
```

### Without _PRESERVE

- Frees old memory
- Allocates new memory
- All elements initialized to zero

### With _PRESERVE

```c
// Update lock ID for _MEM validity
((mem_lock*)((ptrszint*)array)[8])->id = (++mem_lock_id);

if (array[2]&1) {  // If defined
    preserved_elements = MIN(old_count, new_count);
    memcpy(redim_preserve_buffer, (void*)(array[0]),
           preserved_elements * element_size);
    free((void*)(array[0]));
}

array[0] = (ptrszint)calloc(new_count * element_size, 1);

if (old_was_defined)
    memcpy((void*)(array[0]), redim_preserve_buffer,
           preserved_elements * element_size);
```

## LBOUND and UBOUND

```basic
lower% = LBOUND(arr)        ' First dimension lower bound
lower% = LBOUND(arr, 2)     ' Second dimension lower bound
upper% = UBOUND(arr)        ' First dimension upper bound
upper% = UBOUND(arr, 2)     ' Second dimension upper bound
```

### Implementation

```c
ptrszint func_lbound(ptrszint *array, int32 index, int32 num_indexes) {
    if ((index < 1) || (index > num_indexes) || ((array[2] & 1) == 0)) {
        error(9);
        return 0;
    }
    index = num_indexes - index + 1;  // Reverse mapping
    return array[4 * index];
}

ptrszint func_ubound(ptrszint *array, int32 index, int32 num_indexes) {
    index = num_indexes - index + 1;
    return array[4 * index] + array[4 * index + 1] - 1;
}
```

## Multi-Dimensional Arrays

### Index Calculation

```c
offset = (idx1 - base1) +
         (idx2 - base2) * stride1 +
         (idx3 - base3) * stride1 * stride2 + ...

element_ptr = array[0] + offset * element_size
```

### Example: arr(1 TO 10, 0 TO 5)

```
Dimension 2: count=6, stride=1
Dimension 1: count=10, stride=6

arr(i, j) offset = (j - 0) + (i - 1) * 6
```

### Bounds Checking

With `$CHECKING:ON`:
```c
inline ptrszint array_check(uptrszint index, uptrszint limit) {
    if (index < limit)
        return index;
    error(9);  // Subscript out of range
    return 0;
}
```

## Static vs Dynamic Arrays

### Static Arrays

```basic
DIM arr(100) AS INTEGER         ' Global with constants = static
STATIC localArr(50) AS LONG     ' Explicit static in SUB
```

- Flag bit 1 set
- Cannot be REDIMmed
- Allocated at compile-time

### Dynamic Arrays

```basic
REDIM arr(n) AS INTEGER         ' Variable dimension = dynamic
DIM arr(getSize()) AS LONG      ' Function call = dynamic
```

- Flag bit 1 clear
- Can be REDIMmed
- Allocated on heap

### $STATIC and $DYNAMIC

```basic
'$STATIC
DIM arr(100) AS INTEGER    ' Forces static

'$DYNAMIC
DIM arr(100) AS INTEGER    ' Forces dynamic even with constants
```

## SHARED Arrays

```basic
DIM SHARED globalArr(100) AS INTEGER
```

- Accessible in all SUBs/FUNCTIONs
- Must be at module level (outside SUB/FUNCTION)
- Automatically static

## ERASE Statement

```basic
ERASE arr
```

### For Static Arrays

```c
memset((void*)(array[0]), 0, total_bytes);
// For string arrays, also clear string lengths
```

### For Dynamic Arrays

```c
// Free strings if string array
if (is_string_array) {
    for (i = 0; i < total_elements; i++)
        qbs_free((qbs*)((uint64*)(array[0]))[i]);
}

// Free memory
if (array[2] & 4)
    cmem_dynamic_free((uint8*)(array[0]));
else
    free((void*)(array[0]));

// Clear defined flag
array[2] ^= 1;

// Set invalid bounds
for (i = 0; i < num_dims; i++) {
    array[4*i]   = 2147483647;
    array[4*i+1] = 0;
    array[4*i+2] = 0;
}
```

## Passing Arrays to Functions

```basic
SUB ProcessArray(arr() AS INTEGER)
    FOR i = LBOUND(arr) TO UBOUND(arr)
        arr(i) = arr(i) * 2
    NEXT
END SUB

DIM numbers(1 TO 100) AS INTEGER
ProcessArray numbers()
```

- Arrays passed by reference (descriptor pointer)
- Modifications affect original
- Element count must match declaration

## Memory Lock Integration

```c
struct mem_lock {
    int64_t id;       // Lock identifier
    int32_t type;     // 4 = MEM_TYPE_ARRAY
    void *offset;
};
```

Lock ID updated on REDIM _PRESERVE to invalidate old _MEM blocks.

## Error Codes

| Code | Error |
|------|-------|
| 9 | Subscript out of range |
| 10 | Duplicate definition |
| 38 | Array not defined |
| 257 | Out of memory |

## Key Source Files

| File | Content |
|------|---------|
| `source/qb64pe.bas` | allocarray() (13633-14000), ERASE (8018-8150) |
| `internal/c/libqb.cpp` | LBOUND/UBOUND functions |
| `internal/c/qbx.cpp` | array_check inline |
| `internal/c/libqb/include/mem.h` | mem_lock structure |

## Example Usage

### Basic Array

```basic
DIM scores(1 TO 100) AS INTEGER

FOR i = 1 TO 100
    scores(i) = INT(RND * 100)
NEXT

PRINT "First:"; scores(1); "Last:"; scores(100)
```

### Dynamic Resizing

```basic
REDIM names$(0)
count = 0

DO
    INPUT "Enter name (blank to quit): ", n$
    IF n$ = "" THEN EXIT DO
    count = count + 1
    REDIM _PRESERVE names$(count)
    names$(count) = n$
LOOP

PRINT "Names entered:"; count
```

### Multi-Dimensional

```basic
DIM grid(1 TO 10, 1 TO 10) AS INTEGER

FOR y = 1 TO 10
    FOR x = 1 TO 10
        grid(x, y) = x * y
    NEXT
NEXT

PRINT "grid(5,7) ="; grid(5, 7)
```

### Array in SUB

```basic
DIM SHARED gameBoard(8, 8) AS INTEGER

SUB InitBoard
    FOR y = 1 TO 8
        FOR x = 1 TO 8
            gameBoard(x, y) = 0
        NEXT
    NEXT
END SUB

SUB PrintBoard
    FOR y = 1 TO 8
        FOR x = 1 TO 8
            PRINT gameBoard(x, y);
        NEXT
        PRINT
    NEXT
END SUB
```
