//----------------------------------------------------------------------------------------------------------------------
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

/**
 * Enables automatic display updates.
 * 
 * When enabled, the display will automatically refresh after drawing operations
 * without requiring explicit calls to _DISPLAY.
 */
void sub__autodisplay() {
    autodisplay = 1;
}

/**
 * Manually updates the display and disables automatic display updates.
 * 
 * Forces an immediate screen refresh. If autodisplay is enabled, it will be
 * disabled after this call completes. Does nothing if the screen is hidden.
 */
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

/**
 * Sets the fullscreen mode and scaling method.
 * 
 * Controls how the application window is displayed in fullscreen mode.
 * Options include: _OFF (windowed), _STRETCH (stretch to fit), _SQUAREPIXELS (maintain aspect),
 * or OFF (windowed). Optionally enables smooth scaling.
 * 
 * @param method Fullscreen mode: 0=default, 1=_OFF, 2=_STRETCH, 3=_SQUAREPIXELS, 4=OFF
 * @param passed Bit flags: bit 0 = _SMOOTH option provided
 */
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

/**
 * Sets which fullscreen modes and scaling options are allowed.
 * 
 * Restricts which fullscreen modes and scaling methods can be used.
 * Options include: _STRETCH, _SQUAREPIXELS, _OFF, _ALL, or OFF for both mode and smooth.
 * 
 * @param method Allowed fullscreen mode: 1=_STRETCH, 2=_SQUAREPIXELS, 3=_OFF, 4=_ALL, 5=OFF
 * @param smooth Allowed smooth scaling: 1=_SMOOTH, 2=_OFF, 3=_ALL, 4=OFF
 */
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

/**
 * Gets the current fullscreen mode.
 * 
 * Returns the active fullscreen mode: 0=windowed, 1=stretch, 2=square pixels.
 * If a mode change is pending, returns the pending mode instead.
 * 
 * @return Current fullscreen mode (0=windowed, 1=stretch, 2=square pixels)
 */
int32 func__fullscreen() {
    static int32 x;
    x = full_screen_set;
    if (x != -1)
        return x;
    return full_screen;
}

/**
 * Gets whether smooth scaling is enabled in fullscreen mode.
 * 
 * @return -1 if smooth scaling is enabled, 0 if disabled
 */
int32 func__fullscreensmooth() {
    return -fullscreen_smooth;
}

//----------------------------------------------------------------------------------------------------------------------
// Resize handling
//----------------------------------------------------------------------------------------------------------------------

/**
 * Enables or disables window resizing and sets resize behavior.
 * 
 * Controls whether the window can be resized and how content is scaled when resized.
 * Options include enabling/disabling resize snapback and setting auto-scaling mode.
 * 
 * @param on_off Resize control: 1=enable, 2=disable (snapback), 0=no change
 * @param stretch_smooth Auto-scaling mode: 1=stretch, 2=smooth, 0=disable
 */
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

/**
 * Checks if a window resize event has occurred.
 * 
 * Returns -1 if a resize event occurred (and clears the event flag),
 * 0 if no resize event or if resizing is disabled.
 * 
 * @return -1 if resize event occurred, 0 otherwise
 */
int32 func__resize() {
    if (resize_snapback)
        return 0; // resize must be enabled
    if (resize_event) {
        resize_event = 0;
        return -1;
    }
    return 0;
}

/**
 * Gets the width from the most recent resize event.
 * 
 * @return Width in pixels of the resized window
 */
int32 func__resizewidth() {
    return resize_event_x;
}

/**
 * Gets the height from the most recent resize event.
 * 
 * @return Height in pixels of the resized window
 */
int32 func__resizeheight() {
    return resize_event_y;
}

//----------------------------------------------------------------------------------------------------------------------
// Scaled dimensions
//----------------------------------------------------------------------------------------------------------------------

/**
 * Gets the scaled width of the screen.
 * 
 * Returns the width of the screen after applying any scaling transformations.
 * 
 * @return Scaled screen width in pixels
 */
int32 func__scaledwidth() {
    return environment_2d__screen_scaled_width;
}

/**
 * Gets the scaled height of the screen.
 * 
 * Returns the height of the screen after applying any scaling transformations.
 * 
 * @return Scaled screen height in pixels
 */
int32 func__scaledheight() {
    return environment_2d__screen_scaled_height;
}

//----------------------------------------------------------------------------------------------------------------------
// Screen position
//----------------------------------------------------------------------------------------------------------------------

/**
 * Gets the X coordinate of the screen window on the desktop.
 * 
 * Returns the horizontal position of the application window relative to the desktop.
 * On Windows, accounts for window border width. Returns 0 on unsupported platforms.
 * 
 * @return X coordinate of the window in pixels, or 0 if not supported
 */
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

/**
 * Gets the Y coordinate of the screen window on the desktop.
 * 
 * Returns the vertical position of the application window relative to the desktop.
 * On Windows, accounts for window border and header height. Returns 0 on unsupported platforms.
 * 
 * @return Y coordinate of the window in pixels, or 0 if not supported
 */
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
