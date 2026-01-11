
#include "libqb-common.h"

#include <stdlib.h>
#include <string.h>

#include "gui.h"
#include "qblist.h"

/**
 * @file qblist.cpp
 * @brief Implementation of list data structure for QB64-PE
 * 
 * This file implements the list structure used for managing collections of objects
 * (such as handles) with efficient indexing and memory management.
 */

/**
 * @brief Creates a new list (not thread-safe)
 * @param structure_size Size of each structure in the list
 * @return Pointer to newly created list, or NULL on failure
 * @note Initializes a list with the specified structure size.
 *       The list does not use index 0.
 */
list *list_new(intptr_t structure_size) {
    list *L;
    L = (list *)calloc(1, sizeof(list));
    L->structure = (uint8_t *)malloc(sizeof(uint8_t *));
    L->structure_base[1] = (intptr_t)L->structure;
    L->structure_bases = 1;
    L->structure_freed = (intptr_t *)malloc(sizeof(intptr_t *));
    L->index = (intptr_t *)malloc(sizeof(intptr_t *));
    L->user_structure_size = structure_size;
    L->internal_structure_size = structure_size + sizeof(intptr_t);
    return L;
}

/**
 * @brief Creates a new thread-safe list
 * @param structure_size Size of each structure in the list
 * @return Pointer to newly created list, or NULL on failure
 * @note Creates a list with mutex protection for thread-safe add/remove operations.
 */
list *list_new_threadsafe(intptr_t structure_size) {
    list *L = list_new(structure_size);
    L->lock_add = libqb_mutex_new();
    L->lock_remove = libqb_mutex_new();
    return L;
}

/**
 * @brief Adds a new structure to the list
 * @param L List to add to
 * @return Index of the newly added structure, or 0 on failure
 * @note Thread-safe if list was created with list_new_threadsafe().
 *       Reuses freed structure slots when available. Expands buffers as needed.
 *       Index 0 is never returned.
 */
intptr_t list_add(list *L) {
    if (L->lock_add)
        libqb_mutex_lock(L->lock_add);

    intptr_t i;
    if (L->structures_freed) { // retrieve index from freed list if possible
        if (L->lock_remove)
            libqb_mutex_lock(L->lock_remove);

        i = L->structure_freed[L->structures_freed--];
        uint8_t *structure;
        structure = (uint8_t *)L->index[i];
        memset(structure, 0, L->user_structure_size);
        *(intptr_t *)(structure + L->user_structure_size) = i;

        if (L->lock_remove)
            libqb_mutex_unlock(L->lock_remove);
    } else {
        // create new buffer?
        if ((L->structures + 1) > L->structures_last) {
            intptr_t new_structures_last;
            new_structures_last = (L->structures_last * 2) + 1;
            // note: L->structure is only modified by list_add
            L->structure = (uint8_t *)calloc(1, L->internal_structure_size * (new_structures_last + 1));
            if (L->structure == NULL) {
                gui_alert("list_add: failed to allocate new buffer, structure size: %lld", (int64_t)L->internal_structure_size);
            }
            L->structures_last = new_structures_last;
            L->structures = 0;
            L->structure_base[++L->structure_bases] = (intptr_t)L->structure;
        }
        i = ++L->indexes;
        *(intptr_t *)(L->structure + (L->internal_structure_size * (++L->structures)) + L->user_structure_size) = i;
        // allocate new index
        if (L->indexes > L->indexes_last) {
            if (L->index_cleanup != NULL)
                free(L->index_cleanup);
            L->index_cleanup = L->index;
            int32_t new_indexes_last = (L->indexes_last * 2) + 1;
            intptr_t *temp = (intptr_t *)malloc(sizeof(intptr_t) * (new_indexes_last + 1));
            memcpy(temp, L->index, sizeof(intptr_t) * (L->indexes_last + 1));
            L->index = temp;
            L->index[i] = (intptr_t)(L->structure + (L->internal_structure_size * L->structures));
            L->indexes_last = new_indexes_last;
        } else {
            L->index[i] = (intptr_t)(L->structure + (L->internal_structure_size * L->structures));
        }
    }

    if (L->lock_add)
        libqb_mutex_unlock(L->lock_add);

    return i;
} // list_add

/**
 * @brief Removes a structure from the list
 * @param L List to remove from
 * @param i Index of the structure to remove
 * @return -1 on success, 0 on failure
 * @note Thread-safe if list was created with list_new_threadsafe().
 *       The structure is marked as freed and can be reused by future list_add() calls.
 *       Expands the freed list buffer if needed.
 */
intptr_t list_remove(list *L, intptr_t i) { // returns -1 on success, 0 on failure
    if (L->lock_remove)
        libqb_mutex_lock(L->lock_remove);

    if ((i < 1) || (i > L->indexes)) {
        if (L->lock_remove)
            libqb_mutex_unlock(L->lock_remove);

        return 0;
    }
    uint8_t *structure;
    structure = (uint8_t *)(L->index[i]);
    if (!*(intptr_t *)(structure + L->user_structure_size)) {
        if (L->lock_remove)
            libqb_mutex_unlock(L->lock_remove);

        return 0;
    }
    // expand buffer?
    if ((L->structures_freed + 1) > L->structures_freed_last) {
        intptr_t new_structures_freed_last;
        new_structures_freed_last = (L->structures_freed_last * 2) + 1;
        intptr_t *temp = (intptr_t *)malloc(sizeof(intptr_t) * (new_structures_freed_last + 1));
        memcpy(temp, L->structure_freed, sizeof(intptr_t) * (L->structures_freed + 1));
        if (L->structure_freed_cleanup != NULL)
            free(L->structure_freed_cleanup);
        L->structure_freed_cleanup = L->structure_freed;
        L->structure_freed = temp;
        L->structures_freed_last = new_structures_freed_last;
    }
    L->structure_freed[L->structures_freed + 1] = i;
    *(intptr_t *)(structure + L->user_structure_size) = 0;
    L->structures_freed++;

    if (L->lock_remove)
        libqb_mutex_unlock(L->lock_remove);

    return -1;
};

/**
 * @brief Destroys a list and frees all resources
 * @param L List to destroy
 * @note Frees all structure base blocks, freed list, and the list structure itself.
 *       Do not use the list after calling this.
 */
void list_destroy(list *L) {
    intptr_t i;
    for (i = 1; i <= L->structure_bases; i++) {
        free((void *)L->structure_base[i]);
    }
    free(L->structure_freed);
    free(L);
}

/**
 * @brief Gets a pointer to a structure by index
 * @param L List to query
 * @param i Index of the structure
 * @return Pointer to the structure, or NULL if index is invalid or structure is freed
 * @note The returned pointer is valid until the structure is removed or the list is destroyed.
 */
void *list_get(list *L, intptr_t i) { // Returns a pointer to an index's structure
    if ((i < 1) || (i > L->indexes)) {
        return NULL;
    }
    uint8_t *structure;
    structure = (uint8_t *)(L->index[i]);
    if (!*(intptr_t *)(structure + L->user_structure_size))
        return NULL;
    return (void *)structure;
}

/**
 * @brief Gets the index of a structure from its pointer
 * @param L List to query
 * @param structure Pointer to the structure
 * @return Index of the structure, or 0 if invalid
 * @note Useful for finding the index when you only have a pointer to the structure.
 *       The index is stored at the end of each structure.
 */
intptr_t list_get_index(list *L, void *structure) { // Retrieves the index value of a structure
    intptr_t i = *(intptr_t *)(((uint8_t *)structure) + L->user_structure_size);
    return i;
}
