//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Text and Font Module
//  Text output, cursor control, and font management functions
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

// This module provides the public header declarations for text/font functions.
// The actual implementations remain in libqb.cpp pending full extraction.
//
// Functions declared in text.h:
// - Core text output: printchr, chrwidth, newline, qbs_print, tab, makefit
// - Cursor positioning: qbg_sub_locate, func_csrlin, func_pos, func_tab, func_spc
// - Font management: selectfont, func__loadfont, sub__font, func__fontwidth, etc.
// - Print modes: sub__printmode, func__printmode, sub__printstring, func__printwidth
// - View print: qbg_sub_view_print
// - LPRINT: qbs_lprint, lprint_makefit
//
// These functions are currently implemented in libqb.cpp and will be migrated
// here incrementally as dependencies are resolved.

#include "text.h"

// Placeholder - implementations remain in libqb.cpp
// This file exists to:
// 1. Provide the text.h header for the public API
// 2. Reserve the module location for future extraction
// 3. Enable incremental migration without breaking the build
