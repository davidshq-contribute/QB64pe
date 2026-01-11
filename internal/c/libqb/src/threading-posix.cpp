
#include "libqb-common.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "mutex.h"

/**
 * @file threading-posix.cpp
 * @brief POSIX implementation of threading primitives for QB64-PE
 * 
 * This file implements mutexes, condition variables, and threads using POSIX pthread APIs.
 */

/**
 * @brief Thread structure (POSIX implementation)
 */
struct libqb_thread {
    pthread_t thread;
};

/**
 * @brief Mutex structure (POSIX implementation)
 */
struct libqb_mutex {
    pthread_mutex_t mtx;
};

/**
 * @brief Condition variable structure (POSIX implementation)
 */
struct libqb_condvar {
    pthread_cond_t var;
};

/**
 * @brief Creates a new mutex (POSIX implementation)
 * @return Pointer to newly allocated mutex, or NULL on failure
 * @note Uses pthread_mutex_init with default attributes.
 */
struct libqb_mutex *libqb_mutex_new() {
    struct libqb_mutex *m = (struct libqb_mutex *)malloc(sizeof(*m));
    pthread_mutex_init(&m->mtx, NULL);
    return m;
}

/**
 * @brief Frees a mutex (POSIX implementation)
 * @param mutex Mutex to free
 * @note Destroys the pthread mutex and frees the mutex structure.
 */
void libqb_mutex_free(struct libqb_mutex *mutex) {
    pthread_mutex_destroy(&mutex->mtx);
    free(mutex);
}

/**
 * @brief Locks a mutex (POSIX implementation)
 * @param m Mutex to lock
 * @note Blocks until the mutex is available. Uses pthread_mutex_lock.
 */
void libqb_mutex_lock(struct libqb_mutex *m) {
    pthread_mutex_lock(&m->mtx);
}

/**
 * @brief Unlocks a mutex (POSIX implementation)
 * @param m Mutex to unlock
 * @note Releases the mutex. Uses pthread_mutex_unlock.
 */
void libqb_mutex_unlock(struct libqb_mutex *m) {
    pthread_mutex_unlock(&m->mtx);
}

/**
 * @brief Creates a new condition variable (POSIX implementation)
 * @return Pointer to newly allocated condition variable, or NULL on failure
 * @note Uses pthread_cond_init with default attributes.
 */
struct libqb_condvar *libqb_condvar_new() {
    struct libqb_condvar *c = (struct libqb_condvar *)malloc(sizeof(*c));
    pthread_cond_init(&c->var, NULL);
    return c;
}

/**
 * @brief Frees a condition variable (POSIX implementation)
 * @param c Condition variable to free
 * @note Destroys the pthread condition variable and frees the structure.
 */
void libqb_condvar_free(struct libqb_condvar *c) {
    pthread_cond_destroy(&c->var);
    free(c);
}

/**
 * @brief Waits on a condition variable (POSIX implementation)
 * @param condvar Condition variable to wait on
 * @param mutex Mutex to release while waiting
 * @note Atomically releases the mutex and waits. Reacquires mutex before returning.
 *       Uses pthread_cond_wait.
 */
void libqb_condvar_wait(struct libqb_condvar *condvar, struct libqb_mutex *mutex) {
    pthread_cond_wait(&condvar->var, &mutex->mtx);
}

/**
 * @brief Signals a condition variable (POSIX implementation)
 * @param condvar Condition variable to signal
 * @note Wakes one waiting thread. Uses pthread_cond_signal.
 */
void libqb_condvar_signal(struct libqb_condvar *condvar) {
    pthread_cond_signal(&condvar->var);
}

/**
 * @brief Broadcasts to a condition variable (POSIX implementation)
 * @param condvar Condition variable to broadcast
 * @note Wakes all waiting threads. Uses pthread_cond_broadcast.
 */
void libqb_condvar_broadcast(struct libqb_condvar *condvar) {
    pthread_cond_broadcast(&condvar->var);
}

/**
 * @brief Creates a new thread structure (POSIX implementation)
 * @return Pointer to newly allocated thread structure, or NULL on failure
 * @note The thread is not started when this returns. Use libqb_thread_start() to begin execution.
 */
struct libqb_thread *libqb_thread_new() {
    struct libqb_thread *t = (struct libqb_thread *)malloc(sizeof(*t));
    memset(t, 0, sizeof(*t));

    return t;
}

/**
 * @brief Frees a thread structure (POSIX implementation)
 * @param t Thread to free
 * @note The thread should have already been joined before calling this.
 *       Frees the thread structure.
 */
void libqb_thread_free(struct libqb_thread *t) {
    // The thread should have already have been joined.
    free(t);
}

/**
 * @brief Thread wrapper arguments
 * @note Used to pass function pointer and argument to thread wrapper.
 */
struct thread_wrapper_args {
    void (*wrapper)(void *);
    void *arg;
};

/**
 * @brief Wrapper function for thread start (POSIX implementation)
 * @param varg Pointer to thread_wrapper_args structure
 * @return Always NULL
 * @note Calls the user's start function with the provided argument, then frees the wrapper args.
 */
static void *thread_wrapper(void *varg) {
    struct thread_wrapper_args *arg = (struct thread_wrapper_args *)varg;
    (arg->wrapper)(arg->arg);
    free(arg);

    return NULL;
}

/**
 * @brief Starts a thread (POSIX implementation)
 * @param t Thread structure to start
 * @param start_func Function to run in the thread
 * @param start_func_arg Argument to pass to the function
 * @note Uses pthread_create to create the thread. The wrapper converts the function
 *       to the format required by pthread_create.
 */
void libqb_thread_start(struct libqb_thread *t, void (*start_func)(void *), void *start_func_arg) {
    struct thread_wrapper_args *arg = (struct thread_wrapper_args *)malloc(sizeof(*arg));
    arg->wrapper = start_func;
    arg->arg = start_func_arg;

    pthread_create(&t->thread, NULL, thread_wrapper, (void *)arg);
}

/**
 * @brief Joins a thread (POSIX implementation)
 * @param t Thread to join
 * @note Blocks until the thread completes execution. Uses pthread_join.
 */
void libqb_thread_join(struct libqb_thread *t) {
    pthread_join(t->thread, NULL);
}
