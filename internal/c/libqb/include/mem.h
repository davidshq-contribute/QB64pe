//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Memory Management Module
//  Memory block allocation, management and manipulation utilities
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_MEM_H
#define INCLUDE_LIBQB_MEM_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Memory block structure
struct mem_block {
    intptr_t offset;
    intptr_t size;
    int64_t lock_id;      // 64-bit key, must be present at lock's offset or memory region is invalid
    intptr_t lock_offset; // pointer to lock
    intptr_t type;        // https://qb64phoenix.com/qb64wiki/index.php/MEM
    intptr_t elementsize;
    int32_t image;
    int32_t sound;
};

// Memory lock structure
struct mem_lock {
    int64_t id;
    int32_t type; // required to know what action to take (if any) when a request is made to free the block
    // 0=no security (eg. user defined block from _OFFSET)
    // 1=C-malloc'ed block
    // 2=image
    // 3=sub/function scope block
    // 4=array
    // 5=sound
    //---- type specific variables follow ----
    void *offset; // used by malloc'ed blocks to free them
};

// Memory type constants
#define INVALID_MEM_LOCK 1073741821

#define MEM_TYPE_NOSECURITY 0
#define MEM_TYPE_MALLOC 1
#define MEM_TYPE_IMAGE 2
#define MEM_TYPE_SUBFUNC 3
#define MEM_TYPE_ARRAY 4
#define MEM_TYPE_SOUND 5

// Global memory management variables
extern uint64_t mem_lock_id;
extern mem_lock *mem_lock_tmp;
extern mem_lock *mem_lock_base;

// ============================================================================
// MEMORY EXISTENCE AND VALIDATION
// ============================================================================

/// Checks if a memory block is still valid and accessible
/// Validates that the memory block has not been freed or corrupted
/// @param blk Pointer to memory block to check
/// @return Non-zero if memory block exists and is valid, 0 otherwise
int32_t func__memexists(void *blk);

// ============================================================================
// MEMORY FILL OPERATIONS
// ============================================================================

/// Fills memory block with data from another memory block
/// Copies data from source memory region to destination within memory block
/// @param dblk Destination memory block
/// @param doff Destination offset within destination block
/// @param dbytes Number of bytes to fill in destination
/// @param soff Source offset within destination block (for self-copy)
/// @param sbytes Number of bytes to copy from source
void sub__memfill(mem_block *dblk, intptr_t doff, intptr_t dbytes, intptr_t soff, intptr_t sbytes);

/// Fills memory block without bounds checking (internal use)
/// High-performance version without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param soff Source offset within memory block
/// @param sbytes Number of bytes to copy from source
void sub__memfill_nochecks(intptr_t doff, intptr_t dbytes, intptr_t soff, intptr_t sbytes);

/// Fills memory block with 8-bit integer value
/// Repeats 8-bit pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val 8-bit value to fill with
void sub__memfill_1(mem_block *dblk, intptr_t doff, intptr_t dbytes, int8_t val);

/// Fills memory block with 8-bit value without bounds checking
/// High-performance 8-bit fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val 8-bit value to fill with
void sub__memfill_nochecks_1(intptr_t doff, intptr_t dbytes, int8_t val);

/// Fills memory block with 16-bit integer value
/// Repeats 16-bit pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val 16-bit value to fill with
void sub__memfill_2(mem_block *dblk, intptr_t doff, intptr_t dbytes, int16_t val);

/// Fills memory block with 16-bit value without bounds checking
/// High-performance 16-bit fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val 16-bit value to fill with
void sub__memfill_nochecks_2(intptr_t doff, intptr_t dbytes, int16_t val);

/// Fills memory block with 32-bit integer value
/// Repeats 32-bit pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val 32-bit value to fill with
void sub__memfill_4(mem_block *dblk, intptr_t doff, intptr_t dbytes, int32_t val);

/// Fills memory block with 32-bit value without bounds checking
/// High-performance 32-bit fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val 32-bit value to fill with
void sub__memfill_nochecks_4(intptr_t doff, intptr_t dbytes, int32_t val);

/// Fills memory block with 64-bit integer value
/// Repeats 64-bit pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val 64-bit value to fill with
void sub__memfill_8(mem_block *dblk, intptr_t doff, intptr_t dbytes, int64_t val);

/// Fills memory block with 64-bit value without bounds checking
/// High-performance 64-bit fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val 64-bit value to fill with
void sub__memfill_nochecks_8(intptr_t doff, intptr_t dbytes, int64_t val);

/// Fills memory block with single-precision floating point value
/// Repeats 32-bit float pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val Single-precision float value to fill with
void sub__memfill_SINGLE(mem_block *dblk, intptr_t doff, intptr_t dbytes, float val);

/// Fills memory block with single-precision float without bounds checking
/// High-performance single-precision float fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val Single-precision float value to fill with
void sub__memfill_nochecks_SINGLE(intptr_t doff, intptr_t dbytes, float val);

/// Fills memory block with double-precision floating point value
/// Repeats 64-bit double pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val Double-precision float value to fill with
void sub__memfill_DOUBLE(mem_block *dblk, intptr_t doff, intptr_t dbytes, double val);

/// Fills memory block with double-precision float without bounds checking
/// High-performance double-precision float fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val Double-precision float value to fill with
void sub__memfill_nochecks_DOUBLE(intptr_t doff, intptr_t dbytes, double val);

/// Fills memory block with extended-precision floating point value
/// Repeats 80-bit long double pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val Extended-precision float value to fill with
void sub__memfill_FLOAT(mem_block *dblk, intptr_t doff, intptr_t dbytes, long double val);

/// Fills memory block with extended-precision float without bounds checking
/// High-performance extended-precision float fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val Extended-precision float value to fill with
void sub__memfill_nochecks_FLOAT(intptr_t doff, intptr_t dbytes, long double val);

/// Fills memory block with offset/pointer value
/// Repeats pointer-sized pattern across specified memory region
/// @param dblk Destination memory block
/// @param doff Destination offset within block
/// @param dbytes Number of bytes to fill
/// @param val Pointer/offset value to fill with
void sub__memfill_OFFSET(mem_block *dblk, intptr_t doff, intptr_t dbytes, intptr_t val);

/// Fills memory block with offset/pointer value without bounds checking
/// High-performance pointer/offset fill without safety validations
/// @param doff Destination offset within memory block
/// @param dbytes Number of bytes to fill
/// @param val Pointer/offset value to fill with
void sub__memfill_nochecks_OFFSET(intptr_t doff, intptr_t dbytes, intptr_t val);

// ============================================================================
// MEMORY ACCESS OPERATIONS
// ============================================================================

/// Reads data from memory block and returns as pointer
/// Extracts specified number of bytes from memory block at given offset
/// @param blk Memory block to read from
/// @param off Offset within memory block to start reading
/// @param bytes Number of bytes to read
/// @return Pointer to allocated buffer containing read data (caller must free)
void *func__memget(mem_block *blk, intptr_t off, intptr_t bytes);

// ============================================================================
// MEMORY LOCK MANAGEMENT
// ============================================================================

/// Creates a new memory lock entry
/// Initializes tracking for memory block security and management
void new_mem_lock();

/// Frees a memory lock entry
/// Releases memory lock and associated resources
/// @param lock Pointer to memory lock to free
void free_mem_lock(mem_lock *lock);

// ============================================================================
// MEMORY BLOCK CREATION AND MANAGEMENT
// ============================================================================

/// Creates a new memory block with specified parameters
/// Allocates and initializes a memory block with security tracking
/// @param offset Memory offset or pointer
/// @param size Size of memory block in bytes
/// @param type Memory type (MEM_TYPE_*)
/// @param elementsize Size of each element (for arrays)
/// @param lock Memory lock for security tracking
/// @return Initialized memory block structure
mem_block func__mem(intptr_t offset, intptr_t size, int32_t type, intptr_t elementsize, mem_lock *lock);

/// Creates memory block at specific offset
/// Creates memory block reference at existing memory location
/// @param offset Memory offset or pointer
/// @param size Size of memory region
/// @return Memory block structure for the offset
mem_block func__mem_at_offset(intptr_t offset, intptr_t size);

/// Allocates new memory block
/// Creates and allocates a new memory block of specified size
/// @param size Size of memory block to allocate
/// @return Memory block structure for allocated memory
mem_block func__memnew(intptr_t);

/// Frees memory block
/// Releases memory block and associated resources
/// @param Pointer to memory block to free
void sub__memfree(void *);

// ============================================================================
// MEMORY COPY OPERATIONS
// ============================================================================

/// Copies data between memory regions
/// Performs memory copy with proper overlap handling
/// @param sblk Source memory block or pointer
/// @param soff Source offset within source block
/// @param bytes Number of bytes to copy
/// @param dblk Destination memory block or pointer
/// @param doff Destination offset within destination block
void sub__memcopy(void *sblk, intptr_t soff, intptr_t bytes, void *dblk, intptr_t doff);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides comprehensive memory management functionality including:
// - Memory block allocation and tracking
// - Memory filling operations with various data types
// - Memory access and manipulation utilities
// - Memory lock management for security
// - Memory copy operations
//
// Supports different memory types including malloc'ed blocks, images, sounds,
// arrays, and sub/function scope blocks with appropriate security measures.

#endif // INCLUDE_LIBQB_MEM_H
