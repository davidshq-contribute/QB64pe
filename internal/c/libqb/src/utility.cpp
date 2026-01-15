//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Utility Module
//  General utility functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "utility.h"
#include "error_handle.h"
#include "qbs.h"

#include <stdint.h>

// External function declarations (defined in libqb.cpp)
extern void qbs_print(qbs *str, int32_t finish_on_new_line);
extern void qbs_input(int32_t numvariables, uint8_t newline);
extern void *qbs_input_variableoffsets[257];
extern int32_t qbs_input_variabletypes[257];

//----------------------------------------------------------------------------------------------------------------------
// Module state
//----------------------------------------------------------------------------------------------------------------------

// Blink enable state (used by port I/O emulation as well)
int32_t H3C0_blink_enable = 1;

// Random number generator state
static uint32_t rnd_seed = 327680;
static uint32_t rnd_seed_first = 327680; // Note: must contain the same value as rnd_seed

// Frame rate control state
static double max_fps = 60; // 60 is the default
static int32_t auto_fps = 0;  // set to 1 to make QB64 auto-adjust fps based on load

// Control character state
// When set, control characters (like CHR$(7) bell) are printed literally instead of interpreted
static int32_t no_control_characters2 = 0;

//----------------------------------------------------------------------------------------------------------------------
// Blink control functions
//----------------------------------------------------------------------------------------------------------------------

void sub__blink(int32_t onoff) {
    if (onoff == 1)
        H3C0_blink_enable = 1;
    else
        H3C0_blink_enable = 0;
}

int32_t func__blink() {
    return -H3C0_blink_enable;
}

//----------------------------------------------------------------------------------------------------------------------
// Random number generation
//----------------------------------------------------------------------------------------------------------------------

void sub_randomize(double seed, int32_t passed) {
    if (is_error_pending())
        return;

    if (passed == 3) { // USING
        // Dim As Uinteger m = cptr(Uinteger Ptr, @n)[1]
        static uint32_t m;
        m = ((uint32_t *)&seed)[1];
        // m Xor= (m Shr 16)
        m ^= (m >> 16);
        // rnd_seed = (m And &hffff) Shl 8 Or (rnd_seed And &hff)
        rnd_seed = ((m & 0xffff) << 8) | (rnd_seed_first & 0xff); // Note: rnd_seed changed to rnd_seed_first
        return;
    }

    if (passed == 1) {
        // Dim As Uinteger m = cptr(Uinteger Ptr, @n)[1]
        static uint32_t m;
        m = ((uint32_t *)&seed)[1];
        // m Xor= (m Shr 16)
        m ^= (m >> 16);
        // rnd_seed = (m And &hffff) Shl 8 Or (rnd_seed And &hff)
        rnd_seed = ((m & 0xffff) << 8) | (rnd_seed & 0xff);
        return;
    }

    qbs_print(qbs_new_txt("Random-number seed (-32768 to 32767)? "), 0);
    static int16_t integerseed;
    qbs_input_variabletypes[1] = 16; // id.t=16 'a signed 16 bit integer
    qbs_input_variableoffsets[1] = &integerseed;
    qbs_input(1, 1);
    // rnd_seed = (m And &hffff) Shl 8 Or (rnd_seed And &hff) 'nb. same as above
    rnd_seed = ((integerseed & 0xffff) << 8) | (rnd_seed & 0xff);
    return;
}

float func_rnd(float n, int32_t passed) {
    if (is_error_pending())
        return 0;

    static uint32_t m;
    if (!passed)
        n = 1.0f;
    if (n != 0.0) {
        if (n < 0.0) {
            m = *((uint32_t *)&n);
            rnd_seed = (m & 0xFFFFFF) + ((m & 0xFF000000) >> 24);
        }
        rnd_seed = (rnd_seed * 16598013 + 12820163) & 0xFFFFFF;
    }
    return (double)rnd_seed / 0x1000000;
}

//----------------------------------------------------------------------------------------------------------------------
// Frame rate control
//----------------------------------------------------------------------------------------------------------------------

void sub__fps(double fps, int32_t passed) {
    // passed=1 means _AUTO
    // passed=2 means use fps
    if (is_error_pending())
        return;
    if (passed != 1 && passed != 2) {
        error(5);
        return;
    }
    if (passed == 1) {
        auto_fps = 1; //_AUTO
    }
    if (passed == 2) {
        if (fps < 1) {
            error(5);
            return;
        }
        if (fps > 200)
            fps = 200;
        max_fps = fps;
        auto_fps = 0;
    }
}

// Accessor functions for FPS state
double get_max_fps() {
    return max_fps;
}

int32_t get_auto_fps() {
    return auto_fps;
}

void set_auto_fps(int32_t value) {
    auto_fps = value;
}

// Reset RND state (called by RUN command to restore initial state)
void reset_rnd_state() {
    rnd_seed = 327680;
    rnd_seed_first = 327680;
}

//----------------------------------------------------------------------------------------------------------------------
// Control character mode
//----------------------------------------------------------------------------------------------------------------------

void sub__controlchr(int32_t onoff) {
    if (onoff == 2)
        no_control_characters2 = 1;
    else
        no_control_characters2 = 0;
}

int32_t func__controlchr() {
    return -no_control_characters2;
}

// Accessor for control character state (used by qbs_print in libqb.cpp)
int32_t get_control_characters_disabled() {
    return no_control_characters2;
}
