#ifndef INCLUDE_LIBQB_COMPLETION_H
#define INCLUDE_LIBQB_COMPLETION_H

#include "condvar.h"
#include "mutex.h"

/**
 * @file completion.h
 * @brief Completion synchronization primitive for QB64-PE
 * 
 * A completion is a oneshot signal - it waits until finish is called and
 * then never blocks again.
 * 
 * Due to the oneshot nature the order wait() and finish() are called does not matter.
 */

/**
 * @struct completion
 * @brief Completion synchronization structure
 * 
 * A completion provides a one-time synchronization mechanism. Once finished,
 * it will never block waiters again, regardless of when finish() was called.
 */
struct completion {
    int finished;                      ///< Flag indicating if completion is finished
    struct libqb_mutex *mutex;         ///< Mutex for thread synchronization
    struct libqb_condvar *var;         ///< Condition variable for waiting
};

/**
 * @brief Initializes a completion structure
 * @param completion Completion structure to initialize
 * @note Must be called before using the completion. Sets up mutex and condition variable.
 */
void completion_init(struct completion *);

/**
 * @brief Cleans up a completion structure
 * @param completion Completion structure to clean up
 * @note Releases resources associated with the completion. Do not use after calling this.
 */
void completion_clear(struct completion *);

/**
 * @brief Blocks until the completion is finished
 * @param completion Completion to wait for
 * @note Blocks the calling thread until completion_finish() is called. After finish() is called,
 *       this function will never block again for this completion.
 */
void completion_wait(struct completion *);

/**
 * @brief Finishes the completion, unblocks all waiters
 * @param completion Completion to finish
 * @note Marks the completion as finished and wakes all threads waiting on it.
 *       After this call, completion_wait() will never block again for this completion.
 */
void completion_finish(struct completion *);

#endif
