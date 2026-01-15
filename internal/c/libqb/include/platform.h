//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Platform Module
//  Platform-specific functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_PLATFORM_H
#define INCLUDE_LIBQB_PLATFORM_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

struct qbs;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

/// Simulates keyboard input to send text to the active window.
/// Uses platform-specific APIs (SendInput on Windows, CGEvent on macOS).
/// @param txt The text string to send as keyboard input
void sub__screenprint(qbs *txt);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides platform-specific functionality including:
// - Keyboard input simulation via _SCREENPRINT
//
// Platform support:
// - Windows: Uses SendInput API
// - macOS: Uses CGEvent API with virtual key codes
// - Linux: Not currently implemented

#endif // INCLUDE_LIBQB_PLATFORM_H
