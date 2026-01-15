//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Window Module
//  Window query and control functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_WINDOW_H
#define INCLUDE_LIBQB_WINDOW_H

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

/// Gets the native window handle for the QB64 window.
/// On Windows, this returns the HWND (window handle).
/// On other platforms, this typically returns 0.
/// @returns Platform-specific window handle (HWND on Windows, 0 on other platforms)
int64_t func__handle();

/// Gets the current window title text.
/// @returns QB64 string containing the window title
qbs *func__title();

/// Checks if the QB64 window currently has input focus.
/// @returns -1 (true) if window has focus, 0 (false) otherwise
int32_t func__hasfocus();

/// Gets the width of the screen/desktop in pixels.
/// @returns Screen width in pixels, or 0 if not available
int32_t func_screenwidth();

/// Gets the height of the screen/desktop in pixels.
/// @returns Screen height in pixels, or 0 if not available
int32_t func_screenheight();

/// Iconifies (minimizes) the window.
void sub_screenicon();

/// Checks if the QB64 window exists.
/// @returns -1 (true) if window exists, 0 (false) otherwise
int32_t func_windowexists();

/// Checks if the window is currently iconified (minimized).
/// @returns -1 (true) if iconified, 0 (false) otherwise
int32_t func_screenicon();

/// Moves the window to a specified position on the desktop.
/// @param x X coordinate (or special value for centering)
/// @param y Y coordinate
/// @param passed Bit flags: bit 0 = x passed, bit 1 = y passed (3 = _MIDDLE)
void sub__screenmove(int32_t x, int32_t y, int32_t passed);

/// Enables or disables file drop acceptance.
/// @param on_off 1 or 0 to enable, 2 to disable
void sub__filedrop(int32_t on_off);

/// Gets the current file drop acceptance state.
/// @returns -1 if file drop is enabled, 0 if disabled
int32_t func__filedrop();

/// Finishes processing dropped files and clears the drop list.
void sub__finishdrop();

/// Gets the total number of dropped files.
/// @returns Number of dropped files
int32_t func__totaldroppedfiles();

/// Gets a dropped file by index.
/// @param fileIndex 1-based file index (or sequential access if not passed)
/// @param passed 1 if fileIndex was provided, 0 for sequential access
/// @returns QB64 string containing the file path, or empty string on error
qbs *func__droppedfile(int32_t fileIndex, int32_t passed);

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

/// Sets the foreground window (Windows platform only).
/// This is an internal helper function for Windows-specific window management.
/// @param handle Window handle to bring to foreground
void set_foreground_window(intptr_t handle);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides window management functionality including:
// - Native window handle access for platform-specific operations
// - Window title retrieval
// - Window focus state detection
// - Platform-specific window control (Windows foreground window)
//
// Note: Some functions are platform-specific. Window handle access is primarily
// useful on Windows for integration with native Windows APIs.

#endif // INCLUDE_LIBQB_WINDOW_H
