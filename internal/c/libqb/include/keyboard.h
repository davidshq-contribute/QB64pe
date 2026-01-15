//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Keyboard Module
//  Lock key functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_KEYBOARD_H
#define INCLUDE_LIBQB_KEYBOARD_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Lock Key State Queries

/// Checks if Caps Lock is currently enabled.
/// @returns Non-zero if Caps Lock is on, zero if off
int32_t func__capslock();

/// Checks if Scroll Lock is currently enabled.
/// @returns Non-zero if Scroll Lock is on, zero if off
int32_t func__scrolllock();

/// Checks if Num Lock is currently enabled.
/// @returns Non-zero if Num Lock is on, zero if off
int32_t func__numlock();

// Lock Key Control

/// Controls the Caps Lock key state.
/// @param options Control option: 1=ON, 2=OFF, 3=TOGGLE
void sub__capslock(int32_t options);

/// Controls the Scroll Lock key state.
/// @param options Control option: 1=ON, 2=OFF, 3=TOGGLE
void sub__scrolllock(int32_t options);

/// Controls the Num Lock key state.
/// @param options Control option: 1=ON, 2=OFF, 3=TOGGLE
void sub__numlock(int32_t options);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides keyboard lock key functionality including:
// - Querying the state of Caps Lock, Scroll Lock, and Num Lock keys
// - Controlling the state of lock keys programmatically
// - Toggle functionality for lock keys
//
// Note: Lock key control may not work on all platforms due to OS security
// restrictions. State queries should work reliably on all platforms.

#endif // INCLUDE_LIBQB_KEYBOARD_H
