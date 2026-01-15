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

// This module provides the public header declarations for port I/O functions.
// The actual implementations remain in libqb.cpp pending full extraction.
//
// Functions declared in port_io.h:
// - sub_out: Write byte to I/O port (VGA palette, etc.)
// - func_inp: Read byte from I/O port (VGA palette, keyboard, VGA status)
// - sub_wait: Wait for port condition
//
// These functions are currently implemented in libqb.cpp and will be migrated
// here incrementally as dependencies are resolved.
//
// Required globals for extraction:
// - H3C0_blink_enable, H3C7_palette_register_read_index
// - H3C8_palette_register_index, H3C9_next, H3C9_read_next
// - unsupported_port_accessed, vertical_retrace_happened, vertical_retrace_in_progress
// - port60h_event[], port60h_events
// - write_page (for palette access)

#include "port_io.h"

// Placeholder - implementations remain in libqb.cpp
// This file exists to:
// 1. Provide the port_io.h header for the public API
// 2. Reserve the module location for future extraction
// 3. Enable incremental migration without breaking the build
