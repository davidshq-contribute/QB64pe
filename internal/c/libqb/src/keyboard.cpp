//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Keyboard Module
//  Keyboard functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "keyboard.h"
#include "error_handle.h"
#include "libqb_state.h"

#include <stdint.h>

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

// ============================================================================
// KEYBOARD INPUT FUNCTIONS
// ============================================================================

int32_t func__keyhit() {
    // keyhit cyclic buffer:
    //   int64 keyhit[8192];
    //   keyhit specific internal flags: (stored in high 32-bits)
    //     &4294967296->numpad was used
    //   int32 keyhit_nextfree=0;
    //   int32 keyhit_next=0;
    //   note: if full, the oldest message is discarded to make way for the new message
    int64_t value = libqb_keyhit_pop();
    return static_cast<int32_t>(value);  // Return low 32-bits (key code)
}

int32_t func__keydown(int32_t x) {
    if (x <= 0) {
        error(5);
        return 0;
    }
    if (libqb_keyheld(static_cast<uint32_t>(x)))
        return -1;
    return 0;
}

void sub__mapunicode(int32_t unicode_code, int32_t ascii_code) {
    if (is_error_pending())
        return;
    if ((unicode_code < 0) || (unicode_code > 65535)) {
        error(5);
        return;
    }
    if ((ascii_code < 0) || (ascii_code > 255)) {
        error(5);
        return;
    }
    libqb_set_codepage_mapping(ascii_code, static_cast<uint16_t>(unicode_code));
}

int32_t func__mapunicode(int32_t ascii_code) {
    if (is_error_pending())
        return 0;
    if ((ascii_code < 0) || (ascii_code > 255)) {
        error(5);
        return 0;
    }
    return libqb_get_codepage_mapping(ascii_code);
}
