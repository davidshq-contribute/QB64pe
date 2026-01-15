//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Utility Module
//  General utility functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_UTILITY_H
#define INCLUDE_LIBQB_UTILITY_H

#include <stdint.h>

// Blink control functions
// _BLINK returns the current blink state (-1 for on, 0 for off)
int32_t func__blink();
// _BLINK ON/OFF controls whether high intensity background colors blink
void sub__blink(int32_t onoff);

// Random number generation
// RANDOMIZE [USING] seed - seeds the random number generator
void sub_randomize(double seed, int32_t passed);
// RND(n) - returns a pseudo-random number
// n < 0: seed with n and return first value
// n = 0: return last value
// n > 0 or not passed: return next value
float func_rnd(float n, int32_t passed);

// Frame rate control
// _FPS fps or _FPS _AUTO
// passed=1: _AUTO mode
// passed=2: use fps value
void sub__fps(double fps, int32_t passed);

// Access functions for FPS state (used by display subsystem)
double get_max_fps();
int32_t get_auto_fps();
void set_auto_fps(int32_t value);

// Reset RND state (used by RUN command)
void reset_rnd_state();

// Control character mode
// _CONTROLCHR OFF disables interpretation of control characters in PRINT
// _CONTROLCHR ON (default) enables interpretation
void sub__controlchr(int32_t onoff);
int32_t func__controlchr();

// Accessor for control character state (used by qbs_print)
int32_t get_control_characters_disabled();

#endif // INCLUDE_LIBQB_UTILITY_H
