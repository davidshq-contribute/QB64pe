
#include "libqb-common.h"

#include <process.h>
#include <synchapi.h>
#include <windows.h>

#include "condvar.h"
#include "mutex.h"
#include "thread.h"

/**
 * @file threading-windows.cpp
 * @brief Windows implementation of threading primitives for QB64-PE
 * 
 * This file implements mutexes, condition variables, and threads using Windows APIs.
 */

/**
 * @brief Thread structure (Windows implementation)
 */
struct libqb_thread {
    HANDLE thread_handle;
};

/**
 * @brief Mutex structure (Windows implementation)
 */
struct libqb_mutex {
    CRITICAL_SECTION crit_section;
};

/**
 * @brief Condition variable structure (Windows implementation)
 */
struct libqb_condvar {
    CONDITION_VARIABLE var;
};

/**
 * @brief Creates a new mutex (Windows implementation)
 * @return Pointer to newly allocated mutex, or NULL on failure
 * @note Uses CRITICAL_SECTION with spin count of 200 for better performance.
 */
struct libqb_mutex *libqb_mutex_new() {
    struct libqb_mutex *m = (struct libqb_mutex *)malloc(sizeof(*m));

    InitializeCriticalSectionAndSpinCount(&m->crit_section, 200);
    return m;
}

/**
 * @brief Frees a mutex (Windows implementation)
 * @param mutex Mutex to free
 * @note Deletes the critical section and frees the mutex structure.
 */
void libqb_mutex_free(struct libqb_mutex *mutex) {
    DeleteCriticalSection(&mutex->crit_section);
    free(mutex);
}

/**
 * @brief Locks a mutex (Windows implementation)
 * @param m Mutex to lock
 * @note Blocks until the mutex is available. Uses EnterCriticalSection.
 */
void libqb_mutex_lock(struct libqb_mutex *m) {
    EnterCriticalSection(&m->crit_section);
}

/**
 * @brief Unlocks a mutex (Windows implementation)
 * @param m Mutex to unlock
 * @note Releases the mutex. Uses LeaveCriticalSection.
 */
void libqb_mutex_unlock(struct libqb_mutex *m) {
    LeaveCriticalSection(&m->crit_section);
}

/**
 * @brief Creates a new condition variable (Windows implementation)
 * @return Pointer to newly allocated condition variable, or NULL on failure
 * @note Uses Windows CONDITION_VARIABLE.
 */
struct libqb_condvar *libqb_condvar_new() {
    struct libqb_condvar *condvar = (struct libqb_condvar *)malloc(sizeof(*condvar));

    InitializeConditionVariable(&condvar->var);
    return condvar;
}

/**
 * @brief Frees a condition variable (Windows implementation)
 * @param condvar Condition variable to free
 * @note Frees the condition variable structure. No cleanup needed for CONDITION_VARIABLE.
 */
void libqb_condvar_free(struct libqb_condvar *condvar) {
    free(condvar);
}

/**
 * @brief Waits on a condition variable (Windows implementation)
 * @param condvar Condition variable to wait on
 * @param mutex Mutex to release while waiting
 * @note Atomically releases the mutex and waits. Reacquires mutex before returning.
 *       Uses SleepConditionVariableCS with infinite timeout.
 */
void libqb_condvar_wait(struct libqb_condvar *condvar, struct libqb_mutex *mutex) {
    SleepConditionVariableCS(&condvar->var, &mutex->crit_section, INFINITE);
}

/**
 * @brief Signals a condition variable (Windows implementation)
 * @param condvar Condition variable to signal
 * @note Wakes one waiting thread. Uses WakeConditionVariable.
 */
void libqb_condvar_signal(struct libqb_condvar *condvar) {
    WakeConditionVariable(&condvar->var);
}

/**
 * @brief Broadcasts to a condition variable (Windows implementation)
 * @param condvar Condition variable to broadcast
 * @note Wakes all waiting threads. Uses WakeAllConditionVariable.
 */
void libqb_condvar_broadcast(struct libqb_condvar *condvar) {
    WakeAllConditionVariable(&condvar->var);
}

/**
 * @brief Creates a new thread structure (Windows implementation)
 * @return Pointer to newly allocated thread structure, or NULL on failure
 * @note The thread is not started when this returns. Use libqb_thread_start() to begin execution.
 */
struct libqb_thread *libqb_thread_new() {
    struct libqb_thread *t = (struct libqb_thread *)malloc(sizeof(*t));
    t->thread_handle = 0;

    return t;
}

/**
 * @brief Frees a thread structure (Windows implementation)
 * @param t Thread to free
 * @note The handle is closed automatically when using _beginthreadex.
 *       The thread must already be stopped before calling this.
 */
void libqb_thread_free(struct libqb_thread *t) {
    // The handle is closed automatically when using _beginthreadex
    free(t);
}

/**
 * @brief Thread wrapper arguments
 * @note Used to convert portable function pointer to __stdcall for Windows.
 */
struct thread_wrapper_args {
    void (*wrapper)(void *);
    void *arg;
};

/**
 * @brief Wrapper function for thread start (Windows implementation)
 * @param varg Pointer to thread_wrapper_args structure
 * @return Thread exit code (always 0)
 * @note This wrapper allows the caller to provide a portable function pointer
 *       instead of requiring a __stdcall function. Frees the wrapper args after execution.
 */
static unsigned int __stdcall stdcall_thread_wrapper(void *varg) {
    struct thread_wrapper_args *arg = (struct thread_wrapper_args *)varg;
    (arg->wrapper)(arg->arg);
    free(arg);

    return 0;
}

/**
 * @brief Starts a thread (Windows implementation)
 * @param t Thread structure to start
 * @param start_func Function to run in the thread
 * @param start_func_arg Argument to pass to the function
 * @note Uses _beginthreadex to create the thread. The wrapper converts the function
 *       to __stdcall format required by Windows.
 */
void libqb_thread_start(struct libqb_thread *t, void (*start_func)(void *), void *start_func_arg) {
    struct thread_wrapper_args *arg = (struct thread_wrapper_args *)malloc(sizeof(*arg));
    arg->wrapper = start_func;
    arg->arg = start_func_arg;

    t->thread_handle = (HANDLE)_beginthreadex(NULL, 0, stdcall_thread_wrapper, arg, 0, NULL);
}

void libqb_thread_join(struct libqb_thread *t) {
    WaitForSingleObject(t->thread_handle, INFINITE);
}
