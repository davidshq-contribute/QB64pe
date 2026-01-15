//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Port I/O Module
//  Legacy port access functions (INP, OUT, WAIT)
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "port_io.h"

#include "datetime.h"
#include "error_handle.h"
#include "graphics.h"
#include "libqb_state.h"
#include "rounding.h"

#include <cstring>

#ifdef QB64_WINDOWS
#include <windows.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// Module state - VGA palette emulation
//----------------------------------------------------------------------------------------------------------------------

// Palette register indices for VGA emulation
static int32_t H3C7_palette_register_read_index = 0;
static int32_t H3C8_palette_register_index = 0;

// RGB component sequence counters (0=red, 1=green, 2=blue)
static int32_t H3C9_next = 0;      // write sequence
static int32_t H3C9_read_next = 0; // read sequence

// Flag set when an unsupported port is accessed
// Used by sub_wait to return immediately for unsupported ports
static int32_t unsupported_port_accessed = 0;

//----------------------------------------------------------------------------------------------------------------------
// External dependencies
//----------------------------------------------------------------------------------------------------------------------

// Blink enable state (from utility.cpp)
extern int32_t H3C0_blink_enable;

// Vertical retrace flags (set by display code in libqb.cpp)
extern int32_t vertical_retrace_in_progress;
extern int32_t vertical_retrace_happened;

// Keyboard scancode buffer (managed by keyboard input code in libqb.cpp)
extern uint8_t port60h_event[256];
extern int32_t port60h_events;

// Program termination flag
extern uint8_t stop_program;

//----------------------------------------------------------------------------------------------------------------------
// sub_out - Write byte to I/O port
//----------------------------------------------------------------------------------------------------------------------

void sub_out(int32_t port, int32_t data) {
    if (is_error_pending())
        return;

    unsupported_port_accessed = 0;
    port = port & 65535;
    data = data & 255;

    // VGA attribute controller - blink enable
    if (port == 0x3C0) {
        H3C0_blink_enable = data & (1 << 3);
        return;
    }

    // VGA palette read index
    if (port == 0x3C7) {
        H3C7_palette_register_read_index = data;
        H3C9_read_next = 0;
        return;
    }

    // VGA palette write index (0x3C8 = 968)
    if (port == 968) {
        H3C8_palette_register_index = data;
        H3C9_next = 0;
        return;
    }

    // VGA palette data (0x3C9 = 969)
    if (port == 969) {
        data = data & 63;
        img_struct *write_page = libqb_get_write_page();
        if (write_page && write_page->pal) {
            if (H3C9_next == 0) { // red
                write_page->pal[H3C8_palette_register_index] &= 0xFF00FFFF;
                write_page->pal[H3C8_palette_register_index] += (qbr((double)data * 4.063492f - 0.4999999f) << 16);
            }
            if (H3C9_next == 1) { // green
                write_page->pal[H3C8_palette_register_index] &= 0xFFFF00FF;
                write_page->pal[H3C8_palette_register_index] += (qbr((double)data * 4.063492f - 0.4999999f) << 8);
            }
            if (H3C9_next == 2) { // blue
                write_page->pal[H3C8_palette_register_index] &= 0xFFFFFF00;
                write_page->pal[H3C8_palette_register_index] += qbr((double)data * 4.063492f - 0.4999999f);
            }
        }
        H3C9_next = H3C9_next + 1;
        if (H3C9_next == 3) {
            H3C9_next = 0;
            H3C8_palette_register_index = H3C8_palette_register_index + 1;
            H3C8_palette_register_index &= 0xFF;
        }
        return;
    }

    unsupported_port_accessed = 1;
}

//----------------------------------------------------------------------------------------------------------------------
// func_inp - Read byte from I/O port
//----------------------------------------------------------------------------------------------------------------------

int32_t func_inp(int32_t port) {
    int32_t value;

    unsupported_port_accessed = 0;

    if ((port > 65535) || (port < -65536)) {
        error(6); // Overflow
        return 0;
    }
    port &= 0xFFFF;

    // VGA palette data read (0x3C9)
    if (port == 0x3C9) {
        img_struct *write_page = libqb_get_write_page();
        if (write_page && write_page->pal) {
            // Convert 0-255 value to 0-63 value
            if (H3C9_read_next == 0) { // red
                value = qbr_double_to_long((((double)((write_page->pal[H3C7_palette_register_read_index] >> 16) & 255)) / 3.984376 - 0.4999999f));
            }
            if (H3C9_read_next == 1) { // green
                value = qbr_double_to_long((((double)((write_page->pal[H3C7_palette_register_read_index] >> 8) & 255)) / 3.984376 - 0.4999999f));
            }
            if (H3C9_read_next == 2) { // blue
                value = qbr_double_to_long((((double)(write_page->pal[H3C7_palette_register_read_index] & 255)) / 3.984376 - 0.4999999f));
            }
            H3C9_read_next = H3C9_read_next + 1;
            if (H3C9_read_next == 3) {
                H3C9_read_next = 0;
                H3C7_palette_register_read_index = H3C7_palette_register_read_index + 1;
                H3C7_palette_register_read_index &= 0xFF;
            }
            return value;
        }
        return 0; // Non-palette modes
    }

    // VGA status register (0x3DA)
    // Bit 3: Vertical retrace in progress
    if (port == 0x3DA) {
        value = 0;
        if (vertical_retrace_happened || vertical_retrace_in_progress) {
            vertical_retrace_happened = 0;
            value |= 8;
        }
        return value;
    }

    // Keyboard scancode port (0x60)
    if (port == 0x60) {
        if (port60h_events) {
            value = port60h_event[0];
            if (port60h_events > 1)
                memmove(port60h_event, port60h_event + 1, 255);
            port60h_events--;
            return value;
        } else {
            return port60h_event[0];
        }
    }

    unsupported_port_accessed = 1;
    return 0; // Unknown port
}

//----------------------------------------------------------------------------------------------------------------------
// sub_wait - Wait for port condition
//----------------------------------------------------------------------------------------------------------------------

void sub_wait(int32_t port, int32_t andexpression, int32_t xorexpression, int32_t passed) {
    if (is_error_pending())
        return;

    // Algorithm:
    // 1. Read value from port
    // 2. value ^= xorexpression (if passed!)
    // 3. value &= andexpression
    // IMPORTANT: Wait returns immediately if given port is unsupported by QB64
    //            so program can continue

    int32_t value;

    // Error and range checking
    if ((port > 65535) || (port < -65536)) {
        error(6); // Overflow
        return;
    }
    port &= 0xFFFF;

    if ((andexpression < -32768) || (andexpression > 65535)) {
        error(6); // Overflow
        return;
    }
    andexpression &= 0xFF;

    if (passed) {
        if ((xorexpression < -32768) || (xorexpression > 65535)) {
            error(6); // Overflow
            return;
        }
    }
    xorexpression &= 0xFF;

    // Wait loop
    for (;;) {
        value = func_inp(port);
        if (passed)
            value ^= xorexpression;
        value &= andexpression;
        if (value || unsupported_port_accessed || stop_program)
            return;
        Sleep(1);
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Accessor function for unsupported_port_accessed flag
// Used by other modules that may need to check if a port operation was supported
//----------------------------------------------------------------------------------------------------------------------

int32_t port_io_get_unsupported_flag() {
    return unsupported_port_accessed;
}
