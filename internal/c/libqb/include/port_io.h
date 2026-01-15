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

#ifndef INCLUDE_LIBQB_PORT_IO_H
#define INCLUDE_LIBQB_PORT_IO_H

#include <stdint.h>

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// OUT port, data - write byte to I/O port
// Supported ports:
//   0x3C0 - VGA attribute controller (blink enable)
//   0x3C7 - VGA palette read index
//   0x3C8 - VGA palette write index
//   0x3C9 - VGA palette data (RGB, 6-bit per channel)
void sub_out(int32_t port, int32_t data);

// INP(port) - read byte from I/O port
// Supported ports:
//   0x3C9 - VGA palette data read
//   0x3DA - VGA status register (vertical retrace)
//   0x60  - Keyboard scancode buffer
int32_t func_inp(int32_t port);

// WAIT port, and_expr [, xor_expr] - wait for port condition
// Reads port, applies XOR (if passed), then AND, loops until non-zero
// Returns immediately for unsupported ports
void sub_wait(int32_t port, int32_t andexpression, int32_t xorexpression, int32_t passed);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module emulates legacy PC I/O port access for compatibility with
// classic BASIC programs. Only a subset of ports are supported:
//
// VGA Palette Ports (0x3C7, 0x3C8, 0x3C9):
//   Used to read/write the 256-color palette in indexed color modes.
//   Colors are stored as 6-bit RGB values (0-63 range).
//
// VGA Status Port (0x3DA):
//   Bit 3 indicates vertical retrace in progress.
//   Used for smooth animation timing in classic games.
//
// Keyboard Port (0x60):
//   Returns scancodes from the keyboard event buffer.
//
// Dependencies:
// - Uses write_page->pal for palette access
// - Uses port60h_event[] buffer for keyboard scancodes
// - Uses vertical_retrace_happened/in_progress flags

#endif // INCLUDE_LIBQB_PORT_IO_H
