//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE GUI Library
//  GUI dialog and window functions
//  Powered by tinyfiledialogs (http://tinyfiledialogs.sourceforge.net)
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#ifndef INCLUDE_LIBQB_GUI_H
#define INCLUDE_LIBQB_GUI_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdarg.h>
#include <stdint.h>

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct qbs;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

/// Displays a notification popup window.
/// @param qbsTitle Popup title text
/// @param qbsMessage Message text to display
/// @param qbsIconType Icon type ("info", "warning", "error")
/// @param passed Parameter passing flags
void sub__guiNotifyPopup(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsIconType, int32_t passed);

/// Displays a message box dialog and returns the button pressed.
/// @param qbsTitle Dialog title text
/// @param qbsMessage Message text to display
/// @param qbsDialogType Dialog type ("ok", "okcancel", "yesno", "yesnocancel")
/// @param qbsIconType Icon type ("info", "warning", "error", "question")
/// @param nDefaultButton Default button index (0-based)
/// @param passed Parameter passing flags
/// @returns Button index (0-based) indicating which button was pressed
int32_t func__guiMessageBox(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsDialogType, qbs *qbsIconType, int32_t nDefaultButton, int32_t passed);

/// Displays a simple message box dialog (overload with fewer parameters).
/// @param qbsTitle Dialog title text
/// @param qbsMessage Message text to display
/// @param qbsIconType Icon type ("info", "warning", "error")
/// @param passed Parameter passing flags
void sub__guiMessageBox(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsIconType, int32_t passed);

/// Displays an input dialog box.
/// @param qbsTitle Dialog title text
/// @param qbsMessage Prompt message text
/// @param qbsDefaultInput Default input value
/// @param passed Parameter passing flags
/// @returns User input string, or NULL if cancelled
qbs *func__guiInputBox(qbs *qbsTitle, qbs *qbsMessage, qbs *qbsDefaultInput, int32_t passed);

/// Displays a folder selection dialog.
/// @param qbsTitle Dialog title text
/// @param qbsDefaultPath Default folder path
/// @param passed Parameter passing flags
/// @returns Selected folder path, or NULL if cancelled
qbs *func__guiSelectFolderDialog(qbs *qbsTitle, qbs *qbsDefaultPath, int32_t passed);

/// Displays a color chooser dialog.
/// @param qbsTitle Dialog title text
/// @param nDefaultRGB Default RGB color value
/// @param passed Parameter passing flags
/// @returns Selected RGB color value, or 0 if cancelled
uint32_t func__guiColorChooserDialog(qbs *qbsTitle, uint32_t nDefaultRGB, int32_t passed);

/// Displays a file open dialog.
/// @param qbsTitle Dialog title text
/// @param qbsDefaultPathAndFile Default file path
/// @param qbsFilterPatterns File filter patterns (e.g., "*.txt;*.doc")
/// @param qbsSingleFilterDescription Description for the filter
/// @param nAllowMultipleSelects Non-zero to allow multiple file selection
/// @param passed Parameter passing flags
/// @returns Selected file path(s), or NULL if cancelled
qbs *func__guiOpenFileDialog(qbs *qbsTitle, qbs *qbsDefaultPathAndFile, qbs *qbsFilterPatterns, qbs *qbsSingleFilterDescription, int32_t nAllowMultipleSelects,
                             int32_t passed);

/// Displays a file save dialog.
/// @param qbsTitle Dialog title text
/// @param qbsDefaultPathAndFile Default file path
/// @param qbsFilterPatterns File filter patterns (e.g., "*.txt;*.doc")
/// @param qbsSingleFilterDescription Description for the filter
/// @param passed Parameter passing flags
/// @returns Selected file path, or NULL if cancelled
qbs *func__guiSaveFileDialog(qbs *qbsTitle, qbs *qbsDefaultPathAndFile, qbs *qbsFilterPatterns, qbs *qbsSingleFilterDescription, int32_t passed);

// ============================================================================
// INTERNAL HELPERS
// ============================================================================

/// Internal helper function to display alert dialogs (C string version).
/// @param message Alert message text
/// @param title Dialog title
/// @param type Alert type
/// @returns Non-zero on success
int gui_alert(const char *message, const char *title, const char *type);

/// Internal helper function to display alert dialogs (printf-style format).
/// @param fmt Format string (printf-style)
/// @param ... Format arguments
/// @returns True on success
bool gui_alert(const char *fmt, ...);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides cross-platform GUI dialog functionality including:
// - Message boxes and notifications
// - File and folder selection dialogs
// - Color chooser dialogs
// - Input dialogs
//
// Powered by tinyfiledialogs library for native OS dialogs on Windows, Linux, and macOS.

#endif // INCLUDE_LIBQB_GUI_H
