#ifndef INCLUDE_LIBQB_CONDVAR_H
#define INCLUDE_LIBQB_CONDVAR_H

#include "mutex.h"

/**
 * @file condvar.h
 * @brief Condition variable synchronization primitive for QB64-PE
 * 
 * This header provides condition variable functionality for thread synchronization.
 * Condition variables allow threads to wait for a condition to become true.
 */

/**
 * @struct libqb_condvar
 * @brief Opaque condition variable structure
 * 
 * A condition variable is a synchronization primitive that allows threads to wait
 * for a condition to become true. Must be used with a mutex.
 */
struct libqb_condvar;

/**
 * @brief Allocates a new condition variable
 * @return Pointer to newly allocated condition variable, or NULL on failure
 * @note The condition variable is initialized and ready to use
 */
struct libqb_condvar *libqb_condvar_new();

/**
 * @brief Frees a condition variable
 * @param condvar Condition variable to free
 * @note Do not use the condition variable after calling this function
 */
void libqb_condvar_free(struct libqb_condvar *);

/**
 * @brief Waits until the condition variable is signalled
 * @param condvar Condition variable to wait on
 * @param mutex Mutex to atomically unlock while waiting
 * @note Atomically unlocks the mutex and waits for the condition variable to be signalled.
 *       The mutex is re-locked before the function returns.
 */
void libqb_condvar_wait(struct libqb_condvar *, struct libqb_mutex *);

/**
 * @brief Signals a single thread waiting on the condition variable
 * @param condvar Condition variable to signal
 * @note Wakes up one thread that is waiting on this condition variable
 */
void libqb_condvar_signal(struct libqb_condvar *);

/**
 * @brief Signals all threads waiting on the condition variable
 * @param condvar Condition variable to broadcast
 * @note Wakes up all threads that are waiting on this condition variable
 */
void libqb_condvar_broadcast(struct libqb_condvar *);

#endif
