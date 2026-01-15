//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Console Module
//  Console window management and input functions
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_CONSOLE_H
#define INCLUDE_LIBQB_CONSOLE_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// Forward declarations
struct qbs;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Console Window Management

/// Gets the current console window state.
/// @returns 1 if console is visible, 0 if hidden, -1 if no console available
int32_t func__console();

/// Shows or hides the console window.
/// @param onoff 1 to show console, 0 to hide console
void sub__console(int32_t onoff);

/// Sets the console window title.
/// @param s Title string to set
void sub__consoletitle(qbs *s);

/// Changes the console font.
/// @param FontName Name of the font to use
/// @param FontSize Font size in points
void sub__consolefont(qbs *FontName, int32_t FontSize);

/// Controls the console cursor appearance.
/// @param visible 0 to hide cursor, non-zero to show
/// @param cursorsize Cursor size (1-100, percentage of cell height)
/// @param passed Parameter passing flags
void sub__console_cursor(int32_t visible, int32_t cursorsize, int32_t passed);

// Console Input

/// Checks for and retrieves console input events.
/// Call this in a loop to process keyboard events from the console window.
/// @returns Non-zero if an input event was retrieved, 0 if no events pending
int32_t func__getconsoleinput();

/// Gets a character from console input.
/// @param toggle Input mode toggle
/// @param passed Parameter passing flags
/// @returns Character code from console input
int32_t func__cinp(int32_t toggle, int32_t passed);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides console window functionality for QB64 programs:
// - Show/hide the console window at runtime
// - Customize console appearance (title, font, cursor)
// - Read keyboard input from the console window
//
// Console functionality is primarily available on Windows. On other platforms,
// some functions may have limited or no functionality.
//
// The console window is separate from the graphics window created by QB64.
// Programs compiled with $CONSOLE can use both windows simultaneously.

#endif // INCLUDE_LIBQB_CONSOLE_H
