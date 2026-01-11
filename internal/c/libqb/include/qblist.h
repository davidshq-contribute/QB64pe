#pragma once

#include "mutex.h"
#include <stdint.h>

/**
 * @file qblist.h
 * @brief List data structure for QB64-PE
 * 
 * This header provides a unified and optimized list interface for managing
 * collections of objects (such as handles) in QB64. The list does not use index 0.
 */

/**
 * @struct list
 * @brief List structure for managing collections of objects
 * 
 * This structure provides an efficient way to manage lists of objects with
 * automatic memory management and fast indexing. Index 0 is not used.
 */
struct list {
    intptr_t user_structure_size;        ///< Size of user-defined structure
    intptr_t internal_structure_size;    ///< Internal structure size
    uint8_t *structure;                  ///< Block of structures of user-specified size
    intptr_t structures;                 ///< Current number of structures
    intptr_t structures_last;            ///< Last structure index
    intptr_t *structure_freed;           ///< Array of freed structure indices for quick re-use
    intptr_t *structure_freed_cleanup;   ///< Previous structure_freed memory block (for cleanup)
    intptr_t structures_freed;           ///< Number of freed structures
    intptr_t structures_freed_last;      ///< Last freed structure index
    intptr_t structure_base[64];         ///< Base pointers for structure blocks (list doubles each time, so 64 entries is sufficient)
    intptr_t structure_bases;            ///< Number of structure base blocks
    intptr_t *index;                     ///< Pointers to structures indexed by index value
    intptr_t *index_cleanup;             ///< Previous index memory block (for cleanup)
    intptr_t indexes;                    ///< Current number of indexes
    intptr_t indexes_last;               ///< Last index value
    struct libqb_mutex *lock_add;        ///< Mutex for thread-safe additions
    struct libqb_mutex *lock_remove;     ///< Mutex for thread-safe removals
};

/**
 * @brief Creates a new list (not thread-safe)
 * @param structure_size Size of each structure in the list
 * @return Pointer to newly created list, or NULL on failure
 * @note Creates a list that is not thread-safe. Use list_new_threadsafe() for thread-safe operations.
 */
list *list_new(intptr_t structure_size);

/**
 * @brief Creates a new thread-safe list
 * @param structure_size Size of each structure in the list
 * @return Pointer to newly created list, or NULL on failure
 * @note Creates a list with mutex protection for thread-safe add/remove operations.
 */
list *list_new_threadsafe(intptr_t structure_size);

/**
 * @brief Destroys a list and frees all resources
 * @param L List to destroy
 * @note Frees all memory associated with the list. Do not use the list after calling this.
 */
void list_destroy(list *L);

/**
 * @brief Adds a new structure to the list
 * @param L List to add to
 * @return Index of the newly added structure, or 0 on failure
 * @note Returns an index that can be used to access the structure. Index 0 is never returned.
 */
intptr_t list_add(list *L);

/**
 * @brief Removes a structure from the list
 * @param L List to remove from
 * @param i Index of the structure to remove
 * @return -1 on success, 0 on failure
 * @note The structure is marked as freed and can be reused by future list_add() calls.
 */
intptr_t list_remove(list *L, intptr_t i);

/**
 * @brief Gets a pointer to a structure by index
 * @param L List to query
 * @param i Index of the structure
 * @return Pointer to the structure, or NULL if index is invalid
 * @note The returned pointer is valid until the structure is removed or the list is destroyed.
 */
void *list_get(list *L, intptr_t i);

/**
 * @brief Gets the index of a structure from its pointer
 * @param L List to query
 * @param structure Pointer to the structure
 * @return Index of the structure, or 0 if not found
 * @note Useful for finding the index when you only have a pointer to the structure.
 */
intptr_t list_get_index(list *L, void *structure);
