#ifndef INCLUDE_LIBQB_MUTEX_H
#define INCLUDE_LIBQB_MUTEX_H

/**
 * @file mutex.h
 * @brief Mutex synchronization primitive for QB64-PE
 * 
 * This header provides mutex (mutual exclusion) functionality for thread synchronization.
 * Mutexes ensure that only one thread can access a shared resource at a time.
 */

/**
 * @struct libqb_mutex
 * @brief Opaque mutex structure
 * 
 * A mutex is a synchronization primitive that provides mutual exclusion.
 * Only one thread can hold a mutex at a time.
 */
struct libqb_mutex;

/**
 * @brief Allocates a new mutex
 * @return Pointer to newly allocated mutex, or NULL on failure
 * @note The mutex is created in an unlocked state
 */
struct libqb_mutex *libqb_mutex_new();

/**
 * @brief Frees a mutex
 * @param mutex Mutex to free
 * @note Do not use the mutex after calling this function. Ensure the mutex is unlocked.
 */
void libqb_mutex_free(struct libqb_mutex *);

/**
 * @brief Locks the mutex
 * @param mutex Mutex to lock
 * @note Blocks until the mutex can be acquired. Only one thread can hold the mutex at a time.
 */
void libqb_mutex_lock(struct libqb_mutex *);

/**
 * @brief Unlocks the mutex
 * @param mutex Mutex to unlock
 * @note Releases the mutex, allowing another thread to acquire it
 */
void libqb_mutex_unlock(struct libqb_mutex *);

/**
 * @class libqb_mutex_guard
 * @brief RAII guard for automatic mutex locking/unlocking
 * 
 * This class automatically locks a mutex when created and unlocks it when destroyed.
 * Useful for ensuring mutexes are always properly unlocked, even if an exception is thrown.
 */
class libqb_mutex_guard {
  public:
    /**
     * @brief Constructs a mutex guard and locks the mutex
     * @param mtx Mutex to guard
     * @note The mutex is locked immediately upon construction
     */
    libqb_mutex_guard(struct libqb_mutex *mtx) : lock(mtx) {
        libqb_mutex_lock(lock);
    }

    /**
     * @brief Destroys the mutex guard and unlocks the mutex
     * @note The mutex is automatically unlocked when the guard goes out of scope
     */
    ~libqb_mutex_guard() {
        libqb_mutex_unlock(lock);
    }

  private:
    struct libqb_mutex *lock; ///< The mutex being guarded
};

#endif
