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
