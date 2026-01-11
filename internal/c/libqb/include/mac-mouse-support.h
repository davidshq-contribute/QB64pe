#pragma once

/**
 * @file mac-mouse-support.h
 * @brief macOS-specific mouse support for QB64-PE
 * 
 * This header provides macOS-specific mouse position update functionality.
 * On non-macOS platforms, the function is a no-op.
 */

/**
 * @brief Updates the mouse position (macOS only)
 * @param x X coordinate
 * @param y Y coordinate
 * @note On macOS, updates the mouse position. On other platforms, this is a no-op.
 */
#ifdef QB64_MACOSX
void MacMouse_UpdatePosition(int x, int y);
#else
static inline void MacMouse_UpdatePosition(int x, int y) {
    (void)x;
    (void)y;
}
#endif
