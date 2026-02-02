# STRING-INTERNALS.md

Detailed documentation of the QB64pe string (qbs) system internals.

## Overview

The qbs (QB String) system is a sophisticated memory management architecture that handles all string operations in QB64pe with optimizations for temporary strings, memory pooling, and legacy compatibility.

## The qbs Struct

**File:** `internal/c/libqb/include/qbs.h`

```c
struct qbs {
    uint8_t *chr;              // Pointer to string data
    int32_t len;               // String length (signed for comparisons)

    uint8_t in_cmem;           // 1 if in conventional memory
    uint16_t *cmem_descriptor; // DBLOCK descriptor pointer
    uint16_t cmem_descriptor_offset;

    uint32_t listi;            // Index in qbs_list

    uint8_t tmp;               // 1 if temporary (auto-freed)
    uint32_t tmplisti;         // Index in qbs_tmp_list

    uint8_t fixed;             // Fixed-length string
    uint8_t readonly;          // Read-only (string literal)

    qbs_field *field;          // File field binding
};

struct qbs_field {
    int32_t fileno;            // File number
    int64_t fileid;            // Unique file ID
    int64_t size;              // Field size
    int64_t offset;            // Offset in file
};
```

### Field Purposes

| Field | Purpose |
|-------|---------|
| `chr`, `len` | Core string data and length |
| `in_cmem` | Legacy 16-bit memory flag |
| `cmem_descriptor` | DBLOCK descriptor for cmem strings |
| `listi` | Track position in memory pool |
| `tmp`, `tmplisti` | Temporary string tracking for cleanup |
| `fixed` | Fixed-length string (no resize) |
| `readonly` | String literal (no modify) |
| `field` | File I/O field binding |

## Memory Pool Management

### Descriptor Pool (65536 entries)

```c
static qbs *qbs_malloc = (qbs *)calloc(sizeof(qbs) * 65536, 1);  // ~1MB
static uint32_t qbs_malloc_next = 0;
static intptr_t *qbs_malloc_freed = (intptr_t *)malloc(...);
static uint32_t qbs_malloc_freed_num = 0;
```

**Strategy:**
1. Allocate from current pool sequentially
2. When freed, push to `qbs_malloc_freed` stack
3. Reuse freed descriptors first (LIFO for cache locality)
4. Create new 65536-entry pool when exhausted

### String Data Pool (Extended Memory)

```c
static uint8_t *qbs_data = (uint8_t *)malloc(1048576);  // 1MB initial
static uint32_t qbs_data_size = 1048576;
static uint32_t qbs_sp = 0;  // Stack pointer (allocation offset)
```

### Tracking List

```c
static intptr_t *qbs_list = (intptr_t *)malloc(65536 * sizeof(intptr_t));
static uint32_t qbs_list_nexti = 0;  // Next available index
```

### Temporary String List

```c
intptr_t *qbs_tmp_list = (intptr_t *)calloc(65536 * sizeof(intptr_t), 1);
uint32_t qbs_tmp_list_nexti;  // IMPORTANT: First index MUST be 0
```

## The "First Index MUST be 0" Invariant

The `qbs_tmp_list` is initialized with `calloc(..., 1)`, zeroing all entries. Index 0 is never used as a valid entry.

**Why this matters:**

1. **Cleanup Termination**: `qbs_cleanup()` loops `while (nexti > base)`. Index 0 serves as safe termination.

2. **NULL Prevention**: Prevents NULL pointer dereference in cleanup.

3. **Compact Safety**: When trimming `-1` entries, index 0 is a sentinel that stops the trim.

4. **Scope Cleanup**: Functions save `nexti`, allocate temps, then cleanup back to saved value. Index 0 ensures safe base.

## String Lifecycle

### Allocation: qbs_new_descriptor()

```c
static qbs *qbs_new_descriptor() {
    // Reuse freed descriptor first (cache-friendly)
    if (qbs_malloc_freed_num) {
        return (qbs *)memset((void *)qbs_malloc_freed[--qbs_malloc_freed_num],
                             0, sizeof(qbs));
    }

    // Allocate from current pool
    if (qbs_malloc_next == 65536) {
        qbs_malloc = (qbs *)calloc(sizeof(qbs) * 65536, 1);
        qbs_malloc_next = 0;
    }
    return &qbs_malloc[qbs_malloc_next++];
}
```

### Freeing: qbs_free()

```c
void qbs_free(qbs *str) {
    // 1. Free file field resources
    if (str->field) field_free(str);

    // 2. Remove from temp tracking
    if (str->tmplisti) {
        qbs_tmp_list[str->tmplisti] = -1;
        // Compact trailing -1 entries
        while (qbs_tmp_list[qbs_tmp_list_nexti - 1] == -1)
            qbs_tmp_list_nexti--;
    }

    // 3. Fixed/readonly: just recycle descriptor
    if (str->fixed || str->readonly) {
        qbs_free_descriptor(str);
        return;
    }

    // 4. Handle cmem vs extended memory
    if (str->in_cmem) {
        qbs_remove_cmem(str);
    } else {
        qbs_list[str->listi] = -1;
        // Compact and recalculate qbs_sp
        // ...
    }

    qbs_free_descriptor(str);
}
```

### Temporary Cleanup: qbs_cleanup()

```c
template <typename T>
static T qbs_cleanup(uint32_t base, T passvalue) {
    while (qbs_tmp_list_nexti > base) {
        qbs_tmp_list_nexti--;
        if (qbs_tmp_list[qbs_tmp_list_nexti] != -1)
            qbs_free((qbs *)qbs_tmp_list[qbs_tmp_list_nexti]);
    }
    return passvalue;
}
```

**Usage Pattern:**
```c
uint32_t saved = qbs_tmp_list_nexti;
// ... allocate temporary strings ...
return qbs_cleanup(saved, result);  // Frees all temps, returns result
```

## Core Functions

### qbs_new() - Create Dynamic String

```c
qbs *qbs_new(int32_t size, uint8_t tmp) {
    // Trigger compaction if needed
    if ((qbs_sp + size + 32) > qbs_data_size)
        qbs_concat(size + 32);

    qbs *newstr = qbs_new_descriptor();
    newstr->len = size;
    newstr->chr = qbs_data + qbs_sp;
    qbs_sp += size + 32;  // 32-byte gap for internal use

    // Track in string list
    newstr->listi = qbs_list_nexti;
    qbs_list[qbs_list_nexti++] = (intptr_t)newstr;

    // Mark temporary if needed
    if (tmp) {
        newstr->tmplisti = qbs_tmp_list_nexti;
        qbs_tmp_list[qbs_tmp_list_nexti++] = (intptr_t)newstr;
        newstr->tmp = 1;
    }

    return newstr;
}
```

### qbs_new_txt() - Create Read-Only String

```c
qbs *qbs_new_txt(const char *txt) {
    qbs *newstr = qbs_new_descriptor();
    newstr->len = txt ? strlen(txt) : 0;
    newstr->chr = (uint8_t *)txt;  // Point to literal
    newstr->tmp = 1;
    newstr->readonly = 1;
    // Add to tmp_list...
    return newstr;
}
```

### qbs_set() - String Assignment

The most complex function with multiple optimization strategies:

```c
qbs *qbs_set(qbs *deststr, qbs *srcstr) {
    // CASE 1: Fixed-length destination
    if (deststr->fixed) {
        // Copy with truncation or padding
    }

    // CASE 2: Can acquire srcstr's storage?
    if (srcstr->tmp && !srcstr->fixed && !srcstr->readonly &&
        srcstr->in_cmem == deststr->in_cmem) {
        // Transfer ownership without copy
        deststr->chr = srcstr->chr;
        deststr->len = srcstr->len;
        // Swap list indices, free srcstr descriptor
        return deststr;
    }

    // CASE 3: Source fits in destination's space
    if (srcstr->len <= deststr->len) {
        memcpy(deststr->chr, srcstr->chr, srcstr->len);
        deststr->len = srcstr->len;
        goto cleanup;
    }

    // CASE 4: Need new allocation
    // Check for collision with next string
    // Reallocate if necessary
    // ...

cleanup:
    if (srcstr->tmp) qbs_free(srcstr);
    return deststr;
}
```

**Optimization Strategies:**

1. **Acquisition**: Temp source transfers storage without copy
2. **In-place**: If source fits in dest's space, just copy
3. **Collision Detection**: Check next string before overwriting
4. **List Truncation**: Remove invalid trailing entries
5. **Lazy Compaction**: Only compact when necessary

## String Concatenation

### Memory Compaction: qbs_concat()

```c
static void qbs_concat(uint32_t bytesrequired) {
    // Compact all strings to base (removes holes)
    uint8_t *dest = qbs_data;
    qbs_sp = 0;

    for (i = 0; i < qbs_list_nexti; i++) {
        if (qbs_list[i] != -1) {
            qbs *tqbs = (qbs *)qbs_list[i];
            if ((tqbs->chr - dest) > 32) {
                memmove(dest, tqbs->chr, tqbs->len);
                tqbs->chr = dest;
            }
            dest = tqbs->chr + tqbs->len;
            qbs_sp = dest - qbs_data;
        }
    }

    // Double allocation if still not enough
    if (((qbs_sp * 2) + bytesrequired + 32) >= qbs_data_size) {
        uint8_t *oldbase = qbs_data;
        qbs_data_size = qbs_data_size * 2 + bytesrequired;
        qbs_data = (uint8_t *)realloc(qbs_data, qbs_data_size);

        // Update all pointers
        for (i = 0; i < qbs_list_nexti; i++) {
            if (qbs_list[i] != -1) {
                qbs *tqbs = (qbs *)qbs_list[i];
                tqbs->chr = tqbs->chr - oldbase + qbs_data;
            }
        }
    }
}
```

### String Addition: qbs_add()

```c
qbs *qbs_add(qbs *str1, qbs *str2) {
    if (!str2->len) return str1;
    if (!str1->len) return str2;

    qbs *tqbs = qbs_new(str1->len + str2->len, 1);
    memcpy(tqbs->chr, str1->chr, str1->len);
    memcpy(tqbs->chr + str1->len, str2->chr, str2->len);

    if (str1->tmp) qbs_free(str1);
    if (str2->tmp) qbs_free(str2);

    return tqbs;
}
```

## Conventional Memory Support

For legacy 16-bit compatibility, strings can be allocated in "conventional memory" (DBLOCK).

### DBLOCK Layout

```
256 -----> qbs_cmem_sp (string data grows up)
           ...
65536 ---> cmem_sp (descriptors grow down)
           Descriptor: [length:2][offset:2]
```

### qbs_new_cmem()

```c
qbs *qbs_new_cmem(int32_t size, uint8_t tmp) {
    qbs *newstr = qbs_new_descriptor();
    qbs_create_cmem(size, tmp, newstr);
    return newstr;
}
```

### qbs_create_cmem()

```c
void qbs_create_cmem(int32_t size, uint8_t tmp, qbs *newstr) {
    if ((qbs_cmem_sp + size) > cmem_sp)
        qbs_concat_cmem(size);

    newstr->len = size;
    newstr->chr = (uint8_t *)dblock + qbs_cmem_sp;
    qbs_cmem_sp += size;
    newstr->in_cmem = 1;

    if (!tmp) {
        // Non-temporary: descriptor in DBLOCK
        cmem_sp -= 4;
        newstr->cmem_descriptor = (uint16_t *)(dblock + cmem_sp);
        newstr->cmem_descriptor[0] = newstr->len;
        newstr->cmem_descriptor[1] = (uint16_t)(newstr->chr - dblock);
    }
}
```

## String Operations

### Comparison Functions

```c
int32_t qbs_equal(qbs *str1, qbs *str2) {
    if (str1->len != str2->len) return 0;
    if (memcmp(str1->chr, str2->chr, str1->len) == 0)
        return -1;  // QB: -1 = true
    return 0;
}

int32_t qbs_lessthan(qbs *str1, qbs *str2) {
    auto limit = std::min(str1->len, str2->len);
    auto cmp = memcmp(str1->chr, str2->chr, limit);
    if (cmp < 0) return -1;
    if (cmp > 0) return 0;
    return (str1->len < str2->len) ? -1 : 0;
}
```

### Case Conversion

```c
qbs *qbs_ucase(qbs *str) {
    if (!str->len) return str;

    qbs *tqbs;
    // In-place if temp, non-fixed, non-readonly, non-cmem
    if (str->tmp && !str->fixed && !str->readonly && !str->in_cmem) {
        tqbs = str;
    } else {
        tqbs = qbs_new(str->len, 1);
        memcpy(tqbs->chr, str->chr, str->len);
    }

    for (int32_t i = 0; i < str->len; i++) {
        if (tqbs->chr[i] >= 'a' && tqbs->chr[i] <= 'z')
            tqbs->chr[i] &= 223;  // Clear bit 5
    }

    if (tqbs != str && str->tmp) qbs_free(str);
    return tqbs;
}
```

### Substring Operations

**qbs_left():**
```c
qbs *qbs_left(qbs *str, int32_t l) {
    if (l >= str->len) return str;

    // In-place for temp strings
    if (str->tmp && !str->fixed && !str->readonly && !str->in_cmem) {
        str->len = l;
        return str;
    }

    qbs *tqbs = qbs_new(l, 1);
    memcpy(tqbs->chr, str->chr, l);
    if (str->tmp) qbs_free(str);
    return tqbs;
}
```

**qbs_right():**
```c
qbs *qbs_right(qbs *str, int32_t l) {
    if (l >= str->len) return str;

    // In-place: just adjust pointer
    if (str->tmp && !str->fixed && !str->readonly && !str->in_cmem) {
        str->chr += (str->len - l);
        str->len = l;
        return str;
    }
    // ... copy version
}
```

**func_mid():**
```c
qbs *func_mid(qbs *str, int32_t start, int32_t l, int32_t passed) {
    // Validate parameters...

    // Pass through if entire string
    if ((start == 1) && (l == str->len)) return str;

    // In-place for temp strings
    if (str->tmp && !str->fixed && !str->readonly && !str->in_cmem) {
        str->chr += (start - 1);
        str->len = l;
        return str;
    }
    // ... copy version
}
```

### Trimming

```c
qbs *qbs_ltrim(qbs *str) {
    if (!str->len || *str->chr != 32) return str;

    // In-place: increment pointer
    if (str->tmp && !str->fixed && !str->readonly && !str->in_cmem) {
        while (*str->chr == 32 && str->len > 0) {
            str->chr++;
            str->len--;
        }
        return str;
    }
    // ... copy version
}
```

### Character Operations

```c
int32_t qbs_asc(qbs *str, uint32_t i) {
    i--;  // 1-based to 0-based
    if (i < (uint32_t)str->len) return str->chr[i];
    error(5);  // Illegal function call
    return 0;
}

qbs *func_chr(int32_t value) {
    if (value < 0 || value > 255) {
        error(5);
        return qbs_new(0, 1);
    }
    qbs *tqbs = qbs_new(1, 1);
    tqbs->chr[0] = value;
    return tqbs;
}
```

## Error Codes

| Code | Context | Meaning |
|------|---------|---------|
| 5 | qbs_asc, func_chr | Illegal function call |
| 508 | qbs_malloc_freed | Descriptor pool realloc failed |
| 509 | qbs_cmem_list | CMEM list realloc failed |
| 510 | qbs_list | String list realloc failed |
| 511 | qbs_tmp_list | Temp list realloc failed |
| 512 | qbs_data | Data pool realloc failed |
| 513 | qbs_concat_cmem | Conventional memory exhausted |
| 514 | qbs_create_cmem | CMEM descriptor space exhausted |
| 515 | qbs_new_fixed_cmem | CMEM fixed string failed |

## Architecture Summary

### Three-Tier Memory

1. **Descriptor Pool**: qbs struct objects (65536 per pool)
2. **Extended Memory**: String data (qbs_data, dynamic size)
3. **Conventional Memory**: Legacy DBLOCK for 16-bit compatibility

### Optimization Strategies

1. **Acquisition**: Temp strings transfer ownership without copying
2. **In-place Modification**: Substring ops adjust pointer/length
3. **32-byte Gap**: Padding prevents fragmentation
4. **Lazy Compaction**: Only compact when space exhausted
5. **LIFO Reuse**: Freed descriptors reused for cache locality

### Temporary String Lifecycle

1. Function entry: Save `qbs_tmp_list_nexti`
2. Operations: Create temps with `tmp=1`
3. Function exit: `qbs_cleanup(saved, result)` frees all temps

### Critical Invariants

1. **Index 0 unused**: `qbs_tmp_list[0]` always 0 (sentinel)
2. **Signed length**: `len` is int32_t for comparison safety
3. **32-byte gap**: Allocated with `size + 32` for internal use
4. **Descriptor recycling**: Always reuse freed descriptors first
