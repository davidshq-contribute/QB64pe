# FILE-IO.md

Documentation of the QB64pe file I/O implementation.

## Overview

QB64pe provides QBasic-compatible file I/O with platform-specific optimizations for Windows and Unix/Linux systems.

## File Handle Management

### FREEFILE Function

```c
int32 func_freefile() {
    return gfs_fileno_freefile();
}
```

Returns lowest available file number (1-based).

### File Number Tracking

| Array | Purpose |
|-------|---------|
| `gfs_fileno[]` | Maps QB file numbers to GFS indices |
| `gfs_file[]` | Array of file structures |
| `gfs_freed[]` | Pool of freed GFS indices |

### Functions

| Function | Purpose |
|----------|---------|
| `gfs_fileno_freefile()` | Find lowest unused file number |
| `gfs_fileno_valid(f)` | Check if file number is in use |
| `gfs_fileno_use(f, i)` | Link QB number to GFS index |
| `gfs_fileno_free()` | Mark file number as unused |

## OPEN Statement

### Syntax

```basic
OPEN filename FOR mode [ACCESS access] [lock] AS #filenumber [LEN=recordlen]
```

### File Modes

| Type | Mode | Description |
|------|------|-------------|
| 1 | RANDOM | Read/write, record-based |
| 2 | BINARY | Read/write, byte-based |
| 3 | INPUT | Read-only, text |
| 4 | OUTPUT | Write-only, creates/truncates |
| 5 | APPEND | Write-only, appends |

### Access Modes

| Value | Mode |
|-------|------|
| 1 | Read/Write (default for RANDOM/BINARY) |
| 2 | Read-only |
| 3 | Write-only |

### Sharing Modes

| Value | Mode |
|-------|------|
| 0 | Shared (default) |
| 1 | Lock read |
| 2 | Lock write |
| 3 | Exclusive |

### Default Record Length

- RANDOM mode: 128 bytes
- Other modes: Not applicable

## gfs_file_struct

```c
struct gfs_file_struct {
    int64_t id;                  // Unique ID for FIELD tracking
    uint8_t open;                // File is open
    uint8_t read;                // Read permission
    uint8_t write;               // Write permission
    uint8_t lock_read;           // Others cannot read
    uint8_t lock_write;          // Others cannot write
    int64_t pos;                 // Current position
    uint8_t eof_reached;         // Read last character
    uint8_t eof_passed;          // Attempted read past EOF
    int32_t fileno;              // QB file number
    uint8_t type;                // Access method (1-5)
    int64_t record_length;       // RANDOM record size
    uint8_t *field_buffer;       // RANDOM record buffer
    qbs **field_strings;         // Field string pointers
    int32_t field_strings_n;     // Field string count
    int64_t column;              // OUTPUT column (1-based)
    std::fstream *file_handle;   // C++ iostream (Unix)
    HANDLE win_handle;           // Windows handle
    uint8_t scrn;                // SCRN: pseudo-device
    // COM port fields...
};
```

## FIELD Statement

### Purpose

Links fixed-length string variables to portions of a random access record buffer.

### Implementation (file-fields.cpp)

| Function | Purpose |
|----------|---------|
| `field_new(fileno)` | Initialize FIELD for file |
| `field_add(str, size)` | Add field to record |
| `field_get(fileno, offset)` | Read record into buffer |
| `field_put(fileno, offset)` | Write buffer to record |
| `field_update(fileno)` | Sync strings from buffer |
| `lrset_field(str)` | Store string to buffer |

### Constraints

- File must be type 1 (RANDOM)
- Total field size ≤ record_length
- Fields have offset and size within record

## GET/PUT Operations

### GET (libqb.cpp 15856-16001)

```c
void sub_get(int32 i, int64 offset, void *element, int32 passed)
```

**RANDOM mode:**
- Offset = record number (1-based)
- Converted to byte offset: `(record - 1) * record_length`

**BINARY mode:**
- Offset = byte position (1-based if passed)

**Error -10:** Reading past EOF

### PUT (libqb.cpp 16184-16298)

```c
void sub_put(int32 i, int64 offset, void *element, int32 passed)
```

- Mirrors GET logic
- **PUT2** variant: Adds length descriptor for variable strings

### Special Handles

Negative file numbers access special handles:
- Stream handles from `special_handles` list
- HTTP connections
- Work with GET/PUT

## INPUT#, LINE INPUT#, PRINT#

### INPUT# (func_input, libqb.cpp 18036-18101)

```c
qbs *func_input(int32 n, int32 i, int32 passed)
```

- Reads n characters
- **INPUT mode**: Stops at CHR$(26) EOF marker
- **BINARY mode**: Returns null string past EOF

### file_input_chr() (libqb.cpp 14228-14263)

- Reads single byte
- Returns -1 on EOF (including CHR$(26))
- Returns -2 on I/O error
- Sets `eof_passed` flag

### LINE INPUT#

- Reads until CR/LF pair
- Handles both Unix and DOS line endings
- Stores in string variable

### PRINT# (libqb.cpp 14324-14450)

- Valid for OUTPUT mode (type 4)
- Writes via `gfs_write()`
- Automatic CRLF on newline
- 14-column TAB intervals

## File Locking

### LOCK (libqb.cpp 23635-23696)

```c
void sub_lock(int32 i, int64 start, int64 end, int32 passed)
```

**Sequential files (type > 2):**
- Locks entire file regardless of range

**RANDOM files:**
- start/end converted from record numbers
- `start = (record - 1) * record_length`
- `end = record * record_length - 1`

**BINARY files:**
- start/end used as byte offsets (1-based)

### UNLOCK (libqb.cpp 23698-23759)

- Symmetric to LOCK
- Same range conversion

### Platform Support

| Platform | Implementation |
|----------|----------------|
| Windows | `LockFile()` / `UnlockFile()` API |
| Unix/Linux | No-op (returns 0) |

## EOF, LOC, LOF Functions

### LOF (Length of File)

```c
// Returns total file size in bytes
int64 func_lof(int32 i)
```

- Supports special handles (Stream, HTTP)
- Error 5 on failure

### EOF (End of File)

```c
// Returns -1 (TRUE) if at/past EOF, 0 otherwise
int32 func_eof(int32 i)
```

**RANDOM/BINARY:** Checks `gfs_eof_passed` flag
**INPUT:** Checks both `eof_reached` and `eof_passed`

### LOC (Location)

```c
int64 func_loc(int32 i)
```

| Mode | Returns |
|------|---------|
| RANDOM | Current record number |
| BINARY | Current byte position |
| INPUT/OUTPUT/APPEND | position / 128 |
| COM ports | Bytes in input buffer (Windows) |

## GFS Layer Functions

| Function | Purpose |
|----------|---------|
| `gfs_new()` | Allocate file structure |
| `gfs_open()` | Open file |
| `gfs_close()` | Close and free |
| `gfs_read()` | Low-level read |
| `gfs_write()` | Low-level write |
| `gfs_lof()` | Get file size |
| `gfs_eof_reached()` | EOF detection |
| `gfs_lock()` / `gfs_unlock()` | Range locking |
| `gfs_setpos()` / `gfs_getpos()` | Position management |

## Platform Differences

### GFS_C (Unix/Linux)

- Uses `std::fstream` / `std::ofstream`
- File locking not supported (no-op)

### GFS_WINDOWS

- Uses `CreateFile()`, `ReadFile()`, `WriteFile()` APIs
- Full file locking support

## Error Code Mapping

| GFS Code | QB Error | Meaning |
|----------|----------|---------|
| -2 | 258 | Invalid handle |
| -3 | 54 | Bad file mode |
| -4 | 5 | Illegal function call |
| -5 | 53 | File not found |
| -6 | 76 | Path not found |
| -7 | 70 | Permission denied |
| -8 | 68 | Device unavailable |
| -9 | 75 | Path/file access error |
| -10 | 62 | Input past end of file |
| -11 | 64 | Bad file name |

## Example Usage

### Sequential File

```basic
OPEN "data.txt" FOR OUTPUT AS #1
PRINT #1, "Hello, World!"
CLOSE #1

OPEN "data.txt" FOR INPUT AS #1
LINE INPUT #1, a$
CLOSE #1
```

### Random Access File

```basic
TYPE RecordType
    name AS STRING * 20
    value AS LONG
END TYPE

DIM rec AS RecordType

OPEN "data.dat" FOR RANDOM AS #1 LEN = LEN(rec)
rec.name = "Test"
rec.value = 42
PUT #1, 1, rec
CLOSE #1
```

### Binary File

```basic
OPEN "data.bin" FOR BINARY AS #1
x& = 12345
PUT #1, , x&
SEEK #1, 1
GET #1, , x&
CLOSE #1
```

## Key Source Files

| File | Content |
|------|---------|
| `internal/c/libqb.cpp` | High-level file functions |
| `internal/c/libqb/src/gfs.cpp` | GFS layer |
| `internal/c/libqb/src/file-fields.cpp` | FIELD operations |
| `internal/c/libqb/include/gfs.h` | Structure definitions |
