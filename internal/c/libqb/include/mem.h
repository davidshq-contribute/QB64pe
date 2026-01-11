#pragma once

#include <stdint.h>

/**
 * @file mem.h
 * @brief Memory management functions for QB64-PE
 * 
 * This header provides functions for managing memory blocks, including allocation,
 * deallocation, copying, and filling operations. Memory blocks are tracked with
 * locks to ensure safe access and prevent use-after-free errors.
 */

/**
 * @struct mem_block
 * @brief Represents a memory block with metadata
 * 
 * This structure describes a memory region that can be accessed through QB64's
 * memory functions. Each block has a lock ID that must match the lock at the
 * lock_offset to be valid.
 */
struct mem_block {
    intptr_t offset;      ///< Memory offset (pointer value)
    intptr_t size;        ///< Size of the memory block in bytes
    int64_t lock_id;      ///< 64-bit key that must match lock's ID for validity
    intptr_t lock_offset; ///< Pointer to the mem_lock structure
    intptr_t type;        ///< Memory type (see MEM_TYPE_* constants)
    intptr_t elementsize; ///< Size of each element (for arrays)
    int32_t image;        ///< Associated image handle (if type is MEM_TYPE_IMAGE)
    int32_t sound;        ///< Associated sound handle (if type is MEM_TYPE_SOUND)
};

#define INVALID_MEM_LOCK 1073741821 ///< Invalid memory lock identifier

/**
 * @name Memory Type Constants
 * @brief Memory block type identifiers
 * @see https://qb64phoenix.com/qb64wiki/index.php/MEM
 */
///@{
#define MEM_TYPE_NOSECURITY 0 ///< No security (user-defined block from _OFFSET)
#define MEM_TYPE_MALLOC 1    ///< C malloc'ed block
#define MEM_TYPE_IMAGE 2     ///< Image memory block
#define MEM_TYPE_SUBFUNC 3   ///< Sub/function scope block
#define MEM_TYPE_ARRAY 4     ///< Array memory block
#define MEM_TYPE_SOUND 5     ///< Sound memory block
///@}

/**
 * @struct mem_lock
 * @brief Memory lock structure for tracking memory blocks
 * 
 * Each memory block has an associated lock that validates the block's existence
 * and tracks its type for proper cleanup.
 */
struct mem_lock {
    int64_t id;   ///< Unique lock identifier
    int32_t type; ///< Memory type (see MEM_TYPE_* constants)
    void *offset; ///< Pointer to allocated memory (used by malloc'ed blocks for freeing)
};

extern uint64_t mem_lock_id;      ///< Global counter for generating unique lock IDs
extern mem_lock *mem_lock_tmp;    ///< Temporary lock pointer
extern mem_lock *mem_lock_base;   ///< Base of the lock array

/**
 * @brief Checks if a memory block exists and is valid
 * @param blk Pointer to the memory block to check
 * @return Non-zero if block exists and is valid, 0 otherwise
 * @note Validates that the block's lock ID matches the lock at the lock offset
 */
int32_t func__memexists(void *blk);

/**
 * @brief Fills memory by copying from source to destination (QB64 _MEMFILL statement)
 * @param dblk Destination memory block
 * @param doff Offset within destination block
 * @param dbytes Number of bytes to fill
 * @param soff Source offset
 * @param sbytes Number of bytes to copy from source
 * @note Performs bounds checking. Copies sbytes from source, repeating if dbytes > sbytes.
 */
void sub__memfill(mem_block *dblk, intptr_t doff, intptr_t dbytes, intptr_t soff, intptr_t sbytes);

/**
 * @brief Fills memory by copying from source to destination without bounds checking
 * @param doff Destination offset
 * @param dbytes Number of bytes to fill
 * @param soff Source offset
 * @param sbytes Number of bytes to copy from source
 * @warning No bounds checking is performed. Use only when bounds are guaranteed.
 */
void sub__memfill_nochecks(intptr_t doff, intptr_t dbytes, intptr_t soff, intptr_t sbytes);

/**
 * @name Typed Memory Fill Functions
 * @brief Fill memory with a specific value of a given type
 * @param dblk Destination memory block (checked versions only)
 * @param doff Offset within destination block
 * @param dbytes Number of bytes to fill
 * @param val Value to fill with
 * @note The _nochecks versions skip bounds checking. Use with caution.
 */
///@{
void sub__memfill_1(mem_block *dblk, intptr_t doff, intptr_t dbytes, int8_t val);
void sub__memfill_nochecks_1(intptr_t doff, intptr_t dbytes, int8_t val);
void sub__memfill_2(mem_block *dblk, intptr_t doff, intptr_t dbytes, int16_t val);
void sub__memfill_nochecks_2(intptr_t doff, intptr_t dbytes, int16_t val);
void sub__memfill_4(mem_block *dblk, intptr_t doff, intptr_t dbytes, int32_t val);
void sub__memfill_nochecks_4(intptr_t doff, intptr_t dbytes, int32_t val);
void sub__memfill_8(mem_block *dblk, intptr_t doff, intptr_t dbytes, int64_t val);
void sub__memfill_nochecks_8(intptr_t doff, intptr_t dbytes, int64_t val);
void sub__memfill_SINGLE(mem_block *dblk, intptr_t doff, intptr_t dbytes, float val);
void sub__memfill_nochecks_SINGLE(intptr_t doff, intptr_t dbytes, float val);
void sub__memfill_DOUBLE(mem_block *dblk, intptr_t doff, intptr_t dbytes, double val);
void sub__memfill_nochecks_DOUBLE(intptr_t doff, intptr_t dbytes, double val);
void sub__memfill_FLOAT(mem_block *dblk, intptr_t doff, intptr_t dbytes, long double val);
void sub__memfill_nochecks_FLOAT(intptr_t doff, intptr_t dbytes, long double val);
void sub__memfill_OFFSET(mem_block *dblk, intptr_t doff, intptr_t dbytes, intptr_t val);
void sub__memfill_nochecks_OFFSET(intptr_t doff, intptr_t dbytes, intptr_t val);
///@}

/**
 * @brief Gets a pointer to memory within a block (QB64 _MEMGET function)
 * @param blk Memory block
 * @param off Offset within the block
 * @param bytes Number of bytes to access
 * @return Pointer to the memory location, or NULL if invalid
 * @note Returns a pointer that can be used to access the memory. No bounds checking is performed.
 * @warning The returned pointer is only valid while the block exists and is locked.
 */
void *func__memget(mem_block *blk, intptr_t off, intptr_t bytes);

/**
 * @brief Creates a new memory lock
 * @note Allocates a new lock structure. Use free_mem_lock() to release it.
 */
void new_mem_lock();

/**
 * @brief Frees a memory lock
 * @param lock Lock to free
 * @note Releases the lock structure. Do not use the lock after calling this.
 */
void free_mem_lock(mem_lock *lock);

/**
 * @brief Creates a memory block descriptor (QB64 _MEM function)
 * @param offset Memory offset (pointer value)
 * @param size Size of the memory block
 * @param type Memory type (see MEM_TYPE_* constants)
 * @param elementsize Size of each element (for arrays)
 * @param lock Memory lock to associate with the block
 * @return mem_block structure describing the memory region
 * @note Creates a descriptor for an existing memory region. The lock validates the block.
 */
mem_block func__mem(intptr_t offset, intptr_t size, int32_t type, intptr_t elementsize, mem_lock *lock);

/**
 * @brief Creates a memory block descriptor at a specific offset
 * @param offset Memory offset (pointer value)
 * @param size Size of the memory block
 * @return mem_block structure describing the memory region
 * @note Creates a descriptor without a lock. Use with caution.
 */
mem_block func__mem_at_offset(intptr_t offset, intptr_t size);

/**
 * @brief Allocates a new memory block (QB64 _MEMNEW function)
 * @param size Size of memory to allocate in bytes
 * @return mem_block structure describing the allocated memory
 * @note Allocates memory using malloc. Use sub__memfree() to free it.
 */
mem_block func__memnew(intptr_t size);

/**
 * @brief Frees a memory block (QB64 _MEMFREE statement)
 * @param blk Pointer to the memory block to free
 * @note Frees the memory associated with the block. The block becomes invalid after this call.
 */
void sub__memfree(void *blk);

/**
 * @brief Copies memory from source to destination (QB64 _MEMCOPY statement)
 * @param sblk Source memory block
 * @param soff Offset within source block
 * @param bytes Number of bytes to copy
 * @param dblk Destination memory block
 * @param doff Offset within destination block
 * @note Performs bounds checking. Both blocks must be valid.
 */
void sub__memcopy(void *sblk, intptr_t soff, intptr_t bytes, void *dblk, intptr_t doff);
