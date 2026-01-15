//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Mouse Module
//  Mouse input functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_MOUSE_H
#define INCLUDE_LIBQB_MOUSE_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct qbs;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Mouse Visibility Control

/// Hides the mouse cursor.
void sub__mousehide();

/// Shows the mouse cursor with optional style.
/// @param style Cursor style string (optional, e.g., "arrow", "hand", "crosshair")
/// @param passed Parameter passing flags
void sub__mouseshow(qbs *style, int32_t passed);

/// Checks if the mouse cursor is currently hidden.
/// @returns Non-zero if mouse is hidden, zero if visible
int32_t func__mousehidden();

// Mouse Position Functions

/// Gets the current mouse X coordinate.
/// @returns Mouse X position in pixels
float func__mousex();

/// Gets the current mouse Y coordinate.
/// @returns Mouse Y position in pixels
float func__mousey();

/// Gets the mouse X movement since last check.
/// @returns X movement delta in pixels
float func__mousemovementx();

/// Gets the mouse Y movement since last check.
/// @returns Y movement delta in pixels
float func__mousemovementy();

/// Moves the mouse cursor to a specific position.
/// @param x Target X coordinate
/// @param y Target Y coordinate
void sub__mousemove(float x, float y);

// Mouse Input Functions

/// Gets the current mouse input state (button presses, movement, etc.).
/// @returns Mouse input state flags
int32_t func__mouseinput();

/// Checks the state of a specific mouse button.
/// @param i Button number (1=left, 2=right, 3=middle, etc.)
/// @returns Non-zero if button is pressed, zero otherwise
int32_t func__mousebutton(int32_t i);

/// Gets the mouse wheel movement.
/// @returns Wheel movement value (positive=up/forward, negative=down/backward, 0=no movement)
int32_t func__mousewheel();

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides comprehensive mouse input functionality including:
// - Mouse cursor visibility control
// - Mouse position tracking and movement
// - Mouse button state detection
// - Mouse wheel input
// - Relative movement tracking
//
// Coordinates are relative to the current graphics window/viewport.

#endif // INCLUDE_LIBQB_MOUSE_H
