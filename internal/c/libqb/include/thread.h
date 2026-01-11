#ifndef INCLUDE_LIBQB_THREAD_H
#define INCLUDE_LIBQB_THREAD_H

/**
 * @file thread.h
 * @brief Thread management functions for QB64-PE
 * 
 * This header provides functions for creating, starting, and managing threads.
 */

/**
 * @struct libqb_thread
 * @brief Opaque thread structure
 * 
 * Represents a thread that can execute code concurrently with other threads.
 */
struct libqb_thread;

/**
 * @brief Allocates a new thread structure
 * @return Pointer to newly allocated thread structure, or NULL on failure
 * @note The thread is not running when this returns. Use libqb_thread_start() to begin execution.
 *       The thread must be stopped/joined before calling libqb_thread_free().
 */
struct libqb_thread *libqb_thread_new();

/**
 * @brief Frees a thread structure
 * @param thread Thread to free
 * @note The thread must already be stopped and joined before calling this function.
 *       Do not use the thread after calling this function.
 */
void libqb_thread_free(struct libqb_thread *);

/**
 * @brief Configures and starts a thread to begin its execution
 * @param thread Thread structure to start
 * @param start_func Function pointer to the function that will run in the thread
 * @param arg Argument to pass to the start function
 * @note Begins execution of the thread. The start function will be called with arg as its parameter.
 */
void libqb_thread_start(struct libqb_thread *, void (*start_func)(void *), void *arg);

/**
 * @brief Joins a thread to wait for its execution to complete
 * @param thread Thread to join
 * @note Blocks until the thread completes execution. Must be called before freeing the thread.
 */
void libqb_thread_join(struct libqb_thread *);

#endif
