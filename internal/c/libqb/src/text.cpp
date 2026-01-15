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

#include "libqb-common.h"

#include "text.h"
#include "error_handle.h"
#include "gfs.h"
#include "libqb_state.h"
#include "qbs.h"
#include "rounding.h"
#include "utility.h"
#include "datetime.h"
#include "../../libqb.h"
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>

#ifdef QB64_WINDOWS
#include <windows.h>
#endif

// Forward declarations for FreeType font functions
// Note: These are declared in font.h, but we forward declare them here to avoid circular dependencies
extern bool FontRenderTextASCII(int32_t fh, const uint8_t *codepoint, int32_t codepoints, int32_t options, uint8_t **out_data, int32_t *out_x, int32_t *out_y);
extern int32_t FontPrintWidthASCII(int32_t fh, const uint8_t *codepoint, int32_t codepoints);
extern int32_t FontPrintWidthUTF32(int32_t fh, const char32_t *codepoint, int32_t codepoints);
extern int32_t FontLoad(const uint8_t *content_original, int32_t content_bytes, int32_t default_pixel_height, int32_t which_font, int32_t &options);
extern int32_t FontWidth(int32_t fh);
extern void FontFree(int32_t fh);
extern uint8_t *FontLoadFileToMemory(const char *file_path_name, int32_t *out_bytes);

// Forward declarations for other libqb functions
extern void pset(int32_t x, int32_t y, uint32_t col);
extern void pset_and_clip(int32_t x, int32_t y, uint32_t col);
extern void fast_boxfill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);
extern void qb32_boxfill(float x1f, float y1f, float x2f, float y2f, uint32_t col);
extern void validatepage(int32_t i);
extern void sub__dest(int32_t i);
extern int32_t func__dest();
extern int32_t func__newimage(int32_t x, int32_t y, int32_t bpp, int32_t passed);
extern void qbg_sub_color(uint32_t col1, uint32_t col2, uint32_t bordercolor, int32_t passed);
extern void sub_beep();
extern void sub__printimage(int32_t i);
extern void key_update();
extern double func_timer(double accuracy, int32_t passed);
extern std::string filepath_fix_directory(std::string &path);

// Global variables from libqb.cpp
extern img_struct *img;
extern int32_t nextimg;
extern int32_t *page;
extern int32_t pages;
extern img_struct *write_page;
extern int32_t *font;
extern int32_t *fontheight;
extern int32_t *fontwidth;
extern int32_t *fontflags;
extern int32_t lastfont;
extern uint8_t charset8x8[256][8][8];
extern uint8_t charset8x16[256][16][8];
extern qbs *singlespace;
extern int32_t key_display_redraw;
extern int32_t autodisplay;
extern int32_t lock_display;
extern int32_t lock_display_required;
extern int32_t screen_last_valid;
extern uint8_t cmem[];
extern int32_t width8050switch;

// LPRINT variables
extern int32_t lprint;
extern int32_t lprint_image;
extern double lprint_last;
extern int32_t lprint_buffered;
extern int32_t lprint_locked;
extern int32_t lpos;
extern int32_t width_lprint;

// TAB/SPC variables (defined in generated code)
extern int32_t tab_LPRINT;
extern int32_t tab_spc_cr_size;
extern int32_t tab_fileno;

// Control characters flag
int32_t no_control_characters = 0;

// Font flag constants
#define FONT_LOAD_DONTBLEND 8
#define FONT_LOAD_MONOSPACE 16
#define FONT_LOAD_UNICODE 32
#define FONT_LOAD_AUTOMONO 64
#define INVALID_FONT_HANDLE 0

// IMG_SCREEN is defined in graphics.h (included via libqb.h)

// Image debug logging (conditional)
#ifdef IMAGE_DEBUG
#define image_log_trace(...) fprintf(stderr, __VA_ARGS__)
#else
#define image_log_trace(...)
#endif

// ============================================================================
// FONT SELECTION HELPER
// ============================================================================

int32_t selectfont(int32_t f, img_struct *im) {
    im->font = f;
    im->cursor_x = 1;
    im->cursor_y = 1;
    im->top_row = 1;
    if (im->compatible_mode)
        im->bottom_row = im->height / fontheight[f];
    else
        im->bottom_row = im->height;
    im->bottom_row--;
    if (im->bottom_row <= 0)
        im->bottom_row = 1;
    return 1; // success
}

// ============================================================================
// CHARACTER OUTPUT FUNCTIONS
// ============================================================================

void printchr(int32_t character) {
    static uint32_t x, x2, y, y2, w, h, z, z2, z3, a, a2, a3, color, background_color, f;
    static uint32_t *lp;
    static uint8_t *cp;
    static img_struct *im;

    im = write_page;
    color = im->color;
    background_color = im->background_color;

    if (im->text) {
        im->offset[(((im->cursor_y - 1) * im->width + im->cursor_x - 1)) << 1] = character;
        im->offset[((((im->cursor_y - 1) * im->width + im->cursor_x - 1)) << 1) + 1] = (color & 0xF) + background_color * 16 + (color & 16) * 8;
        return;
    }

    // precalculations
    f = im->font;
    x = fontwidth[f];
    if (x)
        x *= (im->cursor_x - 1);
    else
        x = im->cursor_x - 1;
    y = (im->cursor_y - 1) * fontheight[f];
    h = fontheight[f];
    if ((fontflags[f] & FONT_LOAD_UNICODE) == 0)
        character &= 255; // unicodefontsupport

    if (f >= 32) { // custom font
        // 8-bit / alpha-disabled 32-bit / dont-blend(alpha may still be applied)
        if ((im->bytes_per_pixel == 1) || ((im->bytes_per_pixel == 4) && (im->alpha_disabled)) || (fontflags[f] & FONT_LOAD_DONTBLEND)) {
            // render character
            static int32_t ok;
            static uint8_t *rt_data;
            static int32_t rt_w, rt_h;
            ok = FontRenderTextASCII(font[f], (const uint8_t *)&character, 1, 1, &rt_data, &rt_w, &rt_h);
            if (!ok)
                return;

            w = rt_w;

            switch (im->print_mode) {
            case 3:
                for (y2 = 0; y2 < h; y2++) {
                    cp = rt_data + y2 * w;
                    for (x2 = 0; x2 < w; x2++) {
                        if (*cp++)
                            pset(x + x2, y + y2, color);
                        else
                            pset(x + x2, y + y2, background_color);
                    }
                }
                break;
            case 1:
                for (y2 = 0; y2 < h; y2++) {
                    cp = rt_data + y2 * w;
                    for (x2 = 0; x2 < w; x2++) {
                        if (*cp++)
                            pset(x + x2, y + y2, color);
                    }
                }
                break;
            case 2:
                for (y2 = 0; y2 < h; y2++) {
                    cp = rt_data + y2 * w;
                    for (x2 = 0; x2 < w; x2++) {
                        if (!(*cp++))
                            pset(x + x2, y + y2, background_color);
                    }
                }
                break;
            default:
                break;
            }

            free(rt_data);
            return;
        } // 1-8 bit
        // assume 32-bit blended

        a = (color >> 24) + 1;
        a2 = (background_color >> 24) + 1;
        z = color & 0xFFFFFF;
        z2 = background_color & 0xFFFFFF;

        // render character
        static int32_t ok;
        static uint8_t *rt_data;
        static int32_t rt_w, rt_h;
        ok = FontRenderTextASCII(font[f], (const uint8_t *)&character, 1, 0, &rt_data, &rt_w, &rt_h);
        if (!ok)
            return;

        w = rt_w;

        switch (im->print_mode) {
        case 3: {
            static float r1, g1, b1, alpha1, r2, g2, b2, alpha2;
            alpha1 = (color >> 24) & 255;
            r1 = (color >> 16) & 255;
            g1 = (color >> 8) & 255;
            b1 = color & 255;
            alpha2 = (background_color >> 24) & 255;
            r2 = (background_color >> 16) & 255;
            g2 = (background_color >> 8) & 255;
            b2 = background_color & 255;
            static float dr, dg, db, da;
            dr = r2 - r1;
            dg = g2 - g1;
            db = b2 - b1;
            da = alpha2 - alpha1;
            static float cw; // color weight multiplier
            if (alpha1)
                cw = alpha2 / alpha1;
            else
                cw = 100000;
            static float d;

            for (y2 = 0; y2 < h; y2++) {
                cp = rt_data + y2 * w;
                for (x2 = 0; x2 < w; x2++) {
                    d = *cp++;
                    d = 255 - d;
                    d /= 255.0;
                    static float r3, g3, b3, alpha3;
                    alpha3 = alpha1 + da * d;
                    d *= cw;
                    if (d > 1.0)
                        d = 1.0;
                    r3 = r1 + dr * d;
                    g3 = g1 + dg * d;
                    b3 = b1 + db * d;
                    static int32_t r4, g4, b4, alpha4;
                    r4 = qbr_float_to_long(r3);
                    g4 = qbr_float_to_long(g3);
                    b4 = qbr_float_to_long(b3);
                    alpha4 = qbr_float_to_long(alpha3);
                    pset(x + x2, y + y2, b4 + (g4 << 8) + (r4 << 16) + (alpha4 << 24));
                }
            }
            break;
        }
        case 1:
            for (y2 = 0; y2 < h; y2++) {
                cp = rt_data + y2 * w;
                for (x2 = 0; x2 < w; x2++) {
                    z3 = *cp++;
                    if (z3)
                        pset(x + x2, y + y2, ((z3 * a) >> 8 << 24) + z);
                }
            }
            break;
        case 2:
            for (y2 = 0; y2 < h; y2++) {
                cp = rt_data + y2 * w;
                for (x2 = 0; x2 < w; x2++) {
                    z3 = *cp++;
                    if (z3 != 255)
                        pset(x + x2, y + y2, (((255 - z3) * a2) >> 8 << 24) + z2);
                }
            }
            break;
        default:
            break;
        }
        free(rt_data);
        return;
    } // custom font

    // default fonts
    if (im->font == 8)
        cp = &charset8x8[character][0][0];
    if (im->font == 14)
        cp = &charset8x16[character][1][0];
    if (im->font == 16)
        cp = &charset8x16[character][0][0];
    switch (im->print_mode) {
    case 3:
        for (y2 = 0; y2 < h; y2++) {
            for (x2 = 0; x2 < 8; x2++) {
                if (*cp++)
                    pset(x + x2, y + y2, color);
                else
                    pset(x + x2, y + y2, background_color);
            }
        }
        break;
    case 1:
        for (y2 = 0; y2 < h; y2++) {
            for (x2 = 0; x2 < 8; x2++) {
                if (*cp++)
                    pset(x + x2, y + y2, color);
            }
        }
        break;
    case 2:
        for (y2 = 0; y2 < h; y2++) {
            for (x2 = 0; x2 < 8; x2++) {
                if (!(*cp++))
                    pset(x + x2, y + y2, background_color);
            }
        }
        break;
    default:
        break;
    }
    return;
}

int32_t chrwidth(uint32_t character) {
    auto im = write_page;
    auto f = im->font;
    auto w = fontwidth[f];
    if (w)
        return w;

    // Custom font
    if ((fontflags[f] & FONT_LOAD_UNICODE)) { // UNICODE character
        w = FontPrintWidthUTF32(font[f], (const char32_t *)&character, 1);
    } else { // ASCII character
        character &= 255;
        w = FontPrintWidthASCII(font[f], (const uint8_t *)&character, 1);
    }

    return w;
}

// ============================================================================
// NEWLINE AND SCROLLING
// ============================================================================

void newline() {
    static uint32_t *lp;
    static uint16_t *sp;
    static int32_t z, z2;

    // move cursor to new line
    write_page->cursor_y++;
    write_page->cursor_x = 1;

    // scroll up screen if necessary
    if (write_page->cursor_y > write_page->bottom_row) {
        if (lprint) {
            sub__printimage(lprint_image);
            sub_cls(NULL, 15, 2);
            lprint_buffered = 0;
            return;
        }

        if (write_page->text) {
            // text
            // move lines up
            memmove(write_page->offset + (write_page->top_row - 1) * 2 * write_page->width, write_page->offset + write_page->top_row * 2 * write_page->width,
                    (write_page->bottom_row - write_page->top_row) * 2 * write_page->width);
            // erase bottom line
            z2 = (write_page->color & 0xF) + (write_page->background_color & 7) * 16 + (write_page->color & 16) * 8;
            z2 <<= 8;
            z2 += 32;
            sp = ((uint16_t *)(write_page->offset + (write_page->bottom_row - 1) * 2 * write_page->width));
            z = write_page->width;
            while (z--)
                *sp++ = z2;
        } else {
            // graphics
            // move lines up
            memmove(write_page->offset + (write_page->top_row - 1) * write_page->bytes_per_pixel * write_page->width * fontheight[write_page->font],
                    write_page->offset + write_page->top_row * write_page->bytes_per_pixel * write_page->width * fontheight[write_page->font],
                    (write_page->bottom_row - write_page->top_row) * write_page->bytes_per_pixel * write_page->width * fontheight[write_page->font]);
            // erase bottom line
            if (write_page->bytes_per_pixel == 1) {
                memset(write_page->offset + (write_page->bottom_row - 1) * write_page->width * fontheight[write_page->font], write_page->background_color,
                       write_page->width * fontheight[write_page->font]);
            } else {
                // assume 32-bit
                z2 = write_page->background_color;
                lp = write_page->offset32 + (write_page->bottom_row - 1) * write_page->width * fontheight[write_page->font];
                z = write_page->width * fontheight[write_page->font];
                while (z--)
                    *lp++ = z2;
            }
        } // graphics
        write_page->cursor_y = write_page->bottom_row;
    } // scroll up
}

// ============================================================================
// TEXT FITTING HELPERS
// ============================================================================

void makefit(qbs *text) {
    static int32_t w, x, x2, x3;
    if (write_page->holding_cursor)
        return;
    if (write_page->cursor_x != 1) { // if already at left-most, nothing more can be done
        if (write_page->text) {
            if ((write_page->cursor_x + text->len - 1) > write_page->width)
                newline();
        } else {
            w = func__printwidth(text, NULL, NULL);
            x = fontwidth[write_page->font];
            if (!x)
                x = 1;
            x = x * (write_page->cursor_x - 1);
            if ((x + w) > write_page->width)
                newline();
        }
    }
}

void lprint_makefit(qbs *text) {
    // stub
}

// ============================================================================
// TAB FUNCTION
// ============================================================================

void tab() {
    static int32_t x, x2, w;

    // tab() on a held-cursor only sets the cursor to the left hand position of the next line
    if (write_page->holding_cursor) {
        newline();
        write_page->holding_cursor = 0;
        return;
    }

#ifdef QB64_WINDOWS // if Windows console
    if (write_page->console) {
        if (func_pos(0) > write_page->width - 10) {
            printf("\n");
            return;
        } else {
            do {
                printf(" ");
            } while (func_pos(0) % 10 != 0);
        }
        return;
    }
#endif

    // text
    if (write_page->text) {
        qbs_print(singlespace, 0);
    text:
        if (write_page->cursor_x != 1) {
            if (((write_page->cursor_x - 1) % 14) || (write_page->cursor_x > (write_page->width - 13))) {
                if (write_page->cursor_x < write_page->width) {
                    qbs_print(singlespace, 0);
                    goto text;
                }
            }
        }
        return;
    }

    x = fontwidth[write_page->font];
    if (!x) {
        // variable width
        x = write_page->cursor_x - 1;
        x2 = (x / 112 + 1) * 112;      // next position
        if (x2 >= write_page->width) { // it doesn't fit on line
            if (write_page->print_mode != 1)
                fast_boxfill(x, (write_page->cursor_y - 1) * fontheight[write_page->font], write_page->width - 1,
                             write_page->cursor_y * fontheight[write_page->font] - 1, write_page->background_color);
            newline();
        } else { // fits on line
            if (write_page->print_mode != 1)
                fast_boxfill(x, (write_page->cursor_y - 1) * fontheight[write_page->font], x2 - 1, write_page->cursor_y * fontheight[write_page->font] - 1,
                             write_page->background_color);
            write_page->cursor_x = x2;
        }
    } else {
        // fixed width
        w = write_page->width / x;

        qbs_print(singlespace, 0);
    fixwid:
        if (write_page->cursor_x != 1) {
            if (((write_page->cursor_x - 1) % 14) || (write_page->cursor_x > (w - 13))) {
                if (write_page->cursor_x < w) {
                    qbs_print(singlespace, 0);
                    goto fixwid;
                }
            }
        }
    }
    return;
}

// ============================================================================
// LPRINT FUNCTIONS
// ============================================================================

int32_t func_lpos(int32_t lpt) {
    // lpt values: 0 = LPT1, 1 = LPT1, 2 = LPT2, 3 = LPT3
    if ((lpt < 0) || (lpt > 3)) {
        error(5);
        return 0;
    }
    return lpos;
}

void qbs_lprint(qbs *str, int32_t finish_on_new_line) {
    static int32_t old_dest;
    while (lprint_locked)
        Sleep(64);
    lprint = 1;
    old_dest = func__dest();
    if (!lprint_image) {
        lprint_image = func__newimage(640, 960, 13, 1);
        sub__dest(lprint_image);
        sub_cls(NULL, 15, 2);
        sub__font(16, NULL, 0);
        qbg_sub_color(0, 15, NULL, 3);
        qbg_sub_view_print(1, 60, 1);
    } else {
        sub__dest(lprint_image);
    }
    lprint_buffered = 1;
    lprint_last = func_timer(0.001, 1);
    qbs_print(str, finish_on_new_line);
    sub__dest(old_dest);
    lprint = 0;
}

// ============================================================================
// MAIN PRINT FUNCTION
// ============================================================================

void qbs_print(qbs *str, int32_t finish_on_new_line) {
    if (is_error_pending())
        return;
    int32_t i, i2, entered_new_line, x, x2, y, y2, z, z2, w;
    entered_new_line = 0;
    static uint32_t character;

    if (write_page->console) {
        static qbs *strz;
        if (!strz)
            strz = qbs_new(0, 0);
        qbs_set(strz, qbs_add(str, qbs_new_txt_len("\0", 1)));
        if (finish_on_new_line)
            std::cout << (char *)strz->chr << std::endl;
        else
            std::cout << (char *)strz->chr;
#ifndef QB64_WINDOWS
        std::cout.flush();
#endif
        return;
    }

    // holding cursor?
    if (write_page->holding_cursor) {
        if (str->len) {
            write_page->holding_cursor = 0;
            newline();
        } else {
            // null length print string
            if (finish_on_new_line)
                write_page->holding_cursor = 0; // new line will be entered automatically
        }
    }

    for (i = 0; i < str->len; i++) {
        character = str->chr[i];

        if (fontflags[write_page->font] & FONT_LOAD_UNICODE) { // unicode font
            if (i > (str->len - 4))
                break; // not enough data for a utf32 encoding
            character = *((int32_t *)(&str->chr[i]));
            i += 3;
        }

        if (lprint)
            lprint_buffered = 1;
        entered_new_line = 0;

        // special characters
        if (no_control_characters || get_control_characters_disabled())
            goto skip_control_characters;

        if (character == 28) {
            // advance one cursor position
            if (lprint) {
                if (lpos < width_lprint)
                    lpos++;
            }
            // can cursor advance?
            if (write_page->cursor_y >= write_page->bottom_row) {
                if (write_page->text) {
                    if (write_page->cursor_x >= write_page->width)
                        goto skip;
                } else {
                    if (fontwidth[write_page->font]) {
                        if (write_page->cursor_x >= (write_page->width / fontwidth[write_page->font]))
                            goto skip;
                    } else {
                        if (write_page->cursor_x >= write_page->width)
                            goto skip;
                    }
                }
            }
            write_page->cursor_x++;
            if (write_page->text) {
                if (write_page->cursor_x > write_page->width) {
                    write_page->cursor_y++;
                    write_page->cursor_x = 1;
                }
            } else {
                if (fontwidth[write_page->font]) {
                    if (write_page->cursor_x > (write_page->width / fontwidth[write_page->font])) {
                        write_page->cursor_y++;
                        write_page->cursor_x = 1;
                    }
                } else {
                    if (write_page->cursor_x > write_page->width) {
                        write_page->cursor_y++;
                        write_page->cursor_x = 1;
                    }
                }
            }
            goto skip;
        }

        if (character == 29) {
            // go back one cursor position
            if (lprint) {
                if (lpos > 1)
                    lpos--;
            }
            // can cursor go back?
            if ((write_page->cursor_y == write_page->top_row) || (write_page->cursor_y > write_page->bottom_row)) {
                if (write_page->cursor_x == 1)
                    goto skip;
            }
            write_page->cursor_x--;
            if (write_page->cursor_x < 1) {
                write_page->cursor_y--;
                if (write_page->text) {
                    write_page->cursor_x = write_page->width;
                } else {
                    if (fontwidth[write_page->font]) {
                        write_page->cursor_x = write_page->width / fontwidth[write_page->font];
                    } else {
                        write_page->cursor_x = write_page->width;
                    }
                }
            }
            goto skip;
        }

        if (character == 30) {
            // previous row, same column
            if ((write_page->cursor_y > write_page->top_row) && (write_page->cursor_y <= write_page->bottom_row)) {
                write_page->cursor_y--;
            }
            goto skip;
        }

        if (character == 31) {
            // next row, same column
            if ((write_page->cursor_y >= write_page->top_row) && (write_page->cursor_y < write_page->bottom_row)) {
                write_page->cursor_y++;
            }
            goto skip;
        }

        if (character == 12) { // aka form feed
            if (lprint) {
                sub__printimage(lprint_image);
                lprint_buffered = 0;
            }
            sub_cls(NULL, NULL, 0);
            if (lprint)
                lpos = 1;
            goto skip;
        }

        if (character == 11) {
            write_page->cursor_x = 1;
            write_page->cursor_y = write_page->top_row;
            if (lprint)
                lpos = 1;
            goto skip;
        }

        if (character == 9) {
            // moves to next multiple of 8 (always advances at least one space)
            if (!fontwidth[write_page->font]) {
                // variable width!
                x = write_page->cursor_x - 1;
                x2 = (x / 64 + 1) * 64;        // next position
                if (x2 >= write_page->width) { // it doesn't fit on line
                    if (write_page->print_mode != 1)
                        fast_boxfill(x, (write_page->cursor_y - 1) * fontheight[write_page->font], write_page->width - 1,
                                     write_page->cursor_y * fontheight[write_page->font] - 1, write_page->background_color);
                    newline();
                    entered_new_line = 1;
                } else { // fits on line
                    if (write_page->print_mode != 1)
                        fast_boxfill(x, (write_page->cursor_y - 1) * fontheight[write_page->font], x2 - 1,
                                     write_page->cursor_y * fontheight[write_page->font] - 1, write_page->background_color);
                    write_page->cursor_x = x2;
                }
                goto skip;
            } else {
                if (write_page->cursor_x % 8) { // next cursor position not a multiple of 8
                    i--;                        // more spaces will be required
                }
                character = 32; // override character 9
            }
        } // 9

        if (character == 7) {
            sub_beep();
            goto skip;
        }

        if ((character == 10) || (character == 13)) {
            newline();
            if (lprint)
                lpos = 1;
            goto skip;
        }

    skip_control_characters:

        // check if character fits on line, if not move to next line
        if (!fontwidth[write_page->font]) { // unpredictable width
            w = chrwidth(character);
            if ((write_page->cursor_x - 1 + w) > write_page->width) {
                newline();
            }
        }

        // print the character
        printchr(character);

        // advance lpos, begin new line if necessary
        if (lprint) {
            lpos++;
            if (lpos > width_lprint) {
                newline();
                entered_new_line = 1;
                lpos = 1;
                goto skip;
            }
        }

        // advance cursor
        if (fontwidth[write_page->font]) {
            write_page->cursor_x++;
        } else {
            write_page->cursor_x += w;
        }

        // check if another character could fit at cursor_x's location
        if (write_page->compatible_mode) { // graphics
            x = fontwidth[write_page->font];
            if (!x)
                x = 1;
            x2 = x * (write_page->cursor_x - 1);
            if (x2 > (write_page->width - x)) {
                if (!finish_on_new_line) {
                    if (i == (str->len - 1)) { // last character
                        write_page->cursor_x = write_page->width / x;
                        write_page->holding_cursor = 1;
                        goto held_cursor;
                    }
                }
                newline();
                entered_new_line = 1;
            }
        } else { // text
            if (write_page->cursor_x > write_page->width) {
                if (!finish_on_new_line) {
                    if (i == (str->len - 1)) {  // last character
                        write_page->cursor_x--; // move horizontal cursor back to right-most valid position
                        write_page->holding_cursor = 1;
                        goto held_cursor;
                    }
                }
                newline();
                entered_new_line = 1;
            }
        }
    held_cursor:

    skip:;
    } // i

    if (finish_on_new_line && (!entered_new_line))
        newline();
    if (lprint) {
        if (finish_on_new_line)
            lpos = 1;
    }

    return;
}

// ============================================================================
// VIEW PRINT
// ============================================================================

void qbg_sub_view_print(int32_t topline, int32_t bottomline, int32_t passed) {
    if (is_error_pending())
        return;

    static int32_t maxrows;
    maxrows = write_page->height;
    if (!write_page->text)
        maxrows /= fontheight[write_page->font];

    if (!passed) { // topline and bottomline not passed
        write_page->top_row = 1;
        write_page->bottom_row = maxrows;
        write_page->cursor_y = 1;
        write_page->cursor_x = 1;
        write_page->holding_cursor = 0;
        return;
    }

    if (topline <= 0)
        goto error;
    if (topline > maxrows)
        goto error;
    if (bottomline < topline)
        goto error;
    if (bottomline > maxrows)
        goto error;

    write_page->top_row = topline;
    write_page->bottom_row = bottomline;
    write_page->cursor_y = write_page->top_row;
    write_page->cursor_x = 1;
    write_page->holding_cursor = 0;
    return;

error:
    error(5);
    return;
}

// ============================================================================
// CLS (CLEAR SCREEN)
// ============================================================================

// Forward declaration for qbsub_width (remains in libqb.cpp due to screen mode complexity)
extern void qbsub_width(int32_t option, int32_t value1, int32_t value2, int32_t value3, int32_t value4, int32_t passed);

void sub_clsDest(int32_t method, uint32_t use_color, int32_t dest, int32_t passed) {
    int32_t tempDest;
    if (passed & 4) {
        tempDest = func__dest(); // get the old dest
        sub__dest(dest);         // set the new dest
    }

    sub_cls(method, use_color, passed & 3);

    if (passed & 4) {
        sub__dest(tempDest);
    }
}

void sub_cls(int32_t method, uint32_t use_color, int32_t passed) {
    if (is_error_pending())
        return;
    static int32_t characters, i;
    static uint16_t *sp;
    static uint16_t clearvalue;

    if (write_page->console) {
#ifdef QB64_WINDOWS
        system("cls");
        qbg_sub_locate(1, 1, 0, 0, 0, 3);
#else
        if (passed & 2)
            qbg_sub_color(0, use_color, 0, 2);
        std::cout << "\033[2J";
        qbg_sub_locate(1, 1, 0, 0, 0, 3);
#endif
        return;
    }

    // validate
    if (passed & 2) {
        if (write_page->bytes_per_pixel != 4) {
            if (use_color > write_page->mask)
                goto error;
        }
    } else {
        use_color = write_page->background_color;
    }

    if (passed & 1) {
        if ((method > 2) || (method < 0))
            goto error;
    }

    // all CLS methods reset the cursor position
    write_page->cursor_y = write_page->top_row;
    write_page->cursor_x = 1;

    // all CLS methods reset DRAW attributes
    write_page->draw_ta = 0.0;
    write_page->draw_scale = 1.0;

    if (write_page->text) {
        clearvalue = (write_page->color & 0xF) + (use_color & 7) * 16 + (write_page->color & 16) * 8;
        clearvalue <<= 8;
        clearvalue += 32;
    }

    if ((passed & 1) == 0) { // no method specified
        if (write_page->text) {
            // text view port
            characters = write_page->width * (write_page->bottom_row - write_page->top_row + 1);
            sp = (uint16_t *)&write_page->offset[(write_page->top_row - 1) * write_page->width * 2];
            for (i = 0; i < characters; i++) {
                sp[i] = clearvalue;
            }
            // bottom line
            characters = write_page->width;
            sp = (uint16_t *)&write_page->offset[(write_page->height - 1) * write_page->width * 2];
            for (i = 0; i < characters; i++) {
                sp[i] = clearvalue;
            }
            key_display_redraw = 1;
            key_update();
            return;
        } else { // graphics
            if (write_page->bytes_per_pixel == 1) { // 8-bit
                if (write_page->clipping_or_scaling) {
                    qb32_boxfill(write_page->window_x1, write_page->window_y1, write_page->window_x2, write_page->window_y2, use_color);
                } else {
                    memset(write_page->offset, use_color, write_page->width * write_page->height);
                }
            } else { // 32-bit
                i = write_page->alpha_disabled;
                write_page->alpha_disabled = 1;
                if (write_page->clipping_or_scaling) {
                    qb32_boxfill(write_page->window_x1, write_page->window_y1, write_page->window_x2, write_page->window_y2, use_color);
                } else {
                    fast_boxfill(0, 0, write_page->width - 1, write_page->height - 1, use_color);
                }
                write_page->alpha_disabled = i;
            }
        }

        if (write_page->clipping_or_scaling == 2) {
            write_page->x = ((float)(write_page->view_x2 - write_page->view_x1 + 1)) / write_page->scaling_x / 2.0f + write_page->scaling_offset_x;
            write_page->y = ((float)(write_page->view_y2 - write_page->view_y1 + 1)) / write_page->scaling_y / 2.0f + write_page->scaling_offset_y;
        } else {
            write_page->x = ((float)(write_page->view_x2 - write_page->view_x1 + 1)) / 2.0f;
            write_page->y = ((float)(write_page->view_y2 - write_page->view_y1 + 1)) / 2.0f;
        }

        key_display_redraw = 1;
        key_update();
        return;
    }

    if (method == 0) { // clear everything
        if (write_page->text) {
            characters = write_page->height * write_page->width;
            sp = (uint16_t *)write_page->offset;
            for (i = 0; i < characters; i++) {
                sp[i] = clearvalue;
            }
            key_display_redraw = 1;
            key_update();
            return;
        } else {
            if (write_page->bytes_per_pixel == 1) {
                memset(write_page->offset, use_color, write_page->width * write_page->height);
            } else {
                i = write_page->alpha_disabled;
                write_page->alpha_disabled = 1;
                fast_boxfill(0, 0, write_page->width - 1, write_page->height - 1, use_color);
                write_page->alpha_disabled = i;
            }
        }

        if (write_page->clipping_or_scaling == 2) {
            write_page->x = ((float)(write_page->view_x2 - write_page->view_x1 + 1)) / write_page->scaling_x / 2.0f + write_page->scaling_offset_x;
            write_page->y = ((float)(write_page->view_y2 - write_page->view_y1 + 1)) / write_page->scaling_y / 2.0f + write_page->scaling_offset_y;
        } else {
            write_page->x = ((float)(write_page->view_x2 - write_page->view_x1 + 1)) / 2.0f;
            write_page->y = ((float)(write_page->view_y2 - write_page->view_y1 + 1)) / 2.0f;
        }

        key_display_redraw = 1;
        key_update();
        return;
    }

    if (method == 1) { // ONLY clear the graphics viewport
        if (write_page->text)
            return;
        if (write_page->bytes_per_pixel == 1) {
            if (write_page->clipping_or_scaling) {
                qb32_boxfill(write_page->window_x1, write_page->window_y1, write_page->window_x2, write_page->window_y2, use_color);
            } else {
                memset(write_page->offset, use_color, write_page->width * write_page->height);
            }
        } else {
            i = write_page->alpha_disabled;
            write_page->alpha_disabled = 1;
            if (write_page->clipping_or_scaling) {
                qb32_boxfill(write_page->window_x1, write_page->window_y1, write_page->window_x2, write_page->window_y2, use_color);
            } else {
                fast_boxfill(0, 0, write_page->width - 1, write_page->height - 1, use_color);
            }
            write_page->alpha_disabled = i;
        }

        if (write_page->clipping_or_scaling == 2) {
            write_page->x = ((float)(write_page->view_x2 - write_page->view_x1 + 1)) / write_page->scaling_x / 2.0f + write_page->scaling_offset_x;
            write_page->y = ((float)(write_page->view_y2 - write_page->view_y1 + 1)) / write_page->scaling_y / 2.0f + write_page->scaling_offset_y;
        } else {
            write_page->x = ((float)(write_page->view_x2 - write_page->view_x1 + 1)) / 2.0f;
            write_page->y = ((float)(write_page->view_y2 - write_page->view_y1 + 1)) / 2.0f;
        }

        key_display_redraw = 1;
        key_update();
        return;
    }

    if (method == 2) { // ONLY clear the VIEW PRINT range text viewport
        if (write_page->text) {
            characters = write_page->width * (write_page->bottom_row - write_page->top_row + 1);
            sp = (uint16_t *)&write_page->offset[(write_page->top_row - 1) * write_page->width * 2];
            for (i = 0; i < characters; i++) {
                sp[i] = clearvalue;
            }
            return;
        } else {
            if (write_page->bytes_per_pixel == 1) {
                memset(&write_page->offset[write_page->width * fontheight[write_page->font] * (write_page->top_row - 1)], use_color,
                       write_page->width * fontheight[write_page->font] * (write_page->bottom_row - write_page->top_row + 1));
            } else {
                i = write_page->alpha_disabled;
                write_page->alpha_disabled = 1;
                fast_boxfill(0, fontheight[write_page->font] * (write_page->top_row - 1), write_page->width - 1,
                             fontheight[write_page->font] * write_page->bottom_row - 1, use_color);
                write_page->alpha_disabled = i;
            }
            return;
        }
    }

    return;
error:
    error(5);
    return;
}

// ============================================================================
// LOCATE STATEMENT
// ============================================================================

void qbg_sub_locate(int32_t row, int32_t column, int32_t cursor, int32_t start, int32_t stop, int32_t passed) {
    static int32_t h, w, i;
    if (is_error_pending())
        return;

    if (write_page->console) {
#ifdef QB64_WINDOWS
        CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;
        SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
        HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
        GetConsoleScreenBufferInfo(cl_conout, &cl_bufinfo);
        if (column == 0)
            column = cl_bufinfo.dwCursorPosition.X + 1;
        if (row == 0)
            row = cl_bufinfo.dwCursorPosition.Y + 1;
        COORD pos = {(SHORT)(column - 1), (SHORT)(row - 1)};
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleCursorPosition(output, pos);
#else
        if (!(passed & 1 && passed & 2))
            return;
        printf("\033[%d;%dH", row, column);
#endif
        return;
    }

    // calculate height & width in characters
    if (write_page->compatible_mode) {
        h = write_page->height / fontheight[write_page->font];
        if (fontwidth[write_page->font]) {
            w = write_page->width / fontwidth[write_page->font];
        } else {
            w = write_page->width;
        }
    } else {
        h = write_page->height;
        w = write_page->width;
    }

    // PRE-ERROR CHECKING
    if (passed & 1) {
        if (row < write_page->top_row)
            goto error;
        if ((row != h) && (row > write_page->bottom_row)) {
            if (width8050switch) {
                width8050switch = 0;
                if (row <= 50) {
                    if (passed & 2) {
                        if (column < 1)
                            goto error;
                        if (column > w)
                            goto error;
                    }
                    char *buffer;
                    uint32_t c, c2;
                    buffer = (char *)malloc(80 * 25 * 2);
                    c = write_page->color;
                    c2 = write_page->background_color;
                    memcpy(buffer, &cmem[0xB8000], 80 * 25 * 2);
                    qbsub_width(0, 80, 50, 0, 0, 3);
                    memcpy(&cmem[0xB8000], buffer, 80 * 25 * 2);
                    write_page->color = c;
                    write_page->background_color = c2;
                    free(buffer);
                    goto width8050switch_done;
                }
            }
            goto error;
        }
    }
width8050switch_done:
    if (passed & 2) {
        if (column < 1)
            goto error;
        if (column > w)
            goto error;
    }
    if (passed & 4) {
        if (cursor < 0)
            goto error;
        if (cursor > 1)
            goto error;
    }
    if (passed & 8) {
        if (start < 0)
            goto error;
        if (start > 31)
            goto error;
    }
    if (passed & 16) {
        if (stop < 0)
            goto error;
        if (stop > 31)
            goto error;
    }

    if (passed & 1) {
        write_page->cursor_y = row;
        write_page->holding_cursor = 0;
    }
    if (passed & 2) {
        write_page->cursor_x = column;
        write_page->holding_cursor = 0;
    }
    if ((passed & 3) == 0) {
        if (write_page->holding_cursor)
            write_page->holding_cursor = 2; // special case
    }

    if (passed & 4) {
        if (cursor)
            cursor = 1;
        write_page->cursor_show = cursor;
        if (write_page->flags & IMG_SCREEN) {
            for (i = 0; i < pages; i++) {
                if (page[i])
                    img[i].cursor_show = cursor;
            }
        }
    }

    if (passed & 8) {
        write_page->cursor_firstvalue = start;
    } else {
        start = write_page->cursor_firstvalue;
    }
    if (passed & 16) {
        write_page->cursor_lastvalue = stop;
    } else {
        stop = write_page->cursor_lastvalue;
    }
    if (passed & (8 + 16)) {
        if (write_page->flags & IMG_SCREEN) {
            for (i = 0; i < pages; i++) {
                if (page[i]) {
                    img[i].cursor_firstvalue = start;
                    img[i].cursor_lastvalue = stop;
                }
            }
        }
    }

    return;

error:
    error(5);
    return;
}

// ============================================================================
// CURSOR POSITION FUNCTIONS
// ============================================================================

int32_t func_csrlin() {
#ifdef QB64_WINDOWS
    if (write_page->console) {
        CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;
        SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
        HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
        GetConsoleScreenBufferInfo(cl_conout, &cl_bufinfo);
        return cl_bufinfo.dwCursorPosition.Y + 1;
    }
#endif
    if (write_page->holding_cursor) {
        if (write_page->cursor_y >= write_page->bottom_row)
            return write_page->bottom_row;
        else
            return write_page->cursor_y + 1;
    }
    return write_page->cursor_y;
}

int32_t func_pos(int32_t ignore) {
#ifdef QB64_WINDOWS
    if (write_page->console) {
        CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;
        SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
        HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
        GetConsoleScreenBufferInfo(cl_conout, &cl_bufinfo);
        return cl_bufinfo.dwCursorPosition.X + 1;
    }
#endif
    if (write_page->holding_cursor)
        return 1;
    return write_page->cursor_x;
}

// ============================================================================
// TAB AND SPC FUNCTIONS
// ============================================================================

qbs *func_tab(int32_t pos) {
    if (is_error_pending())
        return qbs_new(0, 1);

    static int32_t tab_LPRINT_olddest;
    if (tab_LPRINT) {
        if (!lprint_image)
            qbs_lprint(qbs_new(0, 1), 0); // send dummy data to init the LPRINT image
        tab_LPRINT_olddest = func__dest();
        sub__dest(lprint_image);
    }

    static int32_t w, div, cursor;

    static int32_t cr_size;
    cr_size = tab_spc_cr_size;
    // calculate width in spaces & current position
    if (cr_size == 2) {
        // print to file
        div = 1;
        w = 2147483647;
        cursor = 1;
        static int32_t i;
        i = tab_fileno;
        if (i < 0)
            goto invalid_file;
        if (gfs_fileno_valid(i) != 1)
            goto invalid_file;
        i = gfs_get_fileno(i);
        if (gfs_get_file_struct(i)->scrn == 1) {
            cr_size = 1;
        } else {
            cursor = gfs_get_file_struct(i)->column;
        }
    invalid_file:;
    }
    if (cr_size == 1) {
        // print to surface
        if (write_page->text) {
            w = write_page->width;
            div = 1;
        } else {
            if (fontwidth[write_page->font]) {
                w = write_page->width / fontwidth[write_page->font];
                div = 1;
            } else {
                w = write_page->width;
                div = func__printwidth(singlespace, NULL, 0);
            }
        }
        cursor = write_page->cursor_x;
    }

    static qbs *tqbs;
    if ((pos < -32768) || (pos > 32767)) {
        if (tab_LPRINT)
            sub__dest(tab_LPRINT_olddest);
        tqbs = qbs_new(0, 1);
        error(6);
        return tqbs;
    }
    if (pos > w)
        pos %= w;
    if (pos < 1)
        pos = 1;
    static int32_t size, spaces, cr;
    size = 0;
    spaces = 0;
    cr = 0;
    if (cursor > pos) {
        cr = 1;
        size = cr_size;
        spaces = pos / div;
        if (pos % div)
            spaces++;
        spaces--;
        size += spaces;
    } else {
        spaces = (pos - cursor) / div;
        if ((pos - cursor) % div)
            spaces++;
        size = spaces;
    }
    // build custom string
    tqbs = qbs_new(size, 1);
    if (cr) {
        tqbs->chr[0] = 13;
        if (cr_size == 2)
            tqbs->chr[1] = 10;
        memset(&tqbs->chr[cr_size], 32, spaces);
    } else {
        memset(tqbs->chr, 32, spaces);
    }
    if (tab_LPRINT)
        sub__dest(tab_LPRINT_olddest);
    return tqbs;
}

qbs *func_spc(int32_t spaces) {
    if (is_error_pending())
        return qbs_new(0, 1);

    static qbs *tqbs;
    if ((spaces < -32768) || (spaces > 32767)) {
        tqbs = qbs_new(0, 1);
        error(6);
        return tqbs;
    }
    if (spaces < 0)
        spaces = 0;

    // for files, spc simply adds that many spaces
    if (tab_spc_cr_size == 2) {
        tqbs = qbs_new(spaces, 1);
        memset(tqbs->chr, 32, spaces);
        return tqbs;
    }

    static int32_t tab_LPRINT_olddest;
    if (tab_LPRINT) {
        if (!lprint_image)
            qbs_lprint(qbs_new(0, 1), 0);
        tab_LPRINT_olddest = func__dest();
        sub__dest(lprint_image);
    }

    static int32_t x, x2;
    static int32_t spaces_left_on_line;
    static qbs *onespace = NULL;
    if (!onespace) {
        onespace = qbs_new(1, 0);
        onespace->chr[0] = 32;
    }
    static int32_t onespace_width;
    if (write_page->text) {
        spaces_left_on_line = write_page->width - write_page->cursor_x + 1;
        spaces %= write_page->width;
    } else {
        x = fontwidth[write_page->font];
        if (x) {
            x2 = write_page->width / x;
            spaces_left_on_line = x2 - write_page->cursor_x + 1;
            spaces %= x2;
        } else {
            x2 = write_page->width - write_page->cursor_x + 1;
            onespace_width = func__printwidth(onespace, NULL, 0);
            spaces_left_on_line = x2 / onespace_width;
            spaces %= (write_page->width / onespace_width);
        }
    }

    // build string
    if (spaces_left_on_line >= spaces) {
        tqbs = qbs_new(spaces, 1);
        memset(tqbs->chr, 32, spaces);
    } else {
        spaces -= spaces_left_on_line;
        tqbs = qbs_new(1 + spaces, 1);
        tqbs->chr[0] = 13;
        memset(tqbs->chr + 1, 32, spaces);
    }

    if (tab_LPRINT)
        sub__dest(tab_LPRINT_olddest);
    return tqbs;
}

// ============================================================================
// _PRINTSTRING
// ============================================================================

void sub__printstring(float x, float y, qbs *text, int32_t i, int32_t passed) {
    if (is_error_pending())
        return;

    int32_t old_dest = func__dest();

    if (passed & 2) {
        sub__dest(i);
        if (i >= 0) {
            validatepage(i);
            i = page[i];
        } else {
            i = -i;
            if (i >= nextimg) {
                error(258);
                goto printstring_exit;
            }
            if (!img[i].valid) {
                error(258);
                goto printstring_exit;
            }
        }
    } else {
        i = libqb_get_write_page_index();
    }
    static img_struct *im;
    im = &img[i];
    if (!text->len)
        goto printstring_exit;
    if (im->text) {
        int oldx = func_pos(0), oldy = func_csrlin();
        qbg_sub_locate(y, x, 0, 0, 0, 3);
        qbs_print(text, 0);
        qbg_sub_locate(oldy, oldx, 0, 0, 0, 3);
        goto printstring_exit;
    }
    // graphics modes only
    if (!text->len)
        goto printstring_exit;
    // Step?
    if (passed & 1) {
        im->x += x;
        im->y += y;
    } else {
        im->x = x;
        im->y = y;
    }
    // Adjust co-ordinates for viewport?
    static int32_t x2, y2;
    if (im->clipping_or_scaling) {
        if (im->clipping_or_scaling == 2) {
            x2 = qbr_float_to_long(im->x * im->scaling_x + im->scaling_offset_x) + im->view_offset_x;
            y2 = qbr_float_to_long(im->y * im->scaling_y + im->scaling_offset_y) + im->view_offset_y;
        } else {
            x2 = qbr_float_to_long(im->x) + im->view_offset_x;
            y2 = qbr_float_to_long(im->y) + im->view_offset_y;
        }
    } else {
        x2 = qbr_float_to_long(im->x);
        y2 = qbr_float_to_long(im->y);
    }

    if (!text->len)
        goto printstring_exit;

    static uint32_t w, h, z, z2, z3, a, a2, a3, color, background_color, f;
    static uint32_t *lp;
    static uint8_t *cp;

    color = im->color;
    background_color = im->background_color;

    f = im->font;
    h = fontheight[f];

    if (f >= 32) { // custom font
        if ((im->bytes_per_pixel == 1) || ((im->bytes_per_pixel == 4) && (im->alpha_disabled)) || (fontflags[f] & FONT_LOAD_DONTBLEND)) {
            static int32_t ok;
            static uint8_t *rt_data;
            static int32_t rt_w, rt_h;
            ok = FontRenderTextASCII(font[f], (const uint8_t *)text->chr, text->len, 1, &rt_data, &rt_w, &rt_h);
            if (!ok)
                goto printstring_exit;

            w = rt_w;

            switch (im->print_mode) {
            case 3:
                for (y2 = 0; y2 < h; y2++) {
                    cp = rt_data + y2 * w;
                    for (x2 = 0; x2 < w; x2++) {
                        if (*cp++)
                            pset_and_clip(x + x2, y + y2, color);
                        else
                            pset_and_clip(x + x2, y + y2, background_color);
                    }
                }
                break;
            case 1:
                for (y2 = 0; y2 < h; y2++) {
                    cp = rt_data + y2 * w;
                    for (x2 = 0; x2 < w; x2++) {
                        if (*cp++)
                            pset_and_clip(x + x2, y + y2, color);
                    }
                }
                break;
            case 2:
                for (y2 = 0; y2 < h; y2++) {
                    cp = rt_data + y2 * w;
                    for (x2 = 0; x2 < w; x2++) {
                        if (!(*cp++))
                            pset_and_clip(x + x2, y + y2, background_color);
                    }
                }
                break;
            default:
                break;
            }

            free(rt_data);
            goto printstring_exit;
        }
        // assume 32-bit blended

        a = (color >> 24) + 1;
        a2 = (background_color >> 24) + 1;
        z = color & 0xFFFFFF;
        z2 = background_color & 0xFFFFFF;

        static int32_t ok;
        static uint8_t *rt_data;
        static int32_t rt_w, rt_h;
        ok = FontRenderTextASCII(font[f], (const uint8_t *)text->chr, text->len, 0, &rt_data, &rt_w, &rt_h);

        if (!ok)
            goto printstring_exit;

        w = rt_w;

        switch (im->print_mode) {
        case 3: {
            static float r1, g1, b1, alpha1, r2, g2, b2, alpha2;
            alpha1 = (color >> 24) & 255;
            r1 = (color >> 16) & 255;
            g1 = (color >> 8) & 255;
            b1 = color & 255;
            alpha2 = (background_color >> 24) & 255;
            r2 = (background_color >> 16) & 255;
            g2 = (background_color >> 8) & 255;
            b2 = background_color & 255;
            static float dr, dg, db, da;

            dr = r2 - r1;
            dg = g2 - g1;
            db = b2 - b1;
            da = alpha2 - alpha1;
            static float cw;
            if (alpha1)
                cw = alpha2 / alpha1;
            else
                cw = 100000;
            static float d;

            for (y2 = 0; y2 < h; y2++) {
                cp = rt_data + y2 * w;
                for (x2 = 0; x2 < w; x2++) {
                    d = *cp++;
                    d = 255 - d;
                    d /= 255.0;
                    static float r3, g3, b3, alpha3;
                    alpha3 = alpha1 + da * d;
                    d *= cw;
                    if (d > 1.0)
                        d = 1.0;
                    r3 = r1 + dr * d;
                    g3 = g1 + dg * d;
                    b3 = b1 + db * d;
                    static int32_t r4, g4, b4, alpha4;
                    r4 = qbr_float_to_long(r3);
                    g4 = qbr_float_to_long(g3);
                    b4 = qbr_float_to_long(b3);
                    alpha4 = qbr_float_to_long(alpha3);
                    pset_and_clip(x + x2, y + y2, b4 + (g4 << 8) + (r4 << 16) + (alpha4 << 24));
                }
            }
            break;
        }
        case 1:
            for (y2 = 0; y2 < h; y2++) {
                cp = rt_data + y2 * w;
                for (x2 = 0; x2 < w; x2++) {
                    z3 = *cp++;
                    if (z3)
                        pset_and_clip(x + x2, y + y2, ((z3 * a) >> 8 << 24) + z);
                }
            }
            break;
        case 2:
            for (y2 = 0; y2 < h; y2++) {
                cp = rt_data + y2 * w;
                for (x2 = 0; x2 < w; x2++) {
                    z3 = *cp++;
                    if (z3 != 255)
                        pset_and_clip(x + x2, y + y2, (((255 - z3) * a2) >> 8 << 24) + z2);
                }
            }
            break;
        default:
            break;
        }
        free(rt_data);
        goto printstring_exit;
    } // custom font

    // default fonts
    static int32_t character, character_c;
    for (character_c = 0; character_c < text->len; character_c++) {
        character = text->chr[character_c];
        if (im->font == 8)
            cp = &charset8x8[character][0][0];
        if (im->font == 14)
            cp = &charset8x16[character][1][0];
        if (im->font == 16)
            cp = &charset8x16[character][0][0];
        switch (im->print_mode) {
        case 3:
            for (y2 = 0; y2 < h; y2++) {
                for (x2 = 0; x2 < 8; x2++) {
                    if (*cp++)
                        pset_and_clip(x + x2, y + y2, color);
                    else
                        pset_and_clip(x + x2, y + y2, background_color);
                }
            }
            break;
        case 1:
            for (y2 = 0; y2 < h; y2++) {
                for (x2 = 0; x2 < 8; x2++) {
                    if (*cp++)
                        pset_and_clip(x + x2, y + y2, color);
                }
            }
            break;
        case 2:
            for (y2 = 0; y2 < h; y2++) {
                for (x2 = 0; x2 < 8; x2++) {
                    if (!(*cp++))
                        pset_and_clip(x + x2, y + y2, background_color);
                }
            }
            break;
        default:
            break;
        }
        x += 8;
    }

printstring_exit:
    if (passed & 2)
        sub__dest(old_dest);
    return;
}

// ============================================================================
// _PRINTWIDTH
// ============================================================================

int32_t func__printwidth(qbs *text, int32_t screenhandle, int32_t passed) {
    // Validate screenhandle
    if (passed) {
        if (screenhandle >= 0) {
            validatepage(screenhandle);
            screenhandle = page[screenhandle];
        } else {
            screenhandle = -screenhandle;
            if (screenhandle >= nextimg) {
                error(258);
                return 0;
            }
            if (!img[screenhandle].valid) {
                error(258);
                return 0;
            }
        }
    } else {
        screenhandle = libqb_get_write_page_index();
    }

    if (text->len == 0)
        return 0;

    if (img[screenhandle].text) {
        return text->len;
    }

    auto fonthandle = img[screenhandle].font;
    auto fwidth = func__fontwidth(fonthandle, 1);
    if (fwidth)
        return fwidth * text->len;

    return FontPrintWidthASCII(font[fonthandle], (const uint8_t *)text->chr, text->len);
}

// ============================================================================
// _LOADFONT
// ============================================================================

int32_t func__loadfont(const qbs *qbsFileName, int32_t size, const qbs *qbsRequirements, int32_t font_index, int32_t passed) {
    if (is_error_pending() || !qbsFileName->len)
        return INVALID_FONT_HANDLE;

    if (size < 1) {
        error(5);
        return INVALID_FONT_HANDLE;
    }

    auto isLoadFromMemory = false;
    int32_t options = 0;

    if ((passed & 1) && qbsRequirements->len) {
        std::string requirements(reinterpret_cast<char *>(qbsRequirements->chr), qbsRequirements->len);
        std::transform(requirements.begin(), requirements.end(), requirements.begin(), [](unsigned char c) { return std::toupper(c); });

        image_log_trace("Parsing requirements string: %s", requirements.c_str());

        if (requirements.find("DONTBLEND") != std::string::npos) {
            options |= FONT_LOAD_DONTBLEND;
            image_log_trace("No alpha blending requested");
        }

        if (requirements.find("MONOSPACE") != std::string::npos) {
            options |= FONT_LOAD_MONOSPACE;
            image_log_trace("Monospaced font requested");
        }

        if (requirements.find("UNICODE") != std::string::npos) {
            options |= FONT_LOAD_UNICODE;
            image_log_trace("Unicode requested");
        }

        if (requirements.find("MEMORY") != std::string::npos) {
            isLoadFromMemory = true;
            image_log_trace("Loading from memory requested");
        }

        if (requirements.find("AUTOMONO") != std::string::npos) {
            options |= FONT_LOAD_AUTOMONO;
            image_log_trace("Automatic monospacing requested");
        }
    }

    if (passed & 2) {
        image_log_trace("Loading font index %i", font_index);
    } else {
        image_log_trace("Loading default font index (0)");
        font_index = 0;
    }

    uint8_t *content;
    int32_t bytes;

    if (isLoadFromMemory) {
        content = qbsFileName->chr;
        bytes = qbsFileName->len;
        image_log_trace("Loading font from memory. Size = %i", bytes);
    } else {
        std::string fileName(reinterpret_cast<char *>(qbsFileName->chr), qbsFileName->len);
        std::string fixedPath = filepath_fix_directory(fileName);
        content = FontLoadFileToMemory(fixedPath.c_str(), &bytes);
        image_log_trace("Loading font from file %s", fileName.c_str());
    }

    if (!content)
        return INVALID_FONT_HANDLE;

    int32_t i;
    for (i = 32; i <= lastfont; i++) {
        if (!font[i])
            goto got_font_index;
    }
    lastfont++;
    font = (int32_t *)realloc(font, 4 * (lastfont + 1));
    font[lastfont] = NULL;
    fontheight = (int32_t *)realloc(fontheight, 4 * (lastfont + 1));
    fontwidth = (int32_t *)realloc(fontwidth, 4 * (lastfont + 1));
    fontflags = (int32_t *)realloc(fontflags, 4 * (lastfont + 1));
    i = lastfont;

got_font_index:
    auto h = FontLoad((const uint8_t *)content, bytes, size, font_index, options);

    if (!isLoadFromMemory)
        free(content);

    if (!h)
        return INVALID_FONT_HANDLE;

    font[i] = h;
    fontflags[i] = options;
    fontheight[i] = size;
    fontwidth[i] = FontWidth(h);

    return i;
}

// ============================================================================
// _FONT
// ============================================================================

void sub__font(int32_t f, int32_t i, int32_t passed) {
    int32_t i2 = 0;
    static img_struct *im;
    if (is_error_pending())
        return;
    if (passed & 1) {
        if (i >= 0) {
            validatepage(i);
            i = page[i];
        } else {
            i = -i;
            if (i >= nextimg) {
                error(258);
                return;
            }
            if (!img[i].valid) {
                error(258);
                return;
            }
        }
    } else {
        i = libqb_get_write_page_index();
    }
    im = &img[i];
    // validate f
    if ((f == 8) || (f == 9) || ((f > 13) && (f < 18)))
        i2 = 1;
    if (((f == 9) || (f == 15) || (f == 17)) && (!im->text)) {
        error(5);
        return;
    }
    if (f >= 32 && f <= lastfont) {
        if (font[f])
            i2 = 1;
    }
    if (!i2) {
        error(258);
        return;
    }

    if (im->text && ((fontflags[f] & FONT_LOAD_MONOSPACE) == 0)) {
        error(5);
        return;
    }

    if (im->text) {
        if (im->flags & IMG_SCREEN) {
            if (autodisplay) {
                if (lock_display == 0)
                    lock_display = 1;
                while (lock_display != 2)
                    Sleep(0);
            }
            screen_last_valid = 0;
            for (i = 0; i < pages; i++) {
                if (page[i]) {
                    im = &img[page[i]];
                    im->font = f;
                }
            }
            if (autodisplay) {
                if (lock_display_required)
                    lock_display = 0;
            }
            return;
        }
    }

    im->font = f;
    im->cursor_x = 1;
    im->cursor_y = 1;
    im->top_row = 1;
    if (im->compatible_mode)
        im->bottom_row = im->height / fontheight[f];
    else
        im->bottom_row = im->height;
    im->bottom_row--;
    if (im->bottom_row <= 0)
        im->bottom_row = 1;
    return;
}

// ============================================================================
// _FONTWIDTH, _FONTHEIGHT, _FONT (query)
// ============================================================================

int32_t func__fontwidth(int32_t f, int32_t passed) {
    static int32_t i2;
    if (is_error_pending())
        return 0;
    if (passed) {
        i2 = 0;
        if (f == 8)
            i2 = 1;
        if (f == 14)
            i2 = 1;
        if (f == 16)
            i2 = 1;
        if (f >= 32 && f <= lastfont) {
            if (font[f])
                i2 = 1;
        }
        if (!i2) {
            error(258);
            return 0;
        }
    } else {
        f = write_page->font;
    }
    return fontwidth[f];
}

int32_t func__fontheight(int32_t f, int32_t passed) {
    static int32_t i2;
    if (is_error_pending())
        return 0;
    if (passed) {
        i2 = 0;
        if (f == 8)
            i2 = 1;
        if (f == 14)
            i2 = 1;
        if (f == 16)
            i2 = 1;
        if (f >= 32 && f <= lastfont) {
            if (font[f])
                i2 = 1;
        }
        if (!i2) {
            error(258);
            return 0;
        }
    } else {
        f = write_page->font;
    }
    return fontheight[f];
}

int32_t func__font(int32_t i, int32_t passed) {
    if (is_error_pending())
        return 0;
    if (passed) {
        if (i >= 0) {
            validatepage(i);
            i = page[i];
        } else {
            i = -i;
            if (i >= nextimg) {
                error(258);
                return 0;
            }
            if (!img[i].valid) {
                error(258);
                return 0;
            }
        }
    } else {
        i = libqb_get_write_page_index();
    }
    return img[i].font;
}

// ============================================================================
// _FREEFONT
// ============================================================================

void sub__freefont(int32_t f) {
    if (is_error_pending())
        return;
    static int32_t i, i2;
    i2 = 0;
    if (f >= 32 && f <= lastfont) {
        if (font[f])
            i2 = 1;
    }
    if (!i2) {
        error(258);
        return;
    }
    // check all surfaces, no surface can be using the font
    for (i = 1; i < nextimg; i++) {
        if (img[i].valid) {
            if (img[i].font == f) {
                error(5);
                return;
            }
        }
    }
    FontFree(font[f]);
    font[f] = NULL;
}

// ============================================================================
// _PRINTMODE
// ============================================================================

void sub__printmode(int32_t mode, int32_t i, int32_t passed) {
    if (is_error_pending())
        return;
    if (passed) {
        if (i >= 0) {
            validatepage(i);
            i = page[i];
        } else {
            i = -i;
            if (i >= nextimg) {
                error(258);
                return;
            }
            if (!img[i].valid) {
                error(258);
                return;
            }
        }
    } else {
        i = libqb_get_write_page_index();
    }
    if (img[i].text) {
        if (mode != 1) {
            error(5);
            return;
        }
    }
    if (mode == 1)
        img[i].print_mode = 3; // fill
    if (mode == 2)
        img[i].print_mode = 1; // keep
    if (mode == 3)
        img[i].print_mode = 2; // only
}

int32_t func__printmode(int32_t i, int32_t passed) {
    if (is_error_pending())
        return 0;
    if (passed) {
        if (i >= 0) {
            validatepage(i);
            i = page[i];
        } else {
            i = -i;
            if (i >= nextimg) {
                error(258);
                return 0;
            }
            if (!img[i].valid) {
                error(258);
                return 0;
            }
        }
    } else {
        i = libqb_get_write_page_index();
    }
    return img[i].print_mode;
}
