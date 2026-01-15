//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Mouse Module
//  Mouse input functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#include "mouse.h"

#include "error_handle.h"
#include "glut-thread.h"
#include "graphics.h"
#include "qbs.h"
#include "rounding.h"
#include "../../os.h"

#include <cmath>

// Mouse message structure (must match libqb.cpp)
struct mouse_message {
    int16 x;
    int16 y;
    uint32 buttons;
    int16 movementx;
    int16 movementy;
};

// Mouse message queue structure (must match libqb.cpp)
struct mouse_message_queue_struct {
    mouse_message *queue;
    int32 lastIndex;
    int32 current;
    int32 last;
};

// External globals from libqb.cpp
extern mouse_message_queue_struct mouse_message_queue;
extern int32 mouse_hiddden;
extern int mouse_cursor_style;
extern img_struct *display_page;
extern img_struct *read_page;
extern int32 *fontwidth;
extern int32 *fontheight;
extern int32 environment_2d__screen_width;
extern int32 environment_2d__screen_height;
extern int32 environment_2d__screen_x1;
extern int32 environment_2d__screen_y1;
extern float environment_2d__screen_x_scale;
extern float environment_2d__screen_y_scale;

#ifdef QB64_WINDOWS
extern int32 consolemousex;
extern int32 consolemousey;
extern int32 consolebutton;
#endif

void sub__mousehide() {
#ifdef QB64_GUI
#    ifdef QB64_GLUT
    OPTIONAL_GLUT();
    libqb_glut_set_cursor(GLUT_CURSOR_NONE);
#    endif
#endif
    mouse_hiddden = -1;
}

void sub__mouseshow(qbs *style, int32 passed) {
    if (is_error_pending())
        return;

#ifdef QB64_GLUT
    OPTIONAL_GLUT();

    static qbs *str = NULL;
    if (str == NULL)
        str = qbs_new(0, 0);
    if (passed) {
        qbs_set(str, qbs_ucase(style));
        if (qbs_equal(str, qbs_new_txt("DEFAULT"))) {
            mouse_cursor_style = GLUT_CURSOR_LEFT_ARROW;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("LINK"))) {
            mouse_cursor_style = GLUT_CURSOR_INFO;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("TEXT"))) {
            mouse_cursor_style = GLUT_CURSOR_TEXT;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("CROSSHAIR"))) {
            mouse_cursor_style = GLUT_CURSOR_CROSSHAIR;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("VERTICAL"))) {
            mouse_cursor_style = GLUT_CURSOR_UP_DOWN;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("HORIZONTAL"))) {
            mouse_cursor_style = GLUT_CURSOR_LEFT_RIGHT;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("TOPLEFT_BOTTOMRIGHT"))) {
            mouse_cursor_style = GLUT_CURSOR_TOP_LEFT_CORNER;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("TOPRIGHT_BOTTOMLEFT"))) {
            mouse_cursor_style = GLUT_CURSOR_TOP_RIGHT_CORNER;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("WAIT"))) {
            mouse_cursor_style = GLUT_CURSOR_WAIT;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("HELP"))) {
            mouse_cursor_style = GLUT_CURSOR_HELP;
            goto cursor_valid;
        }
        if (qbs_equal(str, qbs_new_txt("CYCLE")) || qbs_equal(str, qbs_new_txt("MOVE"))) {
            mouse_cursor_style = GLUT_CURSOR_CYCLE;
            goto cursor_valid;
        }
        error(5);
        return;
    }
cursor_valid:

    libqb_glut_set_cursor(mouse_cursor_style);
#endif
    mouse_hiddden = 0;
}

int32_t func__mousehidden() {
    return mouse_hiddden;
}

float func__mousemovementx() {
    return mouse_message_queue.queue[mouse_message_queue.current].movementx;
}

float func__mousemovementy() {
    return mouse_message_queue.queue[mouse_message_queue.current].movementy;
}

void sub__mousemove(float x, float y) {
#ifdef QB64_GLUT
    NEEDS_GLUT();

    int32 x2, y2, sx, sy;
    if (display_page->text) {
        sx = fontwidth[display_page->font] * display_page->width;
        sy = fontheight[display_page->font] * display_page->height;
        if (x < 0.5)
            goto error;
        if (y < 0.5)
            goto error;
        if (x > ((float)display_page->width) + 0.5)
            goto error;
        if (y > ((float)display_page->height) + 0.5)
            goto error;
        x -= 0.5;
        y -= 0.5;
        x = x * (float)fontwidth[display_page->font];
        y = y * (float)fontheight[display_page->font];
        x2 = qbr_float_to_long(x);
        y2 = qbr_float_to_long(y);
        if (x2 < 0)
            x2 = 0;
        if (y2 < 0)
            y2 = 0;
        if (x2 > sx - 1)
            x2 = sx - 1;
        if (y2 > sy - 1)
            y2 = sy - 1;
    } else {
        sx = display_page->width;
        sy = display_page->height;
        x2 = qbr_float_to_long(x);
        y2 = qbr_float_to_long(y);
        if (x2 < 0)
            goto error;
        if (y2 < 0)
            goto error;
        if (x2 > sx - 1)
            goto error;
        if (y2 > sy - 1)
            goto error;
    }

    // x2,y2 are pixel co-ordinates
    // adjust for fullscreen position as necessary:
    x2 *= environment_2d__screen_x_scale;
    y2 *= environment_2d__screen_y_scale;
    x2 += environment_2d__screen_x1;
    y2 += environment_2d__screen_y1;

    libqb_glut_warp_pointer(x2, y2);
    return;

error:
    error(5);
#endif
}

float func__mousex() {

    static int32 x, x2;
    static float f;

#ifdef QB64_WINDOWS
    if (read_page->console) {
        return consolemousex;
    }
#endif

    x = mouse_message_queue.queue[mouse_message_queue.current].x;

    // calculate pixel offset of mouse within SCREEN using environment variables
    x -= environment_2d__screen_x1;
    x = qbr_float_to_long((((float)x + 0.5f) / environment_2d__screen_x_scale) - 0.5f);
    if (x < 0)
        x = 0;
    if (x >= environment_2d__screen_width)
        x = environment_2d__screen_width - 1;

    // restrict range to the current display page's range to avoid causing errors
    x2 = display_page->width;
    if (display_page->text)
        x2 *= fontwidth[display_page->font];
    if (x >= x2)
        x = x2 - 1;

    if (display_page->text) {
        f = x;
        x2 = fontwidth[display_page->font];
        f = f / (float)x2 + 0.5f;
        x2 = qbr_float_to_long(f);
        if (x2 > x)
            f -= 0.001f;
        if (x2 < x)
            f += 0.001f;
        return std::floor(f + 0.5);
    }

    return x;
}

float func__mousey() {

    static int32 y, y2;
    static float f;

#ifdef QB64_WINDOWS
    if (read_page->console) {
        return consolemousey;
    }
#endif

    y = mouse_message_queue.queue[mouse_message_queue.current].y;

    // calculate pixel offset of mouse within SCREEN using environment variables
    y -= environment_2d__screen_y1;
    y = qbr_float_to_long((((float)y + 0.5f) / environment_2d__screen_y_scale) - 0.5f);
    if (y < 0)
        y = 0;
    if (y >= environment_2d__screen_height)
        y = environment_2d__screen_height - 1;

    // restrict range to the current display page's range to avoid causing errors
    y2 = display_page->height;
    if (display_page->text)
        y2 *= fontheight[display_page->font];
    if (y >= y2)
        y = y2 - 1;

    if (display_page->text) {
        f = y;
        y2 = fontheight[display_page->font];
        f = f / (float)y2 + 0.5f;
        y2 = qbr_float_to_long(f);
        if (y2 > y)
            f -= 0.001f;
        if (y2 < y)
            f += 0.001f;
        return std::floor(f + 0.5);
    }

    return y;
}

int32 func__mouseinput() {
    if (mouse_message_queue.current == mouse_message_queue.last)
        return 0;
    int32 newIndex = mouse_message_queue.current + 1;
    if (newIndex > mouse_message_queue.lastIndex)
        newIndex = 0;
    mouse_message_queue.current = newIndex;
    return -1;
}

int32 func__mousebutton(int32 i) {
    if (i < 1) {
        error(5);
        return 0;
    }
#ifdef QB64_WINDOWS
    if (read_page->console) { // console may support up to 5 mouse buttons according to the documentation.
        if (i == 1)
            return consolebutton & 1;
        if (i == 2)
            return consolebutton & 2;
        if (i == 3)
            return consolebutton & 4;
        if (i == 4)
            return consolebutton & 8;
        if (i == 5)
            return consolebutton & 16;
        return 0;
    }
#endif

    if (i > 3)
        return 0; // current SDL only supports 3 mouse buttons!
    // swap indexes 2&3
    if (i == 2) {
        i = 3;
    } else {
        if (i == 3)
            i = 2;
    }
    if (mouse_message_queue.queue[mouse_message_queue.current].buttons & (1 << (i - 1)))
        return -1;
    return 0;
}

int32 func__mousewheel() {
    static uint32 x;

#ifdef QB64_WINDOWS
    if (read_page->console) {
        if (consolebutton < -0x100)
            return -1;
        if (consolebutton > 0x100)
            return 1;
        return 0;
    }
#endif

    x = mouse_message_queue.queue[mouse_message_queue.current].buttons;
    if ((x & (8 + 16)) == (8 + 16))
        return 0; // cancelled out change
    if (x & 8)
        return -1; // up
    if (x & 16)
        return 1; // down
    return 0;     // no change
}
