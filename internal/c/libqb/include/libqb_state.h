//----------------------------------------------------------------------------------------------------------------------
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
void libqb_set_last_font(int32_t value);

// Direct array access (for font allocation/modification)
int32_t* libqb_get_font_array();
int32_t* libqb_get_fontwidth_array();
int32_t* libqb_get_fontheight_array();
int32_t* libqb_get_fontflags_array();

// ============================================================================
// LPRINT STATE ACCESSORS
// ============================================================================

int32_t libqb_get_lprint();
void libqb_set_lprint(int32_t value);
int32_t libqb_get_lprint_image();
void libqb_set_lprint_image(int32_t value);

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
// DISPLAY CONTROL ACCESSORS
// ============================================================================

// Screen visibility
int32_t libqb_get_screen_hide();
void libqb_set_screen_hide(int32_t value);

// Auto-display mode (-1=toggle request, 0=off, 1=on)
int32_t libqb_get_autodisplay();
void libqb_set_autodisplay(int32_t value);

// Force display update
void libqb_display();

// ============================================================================
// FULLSCREEN STATE ACCESSORS
// ============================================================================

// Current fullscreen mode (0=off, 1=stretch, 2=squarepixels)
int32_t libqb_get_full_screen();
void libqb_set_full_screen(int32_t value);

// Pending fullscreen mode change (-1=no change pending)
int32_t libqb_get_full_screen_set();
void libqb_set_full_screen_set(int32_t value);

// Fullscreen smooth scaling
int32_t libqb_get_fullscreen_smooth();
void libqb_set_fullscreen_smooth(int32_t value);

// Allowed fullscreen modes
int32_t libqb_get_fullscreen_allowedmode();
void libqb_set_fullscreen_allowedmode(int32_t value);

int32_t libqb_get_fullscreen_allowedsmooth();
void libqb_set_fullscreen_allowedsmooth(int32_t value);

// Force display update flag
int32_t libqb_get_force_display_update();
void libqb_set_force_display_update(int32_t value);

// ============================================================================
// RESIZE STATE ACCESSORS
// ============================================================================

// Resize snapback (1=enabled, 0=disabled)
int32_t libqb_get_resize_snapback();
void libqb_set_resize_snapback(int32_t value);

// Resize auto mode
int32_t libqb_get_resize_auto();
void libqb_set_resize_auto(int32_t value);

// Resize event flag and dimensions
int32_t libqb_get_resize_event();
void libqb_set_resize_event(int32_t value);

int32_t libqb_get_resize_event_x();
void libqb_set_resize_event_x(int32_t value);

int32_t libqb_get_resize_event_y();
void libqb_set_resize_event_y(int32_t value);

// ============================================================================
// FILE DROP STATE ACCESSORS
// ============================================================================

// Accept file drop flag (-1=enabled, 0=disabled)
int32_t libqb_get_accept_filedrop();
void libqb_set_accept_filedrop(int32_t value);

// Total dropped files count
int32_t libqb_get_total_dropped_files();
void libqb_set_total_dropped_files(int32_t value);

// HDROP handle (Windows file drop handle) - stored as void*
void* libqb_get_hdrop();
void libqb_set_hdrop(void* value);

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
