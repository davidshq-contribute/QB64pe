#include "libqb.h"
#include "common.h"

#ifdef QB64_WINDOWS
#    include <fcntl.h>
#    include <shellapi.h>
#endif

#ifdef QB64_MACOSX
#    include <ApplicationServices/ApplicationServices.h>

#    include <mach-o/dyld.h> //required for _NSGetExecutablePath
#endif

#include "audio.h"
#include "bitops.h"
#include "charset_data.h"
#include "static_data.h"
#include "print_using.h"
#include "cmem.h"
#include "command.h"
#include "completion.h"
#include "compression.h"
#include "datetime.h"
#include "encoding.h"
#include "error_handle.h"
#include "event.h"
#include "extended_math.h"
#include "file-fields.h"
#include "filepath.h"
#include "filesystem.h"
#include "font.h"
#include "game_controller.h"
#include "gfs.h"
#include "glut-thread.h"
#include "graphics.h"
#include "gui.h"
#include "hashing.h"
#include "http.h"
#include "image.h"
#include "keyhandler.h"
#include "logging.h"
#include "mac-mouse-support.h"
#include "mem.h"
#include "mutex.h"
#include "qblist.h"
#include "qbs.h"
#include "rounding.h"
#include "shell.h"
#include "thread.h"
#include "handles.h"
#include "fileio.h"
#include "color.h"
#include "screen.h"
#include "mem_legacy.h"
#include "keyboard.h"
#include "mouse.h"
#include "console.h"
#include "utility.h"
#include "window.h"
#include "text.h"
#include "platform.h"
#include "networking.h"

// These are here because they are used in func__loadfont()
#include <algorithm>
#include <string>
#include <vector>

int32 disableEvents = 0;

// Global console values (exported for console.cpp)
int32 consolekey;
int32 consolemousex;  // exported for mouse.cpp
int32 consolemousey;  // exported for mouse.cpp
int32 consolebutton;  // exported for mouse.cpp

// This next block used to be in common.cpp; put here until I can find a better
// place for it (LC, 2018-01-05)

/**
 * Performs a left bit rotation on a 32-bit value.
 * @param word The 32-bit value to rotate
 * @param shift Number of bits to shift left
 * @return The rotated value with bits wrapping around
 */
uint32 rotateLeft(uint32 word, uint32 shift) {
    return (word << shift) | (word >> (32 - shift));
}

#ifdef QB64_UNIX
#    include <libgen.h> //required for dirname()
#    include <pthread.h>
#endif

#ifdef QB64_LINUX
#    include <X11/Xatom.h>
#    include <X11/Xlib.h>
#    include <X11/Xutil.h>
Display *X11_display = NULL;
Window X11_window;
#endif

// X11 threading lock state variables
int32 x11_locked = 0;
int32 x11_lock_request = 0;

/**
 * Acquires X11 display lock for thread-safe operations.
 * Blocks until the lock is successfully acquired.
 */
void x11_lock() {
    x11_lock_request = 1;
    while (x11_locked == 0)
        Sleep(1);
}

/**
 * Releases X11 display lock, allowing other threads to access X11.
 */
void x11_unlock() {
    x11_locked = 0;
}

// forward references
void sub__printimage(int32 i);

// GUI notification variables
int32 force_display_update = 0;

void *generic_window_handle = NULL;
int32 acceptFileDrop = 0;
int32 totalDroppedFiles = 0;
#ifdef QB64_WINDOWS
HWND window_handle = NULL;
HDROP hdrop = NULL;
#endif
//...

extern "C" void QB64_Window_Handle(void *handle) {
    generic_window_handle = handle;
#ifdef QB64_WINDOWS
    window_handle = (HWND)handle;
#endif
    //...
}

// forward references
void set_view(int32 new_mode);
void set_render_source(int32 new_handle);
void set_render_dest(int32 new_handle);

int32 framebufferobjects_supported = 0;

int32 environment_2d__screen_width = 0; // the size of the software SCREEN
int32 environment_2d__screen_height = 0;
int32 environment__window_width = 0; // window may be larger or smaller than the SCREEN
int32 environment__window_height = 0;
int32 environment_2d__screen_x1 = 0; // offsets of 'screen' within the window
int32 environment_2d__screen_y1 = 0;
int32 environment_2d__screen_x2 = 0;
int32 environment_2d__screen_y2 = 0;
int32 environment_2d__screen_scaled_width = 640; // initial values prevent _SCALEDWIDTH/_SCALEDHEIGHT returning 0
int32 environment_2d__screen_scaled_height = 400;
float environment_2d__screen_x_scale = 1.0f;
float environment_2d__screen_y_scale = 1.0f;
int32 environment_2d__screen_smooth = 0; // 1(LINEAR) or 0(NEAREST)
int32 environment_2d__letterbox = 0;     // 1=vertical black stripes required, 2=horizontal black stripes required

int32 window_focused = 0; // Not used on Windows
uint8 *window_title = NULL;

// max_fps and auto_fps moved to utility.cpp

int32 os_resize_event = 0;

int32 resize_auto = 0; // 1=_STRETCH, 2=_SMOOTH
float resize_auto_ideal_aspect = 640.0 / 400.0;
float resize_auto_accept_aspect = 640.0 / 400.0;

int32 fullscreen_allowedmode = 0;
int32 fullscreen_allowedsmooth = 0;
int32 fullscreen_smooth = 0;
int32 fullscreen_width = 0;
int32 fullscreen_height = 0;
int32 screen_scale = 0;
int32 resize_pending = 1;
int32 resize_snapback = 1;
int32 resize_snapback_x = 640;
int32 resize_snapback_y = 400;
int32 resize_event = 0;
int32 resize_event_x = 0;
int32 resize_event_y = 0;

int32 ScreenResizeScale = 0;
int32 ScreenResize = 0;

extern "C" int QB64_Resizable() {
    return ScreenResize;
}

int32 sub_gl_called = 0;

static int32 image_qbicon16_handle;
static int32 image_qbicon32_handle;


static int32 display_x = 640;
static int32 display_y = 400;
int32 display_x_prev = 640, display_y_prev = 400;

static int32 display_required_x = 640;
static int32 display_required_y = 400;

int32 dont_call_sub_gl = 0;

void GLUT_DISPLAY_REQUEST();

struct display_frame_struct {
    int32 state;
    int64 order;
    uint32 *bgra;
    int32 w;
    int32 h;
    int32 bytes; // w*h*4
};

display_frame_struct display_frame[3];
int64 display_frame_order_next = 1;
#define DISPLAY_FRAME_STATE__EMPTY 1
#define DISPLAY_FRAME_STATE__BUILDING 2
#define DISPLAY_FRAME_STATE__READY 3
#define DISPLAY_FRAME_STATE__DISPLAYING 4

// when a new software frame is not required by display(), hardware content may exist,
// and if it does then this variable is used to determine to highest order index to render
int64 last_rendered_hardware_display_frame_order = 0;
int64 last_hardware_display_frame_order = 0;

// special_handle_type, special_handle_struct, stream_type, stream_struct
// are now defined in handles.h

list *special_handles = NULL;
list *stream_handles = NULL;

// stream_free, stream_update, stream_out, connection_close moved to networking.cpp

// Hardware texture state now defined in graphics.cpp
extern RENDER_STATE_GLOBAL render_state;
extern RENDER_STATE_DEST dest_render_state0;
extern list *hardware_img_handles;

int32 first_hardware_command = 0;          // only set once
int32 last_hardware_command_added = 0;     // 0 if none exist
int32 last_hardware_command_rendered = 0;  // 0 if all have been processed
int32 next_hardware_command_to_remove = 0; // 0 if all have been processed

list *hardware_graphics_command_handles = NULL;

int32 SOFTWARE_IMG_HANDLE_MIN = -8388608;
// the lowest integer value a single precision number can exactly represent,
// because users put handles in SINGLEs

// Common
int32 requestedKeyboardOverlayImage = 0;

// Hardware texture functions now defined in graphics.cpp
// (new_hardware_img, NPO2_texture_generate, hardware_img_buffer_to_texture,
//  hardware_img_requires_depthbuffer, get_hardware_img, get_hardware_img_index)

// these lock values increment
int64 display_lock_request = 0;
int64 display_lock_confirmed = 0;
int64 display_lock_released = 0;

// codepage437_to_unicode16 is now defined in libqb/src/static_data.cpp
// and declared in libqb/include/static_data.h

/* Restricted Functionality: (Security focused approach, does not include restricting sound etc)

    Block while compiling: (ONLY things that cannot be caught at runtime)
    - $CHECKING:OFF [X]
    - _MEM(x,y) [X]
    - DECLARE LIBRARY [X]

    Block at runtime:
    - paths [fixdir]
    - MKDIR [sub_mkdir]
    - SHELL(subs/functions)
    [func__shellhide,
    func__shell,
    sub_shell,
    sub_shell2,
    sub_shell3,
    sub_shell4]
    - RUN "filename" [sub_run]
    - CHAIN [sub_chain]
    - SCREENPRINT [sub__screenprint]
    - SCREENCLICK [sub__screenclick]
    - SCREENIMAGE (returns a blank 1024x768 image)[func__screenimage]
    - ENVIRON [func_environ(num&str), sub_environ]

    Reference notes:
    - KILL calls fixdir()

    Ports:
    - Client connections are unrestricted
    - Host ports values are either 1 or 2, but the default is port 1 for out of range values

*/

/*
    int32 allocated_bytes=0;
    void *malloc2(int x){
    allocated_bytes+=x;
    return malloc(x);
    }
    void *realloc2(void *x, int y){
    allocated_bytes+=y;
    return realloc(x,y);
    }
    void *calloc2(int x, int y){
    allocated_bytes+=(x*y);
    return calloc(x,y);
    }
    #define malloc(x) malloc2(x)
    #define calloc(x,y) calloc2(x,y)
    #define realloc(x,y) realloc2(x,y)
*/

int64 device_event_index = 0;
int32 device_mouse_relative = 0;

int32 lock_mainloop = 0; // 0=unlocked, 1=lock requested, 2=locked

int32 lpos = 1;
int32 width_lprint = 80;

// forward refs
int32 autodisplay = 1;
void key_update();
int32 key_display_state = 0;
int32 key_display = 0;
int32 key_display_redraw = 0;

extern ontimer_struct *ontimer;
extern onkey_struct *onkey;
extern int32 onkey_inprogress;

extern int32 console;
extern int32 screen_hide_startup;
extern int32 asserts;
extern int32 vwatch;
//...

int64 exit_code = 0;

int32 console_active = 1;
int32 console_child = 0; // set if console is only being used by this program
int32 console_image = -1;
int32 screen_hide = 0;

// format:[deadkey's symbol in UTF16],[ASCII code of alphabet letter],[resulting UTF16 character]...0

// Global flag to indicate if a glyph (unicode) key event occurred
int32 keydown_glyph = 0;

/**
 * Converts text between different character encodings.
 * @param src_fmt Source format: 1=ASCII(CP437), 8=UTF8, 16=UTF16, 32=UTF32
 * @param src_buf Pointer to source buffer
 * @param src_size Size of source buffer in bytes
 * @param dest_fmt Destination format: 1=ASCII(CP437), 8=UTF8, 16=UTF16, 32=UTF32
 * @param dest_buf Pointer to destination buffer (must be at least src_size*4+4 bytes)
 * @return Number of bytes written to destination buffer
 */
int32 convert_unicode(int32 src_fmt, void *src_buf, int32 src_size, int32 dest_fmt, void *dest_buf) {
    /*
        important: to ensure enough space is available for the conversion, dest_buf must be at least src_size*4+4 in length
        returns: the number of bytes written to dest_buf
        fmt values:
        1=ASCII(CP437)
        8=UTF8
        16=UTF16
        32=UTF32
    */

    static int32 dest_size;
    dest_size = 0;

    // setup source
    uint8 *src_uint8p = NULL;
    if (src_fmt == 1) {
        src_uint8p = (uint8 *)src_buf;
    }
    uint16 *src_uint16p = NULL;
    if (src_fmt == 16) {
        src_uint16p = (uint16 *)src_buf;
        src_size = src_size - (src_size & 1); // cull trailing bytes
    }
    uint32 *src_uint32p = NULL;
    if (src_fmt == 32) {
        src_uint32p = (uint32 *)src_buf;
        src_size = src_size - (src_size & 3); // cull trailing bytes
    }

    // setup dest
    uint16 *dest_uint16p = NULL;
    if (dest_fmt == 16) {
        dest_uint16p = (uint16 *)dest_buf;
    }
    uint32 *dest_uint32p = NULL;
    if (dest_fmt == 32) {
        dest_uint32p = (uint32 *)dest_buf;
    }

    uint32 x; // scalar

    while (src_size) {

        // convert src to scalar UNICODE value 'x'

        if (src_fmt == 1) { // CP437
            x = *src_uint8p++;
            src_size--;
            x = codepage437_to_unicode16[x];
        }
        if (src_fmt == 16) { // UTF16
            src_size -= 2;
            x = *src_uint16p++;
            // note: does not handle surrogate pairs yet
        }
        if (src_fmt == 32) { // UTF32
            src_size -= 4;
            x = *src_uint32p++;
        }

        // convert scalar UNICODE value 'x' to dest

        if (dest_fmt == 16) { // UTF16
            *dest_uint16p++ = x;
            dest_size += 2;
            // note: does not handle surrogate pairs yet
        }
        if (dest_fmt == 32) { // UTF32
            *dest_uint32p++ = x;
            dest_size += 4;
        }

    } // loop

    // add NULL terminator (does not change the size in bytes returned)
    if (dest_fmt == 16)
        *dest_uint16p = 0;
    if (dest_fmt == 32)
        *dest_uint32p = 0;

    return dest_size;
}

#ifdef QB64_WINDOWS
void showvalue(__int64);
#endif

int32 lastfont = 48;
int32 *font = (int32 *)calloc(4 * (48 + 1), 1); // NULL=unused index
int32 *fontheight = (int32 *)calloc(4 * (48 + 1), 1);
int32 *fontwidth = (int32 *)calloc(4 * (48 + 1), 1);
int32 *fontflags = (int32 *)calloc(4 * (48 + 1), 1);

// keyhit cyclic buffer
int64 keyhit[8192];
//    keyhit specific internal flags: (stored in high 32-bits)
//    &4294967296->numpad was used
int32 keyhit_nextfree = 0;
int32 keyhit_next = 0;
// note: if full, the oldest message is discarded to make way for the new message

void update_shift_state();

uint32 bindkey = 0;

void scancodedown(uint8 scancode);
void scancodeup(uint8 scancode);

/*
    QB64 Mapping of audio control keyboard keys:
    MEDIA_PLAY_PAUSE 0x2200
    MEDIA_STOP 0x2400
    MEDIA_NEXT_TRACK 0x1900
    MEDIA_PREV_TRACK 0x1000
*/

// scancode_lookup table is defined in libqb/src/keyboard.cpp
// See keyboard.h for extern declaration

void keydown(uint32 x);
void keyup(uint32 x);

/**
 * Converts a Unicode character to its CP437 (code page 437) equivalent.
 * @param x Unicode character to convert
 * @return CP437 character code, or 0 if no equivalent exists
 */
uint32 unicode_to_cp437(uint32 x) {
    static int32 i;
    for (i = 0; i <= 255; i++) {
        if (x == codepage437_to_unicode16[i])
            return i;
    }
    return 0;
}

// Keyboard state tracking buffers and variables
uint32 *keyheld_buffer = (uint32 *)malloc(1);
uint32 *keyheld_bind_buffer = (uint32 *)malloc(1);
int32 keyheld_n = 0;
int32 keyheld_size = 0;

/**
 * Checks if a specific key is currently being held down.
 * Handles special cases for numpad keys and multimapped keys.
 * @param x Key code to check
 * @return 1 if key is held down, 0 otherwise
 */
int32 keyheld(uint32 x) {
    static int32 i;
    for (i = 0; i < keyheld_n; i++) {
        if (keyheld_buffer[i] == x)
            return 1;
    }
    // check multimapped NUMPAD keys
    if ((x >= 42) && (x <= 57)) {
        if ((x >= 48) && (x <= 57))
            return keyheld(VK + QBVK_KP0 + (x - 48)); // 0-9
        if (x == 46)
            return keyheld(VK + QBVK_KP_PERIOD);
        if (x == 47)
            return keyheld(VK + QBVK_KP_DIVIDE);
        if (x == 42)
            return keyheld(VK + QBVK_KP_MULTIPLY);
        if (x == 45)
            return keyheld(VK + QBVK_KP_MINUS);
        if (x == 43)
            return keyheld(VK + QBVK_KP_PLUS);
    }
    if (x == 13)
        return keyheld(VK + QBVK_KP_ENTER);
    if (x & 0xFF00) {
        static uint32 x2;
        x2 = (x >> 8) & 255;
        if ((x2 >= 71) && (x2 <= 83)) {
            if (x2 == 82)
                return keyheld(QBK + QBVK_KP0 - QBVK_KP0);
            if (x2 == 79)
                return keyheld(QBK + QBVK_KP1 - QBVK_KP0);
            if (x2 == 80)
                return keyheld(QBK + QBVK_KP2 - QBVK_KP0);
            if (x2 == 81)
                return keyheld(QBK + QBVK_KP3 - QBVK_KP0);
            if (x2 == 75)
                return keyheld(QBK + QBVK_KP4 - QBVK_KP0);
            if (x2 == 76)
                return keyheld(QBK + QBVK_KP5 - QBVK_KP0);
            if (x2 == 77)
                return keyheld(QBK + QBVK_KP6 - QBVK_KP0);
            if (x2 == 71)
                return keyheld(QBK + QBVK_KP7 - QBVK_KP0);
            if (x2 == 72)
                return keyheld(QBK + QBVK_KP8 - QBVK_KP0);
            if (x2 == 73)
                return keyheld(QBK + QBVK_KP9 - QBVK_KP0);
            if (x2 == 83)
                return keyheld(QBK + QBVK_KP_PERIOD - QBVK_KP0);
        }
    }
    return 0;
}

/**
 * Adds a key to the held keys buffer.
 * Expands the buffer if necessary and records any bound key.
 * @param x Key code to add
 */
void keyheld_add(uint32 x) {
    static int32 i;
    for (i = 0; i < keyheld_n; i++) {
        if (keyheld_buffer[i] == x)
            return;
    } // already in buffer
    if (keyheld_n == keyheld_size) {
        keyheld_size++;
        keyheld_buffer = (uint32 *)realloc(keyheld_buffer, keyheld_size * 4);
        keyheld_bind_buffer = (uint32 *)realloc(keyheld_bind_buffer, keyheld_size * 4);
    } // expand buffer
    keyheld_buffer[keyheld_n] = x; // add entry
    keyheld_bind_buffer[keyheld_n] = bindkey;
    bindkey = 0; // add binded key (0=none)
    keyheld_n++; // note: inc. must occur after setting entry (threading reasons)
}

/**
 * Removes a key from the held keys buffer.
 * Compacts the buffer by shifting remaining entries.
 * @param x Key code to remove
 */
void keyheld_remove(uint32 x) {
    static int32 i;
    for (i = 0; i < keyheld_n; i++) {
        if (keyheld_buffer[i] == x) { // exists
            memmove(&keyheld_buffer[i], &keyheld_buffer[i + 1], (keyheld_n - i - 1) * 4);
            memmove(&keyheld_bind_buffer[i], &keyheld_bind_buffer[i + 1], (keyheld_n - i - 1) * 4);
            keyheld_n--; // note: dec. must occur after memmove (threading reasons)
            return;
        }
    }
}

/**
 * Unbinds and releases a key that was bound to another key.
 * @param x Key code to unbind
 */
void keyheld_unbind(uint32 x) {
    static int32 i;
    for (i = 0; i < keyheld_n; i++) {
        if (keyheld_bind_buffer[i] == x) { // exists
            keyup(keyheld_buffer[i]);
            return;
        }
    }
}

/**
 * Handles ASCII key down events.
 * @param x ASCII character code
 */
void keydown_ascii(uint32 x) {
    keydown(x);
}

/**
 * Handles Unicode key down events.
 * Maps Unicode characters to ASCII/CP437 equivalents when possible.
 * @param x Unicode character code
 */
void keydown_unicode(uint32 x) {
    keydown_glyph = 1;
    // note: UNICODE 0-127 map directly to ASCII 0-127
    if (x <= 127) {
        keydown_ascii(x);
        return;
    }
    // note: some UNICODE values map directly to CP437 values found in the extended ASCII set
    auto x2 = unicode_to_cp437(x);
    if (x2) {
        keydown_ascii(x2);
        return;
    }
    // note: full width latin characters will be mapped to their normal width equivalents
    // Wikipedia note: Range U+FF01\96FF5E reproduces the characters of ASCII 21 to 7E as fullwidth forms, that is, a fixed width form used in CJK computing.
    // This is useful for typesetting Latin characters in a CJK  environment. U+FF00 does not correspond to a fullwidth ASCII 20 (space character), since that
    // role is already fulfilled by U+3000 "ideographic space."
    if ((x >= 0x0000FF01) && (x <= 0x0000FF5E)) {
        keydown_ascii(x - 0x0000FF01 + 0x21);
        return;
    }
    if (x == 0x3000) {
        keydown_ascii(32);
        return;
    }
    x |= UC;
    keydown(x);
}

/**
 * Handles virtual key down events.
 * @param x Virtual key code
 */
void keydown_vk(uint32 x) {
    keydown(x);
}

/**
 * Handles ASCII key up events.
 * @param x ASCII character code
 */
void keyup_ascii(uint32 x) {
    keyup(x);
}

/**
 * Handles Unicode key up events.
 * Maps Unicode characters to ASCII/CP437 equivalents when possible.
 * @param x Unicode character code
 */
void keyup_unicode(uint32 x) {
    // note: UNICODE 0-127 map directly to ASCII 0-127
    if (x <= 127) {
        keyup_ascii(x);
        return;
    }
    // note: some UNICODE values map directly to CP437 values found in the extended ASCII set
    auto x2 = unicode_to_cp437(x);
    if (x2) {
        keyup_ascii(x2);
        return;
    }
    // note: full width latin characters will be mapped to their normal width equivalents
    // Wikipedia note: Range U+FF01\96FF5E reproduces the characters of ASCII 21 to 7E as fullwidth forms, that is, a fixed width form used in CJK computing.
    // This is useful for typesetting Latin characters in a CJK  environment. U+FF00 does not correspond to a fullwidth ASCII 20 (space character), since that
    // role is already fulfilled by U+3000 "ideographic space."
    if ((x >= 0x0000FF01) && (x <= 0x0000FF5E)) {
        keyup_ascii(x - 0x0000FF01 + 0x21);
        return;
    }
    if (x == 0x3000) {
        keyup_ascii(32);
        return;
    }
    x |= UC;
    keyup(x);
}

/**
 * Handles virtual key up events.
 * @param x Virtual key code
 */
void keyup_vk(uint32 x) {
    keyup(x);
}

int32 exit_ok = 0;

// substitute Windows functionality
#ifndef QB64_WINDOWS
void AllocConsole() {
    return;
}

void FreeConsole() {
    return;
}
#endif

// vc->project->properties->configuration properties->general->configuration type->application(.exe)
// vc->project->properties->configuration properties->general->configuration type->static library(.lib)

extern void QBMAIN(void *);
extern void TIMERTHREAD(void *);
void MAIN_LOOP(void *);

void GLUT_MAINLOOP_THREAD(void *);
void GLUT_DISPLAY_REQUEST();

extern qbs *FUNC__DECODEURL(qbs *_FUNC__DECODEURL_STRING__URL);
extern qbs *FUNC__WHATISMYIP();

int32 lprint = 0; // set to 1 during LPRINT operations
int32 lprint_image = 0;
double lprint_last = 0;    // TIMER(0.001) value at last time LPRINT was used
int32 lprint_buffered = 0; // set to 1 if content is pending to print
int32 lprint_locked = 0;   // set to 1 to deny access by QB64 program


uint16 *unicode16_buf = (uint16 *)malloc(1);
int32 unicode16_buf_size = 1;

void convert_text_to_utf16(int32 fonthandle, void *buf, int32 size) {
    // expand buffer if necessary
    if (unicode16_buf_size < (size * 4 + 4)) {
        unicode16_buf_size = size * 4 + 4;
        free(unicode16_buf);
        unicode16_buf = (uint16 *)malloc(unicode16_buf_size);
    }
    // convert text
    if ((fontflags[fonthandle] & FONT_LOAD_UNICODE) && fonthandle) { // unicode font
        if (size == 1)
            size = 4;
        convert_unicode(32, buf, size, 16, unicode16_buf);
    } else {
        convert_unicode(1, buf, size, 16, unicode16_buf);
    }
}

qbs *unknown_opcode_mess;

int32 exit_blocked = 0;
int32 exit_value = 0;
// 1=X-button
// 2=CTRL-BREAK
// 3=X-button and CTRL-BREAK

// MLP
// int32 qbshlp1=0;

int32 width8050switch = 1; // if set, can automatically switch to WIDTH 80,50 if LOCATE'ing beyond row 26

uint32 pal[256];

extern qbs *nothingstring;

int32 sub_screen_height_in_characters = -1; //-1=undefined
int32 sub_screen_width_in_characters = -1;  //-1=undefined
int32 sub_screen_font = -1;                 //-1=undefined
int32 sub_screen_keep_page0 = 0;

uint32 palette_256[256];
uint32 palette_64[64];

// QB64 2D PROTOTYPE 1.0

int32 pages = 1;
int32 *page = (int32 *)calloc(1, 4);

#define IMG_BUFFERSIZE 4096
img_struct *img = (img_struct *)malloc(IMG_BUFFERSIZE * sizeof(img_struct));
int32 nimg = IMG_BUFFERSIZE;
int32 nextimg = 0;

uint32 *fimg = (uint32 *)malloc(IMG_BUFFERSIZE * 4); // a list to recover freed indexes
int32 nfimg = IMG_BUFFERSIZE;
int32 lastfimg = -1; //-1=no freed indexes exist

uint8 *cblend = NULL;
uint8 *ablend = NULL;
uint8 *ablend127;
uint8 *ablend128;

// to save 16MB of RAM, software blend tables are only allocated if a 32-bit image is created
void init_blend() {
    uint8 *cp;
    int32 i, x2, x3, i2, z;
    float f, f2, f3;
    cblend = (uint8 *)malloc(16777216);
    cp = cblend;
    for (i = 0; i < 256; i++) {            // source alpha
        for (x2 = 0; x2 < 256; x2++) {     // source
            for (x3 = 0; x3 < 256; x3++) { // dest
                f = i;
                f2 = x2;
                f3 = x3;
                f /= 255.0;                                             // 0.0-1.0
                *cp++ = qbr_float_to_long((f * f2) + ((1.0 - f) * f3)); // CINT(0.0-255.0)
            }
        }
    }
    /*
        "60%+60%=84%" formula
        imagine a 60% opaque lens, you can see 40% of what's behind
        now put another 60% opaque lens on top of it
        you can now see 40% of the previous lens of which 40% is of the original scene
        40% of 40% is 16%
        100%-16%=84%
        V1=60, V2=60
        v1=V1/100, v2=V2/100
        iv1=1-v1, iv2=1-v2
        iv3=iv1*iv2
        v3=1-iv3
        V3=v3*100
    */
    ablend = (uint8 *)malloc(65536);
    cp = ablend;
    for (i = 0; i < 256; i++) {        // first alpha value
        for (i2 = 0; i2 < 256; i2++) { // second alpha value
            f = i;
            f2 = i2;
            f /= 255.0;
            f2 /= 255.0;
            f = 1.0 - f;
            f2 = 1.0 - f2;
            f3 = f * f2;
            z = qbr_float_to_long((1.0 - f3) * 255.0);
            *cp++ = z;
        }
    }
    ablend127 = ablend + (127 << 8);
    ablend128 = ablend + (128 << 8);
}

uint32 display_page_index = 0;
uint32 write_page_index = 0;
uint32 read_page_index = 0;
// use of non-indexed forms assumes valid indexes (may not be suitable for all commands)
img_struct *write_page = NULL;
img_struct *read_page = NULL;
img_struct *display_page = NULL;
uint32 *display_surface_offset = 0;

// Software image management functions moved to graphics.cpp:
// restorepalette(), pset(), newimg(), freeimg(), imgrevert(), imgframe(), imgnew()

// flush_old_hardware_commands moved to graphics.cpp
// sub__putimage moved to graphics.cpp

// selectfont moved to text.cpp

int32 nmodes = 0;
int32 anymode = 0;

int32 x_offset = 0, y_offset = 0;
int32 x_limit = 0, y_limit = 0;

int32 x_monitor = 0, y_monitor = 0;

int32 conversion_required = 0;
uint32 *conversion_layer = (uint32 *)malloc(8);

#define AUDIO_CHANNELS 256

#define sndqueue_lastindex 9999
uint32 sndqueue[sndqueue_lastindex + 1];
int32 sndqueue_next = 0;
int32 sndqueue_first = 0;
int32 sndqueue_wait = -1;
int32 sndqueue_played = 0;

void call_int(int32 i);

uint32 frame = 0;

extern uint8 cmem[1114099]; // 16*65535+65535+3 (enough for highest referencable dword in conv memory)

int32 mouse_hiddden = 0;  // exported for mouse.cpp

struct mouse_message {
    int16 x;
    int16 y;
    uint32 buttons;
    int16 movementx;
    int16 movementy;
};

// Mouse message queue
//--------------------
struct mouse_message_queue_struct {
    mouse_message *queue;
    int32 lastIndex;
    int32 current;
    int32 last;
};

mouse_message_queue_struct mouse_message_queue = {NULL, 0, 0, 0};  // exported for mouse.cpp

// x86 Virtual CMEM emulation
// Note: x86 CPU emulation is still experimental and is not available in QB64 yet.
struct cpu_struct {
    // al,ah,ax,eax (unsigned & signed)
    union {
        struct {
            union {
                uint8 al;
                int8 al_signed;
            };

            union {
                uint8 ah;
                int8 ah_signed;
            };
        };

        uint16 ax;
        int16 ax_signed;
        uint32 eax;
        int32 eax_signed;
    };

    // bl,bh,bx,ebx (unsigned & signed)
    union {
        struct {
            union {
                uint8 bl;
                int8 bl_signed;
            };

            union {
                uint8 bh;
                int8 bh_signed;
            };
        };

        uint16 bx;
        int16 bx_signed;
        uint32 ebx;
        int32 ebx_signed;
    };

    // cl,ch,cx,ecx (unsigned & signed)
    union {
        struct {
            union {
                uint8 cl;
                int8 cl_signed;
            };

            union {
                uint8 ch;
                int8 ch_signed;
            };
        };

        uint16 cx;
        int16 cx_signed;
        uint32 ecx;
        int32 ecx_signed;
    };

    // dl,dh,dx,edx (unsigned & signed)
    union {
        struct {
            union {
                uint8 dl;
                int8 dl_signed;
            };

            union {
                uint8 dh;
                int8 dh_signed;
            };
        };

        uint16 dx;
        int16 dx_signed;
        uint32 edx;
        int32 edx_signed;
    };

    // si,esi (unsigned & signed)
    union {
        uint16 si;
        int16 si_signed;
        uint32 esi;
        int32 esi_signed;
    };

    // di,edi (unsigned & signed)
    union {
        uint16 di;
        int16 di_signed;
        uint32 edi;
        int32 edi_signed;
    };

    // bp,ebp (unsigned & signed)
    union {
        uint16 bp;
        int16 bp_signed;
        uint32 ebp;
        int32 ebp_signed;
    };

    // sp,esp (unsigned & signed)
    union {
        uint16 sp;
        int16 sp_signed;
        uint32 esp;
        int32 esp_signed;
    };

    // cs,ss,ds,es,fs,gs (unsigned & signed)
    union {
        uint16 cs;
        uint16 cs_signed;
    };

    union {
        uint16 ss;
        uint16 ss_signed;
    };

    union {
        uint16 ds;
        uint16 ds_signed;
    };

    union {
        uint16 es;
        uint16 es_signed;
    };

    union {
        uint16 fs;
        uint16 fs_signed;
    };

    union {
        uint16 gs;
        uint16 gs_signed;
    };

    // ip,eip (unsigned & signed)
    union {
        uint16 ip;
        uint16 ip_signed;
        uint32 eip;
        uint32 eip_signed;
    };

    // flags
    uint8 overflow_flag;
    uint8 direction_flag;
    uint8 interrupt_flag;
    uint8 trap_flag;
    uint8 sign_flag;
    uint8 zero_flag;
    uint8 auxiliary_flag;
    uint8 parity_flag;
    uint8 carry_flag;
};

cpu_struct cpu;

uint8 *ip;
uint8 *seg;    // default segment (DS unless overridden)
uint8 *seg_bp; // the segment bp will be accessed from (SS unless overridden)

uint8 *reg8[8];
uint16 *reg16[8];
uint32 *reg32[8];
uint16 *segreg[8];

int32 a32;
int32 b32; // size of data to read/write in bits is 32

uint32 sib() {
    static uint32 i; // sib byte
    i = *ip++;
    switch (i >> 6) {
    case 0:
        return *reg32[i & 7] + *reg32[i >> 3 & 7];
        break;
    case 1:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 1);
        break;
    case 2:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 2);
        break;
    case 3:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 3);
        break;
    }

    return 0;
}

uint32 sib_mod0() {
    // Note: Called when top 2 bits of rm byte before sib byte were 0, base register is ignored
    //      and replaced with an int32 following the sib byte
    static uint32 i; // sib byte
    i = *ip++;
    if ((i & 7) == 5) {
        switch (i >> 6) {
        case 0:
            return (*(uint32 *)((ip += 4) - 4)) + *reg32[i >> 3 & 7];
            break;
        case 1:
            return (*(uint32 *)((ip += 4) - 4)) + (*reg32[i >> 3 & 7] << 1);
            break;
        case 2:
            return (*(uint32 *)((ip += 4) - 4)) + (*reg32[i >> 3 & 7] << 2);
            break;
        case 3:
            return (*(uint32 *)((ip += 4) - 4)) + (*reg32[i >> 3 & 7] << 3);
            break;
        }
    }
    switch (i >> 6) {
    case 0:
        return *reg32[i & 7] + *reg32[i >> 3 & 7];
        break;
    case 1:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 1);
        break;
    case 2:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 2);
        break;
    case 3:
        return *reg32[i & 7] + (*reg32[i >> 3 & 7] << 3);
        break;
    }

    return 0;
}

uint8 *rm8() {
    static uint32 i; // r/m byte
    i = *ip++;
    switch (i >> 6) {
    case 3:
        return reg8[i & 7];
        break;
    case 0:
        if (a32) {
            switch (i & 7) {
            case 0:
                return seg + cpu.ax;
                break;
            case 1:
                return seg + cpu.cx;
                break;
            case 2:
                return seg + cpu.dx;
                break;
            case 3:
                return seg + cpu.bx;
                break;
            case 4:
                return seg + (uint16)sib_mod0();
                break;
            case 5:
                return seg + (*(uint16 *)((ip += 4) - 4));
                break;
            case 6:
                return seg + cpu.si;
                break;
            case 7:
                return seg + cpu.di;
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.bx + cpu.si));
                break;
            case 1:
                return seg + ((uint16)(cpu.bx + cpu.di));
                break;
            case 2:
                return seg_bp + ((uint16)(cpu.bp + cpu.si));
                break;
            case 3:
                return seg_bp + ((uint16)(cpu.bp + cpu.di));
                break;
            case 4:
                return seg + cpu.si;
                break;
            case 5:
                return seg + cpu.di;
                break;
            case 6:
                return seg + (*(uint16 *)((ip += 2) - 2));
                break;
            case 7:
                return seg + cpu.bx;
                break;
            }
        }
        break;
    case 1:
        if (a32) {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.eax + *(int8 *)ip++));
                break;
            case 1:
                return seg + ((uint16)(cpu.ecx + *(int8 *)ip++));
                break;
            case 2:
                return seg + ((uint16)(cpu.edx + *(int8 *)ip++));
                break;
            case 3:
                return seg + ((uint16)(cpu.ebx + *(int8 *)ip++));
                break;
            case 4:
                i = sib();
                return seg + ((uint16)(i + *(int8 *)ip++));
                break;
            case 5:
                return seg_bp + ((uint16)(cpu.ebp + *(int8 *)ip++));
                break;
            case 6:
                return seg + ((uint16)(cpu.esi + *(int8 *)ip++));
                break;
            case 7:
                return seg + ((uint16)(cpu.edi + *(int8 *)ip++));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.bx + cpu.si + *(int8 *)ip++));
                break;
            case 1:
                return seg + ((uint16)(cpu.bx + cpu.di + *(int8 *)ip++));
                break;
            case 2:
                return seg_bp + ((uint16)(cpu.bp + cpu.si + *(int8 *)ip++));
                break;
            case 3:
                return seg_bp + ((uint16)(cpu.bp + cpu.di + *(int8 *)ip++));
                break;
            case 4:
                return seg + ((uint16)(cpu.si + *(int8 *)ip++));
                break;
            case 5:
                return seg + ((uint16)(cpu.di + *(int8 *)ip++));
                break;
            case 6:
                return seg_bp + ((uint16)(cpu.bp + *(int8 *)ip++));
                break;
            case 7:
                return seg + ((uint16)(cpu.bx + *(int8 *)ip++));
                break;
            }
        }
        break;
    case 2:
        if (a32) {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.eax + *(uint32 *)((ip += 4) - 4)));
                break;
            case 1:
                return seg + ((uint16)(cpu.ecx + *(uint32 *)((ip += 4) - 4)));
                break;
            case 2:
                return seg + ((uint16)(cpu.edx + *(uint32 *)((ip += 4) - 4)));
                break;
            case 3:
                return seg + ((uint16)(cpu.ebx + *(uint32 *)((ip += 4) - 4)));
                break;
            case 4:
                i = sib();
                return seg + ((uint16)(i + *(uint32 *)((ip += 4) - 4)));
                break;
            case 5:
                return seg_bp + ((uint16)(cpu.ebp + *(uint32 *)((ip += 4) - 4)));
                break;
            case 6:
                return seg + ((uint16)(cpu.esi + *(uint32 *)((ip += 4) - 4)));
                break;
            case 7:
                return seg + ((uint16)(cpu.edi + *(uint32 *)((ip += 4) - 4)));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return seg + ((uint16)(cpu.bx + cpu.si + *(uint16 *)((ip += 2) - 2)));
                break;
            case 1:
                return seg + ((uint16)(cpu.bx + cpu.di + *(uint16 *)((ip += 2) - 2)));
                break;
            case 2:
                return seg_bp + ((uint16)(cpu.bp + cpu.si + *(uint16 *)((ip += 2) - 2)));
                break;
            case 3:
                return seg_bp + ((uint16)(cpu.bp + cpu.di + *(uint16 *)((ip += 2) - 2)));
                break;
            case 4:
                return seg + ((uint16)(cpu.si + *(uint16 *)((ip += 2) - 2)));
                break;
            case 5:
                return seg + ((uint16)(cpu.di + *(uint16 *)((ip += 2) - 2)));
                break;
            case 6:
                return seg_bp + ((uint16)(cpu.bp + *(uint16 *)((ip += 2) - 2)));
                break;
            case 7:
                return seg + ((uint16)(cpu.bx + *(uint16 *)((ip += 2) - 2)));
                break;
            }
        }
        break;
    }

    return nullptr;
}

uint16 *rm16() {
    static int32 i; // r/m byte
    i = *ip;
    switch (i >> 6) {
    case 3:
        ip++;
        return reg16[i & 7];
        break;
    case 0:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + cpu.ax);
                break;
            case 1:
                return (uint16 *)(seg + cpu.cx);
                break;
            case 2:
                return (uint16 *)(seg + cpu.dx);
                break;
            case 3:
                return (uint16 *)(seg + cpu.bx);
                break;
            case 4:
                return (uint16 *)(seg + (uint16)sib_mod0());
                break;
            case 5:
                return (uint16 *)(seg + (*(uint16 *)((ip += 4) - 4)));
                break;
            case 6:
                return (uint16 *)(seg + cpu.si);
                break;
            case 7:
                return (uint16 *)(seg + cpu.di);
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.si)));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.di)));
                break;
            case 2:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.si)));
                break;
            case 3:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.di)));
                break;
            case 4:
                return (uint16 *)(seg + cpu.si);
                break;
            case 5:
                return (uint16 *)(seg + cpu.di);
                break;
            case 6:
                return (uint16 *)(seg + (*(uint16 *)((ip += 2) - 2)));
                break;
            case 7:
                return (uint16 *)(seg + cpu.bx);
                break;
            }
        }
        break;
    case 1:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.eax + *(int8 *)ip++)));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.ecx + *(int8 *)ip++)));
                break;
            case 2:
                return (uint16 *)(seg + ((uint16)(cpu.edx + *(int8 *)ip++)));
                break;
            case 3:
                return (uint16 *)(seg + ((uint16)(cpu.ebx + *(int8 *)ip++)));
                break;
            case 4:
                i = sib();
                return (uint16 *)(seg + ((uint16)(i + *(int8 *)ip++)));
                break;
            case 5:
                return (uint16 *)(seg_bp + ((uint16)(cpu.ebp + *(int8 *)ip++)));
                break;
            case 6:
                return (uint16 *)(seg + ((uint16)(cpu.esi + *(int8 *)ip++)));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.edi + *(int8 *)ip++)));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.si + *(int8 *)ip++)));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.di + *(int8 *)ip++)));
                break;
            case 2:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(int8 *)ip++)));
                break;
            case 3:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(int8 *)ip++)));
                break;
            case 4:
                return (uint16 *)(seg + ((uint16)(cpu.si + *(int8 *)ip++)));
                break;
            case 5:
                return (uint16 *)(seg + ((uint16)(cpu.di + *(int8 *)ip++)));
                break;
            case 6:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + *(int8 *)ip++)));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.bx + *(int8 *)ip++)));
                break;
            }
        }
        break;
    case 2:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.eax + *(uint32 *)((ip += 4) - 4))));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.ecx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 2:
                return (uint16 *)(seg + ((uint16)(cpu.edx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 3:
                return (uint16 *)(seg + ((uint16)(cpu.ebx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 4:
                i = sib();
                return (uint16 *)(seg + ((uint16)(i + *(uint32 *)((ip += 4) - 4))));
                break;
            case 5:
                return (uint16 *)(seg_bp + ((uint16)(cpu.ebp + *(uint32 *)((ip += 4) - 4))));
                break;
            case 6:
                return (uint16 *)(seg + ((uint16)(cpu.esi + *(uint32 *)((ip += 4) - 4))));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.edi + *(uint32 *)((ip += 4) - 4))));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 1:
                return (uint16 *)(seg + ((uint16)(cpu.bx + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 2:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 3:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 4:
                return (uint16 *)(seg + ((uint16)(cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 5:
                return (uint16 *)(seg + ((uint16)(cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 6:
                return (uint16 *)(seg_bp + ((uint16)(cpu.bp + *(uint16 *)((ip += 2) - 2))));
                break;
            case 7:
                return (uint16 *)(seg + ((uint16)(cpu.bx + *(uint16 *)((ip += 2) - 2))));
                break;
            }
        }
        break;
    }

    return nullptr;
}

uint32 *rm32() {
    static int32 i; // r/m byte
    i = *ip;
    switch (i >> 6) {
    case 3:
        ip++;
        return reg32[i & 7];
        break;
    case 0:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + cpu.ax);
                break;
            case 1:
                return (uint32 *)(seg + cpu.cx);
                break;
            case 2:
                return (uint32 *)(seg + cpu.dx);
                break;
            case 3:
                return (uint32 *)(seg + cpu.bx);
                break;
            case 4:
                return (uint32 *)(seg + (uint16)sib_mod0());
                break;
            case 5:
                return (uint32 *)(seg + (*(uint16 *)((ip += 4) - 4)));
                break;
            case 6:
                return (uint32 *)(seg + cpu.si);
                break;
            case 7:
                return (uint32 *)(seg + cpu.di);
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.si)));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.di)));
                break;
            case 2:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.si)));
                break;
            case 3:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.di)));
                break;
            case 4:
                return (uint32 *)(seg + cpu.si);
                break;
            case 5:
                return (uint32 *)(seg + cpu.di);
                break;
            case 6:
                return (uint32 *)(seg + (*(uint16 *)((ip += 2) - 2)));
                break;
            case 7:
                return (uint32 *)(seg + cpu.bx);
                break;
            }
        }
        break;
    case 1:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.eax + *(int8 *)ip++)));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.ecx + *(int8 *)ip++)));
                break;
            case 2:
                return (uint32 *)(seg + ((uint16)(cpu.edx + *(int8 *)ip++)));
                break;
            case 3:
                return (uint32 *)(seg + ((uint16)(cpu.ebx + *(int8 *)ip++)));
                break;
            case 4:
                i = sib();
                return (uint32 *)(seg + ((uint16)(i + *(int8 *)ip++)));
                break;
            case 5:
                return (uint32 *)(seg_bp + ((uint16)(cpu.ebp + *(int8 *)ip++)));
                break;
            case 6:
                return (uint32 *)(seg + ((uint16)(cpu.esi + *(int8 *)ip++)));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.edi + *(int8 *)ip++)));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.si + *(int8 *)ip++)));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.di + *(int8 *)ip++)));
                break;
            case 2:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(int8 *)ip++)));
                break;
            case 3:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(int8 *)ip++)));
                break;
            case 4:
                return (uint32 *)(seg + ((uint16)(cpu.si + *(int8 *)ip++)));
                break;
            case 5:
                return (uint32 *)(seg + ((uint16)(cpu.di + *(int8 *)ip++)));
                break;
            case 6:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + *(int8 *)ip++)));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.bx + *(int8 *)ip++)));
                break;
            }
        }
        break;
    case 2:
        ip++;
        if (a32) {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.eax + *(uint32 *)((ip += 4) - 4))));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.ecx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 2:
                return (uint32 *)(seg + ((uint16)(cpu.edx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 3:
                return (uint32 *)(seg + ((uint16)(cpu.ebx + *(uint32 *)((ip += 4) - 4))));
                break;
            case 4:
                i = sib();
                return (uint32 *)(seg + ((uint16)(i + *(uint32 *)((ip += 4) - 4))));
                break;
            case 5:
                return (uint32 *)(seg_bp + ((uint16)(cpu.ebp + *(uint32 *)((ip += 4) - 4))));
                break;
            case 6:
                return (uint32 *)(seg + ((uint16)(cpu.esi + *(uint32 *)((ip += 4) - 4))));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.edi + *(uint32 *)((ip += 4) - 4))));
                break;
            }
        } else {
            switch (i & 7) {
            case 0:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 1:
                return (uint32 *)(seg + ((uint16)(cpu.bx + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 2:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 3:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 4:
                return (uint32 *)(seg + ((uint16)(cpu.si + *(uint16 *)((ip += 2) - 2))));
                break;
            case 5:
                return (uint32 *)(seg + ((uint16)(cpu.di + *(uint16 *)((ip += 2) - 2))));
                break;
            case 6:
                return (uint32 *)(seg_bp + ((uint16)(cpu.bp + *(uint16 *)((ip += 2) - 2))));
                break;
            case 7:
                return (uint32 *)(seg + ((uint16)(cpu.bx + *(uint16 *)((ip += 2) - 2))));
                break;
            }
        }
        break;
    }

    return nullptr;
}

uint8 *seg_es_ptr;
uint8 *seg_cs_ptr;
uint8 *seg_ss_ptr;
uint8 *seg_ds_ptr;
uint8 *seg_fs_ptr;
uint8 *seg_gs_ptr;

#define seg_es 0
#define seg_cs 1
#define seg_ss 2
#define seg_ds 3
#define seg_fs 4
#define seg_gs 5

#define op_r i & 7

void cpu_call() {

    static int32 i, i2, i3, x, x2, x3, y, y2, y3;
    static uint8 b, b2, b3;
    static uint8 *uint8p;
    static uint16 *uint16p;
    static uint32 *uint32p;
    static uint8 *dseg;
    static int32 r;
    ip = (uint8 *)&cmem[cpu.cs * 16 + cpu.ip];

    seg_es_ptr = (uint8 *)cmem + cpu.es * 16;
    seg_cs_ptr = (uint8 *)cmem + cpu.cs * 16;
    seg_ss_ptr = (uint8 *)cmem + cpu.ss * 16;
    seg_ds_ptr = (uint8 *)cmem + cpu.ds * 16;
    seg_fs_ptr = (uint8 *)cmem + cpu.fs * 16;
    seg_gs_ptr = (uint8 *)cmem + cpu.gs * 16;

next_opcode:
    b32 = 0;
    a32 = 0;
    seg = seg_ds_ptr;
    seg_bp = seg_ss_ptr;

    i = *ip++;

    // read any prefixes
    if (i == 0x66) {
        b32 = 1;
        i = *ip++;
    }
    if (i == 0x26) {
        seg_bp = seg = seg_es_ptr;
        i = *ip++;
    }
    if (i == 0x2E) {
        seg_bp = seg = seg_cs_ptr;
        i = *ip++;
    }
    if (i == 0x36) {
        seg = seg_ss_ptr;
        i = *ip++;
    }
    if (i == 0x3E) {
        seg_bp = seg_ds_ptr;
        i = *ip++;
    }
    if (i == 0x64) {
        seg_bp = seg = seg_fs_ptr;
        i = *ip++;
    }
    if (i == 0x65) {
        seg_bp = seg = seg_gs_ptr;
        i = *ip++;
    }
    if (i == 0x67) {
        a32 = 1;
        i = *ip++;
    }

    if (i == 0x0F)
        goto opcode_0F;

    r = *ip >> 3 & 7;

    // mov
    if (i != 0x8D) {
        if (i >= 0x88 && i <= 0x8E) {
            switch (i) {
            case 0x88: // /r r/m8,r8
                *rm8() = *reg8[r];
                break;
            case 0x89: // /r r/m16(32),r16(32)
                if (b32)
                    *rm32() = *reg32[r];
                else
                    *rm16() = *reg16[r];
                break;
            case 0x8A: // /r r8,r/m8
                *reg8[r] = *rm8();
                break;
            case 0x8B: // /r r16(32),r/m16(32)
                if (b32)
                    *reg32[r] = *rm32();
                else
                    *reg16[r] = *rm16();
                break;
            case 0x8C: // /r r/m16,Sreg
                *rm16() = *segreg[r];
                break;
            case 0x8E: // /r Sreg,r/m16
                *segreg[r] = *rm16();
                if (r == 0)
                    seg_es_ptr = (uint8 *)cmem + *segreg[r] * 16;
                // CS (r==1) cannot be set
                if (r == 2)
                    seg_ss_ptr = (uint8 *)cmem + *segreg[r] * 16;
                if (r == 3)
                    seg_ds_ptr = (uint8 *)cmem + *segreg[r] * 16;
                if (r == 4)
                    seg_fs_ptr = (uint8 *)cmem + *segreg[r] * 16;
                if (r == 5)
                    seg_gs_ptr = (uint8 *)cmem + *segreg[r] * 16;
                break;
            }
            goto done;
        }
    }
    if (i >= 0xA0 && i <= 0xA3) {
        switch (i) {
        case 0xA0: // al,moffs8
            cpu.al = *(seg + *(uint16 *)ip);
            ip += 2;
            break;
        case 0xA1: // (e)ax,moffs16(32)
            if (b32) {
                cpu.eax = *(uint32 *)(seg + *(uint16 *)ip);
                ip += 2;
            } else {
                cpu.ax = *(uint16 *)(seg + *(uint16 *)ip);
                ip += 2;
            }
            break;
        case 0xA2: // moffs8,al
            *(seg + *(uint16 *)ip) = cpu.al;
            ip += 2;
            break;
        case 0xA3: // moffs16(32),(e)ax
            if (b32) {
                *(uint32 *)(seg + *(uint16 *)ip) = cpu.eax;
                ip += 2;
            } else {
                *(uint16 *)(seg + *(uint16 *)ip) = cpu.ax;
                ip += 2;
            }
            break;
        }
        goto done;
    }
    if (i >= 0xB0 && i <= 0xB7) { // +rb reg8,imm8
        *reg8[op_r] = *ip++;
        goto done;
    }
    if (i >= 0xB8 && i <= 0xBF) { // +rw(rd) reg16(32),imm16(32)
        if (b32) {
            *reg32[op_r] = *(uint32 *)ip;
            ip += 4;
        } else {
            *reg16[op_r] = *(uint16 *)ip;
            ip += 2;
        }
        goto done;
    }
    if (i == 0xC6) { // r/m8,imm8
        uint8p = rm8();
        *uint8p = *ip++;
        goto done;
    }
    if (i == 0xC7) { // r/m16(32),imm16(32)
        if (b32) {
            uint32p = rm32();
            *uint32p = *(uint32 *)ip;
            ip += 4;
        } else {
            uint16p = rm16();
            *uint16p = *(uint16 *)ip;
            ip += 2;
        }
        goto done;
    }

    // RET instruction handler - Far return (0xCB) and Near return with imm16 (0xCA)
    // Note: These are stubbed implementations that assume return control
    // TODO: Proper stack management and IP/CS register updates for far returns
    if (i == 0xCB) { //(far)
        // assume return control (revise later)
        return;
    }
    if (i == 0xCA) { // imm16 (far)
        // assume return control (revise later)
        return;
    }

    // INT instruction handler - Software interrupt (0xCD)
    // Calls interrupt vector table, assumes table is at 0xFFFF
    // TODO: Implement proper interrupt vector table lookup and handling
    if (i == 0xCD) {
        call_int(*ip++); // assume interrupt table is 0xFFFF
        goto done;
    }

    // push
    if (i == 0xFF) {
        if (b32) {
            *((uint32 *)(seg_ss_ptr + (cpu.sp -= 4))) = *rm32();
        } else {
            *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *rm16();
        }
        goto done;
    }
    if (i >= 0x50 && i <= 0x57) { //+ /r r16(32)
        if (b32) {
            *((uint32 *)(seg_ss_ptr + (cpu.sp -= 4))) = *reg32[op_r];
        } else {
            *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *reg16[op_r];
        }
        goto done;
    }
    if (i == 0x6A) { // imm8 (sign extended to 16 bits)
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = ((int8)*ip++);
        goto done;
    }
    if (i == 0x68) { // imm16(32)
        if (b32) {
            *((uint32 *)(seg_ss_ptr + (cpu.sp -= 4))) = *(uint32 *)ip;
            ip += 4;
        } else {
            *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *(uint16 *)ip;
            ip += 2;
        }
        goto done;
    }
    if (i == 0x0E) { // CS
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_cs];
        goto done;
    }
    if (i == 0x16) { // SS
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_ss];
        goto done;
    }
    if (i == 0x1E) { // DS
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_ds];
        goto done;
    }
    if (i == 0x06) { // ES
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_es];
        goto done;
    }

    // pop
    if (i == 0x8F) {
        if (b32) {
            *rm32() = *((uint32 *)(seg_ss_ptr - 4 + (cpu.sp += 4)));
        } else {
            *rm16() = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        }
        goto done;
    }
    if (i >= 0x58 && i <= 0x5F) { //+rw(d) r16(32)
        if (b32) {
            *reg32[op_r] = *((uint32 *)(seg_ss_ptr - 4 + (cpu.sp += 4)));
        } else {
            *reg16[op_r] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        }
        goto done;
    }
    if (i == 0x1F) { // DS
        *segreg[seg_ds] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }
    if (i == 0x07) { // ES

        *segreg[seg_es] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }
    if (i == 0x17) { // SS
        *segreg[seg_ss] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }

    goto skip_0F_opcodes;
opcode_0F:
    i = *ip++;
    r = *ip >> 3 & 7; // required???

    // push
    if (i == 0xA0) {
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_fs];
        goto done;
    }
    if (i == 0xA8) {
        *((uint16 *)(seg_ss_ptr + (cpu.sp -= 2))) = *segreg[seg_gs];
        goto done;
    }

    // pop
    if (i == 0xA1) { // FS
        *segreg[seg_fs] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }
    if (i == 0xA9) { // GS
        *segreg[seg_gs] = *((uint16 *)(seg_ss_ptr - 2 + (cpu.sp += 2)));
        goto done;
    }

skip_0F_opcodes:

    i2 = ((i >> 4) & 15);
    if (i2 <= 9)
        i2 += 48;
    else
        i2 = i2 - 10 + 65;
    unknown_opcode_mess->chr[16] = i2;
    i2 = i & 15;
    if (i2 <= 9)
        i2 += 48;
    else
        i2 = i2 - 10 + 65;
    unknown_opcode_mess->chr[17] = i2;
    gui_alert((const char *)unknown_opcode_mess->chr, "X86 Error", "ok");
    exit(86);
done:
    if (*ip)
        goto next_opcode;

    exit(cmem[0]);
}

int32 screen_last_valid = 0;
uint8 *screen_last = (uint8 *)malloc(1);
uint32 screen_last_size = 1;

uint64 asciicode_value = 0;
int32 asciicode_reading = 0;

int32 lock_display = 0;
int32 lock_display_required = 0;

// cost delay, made obsolete by managing thread priorities (consider removal)
#define cost_limit 10000
#define cost_delay 0
uint32 cost = 0;

int64 build_int64(uint32 val2, uint32 val1) {
    static int64 val;
    val = val2;
    val <<= 32;
    val |= val1;
    return val;
}

uint64 build_uint64(uint32 val2, uint32 val1) {
    static uint64 val;
    val = val2;
    val <<= 32;
    val |= val1;
    return val;
}

// nb. abbreviations are used in variable names to save typing, here are some of the expansions
// cmem=conventional memory
// qbs=qbick basic string (refers to the emulation of quick basic strings)
// sp=stack pointer
// dblock=a 64K memory block in conventional memory holding single variables and strings
extern uint8 *cmem_static_pointer;
uint8 *cmem_static_base = &cmem[0] + 1280 + 65536;
extern uint8 *cmem_dynamic_base;
//[1280][DBLOCK][STATIC-><-DYNAMIC][A000-]

extern uint32 qbs_cmem_sp; //=256;
extern uint32 cmem_sp;     //=65536;
extern uint64 *nothingvalue;

uint8 wait_needed = 1;

int32 full_screen = 0;      // 0,1(stretched/closest),2(1:1)
int32 full_screen_set = -1; // 0(windowed),1(stretched/closest),2(1:1)

int32 vertical_retrace_in_progress = 0;
int32 vertical_retrace_happened = 0;

uint8 lock_subsystem = 0;

extern uint8 close_program; //=0;
uint8 program_wait = 0;

extern uint8 suspend_program;
extern uint8 stop_program;

int32 global_counter = 0;
extern double last_line;
void end(void);

void sub__echo(qbs *message);

void unlockvWatchHandle() {
    if (vwatch > 0)
        vwatch = -1;
}

int32 vWatchHandle() {
    return vwatch;
}

void sub__assert(int32 expression, qbs *assert_message, int32 passed) {
    if (asserts == 0)
        return;
    if (expression == 0) {
        if (console == 1 && passed == 1) {
            sub__echo(assert_message);
            error(315);
            return;
        }
        error(314);
    }
    return;
}

void end() {
    dont_call_sub_gl = 1;
    exit_ok |= 1;
    while (!stop_program)
        Sleep(16);
    while (1)
        Sleep(16);
}

int32 stop_program_state() {
    return stop_program;
}

// MEM_STATIC memory manager
/*
    mem_static uses a pointer called mem_static_pointer to allocate linear memory.
    It can also change mem_static_pointer back to a previous location, effectively erasing
    any memory after that point.
    Because memory cannot be guaranteed to be allocated in exactly the same location
    after realloc which QB64 requires to keep functionality of previous pointers when
    the current block of memory is full QB64 creates an entirely new block, much larger
    than the previous block (at least 2x), and "writes-off" the previous block as un-
    reclaimable memory. This tradeoff is worth the speed it recovers.
    This allocation strategy can be shown as follows: (X=1MB)
    X
    XX
    XXXX
    XXXXXXXX
    XXXXXXXXXXXXXXXX
    etc.
*/
uint32 mem_static_size;
extern uint8 *mem_static;
extern uint8 *mem_static_pointer;
extern uint8 *mem_static_limit;

/**
 * Allocates memory from the static memory pool.
 * Aligns allocations to 8-byte boundaries and expands the pool if necessary.
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory block
 */
uint8 *mem_static_malloc(uint32 size) {
    size += 7;
    size -= (size & 7); // align to 8 byte boundary
    if ((mem_static_pointer += size) < mem_static_limit)
        return mem_static_pointer - size;
    mem_static_size = (mem_static_size << 1) + size;
    mem_static = (uint8 *)malloc(mem_static_size);
    if (!mem_static)
        error(504);
    mem_static_pointer = mem_static + size;
    mem_static_limit = mem_static + mem_static_size;
    return mem_static_pointer - size;
}

/**
 * Restores the static memory pointer to a previous position.
 * Used for memory management cleanup and deallocation.
 * @param restore_point Pointer position to restore to
 */
void mem_static_restore(uint8 *restore_point) {
    if ((restore_point >= mem_static) && (restore_point <= mem_static_limit)) {
        mem_static_pointer = restore_point;
    } else {
        // if restore_point is not in the current block, use t=start of current block as a new base
        mem_static_pointer = mem_static;
    }
}

// CMEM_FAR_DYNAMIC memory manager
/*
    (uses a custom "links" based memory manager)
*/

/**
 * Structure representing a memory block in the dynamic memory manager.
 */
struct cmem_dynamic_link_type {
    uint8 *offset;           // Pointer to the start of the memory block
    uint8 *top;              // Pointer to the end/top of the memory block
    uint32 size;             // Size of the memory block in bytes
    uint32 i;                // Index of this link in the link array
    cmem_dynamic_link_type *next; // Pointer to the next memory block
};

// Dynamic memory manager global variables
cmem_dynamic_link_type cmem_dynamic_link[147136 + 1]; //+1 is added because array is used from index 1
cmem_dynamic_link_type *cmem_dynamic_link_first = NULL;
int32 cmem_dynamic_next_link = 0;
int32 cmem_dynamic_free_link = 0;
uint32 cmem_dynamic_free_list[147136];

/**
 * Allocates memory from the dynamic memory pool.
 * Uses a first-fit algorithm to find suitable space between existing blocks.
 * @param size Number of bytes to allocate (max 64KB)
 * @return Pointer to allocated memory block
 */
uint8 *cmem_dynamic_malloc(uint32 size) {
    static int32 i;
    static uint8 *top;
    static cmem_dynamic_link_type *link;
    static cmem_dynamic_link_type *newlink;
    static cmem_dynamic_link_type *prev_link;
    if (size > 65536)
        error(505); //>64K
    // to avoid mismatches between offsets, all 0-byte blocks are given the special offset A000h (the top of the heap)
    if (!size)
        return (&cmem[0] + 655360); // top of heap
    // forces blocks to be multiples of 16 bytes so they align with segment boundaries
    if (size & 15)
        size = size - (size & 15) + 16;
    // is a space large enough between existing blocks available?
    //(if not, memory will be allocated at bottom of heap)
    top = &cmem[0] + 655360; // top is the base of the higher block
    prev_link = NULL;
    if ((link = cmem_dynamic_link_first)) {
    cmem_dynamic_findspace:
        if ((top - link->top) >= size) { // gpf
            // found free space
            goto cmem_dynamic_make_new_link;
        }
        prev_link = link;
        top = link->offset; // set top to the base of current block for future comparisons
        if ((link = link->next))
            goto cmem_dynamic_findspace;
    }
    // no space between existing blocks is large enough, alloc below 'top'
    if ((top - cmem_static_pointer) < size)
        error(506); // a large enough block cannot be created!
    cmem_dynamic_base = top - size;
// get a new link index
cmem_dynamic_make_new_link:
    if (cmem_dynamic_free_link) {
        i = cmem_dynamic_free_list[cmem_dynamic_free_link--];
    } else {
        i = cmem_dynamic_next_link++;
        if (i >= 147136)
            error(507); // not enough blocks
    }
    newlink = (cmem_dynamic_link_type *)&cmem_dynamic_link[i];
    // set link info
    newlink->i = i;
    newlink->offset = top - size;
    newlink->size = size;
    newlink->top = top;
    // attach below prev_link
    if (prev_link) {
        newlink->next = prev_link->next; // NULL if none
        prev_link->next = newlink;
    } else {
        newlink->next = cmem_dynamic_link_first; // NULL if none
        cmem_dynamic_link_first = newlink;
    }
    return newlink->offset;
}

/**
 * Frees a previously allocated dynamic memory block.
 * Removes the block from the linked list and adds its link to the free list.
 * @param block Pointer to the memory block to free
 */
void cmem_dynamic_free(uint8 *block) {
    static cmem_dynamic_link_type *link;
    static cmem_dynamic_link_type *prev_link;
    if (!cmem_dynamic_link_first)
        return;
    if (!block)
        return;
    if (block == (&cmem[0] + 655360))
        return; // to avoid mismatches between offsets, all 0-byte blocks are given the special offset A000h (the top of the heap)
    prev_link = NULL;
    link = cmem_dynamic_link_first;
check_next:
    if (link->offset == block) {
        // unlink
        if (prev_link) {
            prev_link->next = link->next;
        } else {
            cmem_dynamic_link_first = link->next;
        }
        // free link
        cmem_dynamic_free_link++;
        cmem_dynamic_free_list[cmem_dynamic_free_link] = link->i;
        // memory freed successfully!
        return;
    }
    prev_link = link;
    if ((link = link->next))
        goto check_next;
    return;
}

// defseg, sub_defseg, func_peek, sub_poke moved to mem_legacy.cpp

int32 array_ok = 1; // kept to compile legacy versions

// gosub-return handling
extern uint32 next_return_point; //=0;
extern uint32 *return_point;     //=(uint32*)malloc(4*16384);
extern uint32 return_points;     //=16384;

void more_return_points() {
    if (return_points > 2147483647)
        error(256);
    return_points *= 2;
    return_point = (uint32 *)realloc(return_point, return_points * 4);
    if (return_point == NULL)
        error(256);
}

uint8 keyon[65536];

qbs *singlespace;

qbs *func_varptr_helper(uint8 type, uint16 offset) {
    //*creates a 3 byte string using the values given
    qbs *tqbs;
    tqbs = qbs_new(3, 1);
    tqbs->chr[0] = type;
    tqbs->chr[1] = offset & 255;
    tqbs->chr[2] = offset >> 8;
    return tqbs;
}

qbs *qbs_inkey() {
    if (is_error_pending())
        return qbs_new(0, 1);
    qbs *tqbs;
    // Sleep(0);
    tqbs = qbs_new(2, 1);
    if (cmem[0x41a] != cmem[0x41c]) {
        tqbs->chr[0] = cmem[0x400 + cmem[0x41a]];
        tqbs->chr[1] = cmem[0x400 + cmem[0x41a] + 1];
        if (tqbs->chr[0]) {
            tqbs->len = 1;
        } else {
            if (tqbs->chr[1] == 0)
                tqbs->len = 1;
        }
        cmem[0x41a] += 2;
        if (cmem[0x41a] == 62)
            cmem[0x41a] = 30;
    } else {
        tqbs->len = 0;
    }
    return tqbs;
}

void sub__keyclear(int32 buf, int32 passed) {
    if (is_error_pending())
        return;
    if (passed && (buf > 3 || buf < 1))
        error(5);
    //  Sleep(10);
    if ((buf == 1 && passed) || !passed) {
        // INKEY$ buffer
        cmem[0x41a] = 30;
        cmem[0x41b] = 0; // head
        cmem[0x41c] = 30;
        cmem[0x41d] = 0; // tail
    }
    if ((buf == 2 && passed) || !passed) {
        //_KEYHIT buffer
        keyhit_nextfree = 0;
        keyhit_next = 0;
    }
    if ((buf == 3 && passed) || !passed) {
        // INP(&H60) buffer
        port60h_events = 0;
    }
#ifdef QB64_WINDOWS
    // Windows Console Buffer
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
#endif
}

// QBG BLOCK
int32 qbg_mode = -1; //-1 means not initialized!
int32 qbg_text_only;
// text & graphics modes
int32 qbg_height_in_characters, qbg_width_in_characters;
int32 qbg_top_row, qbg_bottom_row;
int32 qbg_cursor_x, qbg_cursor_y;
int32 qbg_character_height, qbg_character_width;
uint32 qbg_color, qbg_background_color;
// text mode ONLY
int32 qbg_cursor_show;
int32 qbg_cursor_firstvalue, qbg_cursor_lastvalue; // these values need revision
// graphics modes ONLY
int32 qbg_width, qbg_height;
float qbg_x, qbg_y;
int32 qbg_bits_per_pixel, qbg_pixel_mask; // for monochrome modes 1b, for 16 color 1111b, for 256 color 11111111b
int32 qbg_bytes_per_pixel;
int32 qbg_clipping_or_scaling; // 1=clipping, 2=clipping and scaling
int32 qbg_view_x1, qbg_view_y1, qbg_view_x2, qbg_view_y2;
int32 qbg_view_offset_x, qbg_view_offset_y;
float qbg_scaling_x, qbg_scaling_y;
float qbg_scaling_offset_x, qbg_scaling_offset_y;
float qbg_window_x1, qbg_window_y1, qbg_window_x2, qbg_window_y2;
int32 qbg_pages;
uint32 *qbg_pageoffsets;
int32 *qbg_cursor_x_previous; // used to recover old cursor position
int32 *qbg_cursor_y_previous;
int32 qbg_active_page;
uint8 *qbg_active_page_offset;
int32 qbg_visual_page;
uint8 *qbg_visual_page_offset;
int32 qbg_color_assign[256]; // for modes with quasi palettes!
uint32 pal_mode10[2][9];

uint8 charset8x8[256][8][8];
uint8 charset8x16[256][16][8];

int32 lineclip_draw; // 1=draw, 0=don't draw
int32 lineclip_x1, lineclip_y1, lineclip_x2, lineclip_y2;
int32 lineclip_skippixels; // the number of pixels from x1,y1 which won't be drawn

void lineclip(int32 x1, int32 y1, int32 x2, int32 y2, int32 xmin, int32 ymin, int32 xmax, int32 ymax) {
    static double mx, my, y, x, d;
    static int32 xdis, ydis;
    lineclip_skippixels = 0;

    if (x1 >= xmin) {
        if (x1 <= xmax) {
            if (y1 >= ymin) {
                if (y1 <= ymax) { //(x1,y1) onscreen?
                    if (x1 == x2)
                        if (y1 == y2)
                            goto singlepoint; // is it a single point? (needed to avoid "division by 0" errors)
                    goto gotx1y1;
                }
            }
        }
    }

    //(x1,y1) offscreen...

    if (x1 == x2)
        if (y1 == y2) {
            lineclip_draw = 0;
            return;
        } // offscreen single point

    // ignore entirely offscreen lines requiring no further calculations
    if (x1 < xmin)
        if (x2 < xmin) {
            lineclip_draw = 0;
            return;
        }
    if (x1 > xmax)
        if (x2 > xmax) {
            lineclip_draw = 0;
            return;
        }
    if (y1 < ymin)
        if (y2 < ymin) {
            lineclip_draw = 0;
            return;
        }
    if (y1 > ymax)
        if (y2 > ymax) {
            lineclip_draw = 0;
            return;
        }

    mx = (x2 - x1) / std::fabs((double)(y2 - y1));
    my = (y2 - y1) / std::fabs((double)(x2 - x1));
    // right wall from right
    if (x1 > xmax) {
        if (mx < 0) {
            y = (double)y1 + ((double)x1 - (double)xmax) * my;
            if (y >= ymin) {
                if (y <= ymax) {
                    // double space indented values calculate pixels to skip
                    xdis = x1;
                    ydis = y1;
                    x1 = xmax;
                    y1 = qbr_float_to_long(y);
                    xdis = abs(xdis - x1);
                    ydis = abs(ydis - y1);
                    if (xdis >= ydis)
                        lineclip_skippixels = xdis;
                    else
                        lineclip_skippixels = ydis;
                    goto gotx1y1;
                }
            }
        }
    }
    // left wall from left
    if (x1 < xmin) {
        if (mx > 0) {
            y = (double)y1 + ((double)xmin - (double)x1) * my;
            if (y >= ymin) {
                if (y <= ymax) {
                    // double space indented values calculate pixels to skip
                    xdis = x1;
                    ydis = y1;
                    x1 = xmin;
                    y1 = qbr_float_to_long(y);
                    xdis = abs(xdis - x1);
                    ydis = abs(ydis - y1);

                    if (xdis >= ydis)
                        lineclip_skippixels = xdis;
                    else
                        lineclip_skippixels = ydis;
                    goto gotx1y1;
                }
            }
        }
    }
    // top wall from top
    if (y1 < ymin) {
        if (my > 0) {
            x = (double)x1 + ((double)ymin - (double)y1) * mx;
            if (x >= xmin) {
                if (x <= xmax) {
                    // double space indented values calculate pixels to skip
                    xdis = x1;
                    ydis = y1;
                    x1 = qbr_float_to_long(x);
                    y1 = ymin;
                    xdis = abs(xdis - x1);
                    ydis = abs(ydis - y1);
                    if (xdis >= ydis)
                        lineclip_skippixels = xdis;
                    else
                        lineclip_skippixels = ydis;
                    goto gotx1y1;
                }
            }
        }
    }
    // bottom wall from bottom
    if (y1 > ymax) {
        if (my < 0) {
            x = (double)x1 + ((double)y1 - (double)ymax) * mx;
            if (x >= xmin) {
                if (x <= xmax) {
                    // double space indented values calculate pixels to skip
                    xdis = x1;
                    ydis = y1;
                    x1 = qbr_float_to_long(x);
                    y1 = ymax;
                    xdis = abs(xdis - x1);
                    ydis = abs(ydis - y1);
                    if (xdis >= ydis)
                        lineclip_skippixels = xdis;
                    else
                        lineclip_skippixels = ydis;
                    goto gotx1y1;
                }
            }
        }
    }
    lineclip_draw = 0;
    return;
gotx1y1:

    if (x2 >= xmin) {
        if (x2 <= xmax) {
            if (y2 >= ymin) {
                if (y2 <= ymax) {
                    goto gotx2y2;
                }
            }
        }
    }

    mx = (x1 - x2) / std::fabs((double)(y1 - y2));
    my = (y1 - y2) / std::fabs((double)(x1 - x2));
    // right wall from right
    if (x2 > xmax) {
        if (mx < 0) {
            y = (double)y2 + ((double)x2 - (double)xmax) * my;
            if (y >= ymin) {
                if (y <= ymax) {
                    x2 = xmax;
                    y2 = qbr_float_to_long(y);
                    goto gotx2y2;
                }
            }
        }
    }
    // left wall from left
    if (x2 < xmin) {
        if (mx > 0) {
            y = (double)y2 + ((double)xmin - (double)x2) * my;
            if (y >= ymin) {
                if (y <= ymax) {
                    x2 = xmin;
                    y2 = qbr_float_to_long(y);
                    goto gotx2y2;
                }
            }
        }
    }
    // top wall from top
    if (y2 < ymin) {
        if (my > 0) {
            x = (double)x2 + ((double)ymin - (double)y2) * mx;
            if (x >= xmin) {
                if (x <= xmax) {
                    x2 = qbr_float_to_long(x);
                    y2 = ymin;
                    goto gotx2y2;
                }
            }
        }
    }
    // bottom wall from bottom
    if (y2 > ymax) {
        if (my < 0) {
            x = (double)x2 + ((double)y2 - (double)ymax) * mx;
            if (x >= xmin) {
                if (x <= xmax) {
                    x2 = qbr_float_to_long(x);
                    y2 = ymax;
                    goto gotx2y2;
                }
            }
        }
    }
    lineclip_draw = 0;
    return;
gotx2y2:
singlepoint:
    lineclip_draw = 1;
    lineclip_x1 = x1;
    lineclip_y1 = y1;
    lineclip_x2 = x2;
    lineclip_y2 = y2;

    return;
}

void qbg_palette(uint32 attribute, uint32 col, int32 passed) {
    static int32 r, g, b;
    if (is_error_pending())
        return;
    if (!passed) {
        restorepalette(write_page);
        return;
    }

    // 32-bit
    if (write_page->bytes_per_pixel == 4)
        goto error;

    attribute &= 255; // patch to support QBASIC overflow "bug"

    if ((write_page->compatible_mode == 13) || (write_page->compatible_mode == 256)) {
        if (col & 0xFFC0C0C0)
            goto error; // 11111111110000001100000011000000b
        r = col & 63;
        g = (col >> 8) & 63;
        b = (col >> 16) & 63;
        r = qbr((double)r * 4.063492f - 0.4999999f);
        g = qbr((double)g * 4.063492f - 0.4999999f);
        b = qbr((double)b * 4.063492f - 0.4999999f);
        write_page->pal[attribute] = b + g * 256 + r * 65536;
        // Upgraded from (((col<<2)&0xFF)<<16)+(((col>>6)&0xFF)<<8)+((col>>14)&0xFF)
        return;
    }

    if (write_page->compatible_mode == 12) {
        if (attribute > 15)
            goto error;
        if (col & 0xFFC0C0C0)
            goto error; // 11111111110000001100000011000000b
        r = col & 63;
        g = (col >> 8) & 63;
        b = (col >> 16) & 63;
        r = qbr((double)r * 4.063492f - 0.4999999f);
        g = qbr((double)g * 4.063492f - 0.4999999f);
        b = qbr((double)b * 4.063492f - 0.4999999f);
        write_page->pal[attribute] = b + g * 256 + r * 65536;
        return;
    }

    if (write_page->compatible_mode == 11) {
        if (attribute > 1)
            goto error;
        if (col & 0xFFC0C0C0)
            goto error; // 11111111110000001100000011000000b
        r = col & 63;
        g = (col >> 8) & 63;
        b = (col >> 16) & 63;
        r = qbr((double)r * 4.063492f - 0.4999999f);
        g = qbr((double)g * 4.063492f - 0.4999999f);
        b = qbr((double)b * 4.063492f - 0.4999999f);
        write_page->pal[attribute] = b + g * 256 + r * 65536;
        return;
    }

    if (write_page->compatible_mode == 10) {
        if (attribute > 3)
            goto error;
        if ((col < 0) || (col > 8))
            goto error;
        write_page->pal[attribute + 4] = col;
        return;
    }

    if (write_page->compatible_mode == 9) {
        if (attribute > 15)
            goto error;
        if ((col < 0) || (col > 63))
            goto error;
        write_page->pal[attribute] = palette_64[col];
        return;
    }

    if (write_page->compatible_mode == 8) {
        if (attribute > 15)
            goto error;
        if ((col < 0) || (col > 15))
            goto error;
        write_page->pal[attribute] = palette_256[col];
        return;
    }

    if (write_page->compatible_mode == 7) {
        if (attribute > 15)
            goto error;
        if ((col < 0) || (col > 15))
            goto error;
        write_page->pal[attribute] = palette_256[col];
        return;
    }

    if (write_page->compatible_mode == 2) {
        if (attribute > 1)
            goto error;
        if ((col < 0) || (col > 15))
            goto error;
        write_page->pal[attribute] = palette_256[col];
        return;
    }

    if (write_page->compatible_mode == 1) {
        if (attribute > 15)
            goto error;
        if ((col < 0) || (col > 15))
            goto error;
        write_page->pal[attribute] = palette_256[col];
        return;
    }

    if (write_page->compatible_mode == 0) {
        if (attribute > 15)
            goto error;
        if ((col < 0) || (col > 63))
            goto error;
        write_page->pal[attribute] = palette_64[col];
        return;
    }

error:
    error(5);
    return;
}

void qbg_sub_color(uint32 col1, uint32 col2, uint32 bordercolor, int32 passed) {
    if (is_error_pending())
        return;
    if (!passed) {
        // performs no action if nothing passed (as in QBASIC for some modes)
        return;
    }

    if (write_page->console) {
#ifdef QB64_WINDOWS
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        int color = col2 * 16 + col1;
        SetConsoleTextAttribute(output, color);
#else
        // Exactly how the colour is rendered depends on your terminal emulator and
        // colour palette. Themes and user-customisation aside, the first 16 colours
        // line up with the old VGA colour scheme.
        // Most terminal emulators can handle 8 bit colour, see
        // https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit for the 8 bit colour palette.
        if ((passed & 1 && (col1 > 255 || col1 < 0)) || (passed & 2 && (col2 > 255 || col2 < 0)))
            goto error;

        if (passed & 1)
            printf("\033[38;5;%dm", col1);

        if (passed & 2)
            printf("\033[48;5;%dm", col2);
#endif
        return;
    }

    if (write_page->compatible_mode == 32) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->background_color = col2;
        return;
    }
    if (write_page->compatible_mode == 256) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 255)
                goto error;
        if (passed & 2)
            if (col2 > 255)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->background_color = col2;
        return;
    }
    if (write_page->compatible_mode == 13) {
        // if (passed&6) goto error;
        // if (col1>255) goto error;
        // write_page->color=col1;
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 255)
                goto error;
        if (passed & 2)
            if (col2 > 255)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->background_color = col2;
        return;
    }
    if (write_page->compatible_mode == 12) {
        // if (passed&6) goto error;
        // if (col1>15) goto error;
        // write_page->color=col1;
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 15)
                goto error;
        if (passed & 2)
            if (col2 > 15)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->background_color = col2;
        return;
    }
    if (write_page->compatible_mode == 11) {
        // if (passed&6) goto error;
        // if (col1>1) goto error;
        // write_page->color=col1;
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 1)
                goto error;
        if (passed & 2)
            if (col2 > 1)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->background_color = col2;
        return;
    }
    if (write_page->compatible_mode == 10) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 3)
                goto error;
        if (passed & 2)
            if (col2 > 8)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        // if (passed&2) ..._color_assign[0]=col2;
        if (passed & 2)
            write_page->pal[4] = col2;
        return;
    }
    if (write_page->compatible_mode == 9) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 15)
                goto error;
        if (passed & 2)
            if (col2 > 63)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->pal[0] = palette_64[col2];
        return;
    }
    if (write_page->compatible_mode == 8) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 15)
                goto error;
        if (passed & 2)
            if (col2 > 15)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->pal[0] = palette_256[col2];
        return;
    }
    if (write_page->compatible_mode == 7) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 15)
                goto error;
        if (passed & 2)
            if (col2 > 15)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->pal[0] = palette_256[col2];
        return;
    }
    if (write_page->compatible_mode == 2) {
        if (passed & 4)
            goto error;
        if (passed & 1)
            if (col1 > 1)
                goto error;
        if (passed & 2)
            if (col2 > 15)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->pal[0] = palette_256[col2];
        return;
    }
    if (write_page->compatible_mode == 1) {
        if (passed & 4)
            goto error;
        if (passed & 1) {
            if (col1 > 15)
                goto error;
            write_page->pal[0] = palette_256[col1];
        }
        if (passed & 2) {
            if (col2 & 1) {
                write_page->pal[1] = palette_256[3];
                write_page->pal[2] = palette_256[5];
                write_page->pal[3] = palette_256[7];
            } else {
                write_page->pal[1] = palette_256[2];
                write_page->pal[2] = palette_256[4];
                write_page->pal[3] = palette_256[6];
            }
        }
        return;
    }
    if (write_page->compatible_mode == 0) {
        if (passed & 1)
            if (col1 > 31)
                goto error;
        if (passed & 2)
            if (col2 > 15)
                goto error;
        if (passed & 1)
            write_page->color = col1;
        if (passed & 2)
            write_page->background_color = col2 & 7;
        return;
    }
error:
    error(5);
    return;
}

void defaultcolors() {
    write_page->color = 15;
    write_page->background_color = 0;
    if (write_page->compatible_mode == 0) {
        write_page->color = 7;
        write_page->background_color = 0;
    }
    if (write_page->compatible_mode == 1) {
        write_page->color = 3;
        write_page->background_color = 0;
    }
    if (write_page->compatible_mode == 2) {
        write_page->color = 1;
        write_page->background_color = 0;
    }
    if (write_page->compatible_mode == 10) {
        write_page->color = 3;
        write_page->background_color = 0;
    }
    if (write_page->compatible_mode == 11) {
        write_page->color = 1;
        write_page->background_color = 0;
    }
    if (write_page->compatible_mode == 32) {
        write_page->color = 0xFFFFFFFF;
        write_page->background_color = 0xFF000000;
    }
    write_page->draw_color = write_page->color;
    return;
}

// Note: Cannot be used to setup page 0, just to validate it
void validatepage(int32 n) {
    static int32 i, i2;
    // add new page indexes if necessary
    if (n >= pages) {
        i = n + 1;
        page = (int32 *)realloc(page, i * 4);
        memset(page + pages, 0, (i - pages) * 4);
        pages = i;
    }
    // create page at index n if none exists
    if (!page[n]) {
        // graphical (assumed)
        i = page[0];
        i2 = imgnew(img[i].width, img[i].height, img[i].compatible_mode);
        // modify based on page 0's attributes
        // i. link palette to page 0's palette (if necessary)
        if (img[i2].bytes_per_pixel != 4) {
            free(img[i2].pal);
            img[i2].flags ^= IMG_FREEPAL;
            img[i2].pal = img[i].pal;
        }
        // ii. set flags
        img[i2].flags |= IMG_SCREEN;
        // iii. inherit font
        selectfont(img[i].font, &img[i2]);
        // text
        //...
        page[n] = i2;
    }
    return;
} // validate_page

void qbg_screen(int32 mode, int32 color_switch, int32 active_page, int32 visual_page, int32 refresh, int32 passed) {
    if (is_error_pending())
        return;

    if (width8050switch) {
        if ((passed != 1) || mode)
            width8050switch = 0;
    }

    static int32 i, i2, i3, x, y, f, p;
    static img_struct *im;
    static int32 prev_width_in_characters, prev_height_in_characters;

    static int32 last_active_page = 0; // used for active page settings migration

    i = 0; // update flags
    // 1=mode change required
    // 2=page change required (used only to see if an early exit without locking is possible)

    i2 = page[0];
    if (passed & 1) {   // mode
        if (mode < 0) { // custom screen
            i3 = -mode;
            if (i3 >= nextimg) {
                error(258);
                return;
            } // within valid range?
            if (!img[i3].valid) {
                error(258);
                return;
            } // valid?
            if (i3 != i2)
                i = 1; // is mode changing?
        } else {
            if (mode == 3)
                goto error;
            if (mode == 4)
                goto error;
            if (mode == 5)
                goto error;
            if (mode == 6)
                goto error;
            if (mode > 13)
                goto error;
            // is mode changing?
            if (i2) {
                if (img[i2].compatible_mode != mode)
                    i = 1;
            } else
                i = 1;
            // force update if special parameters passed
            //(at present, only SCREEN 0 is ever called with these overrides, so handling
            // of these is done only in the SCREEN 0 section of the SCREEN sub)
            if ((sub_screen_width_in_characters != -1) || (sub_screen_height_in_characters != -1) || (sub_screen_font != -1))
                i = 1;
        }
    }

    if (passed & 4) { // active page
        if (active_page < 0)
            goto error;
        if (!(passed & 8)) { // if visual page not specified, set it to the active page
            passed |= 8;
            visual_page = active_page;
        }
        if (!(i & 1)) { // mode not changing
            // validate the passed active page, then see if it is the currently selected page
            validatepage(active_page);
            i2 = page[active_page];
            if ((i2 != read_page_index) || (i2 != write_page_index))
                i |= 2;
        }
    } // passed&4

    if (passed & 8) { // visual page
        i3 = visual_page;
        if (i3 < 0)
            goto error;
        if (!(i & 1)) { // mode not changing
            validatepage(visual_page);
            i2 = page[visual_page];
            if (i2 != display_page_index)
                i |= 2;
        }
    } // passed&8

    // if no changes need to be made exit before locking
    if (!i)
        return;

    if (autodisplay) {
        if (lock_display_required) { // on init of main (), attempting a lock would create an infinite loop
            if (i & 1) {             // avoid locking when only changing the screen page
                if (lock_display == 0)
                    lock_display = 1; // request lock
                while (lock_display != 2) {
                    Sleep(0);
                }
            }
        }
    }

    screen_last_valid = 0; // ignore cache used to update the screen on next update

    if (passed & 1) { // mode
        if (i & 1) {  // mode change necessary

            // calculate previous width & height if possible
            prev_width_in_characters = 0;
            prev_height_in_characters = 0;
            if ((i = page[0])) { // currently in a screen mode?
                im = &img[i];
                if (!im->compatible_mode) {
                    prev_width_in_characters = im->width;
                    prev_height_in_characters = im->height;
                } else {
                    x = fontwidth[im->font];
                    if (!x)
                        x = 1;
                    prev_width_in_characters = im->width / x;
                    prev_height_in_characters = im->height / fontheight[im->font];
                }
            } // currently in a screen mode

            // free any previously allocated surfaces
            // free pages in reverse order
            if (page[0]) { // currently in a screen mode?
                for (i = 1; i < pages; i++) {
                    if ((i2 = page[i])) {
                        // manual delete, freeing video pages is usually illegal
                        if (img[i2].flags & IMG_FREEMEM)
                            free(img[i2].offset); // free pixel data
                        freeimg(i2);
                    } // i2
                } // i
                i = page[0];
                if (sub_screen_keep_page0) {
                    img[i].flags ^= IMG_SCREEN;
                } else {
                    if (img[i].flags & IMG_FREEMEM)
                        free(img[i].offset); // free pixel data
                    if (img[i].flags & IMG_FREEPAL)
                        free(img[i].pal); // free palette
                    freeimg(i);
                }
            } // currently in a screen mode
            sub_screen_keep_page0 = 0; // reset to default status

            pages = 1;
            page[0] = 0;

            if (mode < 0) { // custom screen
                i = -mode;
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
                sub_screen_keep_page0 = 1;
            }

            // 320 x 200 graphics
            // 40 x 25 text format, character box size of 8 x 8
            // Assignment of up to 256K colors to up to 256 attributes
            if (mode == 13) {
                i = imgframe(&cmem[655360], 320, 200, 13);
                memset(img[i].offset, 0, 320 * 200);
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 13

            // 640 x 480 graphics
            // 80 x 30 or 80 x 60 text format, character box size of 8 x 16 or 8 x 8
            // Assignment of up to 256K colors to 16 attributes
            if (mode == 12) {
                i = imgnew(640, 480, 12);
                if ((prev_width_in_characters == 80) && (prev_height_in_characters == 60))
                    selectfont(8, &img[i]); // override default font
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 12

            /*
                Screen 11
                \A6 640 x 480 graphics
                \A6 80 x 30 or 80 x 60 text format, character box size of 8 x 16 or 8 x 8
                \A6 Assignment of up to 256K colors to 2 attributes
            */
            if (mode == 11) {
                i = imgnew(640, 480, 11);
                if ((prev_width_in_characters == 80) && (prev_height_in_characters == 60))
                    selectfont(8, &img[i]); // override default font
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 11

            // SCREEN 10: 640 x 350 graphics, monochrome monitor only
            //  \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
            //  \A6 128K page size, page range is 0 (128K) or 0-1 (256K)
            //  \A6 Up to 9 pseudocolors assigned to 4 attributes
            /*
                'colors swap every half second!
                'using PALETTE does NOT swap color indexes
                '0 black-black
                '1 black-grey
                '2 black-white
                '3 grey-black
                '4 grey-grey
                '5 grey-white
                '6 white-black
                '7 white-grey
                '8 white-white
                '*IMPORTANT* QB sets initial values up different to default palette!
                '0 block-black(0)
                '1 grey-grey(4)
                '2 white-black(6)
                '3 white-white(8)
            */
            if (mode == 10) {
                i = imgnew(640, 350, 10);
                if ((prev_width_in_characters == 80) && (prev_height_in_characters == 43))
                    selectfont(8, &img[i]); // override default font
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];

            } // 10

            /*
                SCREEN 9: 640 x 350 graphics
                \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
                \A6 64K page size, page range is 0 (64K);
                128K page size, page range is 0 (128K) or 0-1 (256K)
                \A6 16 colors assigned to 4 attributes (64K adapter memory), or
                64 colors assigned to 16 attributes (more than 64K adapter memory)
            */
            if (mode == 9) {
                i = imgnew(640, 350, 9);
                if ((prev_width_in_characters == 80) && (prev_height_in_characters == 43))
                    selectfont(8, &img[i]); // override default font
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 9

            /*
                SCREEN 8: 640 x 200 graphics
                \A6 80 x 25 text format, 8 x 8 character box
                \A6 64K page size, page ranges are 0 (64K), 0-1 (128K), or 0-3 (246K)
                \A6 Assignment of 16 colors to any of 16 attributes
            */
            if (mode == 8) {
                i = imgnew(640, 200, 8);
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 8

            /*
                SCREEN 7: 320 x 200 graphics
                \A6 40 x 25 text format, character box size 8 x 8
                \A6 32K page size, page ranges are 0-1 (64K), 0-3 (128K), or 0-7 (256K)
                \A6 Assignment of 16 colors to any of 16 attributes
            */
            if (mode == 7) {
                i = imgnew(320, 200, 7);
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 7

            /*
                SCREEN 4:
                \A6 Supports Olivetti (R) Personal Computers models M24, M240, M28,
                M280, M380, M380/C, M380/T and AT&T (R) Personal Computers 6300
                series
                \A6 640 x 400 graphics
                \A6 80 x 25 text format, 8 x 16 character box
                \A6 1 of 16 colors assigned as the foreground color (selected by the
                COLOR statement); background is fixed at black.
            */
            // Note: QB64 will not support SCREEN 4

            /*
                SCREEN 3: Hercules adapter required, monochrome monitor only
                \A6 720 x 348 graphics
                \A6 80 x 25 text format, 9 x 14 character box
                \A6 2 screen pages (1 only if a second display adapter is installed)
                \A6 PALETTE statement not supported
            */
            // Note: QB64 will not support SCREEN 3

            /*
                SCREEN 2: 640 x 200 graphics
                \A6 80 x 25 text format with character box size of 8 x 8
                \A6 16 colors assigned to 2 attributes with EGA or VGA
            */
            if (mode == 2) {
                i = imgnew(640, 200, 2);
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 2

            /*
                SCREEN 1: 320 x 200 graphics
                \A6 40 x 25 text format, 8 x 8 character box
                \A6 16 background colors and one of two sets of 3 foreground colors assigned
                using COLOR statement with CGA
                \A6 16 colors assigned to 4 attributes with EGA or VGA
            */
            if (mode == 1) {
                i = imgnew(320, 200, 1);
                page[0] = i;
                img[i].flags |= IMG_SCREEN;
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 1

            /*
                MDPA, CGA, EGA, or VGA Adapter Boards
                SCREEN 0: Text mode only
                \A6 Either 40 x 25, 40 x 43, 40 x 50, 80 x 25, 80 x 43, or 80 x 50 text format
                with 8 x 8 character box size (8 x 14, 9 x 14, or 9 x 16 with EGA or VGA)
                \A6 16 colors assigned to 2 attributes
                \A6 16 colors assigned to any of 16 attributes (with CGA or EGA)
                \A6 64 colors assigned to any of 16 attributes (with EGA or VGA)
            */
            /*
                granularity from &HB800
                4096 in 80x25
                2048 in 40x25
                6880 in 80x43 (80x43x2=6880)
                3440 in 40x43 (40x43x2=3440)
                8000 in 80x50 (80x50x2=8000)
                4000 in 40x50 (40x50x2=4000)
            */
            if (mode == 0) {

                if ((sub_screen_width_in_characters != -1) && (sub_screen_height_in_characters != -1) && (sub_screen_font != -1)) {
                    x = sub_screen_width_in_characters;
                    y = sub_screen_height_in_characters;
                    f = sub_screen_font;
                    sub_screen_width_in_characters = -1;
                    sub_screen_height_in_characters = -1;
                    sub_screen_font = -1;
                    goto gotwidth;
                }
                if (sub_screen_width_in_characters != -1) {
                    x = sub_screen_width_in_characters;
                    sub_screen_width_in_characters = -1;
                    y = 25;
                    f = 16; // default
                    if (prev_height_in_characters == 43) {
                        y = 43;
                        f = 14;
                    }
                    if (prev_height_in_characters == 50) {
                        y = 50;
                        f = 8;
                    }
                    if (x == 40)
                        f++;
                    goto gotwidth;
                }
                if (sub_screen_height_in_characters != -1) {
                    y = sub_screen_height_in_characters;
                    sub_screen_height_in_characters = -1;
                    f = 16; // default
                    if (y == 43)
                        f = 14;
                    if (y == 50)
                        f = 8;
                    x = 80; // default
                    if (prev_width_in_characters == 40) {
                        f++;
                        x = 40;
                    }
                    goto gotwidth;
                }

                if ((prev_width_in_characters == 80) && (prev_height_in_characters == 50)) {
                    x = 80;
                    y = 50;
                    f = 8;
                    goto gotwidth;
                }
                if ((prev_width_in_characters == 40) && (prev_height_in_characters == 50)) {
                    x = 40;
                    y = 50;
                    f = 8 + 1;
                    goto gotwidth;
                }
                if ((prev_width_in_characters == 80) && (prev_height_in_characters == 43)) {
                    x = 80;
                    y = 43;
                    f = 8;
                    goto gotwidth;
                }
                if ((prev_width_in_characters == 40) && (prev_height_in_characters == 43)) {
                    x = 40;
                    y = 43;
                    f = 8 + 1;
                    goto gotwidth;
                }
                if ((prev_width_in_characters == 40) && (prev_height_in_characters == 25)) {
                    x = 40;
                    y = 25;
                    f = 16 + 1;
                    goto gotwidth;
                }
                x = 80;
                y = 25;
                f = 16;
            gotwidth:;
                i2 = x * y * 2; // default granularity
                // specific granularities which cannot be calculated
                if ((x == 40) && (y == 25) && (f = (16 + 1)))
                    i2 = 2048;
                if ((x == 80) && (y == 25) && (f = 16))
                    i2 = 4096;
                p = 65536 / i2; // number of pages to allocate in cmem
                if (p > 8)
                    p = 8; // limit cmem pages to 8
                // make sure 8 page indexes exist
                if (7 >= pages) {
                    i = 7 + 1;
                    page = (int32 *)realloc(page, i * 4);
                    memset(page + pages, 0, (i - pages) * 4);
                    pages = i;
                }
                for (i3 = 0; i3 < 8; i3++) {
                    if (i3 < p) {
                        i = imgframe(&cmem[753664 + i2 * i3], x, y, 0);
                    } else {
                        i = imgnew(x, y, 0);
                    }
                    selectfont(f, &img[i]);
                    img[i].flags |= IMG_SCREEN;
                    page[i3] = i;
                }
                // text-clear 64K after seg. &HB800
                for (i = 0; i < 65536; i += 2) {
                    cmem[753664 + i] = 32;
                    cmem[753664 + i + 1] = 7;
                } // init. 64K of memory after B800
                i = page[0];
                display_page_index = i;
                display_page = &img[i];
                write_page_index = i;
                write_page = &img[i];
                read_page_index = i;
                read_page = &img[i];
            } // 0

            write_page->draw_ta = 0.0;
            write_page->draw_scale = 1.0; // reset DRAW attributes (of write_page)

            last_active_page = 0;

            key_display_redraw = 1;
            key_update();

        } // setmode
    } // passed MODE

    // note: changing the active or visual page reselects the default colors!
    if (passed & 4) { // SCREEN ?,?,X,? (active_page)
        i = active_page;
        validatepage(i);
        i = page[i];
        if ((write_page_index != i) || (read_page_index != i)) {
            write_page_index = i;
            write_page = &img[i];
            read_page_index = i;
            read_page = &img[i];
            defaultcolors();
            // reset VIEW PRINT state
            write_page->top_row = 1;
            if (!write_page->text)
                write_page->bottom_row = (write_page->height / write_page->font);
            else
                write_page->bottom_row = write_page->height;
            if (write_page->cursor_y > write_page->bottom_row)
                write_page->cursor_y = 1;
            write_page->bottom_row--;
            if (write_page->bottom_row <= 0)
                write_page->bottom_row = 1;

            // active page migration
            // note: transfers any screen settings which are maintained during a QBASIC active page switch
            if (last_active_page != active_page) {
                static img_struct *old_page;
                old_page = &img[page[last_active_page]];
                // WINDOW settings
                /*
                    SCREEN 7
                    WINDOW (0, 0)-(1, 1)
                    SCREEN 7, , 1, 1
                    PSET (.5, .5), 14
                */
                // VIEW settings
                /*
                    SCREEN 7
                    VIEW SCREEN (50, 50)-(100, 100)
                    SCREEN 7, , 1, 1
                    LINE (0, 0)-(1000, 1000), 1, BF
                */
                // GRAPHICS CURSOR LOCATION
                //(proven)
                // NOT MAINTAINED:
                // X color settings (for both text and graphics)
                // X text cursor location
                // X draw color (reset, as in QBASIC, by defaultcolors())
                if (!write_page->text) {
                    memcpy(&write_page->apm_p1, &old_page->apm_p1, (uint32)(&write_page->apm_p2 - &write_page->apm_p1));
                }
                last_active_page = active_page;
            } // active page migration
        }

    } // passed&4

    if (passed & 8) { // SCREEN ?,?,?,X (visual_page)
        i = visual_page;
        validatepage(i);
        i = page[i];
        if (display_page_index != i) {
            display_page_index = i;
            display_page = &img[i];
            defaultcolors();
            // reset VIEW PRINT state
            write_page->top_row = 1;
            if (!write_page->text)
                write_page->bottom_row = (write_page->height / write_page->font);
            else
                write_page->bottom_row = write_page->height;
            if (write_page->cursor_y > write_page->bottom_row)
                write_page->cursor_y = 1;
            write_page->bottom_row--;
            if (write_page->bottom_row <= 0)
                write_page->bottom_row = 1;
        }
    } // passed&8

    if (autodisplay) {
        if (lock_display_required)
            lock_display = 0; // release lock
    }

    return;
error:
    error(5);
    return;
} // screen (end)

void sub_pcopy(int32 src, int32 dst) {
    if (is_error_pending())
        return;
    static img_struct *s, *d;
    // validate
    if (src >= 0) {
        validatepage(src);
        s = &img[page[src]];
    } else {
        src = -src;
        if (src >= nextimg)
            goto error;
        s = &img[src];
        if (!s->valid)
            goto error;
    }
    if (dst >= 0) {
        validatepage(dst);
        d = &img[page[dst]];
    } else {
        dst = -dst;
        if (dst >= nextimg)
            goto error;
        d = &img[dst];
        if (!d->valid)
            goto error;
    }
    if (s == d)
        return;
    if (s->bytes_per_pixel != d->bytes_per_pixel)
        goto error;
    if ((s->height != d->height) || (s->width != d->width))
        goto error;
    if (s->bytes_per_pixel == 1) {
        if (d->mask < s->mask)
            goto error; // cannot copy onto a palette image with less colors
    }
    memcpy(d->offset, s->offset, d->width * d->height * d->bytes_per_pixel);
    return;
error:

    error(5);
    return;
}

void qbsub_width(int32 option, int32 value1, int32 value2, int32 value3, int32 value4, int32 passed) {
    //[{#|LPRINT}][?],[?]
    static int32 i, i2;

    if (is_error_pending())
        return;

    if (option == 0) { // WIDTH [?][,?]

        width8050switch = 0;

        static uint32 col, col2;

        // used to restore scaling after simple font changes
        // QBASIC/4.5/7.1: PMAP uses old scaling values after WIDTH change
        static float window_x1, window_y1, window_x2, window_y2;

        // Specifics:
        // MODE 0: Changes the resolution based on the desired width
        //        Horizontal width of 1 to 40 uses a double width font
        //        Heights from 1 to 42 use font height 16 pixels
        //        Heights from 43 to 49 use font height 14 pixels
        //        Heights from 50 to ? use font height 8 pixels
        // MODES 1-13: The resolution IS NOT CHANGED
        //            The font is changed to a font usually available for that screen
        //            mode, if available, that fits the given dimensions EXACTLY
        //            If not possible, it may jump back to SCREEN 0 in some instances
        //            just as it did in QBASIC
        // 256/32 BIT MODES: The font is unchanged
        //                  The resolution is changed using the currently selected font
        // note:
        // COLOR selection is kept, all other values are lost (if staying in same "mode")
        static int32 f, f2, width, height;

        width = value1;
        height = value2;

#ifdef QB64_WINDOWS
        if (write_page->console) {
            SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
            HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;

            GetConsoleScreenBufferInfo(cl_conout, &cl_bufinfo); // get the screen buffer information, for blank entries

            if (width <= 0)
                width = cl_bufinfo.srWindow.Right - cl_bufinfo.srWindow.Left + 1;
            ; // if width is omitted, then use existing width
            if (height <= 0)
                height = cl_bufinfo.srWindow.Bottom - cl_bufinfo.srWindow.Top + 1;
            ; // if height is omitted, then use existing height
            if (value3 <= 0)
                value3 = cl_bufinfo.dwSize.X; // if bufferwidth is omitted, then use existing buffer width
            if (value4 <= 0)
                value4 = cl_bufinfo.dwSize.Y; // same as above, but for height
            if (value3 < value1)
                value3 = value1; // don't make the buffer width smaller than the console width itself
            if (value4 < value2)
                value4 = value2; // and don't make that buffer height smaller than the console height

            SMALL_RECT rect = {0, 0, (SHORT)(width - 1), (SHORT)(height - 1)};
            COORD bufferSize = {(SHORT)value3, (SHORT)value4};
            SetConsoleScreenBufferSize(hConsole, bufferSize); // set the buffer
            SetConsoleWindowInfo(hConsole, TRUE, &rect);      // set the console itself
            return;
        }
#endif

        if ((!(passed & 1)) && (!(passed & 2)))
            goto error; // cannot omit both arguments

        if ((write_page->compatible_mode == 32) || (write_page->compatible_mode == 256)) {

            if (!(passed & 1)) { // width omitted
                width = write_page->width;
            } else {
                if (width <= 0)
                    goto error;
                i = fontwidth[write_page->font];
                if (!i)
                    i = 1;
                width *= i;
            }

            if (!(passed & 2)) { // height omitted
                height = write_page->height;
            } else {
                if (height <= 0)
                    goto error;
                height *= fontheight[write_page->font];
            }

            // width & height are now the desired dimensions

            if ((width == write_page->width) && (height == write_page->height))
                return; // no change required

            if (write_page->flags & IMG_SCREEN) {
                // delete pages 1-?
                for (i = 1; i < pages; i++) {
                    if ((i2 = page[i])) {
                        if (display_page_index == i2) {
                            display_page_index = page[0];
                            display_page = &img[display_page_index];
                        }
                        if (read_page_index == i2) {
                            read_page_index = display_page_index;
                            read_page = display_page;
                        }
                        if (write_page_index == i2) {
                            write_page_index = display_page_index;
                            write_page = display_page;
                        }
                        // manual delete, freeing video pages is usually illegal
                        if (img[i2].flags & IMG_FREEMEM)
                            free(img[i2].offset); // free pixel data
                        freeimg(i2);
                    }
                } // i
            }

            if (autodisplay) {
                if (write_page->flags & IMG_SCREEN) {
                    if (lock_display_required) {
                        if (lock_display == 0)
                            lock_display = 1;
                        while (lock_display != 2) {
                            Sleep(0);
                        }
                    }
                }
            }

            col = write_page->color;
            col2 = write_page->background_color;
            f = write_page->font;
            // change resolution
            write_page->width = width;
            write_page->height = height;
            if (write_page->flags & IMG_FREEMEM) {
                free(write_page->offset); // free pixel data
                write_page->offset = (uint8 *)calloc(width * height * write_page->bytes_per_pixel, 1);
            } else { // frame?
                memset(write_page->offset, 0, width * height * write_page->bytes_per_pixel);
            }
            imgrevert(write_page_index);
            write_page->color = col;
            write_page->background_color = col2;
            selectfont(f, write_page);

            if (autodisplay) {
                if (write_page->flags & IMG_SCREEN) {
                    if (lock_display_required)
                        lock_display = 0; // release lock
                }
            }

            return;

        } // 32/256

        if (!(passed & 1)) { // width omitted
            if (height <= 0)
                goto error;

            if (!write_page->compatible_mode) { // 0
                f = 8;
                if (height <= 49)
                    f = 14;
                if (height <= 42)
                    f = 16;
                width = write_page->width;
                if (width <= 40)
                    f++;
                if ((write_page->font == f) && (write_page->height == height))
                    return; // no change
                sub_screen_height_in_characters = height;
                sub_screen_width_in_characters = width;
                sub_screen_font = f;
                qbg_screen(0, 0, 0, 0, 0, 1);
                return;
            } // 0

            if (((write_page->compatible_mode >= 1) && (write_page->compatible_mode <= 8)) || (write_page->compatible_mode == 13)) {
                if (write_page->height == height * 8) { // correct resolution
                    if (write_page->font == 8)
                        return; // correct font, no change required
                    if (write_page->flags & IMG_SCREEN) {
                        // delete pages 1-?
                        for (i = 1; i < pages; i++) {
                            if ((i2 = page[i])) {
                                if (display_page_index == i2) {
                                    display_page_index = page[0];
                                    display_page = &img[display_page_index];
                                }
                                if (read_page_index == i2) {
                                    read_page_index = display_page_index;
                                    read_page = display_page;
                                }
                                if (write_page_index == i2) {
                                    write_page_index = display_page_index;
                                    write_page = display_page;
                                }
                                // manual delete, freeing video pages is usually illegal
                                if (img[i2].flags & IMG_FREEMEM)
                                    free(img[i2].offset); // free pixel data
                                freeimg(i2);
                            }
                        } // i
                    }
                    col = write_page->color;
                    col2 = write_page->background_color;
                    imgrevert(write_page_index);
                    write_page->color = col;
                    write_page->background_color = col2;
                    selectfont(8, write_page);
                    return;
                } // correct resolution
                // fall through
            } // modes 1-8

            /*
                SCREEN 9: 640 x 350 graphics
                \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
                \A6 64K page size, page range is 0 (64K);
                128K page size, page range is 0 (128K) or 0-1 (256K)
                \A6 16 colors assigned to 4 attributes (64K adapter memory), or
                64 colors assigned to 16 attributes (more than 64K adapter memory)
                SCREEN 10: 640 x 350 graphics, monochrome monitor only
                \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
                \A6 128K page size, page range is 0 (128K) or 0-1 (256K)
                \A6 Up to 9 pseudocolors assigned to 4 attributes
            */
            if ((write_page->compatible_mode >= 9) && (write_page->compatible_mode <= 10)) {
                f = 0;
                if (write_page->height == height * 8)
                    f = 8;
                if (write_page->height == height * 14)
                    f = 14;
                if ((height == 43) && (write_page->height == 350))
                    f = 8; //?x350,8x8
                if (f) {   // correct resolution
                    if (write_page->font == f)
                        return; // correct font, no change required
                    if (write_page->flags & IMG_SCREEN) {
                        // delete pages 1-?
                        for (i = 1; i < pages; i++) {
                            if ((i2 = page[i])) {
                                if (display_page_index == i2) {
                                    display_page_index = page[0];
                                    display_page = &img[display_page_index];
                                }
                                if (read_page_index == i2) {
                                    read_page_index = display_page_index;
                                    read_page = display_page;
                                }
                                if (write_page_index == i2) {
                                    write_page_index = display_page_index;
                                    write_page = display_page;
                                }
                                // manual delete, freeing video pages is usually illegal
                                if (img[i2].flags & IMG_FREEMEM)
                                    free(img[i2].offset); // free pixel data
                                freeimg(i2);
                            }
                        } // i
                    }
                    col = write_page->color;
                    col2 = write_page->background_color;
                    window_x1 = write_page->window_x1;
                    window_x2 = write_page->window_x2;
                    window_y1 = write_page->window_y1;
                    window_y2 = write_page->window_y2;
                    imgrevert(write_page_index);
                    qbg_sub_window(window_x1, window_y1, window_x2, window_y2, 1 + 2);
                    write_page->clipping_or_scaling = 0;
                    write_page->color = col;
                    write_page->background_color = col2;
                    selectfont(f, write_page);
                    return;
                } // correct resolution
                // fall through
            } // modes 9,10

            if ((write_page->compatible_mode >= 11) && (write_page->compatible_mode <= 12)) {
                f = 0;
                if (write_page->height == height * 8)
                    f = 8;
                if (write_page->height == height * 16)
                    f = 16;
                if (f) { // correct resolution
                    if (write_page->font == f)
                        return; // correct font, no change required
                    if (write_page->flags & IMG_SCREEN) {
                        // delete pages 1-?
                        for (i = 1; i < pages; i++) {
                            if ((i2 = page[i])) {
                                if (display_page_index == i2) {
                                    display_page_index = page[0];
                                    display_page = &img[display_page_index];
                                }
                                if (read_page_index == i2) {
                                    read_page_index = display_page_index;
                                    read_page = display_page;
                                }
                                if (write_page_index == i2) {
                                    write_page_index = display_page_index;
                                    write_page = display_page;
                                }
                                // manual delete, freeing video pages is usually illegal
                                if (img[i2].flags & IMG_FREEMEM)
                                    free(img[i2].offset); // free pixel data
                                freeimg(i2);
                            }
                        } // i
                    }
                    col = write_page->color;
                    col2 = write_page->background_color;
                    window_x1 = write_page->window_x1;
                    window_x2 = write_page->window_x2;
                    window_y1 = write_page->window_y1;
                    window_y2 = write_page->window_y2;
                    imgrevert(write_page_index);
                    qbg_sub_window(window_x1, window_y1, window_x2, window_y2, 1 + 2);
                    write_page->clipping_or_scaling = 0;
                    write_page->color = col;
                    write_page->background_color = col2;
                    selectfont(f, write_page);
                    return;
                } // correct resolution
                // fall through
            } // modes 11,12

            // fall through:
            if ((height == 25) || (height == 50) || (height == 43)) {
                sub_screen_height_in_characters = height;
                qbg_screen(0, 0, 0, 0, 0, 1);
                return;
            }

            goto error;

        } // width omitted

        if (!(passed & 2)) { // height omitted

            if (width <= 0)
                goto error;

            if (!write_page->compatible_mode) { // 0
                height = write_page->height;
                f = 8;
                if (height <= 49)
                    f = 14;
                if (height <= 42)
                    f = 16;
                if (width <= 40)
                    f++;
                if ((write_page->font == f) && (write_page->width == width))
                    return; // no change
                sub_screen_height_in_characters = height;
                sub_screen_width_in_characters = width;
                sub_screen_font = f;
                qbg_screen(0, 0, 0, 0, 0, 1);
                return;
            } // 0

            if (((write_page->compatible_mode >= 1) && (write_page->compatible_mode <= 8)) || (write_page->compatible_mode == 13)) {
                if (write_page->width == width * 8) { // correct resolution
                    if (write_page->font == 8)
                        return; // correct font, no change required
                    if (write_page->flags & IMG_SCREEN) {
                        // delete pages 1-?
                        for (i = 1; i < pages; i++) {
                            if ((i2 = page[i])) {
                                if (display_page_index == i2) {
                                    display_page_index = page[0];
                                    display_page = &img[display_page_index];
                                }
                                if (read_page_index == i2) {
                                    read_page_index = display_page_index;
                                    read_page = display_page;
                                }
                                if (write_page_index == i2) {
                                    write_page_index = display_page_index;
                                    write_page = display_page;
                                }
                                // manual delete, freeing video pages is usually illegal
                                if (img[i2].flags & IMG_FREEMEM)
                                    free(img[i2].offset); // free pixel data
                                freeimg(i2);
                            }
                        } // i
                    }
                    col = write_page->color;
                    col2 = write_page->background_color;
                    imgrevert(write_page_index);
                    write_page->color = col;
                    write_page->background_color = col2;
                    selectfont(8, write_page);
                    return;
                } // correct resolution
                // fall through
            } // modes 1-8

            /*
                SCREEN 9: 640 x 350 graphics
                \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
                \A6 64K page size, page range is 0 (64K);
                128K page size, page range is 0 (128K) or 0-1 (256K)
                \A6 16 colors assigned to 4 attributes (64K adapter memory), or
                64 colors assigned to 16 attributes (more than 64K adapter memory)
                SCREEN 10: 640 x 350 graphics, monochrome monitor only
                \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
                \A6 128K page size, page range is 0 (128K) or 0-1 (256K)
                \A6 Up to 9 pseudocolors assigned to 4 attributes
            */
            if ((write_page->compatible_mode >= 9) && (write_page->compatible_mode <= 10)) {
                f = 0;
                if (write_page->width == width * 8)
                    f = 8;
                if (f) { // correct resolution
                    f2 = fontheight[write_page->font];
                    if (f2 > 8)
                        f = 14;
                    if (write_page->font == f)
                        return; // correct font, no change required
                    if (write_page->flags & IMG_SCREEN) {
                        // delete pages 1-?
                        for (i = 1; i < pages; i++) {
                            if ((i2 = page[i])) {
                                if (display_page_index == i2) {
                                    display_page_index = page[0];
                                    display_page = &img[display_page_index];
                                }
                                if (read_page_index == i2) {
                                    read_page_index = display_page_index;
                                    read_page = display_page;
                                }
                                if (write_page_index == i2) {
                                    write_page_index = display_page_index;
                                    write_page = display_page;
                                }
                                // manual delete, freeing video pages is usually illegal
                                if (img[i2].flags & IMG_FREEMEM)
                                    free(img[i2].offset); // free pixel data
                                freeimg(i2);
                            }
                        } // i
                    }
                    col = write_page->color;
                    col2 = write_page->background_color;
                    window_x1 = write_page->window_x1;
                    window_x2 = write_page->window_x2;
                    window_y1 = write_page->window_y1;
                    window_y2 = write_page->window_y2;
                    imgrevert(write_page_index);
                    qbg_sub_window(window_x1, window_y1, window_x2, window_y2, 1 + 2);
                    write_page->clipping_or_scaling = 0;
                    write_page->color = col;
                    write_page->background_color = col2;
                    selectfont(f, write_page);
                    return;
                } // correct resolution
                // fall through
            } // modes 9,10

            if ((write_page->compatible_mode >= 11) && (write_page->compatible_mode <= 12)) {
                f = 0;
                if (write_page->width == width * 8)
                    f = 8;
                if (f) { // correct resolution
                    f2 = fontheight[write_page->font];
                    if (f2 > 8)
                        f = 16;
                    if (write_page->font == f)
                        return; // correct font, no change required
                    if (write_page->flags & IMG_SCREEN) {
                        // delete pages 1-?
                        for (i = 1; i < pages; i++) {
                            if ((i2 = page[i])) {
                                if (display_page_index == i2) {
                                    display_page_index = page[0];
                                    display_page = &img[display_page_index];
                                }
                                if (read_page_index == i2) {
                                    read_page_index = display_page_index;
                                    read_page = display_page;
                                }
                                if (write_page_index == i2) {
                                    write_page_index = display_page_index;
                                    write_page = display_page;
                                }
                                // manual delete, freeing video pages is usually illegal
                                if (img[i2].flags & IMG_FREEMEM)
                                    free(img[i2].offset); // free pixel data
                                freeimg(i2);
                            }
                        } // i
                    }
                    col = write_page->color;
                    col2 = write_page->background_color;
                    window_x1 = write_page->window_x1;
                    window_x2 = write_page->window_x2;
                    window_y1 = write_page->window_y1;
                    window_y2 = write_page->window_y2;
                    imgrevert(write_page_index);
                    qbg_sub_window(window_x1, window_y1, window_x2, window_y2, 1 + 2);
                    write_page->clipping_or_scaling = 0;
                    write_page->color = col;
                    write_page->background_color = col2;
                    selectfont(f, write_page);
                    return;
                } // correct resolution
                // fall through
            } // modes 11,12

            // fall through:
            if ((width == 40) || (width == 80)) {
                sub_screen_width_in_characters = width;
                qbg_screen(0, 0, 0, 0, 0, 1);
                return;
            }

            goto error;

        } // height omitted

        // both height & width passed

        if ((width <= 0) || (height <= 0))
            goto error;

        if (!write_page->compatible_mode) { // 0
            f = 8;
            if (height <= 49)
                f = 14;
            if (height <= 42)
                f = 16;
            if (width <= 40)
                f++;
            if ((write_page->font == f) && (write_page->width == width) && (write_page->height == height))
                return; // no change
            sub_screen_height_in_characters = height;
            sub_screen_width_in_characters = width;
            sub_screen_font = f;
            qbg_screen(0, 0, 0, 0, 0, 1);
            return;
        } // 0

        if (((write_page->compatible_mode >= 1) && (write_page->compatible_mode <= 8)) || (write_page->compatible_mode == 13)) {
            if ((write_page->width == width * 8) && (write_page->height == height * 8)) { // correct resolution
                if (write_page->font == 8)
                    return; // correct font, no change required
                if (write_page->flags & IMG_SCREEN) {
                    // delete pages 1-?
                    for (i = 1; i < pages; i++) {
                        if ((i2 = page[i])) {
                            if (display_page_index == i2) {
                                display_page_index = page[0];
                                display_page = &img[display_page_index];
                            }
                            if (read_page_index == i2) {
                                read_page_index = display_page_index;
                                read_page = display_page;
                            }
                            if (write_page_index == i2) {
                                write_page_index = display_page_index;
                                write_page = display_page;
                            }
                            // manual delete, freeing video pages is usually illegal
                            if (img[i2].flags & IMG_FREEMEM)
                                free(img[i2].offset); // free pixel data
                            freeimg(i2);
                        }
                    } // i
                }
                col = write_page->color;
                col2 = write_page->background_color;
                imgrevert(write_page_index);
                write_page->color = col;
                write_page->background_color = col2;
                selectfont(8, write_page);
                return;
            } // correct resolution
            // fall through
        } // modes 1-8

        /*
            SCREEN 9: 640 x 350 graphics
            \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
            \A6 64K page size, page range is 0 (64K);
            128K page size, page range is 0 (128K) or 0-1 (256K)
            \A6 16 colors assigned to 4 attributes (64K adapter memory), or
            64 colors assigned to 16 attributes (more than 64K adapter memory)
            SCREEN 10: 640 x 350 graphics, monochrome monitor only
            \A6 80 x 25 or 80 x 43 text format, 8 x 14 or 8 x 8 character box size
            \A6 128K page size, page range is 0 (128K) or 0-1 (256K)
            \A6 Up to 9 pseudocolors assigned to 4 attributes
        */
        if ((write_page->compatible_mode >= 9) && (write_page->compatible_mode <= 10)) {
            f = 0;
            if (write_page->width == width * 8) {
                if (write_page->height == height * 8)
                    f = 8;
                if (write_page->height == height * 14)
                    f = 14;
                if ((height == 43) && (write_page->height == 350))
                    f = 8; //?x350,8x8
            }
            if (f) { // correct resolution
                if (write_page->font == f)
                    return; // correct font, no change required
                if (write_page->flags & IMG_SCREEN) {
                    // delete pages 1-?
                    for (i = 1; i < pages; i++) {
                        if ((i2 = page[i])) {
                            if (display_page_index == i2) {
                                display_page_index = page[0];
                                display_page = &img[display_page_index];
                            }
                            if (read_page_index == i2) {
                                read_page_index = display_page_index;
                                read_page = display_page;
                            }
                            if (write_page_index == i2) {
                                write_page_index = display_page_index;
                                write_page = display_page;
                            }
                            // manual delete, freeing video pages is usually illegal
                            if (img[i2].flags & IMG_FREEMEM)
                                free(img[i2].offset); // free pixel data
                            freeimg(i2);
                        }
                    } // i
                }
                col = write_page->color;
                col2 = write_page->background_color;
                window_x1 = write_page->window_x1;
                window_x2 = write_page->window_x2;
                window_y1 = write_page->window_y1;
                window_y2 = write_page->window_y2;
                imgrevert(write_page_index);
                qbg_sub_window(window_x1, window_y1, window_x2, window_y2, 1 + 2);
                write_page->clipping_or_scaling = 0;
                write_page->color = col;
                write_page->background_color = col2;
                selectfont(f, write_page);
                return;
            } // correct resolution
            // fall through
        } // modes 9,10

        if ((write_page->compatible_mode >= 11) && (write_page->compatible_mode <= 12)) {
            f = 0;
            if (write_page->width == width * 8) {

                if (write_page->height == height * 8)
                    f = 8;

                if (write_page->height == height * 16)
                    f = 16;
            }
            if (f) { // correct resolution
                if (write_page->font == f)
                    return; // correct font, no change required
                if (write_page->flags & IMG_SCREEN) {
                    // delete pages 1-?
                    for (i = 1; i < pages; i++) {
                        if ((i2 = page[i])) {
                            if (display_page_index == i2) {
                                display_page_index = page[0];
                                display_page = &img[display_page_index];
                            }
                            if (read_page_index == i2) {
                                read_page_index = display_page_index;
                                read_page = display_page;
                            }
                            if (write_page_index == i2) {
                                write_page_index = display_page_index;
                                write_page = display_page;
                            }
                            // manual delete, freeing video pages is usually illegal
                            if (img[i2].flags & IMG_FREEMEM)
                                free(img[i2].offset); // free pixel data
                            freeimg(i2);
                        }
                    } // i
                }
                col = write_page->color;
                col2 = write_page->background_color;
                window_x1 = write_page->window_x1;
                window_x2 = write_page->window_x2;
                window_y1 = write_page->window_y1;
                window_y2 = write_page->window_y2;
                imgrevert(write_page_index);
                qbg_sub_window(window_x1, window_y1, window_x2, window_y2, 1 + 2);
                write_page->clipping_or_scaling = 0;
                write_page->color = col;
                write_page->background_color = col2;
                selectfont(f, write_page);
                return;
            } // correct resolution
            // fall through
        } // modes 11,12

        // fall through:
        if ((width == 40) || (width == 80)) {
            if ((height == 25) || (height == 50) || (height == 43)) {
                sub_screen_width_in_characters = width;
                sub_screen_height_in_characters = height;
                f = 16;
                if (height == 43)
                    f = 14;
                if (height == 50)
                    f = 8;
                if (width == 40)
                    f++;
                sub_screen_font = f;
                qbg_screen(0, 0, 0, 0, 0, 1);
                return;
            }

            goto error;

        } // WIDTH [?][,?]

    } // option==0

    if (option == 2) { // LPRINT
        if (passed != 1)
            goto error;
        if ((value1 < 1) || (value1 > 255))
            goto error;
        width_lprint = value1;
        return;
    } // option==2

    // file/device?
    //...

error:
    error(5);
    return;
}

void pset_and_clip(int32 x, int32 y, uint32 col) {

    if ((x >= write_page->view_x1) && (x <= write_page->view_x2) && (y >= write_page->view_y1) && (y <= write_page->view_y2)) {

        static uint8 *cp;
        static uint32 *o32;
        static uint32 destcol;
        if (write_page->bytes_per_pixel == 1) {
            write_page->offset[y * write_page->width + x] = col & write_page->mask;
            return;
        } else {

            if (write_page->alpha_disabled) {
                write_page->offset32[y * write_page->width + x] = col;
                return;
            }
            switch (col & 0xFF000000) {
            case 0xFF000000: // 100% alpha, so regular pset (fast)
                write_page->offset32[y * write_page->width + x] = col;
                return;
                break;
            case 0x0: // 0%(0) alpha, so no pset (very fast)
                return;
                break;
            case 0x80000000: //~50% alpha (optimized)
                o32 = write_page->offset32 + (y * write_page->width + x);
                *o32 = (((*o32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*o32 >> 24] << 24);
                return;
                break;
            case 0x7F000000: //~50% alpha (optimized)
                o32 = write_page->offset32 + (y * write_page->width + x);
                *o32 = (((*o32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*o32 >> 24] << 24);
                return;
                break;
            default: // other alpha values (uses a lookup table)
                o32 = write_page->offset32 + (y * write_page->width + x);
                destcol = *o32;
                cp = cblend + (col >> 24 << 16);
                *o32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                       (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
            };
        }

    } // within viewport
    return;
}

void qb32_boxfill(float x1f, float y1f, float x2f, float y2f, uint32 col) {
    static int32 x1, y1, x2, y2, i, width, img_width, x, y, d_width, a, a2, v1, v2, v3;
    static uint8 *p, *cp, *cp2, *cp3;
    static uint32 *lp, *lp_last, *lp_first;
    static uint32 *doff32, destcol;

    // resolve coordinates
    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            x1 = qbr_float_to_long(x1f * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            y1 = qbr_float_to_long(y1f * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
            x2 = qbr_float_to_long(x2f * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            y2 = qbr_float_to_long(y2f * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            x1 = qbr_float_to_long(x1f) + write_page->view_offset_x;
            y1 = qbr_float_to_long(y1f) + write_page->view_offset_y;
            x2 = qbr_float_to_long(x2f) + write_page->view_offset_x;
            y2 = qbr_float_to_long(y2f) + write_page->view_offset_y;
        }
    } else {
        x1 = qbr_float_to_long(x1f);
        y1 = qbr_float_to_long(y1f);
        x2 = qbr_float_to_long(x2f);
        y2 = qbr_float_to_long(y2f);
    }

    // swap coordinates (if necessary)
    if (x1 > x2) {
        i = x1;
        x1 = x2;
        x2 = i;
    }
    if (y1 > y2) {
        i = y1;
        y1 = y2;
        y2 = i;
    }

    // exit without rendering if necessary
    if (x2 < write_page->view_x1)
        return;
    if (x1 > write_page->view_x2)
        return;
    if (y2 < write_page->view_y1)
        return;
    if (y1 > write_page->view_y2)
        return;

    // crop coordinates
    if (x1 < write_page->view_x1)
        x1 = write_page->view_x1;
    if (y1 < write_page->view_y1)
        y1 = write_page->view_y1;
    if (x1 > write_page->view_x2)
        x1 = write_page->view_x2;
    if (y1 > write_page->view_y2)
        y1 = write_page->view_y2;
    if (x2 < write_page->view_x1)
        x2 = write_page->view_x1;
    if (y2 < write_page->view_y1)
        y2 = write_page->view_y1;
    if (x2 > write_page->view_x2)
        x2 = write_page->view_x2;
    if (y2 > write_page->view_y2)
        y2 = write_page->view_y2;

    if (write_page->bytes_per_pixel == 1) {
        col &= write_page->mask;
        width = x2 - x1 + 1;
        img_width = write_page->width;
        p = write_page->offset + y1 * write_page->width + x1;
        i = y2 - y1 + 1;
    loop:
        memset(p, col, width);
        p += img_width;
        if (--i)
            goto loop;
        return;
    } // 1

    // assume 32-bit
    // optimized
    // alpha disabled or full alpha?
    a = col >> 24;
    if ((write_page->alpha_disabled) || (a == 255)) {
        width = x2 - x1 + 1;
        y = y2 - y1 + 1;
        img_width = write_page->width;
        // build first line pixel by pixel
        lp_first = write_page->offset32 + y1 * img_width + x1;
        lp = lp_first - 1;
        lp_last = lp + width;
        while (lp++ < lp_last)
            *lp = col;
        // copy remaining lines
        lp = lp_first;
        width *= 4;
        while (y--) {
            memcpy(lp, lp_first, width);
            lp += img_width;
        }
        return;
    }
    // no alpha?
    if (!a)
        return;
    // half alpha?
    img_width = write_page->width;
    doff32 = write_page->offset32 + y1 * img_width + x1;
    width = x2 - x1 + 1;
    d_width = img_width - width;
    if (a == 128) {
        col &= 0xFEFEFE;
        y = y2 - y1 + 1;
        while (y--) {
            x = width;
            while (x--) {
                *doff32++ = (((*doff32 & 0xFEFEFE) + col) >> 1) + (ablend128[*doff32 >> 24] << 24);
            }
            doff32 += d_width;
        }
        return;
    }
    if (a == 127) {
        col &= 0xFEFEFE;
        y = y2 - y1 + 1;
        while (y--) {
            x = width;
            while (x--) {
                *doff32++ = (((*doff32 & 0xFEFEFE) + col) >> 1) + (ablend127[*doff32 >> 24] << 24);
            }
            doff32 += d_width;
        }
        return;
    }
    // ranged alpha
    cp = cblend + (a << 16);
    a2 = a << 8;
    cp3 = cp + (col >> 8 & 0xFF00);
    cp2 = cp + (col & 0xFF00);
    cp += (col << 8 & 0xFF00);
    y = y2 - y1 + 1;
    while (y--) {
        x = width;
        while (x--) {
            destcol = *doff32;
            *doff32++ = cp[destcol & 255] + (cp2[destcol >> 8 & 255] << 8) + (cp3[destcol >> 16 & 255] << 16) + (ablend[(destcol >> 24) + a2] << 24);
        }
        doff32 += d_width;
    }
    return;
}

void fast_boxfill(int32 x1, int32 y1, int32 x2, int32 y2, uint32 col) {
    // assumes:
    // actual coordinates passed
    // left->right, top->bottom order
    // on-screen
    static int32 i, width, img_width, x, y, d_width, a, a2, v1, v2, v3;
    static uint8 *p, *cp, *cp2, *cp3;
    static uint32 *lp, *lp_last, *lp_first;
    static uint32 *doff32, destcol;

    if (write_page->bytes_per_pixel == 1) {
        col &= write_page->mask;
        width = x2 - x1 + 1;
        img_width = write_page->width;
        p = write_page->offset + y1 * write_page->width + x1;
        i = y2 - y1 + 1;
    loop:
        memset(p, col, width);
        p += img_width;
        if (--i)
            goto loop;
        return;
    } // 1

    // assume 32-bit
    // optimized
    // alpha disabled or full alpha?
    a = col >> 24;
    if ((write_page->alpha_disabled) || (a == 255)) {

        width = x2 - x1 + 1;
        y = y2 - y1 + 1;
        img_width = write_page->width;
        // build first line pixel by pixel
        lp_first = write_page->offset32 + y1 * img_width + x1;
        lp = lp_first - 1;
        lp_last = lp + width;
        while (lp++ < lp_last)
            *lp = col;
        // copy remaining lines
        lp = lp_first;
        width *= 4;
        while (y--) {
            memcpy(lp, lp_first, width);
            lp += img_width;
        }
        return;
    }
    // no alpha?
    if (!a)
        return;
    // half alpha?
    img_width = write_page->width;
    doff32 = write_page->offset32 + y1 * img_width + x1;
    width = x2 - x1 + 1;
    d_width = img_width - width;
    if (a == 128) {
        col &= 0xFEFEFE;
        y = y2 - y1 + 1;
        while (y--) {
            x = width;
            while (x--) {
                *doff32++ = (((*doff32 & 0xFEFEFE) + col) >> 1) + (ablend128[*doff32 >> 24] << 24);
            }
            doff32 += d_width;
        }
        return;
    }
    if (a == 127) {
        col &= 0xFEFEFE;
        y = y2 - y1 + 1;
        while (y--) {
            x = width;
            while (x--) {
                *doff32++ = (((*doff32 & 0xFEFEFE) + col) >> 1) + (ablend127[*doff32 >> 24] << 24);
            }
            doff32 += d_width;
        }
        return;
    }
    // ranged alpha
    cp = cblend + (a << 16);
    a2 = a << 8;
    cp3 = cp + (col >> 8 & 0xFF00);
    cp2 = cp + (col & 0xFF00);
    cp += (col << 8 & 0xFF00);
    y = y2 - y1 + 1;
    while (y--) {
        x = width;
        while (x--) {
            destcol = *doff32;
            *doff32++ = cp[destcol & 255] + (cp2[destcol >> 8 & 255] << 8) + (cp3[destcol >> 16 & 255] << 16) + (ablend[(destcol >> 24) + a2] << 24);
        }
        doff32 += d_width;
    }
    return;
}

// copied from qb32_line with the following modifications
// i. pre-WINDOW'd & VIEWPORT'd int32 co-ordinates
// ii. all references to style & lineclip_skippixels commented
// iii. declaration of x1,y1,x2,y2,x1f,y1f changed, some declarations removed
void fast_line(int32 x1, int32 y1, int32 x2, int32 y2, uint32 col) {
    static int32 l, l2, mi;
    static float m, x1f, y1f;

    lineclip(x1, y1, x2, y2, write_page->view_x1, write_page->view_y1, write_page->view_x2, write_page->view_y2);

    // style=(style&65535)+(style<<16);
    // lineclip_skippixels&=15;
    // style=_lrotl(style,lineclip_skippixels);

    if (lineclip_draw) {
        l = abs(lineclip_x1 - lineclip_x2);
        l2 = abs(lineclip_y1 - lineclip_y2);
        if (l > l2) {

            // x-axis distance is larger
            y1f = lineclip_y1;
            if (l) { // following only applies if drawing more than one pixel
                m = ((float)lineclip_y2 - (float)lineclip_y1) / (float)l;
                if (lineclip_x2 >= lineclip_x1)
                    mi = 1;
                else
                    mi = -1; // direction of change
            }
            l++;
            while (l--) {
                if (y1f < 0)
                    lineclip_y1 = y1f - 0.5f;
                else
                    lineclip_y1 = y1f + 0.5f;

                // if ((style=_lrotl(style,1))&1){
                pset(lineclip_x1, lineclip_y1, col);
                //}

                lineclip_x1 += mi;
                y1f += m;
            }

        } else {

            // y-axis distance is larger
            x1f = lineclip_x1;
            if (l2) { // following only applies if drawing more than one pixel
                m = ((float)lineclip_x2 - (float)lineclip_x1) / (float)l2;
                if (lineclip_y2 >= lineclip_y1)
                    mi = 1;
                else
                    mi = -1; // direction of change
            }
            l2++;
            while (l2--) {
                if (x1f < 0)
                    lineclip_x1 = x1f - 0.5f;
                else
                    lineclip_x1 = x1f + 0.5f;
                // if ((style=_lrotl(style,1))&1){
                pset(lineclip_x1, lineclip_y1, col);
                //}
                lineclip_y1 += mi;
                x1f += m;
            }
        }

    } // lineclip_draw
    return;
}

void qb32_line(float x1f, float y1f, float x2f, float y2f, uint32 col, uint32 style) {
    static int32 x1, y1, x2, y2, l, l2, mi;
    static float m;

    // resolve coordinates
    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            x1 = qbr_float_to_long(x1f * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            y1 = qbr_float_to_long(y1f * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
            x2 = qbr_float_to_long(x2f * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            y2 = qbr_float_to_long(y2f * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            x1 = qbr_float_to_long(x1f) + write_page->view_offset_x;
            y1 = qbr_float_to_long(y1f) + write_page->view_offset_y;
            x2 = qbr_float_to_long(x2f) + write_page->view_offset_x;
            y2 = qbr_float_to_long(y2f) + write_page->view_offset_y;
        }
    } else {
        x1 = qbr_float_to_long(x1f);
        y1 = qbr_float_to_long(y1f);
        x2 = qbr_float_to_long(x2f);
        y2 = qbr_float_to_long(y2f);
    }

    lineclip(x1, y1, x2, y2, write_page->view_x1, write_page->view_y1, write_page->view_x2, write_page->view_y2);

    style = (style & 65535) + (style << 16);
    lineclip_skippixels &= 15;
    style = rotateLeft(style, lineclip_skippixels);

    if (lineclip_draw) {
        l = abs(lineclip_x1 - lineclip_x2);
        l2 = abs(lineclip_y1 - lineclip_y2);
        if (l > l2) {

            // x-axis distance is larger
            y1f = lineclip_y1;
            if (l) { // following only applies if drawing more than one pixel
                m = ((float)lineclip_y2 - (float)lineclip_y1) / (float)l;
                if (lineclip_x2 >= lineclip_x1)
                    mi = 1;
                else
                    mi = -1; // direction of change
            }
            l++;
            while (l--) {
                if (y1f < 0)
                    lineclip_y1 = y1f - 0.5f;
                else
                    lineclip_y1 = y1f + 0.5f;

                if ((style = rotateLeft(style, 1)) & 1) {
                    pset(lineclip_x1, lineclip_y1, col);
                }

                lineclip_x1 += mi;
                y1f += m;
            }

        } else {

            // y-axis distance is larger
            x1f = lineclip_x1;
            if (l2) { // following only applies if drawing more than one pixel
                m = ((float)lineclip_x2 - (float)lineclip_x1) / (float)l2;
                if (lineclip_y2 >= lineclip_y1)
                    mi = 1;
                else
                    mi = -1; // direction of change
            }
            l2++;
            while (l2--) {
                if (x1f < 0)
                    lineclip_x1 = x1f - 0.5f;
                else
                    lineclip_x1 = x1f + 0.5f;
                if ((style = rotateLeft(style, 1)) & 1) {
                    pset(lineclip_x1, lineclip_y1, col);
                }
                lineclip_y1 += mi;
                x1f += m;
            }
        }

    } // lineclip_draw
    return;
}


// Drawing primitives (sub_line, sub_paint, sub_circle, func_point, sub_pset, sub_preset) moved to graphics.cpp


// printchr moved to text.cpp

// chrwidth moved to text.cpp

// newline moved to text.cpp

// makefit moved to text.cpp

// lprint_makefit moved to text.cpp

// tab moved to text.cpp

// func_lpos moved to text.cpp

// qbs_lprint moved to text.cpp

extern int32 no_control_characters;
// no_control_characters2, sub__controlchr, func__controlchr moved to utility.cpp

// qbs_print moved to text.cpp

void qbg_sub_window(float x1, float y1, float x2, float y2, int32 passed) {
    //                  &1
    //(passed&2)->SCREEN
    if (is_error_pending())
        return;
    static float i;
    static float old_x, old_y;

    if (write_page->text)
        goto qbg_sub_window_error;
    if ((!(passed & 1)) && (passed & 2))
        goto qbg_sub_window_error; // SCREEN passed without any other arguments!

    // backup current qbg_x & qbg_y coordinates relative to viewport, not window
    if (write_page->clipping_or_scaling == 2) {
        old_x = write_page->x * write_page->scaling_x + write_page->scaling_offset_x;
        old_y = write_page->y * write_page->scaling_y + write_page->scaling_offset_y;
    } else {
        old_x = write_page->x;
        old_y = write_page->y;
    }

    if (passed & 1) {
        if (x1 == x2)
            goto qbg_sub_window_error;
        if (y1 == y2)
            goto qbg_sub_window_error;
        // sort so x1 & y1 contain the lower values
        if (x1 > x2) {
            i = x1;
            x1 = x2;
            x2 = i;
        }
        if (y1 > y2) {
            i = y1;
            y1 = y2;
            y2 = i;
        }
        if (!(passed & 2)) {
            i = y1;
            y1 = y2;
            y2 = i;
        }
        // Note: Window's coordinates are not based on prev. WINDOW values
        write_page->clipping_or_scaling = 2;
        write_page->scaling_x = ((float)(write_page->view_x2 - write_page->view_x1)) / (x2 - x1);
        write_page->scaling_y = ((float)(write_page->view_y2 - write_page->view_y1)) / (y2 - y1);
        write_page->scaling_offset_x = -x1 * write_page->scaling_x; // scaling offset should be applied before scaling
        write_page->scaling_offset_y = -y1 * write_page->scaling_y;
        if (!(passed & 2)) {
            write_page->scaling_offset_y = -y2 * write_page->scaling_y + (write_page->view_y2 - write_page->view_y1);
        }
        write_page->window_x1 = x1;
        write_page->window_x2 = x2;
        write_page->window_y1 = y1;
        write_page->window_y2 = y2;

        if (x1 == 0) {
            if (y1 == 0) {
                if (x2 == (write_page->width - 1)) {
                    if (y2 == (write_page->height - 1)) {
                        if ((write_page->scaling_x == 1) && (write_page->scaling_y == 1)) {
                            if ((write_page->scaling_offset_x == 0) && (write_page->scaling_offset_y == 0)) {
                                goto qbg_sub_window_restore_default;
                            }
                        }
                    }
                }
            }
        }

        // adjust qbg_x & qbg_y according to new window
        write_page->x = (old_x - write_page->scaling_offset_x) / write_page->scaling_x;
        write_page->y = (old_y - write_page->scaling_offset_y) / write_page->scaling_y;

        return;
    } else {
    // restore default WINDOW coordinates
    qbg_sub_window_restore_default:
        write_page->clipping_or_scaling = 1;
        write_page->scaling_x = 1;
        write_page->scaling_y = 1;
        write_page->scaling_offset_x = 0;
        write_page->scaling_offset_y = 0;
        write_page->window_x1 = 0;
        write_page->window_x2 = write_page->width - 1;
        write_page->window_y1 = 0;
        write_page->window_y2 = write_page->height - 1;
        if (write_page->view_x1 == 0) {
            if (write_page->view_y1 == 0) {
                if (write_page->view_x2 == (write_page->width - 1)) {
                    if (write_page->view_y2 == (write_page->height - 1)) {
                        if (write_page->view_offset_x == 0) {
                            if (write_page->view_offset_y == 0) {
                                write_page->clipping_or_scaling = 0;
                            }
                        }
                    }
                }
            }
        }

        // adjust qbg_x & qbg_y according to new window
        write_page->x = old_x;
        write_page->y = old_y;

        return;
    }
qbg_sub_window_error:
    error(5);
    return;
}

// qbg_sub_view_print moved to text.cpp

void qbg_sub_view(int32 x1, int32 y1, int32 x2, int32 y2, int32 fillcolor, int32 bordercolor, int32 passed) {
    //   &1                                   &4              &8
    //    (passed&2)->coords_relative_to_screen
    if (is_error_pending())
        return;
    // format: [{SCREEN}][(?,?)-(?,?)],[?],[?]
    // bordercolor draws a line AROUND THE OUTSIDE of the specified viewport
    // the current WINDOW settings do not affect inputted x,y values
    // the current VIEW settings do not affect inputted x,y values
    // REMEMBER! Recalculate WINDOW values based on new viewport dimensions
    int32 i;

    // PRE-ERROR CHECKING
    if (passed & 1) {
        if (x1 < 0)
            goto error;
        if (x1 >= write_page->width)
            goto error;
        if (y1 < 0)
            goto error;
        if (y1 >= write_page->height)
            goto error;
        if (x2 < 0)
            goto error;
        if (x2 >= write_page->width)
            goto error;
        if (y2 < 0)
            goto error;
        if (y2 >= write_page->height)
            goto error;
    } else {
        if (passed & 2)
            goto error;
        if (passed & 4)
            goto error;
        if (passed & 8)
            goto error;
    }

    // reset DRAW attributes
    write_page->draw_ta = 0.0;
    write_page->draw_scale = 1.0;

    if (passed & 1) {
        // force x1,y1 to be the top left corner
        if (x2 < x1) {
            i = x1;
            x1 = x2;
            x2 = i;
        }
        if (y2 < y1) {
            i = y1;
            y1 = y2;
            y2 = i;
        }

        write_page->view_x1 = x1;
        write_page->view_y1 = y1;
        write_page->view_x2 = x2;
        write_page->view_y2 = y2;
        if ((passed & 2) == 0) {
            write_page->view_offset_x = x1;
            write_page->view_offset_y = y1;
        } else {
            write_page->view_offset_x = 0;
            write_page->view_offset_y = 0;
        }
        if (!write_page->clipping_or_scaling)
            write_page->clipping_or_scaling = 1;
    } else {
        // no arguments passed
        write_page->view_x1 = 0;
        write_page->view_y1 = 0;
        write_page->view_x2 = write_page->width - 1;
        write_page->view_y2 = write_page->height - 1;
        write_page->view_offset_x = 0;
        write_page->view_offset_y = 0;
        if (write_page->clipping_or_scaling == 1)
            write_page->clipping_or_scaling = 0;
    }

    // recalculate window values based on new viewport (if necessary)
    if (write_page->clipping_or_scaling == 2) { // WINDOW'ing in use
        write_page->scaling_x = ((float)(write_page->view_x2 - write_page->view_x1)) / (write_page->window_x2 - write_page->window_x1);
        write_page->scaling_y = ((float)(write_page->view_y2 - write_page->view_y1)) / (write_page->window_y2 - write_page->window_y1);
        write_page->scaling_offset_x = -write_page->window_x1 * write_page->scaling_x;
        write_page->scaling_offset_y = -write_page->window_y1 * write_page->scaling_y;
        if (write_page->window_y2 < write_page->window_y1)
            write_page->scaling_offset_y = -write_page->window_y2 * write_page->scaling_y + write_page->view_y2;
    }

    if (passed & 4) { // fillcolor
        qb32_boxfill(write_page->window_x1, write_page->window_y1, write_page->window_x2, write_page->window_y2, fillcolor);
    }

    if (passed & 8) { // bordercolor
        static int32 bx, by;
        by = write_page->view_y1 - 1;
        if ((by >= 0) && (by < write_page->height)) {
            for (bx = write_page->view_x1 - 1; bx <= write_page->view_x2; bx++) {
                if ((bx >= 0) && (bx < write_page->width)) {
                    pset(bx, by, bordercolor);
                }
            }
        }
        by = write_page->view_y2 + 1;
        if ((by >= 0) && (by < write_page->height)) {
            for (bx = write_page->view_x1 - 1; bx <= write_page->view_x2; bx++) {
                if ((bx >= 0) && (bx < write_page->width)) {
                    pset(bx, by, bordercolor);
                }
            }
        }
        bx = write_page->view_x1 - 1;
        if ((bx >= 0) && (bx < write_page->width)) {
            for (by = write_page->view_y1 - 1; by <= write_page->view_y2; by++) {
                if ((by >= 0) && (by < write_page->height)) {
                    pset(bx, by, bordercolor);
                }
            }
        }
        bx = write_page->view_x2 + 1;
        if ((bx >= 0) && (bx < write_page->width)) {
            for (by = write_page->view_y1 - 1; by <= (write_page->view_y2 + 1); by++) {
                if ((by >= 0) && (by < write_page->height)) {
                    pset(bx, by, bordercolor);
                }
            }
        }
    }

    return;
error:
    error(5);
    return;
}

// sub_clsDest moved to text.cpp

// sub_cls moved to text.cpp

// qbg_sub_locate moved to text.cpp

// input helper functions:
uint64 hexoct2uint64_value;

int32 hexoct2uint64(qbs *h) {
    // returns 0 = failed
    //         1 = HEX value (default if unspecified)
    //         2 = OCT value
    //         3 = BIN value
    static int32 i, i2;
    static uint64 result;
    result = 0;
    static int32 type;
    type = 0;
    hexoct2uint64_value = 0;
    if (!h->len)
        return 1;
    if (h->chr[0] != 38)
        return 0; // not "&"
    if (h->len == 1)
        return 1; // "&" received, but awaiting further input
    i = h->chr[1];
    if ((i == 72) || (i == 104))
        type = 1; // "H" or "h"
    if ((i == 79) || (i == 111))
        type = 2; // "O" or "o"
    if ((i == 66) || (i == 98))
        type = 3; // "B" or "b"
    if (!type)
        return 0;
    if (h->len == 2)
        return type;

    // unsigned _int64 max = 18446744073709551615   (decimal, 20 chars)
    //                     = 1111111 etc., max. 64x (binary, 64 chars)
    //                     = 1777777777777777777777 (octal, 22 chars)
    //                     = FFFFFFFFFFFFFFFF       (hex, 16 chars)

    if (type == 1) {
        if (h->len > 18)
            return 0; // larger than int64
        for (i = 2; i < h->len; i++) {
            result <<= 4;
            i2 = h->chr[i];
            //           0      -      9       /       A      -      F       /       a      -      f
            if (((i2 >= 48) && (i2 <= 57)) || ((i2 >= 65) && (i2 <= 70)) || ((i2 >= 97) && (i2 <= 102))) {
                if (i2 >= 97)
                    i2 -= 32;
                if (i2 >= 65)
                    i2 -= 7;
                i2 -= 48;
                // i2 is now a values between 0 and 15
                result += i2;
            } else
                return 0; // invalid character
        } // i
        hexoct2uint64_value = result;
        return 1;
    } // type == 1

    if (type == 2) {
        if (h->len > 24)
            return 0; // larger than int64
        if (h->len == 24) {
            if ((h->chr[2] != 48) && (h->chr[2] != 49))
                return 0; // larger than int64
        }
        for (i = 2; i < h->len; i++) {
            result <<= 3;
            i2 = h->chr[i];
            if ((i2 >= 48) && (i2 <= 55)) { // 0-7
                i2 -= 48;
                result += i2;
            } else
                return 0; // invalid character
        } // i
        hexoct2uint64_value = result;
        return 2;
    } // type == 2

    if (type == 3) {
        if (h->len > 66)
            return 0; // larger than int64
        for (i = 2; i < h->len; i++) {
            result <<= 1;
            i2 = h->chr[i];
            if ((i2 >= 48) && (i2 <= 49)) { // 0-1
                i2 -= 48;
                result += i2;
            } else
                return 0; // invalid character
        } // i
        hexoct2uint64_value = result;
        return 3;
    } // type == 3

    return 0;
}

// input method (complex, calls other qbs functions)
const char *uint64_max[] = {"18446744073709551615"};
const char *int64_max[] = {"9223372036854775807"};
const char *int64_max_neg[] = {"9223372036854775808"};
const char *single_max[] = {"3402823"};
const char *single_max_neg[] = {"1401298"};
const char *double_max[] = {"17976931"};
const char *double_max_neg[] = {"4940656"};
uint8 significant_digits[1024];
int32 num_significant_digits;

extern void *qbs_input_variableoffsets[257];
extern int32 qbs_input_variabletypes[257];
qbs *qbs_input_arguements[257];
int32 cursor_show_last;

void qbs_input(int32 numvariables, uint8 newline) {
    if (is_error_pending())
        return;
    int32 i, i2, i3, i4, i5, i6, chr;

    static int32 autodisplay_backup;
    autodisplay_backup = autodisplay;
    autodisplay = 1;

    static int32 source_backup;
    source_backup = func__source();
    sub__source(func__dest());

    // duplicate dest image so any changes can be reverted
    static int32 dest_image, dest_image_temp, dest_image_holding_cursor;
    dest_image = func__copyimage(func__dest(), NULL, NULL);
    if (dest_image == -1)
        error(516); // out of memory
    dest_image_temp = func__copyimage(func__dest(), NULL, NULL);
    if (dest_image_temp == -1)
        error(517); // out of memory
    static int32 dest_image_cursor_x, dest_image_cursor_y;
    dest_image_cursor_x = write_page->cursor_x;
    dest_image_cursor_y = write_page->cursor_y;
    dest_image_holding_cursor = write_page->holding_cursor;

    uint32 qbs_tmp_base = qbs_tmp_list_nexti;

    static int32 lineinput;
    lineinput = 0;
    if (qbs_input_variabletypes[1] & ISSTRING) {
        if (qbs_input_variabletypes[1] & 512) {
            qbs_input_variabletypes[1] = -512;
            lineinput = 1;
        }
    }

    cursor_show_last = write_page->cursor_show;
    write_page->cursor_show = 1;

    int32 addspaces;
    addspaces = 0;
    qbs *inpstr = qbs_new(0, 0);  // not temp so must be freed
    qbs *inpstr2 = qbs_new(0, 0); // not temp so must be freed
    qbs *key = qbs_new(0, 0);     // not temp so must be freed
    qbs *c = qbs_new(1, 0);       // not temp so must be freed

    for (i = 1; i <= numvariables; i++)
        qbs_input_arguements[i] = qbs_new(0, 0);

    // init all passed variables to 0 or ""
    for (i = 1; i <= numvariables; i++) {

        if (qbs_input_variabletypes[i] & ISSTRING) { // STRING
            if (((qbs *)qbs_input_variableoffsets[i])->fixed) {
                memset(((qbs *)qbs_input_variableoffsets[i])->chr, 32, ((qbs *)qbs_input_variableoffsets[i])->len);
            } else {
                ((qbs *)qbs_input_variableoffsets[i])->len = 0;
            }
        }

        if ((qbs_input_variabletypes[i] & ISOFFSETINBITS) == 0) { // reg. numeric variable
            memset(qbs_input_variableoffsets[i], 0, (qbs_input_variabletypes[i] & 511) >> 3);
        }

        // bit referenced?

    } // i

qbs_input_next:

    int32 argn, firstchr, toomany;
    toomany = 0;
    argn = 1;
    i = 0;
    i2 = 0;
    qbs_input_arguements[1]->len = 0;
    firstchr = 1;
qbs_input_sep_arg:

    if (i < inpstr->len) {

        if (inpstr->chr[i] == 44) { //","
            if (i2 != 1) {          // not in the middle of a string
                if (!lineinput) {
                    i2 = 0;
                    argn = argn + 1;
                    if (argn > numvariables) {
                        toomany = 1;
                        goto qbs_input_sep_arg_done;
                    }
                    qbs_input_arguements[argn]->len = 0;
                    firstchr = 1;
                    goto qbs_input_next_arg;
                }
            }
        }

        if (inpstr->chr[i] == 34) { //"
            if (firstchr) {
                if (!lineinput) {
                    i2 = 1; // requires closure
                    firstchr = 0;
                    goto qbs_input_next_arg;
                }
            }
            if (i2 == 1) {
                i2 = 2;
                goto qbs_input_next_arg;
            }
        }

        if (i2 == 2) {
            goto backspace; // INVALID! Cannot have any characters after a closed "..."
        }

        c->chr[0] = inpstr->chr[i];
        qbs_set(qbs_input_arguements[argn], qbs_add(qbs_input_arguements[argn], c));

        firstchr = 0;
    qbs_input_next_arg:;
        i++;
        goto qbs_input_sep_arg;
    }
qbs_input_sep_arg_done:
    if (toomany)
        goto backspace;

    // validate current arguments
    // ASSUME LEADING & TRAILING SPACES REMOVED!
    uint8 valid;
    uint8 neg;
    int32 completewith;
    int32 l;
    uint8 *cp, *cp2;
    uint64 max, max_neg, multiple, value;
    uint64 hexvalue;

    completewith = -1;
    valid = 1;
    l = qbs_input_arguements[argn]->len;
    cp = qbs_input_arguements[argn]->chr;
    neg = 0;

    if ((qbs_input_variabletypes[argn] & ISSTRING) == 0) {
        if ((qbs_input_variabletypes[argn] & ISFLOAT) == 0) {
            if ((qbs_input_variabletypes[argn] & 511) <= 32) { // cannot handle INTEGER64 variables using this method!
                int64 finalvalue;
                // it's an integer variable!
                finalvalue = 0;
                if (l == 0) {
                    completewith = 48;
                    goto typechecked_integer;
                }
                // calculate max & max_neg (i4 used to store number of bits)
                i4 = qbs_input_variabletypes[argn] & 511;
                max = 1;
                max <<= i4;
                max--;

                // check for hex/oct/bin
                if ((i3 = hexoct2uint64(qbs_input_arguements[argn]))) {
                    hexvalue = hexoct2uint64_value;
                    if (hexvalue > max) {
                        valid = 0;
                        goto typechecked;
                    }
                    // i. check max num of "digits" required to represent a value, if more exist cull excess
                    // ii. set completewith value (if necessary)
                    if (i3 == 1) {
                        value = max;
                        i = 0;
                        for (i2 = 1; i2 <= 16; i2++) {
                            if (value & 0xF)
                                i = i2;
                            value >>= 4;
                        }
                        if (l > (2 + i)) {
                            valid = 0;
                            goto typechecked;
                        }
                        if (l == 1)
                            completewith = 72; //"H"
                        if (l == 2)
                            completewith = 48; //"0"
                    }
                    if (i3 == 2) {
                        value = max;
                        i = 0;
                        for (i2 = 1; i2 <= 22; i2++) {
                            if (value & 0x7)
                                i = i2;
                            value >>= 3;
                        }
                        if (l > (2 + i)) {
                            valid = 0;
                            goto typechecked;
                        }
                        if (l == 1)
                            completewith = 79; //"O"
                        if (l == 2)
                            completewith = 48; //"0"
                    }
                    if (i3 == 3) {
                        value = max;
                        i = 0;
                        for (i2 = 1; i2 <= 64; i2++) {
                            if (value & 0x1)
                                i = i2;
                            value >>= 1;
                        }
                        if (l > (2 + i)) {
                            valid = 0;
                            goto typechecked;
                        }
                        if (l == 1)
                            completewith = 66; //"B"
                        if (l == 2)
                            completewith = 48; //"0"
                    }
                    finalvalue = hexvalue;
                    goto typechecked_integer;
                }

                // max currently contains the largest UNSIGNED value possible, adjust as necessary
                if (qbs_input_variabletypes[argn] & ISUNSIGNED) {
                    max_neg = 0;
                } else {
                    max >>= 1;
                    max_neg = max + 1;
                }
                // check for - sign
                i2 = 0;
                if ((qbs_input_variabletypes[argn] & ISUNSIGNED) == 0) {
                    if (cp[i2] == 45) { //"-"
                        if (l == 1) {
                            completewith = 48;
                            goto typechecked_integer;
                        }
                        i2++;
                        neg = 1;
                    }
                }
                // after a leading 0 no other digits are possible, return an error if this is the case
                if (cp[i2] == 48) {
                    if (l > (i2 + 1)) {
                        valid = 0;
                        goto typechecked;
                    }
                }
                // scan the "number"...
                multiple = 1;
                value = 0;
                for (i = l - 1; i >= i2; i--) {
                    i3 = cp[i] - 48;
                    if ((i3 >= 0) && (i3 <= 9)) {
                        value += multiple * i3;
                        if (qbs_input_variabletypes[argn] & ISUNSIGNED) {
                            if (value > max) {
                                valid = 0;
                                goto typechecked;
                            }
                        } else {
                            if (neg) {
                                if (value > max_neg) {
                                    valid = 0;
                                    goto typechecked;
                                }
                            } else {
                                if (value > max) {
                                    valid = 0;
                                    goto typechecked;
                                }
                            }
                        }
                    } else {
                        valid = 0;
                        goto typechecked;
                    }
                    multiple *= 10;
                } // next i
                if (neg)
                    finalvalue = -value;
                else
                    finalvalue = value;
            typechecked_integer:
                // set variable to finalvalue
                if ((qbs_input_variabletypes[argn] & ISOFFSETINBITS) == 0) { // reg. numeric variable
                    memcpy(qbs_input_variableoffsets[argn], &finalvalue, (qbs_input_variabletypes[argn] & 511) >> 3);
                }
                goto typechecked;
            }
        }
    }

    if (qbs_input_variabletypes[argn] & ISSTRING) {
        if (((qbs *)qbs_input_variableoffsets[argn])->fixed) {
            if (l > ((qbs *)qbs_input_variableoffsets[argn])->len) {
                valid = 0;
                goto typechecked;
            }
        }
        qbs_set((qbs *)qbs_input_variableoffsets[argn], qbs_input_arguements[argn]);
        goto typechecked;
    }

    // INTEGER64 type
    // int64 range:          \969223372036854775808 to  9223372036854775807
    // uint64 range: 0                    to 18446744073709551615
    if ((qbs_input_variabletypes[argn] & ISSTRING) == 0) {
        if ((qbs_input_variabletypes[argn] & ISFLOAT) == 0) {
            if ((qbs_input_variabletypes[argn] & 511) == 64) {
                if (l == 0) {
                    completewith = 48;
                    *(int64 *)qbs_input_variableoffsets[argn] = 0;
                    goto typechecked;
                }

                // check for hex/oct/bin
                if ((i3 = hexoct2uint64(qbs_input_arguements[argn]))) {
                    hexvalue = hexoct2uint64_value;
                    if (hexvalue > max) {
                        valid = 0;
                        goto typechecked;
                    }
                    // set completewith value (if necessary)
                    if (i3 == 1)
                        if (l == 1)
                            completewith = 72; //"H"
                    if (i3 == 2)
                        if (l == 1)
                            completewith = 79; //"O"
                    if (i3 == 3)
                        if (l == 1)
                            completewith = 66; //"B"
                    if (l == 2)
                        completewith = 48; //"0"
                    *(uint64 *)qbs_input_variableoffsets[argn] = hexvalue;
                    goto typechecked;
                }

                // check for - sign
                i2 = 0;
                if ((qbs_input_variabletypes[argn] & ISUNSIGNED) == 0) {
                    if (cp[i2] == 45) { //"-"
                        if (l == 1) {
                            completewith = 48;
                            *(int64 *)qbs_input_variableoffsets[argn] = 0;
                            goto typechecked;
                        }
                        i2++;
                        neg = 1;
                    }
                }
                // after a leading 0 no other digits are possible, return an error if this is the case
                if (cp[i2] == 48) {
                    if (l > (i2 + 1)) {
                        valid = 0;
                        goto typechecked;
                    }
                }
                // count how many digits are in the number
                i4 = 0;
                for (i = l - 1; i >= i2; i--) {
                    i3 = cp[i] - 48;
                    if ((i3 < 0) || (i3 > 9)) {
                        valid = 0;
                        goto typechecked;
                    }
                    i4++;
                } // i
                if (qbs_input_variabletypes[argn] & ISUNSIGNED) {
                    if (i4 < 20)
                        goto typechecked_int64;
                    if (i4 > 20) {
                        valid = 0;
                        goto typechecked;
                    }

                    cp2 = (uint8 *)uint64_max[0];
                } else {
                    if (i4 < 19)
                        goto typechecked_int64;
                    if (i4 > 19) {
                        valid = 0;
                        goto typechecked;
                    }
                    if (neg)
                        cp2 = (uint8 *)int64_max_neg[0];
                    else
                        cp2 = (uint8 *)int64_max[0];
                }
                // number of digits valid, but exact value requires checking
                cp = qbs_input_arguements[argn]->chr;
                for (i = 0; i < i4; i++) {
                    if (cp[i + i2] < cp2[i])
                        goto typechecked_int64;
                    if (cp[i + i2] > cp2[i]) {
                        valid = 0;
                        goto typechecked;
                    }
                }
            typechecked_int64:
                // add character 0 to end to make it a null terminated string
                c->chr[0] = 0;
                qbs_set(qbs_input_arguements[argn], qbs_add(qbs_input_arguements[argn], c));
                if (qbs_input_variabletypes[argn] & ISUNSIGNED) {
                    sscanf((char *)qbs_input_arguements[argn]->chr, "%" SCNu64, (uint64_t *)qbs_input_variableoffsets[argn]);
                } else {
                    sscanf((char *)qbs_input_arguements[argn]->chr, "%" SCNd64, (int64_t *)qbs_input_variableoffsets[argn]);
                }
                goto typechecked;
            }
        }
    }

    // check ISFLOAT type?
    //[-]9999[.]9999[E/D][+/-]99999
    if (qbs_input_variabletypes[argn] & ISFLOAT) {
        static int32 digits_before_point;
        static int32 digits_after_point;
        static int32 zeros_after_point;
        static int32 neg_power;
        digits_before_point = 0;
        digits_after_point = 0;
        neg_power = 0;
        value = 0;
        zeros_after_point = 0;
        num_significant_digits = 0;

        // set variable to 0
        if ((qbs_input_variabletypes[argn] & 511) == 32)
            *(float *)qbs_input_variableoffsets[argn] = 0;
        if ((qbs_input_variabletypes[argn] & 511) == 64)
            *(double *)qbs_input_variableoffsets[argn] = 0;
        if ((qbs_input_variabletypes[argn] & 511) == 256)
            *(long double *)qbs_input_variableoffsets[argn] = 0;

        // begin with a generic assessment, regardless of whether it is single, double or float
        if (l == 0) {
            completewith = 48;
            goto typechecked;
        }

        // check for hex/oct/bin
        if ((i3 = hexoct2uint64(qbs_input_arguements[argn]))) {
            hexvalue = hexoct2uint64_value;
            // set completewith value (if necessary)
            if (i3 == 1)
                if (l == 1)
                    completewith = 72; //"H"
            if (i3 == 2)
                if (l == 1)
                    completewith = 79; //"O"
            if (i3 == 3)
                if (l == 1)
                    completewith = 66; //"B"
            if (l == 2)
                completewith = 48; //"0"
            // nb. because VC6 didn't support...
            // error C2520: conversion from uint64 to double not implemented, use signed int64
            // I've implemented a work-around so correct values will be returned
            static int64 transfer;
            transfer = 0x7FFFFFFF;
            transfer <<= 32;
            transfer |= 0xFFFFFFFF;
            while (hexvalue > transfer) {
                hexvalue -= transfer;
                if ((qbs_input_variabletypes[argn] & 511) == 32)
                    *(float *)qbs_input_variableoffsets[argn] += transfer;
                if ((qbs_input_variabletypes[argn] & 511) == 64)
                    *(double *)qbs_input_variableoffsets[argn] += transfer;
                if ((qbs_input_variabletypes[argn] & 511) == 256)
                    *(long double *)qbs_input_variableoffsets[argn] += transfer;
            }
            transfer = hexvalue;
            if ((qbs_input_variabletypes[argn] & 511) == 32)
                *(float *)qbs_input_variableoffsets[argn] += transfer;
            if ((qbs_input_variabletypes[argn] & 511) == 64)
                *(double *)qbs_input_variableoffsets[argn] += transfer;
            if ((qbs_input_variabletypes[argn] & 511) == 256)
                *(long double *)qbs_input_variableoffsets[argn] += transfer;
            goto typechecked;
        }

        // check for - sign
        i2 = 0;
        if (cp[i2] == 45) { //"-"
            if (l == 1) {
                completewith = 48;
                goto typechecked;
            }
            i2++;
            neg = 1;
        }
        // if it starts with 0, it may only have one leading 0
        if (cp[i2] == 48) {
            if (l > (i2 + 1)) {
                i2++;
                if (cp[i2] == 46)
                    goto decimal_point;
                valid = 0;
                goto typechecked; // expected a decimal point
                // nb. of course, user could have typed D or E BUT there is no point
                //    calculating 0 to the power of anything!
            } else
                goto typechecked; // validate, as no other data is required
        }
        // scan digits before decimal place
        for (i = i2; i < l; i++) {
            i3 = cp[i];
            if ((i3 == 68) || (i3 == (68 + 32)) || (i3 == 69) || (i3 == (69 + 32))) { // d,D,e,E?
                if (i == i2) {
                    valid = 0;
                    goto typechecked;
                } // cannot begin with d,D,e,E!
                i2 = i;
                goto exponent;
            }
            if (i3 == 46) {
                i2 = i;
                goto decimal_point;
            } // nb. it can begin with a decimal point!
            i3 -= 48;
            if ((i3 < 0) || (i3 > 9)) {
                valid = 0;
                goto typechecked;
            }
            digits_before_point++;
            // nb. because leading 0 is handled differently, all digits are significant
            significant_digits[num_significant_digits] = i3 + 48;
            num_significant_digits++;
        }
        goto assess_float;
    ////////////////////////////////
    decimal_point:;
        i4 = 1;
        if (i2 == (l - 1)) {
            completewith = 48;
            goto assess_float;
        }
        i2++;
        for (i = i2; i < l; i++) {
            i3 = cp[i];
            if ((i3 == 68) || (i3 == (68 + 32)) || (i3 == 69) || (i3 == (69 + 32))) { // d,D,e,E?
                if (num_significant_digits) {
                    if (i == i2) {
                        valid = 0;
                        goto typechecked;
                    } // cannot begin with d,D,e,E just after a decimal point!
                    i2 = i;
                    goto exponent;
                }
            }
            i3 -= 48;
            if ((i3 < 0) || (i3 > 9)) {
                valid = 0;
                goto typechecked;
            }
            if (i3)
                i4 = 0;
            if (i4)
                zeros_after_point++;
            digits_after_point++;
            if ((num_significant_digits) || i3) {
                significant_digits[num_significant_digits] = i3 + 48;
                num_significant_digits++;
            }
        } // i
        goto assess_float;
    ////////////////////////////////
    exponent:;

        // ban d/D for SINGLE precision input
        if ((qbs_input_variabletypes[argn] & 511) == 32) { // SINGLE
            i3 = cp[i2];
            if ((i3 == 68) || (i3 == (68 + 32))) { // d/D
                valid = 0;
                goto typechecked;
            }
        }
        // correct "D" notation for c++ scanf
        i3 = cp[i2];
        if ((i3 == 68) || (i3 == (68 + 32))) { // d/D
            cp[i2] = 69;                       //"E"
        }
        if (i2 == (l - 1)) {
            completewith = 48;
            goto assess_float;
        }
        i2++;
        // check for optional + or -
        i3 = cp[i2];
        if (i3 == 45) { //"-"
            if (i2 == (l - 1)) {
                completewith = 48;
                goto assess_float;
            }
            neg_power = 1;
            i2++;
        }
        if (i3 == 43) { //"+"
            if (i2 == (l - 1)) {
                completewith = 48;
                goto assess_float;
            }
            i2++;
        }
        // nothing valid after a leading 0
        if (cp[i2] == 48) { // 0
            if (l > (i2 + 1)) {
                valid = 0;
                goto typechecked;
            }
        }
        multiple = 1;
        value = 0;
        for (i = l - 1; i >= i2; i--) {
            i3 = cp[i] - 48;
            if ((i3 >= 0) && (i3 <= 9)) {

                value += multiple * i3;
            } else {
                valid = 0;
                goto typechecked;
            }
            multiple *= 10;
        } // i
    //////////////////////////
    assess_float:;
        // nb. 0.???? means digits_before_point==0

        if ((qbs_input_variabletypes[argn] & 511) == 32) { // SINGLE
            // QB:           \B13.402823    E+38 to \B11.401298    E-45
            // WIKIPEDIA:    \B13.4028234   E+38 to ?
            // OTHER SOURCE: \B13.402823466 E+38 to \B11.175494351 E-38
            if (neg_power)
                value = -value;
            // special case->single 0 after point
            if ((zeros_after_point == 1) && (digits_after_point == 1)) {
                digits_after_point = 0;
                zeros_after_point = 0;
            }
            // upper overflow check
            // i. check that value doesn't consist solely of 0's
            if (zeros_after_point > 43) {
                valid = 0;
                goto typechecked;
            } // cannot go any further without reversal by exponent
            if ((digits_before_point == 0) && (digits_after_point == zeros_after_point))
                goto nooverflow_float;
            // ii. calculate the position of the first WHOLE digit (in i)
            i = digits_before_point;
            if (!i)
                i = -zeros_after_point;
            /*EXAMPLES:
                1.0         i=1
                12.0        i=2
                0.1         i=0
                0.01        i=-1
            */
            i = i + value; // apply exponent
            if (i > 39) {
                valid = 0;
                goto typechecked;
            }
            // nb. the above blocks the ability to type a long-int32 number and use a neg exponent
            //    to validate it
            //********IMPORTANT: if i==39 then the first 7 digits MUST be scanned!!!
            if (i == 39) {
                cp2 = (uint8 *)single_max[0];
                i2 = num_significant_digits;
                if (i2 > 7)
                    i2 = 7;
                for (i3 = 0; i3 < i2; i3++) {
                    if (significant_digits[i3] > *cp2) {
                        valid = 0;
                        goto typechecked;
                    }
                    if (significant_digits[i3] < *cp2)
                        break;
                    cp2++;
                }
            }
            // check for pointless levels of precision (eg. 1.21351273512653625116212!)
            if (digits_after_point) {
                if (digits_before_point) {
                    if ((digits_after_point + digits_before_point) > 8) {
                        valid = 0;
                        goto typechecked;
                    }
                } else {
                    if ((digits_after_point - zeros_after_point) > 8) {
                        valid = 0;
                        goto typechecked;
                    }
                }
            }
            // check for "under-flow"
            if (i < -44) {
                valid = 0;
                goto typechecked;
            }
            //********IMPORTANT: if i==-44 then the first 7 digits MUST be scanned!!!
            if (i == -44) {
                cp2 = (uint8 *)single_max_neg[0];
                i2 = num_significant_digits;
                if (i2 > 7)
                    i2 = 7;
                for (i3 = 0; i3 < i2; i3++) {
                    if (significant_digits[i3] < *cp2) {
                        valid = 0;
                        goto typechecked;
                    }
                    if (significant_digits[i3] > *cp2)
                        break;
                    cp2++;
                }
            }
        nooverflow_float:;
            c->chr[0] = 0;
            qbs_set(qbs_input_arguements[argn], qbs_add(qbs_input_arguements[argn], c));
            sscanf((char *)qbs_input_arguements[argn]->chr, "%f", (float *)qbs_input_variableoffsets[argn]);
            goto typechecked;
        }

        if ((qbs_input_variabletypes[argn] & 511) == 64) { // DOUBLE
            // QB: Double (15-digit) precision \B11.7976931 D+308 to \B14.940656 D-324
            // WIKIPEDIA:    \B11.7976931348623157 D+308 to ???
            // OTHER SOURCE: \B11.7976931348623157 D+308 to \B12.2250738585072014E-308

            if (neg_power)
                value = -value;
            // special case->single 0 after point
            if ((zeros_after_point == 1) && (digits_after_point == 1)) {
                digits_after_point = 0;
                zeros_after_point = 0;
            }
            // upper overflow check
            // i. check that value doesn't consist solely of 0's
            if (zeros_after_point > 322) {
                valid = 0;
                goto typechecked;
            } // cannot go any further without reversal by exponent
            if ((digits_before_point == 0) && (digits_after_point == zeros_after_point))
                goto nooverflow_double;
            // ii. calculate the position of the first WHOLE digit (in i)
            i = digits_before_point;
            if (!i)
                i = -zeros_after_point;
            i = i + value; // apply exponent
            if (i > 309) {
                valid = 0;
                goto typechecked;
            }
            // nb. the above blocks the ability to type a long-int32 number and use a neg exponent
            //    to validate it
            //********IMPORTANT: if i==309 then the first 8 digits MUST be scanned!!!
            if (i == 309) {
                cp2 = (uint8 *)double_max[0];
                i2 = num_significant_digits;
                if (i2 > 8)
                    i2 = 8;
                for (i3 = 0; i3 < i2; i3++) {
                    if (significant_digits[i3] > *cp2) {
                        valid = 0;
                        goto typechecked;
                    }
                    if (significant_digits[i3] < *cp2)
                        break;
                    cp2++;
                }
            }
            // check for pointless levels of precision (eg. 1.21351273512653625116212!)
            if (digits_after_point) {
                if (digits_before_point) {
                    if ((digits_after_point + digits_before_point) > 16) {
                        valid = 0;
                        goto typechecked;
                    }
                } else {
                    if ((digits_after_point - zeros_after_point) > 16) {
                        valid = 0;
                        goto typechecked;
                    }
                }
            }
            // check for "under-flow"
            if (i < -323) {
                valid = 0;
                goto typechecked;
            }
            //********IMPORTANT: if i==-323 then the first 7 digits MUST be scanned!!!
            if (i == -323) {
                cp2 = (uint8 *)double_max_neg[0];
                i2 = num_significant_digits;
                if (i2 > 7)
                    i2 = 7;
                for (i3 = 0; i3 < i2; i3++) {
                    if (significant_digits[i3] < *cp2) {
                        valid = 0;
                        goto typechecked;
                    }
                    if (significant_digits[i3] > *cp2)
                        break;
                    cp2++;
                }
            }
        nooverflow_double:;
            c->chr[0] = 0;
            qbs_set(qbs_input_arguements[argn], qbs_add(qbs_input_arguements[argn], c));
            sscanf((char *)qbs_input_arguements[argn]->chr, "%lf", (double *)qbs_input_variableoffsets[argn]);
            goto typechecked;
        }

        if ((qbs_input_variabletypes[argn] & 511) == 256) { // FLOAT
            // at present, there is no defined limit for FLOAT type numbers, so no restrictions
            // are applied!
            c->chr[0] = 0;
            qbs_set(qbs_input_arguements[argn], qbs_add(qbs_input_arguements[argn], c));

            // sscanf((char*)qbs_input_arguements[argn]->chr,"%lf",(long double*)qbs_input_variableoffsets[argn]);
            static double sscanf_fix;
            sscanf((char *)qbs_input_arguements[argn]->chr, "%lf", &sscanf_fix);
            *(long double *)qbs_input_variableoffsets[argn] = sscanf_fix;
        }

    } // ISFLOAT

// undefined/uncheckable types fall through as valid!
typechecked:;
    if (!valid)
        goto backspace;

    qbs_set(inpstr2, inpstr);

    // input a key

qbs_input_invalidinput:

    static int32 showing_cursor;
    showing_cursor = 0;

qbs_input_wait_for_key:

    // toggle box cursor
    if (!write_page->text) {
        i = 1;
        if ((write_page->font >= 32) || (write_page->compatible_mode == 256) || (write_page->compatible_mode == 32)) {
            if (GetTicks() & 512)
                i = 0;
        }
        if (i != showing_cursor) {
            showing_cursor ^= 1;
            static int32 x, y, x2, y2, fx, fy, alpha, cw;
            static uint32 c;
            alpha = write_page->alpha_disabled;
            write_page->alpha_disabled = 1;
            fy = fontheight[write_page->font];
            fx = fontwidth[write_page->font];
            if (!fx)
                fx = 1;
            cw = fx;
            if ((write_page->font >= 32) || (write_page->compatible_mode == 256) || (write_page->compatible_mode == 32))
                cw = 1;
            y2 = (write_page->cursor_y - 1) * fy;
            for (y = 0; y < fy; y++) {
                x2 = (write_page->cursor_x - 1) * fx;
                for (x = 0; x < cw; x++) {
                    pset(x2, y2, point(x2, y2) ^ write_page->color);
                    x2++;
                }
                y2++;
            }
            write_page->alpha_disabled = alpha;
        }
    } //! write_page->text

    if (addspaces) {
        addspaces--;
        c->chr[0] = 32;
        qbs_set(key, c);
    } else {

        if (write_page->console) {
            qbs_set(key, qbs_new_txt(""));
            chr = fgetc(stdin);
            if (chr != EOF) {
                if (chr == '\n')
                    chr = 13;
                qbs_set(key, qbs_new_txt(" "));
                key->chr[0] = chr;
            } else {
                Sleep(10);
            }
        } else {
            Sleep(10);
            qbs_set(key, qbs_inkey());
        }

        qbs_cleanup(qbs_tmp_base, 0);
    }
    if (stop_program)
        return;
    if (key->len != 1)
        goto qbs_input_wait_for_key;

    // remove box cursor
    if (!write_page->text) {
        if (showing_cursor) {
            showing_cursor = 0;
            static int32 x, y, x2, y2, fx, fy, cw, alpha;
            static uint32 c;
            alpha = write_page->alpha_disabled;
            write_page->alpha_disabled = 1;
            fy = fontheight[write_page->font];
            fx = fontwidth[write_page->font];
            if (!fx)
                fx = 1;
            cw = fx;
            if ((write_page->font >= 32) || (write_page->compatible_mode == 256) || (write_page->compatible_mode == 32))
                cw = 1;
            y2 = (write_page->cursor_y - 1) * fy;
            for (y = 0; y < fy; y++) {
                x2 = (write_page->cursor_x - 1) * fx;
                for (x = 0; x < cw; x++) {
                    pset(x2, y2, point(x2, y2) ^ write_page->color);
                    x2++;
                }
                y2++;
            }
            write_page->alpha_disabled = alpha;
        }
    } //! write_page->text

    // input should disallow certain characters
    if (key->chr[0] == 7) {
        qbs_print(key, 0);
        goto qbs_input_next;
    } // beep!
    if (key->chr[0] == 10)
        goto qbs_input_next; // linefeed
    if (key->chr[0] == 9) {  // tab
        i = 8 - (inpstr2->len & 7);
        addspaces = i;
        goto qbs_input_next;
    }
    // other ASCII chars that cannot be printed
    if (key->chr[0] == 11)
        goto qbs_input_next;
    if (key->chr[0] == 12)
        goto qbs_input_next;
    if (key->chr[0] == 28)
        goto qbs_input_next;
    if (key->chr[0] == 29)
        goto qbs_input_next;
    if (key->chr[0] == 30)
        goto qbs_input_next;
    if (key->chr[0] == 31)
        goto qbs_input_next;

    if (key->chr[0] == 13) {
        // assume input is valid

        // autofinish (if necessary)

        // assume all parts entered

        for (i = 1; i <= numvariables; i++) {
            qbs_free(qbs_input_arguements[i]);
        }

        if (newline) {
            c->len = 0;
            if (!write_page->console)
                qbs_print(c, 1);
        }
        qbs_free(c);
        qbs_free(inpstr);
        qbs_free(inpstr2);
        qbs_free(key);

        write_page->cursor_show = cursor_show_last;

        sub__freeimage(dest_image, 1);
        sub__freeimage(dest_image_temp, 1);

        if (autodisplay_backup == 0) {
            autodisplay = -1; // toggle request
            while (autodisplay)
                Sleep(1);
        }

        sub__source(source_backup);

        return;
    }

    if (key->chr[0] == 8) { // backspace
    backspace:
        if (!inpstr->len)
            goto qbs_input_invalidinput;
        inpstr->len--;
        i2 = func__dest();                      // backup current dest
        sub_pcopy(dest_image, dest_image_temp); // copy original background to temp
        // write characters to temp
        sub__dest(dest_image_temp);
        write_page->cursor_x = dest_image_cursor_x;
        write_page->cursor_y = dest_image_cursor_y;
        write_page->holding_cursor = dest_image_holding_cursor;
        for (i = 0; i < inpstr->len; i++) {
            key->chr[0] = inpstr->chr[i];
            qbs_print(key, 0);
        }
        sub__dest(i2);
        // copy temp to dest
        sub_pcopy(dest_image_temp, i2);
        // update cursor
        write_page->cursor_x = img[-dest_image_temp].cursor_x;
        write_page->cursor_y = img[-dest_image_temp].cursor_y;
        goto qbs_input_next;
    }

    // affect inpstr2 with key
    qbs_set(inpstr2, qbs_add(inpstr2, key));

    // perform actual update
    if (!write_page->console)
        qbs_print(key, 0);

    qbs_set(inpstr, inpstr2);

    goto qbs_input_next;

} // qbs_input

// H3C0_blink_enable, sub__blink, func__blink moved to utility.cpp
extern int32 H3C0_blink_enable;

// sub_out, func_inp, sub_wait moved to port_io.cpp

// func__handle, func__title, set_foreground_window, func__hasfocus moved to window.cpp

// sub_randomize, func_rnd, sub__fps moved to utility.cpp

// generic_put, generic_get_bytes_read, and generic_get moved to fileio.cpp

// sub_open, sub_open_gwbasic, and sub_close moved to fileio.cpp

// file_input_chr, file_input_skip1310, and file_input_nextitem moved to fileio.cpp

uint8 sub_file_print_spaces[32];

void sub_file_print(int32 i, qbs *str, int32 extraspace, int32 tab, int32 newline) {
    if (is_error_pending())
        return;
    static int32 x, x2, x3, x4;
    static int32 e;

    // tcp/ip?
    // note: spacing considerations such as 'extraspace' & 'tab' are ignored
    if (i < 0) {

        return;
    }
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);

    if (gfs->scrn == 1) {
        qbs_print(str, newline);
        return;
    };

    if (gfs->type != 4) {
        error(54);
        return;
    } // Bad file mode
    if (!gfs->write) {
        error(75);
        return;
    } // Path/file access error

    e = gfs_write(i, -1, str->chr, str->len);
    if (e) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -3) {
            error(54);
            return;
        } // bad file mode
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        if (e == -7) {
            error(70);
            return;
        } // permission denied
        error(75);
        return; // assume[-9]: path/file access error
    }

    // move column if carriage return found in content
    static int32 stri, strl;
    static uint8 c;
    strl = str->len;
    for (stri = 0; stri < strl; stri++) {
        c = str->chr[stri];
        if ((c != 13) && (c != 10)) {
            gfs->column++;
        } else {
            if (c == 13)
                gfs->column = 1;
        }
    }

    // add extra spaces as needed
    static int32 nspaces;
    static int16 cr_lf = 13 + 10 * 256;
    nspaces = 0;
    if (extraspace) {
        nspaces++;
        gfs->column++;
    }
    if (tab) {
        // a space MUST be added
        nspaces++;
        gfs->column++;
        x = (gfs->column - 1) % 14;
        if (x != 0) {
            x = 14 - x;
            nspaces += x;
            gfs->column += x;
        }
    }
    if (nspaces) {

        e = gfs_write(i, -1, sub_file_print_spaces, nspaces);
        if (e) {
            if (e == -2) {
                error(258);
                return;
            } // invalid handle
            if (e == -3) {
                error(54);
                return;
            } // bad file mode
            if (e == -4) {
                error(5);
                return;
            } // illegal function call
            if (e == -7) {
                error(70);
                return;
            } // permission denied
            error(75);
            return; // assume[-9]: path/file access error
        }
    }
    if (newline) {

        e = gfs_write(i, -1, (uint8 *)&cr_lf, 2);
        if (e) {
            if (e == -2) {
                error(258);
                return;
            } // invalid handle
            if (e == -3) {
                error(54);
                return;
            } // bad file mode
            if (e == -4) {
                error(5);
                return;
            } // illegal function call
            if (e == -7) {
                error(70);
                return;
            } // permission denied
            error(75);
            return; // assume[-9]: path/file access error
        }

        gfs->column = 1;
    }
}

// Number parsing globals and functions (range_*, n_*, n_roundincrement, n_float, n_int64,
// n_uint64, n_inputnumberfromdata, n_inputnumberfromfile) moved to fileio.cpp
// These are declared as extern in fileio.h and defined in fileio.cpp

void revert_input_check() {}

void sub_file_input_string(int32 fileno, qbs *deststr) {
    if (is_error_pending())
        return;
    static qbs *str, *character;
    int32 c, nextc, x, x2, x3, x4;
    int32 i, i1;
    int32 inspeechmarks;
    static uint8 *ucbuf;
    static uint32 ucbufsiz;
    static int32 info;

    // tcp/ip?
    // note: spacing considerations are ignored
    if (fileno < 0) {
        return;
    }

    if (gfs_fileno_valid(fileno) != 1) {
        error(52);
        return;
    } // Bad file name or number
    fileno = gfs_get_fileno(fileno); // convert fileno to gfs index

    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(fileno);
    if (gfs->type != 3) {
        error(54);
        return;
    } // Bad file mode
    if (!gfs->read) {
        error(75);
        return;
    } // Path/file access error

    str = qbs_new(0, 0);
    // skip whitespace (spaces & tabs)
    do {
        c = file_input_chr(fileno);
        if (c == -2)
            return;
        if (c == -1) {
            qbs_set(deststr, str);
            qbs_free(str);
            error(62); // input past end of file
            return;
        }
    } while ((c == 32) || (c == 9));
    // quoted string?
    inspeechmarks = 0;
    if (c == 34) { //"
        inspeechmarks = 1;
        c = file_input_chr(fileno);
    }
    // read string
    character = qbs_new(1, 0);
nextchr:
    if (c == -2)
        return;
    if (c == -1)
        goto gotstr;
    if (inspeechmarks) {
        if (c == 34)
            goto gotstr; //"
    } else {
        if (c == 44)
            goto gotstr; //,
        if (c == 10)
            goto gotstr;
        if (c == 13)
            goto gotstr;
    }
    character->chr[0] = c;
    qbs_set(str, qbs_add(str, character));
    c = file_input_chr(fileno);
    goto nextchr;
gotstr:
    // cull trailing whitespace
    if (!inspeechmarks) {
    cullstr:
        if (str->len) {
            if ((str->chr[str->len - 1] == 32) || (str->chr[str->len - 1] == 9)) {
                str->len--;
                goto cullstr;
            }
        }
    }
nextstr:
    // scan until next item (or eof) reached
    if (c == -2)
        return;
    if (c == -1)
        goto returnstr;
    if (c == 44)
        goto returnstr;
    if ((c == 10) || (c == 13)) { // lf cr
        file_input_skip1310(fileno, c);
        goto returnstr;
    }
    c = file_input_chr(fileno);
    goto nextstr;
// return string
returnstr:
    qbs_set(deststr, str);
    qbs_free(str);
    qbs_free(character);
    return;
}

int64 func_file_input_int64(int32 fileno) {
    if (is_error_pending())
        return 0;
    static int32 i;
    i = n_inputnumberfromfile(fileno);
    if (i == 1) {
        error(6);
        return 0;
    } // overflow
    if (i == 2) {
        error(62);
        return 0;
    } // input past end of file
    if (i == 3)
        return 0; // failed
    if (n_int64())
        return n_int64_value;
    error(6); // overflow
    return 0;
}

uint64 func_file_input_uint64(int32 fileno) {
    if (is_error_pending())
        return 0;
    static int32 i;
    i = n_inputnumberfromfile(fileno);
    if (i == 1) {
        error(6);
        return 0;
    } // overflow
    if (i == 2) {
        error(62);
        return 0;
    } // input past end of file
    if (i == 3)
        return 0; // failed
    if (n_uint64())
        return n_uint64_value;
    error(6); // overflow
    return 0;
}

void sub_read_string(uint8 *data, ptrszint *data_offset, ptrszint data_size, qbs *deststr) {
    if (is_error_pending())
        return;
    static qbs *str, *character;
    static int32 c, inspeechmarks;
    str = qbs_new(0, 0);
    character = qbs_new(1, 0);
    inspeechmarks = 0;

    if (*data_offset >= data_size) {
        error(4);
        goto gotstr;
    } // Out of DATA

    c = data[*data_offset];
nextchr:

    if (c == 44) { //,
        if (inspeechmarks != 1) {
            (*data_offset)++;
            goto gotstr;
        }
    }
    if (inspeechmarks == 2) {
        error(4);
        str->len = 0;
        goto gotstr;
    } // syntax error (expected , after closing " unless at end of data in which " is assumed by QB)

    if (c == 34) { //"
        if (inspeechmarks) {
            inspeechmarks = 2;
            goto skipchr;
        }
        if (!str->len) {
            inspeechmarks = 1;
            goto skipchr;
        }
    }

    character->chr[0] = c;
    qbs_set(str, qbs_add(str, character));
skipchr:

    (*data_offset)++;
    if (*data_offset >= data_size)
        goto gotstr;
    c = data[*data_offset];
    goto nextchr;

gotstr:
    qbs_set(deststr, str);
    qbs_free(str);
    qbs_free(character);
    return;
}

long double func_read_float(uint8 *data, ptrszint *data_offset, ptrszint data_size, int32 typ) {
    if (is_error_pending())
        return 0;
    static int32 i;
    static int64 maxval, minval;
    static int64 value;
    static ptrszint old_data_offset;
    old_data_offset = *data_offset;
    i = n_inputnumberfromdata(data, data_offset, data_size);

    // return values:
    // 0=success!
    // 1=Overflow
    // 2=Out of DATA
    // 3=Syntax error
    // note: when read fails the data_offset MUST be restored to its old position
    if (i == 1) { // Overflow
        goto overflow;
    }
    if (i == 2) { // Out of DATA
        error(4);
        return 0;
    }
    if (i == 3) { // Syntax error
        *data_offset = old_data_offset;
        error(2);
        return 0;
    }

    if (!n_float())
        goto overflow; // returns n_float_value

    // range test & return value
    if (typ & ISFLOAT) {
        if ((typ & 511) >= 64)
            return n_float_value;
        if (n_float_value > 3.402823466E+38)
            goto overflow;
        return n_float_value;
    } else {
        if (n_float_value < (-(9.2233720368547758E+18)))
            goto overflow; // too low to store!
        if (n_float_value > 9.2233720368547758E+18)
            goto overflow; // too high to store!
        value = qbr(n_float_value);
        if ((typ & ISUNSIGNED) || n_hex) {
            maxval = (((int64)1) << (typ & 511)) - 1;
            minval = 0;
        } else {
            maxval = (((int64)1) << ((typ & 511) - 1)) - 1;
            minval = -(((int64)1) << ((typ & 511) - 1));
        }
        if ((value > maxval) || (value < minval))
            goto overflow;

        if (((typ & ISUNSIGNED) == 0) && n_hex) {            // signed hex/oct/bin/...
            if ((((int64)1) << ((typ & 511) - 1)) & value) { // if top bit is set, set all bits above it to form a negative value
                value = (maxval ^ ((int64)-1)) + value;
            }
        }

        return value;
    }

overflow:
    *data_offset = old_data_offset;
    error(6);
    return 0;
}

int64 func_read_int64(uint8 *data, ptrszint *data_offset, ptrszint data_size) {
    if (is_error_pending())
        return 0;
    static int32 i;
    static ptrszint old_data_offset;
    old_data_offset = *data_offset;
    i = n_inputnumberfromdata(data, data_offset, data_size);
    // return values:
    // 0=success!
    // 1=Overflow
    // 2=Out of DATA
    // 3=Syntax error
    // note: when read fails the data_offset MUST be restored to its old position
    if (i == 1) { // Overflow
        goto overflow;
    }
    if (i == 2) { // Out of DATA
        error(4);
        return 0;
    }
    if (i == 3) { // Syntax error
        *data_offset = old_data_offset;
        error(2);
        return 0;
    }
    if (n_int64())
        return n_int64_value;
overflow:
    *data_offset = old_data_offset;
    error(6);
    return 0;
}

uint64 func_read_uint64(uint8 *data, ptrszint *data_offset, ptrszint data_size) {
    if (is_error_pending())
        return 0;
    static int32 i;
    static ptrszint old_data_offset;
    old_data_offset = *data_offset;
    i = n_inputnumberfromdata(data, data_offset, data_size);
    // return values:
    // 0=success!
    // 1=Overflow
    // 2=Out of DATA
    // 3=Syntax error
    // note: when read fails the data_offset MUST be restored to its old position
    if (i == 1) { // Overflow
        goto overflow;
    }
    if (i == 2) { // Out of DATA
        error(4);
        return 0;
    }
    if (i == 3) { // Syntax error
        *data_offset = old_data_offset;
        error(2);
        return 0;
    }
    if (n_uint64())
        return n_uint64_value;
overflow:
    *data_offset = old_data_offset;
    error(6);
    return 0;
}

long double func_file_input_float(int32 fileno, int32 typ) {
    if (is_error_pending())
        return 0;
    static int32 i;
    static int64 maxval, minval;
    static int64 value;
    i = n_inputnumberfromfile(fileno);
    if (i == 1) {
        error(6);
        return 0;
    } // overflow
    if (i == 2) {
        error(62);
        return 0;
    } // input past end of file
    if (i == 3)
        return 0; // failed
    if (!n_float()) {
        error(6);
        return 0;
    } // returns n_float_value
    // range test & return value
    if (typ & ISFLOAT) {
        if ((typ & 511) >= 64)
            return n_float_value;
        if (n_float_value > 3.402823466E+38) {
            error(6);
            return 0;
        }
        return n_float_value;
    } else {
        if (n_float_value < (-(9.2233720368547758E+18))) {
            error(6);
            return 0;
        } // too low to store!
        if (n_float_value > 9.2233720368547758E+18) {
            error(6);
            return 0;
        } // too high to store!
        value = qbr(n_float_value);
        if (typ & ISUNSIGNED) {
            maxval = (((int64)1) << (typ & 511)) - 1;
            minval = 0;
        } else {
            maxval = (((int64)1) << ((typ & 511) - 1)) - 1;
            minval = -(((int64)1) << ((typ & 511) - 1));
        }
        if ((value > maxval) || (value < minval)) {
            error(6);
            return 0;
        }

        if (((typ & ISUNSIGNED) == 0) && n_hex) {            // signed hex/oct/bin/...
            if ((((int64)1) << ((typ & 511) - 1)) & value) { // if top bit is set, set all bits above it to form a negative value
                value = (maxval ^ ((int64)-1)) + value;
            }
        }

        return value;
    }
} // func_file_input_float

void *byte_element(uint64 offset, int32 length) {
    if (length < 0)
        length = 0; // some calculations may result in negative values which mean 0 (no bytes available)
    // add structure to xms stack (byte_element structures are never stored in cmem!)
    void *p;
    if ((mem_static_pointer += 12) < mem_static_limit)
        p = mem_static_pointer - 12;
    else
        p = mem_static_malloc(12);
    *((uint64 *)p) = offset;
    ((uint32 *)p)[2] = length;
    return p;
}

void *byte_element(uint64 offset, int32 length, byte_element_struct *info) {
    if (length < 0)
        length = 0; // some calculations may result in negative values which mean 0 (no bytes available)
    info->length = length;
    info->offset = offset;
    return info;
}

void call_interrupt(int32 intno, void *inregs, void *outregs) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static uint16 *sp;
    /* testing only
        static qbs* s=NULL;
        if (s==NULL) s=qbs_new(0,0);
        qbs_set(s,qbs_str(ele->length));
        MessageBox2(NULL,(char*)s->chr,"CALL INTERRUPT: size",MB_OK|MB_SYSTEMMODAL);
        qbs_set(s,qbs_str( ((uint8*)(ele->offset))[0] ));
        MessageBox2(NULL,(char*)s->chr,"CALL INTERRUPT: value",MB_OK|MB_SYSTEMMODAL);
    */
    /* reference
        TYPE RegType
        AX AS INTEGER
        BX AS INTEGER
        CX AS INTEGER
        DX AS INTEGER
        BP AS INTEGER
        SI AS INTEGER
        DI AS INTEGER
        FLAGS AS INTEGER
        END TYPE
    */
    // error checking
    ele = (byte_element_struct *)outregs;
    if (ele->length < 16) {
        error(5);
        return;
    } // Illegal function call
    ele = (byte_element_struct *)inregs;
    if (ele->length < 16) {
        error(5);
        return;
    } // Illegal function call
    // load virtual registers
    sp = (uint16 *)(ele->offset);
    cpu.ax = sp[0];
    cpu.bx = sp[1];
    cpu.cx = sp[2];
    cpu.dx = sp[3];
    cpu.bp = sp[4];
    cpu.si = sp[5];
    cpu.di = sp[6];
    // note: flags ignored (revise)
    call_int(intno);
    // save virtual registers
    ele = (byte_element_struct *)outregs;
    sp = (uint16 *)(ele->offset);
    sp[0] = cpu.ax;
    sp[1] = cpu.bx;
    sp[2] = cpu.cx;
    sp[3] = cpu.dx;
    sp[4] = cpu.bp;
    sp[5] = cpu.si;
    sp[6] = cpu.di;
    // note: flags ignored (revise)
    return;
}

void call_interruptx(int32 intno, void *inregs, void *outregs) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static uint16 *sp;
    /* reference
        TYPE RegTypeX
        AX AS INTEGER
        BX AS INTEGER
        CX AS INTEGER
        DX AS INTEGER
        BP AS INTEGER
        SI AS INTEGER
        DI AS INTEGER
        FLAGS AS INTEGER
        DS AS INTEGER
        ES AS INTEGER
        END TYPE
    */
    // error checking
    ele = (byte_element_struct *)outregs;
    if (ele->length < 20) {
        error(5);
        return;
    } // Illegal function call
    ele = (byte_element_struct *)inregs;
    if (ele->length < 20) {
        error(5);
        return;
    } // Illegal function call
    // load virtual registers
    sp = (uint16 *)(ele->offset);
    cpu.ax = sp[0];
    cpu.bx = sp[1];
    cpu.cx = sp[2];
    cpu.dx = sp[3];
    cpu.bp = sp[4];
    cpu.si = sp[5];
    cpu.di = sp[6];
    // note: flags ignored (revise)
    cpu.ds = sp[8];
    cpu.es = sp[9];
    call_int(intno);
    // save virtual registers
    ele = (byte_element_struct *)outregs;
    sp = (uint16 *)(ele->offset);
    sp[0] = cpu.ax;
    sp[1] = cpu.bx;
    sp[2] = cpu.cx;
    sp[3] = cpu.dx;
    sp[4] = cpu.bp;
    sp[5] = cpu.si;
    sp[6] = cpu.di;
    // note: flags ignored (revise)
    sp[8] = cpu.ds;
    sp[9] = cpu.es;
    return;
}

void sub_get(int32 i, int64 offset, void *element, int32 passed) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static int32 x, x2;

    if (i < 0) { // special handle?
        stream_struct *st;
        special_handle_struct *sh;

        x = -(i + 1);

        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh) {
            error(52);
            return;
        }

        switch (sh->type) {
        case special_handle_type::Stream:
            st = (stream_struct *)sh->index;

            stream_update(st);
            ele = (byte_element_struct *)element;
            if (st->in_size < ele->length) {
                st->eof = 1;
                return;
            }

            st->eof = 0;
            memcpy((void *)(ele->offset), st->in, ele->length);
            x2 = st->in_size - ele->length;
            if (x2)
                memmove(st->in, st->in + ele->length, x2);
            st->in_size -= ele->length;
            break;

        case special_handle_type::Http:
            ele = (byte_element_struct *)element;

            // If there's not enough data in the internal buffer to fill this
            // fixed length request, then report EOF
            if (libqb_http_get_fixed(x, (char *)ele->offset, ele->length) == -1)
                sh->index = 1;
            else
                sh->index = 2; // Force EOF to report we're not at the end

            break;

        default:
            error(52);
            break;
        }

        return;
    } // special handle

    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->type > 2) {
        error(54);
        return;
    } // Bad file mode
    if (!gfs->read) {
        error(75);
        return;
    } // Path/file access error

    ele = (byte_element_struct *)element;

    if (gfs->type == 1) { // RANDOM
        if (ele->length > gfs->record_length) {
            error(59);
            return;
        } // Bad record length
        if (passed) {
            offset--;
            if (offset < 0) {
                error(63);
                return;
            } // Bad record number
            offset *= gfs->record_length;
        } else {
            offset = -1;
        }
    } else { // BINARY
        if (passed) {
            offset--;
            if (offset < 0) {
                error(63);
                return;
            } // Bad record number
        } else {
            offset = -1;
        }
    }

    static int32 e;

    e = gfs_read(i, offset, (uint8 *)ele->offset, ele->length);
    if (e) {
        if (e != -10) { // note: on eof, unread buffer area becomes NULL
            if (e == -2) {
                error(258);
                return;
            } // invalid handle
            if (e == -3) {
                error(54);
                return;
            } // bad file mode
            if (e == -4) {
                error(5);
                return;
            } // illegal function call
            if (e == -7) {
                error(70);
                return;
            } // permission denied
            error(75);
            return; // assume[-9]: path/file access error
        }
    }

    // seek to beginning of next field
    if (gfs->type == 1) {
        if (e != -10) { // note: seek index not advanced if record did not exist
            if (ele->length < gfs->record_length) {
                if (offset != -1) {
                    e = gfs_setpos(i, offset + gfs->record_length);
                } else {
                    e = gfs_setpos(i, gfs_getpos(i) - ele->length + gfs->record_length);
                }
                if (e) {
                    error(54);
                    return;
                } // assume[-3]: bad file mode
            }
        } // e!=-10
    }

} // get

void sub_get2(int32 i, int64 offset, qbs *str, int32 passed) {
    if (is_error_pending())
        return;
    static int32 x, x2, x3, x4;

    if (i < 0) {          // special handle?
        if (str->fixed) { // following method is only for variable length strings
            static byte_element_struct tbyte_element_struct;
            sub_get(i, offset, byte_element((uint64)str->chr, str->len, &tbyte_element_struct), passed);
            return;
        }
        x = -(i + 1);
        static stream_struct *st;
        static qbs *tqbs;
        static special_handle_struct *sh;
        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh) {
            error(52);
            return;
        }

        size_t length = 0;

        switch (sh->type) {
        case special_handle_type::Stream:
            st = (stream_struct *)sh->index;
            stream_update(st);

            tqbs = qbs_new(st->in_size, 1);
            if (st->in_size)
                memcpy(tqbs->chr, st->in, st->in_size);

            st->in_size = 0;
            st->eof = 0;
            qbs_set(str, tqbs);
            break;

        case special_handle_type::Http:
            sh->index = 0; // Reset EOF flag, this allows EOF to be calculated as normal

            libqb_http_get_length(x, &length);

            tqbs = qbs_new(length, 1);
            if (length)
                length = libqb_http_get(x, (char *)tqbs->chr, &length);

            qbs_set(str, tqbs);
            break;

        default:
            error(52);
            break;
        }

        return;
    } // special handle

    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->type > 2) {
        error(54);
        return;
    } // Bad file mode
    if (!gfs->read) {
        error(75);
        return;
    } // Path/file access error

    if (gfs->type == 2) { // BINARY (use normal sub_get)
        static byte_element_struct tbyte_element_struct;
        sub_get(gfs->fileno, offset, byte_element((uint64)str->chr, str->len, &tbyte_element_struct), passed);
        return;
    }

    if (gfs->record_length < 2) {
        error(59);
        return;
    } // Bad record length

    if (passed) {
        offset--;
        if (offset < 0) {
            error(63);
            return;
        } // Bad record number
        offset *= gfs->record_length;
    } else {
        offset = -1;
    }

    static int32 e;

    static uint8 *data;
    static uint64 l, bytes;
    data = (uint8 *)malloc(gfs->record_length);
    e = gfs_read(i, offset, data, gfs->record_length); // read the whole record (including header & data)
    if (e) {
        if (e != -10) { // note: on eof, unread buffer area becomes NULL
            if (e == -2) {
                error(258);
                return;
            } // invalid handle
            if (e == -3) {
                error(54);
                return;
            } // bad file mode
            if (e == -4) {
                error(5);
                return;
            } // illegal function call
            if (e == -7) {
                error(70);
                return;
            } // permission denied
            error(75);
            return; // assume[-9]: path/file access error
        }
    }

    bytes = gfs_read_bytes(); // note: any unread part of the buffer is set to NULL (by gfs_read) and is treated as valid record data
    if (!bytes) {
        qbs_set(str, qbs_new(0, 1));
        free(data);
        return;
    } // as in QB when 0 bytes read, NULL string returned and (as no bytes read) no seek advancement

    // seek to beginning of next field
    // note: advancement occurs even if e==-10 (eof reached)
    if (bytes < gfs->record_length) {
        if (offset != -1) {
            e = gfs_setpos(i, offset + gfs->record_length);
        } else {
            e = gfs_setpos(i, gfs_getpos(i) - bytes + gfs->record_length);
        }
        if (e) {
            error(54);
            free(data);
            return;
        } // assume[-3]: bad file mode
    }

    x = 2; // offset where string data will be read from
    l = ((uint16 *)data)[0];
    if (l & 32768) {
        if (gfs->record_length < 8) { // record length is too small to read the length!
            // restore seek to original location
            if (offset != -1) {
                e = gfs_setpos(i, offset);
            } else {
                e = gfs_setpos(i, gfs_getpos(i) - gfs->record_length);
            }
            error(59);
            free(data);
            return; // Bad record length
        }
        x = 8;
        l = (l & 0x7FFF) + (((((uint64 *)data)[0]) >> 16) << 15);
    }

    // can record_length cannot fit the required string data?
    if ((gfs->record_length - x2) < l) {
        // restore seek to original location
        if (offset != -1) {
            e = gfs_setpos(i, offset);
        } else {
            e = gfs_setpos(i, gfs_getpos(i) - gfs->record_length);
        }
        error(59);
        free(data);
        return; // Bad record length
    }

    qbs_set(str, qbs_new_txt_len((char *)(data + x), l));
    free(data);
}

void sub_put(int32 i, int64 offset, void *element, int32 passed) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static int32 x, x2;

    if (i < 0) { // special handle?
        x = -(i + 1);
        static special_handle_struct *sh;
        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh) {
            error(52);
            return;
        }

        static stream_struct *st;

        switch (sh->type) {
        case special_handle_type::Stream:
            st = (stream_struct *)sh->index;
            ele = (byte_element_struct *)element;
            stream_out(st, (void *)ele->offset, ele->length);
            break;

        default:
            error(52);
            break;
        }

        return;
    } // special handle

    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->type > 2) {
        error(54);
        return;
    } // Bad file mode
    if (!gfs->write) {
        error(75);
        return;
    } // Path/file access error

    ele = (byte_element_struct *)element;

    if (gfs->type == 1) { // RANDOM
        if (ele->length > gfs->record_length) {
            error(59);
            return;
        } // Bad record length
        if (passed) {
            offset--;
            if (offset < 0) {
                error(63);
                return;
            } // Bad record number
            offset *= gfs->record_length;
        } else {
            offset = -1;
        }
    } else { // BINARY
        if (passed) {
            offset--;
            if (offset < 0) {
                error(63);
                return;
            } // Bad record number
        } else {
            offset = -1;
        }
    }

    static int32 e;

    e = gfs_write(i, offset, (uint8 *)ele->offset, ele->length);
    if (e) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -3) {
            error(54);
            return;
        } // bad file mode
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        if (e == -7) {
            error(70);
            return;
        } // permission denied
        error(75);
        return; // assume[-9]: path/file access error
    }

    // seek to beginning of next field
    if (gfs->type == 1) {
        if (ele->length < gfs->record_length) {
            if (offset != -1) {
                e = gfs_setpos(i, offset + gfs->record_length);
            } else {
                e = gfs_setpos(i, gfs_getpos(i) - ele->length + gfs->record_length);
            }
            if (e)
                error(54);
            return; // assume[-3]: bad file mode
        }
    }
}

// put2 adds a 2-4 byte length descriptor to the data
//(used to PUT variable length strings in RANDOM mode)
void sub_put2(int32 i, int64 offset, void *element, int32 passed) {
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    static int32 x;
    static uint8 *data;

    if (i < 0) {                             // special handle?
        sub_put(i, offset, element, passed); //(use standard put call)
        return;
    }

    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->type > 2) {
        error(54);
        return;
    } // Bad file mode
    if (!gfs->write) {
        error(75);
        return;
    } // Path/file access error

    if (gfs->type == 2) { // BINARY (use normal sub_put)
        sub_put(gfs->fileno, offset, element, passed);
        return;
    }

    // RANDOM
    static uint64 l;
    static int64 lmask;
    lmask = -1;
    lmask >>= 16;
    ele = (byte_element_struct *)element;
    l = ele->length; // note: ele->length is currently 32-bit, but sub_put2 is 64-bit compliant
    //{15}{1}[{48}]
    if (l > 32767) {
        data = (uint8 *)malloc(l + 8);
        memcpy(&data[8], (void *)(ele->offset), l);
        ((uint64 *)data)[0] = 0;
        ((uint16 *)data)[0] = (l & 32767) + 32768;
        l = ((l >> 15) & lmask);
        ((uint64 *)(data + 2))[0] |= l;
        ele->length += 8;
    } else {
        data = (uint8 *)malloc(l + 2);
        memcpy(&data[2], (void *)(ele->offset), l);
        ((uint16 *)data)[0] = l;
        ele->length += 2;
    }
    ele->offset = (uint64)&data[0];
    sub_put(gfs->fileno, offset, element, passed);
    free(data);

} // put2

// sub_graphics_get and sub_graphics_put moved to graphics.cpp

// func_csrlin, func_pos moved to text.cpp

int32 sleep_break = 0;

void sub_sleep(int32 seconds, int32 passed) {
    if (is_error_pending())
        return;

    sleep_break = 0;
    double prev, ms, now, elapsed; // cannot be static
    prev = GetTicks();
    ms = 1000.0 * (double)seconds;

#ifdef QB64_WINDOWS

    if (read_page->console) {
        int32 junk = 0, junk2 = 0;
        DWORD dwRet;
        HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
        FlushConsoleInputBuffer(hStdin);

        do {
            uint32_t sleepms = 10;

            // Check if ms is less than 10, if so sleep for that many ms to
            // try to make the sleep a bit more accurate
            if (passed && ms < 10 && ms > 0)
                sleepms = ms;

            // Only sleep for a max of 10ms intervals so that we can pump the evnt() handler
            dwRet = WaitForSingleObject(hStdin, sleepms); // this should provide our pause

            if (dwRet == WAIT_OBJECT_0) { // this says the console had input
                junk = func__getconsoleinput();
                if (junk == 1) {                     // this is a valid keyboard event.  Let's exit SLEEP in the console.
                    Sleep(100);                      // Give the user time to remove their finger from the key, before clearing the buffer.
                    FlushConsoleInputBuffer(hStdin); // flush the keyboard buffer after, so we don't leave stray events to be processed (such as key up events).
                    return;
                } else { // we had an input event such as the mouse.  Ignore it and clear the buffer so we don't keep responding to mouse inputs
                    FlushConsoleInputBuffer(hStdin);
                }
            }

            // Handle periodic events (timers and some other things)
            evnt(0);

            // evnt() may trigger us to end sleep early
            if (sleep_break)
                return;
            if (stop_program)
                return;

            // We only update the ms count if we were provided a max number of
            // seconds to sleep
            if (passed) {
                now = GetTicks();
                if (now < prev)
                    return;           // value looped?
                elapsed = now - prev; // elapsed time since prev
                ms = ms - elapsed;
                prev = now;
            }

        } while (!passed || ms > 0); // We loop if we weren't provided a number of seconds to sleep, or if we reached the timeout

        // If we got here, then we timed out
        return;
    }
#endif

recalculate:
wait:
    evnt(0); // handle general events
    // exit condition checks/events
    if (sleep_break)
        return;
    if (stop_program)
        return;
    if (ms <= 0) { // untimed SLEEP
        Sleep(9);
        goto wait;
    }
    now = GetTicks();
    if (now < prev) { // value looped?
        return;
    }
    elapsed = now - prev; // elapsed time since prev
    if (elapsed < ms) {
        int64 wait; // cannot be static
        wait = ms - elapsed;
        if (!wait)
            wait = 1;
        if (wait >= 10) {
            Sleep(9);
            // recalculate time
            goto recalculate;
        } else {
            Sleep(wait);
        }
    }
    return;
}

ptrszint func_lbound(ptrszint *array, int32 index, int32 num_indexes) {
    if ((index < 1) || (index > num_indexes) || ((array[2] & 1) == 0)) {
        error(9);
        return 0;
    }
    index = num_indexes - index + 1;
    return array[4 * index];
}

ptrszint func_ubound(ptrszint *array, int32 index, int32 num_indexes) {
    if ((index < 1) || (index > num_indexes) || ((array[2] & 1) == 0)) {
        error(9);
        return 0;
    }
    index = num_indexes - index + 1;
    return array[4 * index] + array[4 * index + 1] - 1;
}

// Keyboard scancode buffer for port I/O emulation
uint8 port60h_event[256];
int32 port60h_events = 0;

// func_inp, sub_wait moved to port_io.cpp (sub_out comment is above)

// func_tab, func_spc moved to text.cpp

float func_pmap(float val, int32 option) {
    static int32 x, y;
    if (is_error_pending())
        return 0;
    if (!write_page->text) {
        // note: for QBASIC/4.5/7.1 compatibility clipping_or_scaling check is skipped
        if (option == 0) {
            x = qbr_float_to_long(val * write_page->scaling_x + write_page->scaling_offset_x);
            return x;
        }
        if (option == 1) {
            y = qbr_float_to_long(val * write_page->scaling_y + write_page->scaling_offset_y);
            return y;
        }
        if (option == 2) {
            return (((double)qbr_float_to_long(val)) - write_page->scaling_offset_x) / write_page->scaling_x;
        }
        if (option == 3) {
            return (((double)qbr_float_to_long(val)) - write_page->scaling_offset_y) / write_page->scaling_y;
        }
    } //! write_page->text
    error(5);
    return 0;
}

uint32 func_screen(int32 y, int32 x, int32 returncol, int32 passed) {

    static uint8 chr[65536];
    static int32 x2, y2, x3, y3, i, i2, i3;
    static uint32 col, firstcol;
    uint8 *cp;

    if (!passed)
        returncol = 0;

#ifdef QB64_WINDOWS
    if (read_page->console) {
        SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
        HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
        COORD cp1 = {(SHORT)(x - 1), (SHORT)(y - 1)};
        DWORD t;
        uint16 a;
        if (returncol) {
            int x1 = ReadConsoleOutputAttribute(cl_conout, &a, 1, cp1, &t);
            return a;
        } else {
            ReadConsoleOutputCharacterA(cl_conout, (char *)&a, 1, cp1, &t);
            return a & 0xff;
        }
    }
#endif

    if (read_page->text) {
        // on screen?
        if ((y < 1) || (y > read_page->height)) {
            error(5);
            return 0;
        }
        if ((x < 1) || (x > read_page->width)) {
            error(5);
            return 0;
        }
        if (returncol) {
            return read_page->offset[((y - 1) * read_page->width + x - 1) * 2 + 1];
        }
        return read_page->offset[((y - 1) * read_page->width + x - 1) * 2];
    }

    // only 8x8,8x14,8x16 supported
    if ((read_page->font != 8) && (read_page->font != 14) && (read_page->font != 16)) {
        error(5);
        return 0;
    }

    // on screen?
    x2 = read_page->width / fontwidth[read_page->font];
    y2 = read_page->height / fontheight[read_page->font];
    if ((y < 1) || (y > y2)) {
        error(5);
        return 0;
    }
    if ((x < 1) || (x > x2)) {
        error(5);
        return 0;
    }

    // create "signature" of character on screen
    x--;
    y--;
    i = 0;
    i3 = 1;
    y3 = y * fontheight[read_page->font];
    for (y2 = 0; y2 < fontheight[read_page->font]; y2++) {
        x3 = x * fontwidth[read_page->font];
        for (x2 = 0; x2 < fontwidth[read_page->font]; x2++) {
            col = point(x3, y3);
            if (col) {
                if (i3) {
                    i3 = 0;
                    firstcol = col;
                }
                col = 255;
            }
            chr[i] = col;
            i++;
            x3++;
        }
        y3++;
    }

    if (i3) { // assume SPACE, no non-zero pixels were found
        if (returncol)
            return 1;
        return 32;
    }

    i3 = i; // number of bytes per character "signature"

    // compare signature with all ascii characters
    for (i = 0; i <= 255; i++) {
        if (read_page->font == 8)
            cp = &charset8x8[i][0][0];
        if (read_page->font == 14)
            cp = &charset8x16[i][1][0];
        if (read_page->font == 16)
            cp = &charset8x16[i][0][0];
        i2 = memcmp(cp, chr, i3);
        if (!i2) { // identical!
            if (returncol)
                return firstcol;
            return i;
        }
    }

    // no match found
    if (returncol)
        return 0;
    return 32;
}

// sub_bsave, sub_bload, func_lof, func_eof moved to fileio.cpp

int32 func__statusCode(int32 handle) {
    if (handle >= 0) {
        error(52);
        return -1;
    }

    int real_handle = -(handle + 1);

    special_handle_struct *sh = (special_handle_struct *)list_get(special_handles, real_handle);
    if (sh->type != special_handle_type::Http) {
        error(52);
        return -1;
    }

    int code = libqb_http_get_status_code(real_handle);

    if (code == -1) {
        // Connection isn't valid, it has no status code.
        error(9);
        return -1;
    }

    return code;
}

// sub_seek, func_seek, func_loc moved to fileio.cpp

qbs *func_input(int32 n, int32 i, int32 passed) {
    if (is_error_pending())
        return qbs_new(0, 1);
    static qbs *str, *str2;
    static int32 x, c;
    if (n < 0)
        str = qbs_new(0, 1);
    else
        str = qbs_new(n, 1);
    if (passed) {

        if (gfs_fileno_valid(i) != 1) {
            error(52);
            return str;
        } // Bad file name or number
        i = gfs_get_fileno(i); // convert fileno to gfs index
        static gfs_file_struct *gfs;
        gfs = gfs_get_file_struct(i);
        if ((gfs->type < 2) || (gfs->type > 3)) {
            error(62);
            return str;
        } // Input past end of file
        // note: RANDOM should be supported
        // note: Unusually, QB returns "Input past end of file" instead of "Bad file mode"
        if (!gfs->read) {
            error(75);
            return str;
        } // Path/file access error

        if (n < 0) {
            error(52);
            return str;
        } // Bad file name or number
        if (n == 0)
            return str;

        // INPUT -> Input past end of file at EOF or CHR$(26)
        //         unlike BINARY, partial strings cannot be returned
        //         (use input_file_chr and modify it to support CHR$(26)
        if (gfs->type == 3) {
            x = 0;
            do {
                c = file_input_chr(i);
                if (c == -1) {
                    error(62);
                    return str;
                } // Input past end of file
                if (c == -2) {
                    error(75);
                    return str;
                } // path/file access error
                str->chr[x] = c;

                if (gfs_get_file_struct(i)->eof_passed != 1) { // If we haven't declared the End of the File, check to see if the next byte is an EOF byte
                    c = file_input_chr(i);                     // read the next byte
                    if (gfs_get_file_struct(i)->eof_passed != 1) {
                        gfs_setpos(i, gfs_getpos(i) - 1);
                    } // and if it's not EOF, move our position back to where it should be
                }

                x++;
            } while (x < n);
            return str;
        }

        // BINARY -> If past EOF, returns a NULL length string!
        //          or as much of the data that was valid as possible
        //          Seek POS is only advanced as far as was read!
        //          Binary can read past CHR$(26)
        //          (simply call C's read statement and manage via .eof
        if (gfs->type == 2) {
            static int32 e;
            e = gfs_read(i, -1, str->chr, n);
            if (e) {
                if (e != -10) { // note: on eof, unread buffer area becomes NULL
                    str->len = 0;
                    if (e == -2) {
                        error(258);
                        return str;
                    } // invalid handle
                    if (e == -3) {
                        error(54);
                        return str;
                    } // bad file mode
                    if (e == -4) {
                        error(5);
                        return str;
                    } // illegal function call
                    if (e == -7) {
                        error(70);
                        return str;
                    } // permission denied
                    error(75);
                    return str; // assume[-9]: path/file access error
                }
            }
            str->len = gfs_read_bytes(); // adjust if not enough data was available
            return str;
        }

        // RANDOM file mode access - for reading/writing at specific positions
        // TODO: Implement RANDOM mode file operations with proper record positioning
        // Current implementation returns empty string as placeholder

        return str;
    } else {
        // keyboard/piped
        //      For extended-two-byte character codes, only the first, CHR$(0), is returned and counts a 1 byte
        if (n < 0) {
            error(52);
            return str;
        }
        if (n == 0)
            return str;
        x = 0;
    waitforinput:
        str2 = qbs_inkey();
        if (str2->len) {
            str->chr[x] = str2->chr[0];
            x++;
        }
        qbs_free(str2);
        if (stop_program)
            return str;
        if (x < n) {
            evnt(0); // check for new events
            Sleep(10);
            goto waitforinput;
        }
        return str;
    }
}

void file_line_input_string_character(int32 filehandle, qbs *deststr) {
    static qbs *str, *character;
    int32 c, nextc;
    int32 inspeechmarks;

    str = qbs_new(0, 0);
    c = file_input_chr(filehandle);
    if (c == -2)
        return;
    if (c == -1) {
        qbs_set(deststr, str);
        qbs_free(str);
        error(62); // input past end of file
        return;
    }
    character = qbs_new(1, 0);
nextchr:
    if (c == -1)
        goto gotstr;
    if (c == 10)
        goto gotstr;
    if (c == 13)
        goto gotstr;
    character->chr[0] = c;
    qbs_set(str, qbs_add(str, character));
    c = file_input_chr(filehandle);
    if (c == -2)
        return;
    goto nextchr;
gotstr:
nextstr:
    // scan until next item (or eof) reached
    if (c == -1)
        goto returnstr;
    if ((c == 10) || (c == 13)) { // lf cr
        file_input_skip1310(filehandle, c);
        goto returnstr;
    }
    c = file_input_chr(filehandle);
    if (c == -2)
        return;
    goto nextstr;
// return string
returnstr:
    qbs_set(deststr, str);
    qbs_free(str);
    qbs_free(character);
    return;
}

void file_line_input_string_binary(int32 fileno, qbs *deststr) {
    int32 filebuf_size = 512;
    int32 filehandle;
    qbs *eol;

    filehandle = gfs_get_fileno(fileno); // convert fileno to gfs index
    eol = qbs_new_txt_len("\n", 1);

    int64 start_byte = func_seek(fileno);
    int64 filelength = func_lof(fileno);
    if (start_byte > filelength) {
        error(62); // input past end of file
        return;
    }
    qbs *buffer = qbs_new(filebuf_size, 0);
    qbs_set(deststr, qbs_new_txt_len("", 0));
    do {
        if (start_byte + filebuf_size > filelength)
            filebuf_size = filelength - start_byte + 1;
        qbs_set(buffer, func_space(qbr(filebuf_size)));

        sub_get2(fileno, start_byte, buffer, 1);
        int32 eol_pos = func_instr(0, buffer, eol, 0);
        if (eol_pos == 0) {
            if ((start_byte + filebuf_size) >= filelength) {
                qbs_set(deststr, buffer);
                gfs_setpos(filehandle, filelength);              // set the position right before the EOF marker
                gfs_get_file_struct(filehandle)->eof_passed = 1; // also set EOF flag;
                qbs_free(buffer);
                return;
            }
            filebuf_size += 512;
        } else {
            qbs_set(deststr, qbs_add(deststr, qbs_left(buffer, eol_pos - 1)));
            break;
        }
    } while (!func_eof(fileno));
    qbs_free(buffer);
    if (start_byte + deststr->len + 2 >= filelength) {   // if we've read to the end of the line
        gfs_setpos(filehandle, filelength);              // set the position right before the EOF marker
        gfs_get_file_struct(filehandle)->eof_passed = 1; // also set EOF flag;
        if (deststr->chr[deststr->len - 1] == '\r')
            qbs_set(deststr, qbs_left(deststr, deststr->len - 1));
        return;
    }
    gfs_setpos(filehandle, start_byte + deststr->len); // set the position at the end of the text
    if (deststr->chr[deststr->len - 1] == '\r')
        qbs_set(deststr, qbs_left(deststr, deststr->len - 1));
}

void sub_file_line_input_string(int32 fileno, qbs *deststr) {
    int32 filehandle;
    if (is_error_pending())
        return;
    if (gfs_fileno_valid(fileno) != 1) {
        error(52);
        return;
    } // Bad file name or number
    filehandle = gfs_get_fileno(fileno); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(filehandle);
    if (!gfs->read) {
        error(75);
        return;
    } // Path/file access error

    if (gfs->type == 2) {
        file_line_input_string_binary(fileno, deststr);
    } else if (gfs->type == 3) {
        file_line_input_string_character(filehandle, deststr);
    } else {
        error(54); // Bad file mode
    }
    return;
}

// func_freefile moved to fileio.cpp

// Mouse functions (sub__mousehide, sub__mouseshow, func__mousex, func__mousey, etc.) moved to mouse.cpp

#ifdef QB64_GLUT
int mouse_cursor_style = GLUT_CURSOR_LEFT_ARROW;
#else
int mouse_cursor_style = 1;
#endif

extern uint16 call_absolute_offsets[256];

void call_absolute(int32 args, uint16 offset) {
    memset(&cpu, 0, sizeof(cpu_struct)); // flush cpu
    cpu.cs = ((defseg - cmem) >> 4);
    cpu.ip = offset;
    cpu.ss = 0xFFFF;
    cpu.sp = 0; // sp "loops" to <65536 after first push
    cpu.ds = 80;
    // push (near) arg offsets
    static int32 i;
    for (i = 0; i < args; i++) {
        cpu.sp -= 2;
        *(uint16 *)(cmem + cpu.ss * 16 + cpu.sp) = call_absolute_offsets[i];
    }
    // push ret segment, then push ret offset (both 0xFFFF to return control to QB64)
    cpu.sp -= 4;
    *(uint32 *)(cmem + cpu.ss * 16 + cpu.sp) = 0xFFFFFFFF;
    cpu_call();
}

void call_int(int32 i) {

    if (i == 0x33) {

        if (cpu.ax == 0) {
            cpu.ax = 0xFFFF; // mouse installed
            cpu.bx = 2;
            return;
        }

        if (cpu.ax == 1) {
            sub__mouseshow(NULL, 0);
            return;
        }
        if (cpu.ax == 2) {
            sub__mousehide();
            return;
        }
        if (cpu.ax == 3) {
            // return the current mouse status
            // buttons

            mouse_message_queue_struct *queue = &mouse_message_queue;

            // buttons
            cpu.bx = queue->queue[queue->last].buttons & 1;
            if (queue->queue[queue->last].buttons & 4)
                cpu.bx += 2;

            // x,y offsets
            static float mx, my;

            // temp override current message index to the most recent event
            static int32 current_mouse_message_backup;
            current_mouse_message_backup = queue->current;
            queue->current = queue->last;

            mx = func__mousex();
            my = func__mousey();

            // restore "current" message index
            queue->current = current_mouse_message_backup;

            cpu.cx = mx;
            cpu.dx = my;
            // double x-axis value for modes 1,7,13
            if ((display_page->compatible_mode == 1) || (display_page->compatible_mode == 7) || (display_page->compatible_mode == 13))
                cpu.cx *= 2;
            if (display_page->text) {
                // note: a range from 0 to columns*8-1 is returned regardless of the number of actual pixels
                cpu.cx = (mx - 0.5) * 8.0;
                if (cpu.cx >= (display_page->width * 8))
                    cpu.cx = (display_page->width * 8) - 1;
                // note: a range from 0 to rows*8-1 is returned regardless of the number of actual pixels
                // obsolete line of code:
                // cpu.dx=(((float)cpu.dx)/((float)(display_page->height*fontheight[display_page->font])))*((float)(display_page->height*8));//(mouse_y/height_in_pixels)*(rows*8)
                cpu.dx = (my - 0.5) * 8.0;
                if (cpu.dx >= (display_page->height * 8))
                    cpu.dx = (display_page->height * 8) - 1;
            }
            return;
        }

        if (cpu.ax == 7) { // horizontal min/max
            return;
        }
        if (cpu.ax == 8) { // vertical min/max
            return;
        }

        // MessageBox2(NULL,"Unknown MOUSE Sub-function","Call Interrupt Error",MB_OK|MB_SYSTEMMODAL);
        // exit(cpu.ax);

        return;
    }
}

// 2D PROTOTYPE QB64<->C CALLS
// Image management functions moved to graphics.cpp:
// func__newimage, func__copyimage, sub__freeimage, freeallimages
// sub__source, sub__dest, func__source, func__dest, func__display
// sub__blend, sub__dontblend, sub__setalpha
// func__width, func__height, func__pixelsize, func__blend

// func__defaultcolor, func__backgroundcolor moved to color.cpp

// Working with 256 color palettes:
// func__palettecolor, sub__palettecolor, sub__copypalette moved to color.cpp

// sub__printstring, func__printwidth, func__loadfont, sub__font,
// func__fontwidth, func__fontheight, func__font, sub__freefont,
// sub__printmode, func__printmode moved to text.cpp

// matchcol, func__rgb, func__rgba, func__alpha, func__red, func__green, func__blue moved to color.cpp

void sub_end() {

    if (sub_gl_called)
        error(271);

    dont_call_sub_gl = 1;

    sub_close(NULL, 0);
    exit_blocked = 0; // allow exit via X-box or CTRL+BREAK

#ifdef DEPENDENCY_CONSOLE_ONLY
    screen_hide = 1;
#endif

    if (!screen_hide) {
        // 1. set the display page as the destination page
        sub__dest(func__display());
        // 2. VIEW PRINT bottomline,bottomline
        static int32 y;
        if (write_page->text) {
            y = write_page->height;
        } else {
            y = write_page->height / fontheight[write_page->font];
        }
        qbg_sub_view_print(y, y, 1 | 2);
        // 3. PRINT 'clears the line without having to worry about its contents/size
        qbs_print(nothingstring, 1);
        // 4. PRINT "Press any key to continue"
        qbs_print(qbs_new_txt("Press any key to continue"), 0);
        // 5. Clear any buffered keypresses
        static uint32 qbs_tmp_base;
        qbs_tmp_base = qbs_tmp_list_nexti;
        while (qbs_cleanup(qbs_tmp_base, qbs_notequal(qbs_inkey(), qbs_new_txt("")))) {
            Sleep(0);
        }
        // 6. Enable autodisplay
        autodisplay = 1;
        // 7. Wait for a new keypress
        do {
            Sleep(100);
            if (stop_program)
                end();
        } while (qbs_cleanup(qbs_tmp_base, qbs_equal(qbs_inkey(), qbs_new_txt(""))));

    } else {
        if (console) {
// screen is hidden, console is visible
#ifdef QB64_WINDOWS
            std::cout << "\nPress any key to continue";
            int32 junk;
            FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)); // clear any stray buffer events before we run END.
            do {                                                     // ignore all console input
                junk = func__getconsoleinput();
            } while (junk != 1); // until we have a key down event
#else
            std::cout << "\nPress enter to continue";
            static int32 ignore;
            ignore = fgetc(stdin);
#endif
        }
    }
    close_program = 1;
    end();
    exit(0); //<-- should never happen
}


void sub_run_init() {
    // Reset ON KEY trapping
    // note: KEY bar F-key bindings are not affected
    static int32 i;
    for (i = 1; i <= 31; i++) {
        onkey[i].id = 0;
        onkey[i].active = 0;
        onkey[i].state = 0;
    }
    onkey_inprogress = 0;
    // note: if already in screen 0:80x25, screen pages are left intact
    // set screen mode to 0 (80x25)
    qbg_screen(0, NULL, 0, 0, NULL, 1 | 4 | 8);
    // make sure WIDTH is 80x25
    qbsub_width(NULL, 80, 25, 0, 0, 1 | 2);
    // restore view print
    qbg_sub_view_print(NULL, NULL, 0);
    // restore palette
    restorepalette(write_page);
    // restore default colors
    write_page->background_color = 0;
    write_page->color = 7;
    // note: cursor state does not appear to be reset by the RUN command
    // im->cursor_show=0; im->cursor_firstvalue=4; im->cursor_lastvalue=4;
    // Reset RND & RANDOMIZE state
    reset_rnd_state();
    // clear keyboard buffers
    sub__keyclear(NULL, 0);
}

void sub_run(qbs *f) {
    if (is_error_pending())
        return;

    // run program
    static qbs *str = NULL;
    if (str == NULL)
        str = qbs_new(0, 0);
    static qbs *strz = NULL;
    if (!strz)
        strz = qbs_new(0, 0);

    qbs_set(str, f);
    filepath_fix_directory(str);

#ifdef QB64_WINDOWS

    qbs_set(strz, qbs_add(str, qbs_new_txt_len("\0", 1)));
    if (WinExec((char *)strz->chr, SW_SHOWDEFAULT) > 31) {
        goto run_exit;
    } else {
        // 0-out of resources/memory
        // ERROR_BAD_FORMAT
        // ERROR_FILE_NOT_FOUND
        // ERROR_PATH_NOT_FOUND
        error(53);
        return; // file not found
    }

#else
    qbs_set(strz, qbs_add(str, qbs_new_txt_len("\0", 1)));
    system((char *)strz->chr);
    // success?
    goto run_exit;

#endif

// exit this program
run_exit:
    close_program = 1;
    end();
    exit(99); //<--this line should never actually be executed
}

#ifdef DEPENDENCY_ICON
void sub__icon(int32 handle_icon, int32 handle_window_icon, int32 passed) {

    if (is_error_pending())
        return;
#    ifndef DEPENDENCY_CONSOLE_ONLY
    if (!(passed & 2))
        handle_window_icon = handle_icon;
    if (!(passed & 1)) {
        if (!image_qbicon16_handle) {
            image_qbicon16_handle = func__newimage(image_qbicon16_w, image_qbicon16_h, 32, 1);
            memcpy(img[-image_qbicon16_handle].offset32, &image_qbicon16[0], image_qbicon16_w * image_qbicon16_h * 4);
        }

        if (!image_qbicon32_handle) {
            image_qbicon32_handle = func__newimage(image_qbicon32_w, image_qbicon32_h, 32, 1);
            memcpy(img[-image_qbicon32_handle].offset32, &image_qbicon32[0], image_qbicon32_w * image_qbicon32_h * 4);
        }

        handle_icon = image_qbicon32_handle;
        handle_window_icon = image_qbicon16_handle;
    }

    static int32 i, i2, ii, w, h;
    static uint32 *o, *o2;
    static int32 x, y, n, c, i3, c2;

    // validation
    for (ii = 1; ii <= 2; ii++) {
        if (ii == 1)
            i = handle_icon;
        if (ii == 2)
            i = handle_window_icon;
        if (i >= 0) { // validate i
            validatepage(i);
            i = page[i];
        } else {
            i = -i;
            if (i >= nextimg) {
                error(258);
                return;
            }
            if (!img[i].valid) {
                error(258);
                return;
            }
        }
        if (img[i].text) {
            error(5);
            return;
        }
        if (ii == 1)
            handle_icon = i;
        if (ii == 2)
            handle_window_icon = i;
    }

#        ifdef QB64_WINDOWS
    HWND win = (HWND)func__handle();
    if (!win) {
        return;
    }

    static HANDLE ExeIcon;
    static HANDLE ExeIcon16;

    // Attempt to load the first icon embedded in the .exe
    if (!ExeIcon)
        ExeIcon = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(0), IMAGE_ICON, 32, 32, 0);
    if (!ExeIcon16)
        ExeIcon16 = LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(0), IMAGE_ICON, 16, 16, 0);

    // If we have an embedded icon, we'll use it instead of QB64-PE's default
    if (!(passed & 1) && (ExeIcon)) {
        SendMessage(win, WM_SETICON, ICON_BIG, (LPARAM)ExeIcon);

        if (ExeIcon16) {
            SendMessage(win, WM_SETICON, ICON_SMALL, (LPARAM)ExeIcon16);
        } else {
            SendMessage(win, WM_SETICON, ICON_SMALL, (LPARAM)ExeIcon);
        }
        return;
    }
    for (ii = 1; ii <= 2; ii++) {

        if (ii == 1) {
            i = handle_icon;
            w = GetSystemMetrics(SM_CXICON);
            h = GetSystemMetrics(SM_CYICON);
        }
        if (ii == 2) {
            i = handle_window_icon;
            w = GetSystemMetrics(SM_CXSMICON);
            h = GetSystemMetrics(SM_CYSMICON);
        }

        // source[http://support.microsoft.com/kb/318876]
        ICONINFO iinfo;
        HDC hdc;
        BITMAPV5HEADER bi;
        HBITMAP hBitmap, hOldBitmap;
        void *lpBits;
        HCURSOR hAlphaCursor = NULL;
        ZeroMemory(&bi, sizeof(BITMAPV5HEADER));
        bi.bV5Size = sizeof(BITMAPV5HEADER);
        bi.bV5Width = w;
        bi.bV5Height = h;
        bi.bV5Planes = 1;
        bi.bV5BitCount = 32;
        bi.bV5Compression = BI_BITFIELDS;
        // The following mask specification specifies a supported 32 BPP
        // alpha format for Windows XP.
        bi.bV5RedMask = 0x00FF0000;
        bi.bV5GreenMask = 0x0000FF00;
        bi.bV5BlueMask = 0x000000FF;
        bi.bV5AlphaMask = 0xFF000000;
        hdc = GetDC(NULL);
        // Create the DIB section with an alpha channel.
        hBitmap = CreateDIBSection(hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS, (void **)&lpBits, NULL, (DWORD)0);
        ReleaseDC(NULL, hdc);

        i2 = func__newimage(w, h, 32, 1);
        sub__dontblend(i2, 1);
        sub__putimage(NULL, NULL, NULL, NULL, -i, i2, NULL, NULL, NULL, NULL, 8 + 32);

        o = img[-i2].offset32;
        o2 = (uint32 *)lpBits;
        for (y = 0; y < h; y++) {
            for (x = 0; x < w; x++) {
                c = o[(h - 1 - y) * w + x];
                o2[y * w + x] = c;
            }
        }

        sub__freeimage(i2, 1);

        // Create an empty mask bitmap.
        HBITMAP hMonoBitmap = CreateBitmap(w, h, 1, 1, NULL);

        iinfo.fIcon = TRUE; // Change fIcon to TRUE to create an alpha icon
        iinfo.xHotspot = 0;
        iinfo.yHotspot = 0;
        iinfo.hbmMask = hMonoBitmap;
        iinfo.hbmColor = hBitmap;
        // Create the alpha cursor with the alpha DIB section.
        hAlphaCursor = CreateIconIndirect(&iinfo);

        DeleteObject(hBitmap);
        DeleteObject(hMonoBitmap);

        if (ii == 1)
            SendMessage(win, WM_SETICON, ICON_BIG, (LPARAM)hAlphaCursor);
        if (ii == 2)
            SendMessage(win, WM_SETICON, ICON_SMALL, (LPARAM)hAlphaCursor);

    } // ii
#        endif // QB64_WINDOWS
#    endif     // DEPENDENCY_CONSOLE_ONLY
} // sub__icon
#endif // DEPENDENCY_ICON

// func_screenwidth, func_screenheight, sub_screenicon, func_windowexists, func_screenicon moved to window.cpp

int32 func__autodisplay() {
    if (autodisplay) {
        return -1;
    }
    return 0;
}

// sub__autodisplay, sub__display moved to screen.cpp
// sub_draw moved to graphics.cpp

#ifdef QB64_WINDOWS
void showvalue(__int64 v) {
    static qbs *s = NULL;
    if (s == NULL)
        s = qbs_new(0, 0);
    qbs_set(s, qbs_str(v));
    gui_alert((char *)s->chr, "showvalue", "ok");
}
#endif


// Networking code (tcp_*, stream_*, connection_*, func__openclient, func__openhost,
// func__openconnection, func__connectionaddress, func__connected) moved to libqb/src/networking.cpp


int32 func__exit() {
    exit_blocked = 1;
    static int32 x;
    x = exit_value;
    if (x)
        exit_value = 0;
    return x;
}

int32 display_called = 0;

void display_now() {
    if (autodisplay) {
        display_called = 0;
        while (!display_called)
            Sleep(1);
    } else {
        display();
    }
}

// sub__fullscreen, sub__allowfullscreen, func__fullscreen, func__fullscreensmooth moved to screen.cpp

void chain_restorescreenstate(int32 i) {
    static int32 i32, i32b, i32c, x, x2;
    generic_get(i, -1, (uint8 *)&i32, 4);

    if (i32 == 256) {
        generic_get(i, -1, (uint8 *)&i32, 4);
        if (i32 != 0)
            qbg_screen(i32, 0, 0, 0, 0, 1);
        generic_get(i, -1, (uint8 *)&i32, 4);
        if (i32 == 258) {
            generic_get(i, -1, (uint8 *)&i32, 4);
            i32b = i32;
            generic_get(i, -1, (uint8 *)&i32, 4);
            qbsub_width(0, i32b, i32, 0, 0, 1 + 2);
            generic_get(i, -1, (uint8 *)&i32, 4);
        }
    }

    if (i32 == 257) {
        generic_get(i, -1, (uint8 *)&i32, 4);
        i32c = i32;
        generic_get(i, -1, (uint8 *)&i32, 4);
        i32b = i32;
        generic_get(i, -1, (uint8 *)&i32, 4);
        qbg_screen(func__newimage(i32b, i32, i32c, 1), 0, 0, 0, 0, 1);
        generic_get(i, -1, (uint8 *)&i32, 4);
    }

    if (i32 == 259) {
        generic_get(i, -1, (uint8 *)&i32, 4);
        sub__font(i32, 0, 0);
        generic_get(i, -1, (uint8 *)&i32, 4);
    }

    static img_struct *ix;
    static img_struct imgs;

    while (i32 == 260) {
        generic_get(i, -1, (uint8 *)&i32, 4);
        x = i32;
        qbg_screen(0, 0, x, 0, 0, 4 + 8); // switch to page (allocates the page)
        ix = &img[page[x]];
        generic_get(i, -1, ix->offset, ix->width * ix->height * ix->bytes_per_pixel);
        imgs = *ix;
        generic_get(i, -1, (uint8 *)ix, sizeof(img_struct));
        // revert specific data
        if (ix->font >= 32)
            ix->font = imgs.font;
        ix->offset = imgs.offset;
        ix->pal = imgs.pal;
        generic_get(i, -1, (uint8 *)&i32, 4);
    }

    if (i32 == 261) {
        generic_get(i, -1, (uint8 *)&i32, 4);
        i32b = i32;
        generic_get(i, -1, (uint8 *)&i32, 4);
        qbg_screen(0, 0, i32b, i32, 0, 4 + 8); // switch to correct active & visual pages
        generic_get(i, -1, (uint8 *)&i32, 4);
    }

    if (i32 == 262) {
        for (x = 0; x <= 255; x++) {
            generic_get(i, -1, (uint8 *)&i32, 4);
            sub__palettecolor(x, i32, 0, 1);
        }
        generic_get(i, -1, (uint8 *)&i32, 4);
    }

    // assume command #511("finished screen state") in i32
}

void chain_savescreenstate(int32 i) { // adds the screen state to file #i
    static int32 i32, x, x2;
    static img_struct *i0, *ix;
    i0 = &img[page[0]];

    if ((i0->offset > cmem) && (i0->offset < (cmem + 1114099))) { // cmem?[need to maintain cmem state]
        //[256][mode]
        i32 = 256;
        generic_put(i, -1, (uint8 *)&i32, 4);
        i32 = i0->compatible_mode;
        generic_put(i, -1, (uint8 *)&i32, 4);
        if (i0->text) {
            //[258][WIDTH:X][Y]
            i32 = 258;
            generic_put(i, -1, (uint8 *)&i32, 4);
            i32 = i0->width;
            generic_put(i, -1, (uint8 *)&i32, 4);
            i32 = i0->height;
            generic_put(i, -1, (uint8 *)&i32, 4);
        }
    } else {
        //[257][mode][X][Y]
        i32 = 257;
        generic_put(i, -1, (uint8 *)&i32, 4);
        i32 = i0->compatible_mode;
        generic_put(i, -1, (uint8 *)&i32, 4);
        i32 = i0->width;
        generic_put(i, -1, (uint8 *)&i32, 4);
        i32 = i0->height;
        generic_put(i, -1, (uint8 *)&i32, 4);
    }
    //[259][font] (standard fonts only)
    if (i0->font < 32) {
        i32 = 259;
        generic_put(i, -1, (uint8 *)&i32, 4);
        i32 = i0->font;
        generic_put(i, -1, (uint8 *)&i32, 4);
    }

    //[260][page][rawdata]
    // note: write page is done last to avoid having its values undone by the later page switch
    x2 = -1;
    for (x = 0; x < pages; x++) {
        if (page[x]) {
            if (page[x] != write_page_index) {
            save_write_page:
                ix = &img[page[x]];
                i32 = 260;
                generic_put(i, -1, (uint8 *)&i32, 4);
                i32 = x;
                generic_put(i, -1, (uint8 *)&i32, 4);
                generic_put(i, -1, ix->offset, ix->width * ix->height * ix->bytes_per_pixel);
                // save structure (specific parts will be reincorporated)
                generic_put(i, -1, (uint8 *)ix, sizeof(img_struct));
                if (x == x2)
                    break;
            } else
                x2 = x;
        }
    }
    if ((x2 != -1) && (x != x2)) {
        x = x2;
        goto save_write_page;
    }

    //[261][activepage][visualpage]
    i32 = 261;
    generic_put(i, -1, (uint8 *)&i32, 4);
    i32 = 0; // note: activepage could be a non-screenpage
    for (x = 0; x < pages; x++) {
        if (page[x] == write_page_index) {
            i32 = x;
            break;
        }
    }
    generic_put(i, -1, (uint8 *)&i32, 4);
    i32 = 0;
    for (x = 0; x < pages; x++) {
        if (page[x] == display_page_index) {
            i32 = x;
            break;
        }
    }
    generic_put(i, -1, (uint8 *)&i32, 4);

    //[262][256x32-bit color palette]
    if (i0->bytes_per_pixel != 4) {
        i32 = 262;
        generic_put(i, -1, (uint8 *)&i32, 4);
        for (x = 0; x <= 255; x++) {
            i32 = func__palettecolor(x, 0, 1);
            generic_put(i, -1, (uint8 *)&i32, 4);
        }
    }

    //[511](screen state finished)
    i32 = 511;
    generic_put(i, -1, (uint8 *)&i32, 4);

} // chain_savescreenstate

void sub_lock(int32 i, int64 start, int64 end, int32 passed) {
    if (is_error_pending())
        return;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);

    // If the file has been opened for sequential input or output, LOCK and UNLOCK affect the entire file, regardless of the range specified by start& and end&.
    if (gfs->type > 2)
        passed = 0;

    if (passed & 1) {
        start--;
        if (start < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            start *= gfs->record_length;
    } else {
        start = -1;
    }

    if (passed & 2) {
        end--;
        if (end < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            end = end * gfs->record_length + gfs->record_length - 1;
    } else {
        end = start;
        if (gfs->type == 1)
            end = start + gfs->record_length - 1;
        if (!(passed & 1))
            end = -1;
    }

    int32 e;
    e = gfs_lock(i, start, end);
    if (e) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        if (e == -7) {
            error(70);
            return;
        } // permission denied
        error(75);
        return; // assume[-9]: path/file access error
    }
}

void sub_unlock(int32 i, int64 start, int64 end, int32 passed) {
    if (is_error_pending())
        return;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);

    // If the file has been opened for sequential input or output, LOCK and UNLOCK affect the entire file, regardless of the range specified by start& and end&.
    if (gfs->type > 2)
        passed = 0;

    if (passed & 1) {
        start--;
        if (start < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            start *= gfs->record_length;
    } else {
        start = -1;
    }

    if (passed & 2) {
        end--;
        if (end < 0) {
            error(5);
            return;
        }
        if (gfs->type == 1)
            end = end * gfs->record_length + gfs->record_length - 1;
    } else {
        end = start;
        if (gfs->type == 1)
            end = start + gfs->record_length - 1;
        if (!(passed & 1))
            end = -1;
    }

    int32 e;
    e = gfs_unlock(i, start, end);
    if (e) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        if (e == -7) {
            error(70);
            return;
        } // permission denied
        error(75);
        return; // assume[-9]: path/file access error
    }
}

#ifdef DEPENDENCY_SCREENIMAGE
int32 func__screenimage(int32 x1, int32 y1, int32 x2, int32 y2, int32 passed) {

#    ifdef QB64_WINDOWS

    static int32 init = 0;
    static int32 x, y, w, h;

    static HWND hwnd;
    static RECT rect;
    static HDC hdc;

    if (!init) {
        hwnd = GetDesktopWindow();
        GetWindowRect(hwnd, &rect);
        x = rect.right - rect.left;
        y = rect.bottom - rect.top;
    }

    hdc = GetDC(NULL);

    static HDC hdc2 = NULL;
    static HBITMAP bitmap;
    static int32 bx, by;

    if (passed) {
        if (x1 < 0)
            x1 = 0;
        if (y1 < 0)
            y1 = 0;
        if (x2 > x - 1)
            x2 = x - 1;
        if (y2 > y - 1)
            y2 = y - 1;
        w = x2 - x1 + 1;
        h = y2 - y1 + 1;
    } else {
        x1 = 0;
        x2 = x - 1;
        y1 = 0;
        y2 = y - 1;
        w = x;
        h = y;
    }

    if (hdc2) {
        if ((w != bx) || (h != by)) {
            DeleteObject(bitmap);
            ReleaseDC(NULL, hdc2);
            hdc2 = CreateCompatibleDC(hdc);
            bitmap = CreateCompatibleBitmap(hdc, w, h);
            bx = w;
            by = h;
            SelectObject(hdc2, bitmap);
        }
    } else {
        hdc2 = CreateCompatibleDC(hdc);
        bitmap = CreateCompatibleBitmap(hdc, w, h);
        bx = w;
        by = h;
        SelectObject(hdc2, bitmap);
    }

    init = 1;

    BitBlt(hdc2, 0, 0, w, h, hdc, x1, y1, SRCCOPY);

    static BITMAPFILEHEADER bmfHeader;
    static BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = w;
    bi.biHeight = -h; // A bottom-up DIB is specified by setting the height to a positive number, while a top-down DIB is specified by setting the height to a
                      // negative number. The bitmap color table will be appended to the BITMAPINFO structure.
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    static int32 i, i2;
    i2 = func__dest();
    i = func__newimage(w, h, 32, 1);
    sub__dest(i);

    GetDIBits(hdc2, bitmap, 0, h, write_page->offset, (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    sub__setalpha(255, NULL, NULL, NULL, 0);
    sub__dest(i2);

    ReleaseDC(NULL, hdc);
    return i;
#    else
    return func__newimage(func_screenwidth(), func_screenheight(), 32, 1);
#    endif
}
#endif // DEPENDENCY_SCREENIMAGE

void sub__screenclick(int32 x, int32 y, int32 button, int32 passed) {

#ifdef QB64_WINDOWS

    static INPUT input;

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
    static HWND hwnd;
    hwnd = GetDesktopWindow();
    static RECT rect;
    GetWindowRect(hwnd, &rect);
    static double x2, y2, fx, fy;
    x2 = rect.right - rect.left;
    y2 = rect.bottom - rect.top;
    fx = x * (65535.0f / x2);
    fy = y * (65535.0f / y2);
    input.mi.dx = fx;
    input.mi.dy = fy;
    SendInput(1, &input, sizeof(INPUT));

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));

    ZeroMemory(&input, sizeof(INPUT));
    input.type = INPUT_MOUSE;

    if (passed) {
        if (button == 1) {
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        }
        if (button == 2) {
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        }
        if (button == 3) {
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEDOWN;
        }
        SendInput(1, &input, sizeof(INPUT));

        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_MOUSE;

        if (button == 1) {
            input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        }
        if (button == 2) {
            input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        }
        if (button == 3) {
            input.mi.dwFlags = MOUSEEVENTF_MIDDLEUP;
        }
    } else {
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

        SendInput(1, &input, sizeof(INPUT));

        ZeroMemory(&input, sizeof(INPUT));
        input.type = INPUT_MOUSE;

        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    }
    SendInput(1, &input, sizeof(INPUT));

#endif

#ifdef QB64_MACOSX
    CGEventRef click1_down = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseDown, CGPointMake(x, y), kCGMouseButtonLeft);
    CGEventRef click1_up = CGEventCreateMouseEvent(NULL, kCGEventLeftMouseUp, CGPointMake(x, y), kCGMouseButtonLeft);
    CGEventPost(kCGHIDEventTap, click1_down);
    CGEventPost(kCGHIDEventTap, click1_up);
    CFRelease(click1_up);
    CFRelease(click1_down);
#endif
}

// sub__screenprint and ASCII_TO_MACVK moved to libqb/src/platform.cpp

#ifndef DEPENDENCY_PRINTER

// stubs
void sub__printimage(int32 i) {
    return;
}

#else

void sub__printimage(int32 i) {

#    ifdef QB64_WINDOWS

    static LPSTR szPrinterName = NULL;
    DWORD dwNameLen;
    HDC dc;
    DOCINFOA di;
    uint32 w, h;
    int32 x, y;
    int32 i2;
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    img_struct *s, *s2;

    if (i >= 0) {
        validatepage(i);
        s = &img[page[i]];
    } else {
        x = -i;
        if (x >= nextimg) {
            error(258);
            return;
        }
        s = &img[x];
        if (!s->valid) {
            error(258);
            return;
        }
    }

    if (!szPrinterName)
        szPrinterName = (LPSTR)malloc(65536);
    dwNameLen = 65536;
    GetDefaultPrinterA(szPrinterName, &dwNameLen);
    if ((dc = CreateDCA("WINSPOOL", szPrinterName, NULL, NULL)) == NULL)
        goto failed;
    ZeroMemory(&di, sizeof(DOCINFO));
    di.cbSize = sizeof(DOCINFO);
    di.lpszDocName = "Document";
    if (StartDocA(dc, &di) <= 0) {
        DeleteDC(dc);
        goto failed;
    }
    if (StartPage(dc) <= 0) {
        EndDoc(dc);
        DeleteDC(dc);
        goto failed;
    }

    w = GetDeviceCaps(dc, HORZRES);
    h = GetDeviceCaps(dc, VERTRES);

    i2 = func__newimage(w, h, 32, 1);
    if (i2 == -1) {
        EndDoc(dc);
        DeleteDC(dc);
        goto failed;
    }
    s2 = &img[-i2];
    sub__dontblend(i2, 1);
    sub__putimage(NULL, NULL, NULL, NULL, i, i2, NULL, NULL, NULL, NULL, 8 + 32);

    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = w;
    bi.biHeight = h;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    for (y = 0; y < h; y++) {
        SetDIBitsToDevice(dc, 0, y, w, 1, 0, 0, 0, 1, s2->offset32 + (y * w), (BITMAPINFO *)&bi, DIB_RGB_COLORS);
    }

    sub__freeimage(i2, 1);

    if (EndPage(dc) <= 0) {
        EndDoc(dc);
        DeleteDC(dc);
        goto failed;
    }
    if (EndDoc(dc) <= 0) {
        DeleteDC(dc);
        goto failed;
    }
    DeleteDC(dc);
failed:;
#    endif
}

#endif

// func__keyhit, func__keydown, sub__mapunicode, func__mapunicode
// moved to libqb/src/keyboard.cpp

int32 addone(int32 x) {
    return x + 1;
} // for testing purposes only

qbs *func__os() {
#ifdef QB64_WINDOWS
#    define QB64_OS_SYSTEM_STR "[WINDOWS]"
#elif defined(QB64_LINUX)
#    define QB64_OS_SYSTEM_STR "[LINUX]"
#elif defined(QB64_MACOSX)
#    define QB64_OS_SYSTEM_STR "[MACOSX]"
#else
#    define QB64_OS_SYSTEM_STR ""
#endif

#ifdef QB64_MACOSX
#    define QB64_OS_SYSTEM_EXTRA_STR "[LINUX]"
#else
#    define QB64_OS_SYSTEM_EXTRA_STR ""
#endif

#ifdef QB64_32
#    define QB64_OS_BITS_STR "[32BIT]"
#else
#    define QB64_OS_BITS_STR "[64BIT]"
#endif

#ifdef QB64_ARM
#    define QB64_OS_ARCH_STR "[ARM]"
#else
#    define QB64_OS_ARCH_STR ""
#endif

    // Have the compiler combine all our selections into one string
    return qbs_new_txt(QB64_OS_SYSTEM_STR QB64_OS_SYSTEM_EXTRA_STR QB64_OS_BITS_STR QB64_OS_ARCH_STR);
}

// func__screenx, func__screeny moved to screen.cpp

// sub__screenmove moved to window.cpp

void key_update() {

    if (key_display_redraw) {
        key_display_redraw = 0;
        if (!key_display)
            return;
    } else {
        if (key_display == key_display_state)
            return;
        key_display_state = key_display;
    }

    // use display page 0
    static int32 olddest;
    olddest = func__dest();
    sub__dest(0);
    static img_struct *i;
    i = write_page;

    static int32 f, z, c, x2;

    // locate bottom-left
    // get current status
    static int32 cx, cy, holding, col1, col2;
    cx = i->cursor_x;
    cy = i->cursor_y;
    holding = i->holding_cursor;
    col1 = i->color;
    col2 = i->background_color;
    static int32 h, w;
    // calculate height & width in characters
    if (i->compatible_mode) {
        h = i->height / fontheight[i->font];
        if (fontwidth[i->font]) {
            w = i->width / fontwidth[i->font];
        } else {
            w = write_page->width;
        }
    } else {
        h = i->height;
        w = i->width;
    }
    i->cursor_x = 1;
    i->cursor_y = h;
    i->holding_cursor = 0;

    static qbs *str = NULL;
    if (!str)
        str = qbs_new(0, 0);
    static qbs *str2 = NULL;
    if (!str2)
        str2 = qbs_new(1, 0);

    // clear bottom row using background color
    if (i->text) {
        for (x2 = 1; x2 <= i->width; x2++) {
            str2->chr[0] = 32;
            qbs_print(str2, 0);
        }
        i->cursor_x = 1;
        i->cursor_y = h;
        i->holding_cursor = 0;
    } else {
        fast_boxfill(0, (i->cursor_y - 1) * fontheight[i->font], i->width - 1, i->cursor_y * fontheight[i->font] - 1, col2 | 0xFF000000);
    }

    if (!key_display)
        goto no_key;

    static int32 item_x, limit_x, row_limit, leeway_x;
    leeway_x = 0;
    if (i->compatible_mode) {
        if (fontwidth[i->font]) {
            item_x = w / 12;
            row_limit = item_x * 12;
            if (item_x < 8) { // cannot fit min. width
                item_x = 8;
                row_limit = (w / 8) * 8;
                if (item_x > w) {
                    item_x = w;
                    row_limit = w;
                } // can't even fit 1!
            }
        } else {
            leeway_x = fontheight[i->font];
            item_x = w / 12;
            row_limit = item_x * 12 - leeway_x;
            x2 = ((float)fontheight[i->font]) * 0.5; // estimate the average character width (it's OK for this to be wrong)
            if (item_x < (x2 * 8 + leeway_x)) {      // cannot fit min. width
                item_x = (x2 * 8 + leeway_x);
                row_limit = (w / (x2 * 8 + leeway_x)) * (x2 * 8 + leeway_x) - leeway_x;
                if (item_x > w) {
                    item_x = w;
                    row_limit = w - leeway_x;
                } // can't even fit 1!
            }
        }
    } else {
        item_x = w / 12;
        row_limit = item_x * 12;
        if (item_x < 8) { // cannot fit min. width
            item_x = 8;
            row_limit = (w / 8) * 8;
            if (item_x > w) {
                item_x = w;
                row_limit = w;
            } // can't even fit 1!
        }
    }

    static int32 final_chr, row_final_chr;

    row_final_chr = 0;
    for (f = 1; f <= 12; f++) {
        final_chr = 0;
        limit_x = f * item_x - leeway_x; // set new limit

        // relocate
        x2 = ((f - 1) * item_x) + 1;
        if (x2 >= row_limit) {
            row_final_chr = 1;
            goto done_f;
        }
        i->cursor_x = x2;

        // number string
        if (fontwidth[i->font]) {
            qbs_set(str, qbs_ltrim(qbs_str(f)));
        } else {
            qbs_set(str, qbs_add(qbs_ltrim(qbs_str(f)), qbs_new_txt(")")));
        }
        for (z = 0; z < str->len; z++) {
            if (i->cursor_x >= row_limit)
                row_final_chr = 1;
            if (i->cursor_x > limit_x)
                goto done_f;
            if (i->cursor_x >= limit_x)
                final_chr = 1;
            str2->chr[0] = str->chr[z];
            qbs_print(str2, 0);
            if (final_chr)
                goto done_f;
        }

        // text
        static int32 fi;
        fi = f;
        if (f > 10)
            fi = f - 11 + 30;
        if (onkey[fi].text) {
            qbs_set(str, onkey[fi].text);
            if (i->text) {
                if (i->background_color) {
                    i->color = 7;
                    i->background_color = 0;
                } else {
                    i->color = 0;
                    i->background_color = 7;
                }
            }
        } else {
            str->len = 0;
        }
        z = 0;
        while (i->cursor_x < limit_x) {
            static int32 c;

            if (z >= str->len) {
                if (!onkey[fi].text)
                    goto done_f;
                c = 32;
            } else {
                c = str->chr[z++];
            }

            if (i->cursor_x >= row_limit)
                row_final_chr = 1;
            if (i->cursor_x > limit_x)
                goto done_f;
            if (i->cursor_x >= limit_x)
                final_chr = 1;
            /*
                7->14
                8->254
                9->26
                10->60
                11->127
                12->22
                13->27
                28->16
                29->17
                30->24
                31->25
                KEY LIST puts spaces instead of non-printables
                QBASIC's KEY LIST differs from QBX in this regard
                CHR$(13) is also turned into a space in KEY LIST, even if it is at the end
            */
            if (c == 7)
                c = 14;
            if (c == 8)
                c = 254;
            if (c == 9)
                c = 26;
            if (c == 10)
                c = 60;
            if (c == 11)
                c = 127;
            if (c == 12)
                c = 22;
            if (c == 13)
                c = 27;
            if (c == 28)
                c = 16;
            if (c == 29)
                c = 17;
            if (c == 30)
                c = 24;
            if (c == 31)
                c = 25;
            str2->chr[0] = c;
            no_control_characters = 1;
            qbs_print(str2, 0);
            no_control_characters = 0;
            if (final_chr)
                goto done_f;
        }

    done_f:;
        i->color = col1;
        i->background_color = col2;
        if (row_final_chr)
            goto done_row;
    }
done_row:;

// revert status
no_key:
    i->cursor_x = cx;
    i->cursor_y = cy;
    i->holding_cursor = holding;
    i->color = col1;
    i->background_color = col2;
    sub__dest(olddest);
}

void key_on() {
    key_display = 1;
    key_update();
}

void key_off() {
    key_display = 0;
    key_update();
}

void key_list() {
    static img_struct *i;
    i = write_page;
    static int32 mono;
    mono = 1;
    if (!fontwidth[i->font])
        if (func__printwidth(qbs_new_txt(" "), NULL, NULL) != func__printwidth(qbs_new_txt(")"), NULL, NULL))
            mono = 0;
    static int32 f, fi;
    static qbs *str = NULL;
    if (!str)
        str = qbs_new(0, 0);
    for (f = 1; f <= 12; f++) {

        // F-number & spacer
        if (fontwidth[i->font]) {
            if (f < 10) {
                qbs_set(str, qbs_add(qbs_ltrim(qbs_str(f)), qbs_new_txt("  ")));
            } else {
                qbs_set(str, qbs_add(qbs_ltrim(qbs_str(f)), qbs_new_txt(" ")));
            }
        } else {
            if ((f < 10) && (mono == 1)) {
                qbs_set(str, qbs_add(qbs_ltrim(qbs_str(f)), qbs_new_txt(")  ")));
            } else {
                qbs_set(str, qbs_add(qbs_ltrim(qbs_str(f)), qbs_new_txt(") ")));
            }
        }
        qbs_set(str, qbs_add(qbs_new_txt("F"), str));

        // text
        fi = f;
        if (f > 10)
            fi = f - 11 + 30;
        if (onkey[fi].text) {
            qbs_print(str, 0);
            /*
                7->14
                8->254
                9->26
                10->60
                11->127
                12->22
                13->27
                28->16
                29->17
                30->24
                31->25
                KEY LIST puts spaces instead of non-printables
                QBASIC's KEY LIST differs from QBX in this regard
                CHR$(13) is also turned into a space in KEY LIST, even if it is at the end
            */
            str->len = 1;
            static int32 x, c;
            for (x = 0; x < onkey[fi].text->len; x++) {
                c = onkey[fi].text->chr[x];
                if ((c >= 7) && (c <= 13))
                    c = 32;
                if ((c >= 28) && (c <= 31))
                    c = 32;
                str->chr[0] = c;
                qbs_print(str, 0);
            }
            str->len = 0;
            qbs_print(str, 1);
        } else {
            qbs_print(str, 1);
        }

    } // f
}

void key_assign(int32 i, qbs *str) {
    if (is_error_pending())
        return;
    static int32 x, x2, i2;

    if (((i >= 1) && (i <= 10)) || (i == 30) || (i == 31)) { // F1-F10,F11,F12
        if (str->len > 15) {
            error(5);
            return;
        }
        if (!onkey[i].text)
            onkey[i].text = qbs_new(0, 0);
        qbs_set(onkey[i].text, str);
        key_display_redraw = 1;
        key_update();
        return;
    } // F1-F10,F11,F12

    if ((i >= 15) && (i <= 29)) { // user defined key
        if (str->len == 0) {
            onkey[i].key_scancode = 0;
        } else {
            x = str->chr[str->len - 1];
            x2 = 0;
            for (i2 = 0; i2 < str->len - 1; i2++)
                x2 |= str->chr[i2];
            onkey[i].key_scancode = x;
            onkey[i].key_flags = x2;
        }
        return;
    } // user defined key

    error(5);
    return;
}

void sub_paletteusing(void *element, int32 bits) {
    // note: bits is either 16(INTEGER) or 32(LONG)
    if (is_error_pending())
        return;
    static byte_element_struct *ele;
    ele = (byte_element_struct *)element;
    static int16 *i16;
    i16 = (int16 *)ele->offset;
    static int32 *i32;
    i32 = (int32 *)ele->offset;
    if (write_page->bits_per_pixel == 32)
        goto error;
    static int32 last_color, i, c;
    last_color = write_page->mask;
    if (ele->length < ((bits / 8) * (last_color + 1)))
        goto error;
    if ((write_page->compatible_mode == 11) || (write_page->compatible_mode == 12) || (write_page->compatible_mode == 13) ||
        (write_page->compatible_mode == 256)) {
        if (bits == 16)
            goto error; // must be an array of type LONG in these modes
    }
    for (i = 0; i <= last_color; i++) {
        if (bits == 16) {
            c = *i16;
            i16++;
        } else {
            c = *i32;
            i32++;
        }
        if (c < -1)
            goto error;
        if (c != -1) {
            qbg_palette(i, c, 1);
            if (is_error_pending())
                return;
        }
    }
    return;
error:

    error(5);
}

extern int32 func__devices();

int32 func_stick(int32 i, int32 axis_group, int32 passed) {
    // note: range: 1-254 (127=neutral), top-left to bottom-right positive
    //             128 returned for unattached devices
    // QB64 extension: 'i' allows for joystick selection 0,1->JoyA, 2,3->JoyB, 4,5->JoyC, etc
    //                'axis_group' selects the pair of axes to read from, 1 is the default
    if (device_last == 0)
        func__devices(); // init device interface (if not already setup)
    if (passed) {
        if (axis_group < 1 || axis_group > 65535) {
            error(5);
            return 0;
        }
    } else {
        axis_group = 1;
    }
    if (i < 0 || i > 65535) {
        error(5);
        return 0;
    }
    static int32 di, axis, i2, v;
    static device_struct *d;
    static float f;
    axis = (i & 1) + (axis_group - 1) * 2;
    i = i >> 1;
    i2 = 0;
    for (di = 1; di <= device_last; di++) {
        d = &devices[di];
        if (d->type == 1) {
            if (i == i2) {
                if (axis < d->lastaxis) {
                    f = getDeviceEventAxisValue(d, d->queued_events - 1, axis);
                    if (f > -0.01 && f <= 0.01)
                        f = 0;
                    v = qbr_float_to_long(f * 127.0) + 127;
                    if (v > 254)
                        v = 254;
                    if (v < 1)
                        v = 1;
                    return v;
                } // axis valid
            }
            i2++;
        } // type==1
    } // di
    return 128;
}

int32 func_strig(int32 i, int32 controller, int32 passed) {
    // note: returns 0 or -1(true)
    // QB64 extension: 'i' refers to a button (b1,b1,b1,b1,b2,b2,b2,b2,b3,b3,b3,b3,b4,...)
    //                'controller' overrides the controller implied by 'i', 1 is the default
    if (device_last == 0)
        func__devices(); // init device interface (if not already setup)
    if (i < 0 || i > 65535) {
        error(5);
        return 0;
    }
    if (passed) {
        if (controller < 1 || controller > 65535) {
            error(5);
            return 0;
        }
    } else {
        controller = 1;
        if (i & 2) {
            controller = 2;
            i -= 2;
        }
    }
    static int32 di, button, method, c, v;
    static device_struct *d;
    button = (i >> 2) + 1;
    method = (i & 1) + 1; // 1=if pressed since last call, 2=currently down
    c = 1;
    for (di = 1; di <= device_last; di++) {
        d = &devices[di];
        if (d->type == 1) {
            if (c == controller) {
                if (button <= d->lastbutton) { // button exists
                    if (method == 1) {
                        // method 1: pressed since last call
                        if (button > 0 && button <= 256) {
                            if (d->STRIG_button_pressed[button - 1]) {
                                d->STRIG_button_pressed[button - 1] = 0;
                                return -1;
                            }
                        }
                        return 0;
                    } else {
                        // method 2: currently down
                        v = getDeviceEventButtonValue(d, d->queued_events - 1, button - 1);
                        if (v)
                            return -1;
                        else
                            return 0;
                    }
                } // button exists
            } // c==controller
            c++;
        } // type==1
    } // di
    return 0;
}

// func__console, sub__console moved to console.cpp

void sub__screenshow() {
#ifdef QB64_GLUT
    screen_hide = 0;
    // $SCREENHIDE programs will not have the window running
    libqb_start_glut_thread();
    libqb_glut_show_window();
#endif
}

void sub__screenhide() {
    if (screen_hide)
        return;

#ifdef QB64_GLUT
    // This is probably unnecessary, no conditions allow for screen_hide==0
    // without GLUT running, but it doesn't hurt anything.
    libqb_start_glut_thread();
    libqb_glut_hide_window();
#endif

    screen_hide = 1;
}

int32 func__screenhide() {
    return -screen_hide;
}

// sub__consoletitle moved to console.cpp

mem_block func__memimage(int32 i, int32 passed) {

    static mem_block b;

    if (is_error_pending())
        goto error;

    static int image_handle;

    static img_struct *im;
    if (passed) {
        if (i >= 0) {
            validatepage(i);
            im = &img[image_handle = page[i]];
            image_handle = -image_handle;
        } else {
            image_handle = i;
            i = -i;
            if (i >= nextimg) {
                error(258);
                goto error;
            }
            im = &img[i];
            if (!im->valid) {
                error(258);
                goto error;
            }
        }
    } else {
        im = write_page;
    }

    if (im->lock_id) {
        b.lock_offset = (ptrszint)im->lock_offset;
        b.lock_id = im->lock_id; // get existing tag
    } else {
        new_mem_lock();
        mem_lock_tmp->type = 2; // image
        b.lock_offset = (ptrszint)mem_lock_tmp;
        b.lock_id = mem_lock_id;
        im->lock_offset = (void *)mem_lock_tmp;
        im->lock_id = mem_lock_id; // create tag
    }

    b.offset = (ptrszint)im->offset;
    b.size = im->bytes_per_pixel * im->width * im->height;
    b.type = im->bytes_per_pixel + 128 + 1024 + 2048; // integer+unsigned+pixeltype
    b.elementsize = im->bytes_per_pixel;
    b.image = image_handle;

    return b;
error:
    b.offset = 0;
    b.size = 0;
    b.lock_offset = (ptrszint)mem_lock_base;
    b.lock_id = 1073741821; // set invalid lock
    b.type = 0;
    b.elementsize = 0;
    b.image = -1;
    return b;
}

void GLUT_key_ascii(int32 key, int32 down) {
#ifdef QB64_GLUT
    static int32 v;

    static int32 mod;
    mod = glutGetModifiers(); // shift=1, control=2, alt=4

#    ifndef CORE_FREEGLUT
    /*
        if (mod&GLUT_ACTIVE_SHIFT){
        keydown_vk(VK+QBVK_LSHIFT);
        }else{
        keyup_vk(VK+QBVK_LSHIFT);
        }

        if (mod&GLUT_ACTIVE_CTRL){
        keydown_vk(VK+QBVK_LCTRL);
        }else{
        keyup_vk(VK+QBVK_LCTRL);
        }

        if (mod&GLUT_ACTIVE_ALT){
        keydown_vk(VK+QBVK_LALT);
        }else{
        keyup_vk(VK+QBVK_LALT);
        }
    */
#    endif

    // Note: The following is required regardless of whether FREEGLUT is/isn't being used
    // #ifdef CORE_FREEGLUT
    // Is CTRL key down? If so, unencode character (applying shift as required)
    if (mod & 2) {
        // if (key==127){ //Removed: Might clash with CTRL+DELETE
        // key=8;
        // goto ctrl_mod;
        //}//127
        // if (key==3){//CTRL+(BREAK|SCROLL-LOCK)
        // if (down) keydown_vk(VK+QBVK_BREAK); else keyup_vk(VK+QBVK_BREAK);
        // return;
        //}
        if (key == 10) {
            key = 13;
            goto ctrl_mod;
        } // 10
        if ((key >= 1) && (key <= 26)) {
            if (mod & 1)
                key = key - 1 + 65;
            else
                key = key - 1 + 97; // assume caps lock off
            goto ctrl_mod;
        } // 1-26
    }
ctrl_mod:
    // #endif

#    ifdef QB64_MACOSX

    // swap DEL and backspace keys

    if (key == 8) {
        key = 127;
    } else {
        if (key == 127) {
            key = 8;
        }
    }

#    endif

    if (key == 127) { // delete
        if (down)
            keydown_vk(0x5300);
        else
            keyup_vk(0x5300);
        return;
    }
    if (down)
        keydown_ascii(key);
    else
        keyup_ascii(key);
#endif
}

void GLUT_KEYBOARD_FUNC(unsigned char key, int x, int y) {

    // glutPostRedisplay();

    // qbs_print(qbs_str(key),0);
    // qbs_print(qbs_str((int32)glutGetModifiers()),1);

    GLUT_key_ascii(key, 1);
}

void GLUT_KEYBOARDUP_FUNC(unsigned char key, int x, int y) {
    GLUT_key_ascii(key, 0);
}

void GLUT_key_special(int32 key, int32 down) {
#ifdef QB64_GLUT
#    ifndef CORE_FREEGLUT
    /*
        static int32 mod;
        mod=glutGetModifiers();//shift=1, control=2, alt=4
        if (mod&GLUT_ACTIVE_SHIFT){
        keydown_vk(VK+QBVK_LSHIFT);
        }else{
        keyup_vk(VK+QBVK_LSHIFT);
        }

        if (mod&GLUT_ACTIVE_CTRL){
        keydown_vk(VK+QBVK_LCTRL);
        }else{
        keyup_vk(VK+QBVK_LCTRL);
        }

        if (mod&GLUT_ACTIVE_ALT){
        keydown_vk(VK+QBVK_LALT);
        }else{
        keyup_vk(VK+QBVK_LALT);
        }
    */
#    endif

    static int32 vk;
    vk = -1;
    if (key == GLUT_KEY_F1) {
        vk = 0x3B00;
    }
    if (key == GLUT_KEY_F2) {
        vk = 0x3C00;
    }
    if (key == GLUT_KEY_F3) {
        vk = 0x3D00;
    }
    if (key == GLUT_KEY_F4) {
        vk = 0x3E00;
    }
    if (key == GLUT_KEY_F5) {
        vk = 0x3F00;
    }
    if (key == GLUT_KEY_F6) {
        vk = 0x4000;
    }
    if (key == GLUT_KEY_F7) {
        vk = 0x4100;
    }
    if (key == GLUT_KEY_F8) {
        vk = 0x4200;
    }
    if (key == GLUT_KEY_F9) {
        vk = 0x4300;
    }
    if (key == GLUT_KEY_F10) {
        vk = 0x4400;
    }
    if (key == GLUT_KEY_F11) {
        vk = 0x8500;
    }
    if (key == GLUT_KEY_F12) {
        vk = 0x8600;
    }
    if (key == GLUT_KEY_LEFT) {
        vk = 0x4B00;
    }
    if (key == GLUT_KEY_UP) {
        vk = 0x4800;
    }
    if (key == GLUT_KEY_RIGHT) {
        vk = 0x4D00;
    }
    if (key == GLUT_KEY_DOWN) {
        vk = 0x5000;
    }
    if (key == GLUT_KEY_PAGE_UP) {
        vk = 0x4900;
    }
    if (key == GLUT_KEY_PAGE_DOWN) {
        vk = 0x5100;
    }
    if (key == GLUT_KEY_HOME) {
        vk = 0x4700;
    }
    if (key == GLUT_KEY_END) {
        vk = 0x4F00;
    }
    if (key == GLUT_KEY_INSERT) {
        vk = 0x5200;
    }

#    ifdef CORE_FREEGLUT
    if (key == 112) {
        vk = VK + QBVK_LSHIFT;
    }
    if (key == 113) {
        vk = VK + QBVK_RSHIFT;
    }
    if (key == 114) {
        vk = VK + QBVK_LCTRL;
    }
    if (key == 115) {
        vk = VK + QBVK_RCTRL;
    }
    if (key == 116) {
        vk = VK + QBVK_LALT;
    }
    if (key == 117) {
        vk = VK + QBVK_RALT;
    }
#    endif

    if (vk != -1) {
#    ifdef QB64_WINDOWS
        if (!func__hasfocus() && !(keyheld(vk) && !down))
            return;
#    endif

        if (down)
            keydown_vk(vk);
        else
            keyup_vk(vk);
    }

#endif
}

void GLUT_SPECIAL_FUNC(int key, int x, int y) {

    // qbs_print(qbs_str((int32)glutGetModifiers()),1);

    GLUT_key_special(key, 1);
}

void GLUT_SPECIALUP_FUNC(int key, int x, int y) {
    GLUT_key_special(key, 0);
}

static int64_t lastTick = 0;
static double deltaTick = 0;

void GLUT_IDLEFUNC() {
    libqb_process_glut_queue();

#ifdef QB64_MACOSX
#    ifdef DEPENDENCY_DEVICEINPUT
    // must be in same thread as GLUT for OSX
    QB64_GAMEPAD_POLL();
    //[[[[NSApplication sharedApplication] mainWindow] standardWindowButton:NSWindowCloseButton] setEnabled:YES];
#    endif
#endif

#ifdef QB64_GLUT

#    ifdef QB64_LINUX
    if (x11_lock_request) {
        x11_locked = 1;
        x11_lock_request = 0;
        while (x11_locked)
            Sleep(1);
    }
#    endif
    int64_t curTime = GetTicks();

    // This is how long the frame took to render
    int64_t elapsed = curTime - lastTick;

    // Calculate out the error between how long the frame was 'supposed' to take vs. how long it actually took.
    deltaTick += ((double)1000 / get_max_fps()) - (double)elapsed;

    lastTick = curTime;

    // If the error is positive, we sleep for that period of time.
    if (deltaTick > 0) {
        int32 msdelay = deltaTick;
        Sleep(msdelay);

        int64_t sleepTime = GetTicks();

        // Subtract off the time we spent sleeping. This should leave deltaTick at zero or slightly negative.
        // If it ends up negative, then we'll sleep less next frame to compensate
        deltaTick -= sleepTime - lastTick;
        lastTick = sleepTime;
    } else {
        // If we fall behind by a full frame or more, then skip to the next one
        while (deltaTick < -((double)1000 / get_max_fps()))
            deltaTick += ((double)1000 / get_max_fps());
    }

    glutPostRedisplay();
#endif
}

#ifdef QB64_MACOSX
#    include <sys/sysctl.h>
#endif

// trigger recompilation = 3

int32 displayorder_screen = 1;
int32 displayorder_hardware = 2;
int32 displayorder_glrender = 3;
int32 displayorder_hardware1 = 4;

// sub__displayorder( 1 , 2 , 4 , 3 );
// id.specialformat =
// "[{_SCREEN|_HARDWARE|_HARDWARE1|_GLRENDER}[,{_SCREEN|_HARDWARE|_HARDWARE1|_GLRENDER}[,{_SCREEN|_HARDWARE|_HARDWARE1|_GLRENDER}[,{_SCREEN|_HARDWARE|_HARDWARE1|_GLRENDER}]]]]"
void sub__displayorder(int32 method1, int32 method2, int32 method3, int32 method4) {

    // check no value has been used twice
    if (method1 != 0)
        if (method1 == method2 || method1 == method3 || method1 == method4) {
            error(5);
            return;
        }
    if (method2 != 0)
        if (method2 == method1 || method2 == method3 || method2 == method4) {
            error(5);
            return;
        }
    if (method3 != 0)
        if (method3 == method1 || method3 == method2 || method3 == method4) {
            error(5);
            return;
        }
    if (method4 != 0)
        if (method4 == method1 || method4 == method2 || method4 == method3) {
            error(5);
            return;
        }
    displayorder_screen = 0;
    displayorder_hardware = 0;
    displayorder_hardware1 = 0;
    displayorder_glrender = 0;
    static int32 i, method;
    for (i = 1; i <= 4; i++) {
        if (i == 1)
            method = method1;
        if (i == 2)
            method = method2;
        if (i == 3)
            method = method3;
        if (i == 4)
            method = method4;
        if (method == 1)
            displayorder_screen = i;
        if (method == 2)
            displayorder_hardware = i;
        if (method == 3)
            displayorder_hardware1 = i;
        if (method == 4)
            displayorder_glrender = i;
    }
}

// int32 gl_render_method=2; //1=behind, 2=ontop[default], 3=only
void sub__glrender(int32 method) {
    // gl_render_method=method;
    if (method == 1)
        sub__displayorder(4, 1, 2, 3);
    if (method == 2)
        sub__displayorder(1, 2, 4, 3);
    if (method == 3)
        sub__displayorder(4, 0, 0, 0);
}

#ifndef QB64_GUI // begin stubs

#else // end stubs

void GLUT_RESHAPE_FUNC(int width, int height) {
    resize_event_x = width;
    resize_event_y = height;
    resize_event = -1;
    display_x_prev = display_x, display_y_prev = display_y;
    display_x = width;
    display_y = height;
    resize_pending = 0;
    os_resize_event = 1;
    set_view(VIEW_MODE__UNKNOWN);
    //***glutReshapeWindow(...) has no effect if called
    //   within GLUT_RESHAPE_FUNC***
}

// displaycall is the window of time to update our display

#    ifdef DEPENDENCY_GL
extern void SUB__GL();
#    endif

#    define GL_BGR 0x80E0
#    define GL_BGRA 0x80E1

/* reference
    struct hardware_img_struct{
    int32 w;
    int32 h;
    int32 texture_handle;
    int32 dest_context_handle;//used when rendering other images onto this image
    int32 temp;//if =1, delete immediately after use
    }
    list *hardware_img_handles=NULL;
*/

// free_hardware_img moved to graphics.cpp

/*
    int32 new_hardware_frame(int32 x, int32 y){
    int32 handle=new_hardware_frame_handle();
    glBindTexture (GL_TEXTURE_2D, handle);
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE,
   NULL); return handle;
    }

    void free_hardware_frame(int32 handle){
    GLuint texture=(GLuint)handle;
    glDeleteTextures(1, &texture);
    }
*/

void prepare_environment_2d() { // called prior to rendering 2D content

    // precalculate critical dimensions, offsets & ratios

    static int32 can_scale; // can the screen be scaled on the window
    can_scale = 0;
    static int32 need_square_pixels; // do we need square_pixels? if not we can
                                     // stretch the screen
    need_square_pixels = 0;

    environment_2d__screen_smooth = 0;

    environment_2d__letterbox = 0;

    if (full_screen > 0) { // in full screen
        // reference: ---int32
        // full_screen_set=-1;//0(windowed),1(stretched/closest),2(1:1)---
        can_scale = 1;
        if (full_screen == 2)
            need_square_pixels = 1;
        // note: 'letter-boxing' is only required where the size of the window
        // cannot be controlled, and the only place where this is the
        //      case is full screen mode _SQUAREPIXELS
        environment_2d__screen_smooth = fullscreen_smooth;
    } else {                   // windowed
        if (resize_auto > 0) { // 1=STRETCH,2=SMOOTH
            can_scale = 1;
            if (resize_auto == 2)
                environment_2d__screen_smooth = 1;
            // note: screen will fix its aspect ratio automatically, so there is
            // no need to enforce squarepixels
        }
    }

    if (environment_2d__screen_width == environment__window_width && environment_2d__screen_height == environment__window_height) {
        // screen size matches window
        environment_2d__screen_x1 = 0;
        environment_2d__screen_y1 = 0;
        environment_2d__screen_x2 = environment_2d__screen_width - 1;
        environment_2d__screen_y2 = environment_2d__screen_height - 1;
        environment_2d__screen_x_scale = 1.0f;
        environment_2d__screen_y_scale = 1.0f;
        environment_2d__screen_scaled_width = environment_2d__screen_width;
        environment_2d__screen_scaled_height = environment_2d__screen_height;
        environment_2d__screen_smooth = 0; // no smoothing required
    } else {
        // screen size does not match
        // calculate ratios
        static float window_ratio;
        static float screen_ratio;
        window_ratio = (float)environment__window_width / (float)environment__window_height;
        screen_ratio = (float)environment_2d__screen_width / (float)environment_2d__screen_height;
        if (can_scale == 0) {
            // screen will appear in the top-left of the window with blank space
            // on the bottom & right
            environment_2d__screen_x1 = 0;
            environment_2d__screen_y1 = 0;
            environment_2d__screen_x2 = environment_2d__screen_width - 1;
            environment_2d__screen_y2 = environment_2d__screen_height - 1;
            goto cant_scale;
        }
        if (need_square_pixels == 0 || (window_ratio == screen_ratio)) {
            // can stretch, no 'letter-box' required
            environment_2d__screen_x1 = 0;
            environment_2d__screen_y1 = 0;
            environment_2d__screen_x2 = environment__window_width - 1;
            environment_2d__screen_y2 = environment__window_height - 1;
        } else {
            //'letter-box' required
            // this section needs revision
            static float x_scale, y_scale;
            static int32 x1, y1, x2, y2, z, x_limit, y_limit, x_offset, y_offset;
            // x_scale=(float)environment_2d__screen_width/(float)environment__window_width;
            // y_scale=(float)environment_2d__screen_height/(float)environment__window_height;
            // x_offset=0; y_offset=0;

            x1 = 0;
            y1 = 0;
            x2 = environment__window_width - 1;
            y2 = environment__window_height - 1;
            // x_limit=x2; y_limit=y2;
            if (window_ratio > screen_ratio) {
                // pad sides
                z = (float)environment__window_height * screen_ratio; // new width
                x1 = environment__window_width / 2 - z / 2;
                x2 = x1 + z - 1;
                environment_2d__letterbox = 1; // vertical black stripes
                                               // required
                // x_offset=-x1;
                // x_scale=(float)environment_2d__screen_width/(float)z;
                // x_limit=z-1;
            } else {
                // pad top/bottom
                z = (float)environment__window_width / screen_ratio; // new height
                y1 = environment__window_height / 2 - z / 2;
                y2 = y1 + z - 1;
                environment_2d__letterbox = 2; // horizontal black stripes required
                // y_offset=-y1;
                // y_scale=(float)environment_2d__screen_height/(float)z;
                // y_limit=z-1;
            }
            environment_2d__screen_x1 = x1;
            environment_2d__screen_y1 = y1;
            environment_2d__screen_x2 = x2;
            environment_2d__screen_y2 = y2;
        }
    cant_scale:
        environment_2d__screen_scaled_width = environment_2d__screen_x2 - environment_2d__screen_x1 + 1;
        environment_2d__screen_scaled_height = environment_2d__screen_y2 - environment_2d__screen_y1 + 1;
        environment_2d__screen_x_scale = (float)environment_2d__screen_scaled_width / (float)environment_2d__screen_width;
        environment_2d__screen_y_scale = (float)environment_2d__screen_scaled_height / (float)environment_2d__screen_height;
    }

} // prepare_environment_2d

int32 environment_2d__get_window_x1_coord(int32 x) {
    return qbr_float_to_long(((float)x) * environment_2d__screen_x_scale) + environment_2d__screen_x1;
}

int32 environment_2d__get_window_y1_coord(int32 y) {
    return qbr_float_to_long((float)y * environment_2d__screen_y_scale) + environment_2d__screen_y1;
}

int32 environment_2d__get_window_x2_coord(int32 x) {
    return qbr_float_to_long(((float)x + 1.0f) * environment_2d__screen_x_scale - 1.0f) + environment_2d__screen_x1;
}

int32 environment_2d__get_window_y2_coord(int32 y) {
    return qbr_float_to_long(((float)y + 1.0f) * environment_2d__screen_y_scale - 1.0f) + environment_2d__screen_y1;
}

struct environment_2d__window_rect_struct {
    int32 x1;
    int32 y1;
    int32 x2;
    int32 y2;
};

// this functions returns a constant rect dimensions to stop warping of image
environment_2d__window_rect_struct tmp_rect;

environment_2d__window_rect_struct *environment_2d__screen_to_window_rect(int32 x1, int32 y1, int32 x2, int32 y2) {
    tmp_rect.x1 = qbr_float_to_long(((float)x1) * environment_2d__screen_x_scale) + environment_2d__screen_x1;
    tmp_rect.y1 = qbr_float_to_long(((float)y1) * environment_2d__screen_y_scale) + environment_2d__screen_y1;
    static int32 w, h;
    w = abs(x2 - x1) + 1;
    h = abs(y2 - y1) + 1;
    // force round upwards to correct gaps when tiling
    w = ((float)w) * environment_2d__screen_x_scale + 0.99f;
    h = ((float)h) * environment_2d__screen_y_scale + 0.99f;
    tmp_rect.x2 = w - 1 + tmp_rect.x1;
    tmp_rect.y2 = h - 1 + tmp_rect.y1;
    //(code which doesn't support tiling)
    // tmp_rect.x2=qbr_float_to_long(((float)w)*environment_2d__screen_x_scale-1.0f)+tmp_rect.x1;
    // tmp_rect.y2=qbr_float_to_long(((float)h)*environment_2d__screen_y_scale-1.0f)+tmp_rect.y1;
    return &tmp_rect;
}

// hardware_buffer_* globals and hardware_buffer_flush, set_smooth, set_texture_wrap moved to graphics.cpp
extern float *hardware_buffer_vertices;
extern int32 hardware_buffer_vertices_max;
extern int32 hardware_buffer_vertices_count;
extern float *hardware_buffer_texcoords;
extern int32 hardware_buffer_texcoords_max;
extern int32 hardware_buffer_texcoords_count;

void set_alpha(int32 new_mode) {
    static int32 current_mode;
    current_mode = render_state.use_alpha;
    if (new_mode == current_mode)
        return;
    hardware_buffer_flush();
    if (new_mode == ALPHA_MODE__DONT_BLEND) {
        glDisable(GL_BLEND);
    }
    if (new_mode == ALPHA_MODE__BLEND) {
        glEnable(GL_BLEND);
        if (framebufferobjects_supported) {
            // glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
            // GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glBlendFuncSeparateEXT(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
        } else {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
    }
    render_state.use_alpha = new_mode;
}

void set_depthbuffer(int32 new_mode) {

    static int32 current_mode;
    current_mode = render_state.depthbuffer_mode;
    if (new_mode == current_mode)
        return;
    hardware_buffer_flush();
    if (new_mode == DEPTHBUFFER_MODE__OFF) {
        glDisable(GL_DEPTH_TEST);
        glAlphaFunc(GL_ALWAYS, 0);
    }
    if (new_mode == DEPTHBUFFER_MODE__ON) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glAlphaFunc(GL_GREATER, 0.001);
        glEnable(GL_ALPHA_TEST);
    }
    if (new_mode == DEPTHBUFFER_MODE__LOCKED) {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glAlphaFunc(GL_ALWAYS, 0);
    }
    render_state.depthbuffer_mode = new_mode;
}

void set_cull_mode(int32 new_mode) {
    static int32 current_mode;
    current_mode = render_state.cull_mode;
    if (new_mode == current_mode)
        return;
    hardware_buffer_flush();
    if (new_mode == CULL_MODE__NONE) {
        glDisable(GL_CULL_FACE);
    }
    if (new_mode == CULL_MODE__CLOCKWISE_ONLY) {
        glFrontFace(GL_CW);
        if (current_mode != CULL_MODE__ANTICLOCKWISE_ONLY)
            glEnable(GL_CULL_FACE);
    }
    if (new_mode == CULL_MODE__ANTICLOCKWISE_ONLY) {
        glFrontFace(GL_CCW);
        if (current_mode != CULL_MODE__CLOCKWISE_ONLY)
            glEnable(GL_CULL_FACE);
    }
    render_state.cull_mode = new_mode;
}

void set_view(int32 new_mode) { // set view can only be called after the correct
                                // destination is chosen
    static int32 current_mode;
    current_mode = render_state.view_mode;
    if (new_mode == current_mode)
        return;
    hardware_buffer_flush();
    if (new_mode == VIEW_MODE__RESET) {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_BLEND);
        glDisable(GL_COLOR_MATERIAL);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDisable(GL_LIGHTING);
        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glAlphaFunc(GL_ALWAYS, 0);
        if (framebufferobjects_supported)
            glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        glColor4f(1.f, 1.f, 1.f, 1.f);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // invalidate current states
        set_alpha(ALPHA_MODE__UNKNOWN);
        set_depthbuffer(DEPTHBUFFER_MODE__UNKNOWN);
        set_cull_mode(CULL_MODE__UNKNOWN);
        set_render_source(INVALID_HARDWARE_HANDLE);
        set_render_dest(INVALID_HARDWARE_HANDLE);
        new_mode = VIEW_MODE__UNKNOWN; // resets are performed before unknown
                                       // operations are executed
    }
    if (new_mode == VIEW_MODE__2D) {
        if (current_mode != VIEW_MODE__3D) {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            glDisable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            set_alpha(ALPHA_MODE__BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glCullFace(GL_BACK);
        }

        if (render_state.dest_handle == 0) {
            static int32 dst_w, dst_h;
            static int32 scale_factor = 0;

#    ifdef QB64_MACOSX
            if (scale_factor == 0) {
                // by default scale_factor should be 1, but in macOS Catalina
                // (10.15.*) scale_factor must be setted in 2
                // * in cases where the app is executed on system with Retina
                // Display
                scale_factor = 1; // by default

                // lookup for retina/5k output from system_profiler (storing all
                // outpun in stream)
                bool b_isRetina, b_is5k;
                FILE *consoleStream = popen("system_profiler SPDisplaysDataType 2>/dev/null", "r");
                if (consoleStream) {
                    char buffer[128];
                    while (!feof(consoleStream)) {
                        if (fgets(buffer, 128, consoleStream) != NULL) {
                            std::string szBuffer(buffer);

                            if (!b_isRetina)
                                b_isRetina = (szBuffer.rfind("Retina") != ULONG_MAX);
                            if (!b_is5k)
                                b_is5k = (szBuffer.rfind("5K") != ULONG_MAX);
                        }
                    }
                }
                pclose(consoleStream);

                if (b_isRetina || b_is5k) {
                    // apply only factor = 2 if macOS is Catalina (11.15.* //
                    // kern.osrelease 19.*)
                    char str[256];
                    size_t size = sizeof(str);
                    int ret = sysctlbyname("kern.osrelease", str, &size, NULL, 0);

                    std::string sz_osrelease(str);
                    if (sz_osrelease.rfind("19.") == 0)
                        scale_factor = 2;
                }
            }
#    else
            scale_factor = 1;
#    endif

            dst_w = environment__window_width;
            dst_h = environment__window_height;

            // alert(dst_w);
            // alert(dst_h);

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(0.0, dst_w, 0.0, dst_h, -1.0, 1.0);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glScalef(1, -1, 1);         // flip vertically
            glTranslatef(0, -dst_h, 0); // move to new vertical position
            glViewport(0, 0, dst_w * scale_factor, dst_h * scale_factor);

        } else {
            static hardware_img_struct *hardware_img;
            hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, render_state.dest_handle);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            gluOrtho2D(0, hardware_img->w, 0, hardware_img->h);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glViewport(0, 0, hardware_img->w, hardware_img->h);
        }
    }
    if (new_mode == VIEW_MODE__3D) {
        if (current_mode != VIEW_MODE__2D) {
            glColor4f(1.f, 1.f, 1.f, 1.f);
            glDisable(GL_COLOR_MATERIAL);
            glDisable(GL_LIGHTING);
            set_alpha(ALPHA_MODE__BLEND);
            glEnable(GL_TEXTURE_2D);
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glCullFace(GL_BACK);
        }
        if (render_state.dest_handle == 0) {
            static int32 dst_w, dst_h;
            dst_w = environment__window_width;
            dst_h = environment__window_height;
            glViewport(0, 0, (GLsizei)dst_w, (GLsizei)dst_h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();

            // note: the max FOV is 90-degrees (this maximum applies to the
            // longest screen dimension)
            float fov;
            if (environment_2d__screen_scaled_width > environment_2d__screen_scaled_height) {
                fov = 90.0f * ((float)environment__window_width / (float)environment_2d__screen_scaled_width);
                // convert fov from horizontal to vertical
                fov = fov * ((float)dst_h / (float)dst_w);
            } else {
                fov = 90.0f * ((float)environment__window_height / (float)environment_2d__screen_scaled_height);
            }
            gluPerspective(fov, (GLfloat)dst_w / (GLfloat)dst_h, 0.1,
                           10000.0); // Set the Field of view angle (in
                                     // degrees), the aspect ratio of our
                                     // window, and the new and far planes
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
        } else {

            static hardware_img_struct *hardware_img;
            hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, render_state.dest_handle);

            static int32 dst_w, dst_h;
            dst_w = hardware_img->w;
            dst_h = hardware_img->h;
            glViewport(0, 0, (GLsizei)dst_w, (GLsizei)dst_h);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glScalef(1.0, -1.0, 1.0);
            // note: the max FOV is 90-degrees (this maximum applies to the
            // longest screen dimension)
            float fov;
            if (dst_w > dst_h) {
                fov = 90.0f;
                // convert fov from horizontal to vertical
                fov = fov * ((float)dst_h / (float)dst_w);
            } else {
                fov = 90.0f;
            }
            gluPerspective(fov, (GLfloat)dst_w / (GLfloat)dst_h, 0.1,
                           10000.0); // Set the Field of view angle (in
                                     // degrees), the aspect ratio of our
                                     // window, and the new and far planes
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            // alert("3D rendering onto FBO not supported yet");
        }
    }
    render_state.view_mode = new_mode;
} // change_render_state

// set_render_source, set_render_dest, hardware_img_put, hardware_img_tri2d,
// clear_depthbuffer, hardware_img_tri3d moved to graphics.cpp


static int32 software_screen_hardware_frame = 0;

static int32 in_GLUT_DISPLAY_REQUEST = 0;

void GLUT_DISPLAY_REQUEST() {

    if (in_GLUT_DISPLAY_REQUEST) {
        return;
    }
    in_GLUT_DISPLAY_REQUEST = 1;

    // general use variables
    static int32 i, i2, i3;
    static int32 x, y, x2, y2;

    // determine which software frame to display
    static int32 last_i; // the last software frame displayed
    last_i = -1;
    for (i2 = 0; i2 <= 2; i2++) {
        if (display_frame[i2].state == DISPLAY_FRAME_STATE__DISPLAYING) {
            last_i = i2;
        }
    }
    i = -1;
    static int64 highest_order;
    highest_order = 0;
    if (last_i != -1)
        highest_order = display_frame[last_i].order; // avoid any frames below the current one
    for (i2 = 0; i2 <= 2; i2++) {
        if (display_frame[i2].state == DISPLAY_FRAME_STATE__READY && display_frame[i2].order > highest_order) {
            highest_order = display_frame[i2].order;
            i = i2;
        }
    }
    if (i == -1)
        i = last_i;
    if (i == -1) {
        in_GLUT_DISPLAY_REQUEST = 0;
        return; // no frames exist yet, so screen size cannot be determined,
                // therefore no action possible
    }
    if (i != last_i) {
        for (i2 = 0; i2 <= 2; i2++) {
            if (display_frame[i2].order < display_frame[i].order &&
                (display_frame[i2].state == DISPLAY_FRAME_STATE__DISPLAYING || display_frame[i2].state == DISPLAY_FRAME_STATE__READY))
                display_frame[i2].state = DISPLAY_FRAME_STATE__EMPTY;
        }
        display_frame[i].state = DISPLAY_FRAME_STATE__DISPLAYING;
    }

    static int64 order;
    order = last_hardware_display_frame_order;

    static int32 first_command_prev_order = 0;
    static int32 rerender_prev_hardware_frame = 0;
    rerender_prev_hardware_frame = 0;

    // if no new software frame, only proceed if there is _GL content to render
    if (last_rendered_hardware_display_frame_order == last_hardware_display_frame_order) {
        if (i == last_i) {
            if (full_screen_set == -1) {    // no pending full-screen changes
                if (os_resize_event == 0) { // no resize events
#    ifndef DEPENDENCY_GL // we aren't using SUB _GL
                    in_GLUT_DISPLAY_REQUEST = 0;
                    return;
#    endif
                    if (displayorder_glrender == 0) {
                        in_GLUT_DISPLAY_REQUEST = 0;
                        return;
                    }
                    if (first_command_prev_order) {
                        rerender_prev_hardware_frame = 1;
                        // reset next command to prev hardware frame's handle
                        // (if any)
                        last_hardware_command_rendered = first_command_prev_order;
                    }
                }
            }
        }
    }

    first_command_prev_order = 0;

    // set environment variables
    environment_2d__screen_width = display_frame[i].w;
    environment_2d__screen_height = display_frame[i].h;

    os_resize_event = 0; // turn off flag which forces a render to take place
                         // even if no content has changed

    if ((full_screen == 0) && (full_screen_set == -1)) { // not in (or attempting to enter) full screen

        display_required_x = display_frame[i].w;
        display_required_y = display_frame[i].h;
        static int32 framesize_changed;
        framesize_changed = 0;
        if ((display_required_x != resize_snapback_x) || (display_required_y != resize_snapback_y))
            framesize_changed = 1;

        resize_auto_ideal_aspect = (float)display_frame[i].w / (float)display_frame[i].h;
        resize_snapback_x = display_required_x;
        resize_snapback_y = display_required_y;

        if (resize_auto) {
            // maintain aspect ratio
            static float ar;
            ar = (float)display_x / (float)display_y;
            if ((ar != resize_auto_accept_aspect) && (ar != resize_auto_ideal_aspect)) {
                // set new size
                static int32 x, y;
                if (display_x_prev == display_x) {
                    y = display_y;
                    x = (float)y * resize_auto_ideal_aspect;
                }
                if (display_y_prev == display_y) {
                    x = display_x;
                    y = (float)x / resize_auto_ideal_aspect;
                }
                if ((display_y_prev != display_y) && (display_x_prev != display_x)) {
                    if (abs(display_y_prev - display_y) < abs(display_x_prev - display_x)) {
                        x = display_x;
                        y = (float)x / resize_auto_ideal_aspect;
                    } else {
                        y = display_y;
                        x = (float)y * resize_auto_ideal_aspect;
                    }
                }
                resize_auto_accept_aspect = (float)x / (float)y;
                resize_pending = 1;
                glutReshapeWindow(x, y);
                glutPostRedisplay();

                goto auto_resized;
            }
        } // resize_auto

        if ((display_required_x != display_x) || (display_required_y != display_y)) {
            if (resize_snapback || framesize_changed) {
                glutReshapeWindow(display_required_x, display_required_y);
                glutPostRedisplay();
                resize_pending = 1;
            }
        }

    auto_resized:;

    } // not in (or attempting to enter) full screen

    // Pseudo-Fullscreen
    if (!resize_pending) {           // avoid switching to fullscreen before resize
                                     // operations take effect
        if (full_screen_set != -1) { // full screen mode change requested
            if (full_screen_set == 0) {
                if (full_screen != 0) {
                    // exit full screen
                    resize_pending = 1;
                    glutReshapeWindow(display_frame[i].w, display_frame[i].h);
                    glutPostRedisplay();
                }
                full_screen = 0;
                full_screen_set = -1;
            } else {
                if (full_screen == 0) {
                    glutFullScreen();
                }
                full_screen = full_screen_set;
                full_screen_set = -1;
            } // enter full screen
        } // full_screen_set check
    } // size pending check

    // This code is deprecated but kept for reference purposes
    // 1) It was found to be unstable
    // 2) Switching modes means a high chance of losing pre-loaded OpenGL
    // hardware textures/surfaces
    /*
        static int32 glut_window;
        //fullscreen
        if (!resize_pending){//avoid switching to fullscreen before resize
       operations take effect if (full_screen_set!=-1){//full screen mode change
       requested if (full_screen_set==0){
        //exit full screen
        glutLeaveGameMode();
        glutSetWindow(glut_window);
        reinit_glut_callbacks();
        full_screen=0;
        full_screen_set=-1;
        return;
        }else{
        static char game_mode_string[1000];
        static int32 game_mode_string_i;
        game_mode_string_i=0;
        game_mode_string_i+=snprintf(&game_mode_string[game_mode_string_i], sizeof(game_mode_string) - game_mode_string_i, "%d",
       display_frame[i].w); game_mode_string[game_mode_string_i++]=120;//"x"
        game_mode_string_i+=snprintf(&game_mode_string[game_mode_string_i], sizeof(game_mode_string) - game_mode_string_i, "%d",
       display_frame[i].h); game_mode_string[game_mode_string_i++]=58;//":"
        game_mode_string_i+=snprintf(&game_mode_string[game_mode_string_i], sizeof(game_mode_string) - game_mode_string_i, "%d",
       32); glutGameModeString(game_mode_string);
        if(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)){
        //full screen using native dimensions which match the frame size
        if (full_screen==0) glut_window=glutGetWindow();
        glutEnterGameMode();
        fullscreen_width=display_frame[i].w;
       fullscreen_height=display_frame[i].h; reinit_glut_callbacks();
        full_screen=full_screen_set;//it's currently irrelevant if it is
       stretched or 1:1 full_screen_set=-1; return; }else{ //native dimensions
       not possible
        //attempt full screen using desktop dimensions
        static int32 w; w=glutGet(GLUT_SCREEN_WIDTH);
        static int32 h; h=glutGet(GLUT_SCREEN_HEIGHT);
        game_mode_string_i=0;
        game_mode_string_i+=snprintf(&game_mode_string[game_mode_string_i], sizeof(game_mode_string) - game_mode_string_i, "%d",
       w); game_mode_string[game_mode_string_i++]=120;//"x"
        game_mode_string_i+=snprintf(&game_mode_string[game_mode_string_i], sizeof(game_mode_string) - game_mode_string_i, "%d",
       h); game_mode_string[game_mode_string_i++]=58;//":"
        game_mode_string_i+=snprintf(&game_mode_string[game_mode_string_i], sizeof(game_mode_string) - game_mode_string_i, "%d",
       32); glutGameModeString(game_mode_string);
        if(glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)){
        //full screen using desktop dimensions
        if (full_screen==0) glut_window=glutGetWindow();
        glutEnterGameMode();
        fullscreen_width=w; fullscreen_height=h;
        reinit_glut_callbacks();
        screen_scale=full_screen_set;
        full_screen=full_screen_set;
        full_screen_set=-1;
        return;
        }else{
        //cannot enter full screen
        full_screen=0;
        full_screen_set=-1;
        }
        }
        }//enter full screen
        }//full_screen_set check
        }//size pending check
    */

    // set window environment variables
    environment__window_width = display_x;
    environment__window_height = display_y;

    prepare_environment_2d();

    // need a few variables here

    static int32 first_hardware_layer_rendered;
    static int32 first_hardware_layer_command;
    first_hardware_layer_rendered = 0;
    first_hardware_layer_command = 0;

    static int32 level;
    for (level = 0; level <= 5; level++) {

        static int32 x1, y1, x2, y2;

        if (level == 0) {
            set_render_dest(0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        } else {

            if (level == displayorder_glrender) {
#    ifdef DEPENDENCY_GL

                set_view(VIEW_MODE__RESET);

                if (close_program || dont_call_sub_gl || suspend_program || stop_program)
                    goto abort_gl;
                display_lock_request++;
                while (display_lock_confirmed < display_lock_request) {
                    if (close_program || dont_call_sub_gl || suspend_program || stop_program)
                        goto abort_gl;
                    qbevent = 1;
                    Sleep(0);
                }
                sub_gl_called = 1;
                SUB__GL();
                sub_gl_called = 0;
            abort_gl:;
                display_lock_released = display_lock_confirmed;

#    endif // DEPENDENCY_GL
            } // level==displayorder_glrender

            if (level == displayorder_screen) { // defaults to 1

                if (software_screen_hardware_frame != 0 && i != last_i) {
                    free_hardware_img(software_screen_hardware_frame, 847001);
                }
                if (i != last_i || software_screen_hardware_frame == 0) {
                    software_screen_hardware_frame = new_hardware_img(display_frame[i].w, display_frame[i].h, display_frame[i].bgra, NULL);
                }

                static hardware_img_struct *f1;
                f1 = (hardware_img_struct *)list_get(hardware_img_handles, software_screen_hardware_frame);
                if (software_screen_hardware_frame == 0) {
                    gui_alert("Invalid software_screen_hardware_frame!!");
                }
                if (f1 == NULL)
                    gui_alert("Invalid software_screen_hardware_frame!");

                static int32 use_alpha;
                use_alpha = 0;
                if (level > 1)
                    use_alpha = 1;

                // put the software screen
                hardware_img_put(0, 0, environment_2d__screen_width - 1, environment_2d__screen_height - 1, software_screen_hardware_frame, 0, 0, 0, f1->w - 1,
                                 f1->h - 1, use_alpha, environment_2d__screen_smooth);
                hardware_buffer_flush();

            } // level==displayorder_screen

            if (level == displayorder_hardware || level == displayorder_hardware1) {

                static int32 dst;
                dst = 0;
                if (level == displayorder_hardware1)
                    dst = -1;

                static int32 command;
                command = 0;

                static int32 caller_flag;
                caller_flag = 0;

                if (first_hardware_layer_rendered == 0) {

                    if (first_hardware_command) {

                        if (last_hardware_command_rendered) {

                            if (rerender_prev_hardware_frame) {
                                command = last_hardware_command_rendered;
                                caller_flag = 100;
                            } else {
                                hardware_graphics_command_struct *last_hgc =
                                    (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_rendered);
                                if (last_hgc == NULL)
                                    gui_alert("Rendering: Last HGC is NULL!");
                                command = last_hgc->next_command;
                                caller_flag = 200;
                            }

                        } else {

                            command = first_hardware_command;
                            caller_flag = 300;
                        }

                        // process/skip pending hardware puts before this
                        // frame's order value
                        while (command) {
                            hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, command);
                            if (hgc->order < order) {

                                if (hgc->command == HARDWARE_GRAPHICS_COMMAND__FREEIMAGE) {
                                    free_hardware_img(hgc->src_img, 847002 + caller_flag);
                                }

                                if (hgc->command == HARDWARE_GRAPHICS_COMMAND__PUTIMAGE) {
                                    if (hgc->dst_img > 0) { // note: rendering to the old
                                                            // default surface is pointless, but
                                                            // renders onto maintained hardware
                                                            // images are still required
                                        hardware_img_put(hgc->dst_x1, hgc->dst_y1, hgc->dst_x2, hgc->dst_y2, hgc->src_img, hgc->dst_img, hgc->src_x1,
                                                         hgc->src_y1, hgc->src_x2, hgc->src_y2, hgc->use_alpha, hgc->smooth);
                                    }
                                }

                                if (hgc->command == HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE) {
                                    if (hgc->dst_img > 0) { // note: rendering to the old
                                                            // default surface is pointless, but
                                                            // renders onto maintained hardware
                                                            // images are still required
                                        hardware_img_tri2d(hgc->dst_x1, hgc->dst_y1, hgc->dst_x2, hgc->dst_y2, hgc->dst_x3, hgc->dst_y3, hgc->src_img,
                                                           hgc->dst_img, hgc->src_x1, hgc->src_y1, hgc->src_x2, hgc->src_y2, hgc->src_x3, hgc->src_y3,
                                                           hgc->use_alpha, hgc->smooth);
                                    }
                                }

                                if (hgc->command == HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE3D) {
                                    if (hgc->dst_img > 0) { // note: rendering to the old
                                                            // default surface is pointless, but
                                                            // renders onto maintained hardware
                                                            // images are still required
                                        hardware_img_tri3d(hgc->dst_x1, hgc->dst_y1, hgc->dst_z1, hgc->dst_x2, hgc->dst_y2, hgc->dst_z2, hgc->dst_x3,
                                                           hgc->dst_y3, hgc->dst_z3, hgc->src_img, hgc->dst_img, hgc->src_x1, hgc->src_y1, hgc->src_x2,
                                                           hgc->src_y2, hgc->src_x3, hgc->src_y3, hgc->use_alpha, hgc->smooth, hgc->cull_mode,
                                                           hgc->depthbuffer_mode);
                                    }
                                }

                                if (hgc->command == HARDWARE_GRAPHICS_COMMAND__CLEAR_DEPTHBUFFER) {
                                    if (hgc->dst_img > 0) { // note: rendering to the old
                                                            // default surface is pointless, but
                                                            // renders onto maintained hardware
                                                            // images are still required
                                        clear_depthbuffer(hgc->dst_img);
                                    }
                                }

                                last_hardware_command_rendered = command;
                                if (next_hardware_command_to_remove == 0)
                                    next_hardware_command_to_remove = command;
                                command = hgc->next_command;
                                hgc->remove = 1;
                            } else {
                                goto found_command_from_current_order;
                            }
                        }
                    found_command_from_current_order:;

                    } // first_hardware_command

                    first_hardware_layer_command = command;
                } else {
                    command = first_hardware_layer_command;
                }

                // process pending hardware puts for this frame's order value
                while (command) {
                    hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, command);
                    if (hgc == NULL) {

                        hardware_graphics_command_struct *hgcx =
                            (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, next_hardware_command_to_remove);
                        gui_alert("Renderer: Command does not exist: command = %i, hgcx->order = %lld, order = %lld", command, hgcx->order, order);
                    }
                    if (hgc->order == order) {
                        if (first_command_prev_order == 0)
                            first_command_prev_order = command;

                        if (hgc->command == HARDWARE_GRAPHICS_COMMAND__FREEIMAGE && rerender_prev_hardware_frame == 0 && first_hardware_layer_rendered == 0) {
                            free_hardware_img(hgc->src_img, 847003);
                        }

                        if (hgc->command == HARDWARE_GRAPHICS_COMMAND__PUTIMAGE) {
                            if (rerender_prev_hardware_frame == 0 || hgc->dst_img <= 0) {
                                if ((hgc->dst_img > 0 && first_hardware_layer_rendered == 0) || hgc->dst_img == dst) {
                                    hardware_img_put(hgc->dst_x1, hgc->dst_y1, hgc->dst_x2, hgc->dst_y2, hgc->src_img, hgc->dst_img, hgc->src_x1, hgc->src_y1,
                                                     hgc->src_x2, hgc->src_y2, hgc->use_alpha, hgc->smooth);
                                }
                            }
                        }

                        if (hgc->command == HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE) {
                            if (rerender_prev_hardware_frame == 0 || hgc->dst_img <= 0) {
                                if ((hgc->dst_img > 0 && first_hardware_layer_rendered == 0) || hgc->dst_img == dst) {
                                    hardware_img_tri2d(hgc->dst_x1, hgc->dst_y1, hgc->dst_x2, hgc->dst_y2, hgc->dst_x3, hgc->dst_y3, hgc->src_img, hgc->dst_img,
                                                       hgc->src_x1, hgc->src_y1, hgc->src_x2, hgc->src_y2, hgc->src_x3, hgc->src_y3, hgc->use_alpha,
                                                       hgc->smooth);
                                }
                            }
                        }

                        if (hgc->command == HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE3D) {
                            if (rerender_prev_hardware_frame == 0 || hgc->dst_img <= 0) {
                                if ((hgc->dst_img > 0 && first_hardware_layer_rendered == 0) || hgc->dst_img == dst) {
                                    hardware_img_tri3d(hgc->dst_x1, hgc->dst_y1, hgc->dst_z1, hgc->dst_x2, hgc->dst_y2, hgc->dst_z2, hgc->dst_x3, hgc->dst_y3,
                                                       hgc->dst_z3, hgc->src_img, hgc->dst_img, hgc->src_x1, hgc->src_y1, hgc->src_x2, hgc->src_y2, hgc->src_x3,
                                                       hgc->src_y3, hgc->use_alpha, hgc->smooth, hgc->cull_mode, hgc->depthbuffer_mode);
                                }
                            }
                        }

                        if (hgc->command == HARDWARE_GRAPHICS_COMMAND__CLEAR_DEPTHBUFFER) {
                            if (rerender_prev_hardware_frame == 0 || hgc->dst_img <= 0) {
                                if ((hgc->dst_img > 0 && first_hardware_layer_rendered == 0) || hgc->dst_img == dst) {
                                    clear_depthbuffer(hgc->dst_img);
                                }
                            }
                        }

                        last_hardware_command_rendered = command;
                        if (next_hardware_command_to_remove == 0)
                            next_hardware_command_to_remove = command; //!!!! should be prev to this command
                        command = hgc->next_command;
                        hgc->remove = 1;
                    } else {
                        goto finished_all_commands_for_current_frame;
                    }
                }
            finished_all_commands_for_current_frame:;

                first_hardware_layer_rendered = 1;

                hardware_buffer_flush();
            } // level==displayorder_hardware||level==displayorder_hardware1

            if (level == 5) {

                if (environment_2d__letterbox) {

                    // create a black texture (if not yet created)
                    static uint32 black_pixel = 0x00000000;
                    static int32 black_texture = 0;
                    if (black_texture == 0) {
                        black_texture = new_hardware_img(1, 1, &black_pixel, NULL);
                    }

                    if (environment_2d__letterbox == 1) {
                        // vertical stripes
                        hardware_img_put(((float)-environment_2d__screen_x1) / environment_2d__screen_x_scale - 1.0f, 0, -1, environment_2d__screen_height - 1,
                                         black_texture, 0, 0, 0, 0, 0, 0, 0);
                        hardware_img_put(environment_2d__screen_width, 0,
                                         (((float)-environment_2d__screen_x1) + (float)environment__window_width - 1.0f) / environment_2d__screen_x_scale +
                                             1.0f,
                                         environment_2d__screen_height - 1, black_texture, 0, 0, 0, 0, 0, 0, 0);
                    } else {
                        // horizontal stripes
                        hardware_img_put(0, ((float)-environment_2d__screen_y1) / environment_2d__screen_y_scale - 1.0f, environment_2d__screen_width - 1, -1,
                                         black_texture, 0, 0, 0, 0, 0, 0, 0);
                        hardware_img_put(0, environment_2d__screen_height, environment_2d__screen_width - 1,
                                         (((float)-environment_2d__screen_y1) + (float)environment__window_height - 1.0f) / environment_2d__screen_y_scale +
                                             1.0f,
                                         black_texture, 0, 0, 0, 0, 0, 0, 0);
                    }
                    hardware_buffer_flush();
                } // letterbox

            } // level==5

        } // level!=0
    } // level loop

    if (requestedKeyboardOverlayImage) {
        int32 src = requestedKeyboardOverlayImage - HARDWARE_IMG_HANDLE_OFFSET;
        hardware_img_struct *src_hardware_img;
        src_hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, src);
        /*
            hardware_img_put(0,0,src_hardware_img->w-1,src_hardware_img->h-1,
            src, 0,
            0,0,src_hardware_img->w-1,src_hardware_img->h-1,
            1,0);
        */
        hardware_img_put(0, 0, environment_2d__screen_width - 1, environment_2d__screen_height - 1, src, 0, 0, 0, src_hardware_img->w - 1,
                         src_hardware_img->h - 1, 1, 0);
        hardware_buffer_flush();
    }

    last_rendered_hardware_display_frame_order = last_hardware_display_frame_order;

    if (suspend_program) { // Otherwise skipped SUB__GL content becomes
                           // "invisible"
        //...
    } else {
        glutSwapBuffers();
    }

    in_GLUT_DISPLAY_REQUEST = 0;

} // GLUT_DISPLAY_REQUEST

void GLUT_MouseButton_Up(int button, int x, int y) {
#    ifdef QB64_GLUT
    int32 i;
    mouse_message_queue_struct *queue = &mouse_message_queue;

    i = queue->last + 1;
    if (i > queue->lastIndex)
        i = 0;
    if (i == queue->current) {
        int32 nextIndex = queue->last + 1;
        if (nextIndex > queue->lastIndex)
            nextIndex = 0;
        queue->current = nextIndex;
    }
    queue->queue[i].x = x;
    queue->queue[i].y = y;
    queue->queue[i].movementx = 0;
    queue->queue[i].movementy = 0;
    queue->queue[i].buttons = queue->queue[queue->last].buttons;
    if (queue->queue[i].buttons & (1 << (button - 1)))
        queue->queue[i].buttons ^= (1 << (button - 1));
    queue->last = i;

    if (device_last) { // core devices required?
        if ((button >= 1) && (button <= 3)) {
            button--;
            static device_struct *d;
            d = &devices[2]; // mouse

            int32 eventIndex = createDeviceEvent(d);
            setDeviceEventButtonValue(d, eventIndex, button, 0);
            commitDeviceEvent(d);

        } // valid range
    } // core devices required

#    endif
}

void GLUT_MouseButton_Down(int button, int x, int y) {
#    ifdef QB64_GLUT

    int32 i;
    mouse_message_queue_struct *queue = &mouse_message_queue;

    i = queue->last + 1;
    if (i > queue->lastIndex)
        i = 0;
    if (i == queue->current) {
        int32 nextIndex = queue->last + 1;
        if (nextIndex > queue->lastIndex)
            nextIndex = 0;
        queue->current = nextIndex;
    }
    queue->queue[i].x = x;
    queue->queue[i].y = y;
    queue->queue[i].movementx = 0;
    queue->queue[i].movementy = 0;
    queue->queue[i].buttons = queue->queue[queue->last].buttons;
    queue->queue[i].buttons |= (1 << (button - 1));
    queue->last = i;

    if (device_last) { // core devices required?
        if ((button >= 1) && (button <= 3)) {
            button--;
            static device_struct *d;
            d = &devices[2]; // mouse

            int32 eventIndex = createDeviceEvent(d);
            setDeviceEventButtonValue(d, eventIndex, button, 1);
            commitDeviceEvent(d);

            // 1-3
        } else {
            // not 1-3
            // mouse wheel?
            if ((button >= 4) && (button <= 5)) {
                static float f;
                if (button == 4)
                    f = -1;
                else
                    f = 1;
                static device_struct *d;
                d = &devices[2]; // mouse

                int32 eventIndex = createDeviceEvent(d);
                setDeviceEventWheelValue(d, eventIndex, 2, f);
                commitDeviceEvent(d);

                eventIndex = createDeviceEvent(d);
                setDeviceEventWheelValue(d, eventIndex, 2, 0);
                commitDeviceEvent(d);

            } // 4-5
        } // not 1-3
    } // core devices required
#    endif
}

void GLUT_MOUSE_FUNC(int glut_button, int state, int x, int y) {
#    ifdef QB64_GLUT
    if (state == GLUT_DOWN)
        GLUT_MouseButton_Down(glut_button + 1, x, y);
    if (state == GLUT_UP)
        GLUT_MouseButton_Up(glut_button + 1, x, y);
#    endif
}

void GLUT_MOTION_FUNC(int x, int y) {

    int32 i, last_i;
    int32 xrel, yrel;

    // This is used to save the last mouse position which is then paired with the mouse wheel event on macOS
    MacMouse_UpdatePosition(x, y);

    mouse_message_queue_struct *queue = &mouse_message_queue;

    // message #1
    last_i = queue->last;
    i = queue->last + 1;
    if (i > queue->lastIndex)
        i = 0; // wrap around
    if (i == queue->current) {
        int32 nextIndex = queue->last + 1;
        if (nextIndex > queue->lastIndex)
            nextIndex = 0;
        queue->current = nextIndex;
    }

#    if defined(QB64_WINDOWS) || defined(QB64_MACOSX)
    // Windows calculates relative movement by intercepting WM_INPUT events
    // macOS uses the Quartz Event Services to get relative movements
    xrel = 0;
    yrel = 0;
#    else
    // TODO: This needs to be correctly implemented on Linux
    xrel = x - queue->queue[queue->last].x;
    yrel = y - queue->queue[queue->last].y;
#    endif

    queue->queue[i].x = x;
    queue->queue[i].y = y;
    queue->queue[i].movementx = xrel;
    queue->queue[i].movementy = yrel;
    queue->queue[i].buttons = queue->queue[last_i].buttons;
    queue->last = i;

    // message #2 (clears movement values to avoid confusion)
    last_i = queue->last;
    i = queue->last + 1;
    if (i > queue->lastIndex)
        i = 0;
    if (i == queue->current) {
        int32 nextIndex = queue->last + 1;
        if (nextIndex > queue->lastIndex)
            nextIndex = 0;
        queue->current = nextIndex;
    }
    queue->queue[i].x = x;
    queue->queue[i].y = y;
    queue->queue[i].movementx = 0;
    queue->queue[i].movementy = 0;
    queue->queue[i].buttons = queue->queue[last_i].buttons;
    queue->last = i;

    if (device_last) { // core devices required?
        if (!device_mouse_relative) {
            static device_struct *d;
            d = &devices[2]; // mouse

            int32 eventIndex = createDeviceEvent(d);
            static float fx, fy;
            static int32 z;
            fx = x;
            fx -= x_offset;
            z = x_monitor - x_offset * 2;
            if (fx < 0)
                fx = 0;
            if (fx >= z)
                fx = z - 1;
            fx = fx / (float)(z - 1); // 0 to 1
            fx *= 2.0;                // 0 to 2
            fx -= 1.0;                //-1 to 1
            fy = y;
            fy -= y_offset;
            z = y_monitor - y_offset * 2;
            if (fy < 0)
                fy = 0;
            if (fy >= z)
                fy = z - 1;
            fy = fy / (float)(z - 1); // 0 to 1
            fy *= 2.0;                // 0 to 2
            fy -= 1.0;                //-1 to 1
            setDeviceEventAxisValue(d, eventIndex, 0, fx);
            setDeviceEventAxisValue(d, eventIndex, 1, fy);
            commitDeviceEvent(d);

        } else {
            static device_struct *d;
            d = &devices[2]; // mouse

            int32 eventIndex = createDeviceEvent(d);
            static float fx, fy;
            static int32 z;
            fx = xrel;
            fy = yrel;
            setDeviceEventWheelValue(d, eventIndex, 0, fx);
            setDeviceEventWheelValue(d, eventIndex, 1, fy);
            commitDeviceEvent(d);

            eventIndex = createDeviceEvent(d);
            fx = 0;
            fy = 0;
            setDeviceEventWheelValue(d, eventIndex, 0, fx);
            setDeviceEventWheelValue(d, eventIndex, 1, fy);
            commitDeviceEvent(d);
        }
    } // core devices required
}

void GLUT_PASSIVEMOTION_FUNC(int x, int y) {
    GLUT_MOTION_FUNC(x, y);
}

void GLUT_MOUSEWHEEL_FUNC(int wheel, int direction, int x, int y) {
#    ifdef QB64_GLUT
    // Note: freeglut specific, limited documentation existed so the following
    // research was done:
    //  qbs_print(qbs_str(wheel),NULL); <-- was always 0 [could 1 indicate
    //  horizontal wheel?] qbs_print(qbs_str(direction),NULL); <-- 1(up) or
    //  -1(down) qbs_print(qbs_str(x),NULL); <--mouse x,y co-ordinates
    //  qbs_print(qbs_str(y),1);    <
    if (direction > 0) {
        GLUT_MouseButton_Down(4, x, y);
        GLUT_MouseButton_Up(4, x, y);
    }
    if (direction < 0) {
        GLUT_MouseButton_Down(5, x, y);
        GLUT_MouseButton_Up(5, x, y);
    }
#    endif
}

#endif

void sub__title(qbs *title) {
    if (is_error_pending())
        return;
    static qbs *cz = NULL;
    if (!cz) {
        cz = qbs_new(1, 0);
        cz->chr[0] = 0;
    }
    static qbs *str = NULL;
    if (!str)
        str = qbs_new(0, 0);
    qbs_set(str, qbs_add(title, cz));

    uint8 *buf, *old_buf;
    buf = (uint8 *)malloc(str->len);
    memcpy(buf, str->chr, str->len);
    old_buf = window_title;
    window_title = buf;
    if (old_buf)
        free(old_buf);

    OPTIONAL_GLUT();

    libqb_glut_set_window_title((char *)window_title);
} // title

void sub__echo(qbs *message) {
    if (is_error_pending())
        return;

    int32 prevDest = func__dest();
    sub__dest(func__console());

    makefit(message);
    qbs_print(message, 0);
    qbs_print(nothingstring, 1);

    sub__dest(prevDest);

} // echo

qbs *func__readfile(qbs *filespec) {
    FILE *file;
    int len; // file handle; file length;
    qbs *namez;
    qbs *cont; // 0-term file name; file contents;

    namez = qbs_add(filespec, func_chr(0)); // add terminator
    filepath_fix_directory(namez);          // fix separators
    file = fopen((const char *)namez->chr, "rb");

    if (file) {
        fseek(file, 0, SEEK_END); // end pos
        len = ftell(file);        //   = file length
        rewind(file);             // rewind to start
        cont = qbs_new(len, 1);   // get new string for file contents
        fread(cont->chr, 1, len, file);
        if (ferror(file)) {
            error(QB_ERROR_PATH_FILE_ACCESS_ERROR); // something went wrong
            cont = qbs_new_txt("");                 // return empty on error
        }
        fclose(file);
        return cont;
    } else {
        error(QB_ERROR_FILE_NOT_FOUND); // most common when trying to read
        return qbs_new_txt("");         // return empty on error
    }
}

void sub__writefile(qbs *filespec, qbs *contents) {
    FILE *file; // file handle;
    qbs *namez; // 0-term file name;

    namez = qbs_add(filespec, func_chr(0)); // add terminator
    filepath_fix_directory(namez);          // fix separators
    file = fopen((const char *)namez->chr, "wb");

    if (file) {
        fwrite(contents->chr, 1, contents->len, file);
        if (ferror(file))
            error(QB_ERROR_PATH_FILE_ACCESS_ERROR); // something went wrong
        fclose(file);
    } else {
        error(QB_ERROR_PATH_NOT_FOUND); // most common when making a new file
    }
}

// sub__filedrop, func__filedrop, sub__finishdrop, func__totaldroppedfiles, func__droppedfile moved to window.cpp

// sub__resize, func__resize, func__resizewidth, func__resizeheight,
// func__scaledwidth, func__scaledheight moved to screen.cpp

extern void set_dynamic_info();

#ifdef QB64_WINDOWS
static bool isValidCygwinPipe(int fd) {
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (GetFileType(h) != FILE_TYPE_PIPE) {
        return false;
    }

    size_t size = 4096;
    std::vector<char> nameinfoBuf(sizeof(FILE_NAME_INFO) + sizeof(WCHAR) * size);
    FILE_NAME_INFO *nameinfo = reinterpret_cast<FILE_NAME_INFO *>(nameinfoBuf.data());

    if (GetFileInformationByHandleEx(h, FileNameInfo, nameinfo, size)) {
        nameinfo->FileName[nameinfo->FileNameLength / sizeof(WCHAR)] = L'\0';

        // When a valid pipe is found, disable buffering so that results are seen immediately
        if (wcsncmp(nameinfo->FileName, L"\\msys-", 6) == 0) {
            setbuf(stdout, NULL);
            setbuf(stderr, NULL);
            return true;
        } else if (wcsncmp(nameinfo->FileName, L"\\cygwin-", 8) == 0) {
            setbuf(stdout, NULL);
            setbuf(stderr, NULL);
            return true;
        }
    }

    return false;
}
#endif

int main(int argc, char *argv[]) {
    clock_init();

#ifdef QB64_WINDOWS
    // `isValidCygwinPipe()` checks for Cygwin-based stdout, which is good
    // enough to use directly. Otherwise we try to connect to the console we
    // were started from (if there is one).
    //
    // If we're a console program and `AttachConsole()` did not work then we
    // will end up spawning our own console.
    if (!isValidCygwinPipe(STDOUT_FILENO) && AttachConsole(ATTACH_PARENT_PROCESS)) {
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
    }
#endif

    libqb_log_init();
    libqb_log_info("Program starting.");

#if defined(QB64_LINUX) && defined(X11)
    XInitThreads();
#endif

    static int32 i, i2, i3, i4;
    static uint8 c, c2, c3, c4;
    static int32 x, x2, x3, x4;
    static int32 y, y2, y3, y4;
    static int32 z, z2, z3, z4;
    static float f, f2, f3, f4;
    static uint8 *cp, *cp2, *cp3, *cp4;

    /********** Render State **********/
    render_state.dest = NULL;
    render_state.source = NULL;
    render_state.dest_handle = INVALID_HARDWARE_HANDLE;
    render_state.source_handle = INVALID_HARDWARE_HANDLE;
    render_state.view_mode = VIEW_MODE__UNKNOWN;
    render_state.use_alpha = ALPHA_MODE__UNKNOWN;
    render_state.depthbuffer_mode = DEPTHBUFFER_MODE__UNKNOWN;
    render_state.cull_mode = CULL_MODE__UNKNOWN;
    /********** Render State **********/

    for (i = 0; i <= 2; i++) {
        display_frame[i].state = DISPLAY_FRAME_STATE__EMPTY;
        display_frame[i].order = 0;
        display_frame[i].bgra = NULL;
        display_frame[i].w = 0;
        display_frame[i].h = 0;
        display_frame[i].bytes = 0;
    }

    set_dynamic_info();
    if (ScreenResize) {
        resize_snapback = 0;
    }
    if (ScreenResizeScale) {
        resize_auto = ScreenResizeScale;
    }

    // setup lists
    special_handles = list_new(sizeof(special_handle_struct));
    stream_handles = list_new(sizeof(stream_struct));
    networking_init();

    hardware_img_handles = list_new_threadsafe(sizeof(hardware_img_struct));
    hardware_graphics_command_handles = list_new(sizeof(hardware_graphics_command_struct));

    // setup default mouse message queue
    mouse_message_queue_struct *queue = &mouse_message_queue;
    queue->lastIndex = 65535;
    queue->queue = (mouse_message *)calloc(1, sizeof(mouse_message) * (queue->lastIndex + 1));

    if (screen_hide_startup)
        screen_hide = 1;

#ifdef QB64_WINDOWS
    if (console) {
        LPDWORD plist = (LPDWORD)malloc(1000);
        if (GetConsoleProcessList(plist, 256) == 1) {
            console_child = 1; // only this program is using the console
        }
    }
#endif

    onkey[1].keycode = 59 << 8; // F1-F10
    onkey[2].keycode = 60 << 8;
    onkey[3].keycode = 61 << 8;
    onkey[4].keycode = 62 << 8;
    onkey[5].keycode = 63 << 8;
    onkey[6].keycode = 64 << 8;
    onkey[7].keycode = 65 << 8;
    onkey[8].keycode = 66 << 8;
    onkey[9].keycode = 67 << 8;
    onkey[10].keycode = 68 << 8;
    onkey[11].keycode = 72 << 8; // up,left,right,down
    onkey[11].keycode_alternate = VK + QBVK_KP8;
    onkey[12].keycode = 75 << 8;
    onkey[12].keycode_alternate = VK + QBVK_KP4;
    onkey[13].keycode = 77 << 8;
    onkey[13].keycode_alternate = VK + QBVK_KP6;
    onkey[14].keycode = 80 << 8;
    onkey[14].keycode_alternate = VK + QBVK_KP2;
    onkey[30].keycode = 133 << 8; // F11,F12
    onkey[31].keycode = 134 << 8;

    ontimer[0].allocated = 1;
    ontimer[0].id = 0;
    ontimer[0].state = 0;
    ontimer[0].active = 0;

    fontwidth[8] = 8;
    fontwidth[14] = 8;
    fontwidth[16] = 8;
    fontheight[8] = 8;
    fontheight[14] = 14;
    fontheight[16] = 16;
    fontflags[8] = FONT_LOAD_MONOSPACE;
    fontflags[14] = FONT_LOAD_MONOSPACE;
    fontflags[16] = FONT_LOAD_MONOSPACE; // monospace flag
    fontwidth[8 + 1] = 8 * 2;
    fontwidth[14 + 1] = 8 * 2;
    fontwidth[16 + 1] = 8 * 2;
    fontheight[8 + 1] = 8;
    fontheight[14 + 1] = 14;
    fontheight[16 + 1] = 16;
    fontflags[8 + 1] = FONT_LOAD_MONOSPACE;
    fontflags[14 + 1] = FONT_LOAD_MONOSPACE;
    fontflags[16 + 1] = FONT_LOAD_MONOSPACE; // monospace flag

    memset(img, 0, IMG_BUFFERSIZE * sizeof(img_struct));
    x = newimg(); // reserve index 0
    img[x].valid = 0;
    x = newimg(); // reserve index 1
    img[x].valid = 0;

    memset(&cpu, 0, sizeof(cpu_struct));

    // uint8 *asmcodep=(uint8*)&asmcode[0];
    // memcpy(&cmem[0],asmcodep,sizeof(asmcode));
    reg8[0] = &cpu.al;
    reg8[1] = &cpu.cl;
    reg8[2] = &cpu.dl;
    reg8[3] = &cpu.bl;
    reg8[4] = &cpu.ah;
    reg8[5] = &cpu.ch;
    reg8[6] = &cpu.dh;
    reg8[7] = &cpu.bh;

    reg16[0] = &cpu.ax;
    reg16[1] = &cpu.cx;
    reg16[2] = &cpu.dx;
    reg16[3] = &cpu.bx;
    reg16[4] = &cpu.sp;
    reg16[5] = &cpu.bp;
    reg16[6] = &cpu.si;
    reg16[7] = &cpu.di;

    reg32[0] = &cpu.eax;
    reg32[1] = &cpu.ecx;
    reg32[2] = &cpu.edx;
    reg32[3] = &cpu.ebx;
    reg32[4] = &cpu.esp;
    reg32[5] = &cpu.ebp;
    reg32[6] = &cpu.esi;
    reg32[7] = &cpu.edi;

    segreg[0] = &cpu.es;
    segreg[1] = &cpu.cs;
    segreg[2] = &cpu.ss;
    segreg[3] = &cpu.ds;
    segreg[4] = &cpu.fs;
    segreg[5] = &cpu.gs;

    for (i = 0; i < 32; i++)
        sub_file_print_spaces[i] = 32;

    port60h_event[0] = 128 + 1; // simulate release of ESC

    mem_static_size = 1048576; // 1MEG
    mem_static = (uint8 *)malloc(mem_static_size);
    mem_static_pointer = mem_static;
    mem_static_limit = mem_static + mem_static_size;

    memset(&cmem[0], 0, sizeof(cmem));

    memset(&keyon[0], 0, sizeof(keyon));

    dblock = (ptrszint)&cmem + 1280; // 0:500h

    // define "nothing"
    cmem_sp -= 8;
    nothingvalue = (uint64 *)(dblock + cmem_sp);
    *nothingvalue = 0;
    nothingstring = qbs_new_cmem(0, 0);
    singlespace = qbs_new_cmem(1, 0);
    singlespace->chr[0] = 32;

    // Store _CWD$ for recall using _STARTDIR$
    FS_SaveStartDirectory();

// switch to directory of this EXE file
// http://stackoverflow.com/questions/1023306/finding-current-executables-path-without-proc-self-exe
#if defined(QB64_WINDOWS) && !defined(QB64_MICROSOFT)
    static char *exepath = (char *)malloc(65536);
    GetModuleFileNameA(NULL, exepath, 65536);
    i = strlen(exepath);
    for (i2 = i - 1; i2 >= 0; i2--) {
        x = exepath[i2];
        if ((x == 92) || (x == 47) || (x == 58)) {
            if (x == 58)
                exepath[i2 + 1] = 0;
            else
                exepath[i2] = 0;
            break;
        }
    }
    chdir(exepath);
#elif defined(QB64_LINUX)
    {
        char pathbuf[65536];
        memset(pathbuf, 0, sizeof(pathbuf));
        readlink("/proc/self/exe", pathbuf, 65535);
        chdir(dirname(pathbuf));
    }
#elif defined(QB64_MACOSX)
    {
        char pathbuf[65536];
        uint32_t pathbufsize = sizeof(pathbuf);
        _NSGetExecutablePath(pathbuf, &pathbufsize);
        chdir(dirname(pathbuf));
    }
#endif

    unknown_opcode_mess = qbs_new(0, 0);
    qbs_set(unknown_opcode_mess, qbs_new_txt_len("Unknown Opcode (  )\0", 20));

    command_initialize(argc, argv);

#ifdef QB64_WINDOWS
    // for caps lock, use the state of the lock (=1)
    // for held keys check against (=-127)
    if (GetKeyState(VK_SCROLL) & 1)
        keyheld_add(QBK + QBK_SCROLL_LOCK_MODE);
    if (GetKeyState(VK_SCROLL) < 0) {
        bindkey = QBVK_SCROLLOCK;
        keydown_vk(VK + QBVK_SCROLLOCK);
    }
    if (GetKeyState(VK_LSHIFT) < 0) {
        bindkey = QBVK_LSHIFT;
        keydown_vk(VK + QBVK_LSHIFT);
    }
    if (GetKeyState(VK_RSHIFT) < 0) {
        bindkey = QBVK_RSHIFT;
        keydown_vk(VK + QBVK_RSHIFT);
    }
    if (GetKeyState(VK_LCONTROL) < 0) {
        bindkey = QBVK_LCTRL;
        keydown_vk(VK + QBVK_LCTRL);
    }
    if (GetKeyState(VK_RCONTROL) < 0) {
        bindkey = QBVK_RCTRL;
        keydown_vk(VK + QBVK_RCTRL);
    }
    if (GetKeyState(VK_LMENU) < 0) {
        bindkey = QBVK_LALT;
        keydown_vk(VK + QBVK_LALT);
    }
    if (GetKeyState(VK_RMENU) < 0) {
        bindkey = QBVK_RALT;
        keydown_vk(VK + QBVK_RALT);
    }
    if (GetKeyState(VK_CAPITAL) & 1) {
        bindkey = QBVK_CAPSLOCK;
        keydown_vk(VK + QBVK_CAPSLOCK);
    }
    if (GetKeyState(VK_NUMLOCK) & 1) {
        bindkey = QBVK_NUMLOCK;
        keydown_vk(VK + QBVK_NUMLOCK);
    }
    update_shift_state();
    keyhit_next = keyhit_nextfree; // skip hitkey events generated by above code
#endif

    // init fake keyb. cyclic buffer
    cmem[0x41a] = 30;
    cmem[0x41b] = 0; // head
    cmem[0x41c] = 30;
    cmem[0x41d] = 0; // tail

    std::ifstream fh;

    // default 256 color palette
    memcpy(&palette_256, &file_qb64_pal[0], file_qb64_pal_len);
    for (i = 0; i < 256; i++)
        palette_256[i] |= 0xFF000000;

    // default EGA(64) color palette
    memcpy(&palette_64, &file_qb64ega_pal[0], file_qb64ega_pal_len);
    for (i = 0; i < 64; i++)
        palette_64[i] |= 0xFF000000;

    // manually set screen 10 palette
    pal_mode10[0][0] = 0;
    pal_mode10[0][1] = 0;
    pal_mode10[0][2] = 0;
    pal_mode10[0][3] = 0x808080;
    pal_mode10[0][4] = 0x808080;
    pal_mode10[0][5] = 0x808080;
    pal_mode10[0][6] = 0xFFFFFF;
    pal_mode10[0][7] = 0xFFFFFF;
    pal_mode10[0][8] = 0xFFFFFF;
    pal_mode10[1][0] = 0;
    pal_mode10[1][1] = 0x808080;
    pal_mode10[1][2] = 0xFFFFFF;

    pal_mode10[1][3] = 0;
    pal_mode10[1][4] = 0x808080;
    pal_mode10[1][5] = 0xFFFFFF;
    pal_mode10[1][6] = 0;
    pal_mode10[1][7] = 0x808080;
    pal_mode10[1][8] = 0xFFFFFF;

    // 8x8 character set
    memcpy(&charset8x8, &file_charset8_raw[0], file_charset8_raw_len);

    // 8x16 character set
    memcpy(&charset8x16, &file_chrset16_raw[0], file_chrset16_raw_len);

    qbg_screen(0, NULL, NULL, NULL, NULL, 1);
    width8050switch = 1; // reaffirm switch reset by above command

    if (console) {
        console_image = func__newimage(80, 25, 0, 0);
        i = -console_image;
        img[i].console = 1;
    }

    // setup default _DEVICE(s)
    i = 0;

    // keyboard
    i++;
    devices[i].type = DEVICETYPE_KEYBOARD;
    devices[i].name = strdup("[KEYBOARD][BUTTON]"); // TODO: when re-writing game_controller.cpp use std::string
    devices[i].lastbutton = 512;
    devices[i].description = "Keyboard";
    setupDevice(&devices[i]);

    // mouse
    i++;
    devices[i].type = DEVICETYPE_MOUSE;
    devices[i].name = strdup("[MOUSE][BUTTON][AXIS][WHEEL]"); // TODO: when re-writing game_controller.cpp use std::string
    devices[i].lastbutton = 3;
    devices[i].lastaxis = 2;
    devices[i].lastwheel = 3;
    devices[i].description = "Mouse";
    setupDevice(&devices[i]);

    device_last = i;

#ifdef DEPENDENCY_DEVICEINPUT
    QB64_GAMEPAD_INIT();
#endif

    libqb_http_init();

    libqb_glut_presetup(argc, argv);

    struct libqb_thread *qbmain = libqb_thread_new();
    libqb_thread_start(qbmain, QBMAIN, NULL);

    struct libqb_thread *timer = libqb_thread_new();
    libqb_thread_start(timer, TIMERTHREAD, NULL);

    lock_display_required = 1;

    libqb_start_main_thread(argc, argv);

    return 0; // Should never get here
}

// ###################### Main Loop ####################
void MAIN_LOOP(void *unused) {

    int32 update = 0; // 0=update input,1=update display

main_loop:

#ifdef DEPENDENCY_DEVICEINPUT
#    ifndef QB64_MACOSX
    QB64_GAMEPAD_POLL();
#    endif
#endif

    if (lock_mainloop == 1) {
        lock_mainloop = 2;
        while (lock_mainloop == 2)
            Sleep(1);
    }

    if (exit_value) {
        if (!exit_blocked)
            goto end_program;
    }

    // update timer bytes in cmem
    uint64_t cmem_ticks;

    cmem_ticks = (uint64_t)(func_timer(0.001, 1) * 1000);

    // make timer value loop after midnight
    // note: there are 86400000 milliseconds in 24hrs(1 day)
    cmem_ticks %= 86400000;
    cmem_ticks = ((double)cmem_ticks) * 0.0182;
    cmem[0x46c] = cmem_ticks & 255;
    cmem[0x46d] = (cmem_ticks >> 8) & 255;
    cmem[0x46e] = (cmem_ticks >> 16) & 255;
    // note: a discrepancy exists of unknown cause

    if (shell_call_in_progress) {
        if (shell_call_in_progress != -1) {
            shell_call_in_progress = -1;
            goto update_display_only;
        }
        Sleep(64);
        goto main_loop;
    }

    Sleep(15);
    vertical_retrace_happened = 1;
    vertical_retrace_in_progress = 1;
    Sleep(1);

    if (close_program) {
        lock_mainloop = 2; // report mainloop as locked so that any process waiting for a successful lock can continue
        goto end_program;
    }

    snd_update();

    update ^= 1; // toggle update

    if (!lprint) { // not currently performing an LPRINT operation
        lprint_locked = 1;
        if (lprint_buffered) {
            if (std::fabs(func_timer(0.001, 1) - lprint_last) >= 10.0) { // 10 seconds elapsed since last LPRINT operation
                sub__printimage(lprint_image);
                lprint_buffered = 0;
                static int32 old_dest;
                old_dest = func__dest();
                sub__dest(lprint_image);
                sub_cls(NULL, 15, 2);
                sub__dest(old_dest);
            }
        }
        lprint_locked = 0;
    }

    // note: this mainloop loops with breaks of 16ms, display is toggled every 2nd loop
    // update display?
    if (update == 1) {
    update_display_only:
        if (autodisplay)
            display(); // occurs every 32ms or 31.25 times per second
        frame++;       //~32 fps
    } // update==1

    vertical_retrace_in_progress = 0;

    if (shell_call_in_progress)
        goto main_loop;

    if (update == 0) {

        static int32 scancode;
        static const uint8 QBVK_2_scancode[] = {
            0,  0,  0,  0,  0,   0,   0,  0,  14, 15, 0,  0,  0,  28, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  0,  0,  0,   0,  57, 0,  0,  0,
            0,  0,  0,  40, 0,   0,   0,  0,  51, 12, 52, 53, 11, 2,  3,  4,  5,  6,  7,  8,  9,  10, 0,  39, 0,  13, 0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  26, 43, 27, 0,  0,  41, 30, 48, 46, 32, 18, 33,  34, 35, 23, 36, 37,
            38, 50, 49, 24, 25,  16,  19, 31, 20, 22, 47, 17, 45, 21, 44, 0,  0,  0,  0,  83, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  82,  79,  80, 81, 75, 76, 77, 71, 72, 73, 83, 53, 55, 74, 78, 28, 0,  72, 80, 77, 75, 82, 71, 79, 73, 81, 59,  60, 61, 62, 63, 64,
            65, 66, 67, 68, 133, 134, 0,  0,  0,  0,  0,  0,  69, 58, 70, 54, 42, 29, 29, 56, 56, 0,  0,  91, 92, 0,  0,  0,  0,  55, 197, 93, 0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  0,  0,  0,  0,
            0,  0,  0,  0,  0,   0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

#ifdef QB64_WINDOWS
        // manage important external keyboard lock/state changes
        if ((GetKeyState(VK_SCROLL) & 1) != keyheld(QBK + QBK_SCROLL_LOCK_MODE)) {
            if (keyheld(QBK + QBK_SCROLL_LOCK_MODE)) {
                keyheld_remove(QBK + QBK_SCROLL_LOCK_MODE);
            } else {
                keyheld_add(QBK + QBK_SCROLL_LOCK_MODE);
            }
            update_shift_state();
        }
        if ((GetKeyState(VK_CAPITAL) & 1) != keyheld(VK + QBVK_CAPSLOCK)) {
            if (keyheld(VK + QBVK_CAPSLOCK)) {
                bindkey = QBVK_CAPSLOCK;
                keyup_vk(VK + QBVK_CAPSLOCK);
            } else {
                bindkey = QBVK_CAPSLOCK;
                keydown_vk(VK + QBVK_CAPSLOCK);
            }
            update_shift_state();
        }
        if ((GetKeyState(VK_NUMLOCK) & 1) != keyheld(VK + QBVK_NUMLOCK)) {
            if (keyheld(VK + QBVK_NUMLOCK)) {
                bindkey = QBVK_NUMLOCK;
                keyup_vk(VK + QBVK_NUMLOCK);
            } else {
                bindkey = QBVK_NUMLOCK;
                keydown_vk(VK + QBVK_NUMLOCK);
            }
            update_shift_state();
        }
#endif

        if (shell_call_in_progress)
            goto main_loop;

    } // update==0

    goto main_loop;

end_program:
    stop_program = 1;
    qbevent = 1;
    while (exit_ok != 3)
        Sleep(16);

    if (lprint_buffered) {
        sub__printimage(lprint_image); // print any pending content
    }

    // close all open files
    sub_close(NULL, 0);

    libqb_http_stop();

// shutdown device interface
#ifdef DEPENDENCY_DEVICEINPUT
    QB64_GAMEPAD_SHUTDOWN();
#endif

    libqb_exit(exit_code);
}

// used to preserve the previous frame's content for comparison/reuse purposes
uint8 *pixeldata = (uint8 *)malloc(1);
int32 pixeldatasize = 1;
uint32 paldata[256];

// note: temporarily swapping a source palette is far more efficient than converting the resulting image pixels
void swap_paldata_BGRA_with_RGBA() {
    static uint32 col;
    static uint32 *pos;
    pos = (uint32 *)&paldata[0];
    static int32 pixels;
    pixels = 256;
    while (pixels--) {
        col = *pos;
        *pos++ = (col & 0xFF00FF00) | ((col & 0xFF0000) >> 16) | ((col & 0x0000FF) << 16);
    }
}

// display updates the visual page onto the visible window/monitor
void display() {

    // general variables
    static int32 i, i2, i3, i4;
    static uint8 c, c2, c3, c4;
    static int32 x, x2, x3, x4;
    static int32 y, y2, y3, y4;
    static int32 z, z2, z3, z4;
    static float f, f2, f3, f4;
    static uint8 *cp, *cp2, *cp3, *cp4;
    uint32 *pixel;

    static uint8 BGRA_to_RGBA; // set to 1 to invert the output to RGBA
    BGRA_to_RGBA = 0;          // default is 0 but 1 is fun

    if (lock_display == 1) {
        lock_display = 2;
        Sleep(0);
    }

    if (screen_hide) {
        display_called = 1;
        return;
    }

    if (lock_display == 0) {

        // Identify which display_frame to build
        static int32 frame_i;
        frame_i = -1;
        // use LOWEST order ready frame
        static int64 lowest_order;
        lowest_order = display_frame_order_next;
        for (i = 0; i <= 2; i++) {
            if (display_frame[i].order < lowest_order && display_frame[i].state == DISPLAY_FRAME_STATE__READY) {
                lowest_order = display_frame[i].order;
                frame_i = i;
            }
        }
        // or preferably an unused frame if possible (note: this check happens 2nd for threading reasons)
        for (i = 0; i <= 2; i++) {
            if (display_frame[i].state == DISPLAY_FRAME_STATE__EMPTY) {
                frame_i = i;
            }
        }
        if (frame_i == -1) {
            gui_alert("Software frame buffer: Failed to find available frame");
            return;
        }
        display_frame[frame_i].state = DISPLAY_FRAME_STATE__BUILDING;
        display_frame[frame_i].order = display_frame_order_next++;

        // validate display_page
        if (!display_page)
            goto display_page_invalid;

        // check what is possible in full screen
        x = display_page->width;
        y = display_page->height;

        if (display_page->compatible_mode == 0) {
            x = display_page->width * fontwidth[display_page->font];
            y = display_page->height * fontheight[display_page->font];
        }

        // check for y-stretch flag?
        if (x <= 512 && y <= 384) {
            x *= 2;
            y *= 2;
        }

        static int32 mode_square, mode_stretch;

        // find best fullscreen mode(s) (eg. square/"1:1", stretched)
        mode_square = -1;
        mode_stretch = -1;

        x = display_page->width;
        y = display_page->height;
        if (display_page->compatible_mode == 0) {
            x = display_page->width * fontwidth[display_page->font];
            y = display_page->height * fontheight[display_page->font];
        }
        x_monitor = x;
        y_monitor = y;

        z = 0; //?

        conversion_required = 0;
        pixel = display_surface_offset; //<-will be made obsolete

        if (!display_page->compatible_mode) { // text

            static int32 show_flashing_last = 0;
            static int32 show_cursor_last = 0;
            static int32 check_last;
            static uint8 *cp, *cp2, *cp_last;
            static uint32 *lp;
            static int32 cx, cy;
            static int32 cx_last = -1, cy_last = -1;
            static int32 show_cursor;
            static int32 show_flashing;
            static uint8 chr, col, chr_last, col_last;
            static int32 qbg_y_offset;

            static int32 f, f_pitch, f_width, f_height; // font info
            f = display_page->font;
            f_width = fontwidth[f];
            f_height = fontheight[f];

            static int32 content_changed;

            check_last = screen_last_valid; // If set, modify the previous pixelbuffer's contents
            content_changed = 0;

            // Realloc pixel-buffer if necessary
            i = display_page->width * display_page->height * 2;
            if (screen_last_size != i) {
                free(screen_last);
                screen_last = (uint8 *)malloc(i);
                screen_last_size = i;
                check_last = 0;
            }

            if (displayorder_screen == 0 && check_last == 1) {
                // a valid frame of the correct dimensions exists and we are not required to display software content
                goto no_new_frame;
            }

            // Check/Prepare palette-buffer
            if (!check_last) {
                // set pal_last (no prev pal was available to compare to)
                memcpy(&paldata, display_page->pal, 256 * 4);
            } else {
                // if palette has changed, update paldata and draw all characters
                if (memcmp(&paldata[0], display_page->pal, 256 * 4)) {
                    // Different palette
                    memcpy(&paldata[0], display_page->pal, 256 * 4);
                    check_last = 0;
                }
            }

            // Check/Prepare content
            if (check_last) {
                // i=display_frame_end;
                if (memcmp(screen_last, display_page->offset, screen_last_size)) {
                    // Different content
                    content_changed = 1;
                }
            }

            // Note: frame is a global variable incremented ~32 times per second [2013]
            if (frame & 8)
                show_cursor = 1;
            else
                show_cursor = 0; //[2013]halved cursor blink rate from 8 changes p/sec -> 4 changes p/sec
            if (frame & 8)
                show_flashing = 1;
            else
                show_flashing = 0;

            // calculate cursor position (base 0)
            cx = display_page->cursor_x - 1;
            cy = display_page->cursor_y - 1;
            if (display_page->holding_cursor == 2) { // special case
                if (cy < (display_page->height - 1)) {
                    cy++;
                    cx = 0;
                }
            }

            if (check_last) {
                if (show_flashing != show_flashing_last)
                    content_changed = 1;
                if (show_cursor != show_cursor_last)
                    content_changed = 1;
                if ((cx != cx_last) || (cy != cy_last))
                    content_changed = 1;
            }

            if (!check_last)
                content_changed = 1;

            if (!content_changed) {
                // No content has changed, so skip the generation & display of this frame
                goto no_new_frame;
            }

            static int64 last_frame_i = 0;

            // ################################ Setup new frame ################################
            {
                static int32 new_size_bytes;
                new_size_bytes = x_monitor * y_monitor * 4;
                if (new_size_bytes > display_frame[frame_i].bytes) {
                    free(display_frame[frame_i].bgra);
                    display_frame[frame_i].bgra = (uint32 *)malloc(new_size_bytes);
                    display_frame[frame_i].bytes = new_size_bytes;
                }
                display_frame[frame_i].w = x_monitor;
                display_frame[frame_i].h = y_monitor;
            }

            display_surface_offset = display_frame[frame_i].bgra;

            // If a compare & update changes method will be used copy the previous content to the new buffer

            if (check_last) {
                // find the most recently published page to compare with
                //(the most recent READY or DISPLAYING page)
                static int64 highest_order;
                highest_order = 0;
                i2 = -1;
                for (i3 = 0; i3 <= 2; i3++) {
                    if ((display_frame[i3].state == DISPLAY_FRAME_STATE__DISPLAYING || display_frame[i3].state == DISPLAY_FRAME_STATE__READY) &&
                        display_frame[i3].order > highest_order) {
                        highest_order = display_frame[i3].order;
                        i2 = i3;
                    }
                }
                if (i2 != -1) {
                    memcpy(display_frame[frame_i].bgra, display_frame[i2].bgra, display_frame[frame_i].w * display_frame[frame_i].h * 4);
                } else {
                    gui_alert("Text Screen Update: Failed to locate previous frame's data for comparison");
                    check_last = 0; // never occurs, safe-guard only
                }
            }

            qbg_y_offset = 0;          // the screen base offset
            cp = display_page->offset; // read from
            cp_last = screen_last;     // written to for future comparisons

            if (BGRA_to_RGBA)
                swap_paldata_BGRA_with_RGBA();

            // outer loop
            y2 = 0;
            for (y = 0; y < display_page->height; y++) {
                x2 = 0;
                for (x = 0; x < display_page->width; x++) {

                    chr = *cp;
                    cp++;
                    col = *cp;
                    cp++;

                    // can be skipped?
                    chr_last = *cp_last;
                    cp_last++;
                    col_last = *cp_last;
                    cp_last++;

                    if (check_last) {
                        if (chr == chr_last) {     // same character
                            if (col == col_last) { // same colours
                                if (col & 128)
                                    if (show_flashing != show_flashing_last)
                                        goto cantskip; // same flash
                                if (x == cx)
                                    if (y == cy)
                                        if (show_cursor != show_cursor_last)
                                            goto cantskip; // same cursor
                                if (x == cx_last) {
                                    if (y == cy_last) {
                                        if ((cx != cx_last) || (cy != cy_last))
                                            goto cantskip; // fixup old cursor's location
                                    }
                                }
                                goto skip;
                            }
                        }
                    }
                cantskip:
                    cp_last -= 2;
                    *cp_last = chr;
                    cp_last++;
                    *cp_last = col;
                    cp_last++;

                    // set cp2 to the character's data
                    z2 = 0; // double-width if set

                    if (f >= 32) { // custom font

                        static uint32 chr_utf32;
                        chr_utf32 = codepage437_to_unicode16[chr];

                        static uint8 *rt_data_last = NULL;
                        static int32 render_option;
                        static int32 ok;
                        static uint8 *rt_data;
                        static int32 rt_w, rt_h;
                        render_option = 1;
                        if (rt_data_last)
                            free(rt_data_last);
                        ok = FontRenderTextUTF32(font[f], (char32_t *)&chr_utf32, 1, render_option, &rt_data, &rt_w, &rt_h);
                        rt_data_last = rt_data;
                        cp2 = rt_data;
                        f_pitch = 0;

                    } else { // default font
                        f_pitch = 0;
                        if (f == 8)
                            cp2 = &charset8x8[chr][0][0];
                        if (f == 14)
                            cp2 = &charset8x16[chr][1][0];
                        if (f == 16)
                            cp2 = &charset8x16[chr][0][0];
                        if (f == (8 + 1)) {
                            cp2 = &charset8x8[chr][0][0];
                            z2 = 1;
                        }
                        if (f == (14 + 1)) {
                            cp2 = &charset8x16[chr][1][0];
                            z2 = 1;
                        }
                        if (f == (16 + 1)) {
                            cp2 = &charset8x16[chr][0][0];
                            z2 = 1;
                        }
                    }
                    c = col & 0xF; // foreground col
                    if (H3C0_blink_enable) {
                        c2 = (col >> 4) & 7; // background col
                        c3 = col >> 7;       // flashing?
                    } else {
                        c2 = (col >> 4); // background col
                    }
                    if (c3 && show_flashing && H3C0_blink_enable)
                        c = c2;
                    i2 = paldata[c];
                    i3 = paldata[c2];
                    lp = display_surface_offset + qbg_y_offset + y2 * x_monitor + x2;
                    z = x_monitor - fontwidth[display_page->font];

                    // inner loop
                    for (y3 = 0; y3 < f_height; y3++) {
                        for (x3 = 0; x3 < f_width; x3++) {
                            if (*cp2)
                                *lp = i2;
                            else
                                *lp = i3;
                            if (z2) {
                                if (x3 & z2)
                                    cp2++;
                            } else {
                                cp2++;
                            }
                            lp++;
                        }
                        lp += z;
                        cp2 += f_pitch;
                    } // y3,x3

                    // draw cursor
                    if (display_page->cursor_show && show_cursor && (cx == x) && (cy == y)) {
                        static int32 v1, v2;
                        static uint8 from_bottom; // bottom is the 2nd bottom scanline in width ?x25
                        static uint8 half_cursor; // if set, overrides all following values
                        static uint8 size;        // if 0, no cursor is drawn, if 255, from begin to bottom
                        static uint8 begin;       // only relevant if from_bottom was not specified
                        v1 = display_page->cursor_firstvalue;
                        v2 = display_page->cursor_lastvalue;
                        from_bottom = 0;
                        half_cursor = 0;
                        size = 0;
                        begin = 0;
                        // RULE: IF V2=0, NOTHING (UNLESS V1=0)
                        if (v2 == 0) {
                            if (v1 == 0) {
                                size = 1;
                                goto cursor_created;
                            }
                            goto nocursor; // no cursor!
                        }
                        // RULE: IF V2<V1, FROM V2 TO BOTTOM
                        if (v2 < v1) {
                            begin = v2;
                            size = 255;
                            goto cursor_created;
                        }
                        // RULE: IF V1=V2, SINGLE SCANLINE AT V1 (OR BOTTOM IF V1>=4)
                        if (v1 == v2) {
                            if (v1 <= 3) {
                                begin = v1;
                                size = 1;
                                goto cursor_created;
                            }
                            from_bottom = 1;
                            size = 1;
                            goto cursor_created;
                        }
                        // NOTE: V2 MUST BE LARGER THAN V1!
                        // RULE: IF V1>=3, CALC. DIFFERENCE BETWEEN V1 & V2
                        //                IF DIFF=1, 2 SCANLINES AT BOTTOM
                        //                IF DIFF=2, 3 SCANLINES AT BOTTOM
                        //                OTHERWISE HALF CURSOR
                        if (v1 >= 3) {
                            if ((v2 - v1) == 1) {
                                from_bottom = 1;
                                size = 2;
                                goto cursor_created;
                            }
                            if ((v2 - v1) == 2) {
                                from_bottom = 1;
                                size = 3;
                                goto cursor_created;
                            }
                            half_cursor = 1;
                            goto cursor_created;
                        }
                        // RULE: IF V1<=1, IF V2<=3 FROM V1 TO V3 ELSE FROM V1 TO BOTTOM
                        if (v1 <= 1) {
                            if (v2 <= 3) {
                                begin = v1;
                                size = v2 - v1 + 1;
                                goto cursor_created;
                            }
                            begin = v1;
                            size = 255;
                            goto cursor_created;
                        }
                        // RULE: IF V1=2, IF V2=3, 2 TO 3
                        //               IF V2=4, 3 SCANLINES AT BOTTOM
                        //               IF V2>=5, FROM 2 TO BOTTOM
                        //(assume V1=2)
                        if (v2 == 3) {
                            begin = 2;
                            size = 2;
                            goto cursor_created;
                        }
                        if (v2 == 4) {
                            from_bottom = 1;
                            size = 3;
                            goto cursor_created;
                        }
                        begin = 2;
                        size = 255;
                    cursor_created:
                        static int32 cw, ch;
                        cw = fontwidth[display_page->font];
                        ch = fontheight[display_page->font];
                        if (half_cursor) {
                            // half cursor
                            y3 = ch - 1;
                            size = ch / 2;
                            c = col & 0xF; // foreground col
                            i2 = paldata[c];
                        draw_half_curs:
                            lp = display_surface_offset + qbg_y_offset + (y2 + y3) * x_monitor + x2;
                            for (x3 = 0; x3 < cw; x3++) {
                                *lp = i2;
                                lp++;
                            }
                            y3--;
                            size--;
                            if (size)
                                goto draw_half_curs;
                        } else {
                            if (from_bottom) {
                                // cursor from bottom
                                y3 = ch - 1;
                                if (y3 == 15)
                                    y3 = 14;   // leave bottom line blank in 8x16 char set
                                c = col & 0xF; // foreground col
                                i2 = paldata[c];
                            draw_curs_from_bottom:
                                lp = display_surface_offset + qbg_y_offset + (y2 + y3) * x_monitor + x2;
                                for (x3 = 0; x3 < cw; x3++) {
                                    *lp = i2;
                                    lp++;
                                }
                                y3--;
                                size--;
                                if (size)
                                    goto draw_curs_from_bottom;
                            } else {
                                // cursor from begin using size
                                if (begin < ch) {
                                    y3 = begin;
                                    c = col & 0xF; // foreground col
                                    i2 = paldata[c];
                                    if (size == 255)
                                        size = ch - begin;
                                draw_curs_from_begin:
                                    lp = display_surface_offset + qbg_y_offset + (y2 + y3) * x_monitor + x2;
                                    for (x3 = 0; x3 < cw; x3++) {
                                        *lp = i2;
                                        lp++;
                                    }
                                    y3++;
                                    size--;
                                    if (size)
                                        goto draw_curs_from_begin;
                                }
                            }
                        }
                    } // draw cursor?
                nocursor:

                // outer loop
                skip:
                    x2 = x2 + fontwidth[display_page->font];
                }
                y2 = y2 + fontheight[display_page->font];
            }

            show_flashing_last = show_flashing;
            show_cursor_last = show_cursor;
            cx_last = cx;
            cy_last = cy;
            screen_last_valid = 1;

            if (BGRA_to_RGBA)
                swap_paldata_BGRA_with_RGBA();

            /*
                //backup for reuse in next frame
                i=display_frame[frame_i].w*display_frame[frame_i].h*4;
                if (i!=pixeldatasize){
                free(pixeldata);
                pixeldata=(uint8*)malloc(i);
                pixeldatasize=i;
                }
                memcpy(pixeldata,display_frame[frame_i].bgra,i);
            */

            last_frame_i = frame_i;

            goto screen_refreshed;

        } // text

        if (display_page->bits_per_pixel == 32) {

            // note: as software->hardware should be avoided at all costs, pixeldata is
            //      still backed up for comparison purposes because in the very likely
            //      event the data has not changed there is no point generating a
            //      new hardware surface from the software frame when the old hardware surface
            //      can be reused. It also saves on BGRA->RGBA conversion on some platforms.

            if (!BGRA_to_RGBA) {
                // find the most recently published page to compare with
                //(the most recent READY or DISPLAYING page)
                static int64 highest_order;
                highest_order = 0;
                i2 = -1;
                for (i3 = 0; i3 <= 2; i3++) {
                    if ((display_frame[i3].state == DISPLAY_FRAME_STATE__DISPLAYING || display_frame[i3].state == DISPLAY_FRAME_STATE__READY) &&
                        display_frame[i3].order > highest_order) {
                        highest_order = display_frame[i3].order;
                        i2 = i3;
                    }
                }
                if (force_display_update)
                    goto update_display32b; // force update
                if (i2 != -1) {
                    if (!screen_last_valid)
                        goto update_display32b; // force update because of mode change?
                    i = display_page->width * display_page->height * 4;
                    if (i != (display_frame[i2].w * display_frame[i2].h * 4))
                        goto update_display32b;

                    if (displayorder_screen == 0) {
                        // a valid frame of the correct dimensions exists and we are not required to display software content
                        goto no_new_frame;
                    }

                    if (memcmp(display_frame[i2].bgra, display_page->offset, i))
                        goto update_display32b;
                    goto no_new_frame; // no need to update display
                }
            update_display32b:;
            } else {

                // BGRA_to_RGBA
                i = display_page->width * display_page->height * 4;
                if (i != pixeldatasize) {
                    free(pixeldata);
                    pixeldata = (uint8 *)malloc(i);
                    pixeldatasize = i;
                    goto update_display32;
                }
                if (force_display_update)
                    goto update_display32; // force update

                if (displayorder_screen == 0) {
                    // a valid frame of the correct dimensions exists and we are not required to display software content
                    goto no_new_frame;
                }

                if (memcmp(pixeldata, display_page->offset, i))
                    goto update_display32;
                if (!screen_last_valid)
                    goto update_display32; // force update because of mode change?
                goto no_new_frame;         // no need to update display
            update_display32:
                memcpy(pixeldata, display_page->offset, i);
            }

            // ################################ Setup new frame ################################
            {
                static int32 new_size_bytes;
                new_size_bytes = x_monitor * y_monitor * 4;
                if (new_size_bytes > display_frame[frame_i].bytes) {
                    free(display_frame[frame_i].bgra);
                    display_frame[frame_i].bgra = (uint32 *)malloc(new_size_bytes);
                    display_frame[frame_i].bytes = new_size_bytes;
                }
                display_frame[frame_i].w = x_monitor;
                display_frame[frame_i].h = y_monitor;
            }

            if (!BGRA_to_RGBA) {
                memcpy(display_frame[frame_i].bgra, display_page->offset, display_frame[frame_i].w * display_frame[frame_i].h * 4);
            } else {
                static uint32 col;
                static uint32 *src_pos;
                static uint32 *dst_pos;
                src_pos = (uint32 *)pixeldata;
                dst_pos = display_frame[frame_i].bgra;
                static int32 pixels;
                pixels = display_frame[frame_i].w * display_frame[frame_i].h;
                if (pixels > 0) {
                    while (pixels--) {
                        col = *src_pos++;
                        *dst_pos++ = (col & 0xFF00FF00) | ((col & 0xFF0000) >> 16) | ((col & 0x0000FF) << 16);
                    }
                }
            }

            goto screen_refreshed;
        } // 32

        // assume <=256 colors using palette

        if (display_page->compatible_mode == 10) { // update SCREEN 10 palette
            i2 = GetTicks() & 512;
            if (i2)
                i2 = 1;
            for (i = 0; i <= 3; i++) {
                display_page->pal[i] = pal_mode10[i2][display_page->pal[i + 4]]; // pal_mode10[0-1][0-8]
            }
        }

        i = display_page->width * display_page->height;
        i2 = 1 << display_page->bits_per_pixel; // unique colors

        // data changed?
        if (i != pixeldatasize) {
            free(pixeldata);
            pixeldata = (uint8 *)malloc(i);
            pixeldatasize = i;
            goto update_display;
        }

        if (force_display_update)
            goto update_display; // force update

        if (displayorder_screen == 0) {
            // a valid frame of the correct dimensions exists and we are not required to display software content
            goto no_new_frame;
        }

        if (memcmp(pixeldata, display_page->offset, i))
            goto update_display;
        // palette changed?
        if (memcmp(paldata, display_page->pal, i2 * 4))
            goto update_display;
        // force update because of mode change?
        if (!screen_last_valid)
            goto update_display;

        goto no_new_frame; // no need to update display

    update_display:

        // ################################ Setup new frame ################################
        {
            static int32 new_size_bytes;
            new_size_bytes = x_monitor * y_monitor * 4;
            if (new_size_bytes > display_frame[frame_i].bytes) {
                free(display_frame[frame_i].bgra);
                display_frame[frame_i].bgra = (uint32 *)malloc(new_size_bytes);
                display_frame[frame_i].bytes = new_size_bytes;
            }
            display_frame[frame_i].w = x_monitor;
            display_frame[frame_i].h = y_monitor;
        }

        display_surface_offset = display_frame[frame_i].bgra;

        memcpy(pixeldata, display_page->offset, i);
        memcpy(paldata, display_page->pal, i2 * 4);

        if (BGRA_to_RGBA)
            swap_paldata_BGRA_with_RGBA();
        static uint8 *cp;
        static uint32 *lp2;
        static uint32 c;
        cp = pixeldata;
        lp2 = display_surface_offset;
        x2 = display_page->width;
        y2 = display_page->height;
        for (y = 0; y < y2; y++) {
            for (x = 0; x < x2; x++) {
                *lp2++ = paldata[*cp++];
            } // x
        } // y
        if (BGRA_to_RGBA)
            swap_paldata_BGRA_with_RGBA();

        goto screen_refreshed;

    screen_refreshed:

        force_display_update = 0;

        screen_last_valid = 1;

        // Set new display frame as ready
        // display_frame_end=frame_i;
        // if (!display_frame_begin) display_frame_begin=frame_i;

        display_frame[frame_i].state = DISPLAY_FRAME_STATE__READY;
        last_hardware_display_frame_order = display_frame[frame_i].order;

    no_new_frame:;
    display_page_invalid:;

        // cancel frame if not built
        if (display_frame[frame_i].state == DISPLAY_FRAME_STATE__BUILDING) {
            last_hardware_display_frame_order = display_frame[frame_i].order;
            display_frame[frame_i].state = DISPLAY_FRAME_STATE__EMPTY;
        }

    } // lock_display==0
    if (lock_display == 1) {
        lock_display = 2;
        Sleep(0);
    }
    if (autodisplay == -1)
        autodisplay = 0;
    display_called = 1;
    return;
}

/*
    int message_loop(
    SDL_Event event,
    SDL_Surface *screen,
    SDL_Surface *back,
    int *inputedWidth,
    Uint16 *inputedString,
    TTF_Font *font)
    {
    SDL_Color fg = {0x66, 0x66, 0xFF};
    SDL_Color bg = {0x00, 0x00, 0x00};
    SDL_Rect rect;
    SDL_Surface *surface;
    SDL_Event eventExpose;

    switch(event.type){
    case SDL_QUIT:
    return 1;
    case SDL_VIDEOEXPOSE:
    SDL_BlitSurface(back, NULL, screen, NULL);
    SDL_UpdateRect(screen, 0, 0, 0, 0);
    break;
    case SDL_KEYDOWN:





    if (event.key.keysym.sym == QBVK_F1) {
    InputMethod_Reset();
    }
    if (event.key.keysym.sym == QBVK_F2) {
    *inputedWidth = 0;
    inputedString[0] = 0x0000;
    InputMethod_Reset();
    }
    if (event.key.keysym.sym == QBVK_F3) {
    InputMethod_Validate();
    rect.x = 0;
    rect.y = 200;
    rect.w = 640;
    rect.h = 100;
    SDL_FillRect(back, &rect, 0x00000000);
    surface = TTF_RenderUTF8_Shaded(
    font, "Valid", fg, bg);
    SDL_BlitSurface(surface, NULL, back, &rect);
    SDL_FreeSurface(surface);
    eventExpose.type = SDL_VIDEOEXPOSE;
    SDL_PushEvent(&eventExpose);
    }
    if (event.key.keysym.sym == QBVK_F4) {
    InputMethod_Invalidate();
    rect.x = 0;
    rect.y = 200;
    rect.w = 640;
    rect.h = 100;
    SDL_FillRect(back, &rect, 0x00000000);
    surface = TTF_RenderUTF8_Shaded(
    font, "Invalid", fg, bg);
    SDL_BlitSurface(surface, NULL, back, &rect);
    SDL_FreeSurface(surface);
    eventExpose.type = SDL_VIDEOEXPOSE;
    SDL_PushEvent(&eventExpose);
    }
    break;
    default:
    break;
    }
    return 0;
    }
*/

void update_shift_state() {
    int32 x;
    /*
        0:417h                   Shift Status
        7 6 5 4 3 2 1 0
        x . . . . . . .      Insert locked
        . x . . . . . .      Caps Lock locked
        . . x . . . . .      Num Lock locked
        . . . x . . . .      Scroll Lock locked
        . . . . x . . .      Alt key is pressed
        . . . . . x . .      Ctrl key is pressed
        . . . . . . x .      Left Shift key is pressed
        . . . . . . . x      Right Shift key is pressed
    */
    x = 0;
    if (keyheld(VK + QBVK_RSHIFT))
        x |= 1;
    if (keyheld(VK + QBVK_LSHIFT))
        x |= 2;
    if (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL))
        x |= 4;
    if (keyheld(VK + QBVK_LALT) || keyheld(VK + QBVK_RALT))
        x |= 8;
    if (keyheld(QBK + QBK_SCROLL_LOCK_MODE))
        x |= 16;
    if (keyheld(VK + QBVK_NUMLOCK))
        x |= 32;
    if (keyheld(VK + QBVK_CAPSLOCK))
        x |= 64;
    // note: insert state is emulated (off by default)
    if (keyheld(QBK + QBK_INSERT_MODE))
        x |= 128;
    cmem[0x417] = x;
    /*
        0:418h                   Extended Shift Status
        (interpret the word 'pressed' as "being held down")
        7 6 5 4 3 2 1 0
        x . . . . . . .      Ins key is pressed
        . x . . . . . .      Caps Lock key is pressed (detection not possible, return 0)
        . . x . . . . .      Num Lock key is pressed (detection not possible, return 0)
        . . . x . . . .      Scroll Lock key is pressed
        . . . . x . . .      Pause key locked
        . . . . . x . .      SysReq key is pressed
        . . . . . . x .      Left Alt key is pressed
        . . . . . . . x      Left Ctrl key is pressed
    */
    x = 0;
    if (keyheld(VK + QBVK_LCTRL))
        x |= 1;
    if (keyheld(VK + QBVK_LALT))
        x |= 2;
    if (keyheld(VK + QBVK_SYSREQ))
        x |= 4;
    if (keyheld(VK + QBVK_PAUSE))
        x |= 8;
    if (keyheld(VK + QBVK_SCROLLOCK))
        x |= 16;
    // if (keyheld(VK+QBVK_NUMLOCK)) x|=32;
    // if (keyheld(VK+QBVK_CAPSLOCK)) x|=64;
    if (keyheld(0x5200))
        x |= 128;
    cmem[0x418] = x;
    /*
        0:496h                   Keyboard Status and Type Flags
        This byte holds keyboard status information.
        Keyboard Status Information
        7 6 5 4 3 2 1 0
        x . . . . . . .       Read ID in progress (always 0)
        . x . . . . . .       Last character was first ID character (always 0)
        . . x . . . . .       Force Num Lock if read ID and KBX (always 0)
        . . . x . . . .       101/102-key keyboard installed (always 1)
        . . . . x . . .       Right Alt key is pressed
        . . . . . x . .       Right Ctrl key is pressed
        . . . . . . x .       Last code was E0 Hidden Code (always 0)
        . . . . . . . x       last code was E1 Hidden Code (always 0)
    */
    x = 0;
    if (keyheld(VK + QBVK_RCTRL))
        x |= 1;
    if (keyheld(VK + QBVK_RALT))
        x |= 2;
    x |= 16;
    cmem[0x496] = x;
}

int32 keyup_mask_last = -1;
uint32 keyup_mask[256]; // NULL values indicate removed masks

void keyup(uint32 x) {

    if (!x)
        x = QBK + QBK_CHR0;

    keyheld_remove(x);

    if (asciicode_reading != 2) { // hide numpad presses related to ALT+1+2+3 type entries
        // identify and revert numpad specific key codes to non-numpad codes
        static uint32 x2;
        static int64 numpadkey;
        numpadkey = 0;
        x2 = x;
        // check multimapped NUMPAD keys
        if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP_ENTER))) {
            numpadkey = 4294967296ll;
            if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP9))) {
                x2 = x - (VK + QBVK_KP0) + 48;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_PERIOD)) {
                x2 = 46;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_DIVIDE)) {
                x2 = 47;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_MULTIPLY)) {
                x2 = 42;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_MINUS)) {
                x2 = 45;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_PLUS)) {
                x2 = 43;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_ENTER)) {
                x2 = 13;
                goto onnumpad;
            }
        }
        if ((x >= (QBK + 0)) && (x <= (QBK + 10))) {
            numpadkey = 4294967296ll;
            x2 = x - QBK;
            if (x2 == 0) {
                x2 = 82 << 8;
                goto onnumpad;
            }
            if (x2 == 1) {
                x2 = 79 << 8;
                goto onnumpad;
            }
            if (x2 == 2) {
                x2 = 80 << 8;
                goto onnumpad;
            }
            if (x2 == 3) {
                x2 = 81 << 8;
                goto onnumpad;
            }
            if (x2 == 4) {
                x2 = 75 << 8;
                goto onnumpad;
            }
            if (x2 == 5) {
                x2 = 76 << 8;
                goto onnumpad;
            }
            if (x2 == 6) {
                x2 = 77 << 8;
                goto onnumpad;
            }
            if (x2 == 7) {
                x2 = 71 << 8;
                goto onnumpad;
            }
            if (x2 == 8) {
                x2 = 72 << 8;
                goto onnumpad;
            }
            if (x2 == 9) {
                x2 = 73 << 8;
                goto onnumpad;
            }
            if (x2 == 10) {
                x2 = 83 << 8;
                goto onnumpad;
            }
        }
    onnumpad:;

        static int32 i;
        for (i = 0; i <= keyup_mask_last; i++) {
            if (x == keyup_mask[i]) {
                keyup_mask[i] = 0;
                goto key_handled;
            }
        }

        // add x2 to keyhit buffer
        static int32 z;
        z = (keyhit_nextfree + 1) & 0x1FFF;
        if (z == keyhit_next) { // remove oldest message when cyclic buffer is full
            keyhit_next = (keyhit_next + 1) & 0x1FFF;
        }
        static int32 sx;
        sx = x2;
        sx = -sx;
        x2 = sx; // negate x2
        keyhit[keyhit_nextfree] = x2 | numpadkey;
        keyhit_nextfree = z;
    } // asciicode_reading!=2

    static int32 shift, alt, ctrl, capslock, numlock;
    numlock = 0;
    capslock = 0;

    if (x <= 255) {
        if (scancode_lookup[x * 10 + 2])
            scancodeup(scancode_lookup[x * 10 + 1]);
        goto key_handled;
    } // x<=255

    // NUMPAD?
    if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP_ENTER))) {
        if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP_PERIOD)))
            numlock = 1;
        x = (x - (VK + QBVK_KP0) + 256) * 256;
        goto numpadkey;
    }
    if ((x >= (QBK + 0)) && (x <= (QBK + 0 + (QBVK_KP_PERIOD - QBVK_KP0)))) {
        x = (x - (QBK + 0) + 256) * 256;
        goto numpadkey;
    }

    if (x <= 65535) {
        static int32 r;
    numpadkey:
        r = (x >> 8) + 256;
        if (scancode_lookup[r * 10 + 2])
            scancodeup(scancode_lookup[r * 10 + 1]);

        if (x == 0x5200) { // INSERT lock emulation
            update_shift_state();
        }

        goto key_handled;
    } // x<=65536

    if (x == (VK + QBVK_LSHIFT)) {
        scancodeup(42);
        update_shift_state();
    }
    if (x == (VK + QBVK_RSHIFT)) {
        scancodeup(54);
        update_shift_state();
    }
    if (x == (VK + QBVK_LALT)) {
        scancodeup(56);
        update_shift_state();
    }
    if (x == (VK + QBVK_RALT)) {
        scancodeup(56);
        update_shift_state();
    }
    if (x == (VK + QBVK_LCTRL)) {
        scancodeup(29);
        update_shift_state();
    }
    if (x == (VK + QBVK_RCTRL)) {
        scancodeup(29);
        update_shift_state();
    }
    if (x == (VK + QBVK_NUMLOCK)) {
        scancodeup(69);
        update_shift_state();
    }
    if (x == (VK + QBVK_CAPSLOCK)) {
        scancodeup(58);
        update_shift_state();
    }
    if (x == (VK + QBVK_SCROLLOCK)) {
        scancodeup(70);
        update_shift_state();
    }

key_handled:;
}

void keydown(uint32 x) {

    if (!x)
        x = QBK + QBK_CHR0;

    static int32 glyph;
    glyph = keydown_glyph;
    keydown_glyph = 0;

    // INSERT lock emulation
    static int32 insert_held;
    if (x == 0x5200)
        insert_held = keyheld(0x5200);

    // SCROLL lock tracking
    static int32 scroll_lock_held;
    if (x == (VK + QBVK_SCROLLOCK))
        scroll_lock_held = keyheld(VK + QBVK_SCROLLOCK);

    keyheld_add(x);

    // note: On early keyboards without a Pause key (before the introduction of 101-key keyboards) the Pause function was assigned to Ctrl+NumLock, and the
    // Break function to Ctrl+ScrLock; these key-combinations still work with most programs, even on modern PCs with modern keyboards. CTRL+BREAK handling
    if ((x == (VK + QBVK_BREAK)) || ((x == (VK + QBVK_SCROLLOCK)) && (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL))) ||
        ((x == (VK + QBVK_F15)) && (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL)))) {
        if (exit_blocked) {
            exit_value |= 2;
            goto key_handled;
        }
        close_program = 1;
        goto key_handled;
    }

#ifdef QB64_WINDOWS
    // note: Alt+F4 is supposed to close the window, but glut windows don't seem to be affected;
    // this addresses the issue:
    if ((x == (0x3E00)) && (keyheld(VK + QBVK_RALT) || keyheld(VK + QBVK_LALT))) {
        if (exit_blocked) {
            exit_value |= 1;
            goto key_handled;
        }
        close_program = 1;
        goto key_handled;
    }
#endif

    // note: On early keyboards without a Pause key (before the introduction of 101-key keyboards) the Pause function was assigned to Ctrl+NumLock, and the
    // Break function to Ctrl+ScrLock; these key-combinations still work with most programs, even on modern PCs with modern keyboards. PAUSE handling
    if ((x == (VK + QBVK_PAUSE)) || ((x == (VK + QBVK_NUMLOCK)) && (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL)))) {
        suspend_program |= 1;
        qbevent = 1;
        goto key_handled;
    } else {
        if (suspend_program & 1) {
            suspend_program ^= 1;
            goto key_handled;
        }
    }

    // ALT+ENTER
    if (keyheld(VK + QBVK_RALT) || keyheld(VK + QBVK_LALT)) {
        if (x == 13) {
            if (fullscreen_allowedmode >= 0) { // fullscreen_allowedmode==-1 bypasses alt+enter allowing it to be user-trappable
                static int32 fs_mode, fs_smooth;
                fs_mode = full_screen_set;
                if (fs_mode == -1)
                    fs_mode = full_screen;
                fs_smooth = fullscreen_smooth;

                int32 fs_combo;
                if (fs_mode == 0)
                    fs_combo = 0;
                if ((fs_mode == 1) && (fs_smooth == 0))
                    fs_combo = 1;
                if ((fs_mode == 1) && (fs_smooth == 1))
                    fs_combo = 2;
                if ((fs_mode == 2) && (fs_smooth == 0))
                    fs_combo = 3;
                if ((fs_mode == 2) && (fs_smooth == 1))
                    fs_combo = 4;

                int32 fs_validmode = 0;
                while (fs_validmode == 0) {
                    fs_combo++;
                    if (fs_combo > 4)
                        fs_combo = 0;

                    switch (fs_combo) {
                    case 0:
                        fs_mode = 0;
                        fullscreen_smooth = 0;
                        break;
                    case 1:
                        fs_mode = 1;
                        fullscreen_smooth = 0;
                        break;
                    case 2:
                        fs_mode = 1;
                        fullscreen_smooth = 1;
                        break;
                    case 3:
                        fs_mode = 2;
                        fullscreen_smooth = 0;
                        break;
                    case 4:
                        fs_mode = 2;
                        fullscreen_smooth = 1;
                        break;
                    }

                    if (fs_combo == 0)
                        break; // 0 is always valid (= _OFF)

                    fs_validmode = 1;
                    // check _ALLOWFULLSCREEN's settings
                    if ((fullscreen_allowedmode > 0) && (fs_mode != fullscreen_allowedmode))
                        fs_validmode = 0;
                    if ((fullscreen_allowedsmooth == 1) && (fullscreen_smooth != 1))
                        fs_validmode = 0;
                    if ((fullscreen_allowedsmooth == -1) && (fullscreen_smooth != 0))
                        fs_validmode = 0;
                }

                // apply
                if (full_screen != fs_mode)
                    full_screen_set = fs_mode;
                force_display_update = 1;
                goto key_handled;
            }
        }
    }

    if (asciicode_reading != 2) { // hide numpad presses related to ALT+1+2+3 type entries

        // identify and revert numpad specific key codes to non-numpad codes
        static uint32 x2;
        static int64 numpadkey;
        numpadkey = 0;
        x2 = x;
        // check multimapped NUMPAD keys
        if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP_ENTER))) {
            numpadkey = 4294967296ll;
            if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP9))) {
                x2 = x - (VK + QBVK_KP0) + 48;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_PERIOD)) {
                x2 = 46;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_DIVIDE)) {
                x2 = 47;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_MULTIPLY)) {
                x2 = 42;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_MINUS)) {
                x2 = 45;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_PLUS)) {
                x2 = 43;
                goto onnumpad;
            }
            if (x == (VK + QBVK_KP_ENTER)) {
                x2 = 13;
                goto onnumpad;
            }
        }
        if ((x >= (QBK + 0)) && (x <= (QBK + 10))) {
            numpadkey = 4294967296ll;
            x2 = x - QBK;
            if (x2 == 0) {
                x2 = 82 << 8;
                goto onnumpad;
            }
            if (x2 == 1) {
                x2 = 79 << 8;
                goto onnumpad;
            }
            if (x2 == 2) {
                x2 = 80 << 8;
                goto onnumpad;
            }
            if (x2 == 3) {
                x2 = 81 << 8;
                goto onnumpad;
            }
            if (x2 == 4) {
                x2 = 75 << 8;
                goto onnumpad;
            }
            if (x2 == 5) {
                x2 = 76 << 8;
                goto onnumpad;
            }
            if (x2 == 6) {
                x2 = 77 << 8;
                goto onnumpad;
            }
            if (x2 == 7) {
                x2 = 71 << 8;
                goto onnumpad;
            }
            if (x2 == 8) {
                x2 = 72 << 8;
                goto onnumpad;
            }
            if (x2 == 9) {
                x2 = 73 << 8;
                goto onnumpad;
            }
            if (x2 == 10) {
                x2 = 83 << 8;
                goto onnumpad;
            }
        }
    onnumpad:;

        // ON KEY trapping
        { // new scope
            static int32 block_onkey = 0;
            static int32 f, x3, scancode, extended, c, flags_mask;
            int32 i, i2; // must not be static!

            // establish scancode (if any)
            scancode = 0;
            if (x <= 255) {
                scancode = scancode_lookup[x * 10 + 1];
                goto onkey_gotscancode;
            }
            //*check for 2 byte scancodes here
            x3 = x;
            if ((x3 >= (VK + QBVK_KP0)) && (x3 <= (VK + QBVK_KP_ENTER))) {
                x3 = (x3 - (VK + QBVK_KP0) + 256) * 256;
                goto onkey_numpadkey;
            }
            if ((x3 >= (QBK + 0)) && (x3 <= (QBK + 0 + (QBVK_KP_PERIOD - QBVK_KP0)))) {
                x3 = (x3 - (QBK + 0) + 256) * 256;
                goto onkey_numpadkey;
            }
            if (x3 <= 65535) {
            onkey_numpadkey:
                i = (x3 >> 8) + 256;
                if (scancode_lookup[i * 10 + 2])
                    scancode = scancode_lookup[i * 10 + 1];
            }
        onkey_gotscancode:

            // check modifier keys
            if (x == (VK + QBVK_LSHIFT)) {
                scancode = 42;
                flags_mask = 3;
            }
            if (x == (VK + QBVK_RSHIFT)) {
                scancode = 54;
                flags_mask = 3;
            }
            if (x == (VK + QBVK_LALT)) {
                scancode = 56;
                flags_mask = 8;
            }
            if (x == (VK + QBVK_RALT)) {
                scancode = 56;
                flags_mask = 8;
            }
            if (x == (VK + QBVK_LCTRL)) {
                scancode = 29;
                flags_mask = 4;
            }
            if (x == (VK + QBVK_RCTRL)) {
                scancode = 29;
                flags_mask = 4;
            }
            if (x == (VK + QBVK_NUMLOCK)) {
                scancode = 69;
                flags_mask = 32;
            }
            if (x == (VK + QBVK_CAPSLOCK)) {
                scancode = 58;
                flags_mask = 64;
            }
            if (x == (VK + QBVK_SCROLLOCK)) {
                scancode = 70;
                // note: no mask required
            }

            // establish if key is an extended key
            extended = 0;
            // arrow-pad (note: num-pad is ignored because x is a QB64 pure key value and only refers to the arrow-pad)
            if (x == 0x4B00)
                extended = 1;
            if (x == 0x4800)
                extended = 1;
            if (x == 0x4D00)
                extended = 1;
            if (x == 0x5000)
                extended = 1;
            // num-pad extended keys
            if (x == VK + QBVK_KP_DIVIDE)
                extended = 1;
            if (x == VK + QBVK_KP_ENTER)
                extended = 1;
            // ins/del/hom/end/pgu/pgd pad
            if (x == 0x5200)
                extended = 1;
            if (x == 0x4700)
                extended = 1;
            if (x == 0x4900)
                extended = 1;
            if (x == 0x5300)
                extended = 1;
            if (x == 0x4F00)
                extended = 1;
            if (x == 0x5100)
                extended = 1;
            // right alt/right control
            if (x == VK + QBVK_RCTRL)
                extended = 1;
            if (x == VK + QBVK_RALT)
                extended = 1;

            if (!block_onkey) {

                // priority #1: user defined keys
                if (scancode) {
                    for (i = 0; i <= 31; i++) {
                        if (onkey[i].key_scancode == scancode) {
                            if (onkey[i].active) {
                                if (onkey[i].id) {
                                    // check keyboard flags
                                    f = onkey[i].key_flags;
                                    // 0 No keyboard flag, 1-3 Either Shift key, 4 Ctrl key, 8 Alt key,32 NumLock key,64 Caps Lock key, 128 Extended keys on a
                                    // 101-key keyboard To specify multiple shift states, add the values together. For example, a value of 12 specifies that the
                                    // user-defined key is used in combination with the Ctrl and Alt keys.
                                    if ((flags_mask & 3) == 0) {
                                        if (f & 3) {
                                            if (keyheld(VK + QBVK_LSHIFT) == 0 && keyheld(VK + QBVK_RSHIFT) == 0)
                                                goto wrong_flags;
                                        } else {
                                            if (keyheld(VK + QBVK_LSHIFT) || keyheld(VK + QBVK_RSHIFT))
                                                goto wrong_flags;
                                        }
                                    }
                                    if ((flags_mask & 4) == 0) {
                                        if (f & 4) {
                                            if (keyheld(VK + QBVK_LCTRL) == 0 && keyheld(VK + QBVK_RCTRL) == 0)
                                                goto wrong_flags;
                                        } else {
                                            if (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL))
                                                goto wrong_flags;
                                        }
                                    }
                                    if ((flags_mask & 8) == 0) {
                                        if (f & 8) {
                                            if (keyheld(VK + QBVK_LALT) == 0 && keyheld(VK + QBVK_RALT) == 0)
                                                goto wrong_flags;
                                        } else {
                                            if (keyheld(VK + QBVK_LALT) || keyheld(VK + QBVK_RALT))
                                                goto wrong_flags;
                                        }
                                    }
                                    if ((flags_mask & 32) == 0) {
                                        if (f & 32) {
                                            if (keyheld(VK + QBVK_NUMLOCK) == 0)
                                                goto wrong_flags;
                                            //*revise
                                        }
                                    }
                                    if ((flags_mask & 64) == 0) {
                                        if (f & 64) {
                                            if (keyheld(VK + QBVK_CAPSLOCK) == 0)
                                                goto wrong_flags;
                                            //*revise
                                        }
                                    }
                                    if ((flags_mask & 128) == 0) {
                                        if (((f & 128) / 128) != extended)
                                            goto wrong_flags;
                                    }
                                    if (onkey[i].active == 1) { //(1)ON
                                        onkey[i].state++;
                                    } else { //(2)STOP
                                        onkey[i].state = 1;
                                    }

                                    qbevent = 1;

                                    // mask trigger key
                                    for (i = 0; i <= keyup_mask_last; i++) {
                                        if (!keyup_mask[i]) {
                                            keyup_mask[i] = x;
                                            break;
                                        }
                                    }
                                    if (i == keyup_mask_last + 1) {
                                        if (keyup_mask_last < 255) {
                                            keyup_mask[i] = x;
                                            keyup_mask_last++;
                                        }
                                    }

                                    goto key_handled;

                                } // id
                            } // active
                        } // scancode==
                    wrong_flags:;
                    } // i
                } // scancode

                // priority #2: fixed index F1-F12, arrows
                for (i = 0; i <= 31; i++) {
                    if (onkey[i].active) {
                        if (onkey[i].id) {
                            if ((x2 == onkey[i].keycode) || x == onkey[i].keycode_alternate) {
                                if (onkey[i].active == 1) { //(1)ON
                                    onkey[i].state++;
                                } else { //(2)STOP
                                    onkey[i].state = 1;
                                }
                                qbevent = 1;

                                // mask trigger key
                                for (i = 0; i <= keyup_mask_last; i++) {
                                    if (!keyup_mask[i]) {
                                        keyup_mask[i] = x;
                                        break;
                                    }
                                }
                                if (i == keyup_mask_last + 1) {
                                    if (keyup_mask_last < 255) {
                                        keyup_mask[i] = x;
                                        keyup_mask_last++;
                                    }
                                }

                                goto key_handled;
                            } // keycode
                        } // id
                    } // active
                } // i

            } // block_onkey

            // priority #3: string insertion
            for (i = 0; i <= 31; i++) {
                if (onkey[i].text) {
                    if (onkey[i].text->len) {
                        if ((x2 == onkey[i].keycode) || x == onkey[i].keycode_alternate) {

                            // mask trigger key
                            { // scope
                                static int32 i;
                                for (i = 0; i <= keyup_mask_last; i++) {
                                    if (!keyup_mask[i]) {
                                        keyup_mask[i] = x;
                                        break;
                                    }
                                }
                                if (i == keyup_mask_last + 1) {
                                    if (keyup_mask_last < 255) {
                                        keyup_mask[i] = x;
                                        keyup_mask_last++;
                                    }
                                }
                            } // descope

                            for (i2 = 0; i2 < onkey[i].text->len; i2++) {
                                block_onkey = 1;
                                keydown_ascii(onkey[i].text->chr[i2]);
                                keyup_ascii(onkey[i].text->chr[i2]);
                                block_onkey = 0;
                            } // i2
                            goto key_handled;
                        } // keycode
                    }
                } // text
            } // i

        } // descope

        /*
            //keyhit cyclic buffer
            int64 keyhit[8192];
            //    keyhit specific internal flags: (stored in high 32-bits)
            //    &4294967296->numpad was used
            int32 keyhit_nextfree=0;
            int32 keyhit_next=0;
            //note: if full, the oldest message is discarded to make way for the new message
        */
        // add x2 to keyhit buffer
        static int32 z;
        z = (keyhit_nextfree + 1) & 0x1FFF;
        if (z == keyhit_next) { // remove oldest message when cyclic buffer is full
            keyhit_next = (keyhit_next + 1) & 0x1FFF;
        }
        keyhit[keyhit_nextfree] = x2 | numpadkey;
        keyhit_nextfree = z;
    } // asciicode_reading!=2

    static int32 shift, alt, ctrl, capslock, numlock;
    numlock = 0;
    capslock = 0;

    if (x == QBK + QBK_CHR0)
        x = 0;

    if (x <= 255) {
        static int32 b1, b2, z, o;
        b1 = x;
        if ((b2 = scancode_lookup[x * 10 + 1])) { // table entry exists
            scancodedown(b2);

            // check for relevant table modifiers
            shift = 0;
            if (keyheld(VK + QBVK_LSHIFT) || keyheld(VK + QBVK_RSHIFT))
                shift = 1;
            ctrl = 0;
            if (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL))
                ctrl = 1;
            alt = 0;
            if (keyheld(VK + QBVK_LALT) || keyheld(VK + QBVK_RALT))
                alt = 1;
            o = 0;
            if (shift)
                o = 1;
            if (ctrl)
                o = 2;
            if (alt)
                o = 3;
            if (glyph) {
                if ((keyheld(VK + QBVK_LALT) == 0) && (keyheld(VK + QBVK_RCTRL) == 0) && keyheld(VK + QBVK_LCTRL) && keyheld(VK + QBVK_RALT))
                    o = 0; // assume alt-gr combo-key
            }
            z = scancode_lookup[x * 10 + 2 + o];
            if (!z)
                goto key_handled; // not possible
            if (z & 0xFF00) {
                b1 = 0;
                b2 = z >> 8;
            } else {
                b1 = z;
            }
        } // b2
        static int32 i, i2, i3;
        i = cmem[0x41a];
        i2 = cmem[0x41c];
        i3 = i2 + 2;
        if (i3 == 62)
            i3 = 30;
        if (i != i3) { // fits in buffer
            cmem[0x400 + i2] = b1;
            cmem[0x400 + i2 + 1] = b2; //(scancode)
            cmem[0x41c] = i3;          // fix tail location
        }
        goto key_handled;
    } // x<=255

    // NUMPAD?
    if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP_ENTER))) {
        if ((x >= (VK + QBVK_KP0)) && (x <= (VK + QBVK_KP_PERIOD)))
            numlock = 1;
        x = (x - (VK + QBVK_KP0) + 256) * 256;
        goto numpadkey;
    }
    if ((x >= (QBK + 0)) && (x <= (QBK + 0 + (QBVK_KP_PERIOD - QBVK_KP0)))) {
        x = (x - (QBK + 0) + 256) * 256;
        goto numpadkey;
    }

    if (x <= 65535) {
        static int32 b1, b2, z, o, r;
    numpadkey:
        b1 = 0;
        b2 = x >> 8;
        r = (x >> 8) + 256;
        if (scancode_lookup[r * 10 + 2]) {
            scancodedown(scancode_lookup[r * 10 + 1]);
            // check relevant modifiers
            shift = 0;
            if (keyheld(VK + QBVK_LSHIFT) || keyheld(VK + QBVK_RSHIFT))
                shift = 1;
            ctrl = 0;
            if (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL))
                ctrl = 1;
            alt = 0;
            if (keyheld(VK + QBVK_LALT) || keyheld(VK + QBVK_RALT))
                alt = 1;

            if (x == 0x5200) {          // INSERT lock emulation
                if (insert_held == 0) { // nullify effects of key repeats
                    if ((alt == 0) && (shift == 0) && (ctrl == 0)) {
                        // toggle insert mode
                        if (keyheld(QBK + QBK_INSERT_MODE)) {
                            keyheld_remove(QBK + QBK_INSERT_MODE);
                        } else {
                            keyheld_add(QBK + QBK_INSERT_MODE);
                        }
                        update_shift_state();
                    }
                }
            }

            o = 0;
            if (shift)
                o = 1;
            if (numlock)
                o = 4;
            if (numlock && shift)
                o = 7;
            if (ctrl)
                o = 2;
            if (alt)
                o = 3;
            z = scancode_lookup[r * 10 + 2 + o];
            if (!z)
                goto key_handled; // invalid combination
            if (z & 0xFF00) {
                b1 = 0;
                b2 = z >> 8;
            } else {
                b1 = z;
                b2 = scancode_lookup[r * 10 + 1];
            }
        } // z
        static int32 i, i2, i3;
        i = cmem[0x41a];
        i2 = cmem[0x41c];
        i3 = i2 + 2;
        if (i3 == 62)
            i3 = 30;
        if (i != i3) { // fits in buffer
            cmem[0x400 + i2] = b1;
            cmem[0x400 + i2 + 1] = b2; //(scancode)
            cmem[0x41c] = i3;          // fix tail location
        }
        goto key_handled;
    } // x<=65536

    if (x == (VK + QBVK_LSHIFT)) {
        scancodedown(42);
        update_shift_state();
    }
    if (x == (VK + QBVK_RSHIFT)) {
        scancodedown(54);
        update_shift_state();
    }
    if (x == (VK + QBVK_LALT)) {
        scancodedown(56);
        update_shift_state();
    }
    if (x == (VK + QBVK_RALT)) {
        scancodedown(56);
        update_shift_state();
    }
    if (x == (VK + QBVK_LCTRL)) {
        scancodedown(29);
        update_shift_state();
    }
    if (x == (VK + QBVK_RCTRL)) {
        scancodedown(29);
        update_shift_state();
    }
    if (x == (VK + QBVK_NUMLOCK)) {
        scancodedown(69);
        update_shift_state();
    }
    if (x == (VK + QBVK_CAPSLOCK)) {
        scancodedown(58);
        update_shift_state();
    }
    if (x == (VK + QBVK_SCROLLOCK)) {
        scancodedown(70);

        if (scroll_lock_held == 0) { // nullify effects of key repeats
            ctrl = 0;
            if (keyheld(VK + QBVK_LCTRL) || keyheld(VK + QBVK_RCTRL))
                ctrl = 1;
            if (ctrl == 0) {
                // toggle insert mode
                if (keyheld(QBK + QBK_SCROLL_LOCK_MODE)) {
                    keyheld_remove(QBK + QBK_SCROLL_LOCK_MODE);
                } else {
                    keyheld_add(QBK + QBK_SCROLL_LOCK_MODE);
                }
            }
        }

        update_shift_state();
    }

key_handled:
    sleep_break = 1;
}

void scancodedown(uint8 scancode) {
    if (port60h_events) {
        if (port60h_event[port60h_events - 1] == scancode)
            return; // avoid duplicate entries in buffer (eg. from key-repeats)
    }
    if (port60h_events == 256) {
        memmove(port60h_event, port60h_event + 1, 255);
        port60h_events = 255;
    }
    port60h_event[port60h_events] = scancode;
    port60h_events++;
}

void scancodeup(uint8 scancode) {
    if (port60h_events) {
        if (port60h_event[port60h_events - 1] == (scancode + 128))
            return; // avoid duplicate entries in buffer
    }
    if (port60h_events == 256) {
        memmove(port60h_event, port60h_event + 1, 255);
        port60h_events = 255;
    }
    port60h_event[port60h_events] = scancode + 128;
    port60h_events++;
}

#define OS_EVENT_PRE_PROCESSING 1
#define OS_EVENT_POST_PROCESSING 2
#define OS_EVENT_RETURN_IMMEDIATELY 3

static void reportKeyState(int code, int down) {
    static device_struct *d;
    d = &devices[1]; // keyboard

    // don't add message if state already matches what we're reporting
    if (getDeviceEventButtonValue(d, d->queued_events - 1, code) != down) {
        int32 eventIndex = createDeviceEvent(d);
        setDeviceEventButtonValue(d, eventIndex, code, down);
        commitDeviceEvent(d);
    }
}

#ifdef QB64_WINDOWS

// Windows only reports one WM_KEYUP event when both shift keys are held. To
// fix that somewhat we're manually check on each window message and reporting
// the keyup ourselves.
static void checkShiftKeys() {
    static int rightShift = 0, leftShift = 0;

    // GetAsyncKeyState() indicates the key state in the most significant bit
    if (rightShift != !!(GetAsyncKeyState(VK_RSHIFT) & 0x8000)) {
        rightShift = !rightShift;
        reportKeyState(0x36, rightShift);
    }

    if (leftShift != !!(GetAsyncKeyState(VK_LSHIFT) & 0x8000)) {
        leftShift = !leftShift;
        reportKeyState(0x2A, leftShift);
    }
}

extern "C" LRESULT qb64_os_event_windows(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int *qb64_os_event_info) {
    if (*qb64_os_event_info == OS_EVENT_PRE_PROCESSING) {
        if (func__hasfocus())
            checkShiftKeys();

        if (uMsg == WM_KEYDOWN || uMsg == WM_SYSKEYDOWN) {
            if (device_last) { // core devices required?
                /*
                    16-23        The scan code. The value depends on the OEM.
                    24        Indicates whether the key is an extended key, such as the right-hand ALT and CTRL keys that appear on an enhanced 101- or 102-key
                   keyboard. The value is 1 if it is an extended key; otherwise, it is 0.
                */
                uint32_t code = (lParam >> 16) & 0x1FF;

                reportKeyState(code, 1);
            }
        }

        if (uMsg == WM_KEYUP || uMsg == WM_SYSKEYUP) {
            if (device_last) { // core devices required?
                uint32_t code = (lParam >> 16) & 0x1FF;

                reportKeyState(code, 0);
            }
        }
    }
    if (*qb64_os_event_info == OS_EVENT_POST_PROCESSING) {
    }
    return 0;
}
#endif

#if defined(QB64_LINUX) && defined(QB64_GUI)
extern "C" void qb64_os_event_linux(XEvent *event, Display *display, int *qb64_os_event_info) {
    if (*qb64_os_event_info == OS_EVENT_PRE_PROCESSING) {

        if (X11_display == NULL) {
            X11_display = display;
            X11_window = event->xexpose.window;
        }
    }

    if (*qb64_os_event_info == OS_EVENT_POST_PROCESSING) {
        switch (event->type) {
        case FocusIn:
            window_focused = -1;
            break;

        case FocusOut:
            window_focused = 0;
            // Iterate over all modifiers
            for (uint32 key = VK + QBVK_RSHIFT; key <= VK + QBVK_MODE; key++) {
                if (keyheld(key))
                    keyup(key);
            }
            break;
        }
    }
}
#endif

void qb64_custom_event_relative_mouse_movement(int deltaX, int deltaY) {
    mouse_message_queue_struct *queue = &mouse_message_queue;
    // message #1
    int32_t i = queue->last + 1;
    if (i > queue->lastIndex)
        i = 0;
    if (i == queue->current) {
        int32_t nextIndex = queue->last + 1;
        if (nextIndex > queue->lastIndex)
            nextIndex = 0;
        queue->current = nextIndex;
    }
    queue->queue[i].x = queue->queue[queue->last].x;
    queue->queue[i].y = queue->queue[queue->last].y;
    queue->queue[i].movementx = deltaX;
    queue->queue[i].movementy = deltaY;
    queue->queue[i].buttons = queue->queue[queue->last].buttons;
    queue->last = i;
    // message #2 (clears movement values to avoid confusion)
    i = queue->last + 1;
    if (i > queue->lastIndex)
        i = 0;
    if (i == queue->current) {
        int32_t nextIndex = queue->last + 1;
        if (nextIndex > queue->lastIndex)
            nextIndex = 0;
        queue->current = nextIndex;
    }
    queue->queue[i].x = queue->queue[queue->last].x;
    queue->queue[i].y = queue->queue[queue->last].y;
    queue->queue[i].movementx = 0;
    queue->queue[i].movementy = 0;
    queue->queue[i].buttons = queue->queue[queue->last].buttons;
    queue->last = i;
}

extern "C" int qb64_custom_event(int event, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, void *p1, void *p2) {
    if (event == QB64_EVENT_CLOSE) {
        exit_value |= 1;
        return 0;
    } // close

    if (event == QB64_EVENT_KEY) {
        if (v1 == VK + QBVK_PAUSE) {
            if (v2 > 0)
                keydown_vk(v1);
            else
                keyup_vk(v1);
            return 0;
        }
        if (v1 == VK + QBVK_BREAK) {
            if (v2 > 0)
                keydown_vk(v1);
            else
                keyup_vk(v1);
            return 0;
        }
        return -1;
    } // key

    // qb64_custom_event(QB64_EVENT_RELATIVE_MOUSE_MOVEMENT,xPosRelative,yPosRelative,0,0,0,0,0,0,NULL,NULL);
    if (event == QB64_EVENT_RELATIVE_MOUSE_MOVEMENT) {
        qb64_custom_event_relative_mouse_movement(v1, v2);
        return 0;
    } // QB64_EVENT_RELATIVE_MOUSE_MOVEMENT

    if (event == QB64_EVENT_FILE_DROP) {
#ifdef QB64_WINDOWS
        if (totalDroppedFiles > 0)
            sub__finishdrop();

        hdrop = (HDROP)p1;
        totalDroppedFiles = DragQueryFile(hdrop, -1, NULL, 0);
#endif
        return 0;
    }

    return -1; // Unknown command (use for debugging purposes only)
}

// func__capslock, func__scrolllock, func__numlock moved to keyboard.cpp
// sub__capslock, sub__scrolllock, sub__numlock moved to keyboard.cpp

// sub__consolefont, sub__console_cursor, func__getconsoleinput, func__cinp moved to console.cpp
