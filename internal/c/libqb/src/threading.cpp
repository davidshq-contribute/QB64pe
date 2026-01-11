
#include "libqb-common.h"

#include "completion.h"
#include "condvar.h"
#include "mutex.h"

/**
 * @file threading.cpp
 * @brief Implementation of completion synchronization primitive for QB64-PE
 * 
 * This file implements the completion synchronization primitive, which allows
 * threads to wait for a task to complete.
 */

/**
 * @brief Initializes a completion structure
 * @param comp Completion structure to initialize
 * @note Creates a new mutex and condition variable for the completion.
 *       Sets finished flag to 0.
 */
void completion_init(struct completion *comp) {
    comp->finished = 0;
    comp->mutex = libqb_mutex_new();
    comp->var = libqb_condvar_new();
}

/**
 * @brief Clears a completion structure
 * @param comp Completion structure to clear
 * @note Frees the mutex and condition variable. Do not use the completion after this.
 */
void completion_clear(struct completion *comp) {
    libqb_mutex_free(comp->mutex);
    libqb_condvar_free(comp->var);
}

/**
 * @brief Waits for a completion to finish
 * @param comp Completion structure to wait on
 * @note Blocks until completion_finish() is called. Uses condition variable
 *       to avoid busy waiting.
 */
void completion_wait(struct completion *comp) {
    libqb_mutex_guard guard(comp->mutex);

    while (!comp->finished)
        libqb_condvar_wait(comp->var, comp->mutex);
}

/**
 * @brief Marks a completion as finished
 * @param comp Completion structure to mark as finished
 * @note Sets the finished flag and broadcasts to all waiting threads.
 *       All threads waiting in completion_wait() will be woken up.
 */
void completion_finish(struct completion *comp) {
    libqb_mutex_guard guard(comp->mutex);

    comp->finished = 1;
    libqb_condvar_broadcast(comp->var);
}
