//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Window Module
//  Window query and control functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"
#include "window.h"
#include "glut-thread.h"
#include "qbs.h"

#ifdef QB64_WINDOWS
#include <windows.h>
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
