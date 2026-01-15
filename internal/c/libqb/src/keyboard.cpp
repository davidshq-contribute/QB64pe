//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Keyboard Module
//  Lock key functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#include "keyboard.h"

#ifdef QB64_WINDOWS
#include <windows.h>
#endif

#ifdef QB64_WINDOWS
// Helper function to toggle a lock key
static void toggle_lock_key(int32_t key_code) {
    keybd_event(key_code, 0x45, 1, 0);
    keybd_event(key_code, 0x45, 3, 0);
}
#endif

int32_t func__capslock() {
#ifdef QB64_WINDOWS
    return -GetKeyState(VK_CAPITAL);
#endif
    return 0;
}

int32_t func__scrolllock() {
#ifdef QB64_WINDOWS
    return -GetKeyState(VK_SCROLL);
#endif
    return 0;
}

int32_t func__numlock() {
#ifdef QB64_WINDOWS
    return -GetKeyState(VK_NUMLOCK);
#endif
    return 0;
}

void sub__capslock(int32_t options) {
#ifdef QB64_WINDOWS
    // VK_CAPITAL
    int32_t currentState = func__capslock();
    switch (options) {
    case 1: // ON
        if (currentState == -1)
            return;
        break;
    case 2: // OFF
        if (currentState == 0)
            return;
        break;
    }
    // _TOGGLE:
    toggle_lock_key(VK_CAPITAL);
#else
    (void)options; // Suppress unused parameter warning
#endif
}

void sub__scrolllock(int32_t options) {
#ifdef QB64_WINDOWS
    // VK_SCROLL
    int32_t currentState = func__scrolllock();
    switch (options) {
    case 1: // ON
        if (currentState == -1)
            return;
        break;
    case 2: // OFF
        if (currentState == 0)
            return;
        break;
    }
    // _TOGGLE:
    toggle_lock_key(VK_SCROLL);
#else
    (void)options; // Suppress unused parameter warning
#endif
}

void sub__numlock(int32_t options) {
#ifdef QB64_WINDOWS
    // VK_NUMLOCK
    int32_t currentState = func__numlock();
    switch (options) {
    case 1: // ON
        if (currentState == -1)
            return;
        break;
    case 2: // OFF
        if (currentState == 0)
            return;
        break;
    }
    // _TOGGLE:
    toggle_lock_key(VK_NUMLOCK);
#else
    (void)options; // Suppress unused parameter warning
#endif
}
