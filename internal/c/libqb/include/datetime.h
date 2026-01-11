#ifndef INCLUDE_LIBQB_DATETIME_H
#define INCLUDE_LIBQB_DATETIME_H

#include "qbs.h"
#include <stdint.h>

/**
 * @file datetime.h
 * @brief Date and time functions for QB64-PE
 * 
 * This header provides functions for getting the current date and time,
 * timing operations, and sleep/delay functionality.
 */

#if defined(QB64_LINUX) || defined(QB64_WINDOWS)
/**
 * @brief Initializes the high-resolution timer
 * @note Should be called at the very beginning of the program to initialize
 *       the clock returned by GetTicks() so that it starts from zero.
 */
void clock_init();
#else
static inline void clock_init() {}
#endif

/**
 * @brief Gets the current tick count in milliseconds
 * @return Number of milliseconds since clock_init() was called
 * @note Returns a high-resolution timer value. Used for timing operations.
 */
int64_t GetTicks();

/**
 * @brief Gets the elapsed time since program start (QB64 TIMER function)
 * @param accuracy Accuracy parameter (if passed > 0)
 * @param passed Flag indicating if accuracy parameter was provided
 * @return Elapsed time in seconds since program start
 * @note Returns a floating-point value representing seconds. If accuracy is provided,
 *       it may affect the precision of the returned value.
 */
double func_timer(double accuracy, int32_t passed);

/**
 * @brief Delays execution for a specified number of seconds (QB64 _DELAY statement)
 * @param seconds Number of seconds to delay
 * @note Sleeps for the specified duration. Fractional seconds are supported.
 */
void sub__delay(double seconds);

/**
 * @brief Limits the frame rate to a specified FPS (QB64 _LIMIT statement)
 * @param fps Target frames per second
 * @note Sleeps as necessary to maintain the target frame rate. Call this once per frame.
 */
void sub__limit(double fps);

// We provide a 'Sleep()' function for non-Windows platforms
#ifndef QB64_WINDOWS
/**
 * @brief Sleeps for a specified number of milliseconds
 * @param milliseconds Number of milliseconds to sleep
 * @note Platform-independent sleep function. On Windows, use the system Sleep() function.
 */
void Sleep(uint32_t milliseconds);
#endif

/**
 * @brief Gets the current time as a string (QB64 TIME$ function)
 * @return qbs string containing the current time in "HH:MM:SS" format, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func_time();

/**
 * @brief Gets the current time and stores it in a string (QB64 TIME statement)
 * @param str qbs string to store the time in (will be modified)
 * @note Stores the current time in "HH:MM:SS" format in the provided string.
 */
void sub_time(qbs *str);

/**
 * @brief Gets the current date as a string (QB64 DATE$ function)
 * @return qbs string containing the current date, or NULL on failure
 * @note Date format is platform-dependent. Caller must free with qbs_free()
 */
qbs *func_date();

/**
 * @brief Gets the current date and stores it in a string (QB64 DATE statement)
 * @param date qbs string to store the date in (will be modified)
 * @note Stores the current date in the provided string. Format is platform-dependent.
 */
void sub_date(qbs *date);

#endif
