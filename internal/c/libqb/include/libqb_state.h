//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Global State Accessor Layer
//  Provides controlled access to shared state for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_STATE_H
#define INCLUDE_LIBQB_STATE_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// Forward declarations
struct img_struct;

// ============================================================================
// IMAGE SYSTEM ACCESSORS
// ============================================================================

// Get the current write page (where drawing operations go)
img_struct* libqb_get_write_page();

// Get the current read page (source for POINT, image copying, etc.)
img_struct* libqb_get_read_page();

// Get the current display page (what's shown on screen)
img_struct* libqb_get_display_page();

// Get an image by handle (returns NULL if invalid)
img_struct* libqb_get_image(int32_t handle);

// Get the number of allocated images (nextimg value)
int32_t libqb_get_image_count();

// Validate an image handle (returns 1 if valid, 0 if invalid)
int32_t libqb_validate_image(int32_t handle);

// ============================================================================
// PAGE INDEX ACCESSORS
// ============================================================================

// Write page index (current drawing target)
int32_t libqb_get_write_page_index();
void libqb_set_write_page_index(int32_t index);

// Read page index (current source for reads)
int32_t libqb_get_read_page_index();
void libqb_set_read_page_index(int32_t index);

// Display page index (currently displayed)
int32_t libqb_get_display_page_index();
void libqb_set_display_page_index(int32_t index);

// ============================================================================
// FONT SYSTEM ACCESSORS
// ============================================================================

// Get font metrics by handle
int32_t libqb_get_font_width(int32_t font_handle);
int32_t libqb_get_font_height(int32_t font_handle);
int32_t libqb_get_font_flags(int32_t font_handle);
int32_t libqb_get_font(int32_t font_handle);

// Get highest used font index
int32_t libqb_get_last_font();

// ============================================================================
// ENVIRONMENT 2D ACCESSORS
// ============================================================================

// Screen dimensions (logical)
int32_t libqb_get_screen_width();
int32_t libqb_get_screen_height();

// Screen position
int32_t libqb_get_screen_x1();
int32_t libqb_get_screen_y1();

// Scaled screen dimensions (physical)
int32_t libqb_get_screen_scaled_width();
int32_t libqb_get_screen_scaled_height();

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides accessor functions for global state that is shared
// across multiple modules. It enables further modularization by decoupling
// modules from direct access to global arrays and variables defined in
// libqb.cpp.
//
// Key globals accessed:
// - Image system: img[], write_page, read_page, display_page, nextimg
// - Font system: font[], fontwidth[], fontheight[], fontflags[], lastfont
// - Page indexes: write_page_index, read_page_index, display_page_index
// - Environment: environment_2d__screen_* variables
//
// Usage: Include this header and call accessor functions instead of using
// extern declarations for global variables.

#endif // INCLUDE_LIBQB_STATE_H
