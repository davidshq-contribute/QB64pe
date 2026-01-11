//----------------------------------------------------------------------------------------------------------------------
// QB64-PE cross-platform clipboard support
// Powered by clip (https://github.com/dacap/clip)
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

/**
 * @file clipboard.h
 * @brief Clipboard operations for QB64-PE
 * 
 * This header provides functions for reading from and writing to the system clipboard,
 * including support for both text and image data.
 */

struct qbs;

/**
 * @brief Gets the current clipboard text (QB64 _CLIPBOARD$ function)
 * @return qbs string containing the clipboard text, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__clipboard();

/**
 * @brief Sets the clipboard text (QB64 _CLIPBOARD statement)
 * @param qbsText Text to copy to the clipboard
 * @note Copies the provided text to the system clipboard
 */
void sub__clipboard(const qbs *qbsText);

/**
 * @brief Gets an image handle from the clipboard (QB64 _CLIPBOARDIMAGE function)
 * @return Image handle if clipboard contains an image, 0 otherwise
 * @note Returns a handle that can be used with image functions. Returns 0 if no image is available.
 */
int32_t func__clipboardimage();

/**
 * @brief Copies an image to the clipboard (QB64 _CLIPBOARDIMAGE statement)
 * @param src Image handle to copy to the clipboard
 * @note Copies the specified image to the system clipboard
 */
void sub__clipboardimage(int32_t src);
