//----------------------------------------------------------------------------------------------------------------------
// QB64-PE Cross-Platform Clipboard Module
// Provides clipboard access for text and image data
// Powered by clip (https://github.com/dacap/clip)
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

struct qbs;

// ============================================================================
// CLIPBOARD TEXT FUNCTIONS
// ============================================================================

/// Gets text content from system clipboard
/// Retrieves current text content from the system clipboard
/// @return QB64 string containing clipboard text, or empty string if no text
qbs *func__clipboard();

/// Sets text content to system clipboard
/// Copies specified text to the system clipboard
/// @param qbsText QB64 string to copy to clipboard
void sub__clipboard(const qbs *qbsText);

// ============================================================================
// CLIPBOARD IMAGE FUNCTIONS
// ============================================================================

/// Gets image from system clipboard
/// Retrieves image data from the system clipboard
/// @return Image handle number, or -1 if no image available
int32_t func__clipboardimage();

/// Sets image to system clipboard
/// Copies specified image to the system clipboard
/// @param src Image handle number to copy to clipboard
void sub__clipboardimage(int32_t src);
