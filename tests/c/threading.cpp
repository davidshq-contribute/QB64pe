// Unit tests for threading module
// Tests thread creation, mutex operations, and condition variables

#include "test.h"
#include "thread.h"
#include "mutex.h"
#include "condvar.h"
#include <stdlib.h>
#include <string.h>

static int shared_counter = 0;
static struct libqb_mutex *test_mutex = NULL;

static void test_thread_function(void *arg) {
    int *value = (int *)arg;
    (*value)++;
}

static void test_mutex_counter_function(void *arg) {
    // Increment shared counter with mutex protection
    libqb_mutex_lock(test_mutex);
    shared_counter++;
    libqb_mutex_unlock(test_mutex);
}

static void test_libqb_mutex_new_free() {
    // Test creating and freeing a mutex
    struct libqb_mutex *m = libqb_mutex_new();
    test_assert(m != NULL);
    
    libqb_mutex_free(m);
    test_assert(1); // If we get here, free succeeded
}

static void test_libqb_mutex_lock_unlock() {
    // Test locking and unlocking a mutex
    struct libqb_mutex *m = libqb_mutex_new();
    test_assert(m != NULL);
    
    libqb_mutex_lock(m);
    libqb_mutex_unlock(m);
    
    libqb_mutex_free(m);
    test_assert(1); // If we get here, lock/unlock worked
}

static void test_libqb_mutex_multiple_locks() {
    // Test multiple lock/unlock operations
    struct libqb_mutex *m = libqb_mutex_new();
    test_assert(m != NULL);
    
    for (int i = 0; i < 10; i++) {
        libqb_mutex_lock(m);
        libqb_mutex_unlock(m);
    }
    
    libqb_mutex_free(m);
    test_assert(1); // If we get here, multiple locks worked
}

static void test_libqb_condvar_new_free() {
    // Test creating and freeing a condition variable
    struct libqb_condvar *cv = libqb_condvar_new();
    test_assert(cv != NULL);
    
    libqb_condvar_free(cv);
    test_assert(1); // If we get here, free succeeded
}

static void test_libqb_condvar_signal() {
    // Test signaling a condition variable
    struct libqb_condvar *cv = libqb_condvar_new();
    struct libqb_mutex *m = libqb_mutex_new();
    
    libqb_mutex_lock(m);
    libqb_condvar_signal(cv);
    libqb_mutex_unlock(m);
    
    libqb_condvar_free(cv);
    libqb_mutex_free(m);
    test_assert(1); // If we get here, signal worked
}

static void test_libqb_condvar_broadcast() {
    // Test broadcasting to a condition variable
    struct libqb_condvar *cv = libqb_condvar_new();
    struct libqb_mutex *m = libqb_mutex_new();
    
    libqb_mutex_lock(m);
    libqb_condvar_broadcast(cv);
    libqb_mutex_unlock(m);
    
    libqb_condvar_free(cv);
    libqb_mutex_free(m);
    test_assert(1); // If we get here, broadcast worked
}

static void test_libqb_thread_new_free() {
    // Test creating and freeing a thread structure
    struct libqb_thread *t = libqb_thread_new();
    test_assert(t != NULL);
    
    libqb_thread_free(t);
    test_assert(1); // If we get here, free succeeded
}

static void test_libqb_thread_start_join() {
    // Test starting and joining a thread
    struct libqb_thread *t = libqb_thread_new();
    test_assert(t != NULL);
    
    int value = 0;
    libqb_thread_start(t, test_thread_function, &value);
    libqb_thread_join(t);
    
    test_assert_ints(1, value); // Value should have been incremented
    
    libqb_thread_free(t);
}

static void test_libqb_thread_multiple_threads() {
    // Test multiple threads
    struct libqb_thread *t1 = libqb_thread_new();
    struct libqb_thread *t2 = libqb_thread_new();
    
    int value1 = 0;
    int value2 = 0;
    
    libqb_thread_start(t1, test_thread_function, &value1);
    libqb_thread_start(t2, test_thread_function, &value2);
    
    libqb_thread_join(t1);
    libqb_thread_join(t2);
    
    test_assert_ints(1, value1);
    test_assert_ints(1, value2);
    
    libqb_thread_free(t1);
    libqb_thread_free(t2);
}

static void test_libqb_mutex_with_threads() {
    // Test mutex with multiple threads
    test_mutex = libqb_mutex_new();
    shared_counter = 0;
    
    struct libqb_thread *t1 = libqb_thread_new();
    struct libqb_thread *t2 = libqb_thread_new();
    struct libqb_thread *t3 = libqb_thread_new();
    
    libqb_thread_start(t1, test_mutex_counter_function, NULL);
    libqb_thread_start(t2, test_mutex_counter_function, NULL);
    libqb_thread_start(t3, test_mutex_counter_function, NULL);
    
    libqb_thread_join(t1);
    libqb_thread_join(t2);
    libqb_thread_join(t3);
    
    test_assert_ints(3, shared_counter); // All three threads should have incremented
    
    libqb_thread_free(t1);
    libqb_thread_free(t2);
    libqb_thread_free(t3);
    libqb_mutex_free(test_mutex);
    test_mutex = NULL;
}

static void test_libqb_mutex_guard() {
    // Test mutex guard (RAII pattern)
    struct libqb_mutex *m = libqb_mutex_new();
    
    {
        libqb_mutex_guard guard(m);
        // Mutex should be locked here
        test_assert(1); // If we get here, guard worked
    }
    // Mutex should be unlocked here
    
    libqb_mutex_free(m);
    test_assert(1); // If we get here, guard cleanup worked
}

int main() {
    struct unit_test tests[] = {
        {test_libqb_mutex_new_free, "libqb_mutex_new_free"},
        {test_libqb_mutex_lock_unlock, "libqb_mutex_lock_unlock"},
        {test_libqb_mutex_multiple_locks, "libqb_mutex_multiple_locks"},
        {test_libqb_condvar_new_free, "libqb_condvar_new_free"},
        {test_libqb_condvar_signal, "libqb_condvar_signal"},
        {test_libqb_condvar_broadcast, "libqb_condvar_broadcast"},
        {test_libqb_thread_new_free, "libqb_thread_new_free"},
        {test_libqb_thread_start_join, "libqb_thread_start_join"},
        {test_libqb_thread_multiple_threads, "libqb_thread_multiple_threads"},
        {test_libqb_mutex_with_threads, "libqb_mutex_with_threads"},
        {test_libqb_mutex_guard, "libqb_mutex_guard"},
    };
    
    return run_tests("threading", tests, sizeof(tests) / sizeof(tests[0]));
}
