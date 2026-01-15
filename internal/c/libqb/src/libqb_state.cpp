//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Global State Accessor Layer
//  Provides controlled access to shared state for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "libqb_state.h"
#include "graphics.h"  // For img_struct

#include "../../os.h"  // For int32, uint32 type definitions

#include <cstring>  // For memmove

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

// Display control globals
extern int32 screen_hide;
extern int32 autodisplay;
extern void display();

// Fullscreen state globals
extern int32 full_screen;
extern int32 full_screen_set;
extern int32 fullscreen_smooth;
extern int32 fullscreen_allowedmode;
extern int32 fullscreen_allowedsmooth;
extern int32 force_display_update;

// Resize state globals
extern int32 resize_snapback;
extern int32 resize_auto;
extern int32 resize_event;
extern int32 resize_event_x;
extern int32 resize_event_y;

// File drop state globals
extern int32 acceptFileDrop;
extern int32 totalDroppedFiles;
#ifdef QB64_WINDOWS
#include <windows.h>
extern HDROP hdrop;
#else
static void* hdrop_stub = nullptr;
#endif

// Keyhit buffer globals
extern int64 keyhit[8192];
extern int32 keyhit_next;
extern int32 keyhit_nextfree;

// Keyheld function (wrapper for internal keyheld() function)
extern int32 keyheld(uint32 keycode);

// Port 60h buffer globals
extern uint8 port60h_event[256];
extern int32 port60h_events;

// Mouse message queue struct (defined in libqb.cpp)
struct mouse_message {
    int32 x;
    int32 y;
    int32 movementX;
    int32 movementY;
    int32 buttons;
    int32 wheel;
};

struct mouse_message_queue_struct {
    mouse_message *queue;
    int32 lastIndex;
    int32 current;
    int32 last;
};

extern mouse_message_queue_struct mouse_message_queue;

// Mouse state globals
extern int32 mouse_hiddden;  // Note: typo in original code (3 d's)
extern int mouse_cursor_style;

// Codepage mapping array
extern uint16 codepage437_to_unicode16[256];

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

// ============================================================================
// DISPLAY CONTROL ACCESSORS
// ============================================================================

int32_t libqb_get_screen_hide() {
    return screen_hide;
}

void libqb_set_screen_hide(int32_t value) {
    screen_hide = value;
}

int32_t libqb_get_autodisplay() {
    return autodisplay;
}

void libqb_set_autodisplay(int32_t value) {
    autodisplay = value;
}

void libqb_display() {
    display();
}

// ============================================================================
// FULLSCREEN STATE ACCESSORS
// ============================================================================

int32_t libqb_get_full_screen() {
    return full_screen;
}

void libqb_set_full_screen(int32_t value) {
    full_screen = value;
}

int32_t libqb_get_full_screen_set() {
    return full_screen_set;
}

void libqb_set_full_screen_set(int32_t value) {
    full_screen_set = value;
}

int32_t libqb_get_fullscreen_smooth() {
    return fullscreen_smooth;
}

void libqb_set_fullscreen_smooth(int32_t value) {
    fullscreen_smooth = value;
}

int32_t libqb_get_fullscreen_allowedmode() {
    return fullscreen_allowedmode;
}

void libqb_set_fullscreen_allowedmode(int32_t value) {
    fullscreen_allowedmode = value;
}

int32_t libqb_get_fullscreen_allowedsmooth() {
    return fullscreen_allowedsmooth;
}

void libqb_set_fullscreen_allowedsmooth(int32_t value) {
    fullscreen_allowedsmooth = value;
}

int32_t libqb_get_force_display_update() {
    return force_display_update;
}

void libqb_set_force_display_update(int32_t value) {
    force_display_update = value;
}

// ============================================================================
// RESIZE STATE ACCESSORS
// ============================================================================

int32_t libqb_get_resize_snapback() {
    return resize_snapback;
}

void libqb_set_resize_snapback(int32_t value) {
    resize_snapback = value;
}

int32_t libqb_get_resize_auto() {
    return resize_auto;
}

void libqb_set_resize_auto(int32_t value) {
    resize_auto = value;
}

int32_t libqb_get_resize_event() {
    return resize_event;
}

void libqb_set_resize_event(int32_t value) {
    resize_event = value;
}

int32_t libqb_get_resize_event_x() {
    return resize_event_x;
}

void libqb_set_resize_event_x(int32_t value) {
    resize_event_x = value;
}

int32_t libqb_get_resize_event_y() {
    return resize_event_y;
}

void libqb_set_resize_event_y(int32_t value) {
    resize_event_y = value;
}

// ============================================================================
// FILE DROP STATE ACCESSORS
// ============================================================================

int32_t libqb_get_accept_filedrop() {
    return acceptFileDrop;
}

void libqb_set_accept_filedrop(int32_t value) {
    acceptFileDrop = value;
}

int32_t libqb_get_total_dropped_files() {
    return totalDroppedFiles;
}

void libqb_set_total_dropped_files(int32_t value) {
    totalDroppedFiles = value;
}

void* libqb_get_hdrop() {
#ifdef QB64_WINDOWS
    return static_cast<void*>(hdrop);
#else
    return hdrop_stub;
#endif
}

void libqb_set_hdrop(void* value) {
#ifdef QB64_WINDOWS
    hdrop = static_cast<HDROP>(value);
#else
    hdrop_stub = value;
#endif
}

// ============================================================================
// KEYHIT BUFFER ACCESSORS
// ============================================================================

int64_t libqb_keyhit_pop() {
    if (keyhit_next == keyhit_nextfree) {
        return 0;
    }
    int64_t value = keyhit[keyhit_next & 0x1FFF];
    keyhit_next++;
    return value;
}

int32_t libqb_keyhit_pending() {
    return keyhit_nextfree - keyhit_next;
}

void libqb_keyhit_push(int64_t value) {
    keyhit[keyhit_nextfree & 0x1FFF] = value;
    keyhit_nextfree++;
}

void libqb_keyhit_clear() {
    keyhit_next = keyhit_nextfree;
}

// ============================================================================
// KEYHELD STATE ACCESSORS
// ============================================================================

int32_t libqb_keyheld(uint32_t keycode) {
    return keyheld(static_cast<uint32>(keycode));
}

// ============================================================================
// PORT 60H BUFFER ACCESSORS
// ============================================================================

int32_t libqb_port60h_events_count() {
    return port60h_events;
}

uint8_t libqb_port60h_peek() {
    // Return front of queue (or last scancode if empty - matches hardware behavior)
    return port60h_event[0];
}

uint8_t libqb_port60h_pop() {
    if (port60h_events == 0) {
        // Return last scancode without popping (matches hardware behavior)
        return port60h_event[0];
    }
    uint8_t value = port60h_event[0];
    port60h_events--;
    if (port60h_events > 0) {
        memmove(port60h_event, port60h_event + 1, port60h_events);
    }
    return value;
}

void libqb_port60h_push(uint8_t scancode) {
    if (port60h_events < 256) {
        port60h_event[port60h_events] = scancode;
        port60h_events++;
    }
}

void libqb_port60h_push_release(uint8_t scancode) {
    libqb_port60h_push(scancode | 0x80);
}

// ============================================================================
// MOUSE QUEUE ACCESSORS
// ============================================================================

void libqb_mouse_get_current(struct libqb_mouse_state* state) {
    if (state == nullptr) {
        return;
    }
    mouse_message* msg = &mouse_message_queue.queue[mouse_message_queue.current];
    state->x = msg->x;
    state->y = msg->y;
    state->movementX = msg->movementX;
    state->movementY = msg->movementY;
    state->buttons = msg->buttons;
    state->wheel = msg->wheel;
}

int32_t libqb_mouse_input_next() {
    if (mouse_message_queue.current == mouse_message_queue.last) {
        return 0;
    }
    mouse_message_queue.current++;
    if (mouse_message_queue.current > mouse_message_queue.lastIndex) {
        mouse_message_queue.current = 0;
    }
    return -1;
}

int32_t libqb_mouse_has_pending() {
    return (mouse_message_queue.current != mouse_message_queue.last) ? 1 : 0;
}

// ============================================================================
// MOUSE STATE ACCESSORS
// ============================================================================

int32_t libqb_get_mouse_hidden() {
    return mouse_hiddden;
}

void libqb_set_mouse_hidden(int32_t value) {
    mouse_hiddden = value;
}

int32_t libqb_get_mouse_cursor_style() {
    return mouse_cursor_style;
}

void libqb_set_mouse_cursor_style(int32_t style) {
    mouse_cursor_style = style;
}

// ============================================================================
// CODEPAGE MAPPING ACCESSORS
// ============================================================================

uint16_t libqb_get_codepage_mapping(int32_t ascii_code) {
    if (ascii_code < 0 || ascii_code > 255) {
        return 0;
    }
    return codepage437_to_unicode16[ascii_code];
}

void libqb_set_codepage_mapping(int32_t ascii_code, uint16_t unicode_code) {
    if (ascii_code < 0 || ascii_code > 255) {
        return;
    }
    codepage437_to_unicode16[ascii_code] = unicode_code;
}
