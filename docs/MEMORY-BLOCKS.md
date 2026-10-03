# MEMORY-BLOCKS.md

Documentation of the QB64pe _MEM memory block system.

## Overview

The _MEM system provides safe, validated memory access with tracking to prevent use-after-free and bounds violations.

## _MEM Type Structure

### QB64 Declaration

```basic
TYPE _MEM
    OFFSET AS _OFFSET           ' Memory address
    SIZE AS _OFFSET             ' Block size in bytes
    $_LOCK_ID AS _INTEGER64     ' Lock identifier
    $_LOCK_OFFSET AS _OFFSET    ' Pointer to mem_lock
    TYPE AS _OFFSET             ' Type identifier
    ELEMENTSIZE AS _OFFSET      ' Element size
    IMAGE AS LONG               ' Image handle
    SOUND AS LONG               ' Sound handle
END TYPE
```

### C Structure (mem.h)

```c
struct mem_block {
    intptr_t offset;
    intptr_t size;
    int64_t lock_id;
    intptr_t lock_offset;
    intptr_t type;
    intptr_t elementsize;
    int32_t image;
    int32_t sound;
};
```

## Memory Lock Structure

```c
struct mem_lock {
    int64_t id;       // Lock identifier
    int32_t type;     // Lock type (0-5)
    void *offset;     // Malloc'd pointer (type 1)
};
```

### Lock Types

| Type | Constant | Purpose |
|------|----------|---------|
| 0 | MEM_TYPE_NOSECURITY | User-defined from _OFFSET |
| 1 | MEM_TYPE_MALLOC | C malloc'd memory |
| 2 | MEM_TYPE_IMAGE | Image resource |
| 3 | MEM_TYPE_SUBFUNC | Sub/function scope |
| 4 | MEM_TYPE_ARRAY | Array-based memory |
| 5 | MEM_TYPE_SOUND | Sound resource |

## _MEMNEW Allocation

```c
mem_block func__memnew(intptr_t bytes)
```

### Operation

1. Validate bytes ≥ 0
2. Allocate memory via `malloc()`
3. Create unique lock (type 1)
4. Increment global `mem_lock_id`
5. Return mem_block with valid lock

### Errors

| Error | Cause |
|-------|-------|
| 5 | bytes < 0 |
| 518 | malloc failed |

### Lock ID

- Starts at 1073741823
- Increments with each allocation
- Ensures unique identification

## _MEMFREE Deallocation

```c
void sub__memfree(void *mem)
```

### Operation

1. Validate lock_offset and lock_id match
2. Only free type 1 (MALLOC) blocks
3. Call `free_mem_lock()`:
   - Invalidate lock (id = 0)
   - Free malloc pointer
   - Add lock to freed pool

### Invalidation

After freeing:
```c
block->lock_id = INVALID_MEM_LOCK;  // 1073741821
```

### Errors

| Error | Cause |
|-------|-------|
| 309 | Memory not initialized (lock_offset = 0) |
| 307 | Memory already freed (lock ID mismatch) |

## _MEMGET Operation

```c
void *func__memget(mem_block *blk, intptr_t off, intptr_t bytes)
```

### Validation

**Check A:** `lock_offset != 0`
**Check B:**
- `off >= block.offset`
- `(off + bytes) <= (block.offset + block.size)`
- `lock_id` matches stored value

### Return

- **Success:** Pointer to memory address
- **Failure:** Malloc'd buffer filled with zeros

### Errors

| Error | Cause |
|-------|-------|
| 309 | Memory not initialized |
| 308 | Memory has been freed |
| 300 | Memory region out of range |

### Compiler Optimization

**Known sizes (1, 2, 4, 8 bytes):**
```c
value = *(int32_t*)ptr;  // Direct cast
```

**Unknown sizes:**
```c
memmove(dest, src, size);
```

## _MEMPUT Operation

### Variants

**Type 1:** `_MEMPUT mem, offset, variable`
- Infers type from variable

**Type 2:** `_MEMPUT mem, offset, value AS type`
- Explicit type cast

### Validation

Same as _MEMGET (bounds and lock checking).

## _MEMCOPY Block Transfer

```c
void sub__memcopy(void *sblk, intptr_t soff, intptr_t bytes,
                  void *dblk, intptr_t doff)
```

### Validation

**Check A:** Both blocks initialized
**Check B:**
- bytes ≥ 0
- Source bounds valid
- Destination bounds valid
- Both lock IDs match

### Errors

| Error | Cause |
|-------|-------|
| 310 | Source not initialized |
| 311 | Destination not initialized |
| 312 | Both not initialized |
| 305 | Source freed |
| 306 | Destination freed |
| 313 | Both freed |
| 301 | Invalid size |
| 302 | Source out of range |
| 303 | Destination out of range |
| 304 | Both out of range |

### Implementation

Uses `memmove()` for actual copy (handles overlapping regions).

## _MEMFILL Operation

### Generic Form

```c
void sub__memfill(mem_block *dblk, intptr_t doff, intptr_t dbytes,
                  intptr_t soff, intptr_t sbytes)
```

Fills destination by repeating source pattern.

### Typed Variants

| Function | Type |
|----------|------|
| `sub__memfill_1()` | 1-byte |
| `sub__memfill_2()` | 2-byte |
| `sub__memfill_4()` | 4-byte |
| `sub__memfill_8()` | 8-byte |
| `sub__memfill_SINGLE()` | SINGLE |
| `sub__memfill_DOUBLE()` | DOUBLE |
| `sub__memfill_FLOAT()` | _FLOAT |
| `sub__memfill_OFFSET()` | _OFFSET |

### No-Check Variants

`sub__memfill_nochecks_*()` - Fast path when `$CHECKING:OFF`

## _MEMIMAGE Resource Access

```c
mem_block func__memimage(int32 i, int32 passed)
```

### Operation

1. Take image handle (positive=page, negative=image ID)
2. Validate image exists
3. Create/reuse type 2 (IMAGE) lock
4. Return mem_block for pixel data

### Returns

| Field | Value |
|-------|-------|
| offset | Pointer to pixel data |
| size | bytes_per_pixel × width × height |
| type | Composite type flags |
| elementsize | bytes_per_pixel |
| image | Handle for tracking |

### Error

- Error 258: Invalid image handle

## _MEMSOUND Resource Access

Similar to _MEMIMAGE but for sound data with type 5 (SOUND) locks.

## _MEMEXISTS Validation

```c
int32_t func__memexists(void *void_blk)
```

### Checks

1. `lock_offset != 0` (initialized)
2. Stored `lock_id` matches current `lock->id`

### Returns

- `-1` (TRUE): Block valid
- `0` (FALSE): Uninitialized or freed

## Memory Block Tracking

### Lock Pool Management

```c
mem_lock *mem_lock_base;      // Array of 10,000 locks
int64_t mem_lock_id;          // Counter (starts 1073741823)
mem_lock **mem_lock_freed;    // Pool of freed locks
```

### Checking Modes

**Safe Mode ($CHECKING:ON - default):**
- Runtime bounds checking
- Lock ID validation
- Descriptive error codes

**Fast Mode ($CHECKING:OFF):**
- Direct pointer access
- No validation
- Unsafe if block freed

## byte_element Helper

### Structure

```c
struct byte_element_struct {
    uint64 offset;
    int32 length;
};
```

### Functions

```c
void *byte_element(uint64 offset, int32 length)
void *byte_element(uint64 offset, int32 length, byte_element_struct *info)
```

### Purpose

- Wraps raw memory offset and length
- Used for generic memory access
- Handles negative lengths (clamps to 0)

## Error Categories

### Bounds/Range Errors (300-304)

| Error | Meaning |
|-------|---------|
| 300 | Memory region out of range |
| 301 | Invalid size |
| 302 | Source out of range |
| 303 | Destination out of range |
| 304 | Both out of range |

### Freed Memory Errors (305-308)

| Error | Meaning |
|-------|---------|
| 305 | Source freed |
| 306 | Destination freed |
| 307 | Memory already freed |
| 308 | Memory has been freed |
| 313 | Both source and dest freed |

### Initialization Errors (309-312)

| Error | Meaning |
|-------|---------|
| 309 | Memory not initialized |
| 310 | Source not initialized |
| 311 | Destination not initialized |
| 312 | Both not initialized |

## Example Usage

### Basic Memory Block

```basic
DIM m AS _MEM
m = _MEMNEW(100)              ' Allocate 100 bytes

_MEMPUT m, m.OFFSET, 42 AS LONG
x& = _MEMGET(m, m.OFFSET, LONG)

_MEMFREE m
```

### Array Access

```basic
DIM arr(100) AS LONG
DIM m AS _MEM
m = _MEM(arr())

' Direct array manipulation
_MEMPUT m, m.OFFSET + 4, 999 AS LONG  ' arr(1) = 999
```

### Image Access

```basic
SCREEN _NEWIMAGE(640, 480, 32)
DIM m AS _MEM
m = _MEMIMAGE(0)

' Read pixel at (100, 100)
pixel& = _MEMGET(m, m.OFFSET + (100 + 100 * 640) * 4, _UNSIGNED LONG)
```

### Memory Copy

```basic
DIM src AS _MEM, dst AS _MEM
src = _MEMNEW(100)
dst = _MEMNEW(100)

_MEMCOPY src, src.OFFSET, 100, dst, dst.OFFSET

_MEMFREE src
_MEMFREE dst
```

### Validation

```basic
DIM m AS _MEM
m = _MEMNEW(100)
_MEMFREE m

IF _MEMEXISTS(m) THEN
    PRINT "Valid"
ELSE
    PRINT "Invalid or freed"  ' This prints
END IF
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb/include/mem.h` | Type definitions |
| `internal/c/libqb/src/mem.cpp` | Core implementations |
| `internal/c/libqb.cpp` | _MEMIMAGE |
| `internal/c/common.h` | byte_element_struct |
| `source/qb64pe.bas` | Code generation |
