//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Global State Accessor Layer
//  Provides controlled access to shared state for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "../../os.h"  // For int32, uint32 type definitions
#include "libqb_state.h"
#include "graphics.h"  // For img_struct

// ============================================================================
// EXTERNAL REFERENCES FROM libqb.cpp
// ============================================================================

// Image system globals
extern img_struct *img;
extern int32 nextimg;
extern img_struct *write_page;
extern img_struct *read_page;
extern img_struct *display_page;

// Page index globals
extern uint32 display_page_index;
extern uint32 write_page_index;
extern uint32 read_page_index;

// Font system globals
extern int32 lastfont;
extern int32 *font;
extern int32 *fontheight;
extern int32 *fontwidth;
extern int32 *fontflags;

// LPRINT state globals
extern int32 lprint;
extern int32 lprint_image;

// Environment 2D globals
extern int32 environment_2d__screen_width;
extern int32 environment_2d__screen_height;
extern int32 environment_2d__screen_x1;
extern int32 environment_2d__screen_y1;
extern int32 environment_2d__screen_scaled_width;
extern int32 environment_2d__screen_scaled_height;

// ============================================================================
// IMAGE SYSTEM ACCESSORS
// ============================================================================

img_struct* libqb_get_write_page() {
    return write_page;
}

img_struct* libqb_get_read_page() {
    return read_page;
}

img_struct* libqb_get_display_page() {
    return display_page;
}

img_struct* libqb_get_image(int32_t handle) {
    if (handle < 0 || handle >= nextimg) {
        return nullptr;
    }
    if (!img[handle].valid) {
        return nullptr;
    }
    return &img[handle];
}

int32_t libqb_get_image_count() {
    return nextimg;
}

int32_t libqb_validate_image(int32_t handle) {
    if (handle < 0 || handle >= nextimg) {
        return 0;
    }
    return img[handle].valid ? 1 : 0;
}

// ============================================================================
// PAGE INDEX ACCESSORS
// ============================================================================

int32_t libqb_get_write_page_index() {
    return static_cast<int32_t>(write_page_index);
}

void libqb_set_write_page_index(int32_t index) {
    write_page_index = static_cast<uint32>(index);
}

int32_t libqb_get_read_page_index() {
    return static_cast<int32_t>(read_page_index);
}

void libqb_set_read_page_index(int32_t index) {
    read_page_index = static_cast<uint32>(index);
}

int32_t libqb_get_display_page_index() {
    return static_cast<int32_t>(display_page_index);
}

void libqb_set_display_page_index(int32_t index) {
    display_page_index = static_cast<uint32>(index);
}

// ============================================================================
// FONT SYSTEM ACCESSORS
// ============================================================================

int32_t libqb_get_font_width(int32_t font_handle) {
    if (font_handle < 0 || font_handle > lastfont) {
        return 0;
    }
    return fontwidth[font_handle];
}

int32_t libqb_get_font_height(int32_t font_handle) {
    if (font_handle < 0 || font_handle > lastfont) {
        return 0;
    }
    return fontheight[font_handle];
}

int32_t libqb_get_font_flags(int32_t font_handle) {
    if (font_handle < 0 || font_handle > lastfont) {
        return 0;
    }
    return fontflags[font_handle];
}

int32_t libqb_get_font(int32_t font_handle) {
    if (font_handle < 0 || font_handle > lastfont) {
        return 0;
    }
    return font[font_handle];
}

int32_t libqb_get_last_font() {
    return lastfont;
}

void libqb_set_last_font(int32_t value) {
    lastfont = value;
}

int32_t* libqb_get_font_array() {
    return font;
}

int32_t* libqb_get_fontwidth_array() {
    return fontwidth;
}

int32_t* libqb_get_fontheight_array() {
    return fontheight;
}

int32_t* libqb_get_fontflags_array() {
    return fontflags;
}

// ============================================================================
// LPRINT STATE ACCESSORS
// ============================================================================

int32_t libqb_get_lprint() {
    return lprint;
}

void libqb_set_lprint(int32_t value) {
    lprint = value;
}

int32_t libqb_get_lprint_image() {
    return lprint_image;
}

void libqb_set_lprint_image(int32_t value) {
    lprint_image = value;
}

// ============================================================================
// ENVIRONMENT 2D ACCESSORS
// ============================================================================

int32_t libqb_get_screen_width() {
    return environment_2d__screen_width;
}

int32_t libqb_get_screen_height() {
    return environment_2d__screen_height;
}

int32_t libqb_get_screen_x1() {
    return environment_2d__screen_x1;
}

int32_t libqb_get_screen_y1() {
    return environment_2d__screen_y1;
}

int32_t libqb_get_screen_scaled_width() {
    return environment_2d__screen_scaled_width;
}

int32_t libqb_get_screen_scaled_height() {
    return environment_2d__screen_scaled_height;
}
