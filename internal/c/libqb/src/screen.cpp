//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Screen Management Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "../../os.h"

#ifdef QB64_WINDOWS
#include <windows.h>
#else
#include "datetime.h"  // For Sleep() on non-Windows
#endif

#include "screen.h"

#if defined(QB64_GUI) && defined(QB64_GLUT)
#ifdef QB64_MACOSX
    #include <GLUT/glut.h>
#else
    #include <GL/freeglut.h>
#endif
#include "error_handle.h"
#include "glut-thread.h"
#endif

// External references from libqb.cpp
extern int32 screen_hide;
extern int32 autodisplay;
extern void display();

extern int32 full_screen;
extern int32 full_screen_set;
extern int32 fullscreen_smooth;
extern int32 fullscreen_allowedmode;
extern int32 fullscreen_allowedsmooth;
extern int32 force_display_update;

extern int32 resize_snapback;
extern int32 resize_auto;
extern int32 resize_event;
extern int32 resize_event_x;
extern int32 resize_event_y;

extern int32 environment_2d__screen_scaled_width;
extern int32 environment_2d__screen_scaled_height;

//----------------------------------------------------------------------------------------------------------------------
// Display control
//----------------------------------------------------------------------------------------------------------------------

void sub__autodisplay() {
    autodisplay = 1;
}

void sub__display() {
    if (screen_hide)
        return;

    // disable autodisplay (if enabled)
    if (autodisplay) {
        autodisplay = -1; // toggle request
        while (autodisplay)
            Sleep(1);
        return; // note: autodisplay is set to 0 after display() has been called so a second call to display() is unnecessary
    }
    display();
}

//----------------------------------------------------------------------------------------------------------------------
// Fullscreen control
//----------------------------------------------------------------------------------------------------------------------

void sub__fullscreen(int32 method, int32 passed) {
    // ref: "[{_OFF|_STRETCH|_SQUAREPIXELS|OFF}][, _SMOOTH]"
    //          1      2           3        4         1
    int32 x;
    if (method == 0)
        x = 1;
    if (method == 1 || method == 4)
        x = 0;
    if (method == 2)
        x = 1;
    if (method == 3)
        x = 2;
    if (passed & 1)
        fullscreen_smooth = 1;
    else
        fullscreen_smooth = 0;
    if (full_screen != x)
        full_screen_set = x;
    force_display_update = 1;
}

void sub__allowfullscreen(int32 method, int32 smooth) {
    // ref: "[{_STRETCH|_SQUAREPIXELS|_OFF|_ALL|OFF}][, _SMOOTH|_OFF|_ALL|OFF]"
    //            1          2         3    4   5         1      2    3   4

    fullscreen_allowedmode = method;
    if (method == 3 || method == 5)
        fullscreen_allowedmode = -1;
    if (method == 4 || method == 0)
        fullscreen_allowedmode = 0;

    fullscreen_allowedsmooth = smooth;
    if (smooth == 2 || smooth == 4)
        fullscreen_allowedsmooth = -1;
    if (smooth == 3 || smooth == 0)
        fullscreen_allowedsmooth = 0;
}

int32 func__fullscreen() {
    static int32 x;
    x = full_screen_set;
    if (x != -1)
        return x;
    return full_screen;
}

int32 func__fullscreensmooth() {
    return -fullscreen_smooth;
}

//----------------------------------------------------------------------------------------------------------------------
// Resize handling
//----------------------------------------------------------------------------------------------------------------------

void sub__resize(int32 on_off, int32 stretch_smooth) {
    if (on_off == 1)
        resize_snapback = 0;
    if (on_off == 2)
        resize_snapback = 1;
    // no change if omitted

    if (stretch_smooth) {
        resize_auto = stretch_smooth;
    } else {
        resize_auto = 0; // revert if omitted
    }
}

int32 func__resize() {
    if (resize_snapback)
        return 0; // resize must be enabled
    if (resize_event) {
        resize_event = 0;
        return -1;
    }
    return 0;
}

int32 func__resizewidth() {
    return resize_event_x;
}

int32 func__resizeheight() {
    return resize_event_y;
}

//----------------------------------------------------------------------------------------------------------------------
// Scaled dimensions
//----------------------------------------------------------------------------------------------------------------------

int32 func__scaledwidth() {
    return environment_2d__screen_scaled_width;
}

int32 func__scaledheight() {
    return environment_2d__screen_scaled_height;
}

//----------------------------------------------------------------------------------------------------------------------
// Screen position
//----------------------------------------------------------------------------------------------------------------------

int32 func__screenx() {
#if defined(QB64_GUI) && defined(QB64_WINDOWS) && defined(QB64_GLUT)
    NEEDS_GLUT(0);
    return libqb_glut_get(GLUT_WINDOW_X) - libqb_glut_get(GLUT_WINDOW_BORDER_WIDTH);
#elif defined(QB64_GUI) && defined(QB64_MACOSX) && defined(QB64_GLUT)
    NEEDS_GLUT(0);
    return libqb_glut_get(GLUT_WINDOW_X);
#endif
    return 0; // if not windows then return 0
}

int32 func__screeny() {
#if defined(QB64_GUI) && defined(QB64_WINDOWS) && defined(QB64_GLUT)
    NEEDS_GLUT(0);
    return libqb_glut_get(GLUT_WINDOW_Y) - libqb_glut_get(GLUT_WINDOW_BORDER_WIDTH) - libqb_glut_get(GLUT_WINDOW_HEADER_HEIGHT);
#elif defined(QB64_GUI) && defined(QB64_MACOSX) && defined(QB64_GLUT)
    NEEDS_GLUT(0);
    return libqb_glut_get(GLUT_WINDOW_Y);
#endif
    return 0; // if not windows then return 0
}
