# ARRAY-IMPLEMENTATION.md

Documentation of the QB64pe array implementation internals.

## Overview

QB64pe arrays use a descriptor-based system with support for multi-dimensional indexing, dynamic resizing, and optional bounds checking.

## Array Descriptor Format

Arrays are stored as `ptrszint*` (pointer to intptr_t) arrays:

### Descriptor Layout

| Index | Purpose |
|-------|---------|
| 0 | Memory pointer to array data |
| 1 | Reserved |
| 2 | Flags (see below) |
| 3 | Reserved |
| 4*N | Lower bound of dimension N |
| 4*N+1 | Element count in dimension N |
| 4*N+2 | Stride multiplier for dimension N |
| 4*N+3 | Reserved |
| Last | Pointer to `mem_lock` structure |

### Flag Bits (Index 2)

| Bit | Value | Meaning |
|-----|-------|---------|
| 0 | 1 | Array is initialized/defined |
| 1 | 2 | Array is static (not dynamic) |
| 2 | 4 | Array data in conventional memory |

### Descriptor Size

```
(4 * num_dimensions + 5) * sizeof(ptrszint) bytes
```

## DIM/REDIM Allocation

### DIM Implementation

The `dim2()` function (line 14455) calls `allocarray()` (line 13633):

1. Calculate descriptor size
2. Allocate via `mem_static_malloc()`
3. Initialize dimension metadata
4. Set flag bit 0 (initialized)

### Static Arrays

- Allocated at program startup
- Data in static memory segment
- Dimensions fixed at compile time
- Flag bit 1 set (`array[2] & 2 = TRUE`)
- Cannot be REDIMmed

### Dynamic Arrays

- Allocated on first use or REDIM
- Data on heap via `malloc()`
- Can be resized with REDIM
- Flag bit 1 clear (`array[2] & 2 = FALSE`)

### REDIM Behavior

**Without PRESERVE:**
- Frees old memory completely
- Allocates new memory
- All elements initialized to zero/empty

**With PRESERVE:**
- Uses `realloc()` to resize
- Preserves existing elements up to overlap
- New elements initialized to zero/empty
- Updates lock ID for _MEM validity

## Bounds Checking

### Runtime Functions (libqb.cpp 17235-17250)

```c
ptrszint func_lbound(ptrszint *array, int32 index, int32 num_indexes) {
    if ((index < 1) || (index > num_indexes) || ((array[2] & 1) == 0)) {
        error(9);  // Subscript out of range
        return 0;
    }
    index = num_indexes - index + 1;  // Reverse mapping
    return array[4 * index];          // Lower bound
}

ptrszint func_ubound(ptrszint *array, int32 index, int32 num_indexes) {
    if ((index < 1) || (index > num_indexes) || ((array[2] & 1) == 0)) {
        error(9);
        return 0;
    }
    index = num_indexes - index + 1;
    return array[4 * index] + array[4 * index + 1] - 1;  // Upper bound
}
```

### Compile-Time Checking

Controlled by `CheckingOn` flag:

**Enabled ($CHECKING:ON - default):**
```c
array_check((index) - base, count)  // Validates each dimension
```

**Disabled ($CHECKING:OFF):**
- No bounds checking code generated
- Faster but unsafe

## Multi-Dimensional Indexing

### Index Calculation Formula

```
final_index = (idx1 - base1) +
              (idx2 - base2) * stride1 +
              (idx3 - base3) * stride1 * stride2 + ...
```

### Stride Calculation

```c
array[stride_index] = array[next_stride] * array[count_index]
```

Each dimension's stride = product of all higher dimension counts.

### Generated Code (arrayreference$)

For `arr(i, j, k)`:

```c
offset = (i - array[4]) +
         ((j - array[8]) * array[10]) +
         ((k - array[12]) * array[14]);
element_ptr = array[0] + offset * element_size;
```

### With Bounds Checking

```c
array_check(i - array[4], array[5]);   // Check dimension 1
array_check(j - array[8], array[9]);   // Check dimension 2
array_check(k - array[12], array[13]); // Check dimension 3
// Then calculate offset
```

## Dynamic vs Static Determination

Arrays are static if:
- Global scope with compile-time constant dimensions
- Uses `STATIC` keyword explicitly
- `$STATIC` metacommand in effect

Arrays are dynamic if:
- In SUB/FUNCTION (unless STATIC keyword)
- `REDIM` used
- `$DYNAMIC` metacommand in effect
- Dimensions contain variables

## LBOUND/UBOUND Implementation

### Compiler Code Generation

```basic
LBOUND(arrayName[, dimension%])
UBOUND(arrayName[, dimension%])
```

### Dimension Mapping

QB64 uses 1-based dimension indexing, but internally reverses:
```c
index = num_indexes - index + 1;
```

So dimension 1 in QB64 maps to the highest internal index.

### Validation

- Dimension must be in range [1, num_dimensions]
- Array must be initialized (flag bit 0)
- Error 9 on violation

## Array Element Access

### Numeric Arrays

```c
element_ptr = array[0] + flattened_index * element_size;
value = *(type*)element_ptr;
```

### String Arrays (Variable-Length)

```c
qbs_ptr = ((uint64*)(array[0]))[flattened_index];
```

### String Arrays (Fixed-Length)

```c
string_data = array[0] + flattened_index * fixed_string_size;
```

### UDT Arrays

```c
udt_ptr = array[0] + flattened_index * udt_size;
```

### byte_element Wrapper

For _MEM support:
```c
byte_element((uint64)data_pointer, byte_count, info)
```

Creates descriptor with:
- Memory offset
- Byte size
- Element information

## ERASE Statement

### Static Arrays

```c
if (array[2] & 2) {  // If static
    // For string arrays: clear lengths
    tmp_long = size;
    while(tmp_long--)
        ((qbs*)(array[0])[tmp_long])->len = 0;

    // For numeric arrays: zero memory
    memset((void*)(array[0]), 0, total_bytes);
}
```

### Dynamic Arrays

```c
if (!(array[2] & 2)) {  // If dynamic
    // Free string pointers
    tmp_long = total_elements;
    while(tmp_long--)
        qbs_free((qbs*)((uint64*)(array[0]))[tmp_long]);

    // Free array memory
    if (array[2] & 4)  // If in cmem
        cmem_dynamic_free((uint8*)(array[0]));
    else
        free((void*)(array[0]));
}
```

### Post-ERASE State

```c
array[2] ^= 1;  // Clear initialized flag

// Set dimensions as undefined
for each dimension:
    array[4*N] = 2147483647;     // Invalid base
    array[4*N + 1] = 0;          // No elements
    array[4*N + 2] = 0;          // No multiplier

// Reset data pointer
if (string_array)
    array[0] = (ptrszint)&nothingstring;
else
    array[0] = (ptrszint)nothingvalue;
```

## Lock Handling

### REDIM PRESERVE Lock Update

```c
((mem_lock*)((ptrszint*)array)[4*num_dims + 4])->id = (++mem_lock_id);
```

This invalidates old _MEM blocks pointing to the array.

### _MEM Integration

Arrays can be accessed via _MEM:
```basic
DIM arr(100) AS LONG
DIM m AS _MEM
m = _MEM(arr())
```

The lock ID ensures _MEM blocks become invalid after REDIM.

## Memory Layout Example

For `DIM arr(1 TO 10, 0 TO 5) AS LONG`:

```
Descriptor:
[0]  = pointer to 66 LONGs (10 * 6 * 4 bytes)
[1]  = reserved
[2]  = flags (1 = initialized, +2 if static)
[3]  = reserved
[4]  = 0    (dim 2 lower bound)
[5]  = 6    (dim 2 count)
[6]  = 1    (dim 2 stride = 1)
[7]  = reserved
[8]  = 1    (dim 1 lower bound)
[9]  = 10   (dim 1 count)
[10] = 6    (dim 1 stride = 6)
[11] = reserved
[12] = pointer to mem_lock

Data (60 LONGs):
arr(1,0), arr(1,1), arr(1,2), arr(1,3), arr(1,4), arr(1,5),
arr(2,0), arr(2,1), arr(2,2), arr(2,3), arr(2,4), arr(2,5),
...
arr(10,0), arr(10,1), arr(10,2), arr(10,3), arr(10,4), arr(10,5)
```

## Error Codes

| Error | Cause |
|-------|-------|
| 9 | Subscript out of range |
| 10 | Duplicate definition |
| 38 | Array not defined |

## Key Source Locations

| Location | Content |
|----------|---------|
| qb64pe.bas 13633-14000 | `allocarray()` function |
| qb64pe.bas 14162-14250 | `arrayreference$()` |
| qb64pe.bas 8018-8115 | ERASE implementation |
| libqb.cpp 17235-17250 | LBOUND/UBOUND functions |
| mem.h | mem_lock structure |
