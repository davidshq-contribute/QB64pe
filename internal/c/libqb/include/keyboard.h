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

// Keyboard Input Functions

/// Returns the next key hit from the keyboard buffer.
/// @returns Key code if a key is available, 0 if buffer is empty
int32_t func__keyhit();

/// Checks if a specific key is currently pressed.
/// @param x The key code to check
/// @returns -1 if key is pressed, 0 otherwise
int32_t func__keydown(int32_t x);

/// Maps a Unicode code point to an ASCII code in the codepage table.
/// @param unicode_code Unicode code point (0-65535)
/// @param ascii_code ASCII code to map to (0-255)
void sub__mapunicode(int32_t unicode_code, int32_t ascii_code);

/// Gets the Unicode code point for an ASCII code.
/// @param ascii_code ASCII code (0-255)
/// @returns Unicode code point mapped to the ASCII code
int32_t func__mapunicode(int32_t ascii_code);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides keyboard functionality including:
// - Querying the state of Caps Lock, Scroll Lock, and Num Lock keys
// - Controlling the state of lock keys programmatically
// - Toggle functionality for lock keys
// - Keyboard input via _KEYHIT and _KEYDOWN
// - Unicode/codepage mapping via _MAPUNICODE
//
// Note: Lock key control may not work on all platforms due to OS security
// restrictions. State queries should work reliably on all platforms.

#endif // INCLUDE_LIBQB_KEYBOARD_H
