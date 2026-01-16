//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File Helpers Module
//  File I/O helper functions for reading, writing, and OS information
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_FILE_HELPERS_H
#define INCLUDE_LIBQB_FILE_HELPERS_H

#include "../../os.h"  // For int32, int64, uint32, uint64 type definitions

// Forward declarations
struct qbs;

// ============================================================================
// WINDOW TITLE MANAGEMENT
// ============================================================================

/// Sets the window title.
/// Updates the application window title with the specified text.
/// @param title Title string to set
void sub__title(qbs *title);

// ============================================================================
// CONSOLE OUTPUT
// ============================================================================

/// Echoes a message to the console.
/// Outputs the message to the console destination, then restores the previous destination.
/// @param message Message string to echo
void sub__echo(qbs *message);

// ============================================================================
// FILE OPERATIONS
// ============================================================================

/// Reads the entire contents of a file into a string.
/// Opens the file in binary mode and reads all data.
/// @param filespec File path to read
/// @return QB64 string containing file contents, or empty string on error
qbs *func__readfile(qbs *filespec);

/// Writes a string to a file.
/// Opens the file in binary write mode and writes all data.
/// @param filespec File path to write to
/// @param contents String data to write
void sub__writefile(qbs *filespec, qbs *contents);

// ============================================================================
// SYSTEM INFORMATION
// ============================================================================

/// Gets the operating system information string.
/// Returns a string identifying the OS, architecture, and bitness.
/// Format: [OS][ARCH][BITS] (e.g., "[WINDOWS][64BIT]")
/// @return QB64 string containing OS information
qbs *func__os();

#endif // INCLUDE_LIBQB_FILE_HELPERS_H
