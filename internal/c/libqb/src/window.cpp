//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Window Module
//  Window query and control functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"
#include "window.h"
#include "error_handle.h"
#include "glut-thread.h"
#include "libqb_state.h"
#include "qbs.h"

#ifdef QB64_WINDOWS
#include <windows.h>
#include <shellapi.h>
#endif

#if defined(QB64_GUI) && defined(QB64_GLUT)
#ifdef QB64_MACOSX
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#endif

// External globals from libqb.cpp
#ifdef QB64_WINDOWS
extern HWND window_handle;
#endif
extern uint8_t *window_title;
extern int32_t window_focused;

//----------------------------------------------------------------------------------------------------------------------
// Window handle
//----------------------------------------------------------------------------------------------------------------------

int64_t func__handle() {
#ifdef QB64_WINDOWS
#    ifdef DEPENDENCY_CONSOLE_ONLY
    if (!window_handle) {
        char pszConsoleTitle[1024];
        GetConsoleTitle(pszConsoleTitle, 1024);
        window_handle = FindWindow(NULL, pszConsoleTitle);
    }
    return (intptr_t)window_handle;
#    endif

    OPTIONAL_GLUT(0);
    return (intptr_t)window_handle;
#else
    return 0;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
// Window title
//----------------------------------------------------------------------------------------------------------------------

qbs *func__title() {
    if (!window_title) {
        return qbs_new_txt("");
    } else {
        return qbs_new_txt((char *)window_title);
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Window focus
//----------------------------------------------------------------------------------------------------------------------

void set_foreground_window(intptr_t i) {
#ifdef QB64_WINDOWS
    SetForegroundWindow((HWND)i);
#else
    (void)i; // Suppress unused parameter warning
#endif
}

int32_t func__hasfocus() {
#ifdef QB64_GUI
#    ifdef QB64_WINDOWS
    return -((HWND)func__handle() == GetForegroundWindow());
#    elif defined(QB64_LINUX)
    return window_focused;
#    endif
#endif
    return -1;
}

//----------------------------------------------------------------------------------------------------------------------
// Screen/desktop dimensions
//----------------------------------------------------------------------------------------------------------------------

/**
 * Gets the width of the screen/desktop in pixels.
 *
 * @return Screen width in pixels, or 0 if not available
 */
int32_t func_screenwidth() {
#ifdef QB64_WINDOWS
    return GetSystemMetrics(SM_CXSCREEN);
#else
#    ifdef QB64_GLUT
    OPTIONAL_GLUT(0);
    return libqb_glut_get(GLUT_SCREEN_WIDTH);
#    else
    return 0;
#    endif
#endif
}

/**
 * Gets the height of the screen/desktop in pixels.
 *
 * @return Screen height in pixels, or 0 if not available
 */
int32_t func_screenheight() {
#ifdef QB64_WINDOWS
    return GetSystemMetrics(SM_CYSCREEN);
#else
#    ifdef QB64_GLUT
    OPTIONAL_GLUT(0);
    return libqb_glut_get(GLUT_SCREEN_HEIGHT);
#    else
    return 0;
#    endif
#endif
}

//----------------------------------------------------------------------------------------------------------------------
// Window state
//----------------------------------------------------------------------------------------------------------------------

/**
 * Iconifies (minimizes) the window.
 */
void sub_screenicon() {
#ifdef QB64_GLUT
    NEEDS_GLUT();
    libqb_glut_iconify_window();
#endif
}

/**
 * Checks if the QB64 window exists.
 *
 * @return -1 (true) if window exists, 0 (false) otherwise
 */
int32_t func_windowexists() {
#ifdef QB64_GLUT
    return libqb_is_glut_up();
#else
    return -1;
#endif
}

/**
 * Checks if the window is currently iconified (minimized).
 *
 * @return -1 (true) if iconified, 0 (false) otherwise
 */
int32_t func_screenicon() {
#ifdef QB64_GLUT
#    ifdef QB64_WINDOWS
    HWND win = (HWND)func__handle();
    if (!win) {
        return 0;
    }
    return -IsIconic(win);
#    else
    /*
        Linux code not compiling for now
        #include <X11/X.h>
        #include <X11/Xlib.h>
        extern Display *X11_display;
        extern Window X11_window;
        extern int32 screen_hide;
        XWindowAttributes attribs;
        while (!(X11_display && X11_window));
        XGetWindowAttributes(X11_display, X11_window, &attribs);
        if (attribs.map_state == IsUnmapped) return -1;
        return 0;
    #endif */
    return 0; // if we get here and haven't exited already, we failed somewhere along the way.
#    endif
#else
    return 0;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
// Window position
//----------------------------------------------------------------------------------------------------------------------

/**
 * Moves the window to a specified position on the desktop.
 *
 * @param x X coordinate (or special value for centering)
 * @param y Y coordinate
 * @param passed Bit flags: bit 0 = x passed, bit 1 = y passed (3 = _MIDDLE)
 */
void sub__screenmove(int32_t x, int32_t y, int32_t passed) {
    if (is_error_pending())
        return;
    if (!passed)
        goto error;
    if (passed == 3)
        goto error;
    if (libqb_get_full_screen())
        return;

#if defined(QB64_GUI) && defined(QB64_GLUT)
    NEEDS_GLUT();

    if (passed == 2) {
        libqb_glut_position_window(x, y);
    } else {
        int32_t SW = -1, SH, WW, WH;
        while (SW == -1) {
            SW = libqb_glut_get(GLUT_SCREEN_WIDTH);
        }
        SH = libqb_glut_get(GLUT_SCREEN_HEIGHT);
        WW = libqb_glut_get(GLUT_WINDOW_WIDTH);
        WH = libqb_glut_get(GLUT_WINDOW_HEIGHT);
        x = (SW - WW) / 2;
        y = (SH - WH) / 2;
        libqb_glut_position_window(x, y);
    }
#endif

    return;

error:
    error(5);
}

//----------------------------------------------------------------------------------------------------------------------
// File drop functions
//----------------------------------------------------------------------------------------------------------------------

/**
 * Enables or disables file drop acceptance.
 *
 * @param on_off 1 or NULL to enable, 2 to disable
 */
void sub__filedrop(int32_t on_off) {
#ifdef QB64_WINDOWS
    HWND win = (HWND)func__handle();
    if (!win)
        return;

    if ((on_off == 0) || (on_off == 1)) {
        DragAcceptFiles(win, TRUE);
        libqb_set_accept_filedrop(-1);
    }
    if (on_off == 2) {
        DragAcceptFiles(win, FALSE);
        libqb_set_accept_filedrop(0);
    }
#else
    (void)on_off;
#endif
}

/**
 * Gets the current file drop acceptance state.
 *
 * @return -1 if file drop is enabled, 0 if disabled
 */
int32_t func__filedrop() {
    return libqb_get_accept_filedrop();
}

/**
 * Finishes processing dropped files and clears the drop list.
 */
void sub__finishdrop() {
#ifdef QB64_WINDOWS
    void* h = libqb_get_hdrop();
    if (h) {
        DragFinish(static_cast<HDROP>(h));
    }
    libqb_set_total_dropped_files(0);
#endif
}

/**
 * Gets the total number of dropped files.
 *
 * @return Number of dropped files
 */
int32_t func__totaldroppedfiles() {
#ifdef QB64_WINDOWS
    return libqb_get_total_dropped_files();
#endif
    return 0;
}

/**
 * Gets a dropped file by index.
 *
 * @param fileIndex 1-based file index (or sequential access if not passed)
 * @param passed 1 if fileIndex was provided, 0 for sequential access
 * @return QB64 string containing the file path, or empty string on error
 */
qbs *func__droppedfile(int32_t fileIndex, int32_t passed) {
#ifdef QB64_WINDOWS
    static int32_t index = -1;
    static char szNextFile[MAX_PATH];

    int32_t totalFiles = libqb_get_total_dropped_files();
    if (totalFiles > 0) {
        index++;
        if (passed)
            index = fileIndex - 1;
        if ((index > totalFiles - 1) || (index < 0)) {
            // out of bounds;
            // if reading _DROPPEDFILE$ sequentially (without an
            // index), hdrop is reset and the list is cleared.
            if (!passed)
                sub__finishdrop();
            index = -1;
            return qbs_new_txt("");
        }
        // fetch file[index] from hdrop:
        HDROP h = static_cast<HDROP>(libqb_get_hdrop());
        if (DragQueryFileA(h, index, szNextFile, MAX_PATH) > 0) {
            if ((!passed) && (index == totalFiles - 1)) {
                // last file read sequentially
                sub__finishdrop();
                index = -1;
            }
            return qbs_new_txt(szNextFile);
        } else {
            // error fetching file from hdrop;
            sub__finishdrop();
            index = -1;
        }
    } else {
        index = -1;
    }
#else
    (void)fileIndex;
    (void)passed;
#endif
    return qbs_new_txt("");
}
