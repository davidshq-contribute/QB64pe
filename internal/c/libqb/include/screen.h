//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Screen Management Module
//  Display control, fullscreen management, and resize handling
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_SCREEN_H
#define INCLUDE_LIBQB_SCREEN_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Display Control

/// Manually updates the display with current screen content.
/// Use this after disabling auto-display for controlled rendering.
void sub__display();

/// Enables automatic display updates after each graphics operation.
/// This is the default mode - display updates automatically.
void sub__autodisplay();

// Fullscreen Control

/// Enters or exits fullscreen mode.
/// @param method Fullscreen method: 1=_STRETCH, 2=_SQUAREPIXELS, 3=_OFF
/// @param passed Parameter passing flags
void sub__fullscreen(int32_t method, int32_t passed);

/// Gets the current fullscreen mode.
/// @returns 0=windowed, 1=_STRETCH, 2=_SQUAREPIXELS
int32_t func__fullscreen();

/// Checks if smooth scaling is enabled in fullscreen mode.
/// @returns Non-zero if smooth scaling is enabled
int32_t func__fullscreensmooth();

/// Configures which fullscreen modes are allowed.
/// @param method Allowed fullscreen method (0=none, 1=_STRETCH, 2=_SQUAREPIXELS, 3=all)
/// @param smooth Whether smooth scaling is allowed (0=off, 1=on, 2=either)
void sub__allowfullscreen(int32_t method, int32_t smooth);

// Resize Handling

/// Enables or disables window resizing.
/// @param on_off 1=enable resize, 0=disable resize
/// @param stretch_smooth Stretch/smooth mode for resize: 0=_STRETCH, 1=_SMOOTH
void sub__resize(int32_t on_off, int32_t stretch_smooth);

/// Checks if window has been resized.
/// Call this in your main loop to detect user window resizes.
/// @returns Non-zero if window was resized since last check, 0 otherwise
int32_t func__resize();

/// Gets the new width after a resize event.
/// @returns New window width in pixels
int32_t func__resizewidth();

/// Gets the new height after a resize event.
/// @returns New window height in pixels
int32_t func__resizeheight();

// Scaled Dimensions

/// Gets the scaled width of the screen (physical pixels).
/// @returns Scaled width in pixels
int32_t func__scaledwidth();

/// Gets the scaled height of the screen (physical pixels).
/// @returns Scaled height in pixels
int32_t func__scaledheight();

// Screen Position

/// Gets the X position of the graphics window on the desktop.
/// @returns Window X position in screen coordinates
int32_t func__screenx();

/// Gets the Y position of the graphics window on the desktop.
/// @returns Window Y position in screen coordinates
int32_t func__screeny();

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides screen management functionality including:
// - Display control (_DISPLAY, _AUTODISPLAY)
// - Fullscreen mode management with smooth scaling options
// - Window resize handling and dimension queries
// - Screen position and scaled dimension utilities
//
// Display Modes:
// - Auto-display (default): Screen updates automatically after drawing operations
// - Manual display: Use _DISPLAY to update, allows smoother animation
//
// Fullscreen Methods:
// - _STRETCH: Stretches screen to fill monitor (may distort aspect ratio)
// - _SQUAREPIXELS: Maintains aspect ratio with black bars if needed
// - _OFF: Returns to windowed mode
//
// Resize Handling:
// - Enable resize with _RESIZE ON
// - Poll func__resize() in main loop to detect user resize
// - Use func__resizewidth/height to get new dimensions
// - Recreate screen with new dimensions as needed

#endif // INCLUDE_LIBQB_SCREEN_H
