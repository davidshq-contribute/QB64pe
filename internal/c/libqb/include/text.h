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

#ifndef INCLUDE_LIBQB_TEXT_H
#define INCLUDE_LIBQB_TEXT_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

// Forward declarations
struct qbs;
struct img_struct;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// Core text output
void printchr(int32_t character);
int32_t chrwidth(uint32_t character);
void newline();
void qbs_print(qbs *str, int32_t finish_on_new_line);
void tab();
void makefit(qbs *text);

// Cursor positioning
void qbg_sub_locate(int32_t row, int32_t column, int32_t cursor,
                    int32_t start, int32_t stop, int32_t passed);
int32_t func_csrlin();
int32_t func_pos(int32_t ignore);
qbs *func_tab(int32_t pos);
qbs *func_spc(int32_t spaces);

// Font selection helper
int32_t selectfont(int32_t f, img_struct *im);

// Font management (_LOADFONT, _FONT, _FREEFONT)
int32_t func__loadfont(const qbs *filename, int32_t size,
                       const qbs *requirements, int32_t font_index, int32_t passed);
void sub__font(int32_t f, int32_t i, int32_t passed);
int32_t func__fontwidth(int32_t f, int32_t passed);
int32_t func__fontheight(int32_t f, int32_t passed);
int32_t func__font(int32_t i, int32_t passed);
void sub__freefont(int32_t f);

// Print modes (_PRINTMODE, _PRINTSTRING, _PRINTWIDTH)
void sub__printmode(int32_t mode, int32_t i, int32_t passed);
int32_t func__printmode(int32_t i, int32_t passed);
void sub__printstring(float x, float y, qbs *text, int32_t i, int32_t passed);
int32_t func__printwidth(qbs *text, int32_t screenhandle, int32_t passed);

// View print (VIEW PRINT statement)
void qbg_sub_view_print(int32_t topline, int32_t bottomline, int32_t passed);

// LPRINT support
void qbs_lprint(qbs *str, int32_t finish_on_new_line);
void lprint_makefit(qbs *text);
int32_t func_lpos(int32_t lpt);

// CLS (clear screen)
void sub_cls(int32_t method, uint32_t use_color, int32_t passed);
void sub_clsDest(int32_t method, uint32_t use_color, int32_t dest, int32_t passed);

// Control characters flag (global)
extern int32_t no_control_characters;

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides text output and font management functionality:
// - Character rendering (printchr) with built-in and TrueType fonts
// - String printing (qbs_print) with control character support
// - Cursor positioning (LOCATE, CSRLIN, POS)
// - Font loading and management (_LOADFONT, _FONT, _FREEFONT)
// - Print modes for alpha blending control (_PRINTMODE)
// - Graphics-mode text positioning (_PRINTSTRING)
// - Text viewport management (VIEW PRINT)
// - Printer output (LPRINT)
//
// Dependencies:
// - Uses libqb_state.h accessors for write_page, font arrays
// - Built-in fonts use charset8x8/charset8x16 from libqb.cpp
// - TrueType fonts use FreeType via parts/video/font/

#endif // INCLUDE_LIBQB_TEXT_H
