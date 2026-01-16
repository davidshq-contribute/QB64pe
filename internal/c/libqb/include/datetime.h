//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Date & Time Module
//  Provides timing, delay, and date/time functionality
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_DATETIME_H
#define INCLUDE_LIBQB_DATETIME_H

#include "qbs.h"
#include <stdint.h>

// ============================================================================
// PLATFORM-SPECIFIC CLOCK INITIALIZATION
// ============================================================================

#if defined(QB64_LINUX) || defined(QB64_WINDOWS)
/// Initializes the clock returned by 'GetTicks()' so that it starts from zero
/// Should be called at the very beginning of the program to normalize timing
void clock_init();
#else
/// Inline no-op for platforms that don't need clock initialization
static inline void clock_init() {}
#endif

// ============================================================================
// HIGH-PRECISION TIMING FUNCTIONS
// ============================================================================

/// Gets the current system tick count with high precision
/// Provides platform-independent high-resolution timing
/// @return Current tick count (platform-specific units, typically milliseconds)
int64_t GetTicks();

/// Returns the current timer value with specified accuracy
/// Measures elapsed time since program start or last timer reset
/// @param accuracy Timer accuracy in seconds (e.g., 0.001 for millisecond precision)
/// @param passed Parameter passing flags (for QB64 compatibility)
/// @return Current timer value in the specified accuracy units
double func_timer(double accuracy, int32_t passed);

/// Pauses program execution for specified duration
/// Provides cross-platform sleep/delay functionality
/// @param seconds Number of seconds to delay (fractional values supported)
void sub__delay(double seconds);

/// Limits program execution to specified frames per second
/// Automatically delays to maintain consistent frame rate
/// @param fps Target frames per second (e.g., 60.0 for 60 FPS)
void sub__limit(double fps);

// ============================================================================
// PLATFORM-SPECIFIC SLEEP FUNCTION
// ============================================================================

/// Provides Sleep() function for non-Windows platforms
/// On Windows, uses the native Sleep() function
/// @param milliseconds Number of milliseconds to sleep
#ifndef QB64_WINDOWS
void Sleep(uint32_t milliseconds);
#endif

// ============================================================================
// SYSTEM DATE/TIME FUNCTIONS
// ============================================================================

/// Gets the current system time as a string
/// Returns time in HH:MM:SS format (24-hour clock)
/// @return QB64 string containing current time
qbs *func_time();

/// Sets the system time (if platform permits)
/// Updates system clock with specified time string
/// @param str Time string in HH:MM:SS format
void sub_time(qbs *str);

/// Gets the current system date as a string
/// Returns date in platform-specific format (usually MM-DD-YYYY)
/// @return QB64 string containing current date
qbs *func_date();

/// Sets the system date (if platform permits)
/// Updates system calendar with specified date string
/// @param date Date string in platform-specific format
void sub_date(qbs *date);

#endif
