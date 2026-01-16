//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Graphics Module
//  Graphics drawing and rendering functions
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "graphics.h"
#include "cmem.h"
#include "error_handle.h"
#include "extended_math.h"
#include "gui.h"
#include "mem.h"
#include "qblist.h"
#include "qbs.h"
#include "rounding.h"

#include "../../os.h"      // For int32 type definitions
#include "../../common.h"  // For byte_element_struct

#include <cmath>
#include <cstdlib>
#include <cstring>

#ifdef QB64_WINDOWS
#include <windows.h>
#endif

// External functions from libqb.cpp
void validatepage(int32_t pageNumber);
void sub__font(int32_t f, int32_t i, int32_t passed);
void init_blend();

// Render state helpers (remain in libqb.cpp)
extern void set_view(int32_t new_mode);
extern void set_alpha(int32_t new_mode);
extern void set_depthbuffer(int32_t new_mode);
extern void set_cull_mode(int32_t new_mode);
extern int32_t framebufferobjects_supported;
extern int32_t SOFTWARE_IMG_HANDLE_MIN;

// Environment 2D screen scaling (from libqb.cpp)
extern float environment_2d__screen_x_scale;
extern float environment_2d__screen_y_scale;
extern int32_t environment_2d__screen_x1;
extern int32_t environment_2d__screen_y1;
extern int32_t environment__window_width;
extern int32_t environment__window_height;

struct environment_2d__window_rect_struct {
    int32_t x1;
    int32_t y1;
    int32_t x2;
    int32_t y2;
};
extern environment_2d__window_rect_struct *environment_2d__screen_to_window_rect(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

// Global variables from libqb.cpp
extern list *hardware_graphics_command_handles;
extern int64_t display_frame_order_next;
extern int32_t first_hardware_command;
extern int32_t last_hardware_command_added;
extern int32_t last_hardware_command_rendered;
extern int32_t next_hardware_command_to_remove;
extern int32_t nextimg;
extern int32_t *page;
extern img_struct *img;
extern img_struct *write_page;
extern img_struct *read_page;
extern img_struct *display_page;
extern int32_t write_page_index;
extern int32_t read_page_index;
extern int32_t display_page_index;
extern uint8_t *cblend;
extern uint8_t *ablend;
extern uint8_t *ablend127;
extern uint8_t *ablend128;

// Image management globals from libqb.cpp
extern int32_t nimg;
extern uint32_t *fimg;
extern int32_t nfimg;
extern int32_t lastfimg;
extern uint32_t palette_256[256];
extern uint32_t palette_64[64];

// Image buffer size constant
#define IMG_BUFFERSIZE 4096

// External helper functions for drawing primitives (remain in libqb.cpp)
void fast_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);
void fast_boxfill(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);

// External qbg_* variables for drawing
extern int32_t qbg_text_only;
extern int32_t qbg_width, qbg_height;
extern int32_t qbg_view_x1, qbg_view_y1, qbg_view_x2, qbg_view_y2;
extern uint8_t *qbg_active_page_offset;

// Module-level global variables
static int32_t depthbuffer_mode0 = DEPTHBUFFER_MODE__ON;
static int32_t depthbuffer_mode1 = DEPTHBUFFER_MODE__ON;

// ============================================================================
// SOFTWARE IMAGE MANAGEMENT FUNCTIONS
// Extracted from libqb.cpp - manages software image lifecycle
// ============================================================================

void restorepalette(img_struct *im) {
    static uint32_t *pal;
    if (im->bytes_per_pixel == 4)
        return;
    pal = im->pal;

    switch (im->compatible_mode) {

    case 1:
        /*
            SCREEN Mode 1 Syntax:  COLOR [background][,palette]
            - background is the screen color (range = 0-15)
            - palette is a three-color palette (range = 0-1)
            0 = green, red, and brown         1 = cyan, magenta, and bright white
            Note: option 1 is the default, palette can override these though
            OPTION 1:*DEFAULT*
            0=black(color 0)
            1=cyan(color 3)
            2=purple(color 5)
            3=light grey(color 7)
            OPTION 0:
            0=black(color 0)
            1=green(color 2)
            2=red(color 4)
            3=brown(color 6)
        */
        pal[0] = palette_256[0];
        pal[1] = palette_256[3];
        pal[2] = palette_256[5];
        pal[3] = palette_256[7];
        return;
        break;

    case 2: // black/white 2 color palette
        pal[0] = 0;
        pal[1] = 0xFFFFFF;
        return;
        break;

    case 9: // 16 colors selected from 64 possibilities
        pal[0] = palette_64[0];
        pal[1] = palette_64[1];
        pal[2] = palette_64[2];
        pal[3] = palette_64[3];
        pal[4] = palette_64[4];
        pal[5] = palette_64[5];
        pal[6] = palette_64[20];
        pal[7] = palette_64[7];
        pal[8] = palette_64[56];
        pal[9] = palette_64[57];
        pal[10] = palette_64[58];
        pal[11] = palette_64[59];
        pal[12] = palette_64[60];
        pal[13] = palette_64[61];
        pal[14] = palette_64[62];
        pal[15] = palette_64[63];
        return;
        break;

    case 10: // 4 colors selected from 9 possibilities (indexes held in array pal[4-7])
        pal[4] = 0;
        pal[5] = 4;
        pal[6] = 6;
        pal[7] = 8;
        return;
        break;

    case 11: // black/white 2 color palette
        pal[0] = 0;
        pal[1] = 0xFFFFFF;
        return;
        break;

    case 13:
        memcpy(pal, palette_256, 1024);
        return;
        break;

    case 256:
        memcpy(pal, palette_256, 1024);
        return;
        break;

    default:
        // default 16 color palette
        memcpy(pal, palette_256, 64);

    }; // switch

} // restorepalette

void pset(int32_t x, int32_t y, uint32_t col) {
    static uint8_t *cp;
    static uint32_t *o32;
    static uint32_t destcol;
    if (write_page->bytes_per_pixel == 1) {
        write_page->offset[y * write_page->width + x] = col & write_page->mask;
        return;
    } else {
        if (write_page->alpha_disabled) {
            write_page->offset32[y * write_page->width + x] = col;
            return;
        }
        switch (col & 0xFF000000) {
        case 0xFF000000: // 100% alpha, so regular pset (fast)
            write_page->offset32[y * write_page->width + x] = col;
            return;
            break;
        case 0x0: // 0%(0) alpha, so no pset (very fast)
            return;
            break;
        case 0x80000000: //~50% alpha (optimized)

            o32 = write_page->offset32 + (y * write_page->width + x);
            *o32 = (((*o32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*o32 >> 24] << 24);
            return;
            break;
        case 0x7F000000: //~50% alpha (optimized)
            o32 = write_page->offset32 + (y * write_page->width + x);
            *o32 = (((*o32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*o32 >> 24] << 24);
            return;
            break;
        default: // other alpha values (uses a lookup table)
            o32 = write_page->offset32 + (y * write_page->width + x);
            destcol = *o32;
            cp = cblend + (col >> 24 << 16);
            *o32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                   (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
        };
    }
}

// returns an index to free img structure
uint32_t newimg() {
    static int32_t i;
    if (lastfimg != -1) {
        i = fimg[lastfimg--];
        goto gotindex;
    }
    if (nextimg < nimg) {
        i = nextimg++;
        goto gotindex;
    }
    img = (img_struct *)realloc(img, (nimg + IMG_BUFFERSIZE) * sizeof(img_struct));
    if (!img)
        error(502);
    // update existing img pointers to new locations
    display_page = &img[display_page_index];
    write_page = &img[write_page_index];
    read_page = &img[read_page_index];
    memset(&img[nimg], 0, IMG_BUFFERSIZE * sizeof(img_struct));
    nimg += IMG_BUFFERSIZE;
    i = nextimg++;
gotindex:
    img[i].valid = 1;
    return i;
}

int32_t freeimg(uint32_t i) {
    // returns: 0=failed, 1=success
    if (i >= (uint32_t)nimg)
        return 0;
    if (!img[i].valid)
        return 0;
    if (lastfimg >= (nfimg - 1)) { // extend
        fimg = (uint32_t *)realloc(fimg, (nfimg + IMG_BUFFERSIZE) * 4);
        if (!fimg)
            error(503);
        nfimg += IMG_BUFFERSIZE;
    }
    if (img[i].lock_id) {
        free_mem_lock((mem_lock *)img[i].lock_offset); // untag
    }
    memset(&img[i], 0, sizeof(img_struct));
    lastfimg++;
    fimg[lastfimg] = i;
    return 1;
}

void imgrevert(int32_t i) {
    static int32_t bpp;
    static img_struct *im;

    im = &img[i];
    bpp = im->compatible_mode;

    // revert to assumed default values
    im->bytes_per_pixel = 1;
    im->font = 16;
    im->color = 15;
    im->print_mode = 3;
    im->background_color = 0;
    im->draw_ta = 0.0;
    im->draw_scale = 1.0;

    // revert to mode's set values
    switch (bpp) {
    case 0:
        im->bits_per_pixel = 16;
        im->bytes_per_pixel = 2;
        im->color = 7;
        im->text = 1;
        im->cursor_show = 0;
        im->cursor_firstvalue = 4;
        im->cursor_lastvalue = 4;
        break;
    case 1:
        im->bits_per_pixel = 2;
        im->font = 8;
        im->color = 3;
        break;
    case 2:
        im->bits_per_pixel = 1;
        im->font = 8; // it gets stretched from 8 to 16 later
        im->color = 1;
        break;
    case 7:
        im->bits_per_pixel = 4;
        im->font = 8;
        break;
    case 8:
        im->bits_per_pixel = 4;
        im->font = 8;
        break;
    case 9:
        im->bits_per_pixel = 4;
        im->font = 14;
        break;
    case 10:
        im->bits_per_pixel = 2;
        im->font = 14;
        im->color = 3;
        break;
    case 11:
        im->bits_per_pixel = 1;
        im->color = 1;
        break;
    case 12:
        im->bits_per_pixel = 4;
        break;
    case 13:
        im->bits_per_pixel = 8;
        im->font = 8;
        break;
    case 256:
        im->bits_per_pixel = 8;
        break;
    case 32:
        im->bits_per_pixel = 32;
        im->bytes_per_pixel = 4;
        im->color = 0xFFFFFFFF;
        im->background_color = 0xFF000000;
        break;
    };
    im->draw_color = im->color;

    // revert palette
    if (bpp != 32) {
        restorepalette(im);
        im->transparent_color = -1;
    }

    // revert calculatable values
    if (im->bits_per_pixel < 32)
        im->mask = (1 << im->bits_per_pixel) - 1;
    else
        im->mask = 0xFFFFFFFF;
    // text
    im->cursor_x = 1;
    im->cursor_y = 1;
    im->top_row = 1;
    if (bpp)
        im->bottom_row = (im->height / im->font);
    else
        im->bottom_row = im->height;
    im->bottom_row--;
    if (im->bottom_row <= 0)
        im->bottom_row = 1;
    if (!bpp)
        return;
    // graphics
    // clipping/scaling
    im->x = ((double)im->width) / 2.0;
    im->y = ((double)im->height) / 2.0;
    im->view_x2 = im->width - 1;
    im->view_y2 = im->height - 1;
    im->scaling_x = 1;
    im->scaling_y = 1;
    im->window_x2 = im->view_x2;
    im->window_y2 = im->view_y2;

    // clear
    if (bpp) { // graphics
        memset(im->offset, 0, im->width * im->height * im->bytes_per_pixel);
    } else { // text
        static int32_t i2, i3;
        static uint16_t *sp;
        i3 = im->width * im->height;
        sp = (uint16_t *)im->offset;
        for (i2 = 0; i2 < i3; i2++) {
            *sp++ = 0x0720;
        }
    }

} // imgrevert

int32_t imgframe(uint8_t *o, int32_t x, int32_t y, int32_t bpp) {
    static int32_t i;
    static img_struct *im;
    if (x <= 0 || y <= 0)
        return 0;
    i = newimg();
    im = &img[i];
    im->offset = o;
    im->width = x;
    im->height = y;

    // assume default values
    im->bytes_per_pixel = 1;
    im->font = 16;
    im->color = 15;
    im->compatible_mode = bpp;
    im->print_mode = 3;
    im->draw_ta = 0.0;
    im->draw_scale = 1.0;

    // set values
    switch (bpp) {
    case 0:
        im->bits_per_pixel = 16;
        im->bytes_per_pixel = 2;
        im->color = 7;
        im->text = 1;
        im->cursor_show = 0;
        im->cursor_firstvalue = 4;
        im->cursor_lastvalue = 4;
        break;
    case 1:
        im->bits_per_pixel = 2;
        im->font = 8;
        im->color = 3;
        break;
    case 2:
        im->bits_per_pixel = 1;
        im->font = 8; // it gets stretched from 8 to 16 later
        im->color = 1;
        break;
    case 7:
        im->bits_per_pixel = 4;
        im->font = 8;
        break;
    case 8:
        im->bits_per_pixel = 4;
        im->font = 8;
        break;
    case 9:
        im->bits_per_pixel = 4;
        im->font = 14;
        break;
    case 10:
        im->bits_per_pixel = 2;
        im->font = 14;
        im->color = 3;
        break;
    case 11:
        im->bits_per_pixel = 1;
        im->color = 1;
        break;
    case 12:
        im->bits_per_pixel = 4;
        break;
    case 13:
        im->bits_per_pixel = 8;
        im->font = 8;
        break;
    case 256:
        im->bits_per_pixel = 8;
        break;
    case 32:
        im->bits_per_pixel = 32;
        im->bytes_per_pixel = 4;
        im->color = 0xFFFFFFFF;
        im->background_color = 0xFF000000;
        break;
    default:
        return 0;
    };
    im->draw_color = im->color;

    // attach palette
    if (bpp != 32) {
        im->pal = (uint32_t *)calloc(256, 4);
        if (!im->pal) {
            freeimg(i);
            return 0;
        }
        im->flags |= IMG_FREEPAL;
        restorepalette(im);
        im->transparent_color = -1;
    }

    // set calculatable values
    if (im->bits_per_pixel < 32)
        im->mask = (1 << im->bits_per_pixel) - 1;
    else
        im->mask = 0xFFFFFFFF;
    // text
    im->cursor_x = 1;
    im->cursor_y = 1;
    im->top_row = 1;
    if (bpp)
        im->bottom_row = (im->height / im->font);
    else
        im->bottom_row = im->height;
    im->bottom_row--;
    if (im->bottom_row <= 0)
        im->bottom_row = 1;
    if (!bpp)
        return i;
    // graphics
    // clipping/scaling
    im->x = ((double)im->width) / 2.0;
    im->y = ((double)im->height) / 2.0;
    im->view_x2 = im->width - 1;
    im->view_y2 = im->height - 1;
    im->scaling_x = 1;
    im->scaling_y = 1;
    im->window_x2 = im->view_x2;
    im->window_y2 = im->view_y2;

    return i;
}

int32_t imgnew(int32_t x, int32_t y, int32_t bpp) {
    static int32_t i, i2, i3;
    static img_struct *im;
    static uint16_t *sp;
    static uint32_t *lp;
    i = imgframe(NULL, x, y, bpp);
    if (!i)
        return 0;
    im = &img[i];
    if (bpp) { // graphics
        if (bpp == 32) {
            if (!cblend)
                init_blend();
            im->offset = (uint8_t *)calloc(x * y, 4);
            if (!im->offset) {
                sub__freeimage(-i, 1);
                return 0;
            }
            // i3=x*y; lp=im->offset32; for (i2=0;i2<i3;i2++){*lp++=0xFF000000;}
        } else {
            im->offset = (uint8_t *)calloc(x * y * im->bytes_per_pixel, 1);
            if (!im->offset) {
                sub__freeimage(-i, 1);
                return 0;
            }
        }
    } else { // text
        im->offset = (uint8_t *)malloc(x * y * im->bytes_per_pixel);
        if (!im->offset) {
            sub__freeimage(-i, 1);
            return 0;
        }
        i3 = x * y;
        sp = (uint16_t *)im->offset;
        for (i2 = 0; i2 < i3; i2++) {
            *sp++ = 0x0720;
        }
    }
    im->flags |= IMG_FREEMEM;
    return i;
}

// ============================================================================
// HARDWARE TEXTURE GLOBAL STATE
// ============================================================================

// Render state management
RENDER_STATE_GLOBAL render_state;
RENDER_STATE_DEST dest_render_state0;

// Hardware image handles list
list *hardware_img_handles = NULL;

// NPO2 (Non-Power-of-2) texture support
int32_t force_NPO2_fix = 0;  // Only set to 1 for debugging QB64
uint32_t *NPO2_buffer = NULL;
int32_t NPO2_buffer_size_in_pixels = 0;

// Vertex buffer for hardware rendering
float *hardware_buffer_vertices = NULL;
int32_t hardware_buffer_vertices_max = 0;
int32_t hardware_buffer_vertices_count = 0;
float *hardware_buffer_texcoords = NULL;
int32_t hardware_buffer_texcoords_max = 0;
int32_t hardware_buffer_texcoords_count = 0;

/**
 * Converts HSB (Hue, Saturation, Brightness) color to RGB.
 * 
 * Converts a color from the HSB color space to RGB color space.
 * Hue is in degrees (0-360), saturation and brightness are normalized (0.0-1.0).
 * 
 * @param hsb Pointer to source HSB color structure
 * @param rgb Pointer to destination RGB color structure
 */
void hsb2rgb(hsb_color *hsb, rgb_color *rgb) {
    double hu, hi, hf, pv, qv, tv;

    if (hsb->s == 0.0) {
        rgb->r = hsb->b; rgb->g = hsb->b; rgb->b = hsb->b; // no saturation = grayscale
    } else {
        hu = hsb->h / 60.0;  // to sixtant [0,5]
        if (hu >= 6.0) hu = hu - 6.0;
        hf = modf(hu, &hi);  // int/frac parts of hue
        pv = hsb->b * (1.0 - hsb->s);
        qv = hsb->b * (1.0 - (hsb->s * hf));
        tv = hsb->b * (1.0 - (hsb->s * (1.0 - hf)));
        switch (lround(hi)) {
            case 0: {rgb->r = hsb->b; rgb->g = tv; rgb->b = pv; break;} //   0- 60 = Red->Yellow
            case 1: {rgb->r = qv; rgb->g = hsb->b; rgb->b = pv; break;} //  60-120 = Yellow->Green
            case 2: {rgb->r = pv; rgb->g = hsb->b; rgb->b = tv; break;} // 120-180 = Green->Cyan
            case 3: {rgb->r = pv; rgb->g = qv; rgb->b = hsb->b; break;} // 180-240 = Cyan->Blue
            case 4: {rgb->r = tv; rgb->g = pv; rgb->b = hsb->b; break;} // 240-300 = Blue->Magenta
            case 5: {rgb->r = hsb->b; rgb->g = pv; rgb->b = qv; break;} // 300-360 = Magenta->Red
        }
    }
}

/**
 * Converts RGB color to HSB (Hue, Saturation, Brightness).
 * 
 * Converts a color from the RGB color space to HSB color space.
 * RGB values are normalized (0.0-1.0). Hue is returned in degrees (0-360),
 * saturation and brightness are normalized (0.0-1.0).
 * 
 * @param rgb Pointer to source RGB color structure
 * @param hsb Pointer to destination HSB color structure
 */
void rgb2hsb(rgb_color *rgb, hsb_color *hsb) {
    double mini, maxi, diff, hu;
    // --- find min/max and difference ---
    mini = fmin(fmin(rgb->r, rgb->g), rgb->b);
    maxi = fmax(fmax(rgb->r, rgb->g), rgb->b);
    diff = maxi - mini;
    // --- brightness ---
    hsb->b = maxi;
    // --- saturation (avoid division by zero) ---
    maxi != 0.0 ? hsb->s = diff / maxi : hsb->s = 0.0;
    // --- hue in degrees ---
    if (hsb->s != 0.0) {
        if (rgb->r == maxi) {
            hu = ((rgb->g - rgb->b) / diff);       // between Yellow & Magenta
            if (hu < 0.0) hu = hu + 6.0;
        } else if (rgb->g == maxi) {
            hu = 2.0 + ((rgb->b - rgb->r) / diff); // between Cyan & Yellow
        } else {
            hu = 4.0 + ((rgb->r - rgb->g) / diff); // between Magenta & Cyan
        }
        hsb->h = hu * 60.0; // to degrees
    } else {
        hsb->h = 0.0; // technically there's no hue w/o saturation, commonly used is 0 (red)
    }
}

/**
 * Creates a 32-bit ARGB color value from HSB components.
 * 
 * Converts hue (0-360 degrees), saturation (0-100%), and brightness (0-100%)
 * to a 32-bit ARGB color value with full alpha (0xFF000000).
 * 
 * @param hue Hue in degrees (0-360, clamped)
 * @param sat Saturation percentage (0-100, clamped)
 * @param bri Brightness percentage (0-100, clamped)
 * @return 32-bit ARGB color value with alpha set to 0xFF
 */
uint32_t func__hsb32(double hue, double sat, double bri) {
    hsb_color hsb; rgb_color rgb;
    // --- prepare values for conversion ---
    (hue < 0.0) ? hsb.h = 0.0 : ((hue > 360.0) ? hsb.h = 360.0 : hsb.h = hue);
    (sat < 0.0) ? hsb.s = 0.0 : ((sat > 100.0) ? hsb.s = 100.0 : hsb.s = sat);
    (bri < 0.0) ? hsb.b = 0.0 : ((bri > 100.0) ? hsb.b = 100.0 : hsb.b = bri);
    hsb.s /= 100.0; hsb.b /= 100.0; // range [0,1]
    // --- convert colorspace ---
    hsb2rgb(&hsb, &rgb);
    // --- build result ---
    return ((lround(rgb.r * 255.0) << 16) + (lround(rgb.g * 255.0) << 8) + lround(rgb.b * 255.0)) | 0xFF000000;
}

/**
 * Creates a 32-bit ARGB color value from HSBA components.
 * 
 * Converts hue (0-360 degrees), saturation (0-100%), brightness (0-100%),
 * and alpha (0-100%) to a 32-bit ARGB color value.
 * 
 * @param hue Hue in degrees (0-360, clamped)
 * @param sat Saturation percentage (0-100, clamped)
 * @param bri Brightness percentage (0-100, clamped)
 * @param alf Alpha percentage (0-100, clamped)
 * @return 32-bit ARGB color value
 */
uint32_t func__hsba32(double hue, double sat, double bri, double alf) {
    hsb_color hsb; rgb_color rgb; double alpha;
    // --- prepare values for conversion ---
    (hue < 0.0) ? hsb.h = 0.0 : ((hue > 360.0) ? hsb.h = 360.0 : hsb.h = hue);
    (sat < 0.0) ? hsb.s = 0.0 : ((sat > 100.0) ? hsb.s = 100.0 : hsb.s = sat);
    (bri < 0.0) ? hsb.b = 0.0 : ((bri > 100.0) ? hsb.b = 100.0 : hsb.b = bri);
    (alf < 0.0) ? alpha = 0.0 : ((alf > 100.0) ? alpha = 100.0 : alpha = alf);
    hsb.s /= 100.0; hsb.b /= 100.0; alpha /= 100.0; // range [0,1]
    // --- convert colorspace ---
    hsb2rgb(&hsb, &rgb);
    // --- build result ---
    return (lround(alpha * 255.0) << 24) + (lround(rgb.r * 255.0) << 16) + (lround(rgb.g * 255.0) << 8) + lround(rgb.b * 255.0);
}

/**
 * Extracts the hue component from a 32-bit ARGB color.
 * 
 * @param argb 32-bit ARGB color value
 * @return Hue in degrees (0-360)
 */
double func__hue32(uint32_t argb) {
    rgb_color rgb; hsb_color hsb;
    // --- prepare values for conversion ---
    rgb.r = ((argb >> 16) & 0xFF) / 255.0;
    rgb.g = ((argb >> 8) & 0xFF) / 255.0;
    rgb.b = (argb & 0xFF) / 255.0;
    // --- convert colorspace ---
    rgb2hsb(&rgb, &hsb);
    // --- build result ---
    return hsb.h;
}

/**
 * Extracts the saturation component from a 32-bit ARGB color.
 * 
 * @param argb 32-bit ARGB color value
 * @return Saturation percentage (0-100)
 */
double func__sat32(uint32_t argb) {
    rgb_color rgb; hsb_color hsb;
    // --- prepare values for conversion ---
    rgb.r = ((argb >> 16) & 0xFF) / 255.0;
    rgb.g = ((argb >> 8) & 0xFF) / 255.0;
    rgb.b = (argb & 0xFF) / 255.0;
    // --- convert colorspace ---
    rgb2hsb(&rgb, &hsb);
    // --- build result ---
    return hsb.s * 100.0;
}

/**
 * Extracts the brightness component from a 32-bit ARGB color.
 * 
 * @param argb 32-bit ARGB color value
 * @return Brightness percentage (0-100)
 */
double func__bri32(uint32_t argb) {
    rgb_color rgb; hsb_color hsb;
    // --- prepare values for conversion ---
    rgb.r = ((argb >> 16) & 0xFF) / 255.0;
    rgb.g = ((argb >> 8) & 0xFF) / 255.0;
    rgb.b = (argb & 0xFF) / 255.0;
    // --- convert colorspace ---
    rgb2hsb(&rgb, &hsb);
    // --- build result ---
    return hsb.b * 100.0;
}

// ============================================================================
// HARDWARE GRAPHICS COMMAND MANAGEMENT
// ============================================================================

/**
 * Flushes old hardware graphics commands that have been rendered.
 *
 * Removes completed hardware graphics commands from the command queue.
 * Also handles FREEIMAGE_REQUEST commands by converting them to FREEIMAGE
 * commands for deferred image cleanup.
 */
void flush_old_hardware_commands() {
    static int32_t old_command;
    static int32_t command_to_remove;
    static hardware_graphics_command_struct *last_rendered_hgc;
    static hardware_graphics_command_struct *old_hgc;
    static hardware_graphics_command_struct *next_hgc;

    if (next_hardware_command_to_remove && last_hardware_command_rendered) {

        last_rendered_hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_rendered);

        old_command = next_hardware_command_to_remove;
        old_hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, old_command);

    remove_next_hgc:

        if (old_hgc->next_command == 0)
            goto cant_remove;
        next_hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, old_hgc->next_command);
        if (next_hgc->order >= last_rendered_hgc->order)
            goto cant_remove;

        command_to_remove = old_command;

        if (old_hgc->command == HARDWARE_GRAPHICS_COMMAND__FREEIMAGE_REQUEST) {
            static hardware_img_struct *himg;
            himg = (hardware_img_struct *)list_get(hardware_img_handles, old_hgc->src_img);
            // add command to free image
            // create new command handle & structure
            int32_t hgch = list_add(hardware_graphics_command_handles);
            hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, hgch);
            hgc->remove = 0;
            // set command values
            hgc->command = HARDWARE_GRAPHICS_COMMAND__FREEIMAGE;
            hgc->src_img = old_hgc->src_img;
            // queue the command
            hgc->next_command = 0;
            hgc->order = display_frame_order_next;
            if (last_hardware_command_added) {
                hardware_graphics_command_struct *hgc2 =
                    (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_added);
                hgc2->next_command = hgch;
            }
            last_hardware_command_added = hgch;
            if (first_hardware_command == 0)
                first_hardware_command = hgch;
        }

        old_command = old_hgc->next_command;
        next_hardware_command_to_remove = old_command;
        old_hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, old_command);
        list_remove(hardware_graphics_command_handles, command_to_remove);

        goto remove_next_hgc;

    cant_remove:;

    } // next_hardware_command_to_remove&&last_hardware_command_rendered
} // flush_old_hardware_commands

/**
 * Controls depth buffer settings for 3D rendering.
 * 
 * Sets depth buffer mode (ON, OFF, LOCK, or _CLEAR) for the specified
 * destination surface. Used for 3D graphics rendering to control
 * depth testing and clearing operations.
 * 
 * @param options Depth buffer option: 1=ON, 2=OFF, 3=LOCK, 4=_CLEAR
 * @param dst Destination image handle (0 or 1 for primary surfaces, negative for hardware images)
 * @param passed Bit flags indicating which parameters were provided
 */
void sub__depthbuffer(int32_t options, int32_t dst, int32_t passed) {
    //                    {ON|OFF|LOCK|_CLEAR}

    if (is_error_pending())
        return;

    if ((passed & 1) == 0)
        dst = 0; // the primary hardware surface is implied
    hardware_img_struct *dst_himg = NULL;
    if (dst < 0) {
        dst_himg = (hardware_img_struct *)list_get(hardware_img_handles, dst - HARDWARE_IMG_HANDLE_OFFSET);
        if (dst_himg == NULL) {
            error(QB_ERROR_INVALID_HANDLE);
            return;
        }
        dst -= HARDWARE_IMG_HANDLE_OFFSET;
    } else {
        if (dst > 1) {
            error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
            return;
        }
        dst = -dst;
    }

    if (options == 4) {
        flush_old_hardware_commands();
        int32_t hgch = list_add(hardware_graphics_command_handles);
        hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, hgch);
        hgc->remove = 0;
        // set command values
        hgc->command = HARDWARE_GRAPHICS_COMMAND__CLEAR_DEPTHBUFFER;
        hgc->dst_img = dst;
        // queue the command
        hgc->next_command = 0;
        hgc->order = display_frame_order_next;
        if (last_hardware_command_added) {
            hardware_graphics_command_struct *hgc2 =
                (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_added);
            hgc2->next_command = hgch;
        }
        last_hardware_command_added = hgch;
        if (first_hardware_command == 0)
            first_hardware_command = hgch;
        return;
    }

    int32_t new_mode;
    if (options == 1) {
        new_mode = DEPTHBUFFER_MODE__ON;
    }
    if (options == 2) {
        new_mode = DEPTHBUFFER_MODE__OFF;
    }
    if (options == 3) {
        new_mode = DEPTHBUFFER_MODE__LOCKED;
    }

    if (dst == 0) {
        depthbuffer_mode0 = new_mode;
        return;
    }
    if (dst == -1) {
        depthbuffer_mode1 = new_mode;
        return;
    }
    dst_himg->depthbuffer_mode = new_mode;
}

/**
 * Maps a textured triangle from a source image to a destination.
 * 
 * Draws a triangle with texture mapping from a source image to a destination.
 * Supports 2D and 3D mapping, culling options, seamless tiling, and smooth scaling.
 * Can work with both software and hardware-accelerated images.
 * 
 * @param cull_options Culling mode: 1=_CLOCKWISE, 2=_ANTICLOCKWISE
 * @param sx1 Source X coordinate of first vertex
 * @param sy1 Source Y coordinate of first vertex
 * @param sx2 Source X coordinate of second vertex
 * @param sy2 Source Y coordinate of second vertex
 * @param sx3 Source X coordinate of third vertex
 * @param sy3 Source Y coordinate of third vertex
 * @param si Source image handle (0 for read page)
 * @param fdx1 Destination X coordinate of first vertex
 * @param fdy1 Destination Y coordinate of first vertex
 * @param fdz1 Destination Z coordinate of first vertex (for 3D)
 * @param fdx2 Destination X coordinate of second vertex
 * @param fdy2 Destination Y coordinate of second vertex
 * @param fdz2 Destination Z coordinate of second vertex (for 3D)
 * @param fdx3 Destination X coordinate of third vertex
 * @param fdy3 Destination Y coordinate of third vertex
 * @param fdz3 Destination Z coordinate of third vertex (for 3D)
 * @param di Destination image handle (0 for write page)
 * @param smooth_options Smooth scaling: 1=_SMOOTH, 2=_SMOOTHSHRUNK, 3=_SMOOTHSTRETCHED
 * @param passed Bit flags indicating which parameters were provided
 */
void sub__maptriangle(int32_t cull_options, float sx1, float sy1, float sx2, float sy2, float sx3, float sy3, int32_t si, float fdx1, float fdy1, float fdz1,
                      float fdx2, float fdy2, float fdz2, float fdx3, float fdy3, float fdz3, int32_t di, int32_t smooth_options, int32_t passed) {
    //[{_CLOCKWISE|_ANTICLOCKWISE}][{_SEAMLESS}](?,?)-(?,?)-(?,?)[,?]{TO}(?,?[,?])-(?,?[,?])-(?,?[,?])[,[?][,{_SMOOTH|_SMOOTHSHRUNK|_SMOOTHSTRETCHED}]]"
    //  (1)       (2)              1                             2           4         8         16    32   (1)     (2)           (3)

    if (is_error_pending())
        return;

    static int32_t dwidth, dheight, swidth, sheight, swidth2, sheight2;
    static int32_t lhs, rhs, lhs1, lhs2, top, bottom, flats, flatg, final, tile, no_edge_overlap;
    flats = 0;
    final = 0;
    tile = 0;
    no_edge_overlap = 0;
    static int32_t v, i, x, x1, x2, y, y1, y2, z, h, ti, lhsi, rhsi, d;
    static int32_t g1x, g2x, g1tx, g2tx, g1ty, g2ty, g1xi, g2xi, g1txi, g2txi, g1tyi, g2tyi, tx, ty, txi, tyi, roff, loff;
    static int64_t i64;
    static img_struct *src, *dst;
    static uint8_t *pixel_offset;
    static uint32_t *pixel_offset32;
    static uint8_t *dst_offset;
    static uint32_t *dst_offset32;
    static uint8_t *src_offset;
    static uint32_t *src_offset32;
    static uint32_t col, destcol;
    static uint8_t *cp;

    // hardware support
    // is source a hardware handle?
    if (si) {

        static int32_t src, dst; // scope is a wonderful thing
        src = si;
        dst = di;
        hardware_img_struct *src_himg = (hardware_img_struct *)list_get(hardware_img_handles, src - HARDWARE_IMG_HANDLE_OFFSET);
        if (src_himg != NULL) { // source is hardware image
            src -= HARDWARE_IMG_HANDLE_OFFSET;

            flush_old_hardware_commands();

            // check dst
            hardware_img_struct *dst_himg = NULL;
            if (dst < 0) {
                dst_himg = (hardware_img_struct *)list_get(hardware_img_handles, dst - HARDWARE_IMG_HANDLE_OFFSET);
                if (dst_himg == NULL) {
                    error(QB_ERROR_INVALID_HANDLE);
                    return;
                }
                dst -= HARDWARE_IMG_HANDLE_OFFSET;
            } else {
                if (dst > 1) {
                    error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
                    return;
                }
                dst = -dst;
            }

            static int32_t use3d;
            use3d = 0;
            if (passed & (4 + 8 + 16))
                use3d = 1;

            if ((passed & 1) == 1 && use3d == 0) {
                error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
                return;
            } // seamless not supported for 2D hardware version yet

            // create new command handle & structure
            int32_t hgch = list_add(hardware_graphics_command_handles);
            hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, hgch);

            hgc->remove = 0;

            // set command values
            if (use3d) {
                hgc->command = HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE3D;
                hgc->cull_mode = CULL_MODE__NONE;
                if (cull_options == 1)
                    hgc->cull_mode = CULL_MODE__CLOCKWISE_ONLY;
                if (cull_options == 2)
                    hgc->cull_mode = CULL_MODE__ANTICLOCKWISE_ONLY;
            } else {
                hgc->command = HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE;
            }

            hgc->src_img = src;
            hgc->src_x1 = sx1;
            hgc->src_y1 = sy1;
            hgc->src_x2 = sx2;
            hgc->src_y2 = sy2;
            hgc->src_x3 = sx3;
            hgc->src_y3 = sy3;

            hgc->dst_img = dst;
            hgc->dst_x1 = fdx1;
            hgc->dst_y1 = fdy1;
            hgc->dst_x2 = fdx2;
            hgc->dst_y2 = fdy2;
            hgc->dst_x3 = fdx3;
            hgc->dst_y3 = fdy3;
            if (use3d) {
                hgc->dst_z1 = fdz1;
                hgc->dst_z2 = fdz2;
                hgc->dst_z3 = fdz3;
                if (dst == 0)
                    hgc->depthbuffer_mode = depthbuffer_mode0;
                if (dst == -1)
                    hgc->depthbuffer_mode = depthbuffer_mode1;
                if (dst_himg != NULL) {
                    hgc->depthbuffer_mode = dst_himg->depthbuffer_mode;
                }
            }

            hgc->smooth = smooth_options;

            hgc->use_alpha = 1;
            if (src_himg->alpha_disabled)
                hgc->use_alpha = 0;
            // only consider dest alpha setting if it is a hardware image
            if (dst_himg != NULL) {
                if (dst_himg->alpha_disabled)
                    hgc->use_alpha = 0;
            }

            // queue the command
            hgc->next_command = 0;
            hgc->order = display_frame_order_next;

            if (last_hardware_command_added) {
                hardware_graphics_command_struct *hgc2 =
                    (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_added);
                hgc2->next_command = hgch;
            }
            last_hardware_command_added = hgch;
            if (first_hardware_command == 0)
                first_hardware_command = hgch;

            return;
        }
    }

    if (passed & (4 + 8 + 16)) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    } // 3D not supported using software surfaces

    // recreate old calling convention
    static int32_t passed_original;
    passed_original = passed;
    passed = 0;
    if (passed_original & 1)
        passed += 1;
    if (passed_original & 2)
        passed += 2;
    if (passed_original & 32)
        passed += 4;
    if (passed_original & 64)
        passed += 8;

    static int32_t dx1, dy1, dx2, dy2, dx3, dy3;
    dx1 = qbr_float_to_long(fdx1);
    dy1 = qbr_float_to_long(fdy1);
    dx2 = qbr_float_to_long(fdx2);
    dy2 = qbr_float_to_long(fdy2);
    dx3 = qbr_float_to_long(fdx3);
    dy3 = qbr_float_to_long(fdy3);

    // get/validate src/dst images
    if (passed & 2) {
        if (si >= 0) { // validate si
            validatepage(si);
            si = page[si];
        } else {
            si = -si;
            if (si >= nextimg) {
                error(QB_ERROR_INVALID_HANDLE);
                return;
            }
            if (!img[si].valid) {
                error(QB_ERROR_INVALID_HANDLE);
                return;
            }
        }
        src = &img[si];
    } else {
        src = read_page;
    }
    if (passed & 4) {
        if (di >= 0) { // validate di
            validatepage(di);
            di = page[di];
        } else {
            di = -di;
            if (di >= nextimg) {
                error(QB_ERROR_INVALID_HANDLE);
                return;
            }
            if (!img[di].valid) {
                error(QB_ERROR_INVALID_HANDLE);
                return;
            }
        }
        dst = &img[di];
    } else {
        dst = write_page;
    }
    if (src->text || dst->text) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    if (src->bytes_per_pixel != dst->bytes_per_pixel) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }

    if (passed & 1)
        no_edge_overlap = 1;

    dwidth = dst->width;
    dheight = dst->height;
    swidth = src->width;
    sheight = src->height;
    swidth2 = swidth << 16;
    sheight2 = sheight << 16;

    struct PointType {
        int32_t x;
        int32_t y;
        int32_t tx;
        int32_t ty;
    };

    static PointType p[4], *p1, *p2, *tp, *tempp;

    struct GradientType {
        int32_t x;
        int32_t xi;
        int32_t tx;
        int32_t ty;
        int32_t txi;
        int32_t tyi;
        int32_t y1;
        int32_t y2;
        //----
        PointType *p1;
        PointType *p2; // needed for clipping above screen
    };

    static GradientType g[4], *tg, *g1, *g2, *g3, *tempg;
    memset(&g, 0, sizeof(GradientType) * 4);

    /*
        'Reference:
        'Fixed point division: a/b -> a*65536/b (using intermediate _INTEGER64)
    */

    /* debugging method
        std::ofstream f;
        char fn[] = "c:\\qb64\\20c.txt";
        f.open(fn, std::ios::app);
        f<<"\n";
        f<<variablename;
        f<<"\n";
        f.close();
    */

    static int32_t limit, limit2, nlimit, nlimit2;

    //----------------------------------------------------------------------------------------------------------------------------------------------------

    limit = 16383;
    limit2 = (limit << 16) + 32678;
    nlimit = -limit;
    nlimit2 = -limit2;

    // convert texture coords to fixed-point & adjust so 0,0 effectively becomes 0.5,0.5 (ie. 32768,32768)
    v = ((int32_t)(sx1 * 65536.0)) + 32768;
    if (v < 16 || v >= swidth2 - 16)
        tile = 1;
    if (v < nlimit2 || v > limit2) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    p[1].tx = v;
    v = ((int32_t)(sx2 * 65536.0)) + 32768;
    if (v < 16 || v >= swidth2 - 16)
        tile = 1;
    if (v < nlimit2 || v > limit2) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    p[2].tx = v;
    v = ((int32_t)(sx3 * 65536.0)) + 32768;
    if (v < 16 || v >= swidth2 - 16)
        tile = 1;
    if (v < nlimit2 || v > limit2) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    p[3].tx = v;
    v = ((int32_t)(sy1 * 65536.0)) + 32768;
    if (v < 16 || v >= sheight2 - 16)
        tile = 1;
    if (v < nlimit2 || v > limit2) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    p[1].ty = v;
    v = ((int32_t)(sy2 * 65536.0)) + 32768;
    if (v < 16 || v >= sheight2 - 16)
        tile = 1;
    if (v < nlimit2 || v > limit2) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    p[2].ty = v;
    v = ((int32_t)(sy3 * 65536.0)) + 32768;
    if (v < 0 || v >= sheight2 - 16)
        tile = 1;
    if (v < nlimit2 || v > limit2) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    p[3].ty = v;

    if (tile) {
        // shifting to positive range is required for tiling | mod on negative coords will fail
        // shifting may also alleviate the need for tiling if(shifted coords fall within textures normal range
        // does texture extend beyond surface dimensions?
        lhs = 2147483647;
        rhs = -2147483648;
        top = 2147483647;
        bottom = -2147483648;
        for (i = 1; i <= 3; i++) {
            tp = &p[i];
            y = tp->ty;
            if (y > bottom)
                bottom = y;
            if (y < top)
                top = y;
            x = tp->tx;
            if (x > rhs)
                rhs = x;
            if (x < lhs)
                lhs = x;
        }
        z = 0;
        if (lhs < 0) {
            // shift texture coords right
            v = ((lhs + 1) / -swidth2 + 1) * swidth2; // offset to move by
            for (i = 1; i <= 3; i++) {
                tp = &p[i];
                tp->tx = tp->tx + v;
                z = 1;
            }
        } else {
            if (lhs >= swidth2) {
                // shift texture coords left
                z = 1;
                v = (lhs / swidth2) * swidth2; // offset to move by
                for (i = 1; i <= 3; i++) {
                    tp = &p[i];
                    tp->tx = tp->tx - v;
                    z = 1;
                }
            }
        }
        if (top < 0) {
            // shift texture coords down
            v = ((top + 1) / -sheight2 + 1) * sheight2; // offset to move by
            for (i = 1; i <= 3; i++) {
                tp = &p[i];
                tp->ty = tp->ty + v;
                z = 1;
            }
        } else {
            if (top >= swidth2) {
                // shift texture coords up
                v = (top / sheight2) * sheight2; // offset to move by
                for (i = 1; i <= 3; i++) {
                    tp = &p[i];
                    tp->ty = tp->ty - v;
                    z = 1;
                }
                z = 1;
            }
        }
        if (z) {
            // reassess need for tiling
            z = 0;
            for (i = 1; i <= 3; i++) {
                tp = &p[i];
                v = tp->tx;
                if (v < 16 || v >= swidth2 - 16) {
                    z = 1;
                    break;
                }
                v = tp->ty;
                if (v < 16 || v >= sheight2 - 16) {
                    z = 1;
                    break;
                }
            }
            if (z == 0)
                tile = 0; // remove tiling flag, this will greatly improve blit speed
        }
    }

    // validate dest points
    if (dx1 < nlimit || dx1 > limit) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    if (dx2 < nlimit || dx2 > limit) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    if (dx3 < nlimit || dx3 > limit) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    if (dy1 < nlimit || dy1 > limit) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    if (dy2 < nlimit || dy2 > limit) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }
    if (dy3 < nlimit || dy3 > limit) {
        error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
        return;
    }

    // setup dest points
    p[1].x = (dx1 << 16) + 32768;
    p[2].x = (dx2 << 16) + 32768;
    p[3].x = (dx3 << 16) + 32768;
    p[1].y = dy1;
    p[2].y = dy2;
    p[3].y = dy3;

    // get dest extents
    lhs = 2147483647;
    rhs = -2147483648;
    top = 2147483647;
    bottom = -2147483648;
    for (i = 1; i <= 3; i++) {
        tp = &p[i];
        y = tp->y;
        if (y > bottom)
            bottom = y;
        if (y < top)
            top = y;
        if (tp->x < 0)
            x = (tp->x - 65535) / 65536;
        else
            x = tp->x / 65536;
        if (x > rhs)
            rhs = x;
        if (x < lhs)
            lhs = x;
    }

    if (bottom < 0 || top >= dheight || rhs < 0 || lhs >= dwidth)
        return; // clip entire triangle

    for (i = 1; i <= 3; i++) {
        tg = &g[i];
        p1 = &p[i];
        if (i == 3)
            p2 = &p[1];
        else
            p2 = &p[i + 1];

        if (p1->y > p2->y) {
            tempp = p1;
            p1 = p2;
            p2 = tempp;
        }
        tg->tx = p1->tx;
        tg->ty = p1->ty; // starting co-ordinates
        tg->x = p1->x;
        tg->y1 = p1->y;
        tg->y2 = p2->y; // top & bottom
        h = tg->y2 - tg->y1;
        if (h == 0) {
            flats = flats + 1; // number of flat(horizontal) gradients
            flatg = i;         // last detected flat gradient
        } else {
            tg->xi = (p2->x - p1->x) / h;
            tg->txi = (p2->tx - p1->tx) / h;
            tg->tyi = (p2->ty - p1->ty) / h;
        }
        tg->p2 = p2;
        tg->p1 = p1;
    }

    g1 = &g[1];
    g2 = &g[2];
    g3 = &g[3];
    if (flats == 0) {
        // select 2 top-most gradients
        if (g3->y1 < g1->y1) {
            tempg = g1;
            g1 = g3;
            g3 = tempg;
        }
        if (g3->y1 < g2->y1) {
            tempg = g2;
            g2 = g3;
            g3 = tempg;
        }
    } else {
        if (flats == 1) {
            // select the only 2 non-flat gradients available
            if (flatg == 1) {
                tempg = g1;
                g1 = g3;
                g3 = tempg;
            }
            if (flatg == 2) {
                tempg = g2;
                g2 = g3;
                g3 = tempg;
            }
            final = 1; // don't check for continuation
        } else {
            // 3 flats
            // create a row from the leftmost to rightmost point
            // find leftmost-rightmost points
            lhs = 2147483647;
            rhs = -2147483648;
            for (ti = 1; ti <= 3; ti++) {
                x = p[ti].x;
                if (x <= lhs) {
                    lhs = x;
                    lhsi = ti;
                }
                if (x >= rhs) {
                    rhs = x;
                    rhsi = ti;
                }
            }
            // build (dummy) gradients
            g[1].x = lhs;
            g[2].x = rhs;
            g[1].tx = p[lhsi].tx;
            g[1].ty = p[lhsi].ty;
            g[2].tx = p[rhsi].tx;
            g[2].ty = p[rhsi].ty;
            final = 1; // don't check for continuation
        }
    }

    y1 = g1->y1;
    if (g1->y2 > g2->y2)
        y2 = g2->y2;
    else
        y2 = g1->y2;

    // compare the mid-point x-coords of both runs to determine which is on the left & right
    y = y2 - y1;
    lhs1 = g1->x + (g1->xi * y) / 2;
    lhs2 = g2->x + (g2->xi * y) / 2;
    if (lhs1 > lhs2) {
        tempg = g1;
        g1 = g2;
        g2 = tempg;
    }

    //----------------------------------------------------------------------------------------------------------------------------------------------------

    if (src->bytes_per_pixel == 4) {
        dst_offset32 = dst->offset32;
        src_offset32 = src->offset32;
        if (src->alpha_disabled || dst->alpha_disabled) {
            if (tile) {

            mtri1t_usegrad3:;

                if (final == 1) {
                    if (no_edge_overlap)
                        y2 = y2 - 1;
                }

                // not on screen?
                if (y1 >= dheight) {
                    return;
                }
                if (y2 < 0) {
                    if (final)
                        return;
                    // jump to y2's position
                    // note; original point locations are referenced because they are unmodified
                    // & represent the true distance of the run
                    y = y2 - y1;
                    p1 = g1->p1;
                    p2 = g1->p2;
                    d = g1->y2 - g1->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g1->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g1->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g1->x += i64 * y / d;
                        p1 = g2->p1;
                        p2 = g2->p2;
                    }
                    d = g2->y2 - g2->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g2->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g2->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g2->x += i64 * y / d;
                    }
                    goto mtri1t_final;
                }

                // clip top
                if (y1 < 0) {
                    // note; original point locations are referenced because they are unmodified
                    // & represent the true distance of the run
                    y = -y1;
                    p1 = g1->p1;
                    p2 = g1->p2;
                    d = g1->y2 - g1->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g1->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g1->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g1->x += i64 * y / d;
                        p1 = g2->p1;
                        p2 = g2->p2;
                    }
                    d = g2->y2 - g2->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g2->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g2->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g2->x += i64 * y / d;
                    }
                    y1 = 0;
                }

                if (y2 >= dheight) { // clip bottom
                    y2 = dheight - 1;
                }

                // move indexed variable values into direct variables for faster referencing
                // within 2nd bottleneck
                g1x = g1->x;
                g2x = g2->x;
                g1tx = g1->tx;
                g2tx = g2->tx;
                g1ty = g1->ty;
                g2ty = g2->ty;
                g1xi = g1->xi;
                g2xi = g2->xi;
                g1txi = g1->txi;
                g2txi = g2->txi;
                g1tyi = g1->tyi;
                g2tyi = g2->tyi;

                // 2nd bottleneck
                for (y = y1; y <= y2; y++) {

                    if (g1x < 0)
                        x1 = (g1x - 65535) / 65536;
                    else
                        x1 = g1x / 65536; // int-style rounding of fixed-point value
                    if (g2x < 0)
                        x2 = (g2x - 65535) / 65536;
                    else
                        x2 = g2x / 65536;

                    if (x1 >= dwidth || x2 < 0)
                        goto mtri1t_donerow; // crop if(entirely offscreen

                    tx = g1tx;
                    ty = g1ty;

                    // calculate gradients if they might be required
                    if (x1 != x2) {
                        d = g2x - g1x;
                        i64 = g2tx - g1tx;
                        txi = (i64 << 16) / d;
                        i64 = g2ty - g1ty;
                        tyi = (i64 << 16) / d;
                    } else {
                        txi = 0;
                        tyi = 0;
                    }

                    // calculate pixel offsets from ideals
                    loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                                    // values
                    roff = ((g2x & 65535) - 32768);

                    if (roff < 0) {                                // not enough of rhs pixel exists to use
                        if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                            // draw rhs pixel as is
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                            *(dst_offset32 + (y * dwidth + x2)) = src_offset32[((g2ty >> 16) % sheight) * swidth + ((g2tx >> 16) % swidth)];
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        }
                        // move left one position
                        x2--;
                        if (x1 > x2 || x2 < 0)
                            goto mtri1t_donerow; // no more to do
                    } else {
                        if (no_edge_overlap) {
                            x2 = x2 - 1;
                            if (x1 > x2 || x2 < 0)
                                goto mtri1t_donerow; // no more to do
                        }
                    }

                    if (loff > 0) {
                        // draw lhs pixel as is
                        if (x1 >= 0) {
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                            *(dst_offset32 + (y * dwidth + x1)) = src_offset32[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)];
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        }
                        // skip to next x location, effectively reducing steps by 1
                        x1++;
                        if (x1 > x2)
                            goto mtri1t_donerow;
                        loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
                    }

                    // align to loff
                    i64 = -loff;
                    tx += (i64 * txi) / 65536;
                    ty += (i64 * tyi) / 65536;

                    if (x1 < 0) { // clip left
                        d = g2x - g1x;
                        i64 = g2tx - g1tx;
                        tx += ((i64 << 16) * -x1) / d;
                        i64 = g2ty - g1ty;
                        ty += ((i64 << 16) * -x1) / d;
                        if (x1 < 0)
                            x1 = 0;
                    }

                    if (x2 >= dwidth) {
                        x2 = dwidth - 1; // clip right
                    }

                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    pixel_offset32 = dst_offset32 + (y * dwidth + x1);
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                    // bottleneck
                    for (x = x1; x <= x2; x++) {

                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        *(pixel_offset32++) = src_offset32[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)];
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                        tx += txi;
                        ty += tyi;
                    }

                mtri1t_donerow:;

                    if (y != y2) {
                        g1x += g1xi;
                        g1tx += g1txi;
                        g1ty += g1tyi;
                        g2x += g2xi;
                        g2tx += g2txi;
                        g2ty += g2tyi;
                    }
                }

                if (final == 0) {

                    // update indexed variable values with direct variable values which have
                    // changed & may be required
                    g1->x = g1x;
                    g2->x = g2x;
                    g1->tx = g1tx;
                    g2->tx = g2tx;
                    g1->ty = g1ty;
                    g2->ty = g2ty;

                mtri1t_final:;
                    if (y2 < dheight - 1) { // no point continuing if(offscreen!
                        if (g1->y2 < g2->y2)
                            g1 = g3;
                        else
                            g2 = g3;

                        // avoid doing the same row twice
                        y1 = g3->y1 + 1;
                        y2 = g3->y2;
                        g1->x += g1->xi;
                        g1->tx += g1->txi;
                        g1->ty += g1->tyi;
                        g2->x += g2->xi;
                        g2->tx += g2->txi;
                        g2->ty += g2->tyi;

                        final = 1;
                        goto mtri1t_usegrad3;
                    }
                }

                return;
            }

        mtri1_usegrad3:;

            if (final == 1) {
                if (no_edge_overlap)
                    y2 = y2 - 1;
            }

            // not on screen?
            if (y1 >= dheight) {
                return;
            }
            if (y2 < 0) {
                if (final)
                    return;
                // jump to y2's position
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = y2 - y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                goto mtri1_final;
            }

            // clip top
            if (y1 < 0) {
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = -y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                y1 = 0;
            }

            if (y2 >= dheight) { // clip bottom
                y2 = dheight - 1;
            }

            // move indexed variable values into direct variables for faster referencing
            // within 2nd bottleneck
            g1x = g1->x;
            g2x = g2->x;
            g1tx = g1->tx;
            g2tx = g2->tx;
            g1ty = g1->ty;
            g2ty = g2->ty;
            g1xi = g1->xi;
            g2xi = g2->xi;
            g1txi = g1->txi;
            g2txi = g2->txi;
            g1tyi = g1->tyi;
            g2tyi = g2->tyi;

            // 2nd bottleneck
            for (y = y1; y <= y2; y++) {

                if (g1x < 0)
                    x1 = (g1x - 65535) / 65536;
                else
                    x1 = g1x / 65536; // int-style rounding of fixed-point value
                if (g2x < 0)
                    x2 = (g2x - 65535) / 65536;
                else
                    x2 = g2x / 65536;

                if (x1 >= dwidth || x2 < 0)
                    goto mtri1_donerow; // crop if(entirely offscreen

                tx = g1tx;
                ty = g1ty;

                // calculate gradients if they might be required
                if (x1 != x2) {
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    txi = (i64 << 16) / d;
                    i64 = g2ty - g1ty;
                    tyi = (i64 << 16) / d;
                } else {
                    txi = 0;
                    tyi = 0;
                }

                // calculate pixel offsets from ideals
                loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                                // values
                roff = ((g2x & 65535) - 32768);

                if (roff < 0) {                                // not enough of rhs pixel exists to use
                    if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                        // draw rhs pixel as is
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        *(dst_offset32 + (y * dwidth + x2)) = src_offset32[(g2ty >> 16) * swidth + (g2tx >> 16)];
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // move left one position
                    x2--;
                    if (x1 > x2 || x2 < 0)
                        goto mtri1_donerow; // no more to do
                } else {
                    if (no_edge_overlap) {
                        x2 = x2 - 1;
                        if (x1 > x2 || x2 < 0)
                            goto mtri1_donerow; // no more to do
                    }
                }

                if (loff > 0) {
                    // draw lhs pixel as is
                    if (x1 >= 0) {
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        *(dst_offset32 + (y * dwidth + x1)) = src_offset32[(ty >> 16) * swidth + (tx >> 16)];
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // skip to next x location, effectively reducing steps by 1
                    x1++;
                    if (x1 > x2)
                        goto mtri1_donerow;
                    loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
                }

                // align to loff
                i64 = -loff;
                tx += (i64 * txi) / 65536;
                ty += (i64 * tyi) / 65536;

                if (x1 < 0) { // clip left
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    tx += ((i64 << 16) * -x1) / d;
                    i64 = g2ty - g1ty;
                    ty += ((i64 << 16) * -x1) / d;
                    if (x1 < 0)
                        x1 = 0;
                }

                if (x2 >= dwidth) {
                    x2 = dwidth - 1; // clip right
                }

                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                pixel_offset32 = dst_offset32 + (y * dwidth + x1);
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                // bottleneck
                for (x = x1; x <= x2; x++) {

                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    *(pixel_offset32++) = src_offset32[(ty >> 16) * swidth + (tx >> 16)];
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                    tx += txi;
                    ty += tyi;
                }

            mtri1_donerow:;

                if (y != y2) {
                    g1x += g1xi;
                    g1tx += g1txi;
                    g1ty += g1tyi;
                    g2x += g2xi;
                    g2tx += g2txi;
                    g2ty += g2tyi;
                }
            }

            if (final == 0) {

                // update indexed variable values with direct variable values which have
                // changed & may be required
                g1->x = g1x;
                g2->x = g2x;
                g1->tx = g1tx;
                g2->tx = g2tx;
                g1->ty = g1ty;
                g2->ty = g2ty;

            mtri1_final:;
                if (y2 < dheight - 1) { // no point continuing if(offscreen!
                    if (g1->y2 < g2->y2)
                        g1 = g3;
                    else
                        g2 = g3;

                    // avoid doing the same row twice
                    y1 = g3->y1 + 1;
                    y2 = g3->y2;
                    g1->x += g1->xi;
                    g1->tx += g1->txi;
                    g1->ty += g1->tyi;
                    g2->x += g2->xi;
                    g2->tx += g2->txi;
                    g2->ty += g2->tyi;

                    final = 1;
                    goto mtri1_usegrad3;
                }
            }

            return;
        } else {
            if (tile) {

            mtri2t_usegrad3:;

                if (final == 1) {
                    if (no_edge_overlap)
                        y2 = y2 - 1;
                }

                // not on screen?
                if (y1 >= dheight) {
                    return;
                }
                if (y2 < 0) {
                    if (final)
                        return;
                    // jump to y2's position
                    // note; original point locations are referenced because they are unmodified
                    // & represent the true distance of the run
                    y = y2 - y1;
                    p1 = g1->p1;
                    p2 = g1->p2;
                    d = g1->y2 - g1->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g1->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g1->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g1->x += i64 * y / d;
                        p1 = g2->p1;
                        p2 = g2->p2;
                    }
                    d = g2->y2 - g2->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g2->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g2->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g2->x += i64 * y / d;
                    }
                    goto mtri2t_final;
                }

                // clip top
                if (y1 < 0) {
                    // note; original point locations are referenced because they are unmodified
                    // & represent the true distance of the run
                    y = -y1;
                    p1 = g1->p1;
                    p2 = g1->p2;
                    d = g1->y2 - g1->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g1->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g1->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g1->x += i64 * y / d;
                        p1 = g2->p1;
                        p2 = g2->p2;
                    }
                    d = g2->y2 - g2->y1;
                    if (d) {
                        i64 = p2->tx - p1->tx;
                        g2->tx += i64 * y / d;
                        i64 = p2->ty - p1->ty;
                        g2->ty += i64 * y / d;
                        i64 = p2->x - p1->x;
                        g2->x += i64 * y / d;
                    }
                    y1 = 0;
                }

                if (y2 >= dheight) { // clip bottom
                    y2 = dheight - 1;
                }

                // move indexed variable values into direct variables for faster referencing
                // within 2nd bottleneck
                g1x = g1->x;
                g2x = g2->x;
                g1tx = g1->tx;
                g2tx = g2->tx;
                g1ty = g1->ty;
                g2ty = g2->ty;
                g1xi = g1->xi;
                g2xi = g2->xi;
                g1txi = g1->txi;
                g2txi = g2->txi;
                g1tyi = g1->tyi;
                g2tyi = g2->tyi;

                // 2nd bottleneck
                for (y = y1; y <= y2; y++) {

                    if (g1x < 0)
                        x1 = (g1x - 65535) / 65536;
                    else
                        x1 = g1x / 65536; // int-style rounding of fixed-point value
                    if (g2x < 0)
                        x2 = (g2x - 65535) / 65536;
                    else
                        x2 = g2x / 65536;

                    if (x1 >= dwidth || x2 < 0)
                        goto mtri2t_donerow; // crop if(entirely offscreen

                    tx = g1tx;
                    ty = g1ty;

                    // calculate gradients if they might be required
                    if (x1 != x2) {
                        d = g2x - g1x;
                        i64 = g2tx - g1tx;
                        txi = (i64 << 16) / d;
                        i64 = g2ty - g1ty;
                        tyi = (i64 << 16) / d;
                    } else {
                        txi = 0;
                        tyi = 0;
                    }

                    // calculate pixel offsets from ideals
                    loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                                    // values
                    roff = ((g2x & 65535) - 32768);

                    if (roff < 0) {                                // not enough of rhs pixel exists to use
                        if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                            // draw rhs pixel as is
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                            pixel_offset32 = dst_offset32 + (y * dwidth + x2);
                            //--------plot pixel--------
                            switch ((col = src_offset32[((g2ty >> 16) % sheight) * swidth + ((g2tx >> 16) % swidth)]) & 0xFF000000) {
                            case 0xFF000000:
                                *pixel_offset32 = col;
                                break;
                            case 0x0:
                                break;
                            case 0x80000000:
                                *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*pixel_offset32 >> 24] << 24);
                                break;
                            case 0x7F000000:
                                *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*pixel_offset32 >> 24] << 24);
                                break;
                            default:
                                destcol = *pixel_offset32;
                                cp = cblend + (col >> 24 << 16);
                                *pixel_offset32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                                  (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) +
                                                  (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                            }; // switch
                            //--------done plot pixel--------
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        }
                        // move left one position
                        x2--;
                        if (x1 > x2 || x2 < 0)
                            goto mtri2t_donerow; // no more to do
                    } else {
                        if (no_edge_overlap) {
                            x2 = x2 - 1;
                            if (x1 > x2 || x2 < 0)
                                goto mtri2t_donerow; // no more to do
                        }
                    }

                    if (loff > 0) {
                        // draw lhs pixel as is
                        if (x1 >= 0) {
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                            pixel_offset32 = dst_offset32 + (y * dwidth + x1);
                            //--------plot pixel--------
                            switch ((col = src_offset32[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)]) & 0xFF000000) {
                            case 0xFF000000:
                                *pixel_offset32 = col;
                                break;
                            case 0x0:
                                break;
                            case 0x80000000:
                                *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*pixel_offset32 >> 24] << 24);
                                break;
                            case 0x7F000000:
                                *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*pixel_offset32 >> 24] << 24);
                                break;
                            default:
                                destcol = *pixel_offset32;
                                cp = cblend + (col >> 24 << 16);
                                *pixel_offset32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                                  (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) +
                                                  (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                            }; // switch
                            //--------done plot pixel--------
                            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        }
                        // skip to next x location, effectively reducing steps by 1
                        x1++;
                        if (x1 > x2)
                            goto mtri2t_donerow;
                        loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
                    }

                    // align to loff
                    i64 = -loff;
                    tx += (i64 * txi) / 65536;
                    ty += (i64 * tyi) / 65536;

                    if (x1 < 0) { // clip left
                        d = g2x - g1x;
                        i64 = g2tx - g1tx;
                        tx += ((i64 << 16) * -x1) / d;
                        i64 = g2ty - g1ty;
                        ty += ((i64 << 16) * -x1) / d;
                        if (x1 < 0)
                            x1 = 0;
                    }

                    if (x2 >= dwidth) {
                        x2 = dwidth - 1; // clip right
                    }

                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    pixel_offset32 = dst_offset32 + (y * dwidth + x1);
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                    // bottleneck
                    for (x = x1; x <= x2; x++) {

                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        //--------plot pixel--------
                        switch ((col = src_offset32[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)]) & 0xFF000000) {
                        case 0xFF000000:
                            *pixel_offset32 = col;
                            break;
                        case 0x0:
                            break;
                        case 0x80000000:
                            *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*pixel_offset32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*pixel_offset32 >> 24] << 24);
                            break;
                        default:
                            destcol = *pixel_offset32;
                            cp = cblend + (col >> 24 << 16);
                            *pixel_offset32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                              (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        pixel_offset32++;
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                        tx += txi;
                        ty += tyi;
                    }

                mtri2t_donerow:;

                    if (y != y2) {
                        g1x += g1xi;
                        g1tx += g1txi;
                        g1ty += g1tyi;
                        g2x += g2xi;
                        g2tx += g2txi;
                        g2ty += g2tyi;
                    }
                }

                if (final == 0) {

                    // update indexed variable values with direct variable values which have
                    // changed & may be required
                    g1->x = g1x;
                    g2->x = g2x;
                    g1->tx = g1tx;
                    g2->tx = g2tx;
                    g1->ty = g1ty;
                    g2->ty = g2ty;

                mtri2t_final:;
                    if (y2 < dheight - 1) { // no point continuing if(offscreen!
                        if (g1->y2 < g2->y2)
                            g1 = g3;
                        else
                            g2 = g3;

                        // avoid doing the same row twice
                        y1 = g3->y1 + 1;
                        y2 = g3->y2;
                        g1->x += g1->xi;
                        g1->tx += g1->txi;
                        g1->ty += g1->tyi;
                        g2->x += g2->xi;
                        g2->tx += g2->txi;
                        g2->ty += g2->tyi;

                        final = 1;
                        goto mtri2t_usegrad3;
                    }
                }

                return;
            }

        mtri2_usegrad3:;

            if (final == 1) {
                if (no_edge_overlap)
                    y2 = y2 - 1;
            }

            // not on screen?
            if (y1 >= dheight) {
                return;
            }
            if (y2 < 0) {
                if (final)
                    return;
                // jump to y2's position
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = y2 - y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                goto mtri2_final;
            }

            // clip top
            if (y1 < 0) {
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = -y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                y1 = 0;
            }

            if (y2 >= dheight) { // clip bottom
                y2 = dheight - 1;
            }

            // move indexed variable values into direct variables for faster referencing
            // within 2nd bottleneck
            g1x = g1->x;
            g2x = g2->x;
            g1tx = g1->tx;
            g2tx = g2->tx;
            g1ty = g1->ty;
            g2ty = g2->ty;
            g1xi = g1->xi;
            g2xi = g2->xi;
            g1txi = g1->txi;
            g2txi = g2->txi;
            g1tyi = g1->tyi;
            g2tyi = g2->tyi;

            // 2nd bottleneck
            for (y = y1; y <= y2; y++) {

                if (g1x < 0)
                    x1 = (g1x - 65535) / 65536;
                else
                    x1 = g1x / 65536; // int-style rounding of fixed-point value
                if (g2x < 0)
                    x2 = (g2x - 65535) / 65536;
                else
                    x2 = g2x / 65536;

                if (x1 >= dwidth || x2 < 0)
                    goto mtri2_donerow; // crop if(entirely offscreen

                tx = g1tx;
                ty = g1ty;

                // calculate gradients if they might be required
                if (x1 != x2) {
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    txi = (i64 << 16) / d;
                    i64 = g2ty - g1ty;
                    tyi = (i64 << 16) / d;
                } else {
                    txi = 0;
                    tyi = 0;
                }

                // calculate pixel offsets from ideals
                loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                                // values
                roff = ((g2x & 65535) - 32768);

                if (roff < 0) {                                // not enough of rhs pixel exists to use
                    if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                        // draw rhs pixel as is
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        pixel_offset32 = dst_offset32 + (y * dwidth + x2);
                        //--------plot pixel--------
                        switch ((col = src_offset32[(g2ty >> 16) * swidth + (g2tx >> 16)]) & 0xFF000000) {
                        case 0xFF000000:
                            *pixel_offset32 = col;
                            break;
                        case 0x0:
                            break;
                        case 0x80000000:
                            *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*pixel_offset32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*pixel_offset32 >> 24] << 24);
                            break;
                        default:
                            destcol = *pixel_offset32;
                            cp = cblend + (col >> 24 << 16);
                            *pixel_offset32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                              (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // move left one position
                    x2--;
                    if (x1 > x2 || x2 < 0)
                        goto mtri2_donerow; // no more to do
                } else {
                    if (no_edge_overlap) {
                        x2 = x2 - 1;
                        if (x1 > x2 || x2 < 0)
                            goto mtri2_donerow; // no more to do
                    }
                }

                if (loff > 0) {
                    // draw lhs pixel as is
                    if (x1 >= 0) {
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        pixel_offset32 = dst_offset32 + (y * dwidth + x1);
                        //--------plot pixel--------
                        switch ((col = src_offset32[(ty >> 16) * swidth + (tx >> 16)]) & 0xFF000000) {
                        case 0xFF000000:
                            *pixel_offset32 = col;
                            break;
                        case 0x0:
                            break;
                        case 0x80000000:
                            *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*pixel_offset32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*pixel_offset32 >> 24] << 24);
                            break;
                        default:
                            destcol = *pixel_offset32;
                            cp = cblend + (col >> 24 << 16);
                            *pixel_offset32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                              (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // skip to next x location, effectively reducing steps by 1
                    x1++;
                    if (x1 > x2)
                        goto mtri2_donerow;
                    loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
                }

                // align to loff
                i64 = -loff;
                tx += (i64 * txi) / 65536;
                ty += (i64 * tyi) / 65536;

                if (x1 < 0) { // clip left
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    tx += ((i64 << 16) * -x1) / d;
                    i64 = g2ty - g1ty;
                    ty += ((i64 << 16) * -x1) / d;
                    if (x1 < 0)
                        x1 = 0;
                }

                if (x2 >= dwidth) {
                    x2 = dwidth - 1; // clip right
                }

                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                pixel_offset32 = dst_offset32 + (y * dwidth + x1);
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                // bottleneck
                for (x = x1; x <= x2; x++) {

                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    //--------plot pixel--------
                    switch ((col = src_offset32[(ty >> 16) * swidth + (tx >> 16)]) & 0xFF000000) {
                    case 0xFF000000:
                        *pixel_offset32 = col;
                        break;
                    case 0x0:
                        break;
                    case 0x80000000:
                        *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*pixel_offset32 >> 24] << 24);
                        break;
                    case 0x7F000000:
                        *pixel_offset32 = (((*pixel_offset32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*pixel_offset32 >> 24] << 24);
                        break;
                    default:
                        destcol = *pixel_offset32;
                        cp = cblend + (col >> 24 << 16);
                        *pixel_offset32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                          (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                    }; // switch
                    //--------done plot pixel--------
                    pixel_offset32++;
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                    tx += txi;
                    ty += tyi;
                }

            mtri2_donerow:;

                if (y != y2) {
                    g1x += g1xi;
                    g1tx += g1txi;
                    g1ty += g1tyi;
                    g2x += g2xi;
                    g2tx += g2txi;
                    g2ty += g2tyi;
                }
            }

            if (final == 0) {

                // update indexed variable values with direct variable values which have
                // changed & may be required
                g1->x = g1x;
                g2->x = g2x;
                g1->tx = g1tx;
                g2->tx = g2tx;
                g1->ty = g1ty;
                g2->ty = g2ty;

            mtri2_final:;
                if (y2 < dheight - 1) { // no point continuing if(offscreen!
                    if (g1->y2 < g2->y2)
                        g1 = g3;
                    else
                        g2 = g3;

                    // avoid doing the same row twice
                    y1 = g3->y1 + 1;
                    y2 = g3->y2;
                    g1->x += g1->xi;
                    g1->tx += g1->txi;
                    g1->ty += g1->tyi;
                    g2->x += g2->xi;
                    g2->tx += g2->txi;
                    g2->ty += g2->tyi;

                    final = 1;
                    goto mtri2_usegrad3;
                }
            }

            return;
        }
    } // 4

    // assume 1 byte per pixel
    dst_offset = dst->offset;
    src_offset = src->offset;
    auto transparent_color = src->transparent_color;
    if (transparent_color == -1) {
        if (tile) {

        mtri3t_usegrad3:;

            if (final == 1) {
                if (no_edge_overlap)
                    y2 = y2 - 1;
            }

            // not on screen?
            if (y1 >= dheight) {
                return;
            }
            if (y2 < 0) {
                if (final)
                    return;
                // jump to y2's position
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = y2 - y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                goto mtri3t_final;
            }

            // clip top
            if (y1 < 0) {
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = -y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                y1 = 0;
            }

            if (y2 >= dheight) { // clip bottom
                y2 = dheight - 1;
            }

            // move indexed variable values into direct variables for faster referencing
            // within 2nd bottleneck
            g1x = g1->x;
            g2x = g2->x;
            g1tx = g1->tx;
            g2tx = g2->tx;
            g1ty = g1->ty;
            g2ty = g2->ty;
            g1xi = g1->xi;
            g2xi = g2->xi;
            g1txi = g1->txi;
            g2txi = g2->txi;
            g1tyi = g1->tyi;
            g2tyi = g2->tyi;

            // 2nd bottleneck
            for (y = y1; y <= y2; y++) {

                if (g1x < 0)
                    x1 = (g1x - 65535) / 65536;
                else
                    x1 = g1x / 65536; // int-style rounding of fixed-point value
                if (g2x < 0)
                    x2 = (g2x - 65535) / 65536;
                else
                    x2 = g2x / 65536;

                if (x1 >= dwidth || x2 < 0)
                    goto mtri3t_donerow; // crop if(entirely offscreen

                tx = g1tx;
                ty = g1ty;

                // calculate gradients if they might be required
                if (x1 != x2) {
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    txi = (i64 << 16) / d;
                    i64 = g2ty - g1ty;
                    tyi = (i64 << 16) / d;
                } else {
                    txi = 0;
                    tyi = 0;
                }

                // calculate pixel offsets from ideals
                loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                                // values
                roff = ((g2x & 65535) - 32768);

                if (roff < 0) {                                // not enough of rhs pixel exists to use
                    if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                        // draw rhs pixel as is
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        *(dst_offset + (y * dwidth + x2)) = src_offset[((g2ty >> 16) % sheight) * swidth + ((g2tx >> 16) % swidth)];
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // move left one position
                    x2--;
                    if (x1 > x2 || x2 < 0)
                        goto mtri3t_donerow; // no more to do
                } else {
                    if (no_edge_overlap) {
                        x2 = x2 - 1;
                        if (x1 > x2 || x2 < 0)
                            goto mtri3t_donerow; // no more to do
                    }
                }

                if (loff > 0) {
                    // draw lhs pixel as is
                    if (x1 >= 0) {
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        *(dst_offset + (y * dwidth + x1)) = src_offset[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)];
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // skip to next x location, effectively reducing steps by 1
                    x1++;
                    if (x1 > x2)
                        goto mtri3t_donerow;
                    loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
                }

                // align to loff
                i64 = -loff;
                tx += (i64 * txi) / 65536;
                ty += (i64 * tyi) / 65536;

                if (x1 < 0) { // clip left
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    tx += ((i64 << 16) * -x1) / d;
                    i64 = g2ty - g1ty;
                    ty += ((i64 << 16) * -x1) / d;
                    if (x1 < 0)
                        x1 = 0;
                }

                if (x2 >= dwidth) {
                    x2 = dwidth - 1; // clip right
                }

                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                pixel_offset = dst_offset + (y * dwidth + x1);
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                // bottleneck
                for (x = x1; x <= x2; x++) {

                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    *(pixel_offset++) = src_offset[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)];
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                    tx += txi;
                    ty += tyi;
                }

            mtri3t_donerow:;

                if (y != y2) {
                    g1x += g1xi;
                    g1tx += g1txi;
                    g1ty += g1tyi;
                    g2x += g2xi;
                    g2tx += g2txi;
                    g2ty += g2tyi;
                }
            }

            if (final == 0) {

                // update indexed variable values with direct variable values which have
                // changed & may be required
                g1->x = g1x;
                g2->x = g2x;
                g1->tx = g1tx;
                g2->tx = g2tx;
                g1->ty = g1ty;
                g2->ty = g2ty;

            mtri3t_final:;
                if (y2 < dheight - 1) { // no point continuing if(offscreen!
                    if (g1->y2 < g2->y2)
                        g1 = g3;
                    else
                        g2 = g3;

                    // avoid doing the same row twice
                    y1 = g3->y1 + 1;
                    y2 = g3->y2;
                    g1->x += g1->xi;
                    g1->tx += g1->txi;
                    g1->ty += g1->tyi;
                    g2->x += g2->xi;
                    g2->tx += g2->txi;
                    g2->ty += g2->tyi;

                    final = 1;
                    goto mtri3t_usegrad3;
                }
            }

            return;
        }

    mtri3_usegrad3:;

        if (final == 1) {
            if (no_edge_overlap)
                y2 = y2 - 1;
        }

        // not on screen?
        if (y1 >= dheight) {
            return;
        }
        if (y2 < 0) {
            if (final)
                return;
            // jump to y2's position
            // note; original point locations are referenced because they are unmodified
            // & represent the true distance of the run
            y = y2 - y1;
            p1 = g1->p1;
            p2 = g1->p2;
            d = g1->y2 - g1->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g1->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g1->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g1->x += i64 * y / d;
                p1 = g2->p1;
                p2 = g2->p2;
            }
            d = g2->y2 - g2->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g2->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g2->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g2->x += i64 * y / d;
            }
            goto mtri3_final;
        }

        // clip top
        if (y1 < 0) {
            // note; original point locations are referenced because they are unmodified
            // & represent the true distance of the run
            y = -y1;
            p1 = g1->p1;
            p2 = g1->p2;
            d = g1->y2 - g1->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g1->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g1->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g1->x += i64 * y / d;
                p1 = g2->p1;
                p2 = g2->p2;
            }
            d = g2->y2 - g2->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g2->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g2->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g2->x += i64 * y / d;
            }
            y1 = 0;
        }

        if (y2 >= dheight) { // clip bottom
            y2 = dheight - 1;
        }

        // move indexed variable values into direct variables for faster referencing
        // within 2nd bottleneck
        g1x = g1->x;
        g2x = g2->x;
        g1tx = g1->tx;
        g2tx = g2->tx;
        g1ty = g1->ty;
        g2ty = g2->ty;
        g1xi = g1->xi;
        g2xi = g2->xi;
        g1txi = g1->txi;
        g2txi = g2->txi;
        g1tyi = g1->tyi;
        g2tyi = g2->tyi;

        // 2nd bottleneck
        for (y = y1; y <= y2; y++) {

            if (g1x < 0)
                x1 = (g1x - 65535) / 65536;
            else
                x1 = g1x / 65536; // int-style rounding of fixed-point value
            if (g2x < 0)
                x2 = (g2x - 65535) / 65536;
            else
                x2 = g2x / 65536;

            if (x1 >= dwidth || x2 < 0)
                goto mtri3_donerow; // crop if(entirely offscreen

            tx = g1tx;
            ty = g1ty;

            // calculate gradients if they might be required
            if (x1 != x2) {
                d = g2x - g1x;
                i64 = g2tx - g1tx;
                txi = (i64 << 16) / d;
                i64 = g2ty - g1ty;
                tyi = (i64 << 16) / d;
            } else {
                txi = 0;
                tyi = 0;
            }

            // calculate pixel offsets from ideals
            loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                            // values
            roff = ((g2x & 65535) - 32768);

            if (roff < 0) {                                // not enough of rhs pixel exists to use
                if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                    // draw rhs pixel as is
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    *(dst_offset + (y * dwidth + x2)) = src_offset[(g2ty >> 16) * swidth + (g2tx >> 16)];
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                }
                // move left one position
                x2--;
                if (x1 > x2 || x2 < 0)
                    goto mtri3_donerow; // no more to do
            } else {
                if (no_edge_overlap) {
                    x2 = x2 - 1;
                    if (x1 > x2 || x2 < 0)
                        goto mtri3_donerow; // no more to do
                }
            }

            if (loff > 0) {
                // draw lhs pixel as is
                if (x1 >= 0) {
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    *(dst_offset + (y * dwidth + x1)) = src_offset[(ty >> 16) * swidth + (tx >> 16)];
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                }
                // skip to next x location, effectively reducing steps by 1
                x1++;
                if (x1 > x2)
                    goto mtri3_donerow;
                loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
            }

            // align to loff
            i64 = -loff;
            tx += (i64 * txi) / 65536;
            ty += (i64 * tyi) / 65536;

            if (x1 < 0) { // clip left
                d = g2x - g1x;
                i64 = g2tx - g1tx;
                tx += ((i64 << 16) * -x1) / d;
                i64 = g2ty - g1ty;
                ty += ((i64 << 16) * -x1) / d;
                if (x1 < 0)
                    x1 = 0;
            }

            if (x2 >= dwidth) {
                x2 = dwidth - 1; // clip right
            }

            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            pixel_offset = dst_offset + (y * dwidth + x1);
            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

            // bottleneck
            for (x = x1; x <= x2; x++) {

                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                *(pixel_offset++) = src_offset[(ty >> 16) * swidth + (tx >> 16)];
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                tx += txi;
                ty += tyi;
            }

        mtri3_donerow:;

            if (y != y2) {
                g1x += g1xi;
                g1tx += g1txi;
                g1ty += g1tyi;
                g2x += g2xi;
                g2tx += g2txi;
                g2ty += g2tyi;
            }
        }

        if (final == 0) {

            // update indexed variable values with direct variable values which have
            // changed & may be required
            g1->x = g1x;
            g2->x = g2x;
            g1->tx = g1tx;
            g2->tx = g2tx;
            g1->ty = g1ty;
            g2->ty = g2ty;

        mtri3_final:;
            if (y2 < dheight - 1) { // no point continuing if(offscreen!
                if (g1->y2 < g2->y2)
                    g1 = g3;
                else
                    g2 = g3;

                // avoid doing the same row twice
                y1 = g3->y1 + 1;
                y2 = g3->y2;
                g1->x += g1->xi;
                g1->tx += g1->txi;
                g1->ty += g1->tyi;
                g2->x += g2->xi;
                g2->tx += g2->txi;
                g2->ty += g2->tyi;

                final = 1;
                goto mtri3_usegrad3;
            }
        }

        return;
    } else {
        if (tile) {

        mtri4t_usegrad3:;

            if (final == 1) {
                if (no_edge_overlap)
                    y2 = y2 - 1;
            }

            // not on screen?
            if (y1 >= dheight) {
                return;
            }
            if (y2 < 0) {
                if (final)
                    return;
                // jump to y2's position
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = y2 - y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                goto mtri4t_final;
            }

            // clip top
            if (y1 < 0) {
                // note; original point locations are referenced because they are unmodified
                // & represent the true distance of the run
                y = -y1;
                p1 = g1->p1;
                p2 = g1->p2;
                d = g1->y2 - g1->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g1->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g1->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g1->x += i64 * y / d;
                    p1 = g2->p1;
                    p2 = g2->p2;
                }
                d = g2->y2 - g2->y1;
                if (d) {
                    i64 = p2->tx - p1->tx;
                    g2->tx += i64 * y / d;
                    i64 = p2->ty - p1->ty;
                    g2->ty += i64 * y / d;
                    i64 = p2->x - p1->x;
                    g2->x += i64 * y / d;
                }
                y1 = 0;
            }

            if (y2 >= dheight) { // clip bottom
                y2 = dheight - 1;
            }

            // move indexed variable values into direct variables for faster referencing
            // within 2nd bottleneck
            g1x = g1->x;
            g2x = g2->x;
            g1tx = g1->tx;
            g2tx = g2->tx;
            g1ty = g1->ty;
            g2ty = g2->ty;
            g1xi = g1->xi;
            g2xi = g2->xi;
            g1txi = g1->txi;
            g2txi = g2->txi;
            g1tyi = g1->tyi;
            g2tyi = g2->tyi;

            // 2nd bottleneck
            for (y = y1; y <= y2; y++) {

                if (g1x < 0)
                    x1 = (g1x - 65535) / 65536;
                else
                    x1 = g1x / 65536; // int-style rounding of fixed-point value
                if (g2x < 0)
                    x2 = (g2x - 65535) / 65536;
                else
                    x2 = g2x / 65536;

                if (x1 >= dwidth || x2 < 0)
                    goto mtri4t_donerow; // crop if(entirely offscreen

                tx = g1tx;
                ty = g1ty;

                // calculate gradients if they might be required
                if (x1 != x2) {
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    txi = (i64 << 16) / d;
                    i64 = g2ty - g1ty;
                    tyi = (i64 << 16) / d;
                } else {
                    txi = 0;
                    tyi = 0;
                }

                // calculate pixel offsets from ideals
                loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                                // values
                roff = ((g2x & 65535) - 32768);

                if (roff < 0) {                                // not enough of rhs pixel exists to use
                    if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                        // draw rhs pixel as is
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        col = src_offset[((g2ty >> 16) % sheight) * swidth + ((g2tx >> 16) % swidth)];
                        if (col != uint32_t(transparent_color))
                            *(dst_offset + (y * dwidth + x2)) = col;
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // move left one position
                    x2--;
                    if (x1 > x2 || x2 < 0)
                        goto mtri4t_donerow; // no more to do
                } else {
                    if (no_edge_overlap) {
                        x2 = x2 - 1;
                        if (x1 > x2 || x2 < 0)
                            goto mtri4t_donerow; // no more to do
                    }
                }

                if (loff > 0) {
                    // draw lhs pixel as is
                    if (x1 >= 0) {
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                        col = src_offset[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)];
                        if (col != uint32_t(transparent_color))
                            *(dst_offset + (y * dwidth + x1)) = col;
                        //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    }
                    // skip to next x location, effectively reducing steps by 1
                    x1++;
                    if (x1 > x2)
                        goto mtri4t_donerow;
                    loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
                }

                // align to loff
                i64 = -loff;
                tx += (i64 * txi) / 65536;
                ty += (i64 * tyi) / 65536;

                if (x1 < 0) { // clip left
                    d = g2x - g1x;
                    i64 = g2tx - g1tx;
                    tx += ((i64 << 16) * -x1) / d;
                    i64 = g2ty - g1ty;
                    ty += ((i64 << 16) * -x1) / d;
                    if (x1 < 0)
                        x1 = 0;
                }

                if (x2 >= dwidth) {
                    x2 = dwidth - 1; // clip right
                }

                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                pixel_offset = dst_offset + (y * dwidth + x1);
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                // bottleneck
                for (x = x1; x <= x2; x++) {

                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    col = src_offset[((ty >> 16) % sheight) * swidth + ((tx >> 16) % swidth)];
                    if (col != uint32_t(transparent_color))
                        *pixel_offset = col;
                    pixel_offset++;
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                    tx += txi;
                    ty += tyi;
                }

            mtri4t_donerow:;

                if (y != y2) {
                    g1x += g1xi;
                    g1tx += g1txi;
                    g1ty += g1tyi;
                    g2x += g2xi;
                    g2tx += g2txi;
                    g2ty += g2tyi;
                }
            }

            if (final == 0) {

                // update indexed variable values with direct variable values which have
                // changed & may be required
                g1->x = g1x;
                g2->x = g2x;
                g1->tx = g1tx;
                g2->tx = g2tx;
                g1->ty = g1ty;
                g2->ty = g2ty;

            mtri4t_final:;
                if (y2 < dheight - 1) { // no point continuing if(offscreen!
                    if (g1->y2 < g2->y2)
                        g1 = g3;
                    else
                        g2 = g3;

                    // avoid doing the same row twice
                    y1 = g3->y1 + 1;
                    y2 = g3->y2;
                    g1->x += g1->xi;
                    g1->tx += g1->txi;
                    g1->ty += g1->tyi;
                    g2->x += g2->xi;
                    g2->tx += g2->txi;
                    g2->ty += g2->tyi;

                    final = 1;
                    goto mtri4t_usegrad3;
                }
            }

            return;
        }

    mtri4_usegrad3:;

        if (final == 1) {
            if (no_edge_overlap)
                y2 = y2 - 1;
        }

        // not on screen?
        if (y1 >= dheight) {
            return;
        }
        if (y2 < 0) {
            if (final)
                return;
            // jump to y2's position
            // note; original point locations are referenced because they are unmodified
            // & represent the true distance of the run
            y = y2 - y1;
            p1 = g1->p1;
            p2 = g1->p2;
            d = g1->y2 - g1->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g1->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g1->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g1->x += i64 * y / d;
                p1 = g2->p1;
                p2 = g2->p2;
            }
            d = g2->y2 - g2->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g2->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g2->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g2->x += i64 * y / d;
            }
            goto mtri4_final;
        }

        // clip top
        if (y1 < 0) {
            // note; original point locations are referenced because they are unmodified
            // & represent the true distance of the run
            y = -y1;
            p1 = g1->p1;
            p2 = g1->p2;
            d = g1->y2 - g1->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g1->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g1->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g1->x += i64 * y / d;
                p1 = g2->p1;
                p2 = g2->p2;
            }
            d = g2->y2 - g2->y1;
            if (d) {
                i64 = p2->tx - p1->tx;
                g2->tx += i64 * y / d;
                i64 = p2->ty - p1->ty;
                g2->ty += i64 * y / d;
                i64 = p2->x - p1->x;
                g2->x += i64 * y / d;
            }
            y1 = 0;
        }

        if (y2 >= dheight) { // clip bottom
            y2 = dheight - 1;
        }

        // move indexed variable values into direct variables for faster referencing
        // within 2nd bottleneck
        g1x = g1->x;
        g2x = g2->x;
        g1tx = g1->tx;
        g2tx = g2->tx;
        g1ty = g1->ty;
        g2ty = g2->ty;
        g1xi = g1->xi;
        g2xi = g2->xi;
        g1txi = g1->txi;
        g2txi = g2->txi;
        g1tyi = g1->tyi;
        g2tyi = g2->tyi;

        // 2nd bottleneck
        for (y = y1; y <= y2; y++) {

            if (g1x < 0)
                x1 = (g1x - 65535) / 65536;
            else
                x1 = g1x / 65536; // int-style rounding of fixed-point value
            if (g2x < 0)
                x2 = (g2x - 65535) / 65536;
            else
                x2 = g2x / 65536;

            if (x1 >= dwidth || x2 < 0)
                goto mtri4_donerow; // crop if(entirely offscreen

            tx = g1tx;
            ty = g1ty;

            // calculate gradients if they might be required
            if (x1 != x2) {
                d = g2x - g1x;
                i64 = g2tx - g1tx;
                txi = (i64 << 16) / d;
                i64 = g2ty - g1ty;
                tyi = (i64 << 16) / d;
            } else {
                txi = 0;
                tyi = 0;
            }

            // calculate pixel offsets from ideals
            loff = ((g1x & 65535) - 32768); // note; works for positive & negative
                                            // values
            roff = ((g2x & 65535) - 32768);

            if (roff < 0) {                                // not enough of rhs pixel exists to use
                if (x2 < dwidth && no_edge_overlap == 0) { // onscreen check
                    // draw rhs pixel as is
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    col = src_offset[(g2ty >> 16) * swidth + (g2tx >> 16)];
                    if (col != uint32_t(transparent_color))
                        *(dst_offset + (y * dwidth + x2)) = col;
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                }
                // move left one position
                x2--;
                if (x1 > x2 || x2 < 0)
                    goto mtri4_donerow; // no more to do
            } else {
                if (no_edge_overlap) {
                    x2 = x2 - 1;
                    if (x1 > x2 || x2 < 0)
                        goto mtri4_donerow; // no more to do
                }
            }

            if (loff > 0) {
                // draw lhs pixel as is
                if (x1 >= 0) {
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                    col = src_offset[(ty >> 16) * swidth + (tx >> 16)];
                    if (col != uint32_t(transparent_color))
                        *(dst_offset + (y * dwidth + x1)) = col;
                    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                }
                // skip to next x location, effectively reducing steps by 1
                x1++;
                if (x1 > x2)
                    goto mtri4_donerow;
                loff = -(65536 - loff); // adjust alignment to jump to next ideal offset
            }

            // align to loff
            i64 = -loff;
            tx += (i64 * txi) / 65536;
            ty += (i64 * tyi) / 65536;

            if (x1 < 0) { // clip left
                d = g2x - g1x;
                i64 = g2tx - g1tx;
                tx += ((i64 << 16) * -x1) / d;
                i64 = g2ty - g1ty;
                ty += ((i64 << 16) * -x1) / d;
                if (x1 < 0)
                    x1 = 0;
            }

            if (x2 >= dwidth) {
                x2 = dwidth - 1; // clip right
            }

            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
            pixel_offset = dst_offset + (y * dwidth + x1);
            //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

            // bottleneck
            for (x = x1; x <= x2; x++) {

                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                col = src_offset[(ty >> 16) * swidth + (tx >> 16)];
                if (col != uint32_t(transparent_color))
                    *pixel_offset = col;
                pixel_offset++;
                //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                tx += txi;
                ty += tyi;
            }

        mtri4_donerow:;

            if (y != y2) {
                g1x += g1xi;
                g1tx += g1txi;
                g1ty += g1tyi;
                g2x += g2xi;
                g2tx += g2txi;
                g2ty += g2tyi;
            }
        }

        if (final == 0) {

            // update indexed variable values with direct variable values which have
            // changed & may be required
            g1->x = g1x;
            g2->x = g2x;
            g1->tx = g1tx;
            g2->tx = g2tx;
            g1->ty = g1ty;
            g2->ty = g2ty;

        mtri4_final:;
            if (y2 < dheight - 1) { // no point continuing if(offscreen!
                if (g1->y2 < g2->y2)
                    g1 = g3;
                else
                    g2 = g3;

                // avoid doing the same row twice
                y1 = g3->y1 + 1;
                y2 = g3->y2;
                g1->x += g1->xi;
                g1->tx += g1->txi;
                g1->ty += g1->tyi;
                g2->x += g2->xi;
                g2->tx += g2->txi;
                g2->ty += g2->tyi;

                final = 1;
                goto mtri4_usegrad3;
            }
        }

        return;
    } // 1

    error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
    return;
} // sub__maptriangle

//----------------------------------------------------------------------------------------------------------------------
// Drawing Primitives - LINE, CIRCLE, PAINT, PSET, PRESET, POINT
// Extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

/**
 * Draws a line, rectangle outline, or filled rectangle.
 * 
 * Draws a line between two points, a rectangle outline, or a filled rectangle
 * depending on the bf parameter. Supports STEP coordinates, custom colors,
 * and line styles (dashed patterns).
 * 
 * @param x1 X coordinate of first point (or start point if using STEP)
 * @param y1 Y coordinate of first point (or start point if using STEP)
 * @param x2 X coordinate of second point (or offset if using STEP)
 * @param y2 Y coordinate of second point (or offset if using STEP)
 * @param col Color value (uses current color if not specified)
 * @param bf Drawing mode: 0=line, 1=rectangle outline, 2=filled rectangle
 * @param style Line style pattern (16-bit mask for dashed lines)
 * @param passed Bit flags: bit 0=first point provided, bit 1=second point provided,
 *               bit 2=STEP for first point, bit 3=STEP for second point,
 *               bit 4=color provided, bit 5=style provided
 */
void sub_line(float x1, float y1, float x2, float y2, uint32 col, int32 bf, uint32 style, int32 passed) {
    if (is_error_pending())
        return;
    if (write_page->text) {
        error(5);
        return;
    }
    /*
        '"[[{STEP}](?,?)]-[{STEP}](?,?)[,[?][,[{B|BF}][,?]]]"
        LINE -(10, 10) 'flags: 0
        LINE (0, 0)-(10, 10) 'flags: 1
        LINE -STEP(10, 10) 'flags: 2
        LINE STEP(0, 0)-(10, 10) 'flags: 1+4
    */

    // adjust coordinates and qb graphics cursor position based on STEP
    if (passed & 1) {
        if (passed & 4) {
            x1 = write_page->x + x1;
            y1 = write_page->y + y1;
        }
        write_page->x = x1;
        write_page->y = y1;
    } else {
        x1 = write_page->x;
        y1 = write_page->y;
    }
    if (passed & 2) {
        x2 = write_page->x + x2;
        y2 = write_page->y + y2;
    }
    write_page->x = x2;
    write_page->y = y2;

    if (bf == 0) { // line
        if ((passed & 16) == 0)
            style = 0xFFFF;
        if ((passed & 8) == 0)
            col = write_page->color;
        write_page->draw_color = col;
        qb32_line(x1, y1, x2, y2, col, style);
        return;
    }

    if (bf == 1) { // rectangle
        if ((passed & 16) == 0)
            style = 0xFFFF;
        if ((passed & 8) == 0)
            col = write_page->color;
        write_page->draw_color = col;
        qb32_line(x1, y1, x2, y1, col, style);
        qb32_line(x2, y1, x2, y2, col, style);
        qb32_line(x2, y2, x1, y2, col, style);
        qb32_line(x1, y2, x1, y1, col, style);
        return;
    }

    if (bf == 2) { // filled box
        if ((passed & 8) == 0)
            col = write_page->color;
        write_page->draw_color = col;
        qb32_boxfill(x1, y1, x2, y2, col);
        return;
    }

} // sub_line

// 3 paint routines exist for color (not textured) filling
// i) 8-bit
// ii) 32-bit no-alpha
// iii) 32-bit
// simple comparisons are used, the alpha value is part of that comparison in all cases
// even if blending is disabled (a fixed color is likely to have a fixed alpha value anyway),
// and this allows for filling alpha regions

/**
 * Fills an area with a color using flood fill algorithm (32-bit with blending).
 * 
 * Fills a bounded area starting at the specified point with the fill color.
 * The fill stops at the border color. Supports alpha blending for 32-bit images.
 * Uses a two-buffer algorithm for efficient flood filling.
 * 
 * @param x X coordinate of starting point (or offset if using STEP)
 * @param y Y coordinate of starting point (or offset if using STEP)
 * @param fillcol Fill color (uses current color if not specified)
 * @param bordercol Border color (uses fill color if not specified)
 * @param passed Bit flags: bit 0=STEP coordinates, bit 1=fill color provided, bit 2=border color provided
 */
// 32-bit WITH BENDING
void sub_paint32(float x, float y, uint32 fillcol, uint32 bordercol, int32 passed) {

    // uses 2 buffers, a and b, and swaps between them for reading and creating
    static uint32 a_n = 0;
    static uint16 *a_x = (uint16 *)malloc(2 * 65536), *a_y = (uint16 *)malloc(2 * 65536);
    static uint8 *a_t = (uint8 *)malloc(65536);
    static uint32 b_n = 0;
    static uint16 *b_x = (uint16 *)malloc(2 * 65536), *b_y = (uint16 *)malloc(2 * 65536);
    static uint8 *b_t = (uint8 *)malloc(65536);
    static uint8 *done = (uint8 *)calloc(640 * 480, 1);
    static int32 ix, iy, i, t, x2, y2;
    static uint32 offset;
    static uint8 *cp;
    static uint16 *sp;
    // overrides
    static int32 done_size = 640 * 480;
    static uint32 *qbg_active_page_offset;                                      // override
    static int32 qbg_width, qbg_view_x1, qbg_view_y1, qbg_view_x2, qbg_view_y2; // override
    static uint32 *doff32, destcol;

    if ((passed & 2) == 0)
        fillcol = write_page->color;
    if ((passed & 4) == 0)
        bordercol = fillcol;
    write_page->draw_color = fillcol;

    if (passed & 1) {
        write_page->x += x;
        write_page->y += y;
    } else {
        write_page->x = x;
        write_page->y = y;
    }

    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            ix = qbr_float_to_long(write_page->x * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            ix = qbr_float_to_long(write_page->x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y) + write_page->view_offset_y;
        }
    } else {
        ix = qbr_float_to_long(write_page->x);
        iy = qbr_float_to_long(write_page->y);
    }

    // return if offscreen
    if ((ix < write_page->view_x1) || (iy < write_page->view_y1) || (ix > write_page->view_x2) || (iy > write_page->view_y2)) {
        return;
    }

    // overrides
    qbg_active_page_offset = write_page->offset32;
    qbg_width = write_page->width;
    qbg_view_x1 = write_page->view_x1;
    qbg_view_y1 = write_page->view_y1;
    qbg_view_x2 = write_page->view_x2;
    qbg_view_y2 = write_page->view_y2;
    i = write_page->width * write_page->height;
    if (i > done_size) {
        free(done);
        done = (uint8 *)calloc(i, 1);
    }

    // return if first point is the bordercolor
    if (qbg_active_page_offset[iy * qbg_width + ix] == bordercol)
        return;

    // create first node
    a_x[0] = ix;
    a_y[0] = iy;
    a_t[0] = 15;
    // types:
    //&1=check left
    //&2=check right
    //&4=check above
    //&8=check below

    a_n = 1;
    // qbg_active_page_offset[iy*qbg_width+ix]=fillcol;
    offset = iy * qbg_width + ix;
    //--------plot pixel--------
    doff32 = qbg_active_page_offset + offset;
    switch (fillcol & 0xFF000000) {
    case 0xFF000000:
        *doff32 = fillcol;
        break;
    case 0x0:
        // doff32;
        break;
    case 0x80000000:
        *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
        break;
    case 0x7F000000:
        *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
        break;
    default:
        destcol = *doff32;
        cp = cblend + (fillcol >> 24 << 16);
        *doff32 = cp[(fillcol << 8 & 0xFF00) + (destcol & 255)] + (cp[(fillcol & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                  (cp[(fillcol >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(fillcol >> 24) + (destcol >> 16 & 0xFF00)] << 24);
    }; // switch
    //--------done plot pixel--------
    done[iy * qbg_width + ix] = 1;

nextpass:
    b_n = 0;
    for (i = 0; i < a_n; i++) {
        t = a_t[i];
        ix = a_x[i];
        iy = a_y[i];

        // left
        if (t & 1) {
            x2 = ix - 1;
            y2 = iy;
            if (x2 >= qbg_view_x1) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        //--------plot pixel--------
                        doff32 = qbg_active_page_offset + offset;
                        switch (fillcol & 0xFF000000) {
                        case 0xFF000000:
                            *doff32 = fillcol;
                            break;
                        case 0x0:
                            // doff32;
                            break;
                        case 0x80000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                            break;
                        default:
                            destcol = *doff32;
                            cp = cblend + (fillcol >> 24 << 16);
                            *doff32 = cp[(fillcol << 8 & 0xFF00) + (destcol & 255)] + (cp[(fillcol & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                      (cp[(fillcol >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(fillcol >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        b_t[b_n] = 13;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // right
        if (t & 2) {
            x2 = ix + 1;
            y2 = iy;
            if (x2 <= qbg_view_x2) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        //--------plot pixel--------
                        doff32 = qbg_active_page_offset + offset;
                        switch (fillcol & 0xFF000000) {
                        case 0xFF000000:
                            *doff32 = fillcol;
                            break;
                        case 0x0:
                            // doff32;
                            break;
                        case 0x80000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                            break;
                        default:
                            destcol = *doff32;
                            cp = cblend + (fillcol >> 24 << 16);
                            *doff32 = cp[(fillcol << 8 & 0xFF00) + (destcol & 255)] + (cp[(fillcol & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                      (cp[(fillcol >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(fillcol >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        b_t[b_n] = 14;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // above
        if (t & 4) {
            x2 = ix;
            y2 = iy - 1;
            if (y2 >= qbg_view_y1) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        //--------plot pixel--------
                        doff32 = qbg_active_page_offset + offset;
                        switch (fillcol & 0xFF000000) {
                        case 0xFF000000:
                            *doff32 = fillcol;
                            break;
                        case 0x0:
                            // doff32;
                            break;
                        case 0x80000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                            break;
                        default:
                            destcol = *doff32;
                            cp = cblend + (fillcol >> 24 << 16);
                            *doff32 = cp[(fillcol << 8 & 0xFF00) + (destcol & 255)] + (cp[(fillcol & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                      (cp[(fillcol >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(fillcol >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        b_t[b_n] = 7;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // below
        if (t & 8) {
            x2 = ix;
            y2 = iy + 1;
            if (y2 <= qbg_view_y2) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        //--------plot pixel--------
                        doff32 = qbg_active_page_offset + offset;
                        switch (fillcol & 0xFF000000) {
                        case 0xFF000000:
                            *doff32 = fillcol;
                            break;
                        case 0x0:
                            // doff32;
                            break;
                        case 0x80000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                            break;
                        case 0x7F000000:
                            *doff32 = (((*doff32 & 0xFEFEFE) + (fillcol & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                            break;
                        default:
                            destcol = *doff32;
                            cp = cblend + (fillcol >> 24 << 16);
                            *doff32 = cp[(fillcol << 8 & 0xFF00) + (destcol & 255)] + (cp[(fillcol & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                                      (cp[(fillcol >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(fillcol >> 24) + (destcol >> 16 & 0xFF00)] << 24);
                        }; // switch
                        //--------done plot pixel--------
                        b_t[b_n] = 11;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

    } // i

    // no new nodes?
    if (b_n == 0) {
        memset(done, 0, write_page->width * write_page->height); // cleanup
        return;                                                  // finished!
    }

    // swap a & b arrays
    sp = a_x;
    a_x = b_x;
    b_x = sp;
    sp = a_y;
    a_y = b_y;
    b_y = sp;
    cp = a_t;
    a_t = b_t;
    b_t = cp;
    a_n = b_n;

    goto nextpass;
}

// 32-bit NO ALPHA BENDING
void sub_paint32x(float x, float y, uint32 fillcol, uint32 bordercol, int32 passed) {

    // uses 2 buffers, a and b, and swaps between them for reading and creating
    static uint32 a_n = 0;
    static uint16 *a_x = (uint16 *)malloc(2 * 65536), *a_y = (uint16 *)malloc(2 * 65536);
    static uint8 *a_t = (uint8 *)malloc(65536);
    static uint32 b_n = 0;
    static uint16 *b_x = (uint16 *)malloc(2 * 65536), *b_y = (uint16 *)malloc(2 * 65536);
    static uint8 *b_t = (uint8 *)malloc(65536);
    static uint8 *done = (uint8 *)calloc(640 * 480, 1);
    static int32 ix, iy, i, t, x2, y2;
    static uint32 offset;
    static uint8 *cp;
    static uint16 *sp;
    // overrides
    static int32 done_size = 640 * 480;
    static uint32 *qbg_active_page_offset;                                      // override
    static int32 qbg_width, qbg_view_x1, qbg_view_y1, qbg_view_x2, qbg_view_y2; // override

    if ((passed & 2) == 0)
        fillcol = write_page->color;
    if ((passed & 4) == 0)
        bordercol = fillcol;
    write_page->draw_color = fillcol;

    if (passed & 1) {
        write_page->x += x;
        write_page->y += y;
    } else {
        write_page->x = x;
        write_page->y = y;
    }

    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            ix = qbr_float_to_long(write_page->x * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            ix = qbr_float_to_long(write_page->x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y) + write_page->view_offset_y;
        }
    } else {
        ix = qbr_float_to_long(write_page->x);
        iy = qbr_float_to_long(write_page->y);
    }

    // return if offscreen
    if ((ix < write_page->view_x1) || (iy < write_page->view_y1) || (ix > write_page->view_x2) || (iy > write_page->view_y2)) {
        return;
    }

    // overrides
    qbg_active_page_offset = write_page->offset32;
    qbg_width = write_page->width;
    qbg_view_x1 = write_page->view_x1;
    qbg_view_y1 = write_page->view_y1;
    qbg_view_x2 = write_page->view_x2;
    qbg_view_y2 = write_page->view_y2;
    i = write_page->width * write_page->height;
    if (i > done_size) {
        free(done);
        done = (uint8 *)calloc(i, 1);
    }

    // return if first point is the bordercolor
    if (qbg_active_page_offset[iy * qbg_width + ix] == bordercol)
        return;

    // create first node
    a_x[0] = ix;
    a_y[0] = iy;
    a_t[0] = 15;
    // types:
    //&1=check left
    //&2=check right
    //&4=check above
    //&8=check below

    a_n = 1;
    qbg_active_page_offset[iy * qbg_width + ix] = fillcol;
    done[iy * qbg_width + ix] = 1;

nextpass:
    b_n = 0;
    for (i = 0; i < a_n; i++) {
        t = a_t[i];
        ix = a_x[i];
        iy = a_y[i];

        // left
        if (t & 1) {
            x2 = ix - 1;
            y2 = iy;
            if (x2 >= qbg_view_x1) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 13;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // right
        if (t & 2) {
            x2 = ix + 1;
            y2 = iy;
            if (x2 <= qbg_view_x2) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 14;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // above
        if (t & 4) {
            x2 = ix;
            y2 = iy - 1;
            if (y2 >= qbg_view_y1) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 7;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // below
        if (t & 8) {
            x2 = ix;
            y2 = iy + 1;
            if (y2 <= qbg_view_y2) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 11;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

    } // i

    // no new nodes?
    if (b_n == 0) {
        memset(done, 0, write_page->width * write_page->height); // cleanup
        return;                                                  // finished!
    }

    // swap a & b arrays
    sp = a_x;
    a_x = b_x;
    b_x = sp;
    sp = a_y;
    a_y = b_y;
    b_y = sp;
    cp = a_t;
    a_t = b_t;
    b_t = cp;
    a_n = b_n;

    goto nextpass;
}

// 8-bit (default entry point)
void sub_paint(float x, float y, uint32 fillcol, uint32 bordercol, qbs *backgroundstr, int32 passed) {
    if (is_error_pending())
        return;
    if (write_page->text) {
        error(5);
        return;
    }
    if (passed & 8) {
        error(5);
        return;
    }

    if (write_page->bytes_per_pixel == 4) {
        if (write_page->alpha_disabled) {
            sub_paint32x(x, y, fillcol, bordercol, passed);
            return;
        } else {
            sub_paint32(x, y, fillcol, bordercol, passed);
            return;
        }
    }

    // uses 2 buffers, a and b, and swaps between them for reading and creating
    static uint32 a_n = 0;
    static uint16 *a_x = (uint16 *)malloc(2 * 65536), *a_y = (uint16 *)malloc(2 * 65536);
    static uint8 *a_t = (uint8 *)malloc(65536);
    static uint32 b_n = 0;
    static uint16 *b_x = (uint16 *)malloc(2 * 65536), *b_y = (uint16 *)malloc(2 * 65536);
    static uint8 *b_t = (uint8 *)malloc(65536);
    static uint8 *done = (uint8 *)calloc(640 * 480, 1);
    static int32 ix, iy, i, t, x2, y2;
    static uint32 offset;
    static uint8 *cp;
    static uint16 *sp;
    // overrides
    static int32 done_size = 640 * 480;
    static uint8 *qbg_active_page_offset;                                       // override
    static int32 qbg_width, qbg_view_x1, qbg_view_y1, qbg_view_x2, qbg_view_y2; // override

    if ((passed & 2) == 0)
        fillcol = write_page->color;
    if ((passed & 4) == 0)
        bordercol = fillcol;
    fillcol &= write_page->mask;
    bordercol &= write_page->mask;
    write_page->draw_color = fillcol;

    if (passed & 1) {
        write_page->x += x;
        write_page->y += y;
    } else {
        write_page->x = x;
        write_page->y = y;
    }

    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            ix = qbr_float_to_long(write_page->x * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            ix = qbr_float_to_long(write_page->x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y) + write_page->view_offset_y;
        }
    } else {
        ix = qbr_float_to_long(write_page->x);
        iy = qbr_float_to_long(write_page->y);
    }

    // return if offscreen
    if ((ix < write_page->view_x1) || (iy < write_page->view_y1) || (ix > write_page->view_x2) || (iy > write_page->view_y2)) {
        return;
    }

    // overrides
    qbg_active_page_offset = write_page->offset;
    qbg_width = write_page->width;
    qbg_view_x1 = write_page->view_x1;
    qbg_view_y1 = write_page->view_y1;
    qbg_view_x2 = write_page->view_x2;
    qbg_view_y2 = write_page->view_y2;
    i = write_page->width * write_page->height;
    if (i > done_size) {
        free(done);
        done = (uint8 *)calloc(i, 1);
    }

    // return if first point is the bordercolor
    if (qbg_active_page_offset[iy * qbg_width + ix] == bordercol)
        return;

    // create first node
    a_x[0] = ix;
    a_y[0] = iy;
    a_t[0] = 15;
    // types:
    //&1=check left
    //&2=check right
    //&4=check above
    //&8=check below

    a_n = 1;
    qbg_active_page_offset[iy * qbg_width + ix] = fillcol;
    done[iy * qbg_width + ix] = 1;

nextpass:
    b_n = 0;
    for (i = 0; i < a_n; i++) {
        t = a_t[i];
        ix = a_x[i];
        iy = a_y[i];

        // left
        if (t & 1) {
            x2 = ix - 1;
            y2 = iy;
            if (x2 >= qbg_view_x1) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 13;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // right
        if (t & 2) {
            x2 = ix + 1;
            y2 = iy;
            if (x2 <= qbg_view_x2) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 14;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // above
        if (t & 4) {
            x2 = ix;
            y2 = iy - 1;
            if (y2 >= qbg_view_y1) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 7;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

        // below
        if (t & 8) {
            x2 = ix;
            y2 = iy + 1;
            if (y2 <= qbg_view_y2) {
                offset = y2 * qbg_width + x2;
                if (!done[offset]) {
                    done[offset] = 1;
                    if (qbg_active_page_offset[offset] != bordercol) {
                        qbg_active_page_offset[offset] = fillcol;
                        b_t[b_n] = 11;
                        b_x[b_n] = x2;
                        b_y[b_n] = y2;
                        b_n++; // add new node
                    }
                }
            }
        }

    } // i

    // no new nodes?
    if (b_n == 0) {
        memset(done, 0, write_page->width * write_page->height); // cleanup
        return;                                                  // finished!
    }

    // swap a & b arrays
    sp = a_x;
    a_x = b_x;
    b_x = sp;
    sp = a_y;
    a_y = b_y;
    b_y = sp;
    cp = a_t;
    a_t = b_t;
    b_t = cp;
    a_n = b_n;

    goto nextpass;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void getptsize_1bpp(const qbs *pt, int32 *sx, int32 *sy) {
    *sx = 8;
    *sy = pt->len;
}

uint32 getptcol_1bpp(const qbs *pt, int32 x, int32 y) {
    return (pt->chr[y] >> (7 - x)) & 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void getptsize_2bpp(const qbs *pt, int32 *sx, int32 *sy) {
    *sx = 4;
    *sy = pt->len;
}

uint32 getptcol_2bpp(const qbs *pt, int32 x, int32 y) {
    return (pt->chr[y] >> ((3 - x) << 1)) & 0x03;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void getptsize_4bpp(const qbs *pt, int32 *sx, int32 *sy) {
    *sx = 8;
    *sy = pt->len >> 2;
    if (((*sy) << 2) < pt->len)
        ++*sy;
}

uint32 getptcol_4bpp(const qbs *pt, int32 x, int32 y) {
    int quadstart = (y << 2);
    uint32 clr = 0;
    for (uint8 i = 0; i < 4; ++i) {
        uint8 byte = ((i + quadstart) >= pt->len ? 0 : pt->chr[i + quadstart]);
        if (byte & (1 << (7 - x)))
            clr |= (1 << i);
    }
    return clr;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void getptsize_8bpp(const qbs *pt, int32 *sx, int32 *sy) {
    *sx = 1;
    *sy = pt->len;
}

uint32 getptcol_8bpp(const qbs *pt, int x, int y) {
    return pt->chr[y];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void sub_paint(float x, float y, qbs *fillstr, uint32 bordercol, qbs *backgroundstr, int32 passed) {
    if (is_error_pending())
        return;

    // uses 2 buffers, a and b, and swaps between them for reading and creating
    static uint32 a_n = 0;
    static uint16 *a_x = (uint16 *)malloc(2 * 65536), *a_y = (uint16 *)malloc(2 * 65536);
    static uint8 *a_t = (uint8 *)malloc(65536);
    static uint32 b_n = 0;
    static uint16 *b_x = (uint16 *)malloc(2 * 65536), *b_y = (uint16 *)malloc(2 * 65536);
    static uint8 *b_t = (uint8 *)malloc(65536);
    static uint8 *done = (uint8 *)calloc(640 * 480, 1);
    static int32 ix, iy, i, t, x2, y2;
    static uint32 offset;
    static uint8 *cp;
    static uint16 *sp;
    static int32 done_size = 640 * 480;

    if (qbg_text_only) {
        error(5);
        return;
    }
    if ((passed & 2) == 0) {
        error(5);
        return;
    } // must be called with this parameter!

    if (fillstr->len == 0) {
        error(5);
        return;
    }

    static uint8 tile[8][64];
    static int32 sx, sy;
    void (*getptsize)(const qbs *pt, int32 *sx, int32 *sy) = getptsize_4bpp;
    uint32 (*getptcol)(const qbs *pt, int32 x, int32 y) = getptcol_4bpp;

    switch (write_page->compatible_mode) {
    case 1:
        getptsize = getptsize_2bpp;
        getptcol = getptcol_2bpp;
        break;
    case 2:
        getptsize = getptsize_1bpp;
        getptcol = getptcol_1bpp;
        break;
    case 7:
    case 8:
    case 9:
    case 12:
        getptsize = getptsize_4bpp;
        getptcol = getptcol_4bpp;
        break;
    case 13:
        getptsize = getptsize_8bpp;
        getptcol = getptcol_8bpp;
        break;
    }

    getptsize(fillstr, &sx, &sy);
    for (int x = 0; x < sx; ++x)
        for (int y = 0; y < sy; ++y)
            tile[x][y] = getptcol(fillstr, x, y);

    bordercol &= write_page->mask;

    if (passed & 1) {
        write_page->x += x;
        write_page->y += y;
    } else {
        write_page->x = x;
        write_page->y = y;
    }

    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            ix = qbr_float_to_long(write_page->x * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            ix = qbr_float_to_long(write_page->x) + write_page->view_offset_x;
            iy = qbr_float_to_long(write_page->y) + write_page->view_offset_y;
        }
    } else {
        ix = qbr_float_to_long(write_page->x);
        iy = qbr_float_to_long(write_page->y);
    }

    // return if offscreen
    if ((ix < write_page->view_x1) || (iy < write_page->view_y1) || (ix > write_page->view_x2) || (iy > write_page->view_y2)) {
        return;
    }

    // overrides
    qbg_active_page_offset = write_page->offset;
    qbg_width = write_page->width;
    qbg_view_x1 = write_page->view_x1;
    qbg_view_y1 = write_page->view_y1;
    qbg_view_x2 = write_page->view_x2;
    qbg_view_y2 = write_page->view_y2;
    i = write_page->width * write_page->height;
    if (i > done_size) {
        free(done);
        done = (uint8 *)calloc(i, 1);
    }

    // The original color of the starting location
    uint32_t startingColor = qbg_active_page_offset[iy * qbg_width + ix];

    bool borderColorProvided = passed & 4;

    // Exit early if we're already at the border color
    if (borderColorProvided && qbg_active_page_offset[iy * qbg_width + ix] == bordercol)
        return;

    // create first node
    a_x[0] = ix;
    a_y[0] = iy;
    a_t[0] = 15;

    a_n = 1;
    qbg_active_page_offset[iy * qbg_width + ix] = tile[ix % sx][iy % sy];
    done[iy * qbg_width + ix] = 1;

    // Each index maps to a direction, in the order:
    // Left, Right, Up, Down
    int32_t xdelta[4] = {-1, 1, 0, 0};
    int32_t ydelta[4] = {0, 0, -1, 1};

    // The bits indicate the directions that should be checked for the next
    // pixel, we ignore the direction we came from.
    uint32_t dirCheckMap[4] = {
        1 | 4 | 8, // Left, Up, Down
        2 | 4 | 8, // Right, Up, Down
        1 | 2 | 4, // Left, Right, Up
        1 | 2 | 8, // Left, Right, Down
    };

    while (true) {
        b_n = 0;
        for (i = 0; i < a_n; i++) {
            t = a_t[i];
            ix = a_x[i];
            iy = a_y[i];

            for (int k = 0; k < 4; k++) {
                if ((t & (1 << k)) == 0)
                    continue;

                x2 = ix + xdelta[k];
                y2 = iy + ydelta[k];

                // Verify dimensions are within bounds
                if (x2 >= qbg_view_x1 && x2 <= qbg_view_x2 && y2 >= qbg_view_y1 && y2 <= qbg_view_y2) {
                    offset = y2 * qbg_width + x2;

                    // Check that we haven't done this pixel yet
                    if (!done[offset]) {
                        done[offset] = 1;

                        // We either check that we didn't hit the border color
                        // (if provided), or that we're still the starting
                        // color.
                        if ((borderColorProvided && qbg_active_page_offset[offset] != bordercol) ||
                            (!borderColorProvided && qbg_active_page_offset[offset] == startingColor)) {

                            qbg_active_page_offset[offset] = tile[x2 % sx][y2 % sy];
                            b_t[b_n] = dirCheckMap[k];
                            b_x[b_n] = x2;
                            b_y[b_n] = y2;
                            b_n++; // add new node
                        }
                    }
                }
            }
        }

        // no new nodes?
        if (b_n == 0) {
            memset(done, 0, write_page->width * write_page->height); // cleanup
            return;                                                  // finished!
        }

        // swap a & b arrays
        sp = a_x;
        a_x = b_x;
        b_x = sp;

        sp = a_y;
        a_y = b_y;
        b_y = sp;

        cp = a_t;
        a_t = b_t;
        b_t = cp;

        a_n = b_n;
    }
}

void sub_circle(double x, double y, double r, uint32 col, double start, double end, double aspect, int32 passed) {
    //                                                &2         &4           &8         &16
    //[{STEP}](?,?),?[,[?][,[?][,[?][,?]]]]
    if (is_error_pending())
        return;

    // data
    static double pi = 3.1415926535897932, pi2 = 6.2831853071795865;
    static int32 line_to_start, line_from_end;
    static int32 ix, iy; // integer screen co-ordinates of circle's centre
    static double xspan, yspan;
    static double c; // circumference
    static double px, py;
    static double sinb, cosb; // second angle used in double-angle-formula
    static int32 pixels;
    static double tmp;
    static int32 tmpi;
    static int32 i;
    static int32 exclusive;
    static double arc1, arc2, arc3, arc4, arcinc;
    static double px2, py2;
    static int32 x2, y2;
    static int32 x3, y3;
    static int32 lastplotted_x2, lastplotted_y2;
    static int32 lastchecked_x2, lastchecked_y2;

    if (write_page->text) {
        error(5);
        return;
    }

    // lines to & from centre
    if (!(passed & 4))
        start = 0;
    if (!(passed & 8))
        end = pi2;
    line_to_start = 0;
    if (start < 0) {
        line_to_start = 1;
        start = -start;
    }
    line_from_end = 0;
    if (end < 0) {
        line_from_end = 1;
        end = -end;
    }

    // error checking
    if (start > pi2) {
        error(5);
        return;
    }
    if (end > pi2) {
        error(5);
        return;
    }

    // when end<start, the arc of the circle that wouldn't have been drawn if start & end
    // were swapped is drawn
    exclusive = 0;
    if (end < start) {
        tmp = start;
        start = end;
        end = tmp;
        tmpi = line_to_start;
        line_to_start = line_from_end;
        line_from_end = tmpi;
        exclusive = 1;
    }

    // calc. centre
    if (passed & 1) {
        x = write_page->x + x;
        y = write_page->y + y;
    }
    write_page->x = x;
    write_page->y = y; // set graphics cursor position to circle's centre

    r = x + r; // the difference between x & x+r in pixels will be the radius in pixels
    // resolve coordinates (but keep as floats)
    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            x = x * write_page->scaling_x + write_page->scaling_offset_x + write_page->view_offset_x;
            y = y * write_page->scaling_y + write_page->scaling_offset_y + write_page->view_offset_y;
            r = r * write_page->scaling_x + write_page->scaling_offset_x + write_page->view_offset_x;
        } else {
            x = x + write_page->view_offset_x;
            y = y + write_page->view_offset_y;
            r = r + write_page->view_offset_x;
        }
    }
    if (x < 0)
        ix = x - 0.5;
    else
        ix = x + 0.5;
    if (y < 0)
        iy = y - 0.5;
    else
        iy = y + 0.5;
    r = std::fabs(r - x); // r is now a radius in pixels

    // adjust vertical and horizontal span of the circle based on aspect ratio
    xspan = r;
    yspan = r;
    if (!(passed & 16)) {
        aspect = 1; // Note: default aspect ratio is 1:1 for QB64 specific modes (256/32)
        if (write_page->compatible_mode == 1)
            aspect = 4.0 * (200.0 / 320.0) / 3.0;
        if (write_page->compatible_mode == 2)
            aspect = 4.0 * (200.0 / 640.0) / 3.0;
        if (write_page->compatible_mode == 7)
            aspect = 4.0 * (200.0 / 320.0) / 3.0;
        if (write_page->compatible_mode == 8)
            aspect = 4.0 * (200.0 / 640.0) / 3.0;
        if (write_page->compatible_mode == 9)
            aspect = 4.0 * (350.0 / 640.0) / 3.0;
        if (write_page->compatible_mode == 10)
            aspect = 4.0 * (350.0 / 640.0) / 3.0;
        if (write_page->compatible_mode == 11)
            aspect = 4.0 * (480.0 / 640.0) / 3.0;
        if (write_page->compatible_mode == 12)
            aspect = 4.0 * (480.0 / 640.0) / 3.0;
        if (write_page->compatible_mode == 13)
            aspect = 4.0 * (200.0 / 320.0) / 3.0;
        // Old method: aspect=4.0*((double)write_page->height/(double)write_page->width)/3.0;
    }
    if (aspect >= 0) {
        if (aspect < 1) {
            // aspect: 0 to 1
            yspan *= aspect;
        }
        if (aspect > 1) {
            // aspect: 1 to infinity
            xspan /= aspect;
        }
    } else {
        if (aspect > -1) {
            // aspect: -1 to 0
            yspan *= (1 + aspect);
        }
        // if aspect<-1 no change is required
    }

    // skip everything if none of the circle is inside current viwport
    if ((x + xspan + 0.5) < write_page->view_x1)
        return;
    if ((y + yspan + 0.5) < write_page->view_y1)
        return;
    if ((x - xspan - 0.5) > write_page->view_x2)
        return;
    if ((y - yspan - 0.5) > write_page->view_y2)
        return;

    if (!(passed & 2))
        col = write_page->color;
    write_page->draw_color = col;

    // pre-set/pre-calculate values
    c = pi2 * r;
    pixels = c / 4.0 + 0.5;
    arc1 = 0;
    arc2 = pi;
    arc3 = pi;
    arc4 = pi2;
    arcinc = (pi / 2) / (double)pixels;
    sinb = std::sin(arcinc);
    cosb = std::cos(arcinc);
    lastplotted_x2 = -1;
    lastchecked_x2 = -1;
    i = 0;

    if (line_to_start) {
        px = std::cos(start);
        py = std::sin(start);
        x2 = px * xspan + 0.5;
        y2 = py * yspan - 0.5;
        fast_line(ix, iy, ix + x2, iy - y2, col);
    }

    px = 1;
    py = 0;

drawcircle:
    x2 = px * xspan + 0.5;
    y2 = py * yspan - 0.5;

    if (i == 0) {
        lastchecked_x2 = x2;
        lastchecked_y2 = y2;
        goto plot;
    }

    if ((abs(x2 - lastplotted_x2) >= 2) || (abs(y2 - lastplotted_y2) >= 2)) {
    plot:
        if (exclusive) {
            if ((arc1 <= start) || (arc1 >= end)) {
                pset_and_clip(ix + lastchecked_x2, iy + lastchecked_y2, col);
            }
            if ((arc2 <= start) || (arc2 >= end)) {
                pset_and_clip(ix - lastchecked_x2, iy + lastchecked_y2, col);
            }
            if ((arc3 <= start) || (arc3 >= end)) {
                pset_and_clip(ix - lastchecked_x2, iy - lastchecked_y2, col);
            }
            if ((arc4 <= start) || (arc4 >= end)) {
                pset_and_clip(ix + lastchecked_x2, iy - lastchecked_y2, col);
            }
        } else { // inclusive
            if ((arc1 >= start) && (arc1 <= end)) {
                pset_and_clip(ix + lastchecked_x2, iy + lastchecked_y2, col);
            }
            if ((arc2 >= start) && (arc2 <= end)) {
                pset_and_clip(ix - lastchecked_x2, iy + lastchecked_y2, col);
            }
            if ((arc3 >= start) && (arc3 <= end)) {
                pset_and_clip(ix - lastchecked_x2, iy - lastchecked_y2, col);
            }
            if ((arc4 >= start) && (arc4 <= end)) {
                pset_and_clip(ix + lastchecked_x2, iy - lastchecked_y2, col);
            }
        }
        if (i > pixels)
            goto allplotted;
        lastplotted_x2 = lastchecked_x2;
        lastplotted_y2 = lastchecked_y2;
    }
    lastchecked_x2 = x2;
    lastchecked_y2 = y2;

    if (i <= pixels) {
        i++;
        if (i > pixels)
            goto plot;
        px2 = px * cosb + py * sinb;
        py = py * cosb - px * sinb;
        px = px2;
        if (i) {
            arc1 += arcinc;
            arc2 -= arcinc;
            arc3 += arcinc;
            arc4 -= arcinc;
        }
        goto drawcircle;
    }
allplotted:

    if (line_from_end) {
        px = std::cos(end);
        py = std::sin(end);
        x2 = px * xspan + 0.5;
        y2 = py * yspan - 0.5;
        fast_line(ix, iy, ix + x2, iy - y2, col);
    }

} // sub_circle

/**
 * Gets the color value at a specific pixel position (no clipping).
 * 
 * Returns the color value at the specified coordinates without any clipping
 * or coordinate transformation. Used internally by func_point().
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @return Color value at the specified position, or NULL on error
 */
uint32 point(int32 x, int32 y) { // does not clip!
    if (read_page->bytes_per_pixel == 1) {
        return read_page->offset[y * read_page->width + x] & read_page->mask;
    } else {
        return read_page->offset32[y * read_page->width + x];
    }
    return NULL;
}

/**
 * Gets the color value at a point or cursor position information.
 * 
 * If coordinates are provided, returns the color value at that point.
 * If no coordinates are provided (x=0-3), returns cursor position information:
 * 0=scaled X, 1=scaled Y, 2=unscaled X, 3=unscaled Y.
 * 
 * @param x X coordinate or cursor info index (0-3)
 * @param y Y coordinate (ignored if x is 0-3)
 * @param passed Bit flags indicating which parameters were provided
 * @return Color value, cursor position, or -1 if point is outside viewport
 */
double func_point(float x, float y, int32 passed) {
    static int32 x2, y2, i;

    if (!passed) {
        if (write_page->text) {
            error(5);
            return 0;
        }
        i = qbr_float_to_long(x);
        if ((i < 0) || (i > 3)) {
            error(5);
            return 0;
        }
        switch (i) {
        case 0:
            if (write_page->clipping_or_scaling == 2) {
                return qbr_float_to_long(write_page->x * write_page->scaling_x + write_page->scaling_offset_x);
            }
            return qbr_float_to_long(write_page->x);
            break;
        case 1:
            if (write_page->clipping_or_scaling == 2) {
                return qbr_float_to_long(write_page->y * write_page->scaling_y + write_page->scaling_offset_y);
            }
            return qbr_float_to_long(write_page->y);
            break;
        case 2:
            return write_page->x;
            break;
        case 3:
            return write_page->y;
            break;
        default:
            error(5);
            return 0;
        }
    } //! passed

    if (read_page->text) {
        error(5);
        return 0;
    }
    if (read_page->clipping_or_scaling) {
        if (read_page->clipping_or_scaling == 2) {
            x2 = qbr_float_to_long(x * read_page->scaling_x + read_page->scaling_offset_x) + read_page->view_offset_x;
            y2 = qbr_float_to_long(y * read_page->scaling_y + read_page->scaling_offset_y) + read_page->view_offset_y;
        } else {
            x2 = qbr_float_to_long(x) + read_page->view_offset_x;
            y2 = qbr_float_to_long(y) + read_page->view_offset_y;
        }
    } else {
        x2 = qbr_float_to_long(x);
        y2 = qbr_float_to_long(y);
    }
    if (x2 >= read_page->view_x1) {
        if (x2 <= read_page->view_x2) {
            if (y2 >= read_page->view_y1) {
                if (y2 <= read_page->view_y2) {
                    return point(x2, y2);
                }
            }
        }
    }
    return -1;
}

/**
 * Sets a pixel to a specific color.
 * 
 * Draws a single pixel at the specified coordinates with the given color.
 * Supports STEP coordinates and uses the current color if not specified.
 * Updates the graphics cursor position.
 * 
 * @param x X coordinate (or offset if using STEP)
 * @param y Y coordinate (or offset if using STEP)
 * @param col Color value (uses current color if not specified)
 * @param passed Bit flags: bit 0=STEP coordinates, bit 1=color provided
 */
void sub_pset(float x, float y, uint32 col, int32 passed) {
    if (is_error_pending())
        return;
    static int32 x2, y2;
    if (!write_page->compatible_mode) {
        error(5);
        return;
    }
    // Special Format: [{STEP}](?,?),[?]
    if (passed & 1) {
        write_page->x += x;
        write_page->y += y;
    } else {
        write_page->x = x;
        write_page->y = y;
    }
    if (!(passed & 2))
        col = write_page->color;
    write_page->draw_color = col;
    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            x2 = qbr(write_page->x * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            y2 = qbr(write_page->y * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            x2 = qbr(write_page->x) + write_page->view_offset_x;
            y2 = qbr(write_page->y) + write_page->view_offset_y;
        }
        if (x2 >= write_page->view_x1) {
            if (x2 <= write_page->view_x2) {
                if (y2 >= write_page->view_y1) {
                    if (y2 <= write_page->view_y2) {
                        pset(x2, y2, col);
                    }
                }
            }
        }
        return;
    } else {
        x2 = qbr(write_page->x);
        if (x2 >= 0) {
            if (x2 < write_page->width) {
                y2 = qbr(write_page->y);
                if (y2 >= 0) {
                    if (y2 < write_page->height) {
                        pset(x2, y2, col);
                    }
                }
            }
        }
    }
    return;
}

/**
 * Sets a pixel to the background color.
 * 
 * Draws a single pixel at the specified coordinates using the background color
 * (or specified color if provided). This is equivalent to PSET with background color.
 * 
 * @param x X coordinate (or offset if using STEP)
 * @param y Y coordinate (or offset if using STEP)
 * @param col Color value (uses background color if not specified)
 * @param passed Bit flags: bit 0=STEP coordinates, bit 1=color provided
 */
void sub_preset(float x, float y, uint32 col, int32 passed) {
    if (is_error_pending())
        return;
    if (!(passed & 2)) {
        col = write_page->background_color;
        passed |= 2;
    }
    sub_pset(x, y, col, passed);
    return;
}

//----------------------------------------------------------------------------------------------------------------------
// Image Management Functions
// Extracted from libqb.cpp - handles image creation, copying, freeing, and properties
//----------------------------------------------------------------------------------------------------------------------

// Additional extern declarations for image functions
extern int32 console_image;

// Constants for hardware image creation (from libqb.cpp)
#define NEW_HARDWARE_IMG__BUFFER_CONTENT 1
#define NEW_HARDWARE_IMG__DUPLICATE_PROVIDED_BUFFER 2

/**
 * Creates a new image with specified dimensions and color depth.
 * 
 * Allocates a new image buffer with the given width, height, and bits per pixel.
 * If bpp is not specified, uses the current write page's color mode.
 * The new image adopts the current page's palette, font, colors, and settings.
 * 
 * @param x Width of the image in pixels (must be > 0)
 * @param y Height of the image in pixels (must be > 0)
 * @param bpp Bits per pixel: 0-2, 7-13, 256, or 32 (uses current mode if not specified)
 * @param passed Bit flags indicating which parameters were provided
 * @return Negative image handle on success, -1 on error, 0 if error pending
 */
int32 func__newimage(int32 x, int32 y, int32 bpp, int32 passed) {
    static int32 i;
    if (is_error_pending())
        return 0;
    if (x <= 0 || y <= 0) {
        error(5);
        return 0;
    }
    if (!passed) {
        bpp = write_page->compatible_mode;
    } else {
        i = 0;
        if (bpp >= 0 && bpp <= 2)
            i = 1;
        if (bpp >= 7 && bpp <= 13)
            i = 1;
        if (bpp == 256)
            i = 1;
        if (bpp == 32)
            i = 1;
        if (!i) {
            error(5);
            return 0;
        }
    }
    i = imgnew(x, y, bpp);
    if (!i)
        return -1;
    if (!passed) {
        // adopt palette
        if (write_page->pal) {
            memcpy(img[i].pal, write_page->pal, 1024);
        }
        // adopt font
        sub__font(write_page->font, -i, 1);
        // adopt colors
        img[i].color = write_page->color;
        img[i].background_color = write_page->background_color;
        // adopt transparent color
        img[i].transparent_color = write_page->transparent_color;
        // adopt blend state
        img[i].alpha_disabled = write_page->alpha_disabled;
        // adopt print mode
        img[i].print_mode = write_page->print_mode;
    }
    return -i;
}

/**
 * Creates a copy of an existing image.
 * 
 * Duplicates an image, creating a new image with the same dimensions,
 * color depth, and pixel data. Optionally converts between software and
 * hardware image formats.
 * 
 * @param i Source image handle to copy
 * @param mode Optional target color mode (for conversion)
 * @param passed Bit flags indicating which parameters were provided
 * @return Negative image handle of the copy, or hardware image handle on conversion
 */
int32 func__copyimage(int32 i, int32 mode, int32 passed) {
    static int32 i2, bytes;
    static img_struct *s, *d;
    if (is_error_pending())
        return 0;
    // if (passed){
    if (i >= 0) { // validate i
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
    // }else{
    // i=write_page_index;
    // }

    s = &img[i];

    if (passed & 1) {
        if (mode != s->compatible_mode) {
            if (mode != 33 || s->compatible_mode != 32) {
                error(5);
                return -1;
            }
            // create new buffered hardware image
            i2 = new_hardware_img(s->width, s->height, (uint32 *)s->offset32, NEW_HARDWARE_IMG__BUFFER_CONTENT | NEW_HARDWARE_IMG__DUPLICATE_PROVIDED_BUFFER);
            return i2 + HARDWARE_IMG_HANDLE_OFFSET;
        }
    }

    // duplicate structure
    i2 = newimg();
    d = &img[i2];
    memcpy(d, s, sizeof(img_struct));
    // don't duplicate the memory lock (if any),
    //_MEMIMAGE needs to obtain a new lock for the copy
    img[i2].lock_id = NULL;
    img[i2].lock_offset = NULL;
    // duplicate pixel data
    bytes = d->width * d->height * d->bytes_per_pixel;
    d->offset = (uint8 *)malloc(bytes);
    if (!d->offset) {
        freeimg(i2);
        return -1;
    }
    memcpy(d->offset, s->offset, bytes);
    d->flags |= IMG_FREEMEM;
    // duplicate palette
    if (d->pal) {
        d->pal = (uint32 *)malloc(1024);
        if (!d->pal) {
            free(d->offset);
            freeimg(i2);
            return -1;
        }
        memcpy(d->pal, s->pal, 1024);
        d->flags |= IMG_FREEPAL;
    }
    // adjust flags
    if (d->flags & IMG_SCREEN)
        d->flags ^= IMG_SCREEN;
    // return new handle
    return -i2;
}

/**
 * Frees an image from memory.
 * 
 * Releases the memory allocated for an image. Cannot free screen pages.
 * For hardware images, queues a command to free the image on the graphics thread.
 * 
 * @param i Image handle to free (negative for regular images, positive for screen pages)
 * @param passed Bit flags indicating which parameters were provided
 */
void sub__freeimage(int32 i, int32 passed) {
    if (is_error_pending())
        return;
    if (passed) {
        if (i >= 0) { // validate i
            error(5);
            return; // The SCREEN's pages cannot be freed!
        } else {

            auto himg = get_hardware_img(i);
            if (himg) {
                flush_old_hardware_commands();
                // add command to free image
                // create new command handle & structure
                int32 hgch = list_add(hardware_graphics_command_handles);
                hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, hgch);
                hgc->remove = 0;
                // set command values
                hgc->command = HARDWARE_GRAPHICS_COMMAND__FREEIMAGE_REQUEST;
                hgc->src_img = get_hardware_img_index(i);
                himg->valid = 0;

                // queue the command
                hgc->next_command = 0;
                hgc->order = display_frame_order_next;
                if (last_hardware_command_added) {
                    hardware_graphics_command_struct *hgc2 =
                        (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_added);
                    hgc2->next_command = hgch;
                }
                last_hardware_command_added = hgch;
                if (first_hardware_command == 0)
                    first_hardware_command = hgch;

                return;
            }

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
        i = write_page_index;
    }
    if (img[i].flags & IMG_SCREEN) {
        error(5);
        return;
    } // The SCREEN's pages cannot be freed!
    if (write_page_index == i)
        sub__dest(-display_page_index);
    if (read_page_index == i)
        sub__source(-display_page_index);
    if (img[i].flags & IMG_FREEMEM)
        free(img[i].offset); // free pixel data (potential crash here)
    if (img[i].flags & IMG_FREEPAL)
        free(img[i].pal); // free palette
    freeimg(i);
}

void freeallimages() {
    static int32 i;
    // note: handles 0 & -1(1) are reserved
    for (i = 2; i < nextimg; i++) {
        if (img[i].valid && i != abs(console_image)) {
            if ((img[i].flags & IMG_SCREEN) == 0) { // The SCREEN's pages cannot be freed!
                sub__freeimage(-i, 1);
            }
        } // valid
    } // i
}

// Selecting images:

void sub__source(int32 i) {
    if (is_error_pending())
        return;
    if (i >= 0) { // validate i
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
    read_page_index = i;
    read_page = &img[i];
}

void sub__dest(int32 i) {
    if (is_error_pending())
        return;
    if (i >= 0) { // validate i
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
    write_page_index = i;
    write_page = &img[i];
}

int32 func__source() {
    return -read_page_index;
}

int32 func__dest() {
    return -write_page_index;
}

int32 func__display() {
    return -display_page_index;
}

// Changing the settings of an image surface:

void sub__blend(int32 i, int32 passed) {
    if (is_error_pending())
        return;
    if (passed) {
        if (i >= 0) { // validate i
            validatepage(i);
            i = page[i];
        } else {
            auto himg = get_hardware_img(i);
            if (himg) {
                himg->alpha_disabled = 0;
                return;
            }
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
        i = write_page_index;
    }
    if (img[i].bytes_per_pixel != 4) {
        error(5);
        return;
    }
    img[i].alpha_disabled = 0;
}

void sub__dontblend(int32 i, int32 passed) {
    if (is_error_pending())
        return;
    if (passed) {
        if (i >= 0) { // validate i
            validatepage(i);
            i = page[i];
        } else {
            auto himg = get_hardware_img(i);
            if (himg) {
                himg->alpha_disabled = 1;
                return;
            }
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
        i = write_page_index;
    }
    if (img[i].bytes_per_pixel != 4)
        return;
    img[i].alpha_disabled = 1;
}

// sub__clearcolor moved to color.cpp

// Changing/Using an image surface:

//_PUT "[(?,?)[-(?,?)]][,[?][,[?][,[(?,?)[-(?,?)]]]]]"
//(defined elsewhere)

//_IMGALPHA "?[,[?[{TO}?]][,?]]"
void sub__setalpha(int32 a, uint32 c, uint32 c2, int32 i, int32 passed) {
    //-->                             1        4        2
    static img_struct *im;
    static int32 z;
    static uint32 *lp, *last;
    static uint8 b_max, b_min, g_max, g_min, r_max, r_min, a_max, a_min;
    static uint8 *cp, *clast, v;
    if (is_error_pending())
        return;
    if (passed & 2) {
        if (i >= 0) { // validate i
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
        i = write_page_index;
    }
    im = &img[i];
    if (im->pal) {
        error(5);
        return;
    } // does not work on paletted images!
    if (a < 0 || a > 255) {
        error(5);
        return;
    } // invalid range
    if (passed & 4) {
        // ranged
        if (c == c2)
            goto uniquerange;
        b_min = c & 0xFF;
        g_min = c >> 8 & 0xFF;
        r_min = c >> 16 & 0xFF;
        a_min = c >> 24 & 0xFF;
        b_max = c2 & 0xFF;
        g_max = c2 >> 8 & 0xFF;
        r_max = c2 >> 16 & 0xFF;
        a_max = c2 >> 24 & 0xFF;
        if (b_min > b_max)
            std::swap(b_min, b_max);

        if (g_min > g_max)
            std::swap(g_min, g_max);
        if (r_min > r_max)
            std::swap(r_min, r_max);
        if (a_min > a_max)
            std::swap(a_min, a_max);
        cp = im->offset;
        z = im->width * im->height;
    setalpha:
        if (z--) {
            v = *cp;
            if (v <= b_max && v >= b_min) {
                v = *(cp + 1);
                if (v <= g_max && v >= g_min) {
                    v = *(cp + 2);
                    if (v <= r_max && v >= r_min) {
                        v = *(cp + 3);
                        if (v <= a_max && v >= a_min) {
                            *(cp + 3) = a;
                        }
                    }
                }
            }
            cp += 4;
            goto setalpha;
        }
        return;
    }
    if (passed & 1) {
    uniquerange:
        // alpha of c=a
        c2 = a << 24;
        lp = im->offset32 - 1;
        last = im->offset32 + im->width * im->height - 1;
        while (lp < last) {
            if (*++lp == c) {
                *lp = (*lp & 0xFFFFFF) | c2;
            }
        }
        return;
    }
    // all alpha=a
    cp = im->offset - 1;
    clast = im->offset + im->width * im->height * 4 - 4;
    while (cp < clast) {
        *(cp += 4) = a;
    }
    return;
}

// Finding information about an image surface:

int32 func__width(int32 i, int32 passed) {
    if (is_error_pending())
        return 0;

#ifdef QB64_WINDOWS
    if ((read_page->console && !passed) || i == console_image) {
        SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
        HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
        CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;
        GetConsoleScreenBufferInfo(cl_conout, &cl_bufinfo);
        return cl_bufinfo.srWindow.Right - cl_bufinfo.srWindow.Left + 1;
    }
#endif

    if (passed) {
        if (i >= 0) { // validate i
            validatepage(i);
            i = page[i];
        } else {
            auto himg = get_hardware_img(i);
            if (himg) {
                return himg->w;
            }
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
        i = write_page_index;
    }
    return img[i].width;
}

int32 func__height(int32 i, int32 passed) {
    if (is_error_pending())
        return 0;

#ifdef QB64_WINDOWS
    if ((read_page->console && !passed) || i == console_image) {
        SECURITY_ATTRIBUTES SecAttribs = {sizeof(SECURITY_ATTRIBUTES), 0, 1};
        HANDLE cl_conout = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, &SecAttribs, OPEN_EXISTING, 0, 0);
        CONSOLE_SCREEN_BUFFER_INFO cl_bufinfo;
        GetConsoleScreenBufferInfo(cl_conout, &cl_bufinfo);
        return cl_bufinfo.srWindow.Bottom - cl_bufinfo.srWindow.Top + 1;
        return cl_bufinfo.dwMaximumWindowSize.Y;
    }
#endif

    if (passed) {
        if (i >= 0) { // validate i
            validatepage(i);
            i = page[i];
        } else {
            auto himg = get_hardware_img(i);
            if (himg) {
                return himg->h;
            }
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
        i = write_page_index;
    }
    return img[i].height;
}

int32 func__pixelsize(int32 i, int32 passed) {
    if (is_error_pending())
        return 0;
    if (passed) {
        if (i >= 0) { // validate i
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
        i = write_page_index;
    }
    i = img[i].compatible_mode;
    if (i == 32)
        return 4;
    if (!i)
        return 0;
    return 1;
}

// func__clearcolor moved to color.cpp

int32 func__blend(int32 i, int32 passed) {
    if (is_error_pending())
        return 0;
    if (passed) {
        if (i >= 0) { // validate i
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
        i = write_page_index;
    }
    if (img[i].compatible_mode == 32) {
        if (!img[i].alpha_disabled)
            return -1;
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
// DRAW Command
// Extracted from libqb.cpp - implements the BASIC DRAW statement for vector graphics
//----------------------------------------------------------------------------------------------------------------------

// DRAW command state variables
static int32 sub_draw_i;
static uint8 *sub_draw_cp;
static int32 sub_draw_len;

static int32 draw_num_invalid;
static int32 draw_num_undefined;

static double draw_num() {
    static int32 c, dp, vptr, x, offset;
    static double d, dp_mult, sgn;

    draw_num_invalid = 0;
    draw_num_undefined = 1;
    d = 0;
    dp = 0;
    sgn = 1;
    vptr = 0;

nextchar:
    if (sub_draw_i >= sub_draw_len)
        return d * sgn;
    c = sub_draw_cp[sub_draw_i];

    if (vptr) {
        if ((sub_draw_i + 2) >= sub_draw_len) {
            draw_num_invalid = 1;
            return 0;
        } // not enough data!
        offset = sub_draw_cp[sub_draw_i + 2] * 256 + sub_draw_cp[sub_draw_i + 1];
        sub_draw_i += 3;
        vptr = 0;
        /*
            'BYTE=1
            'INTEGER=2
            'STRING=3 (unsupported)
            'SINGLE=4
            'INT64=5
            'FLOAT=6
            'DOUBLE=8
            'LONG=20
            'BIT=64+n (unsupported)
        */
        if (c == 1) {
            d = *((int8 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == (1 + 128)) {
            d = *((uint8 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == 2) {
            d = *((int16 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == (2 + 128)) {
            d = *((uint16 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == 4) {
            d = *((float *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == 5) {
            d = *((int64 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == (5 + 128)) {
            d = *((uint64 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == 6) {
            d = *((long double *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == 8) {
            d = *((double *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == 20) {
            d = *((int32 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        if (c == (20 + 128)) {
            d = *((uint32 *)(&cmem[1280 + offset]));
            goto nextcharv;
        }
        // unknown/unsupported types(bit/string) return an error
        draw_num_invalid = 1;
        return 0;
    nextcharv:
        draw_num_invalid = 0;
        draw_num_undefined = 0;
        return d;
    }

    if ((c == 32) || (c == 9)) {
        sub_draw_i++;
        goto nextchar;
    } // skip whitespace

    if ((c >= 48) && (c <= 57)) {
        c -= 48;
        if (dp) {
            d += (((double)c) * dp_mult);
            dp_mult /= 10.0;
        } else {
            d = (d * 10) + c;
        }
        draw_num_undefined = 0;
        draw_num_invalid = 0;
        sub_draw_i++;
        goto nextchar;
    }

    if (c == 45) { //-
        if (dp || (!draw_num_undefined))
            return d * sgn;
        sgn = -sgn;
        draw_num_invalid = 1;
        sub_draw_i++;
        goto nextchar;
    }

    if (c == 43) { //+
        if (dp || (!draw_num_undefined))
            return d * sgn;
        draw_num_invalid = 1;
        sub_draw_i++;
        goto nextchar;
    }

    if (c == 46) { //.
        if (dp)
            return d * sgn;
        dp = 1;
        dp_mult = 0.1;
        if (!draw_num_undefined)
            draw_num_invalid = 1;
        sub_draw_i++;
        goto nextchar;
    }

    if (c == 61) { //=
        if (draw_num_invalid || dp || (!draw_num_undefined)) {
            draw_num_invalid = 1;
            return 0;
        } // leading data invalid!
        vptr = 1;
        sub_draw_i++;
        goto nextchar;
    }

    return d * sgn;
}

void sub_draw(qbs *s) {
    if (is_error_pending())
        return;

    /*

        Aspect ratio determination:
        32/256 modes always assume 1:1 ratio
        All other modes (1-13) determine their aspect ratio from the destination surface's dimensions (presuming it is stretched onto a 4:3 ratio monitor)

        Reference:
        Line-drawing and cursor-movement commands:
        D[n%]            Moves cursor down n% units.
        E[n%]            Moves cursor up and right n% units.
        F[n%]            Moves cursor down and right n% units.
        G[n%]            Moves cursor down and left n% units.
        H[n%]            Moves cursor up and left n% units.
        L[n%]            Moves cursor left n% units.
        M[{+|-}]x%,y%    Moves cursor to point x%,y%. If x% is preceded
        by + or -, moves relative to the current point.
        -+/- relative ONLY if after the M, after comma doesn't affect method
        -nothing to do with VIEW/WINDOW coordinates (but still clipped)
        R[n%]            Moves cursor right n% units.
        U[n%]            Moves cursor up n% units.
        [B]              Optional prefix that moves cursor without drawing.
        [N]              Optional prefix that draws and returns cursor to
        its original position.
        *Prefixes B&N can be used anywhere. They set (not toggle) their respective states. They are only cleared if they are used in a statement. They are
       forgotten when a new DRAW statement is called. Color, rotation, and scale commands: An%              Rotates an object n% * 90 degrees (n% can be 0, 1,
        2, or 3).
        Cn%              Sets the drawing color (n% is a color attribute).

        Pn1%,n2%         Sets the paint fill and border colors of an object
        (n1% is the fill-color attribute, n2% is the
        border-color attribute).
        Sn%              Determines the drawing scale by setting the length
        of a unit of cursor movement. The default n% is 4,
        which is equivalent to 1 pixel.
        TAn%             Turns an angle n% degrees (-360 through 360).

        -If you omit n% from line-drawing and cursor-movement commands, the
        cursor moves 1 unit.
        -To execute a DRAW command substring from a DRAW command string, use
        the "X" command:
        DRAW "X"+ VARPTR$(commandstring$)
    */

    static double r, ir, vx, vy, hx, hy, ex, ey, fx, fy, xx, yy, px, py, px2, py2, d, d2, sin_ta, cos_ta;
    static int64 c64, c64b, c64c;
    static uint32 col;
    static int32 x, c, prefix_b, prefix_n, offset;
    static uint8 *stack_s[8192];
    static uint16 stack_len[8192];
    static uint16 stack_i[8192];
    static int32 stacksize;
    static double draw_ta;
    static double draw_scale;

    if (write_page->text) {
        error(5);
        return;
    }

    draw_ta = write_page->draw_ta;
    draw_scale = write_page->draw_scale;

    if (write_page->compatible_mode <= 13) {
        if (write_page->compatible_mode == 1)
            r = 4.0 / ((3.0 / 200.0) * 320.0);
        if (write_page->compatible_mode == 2)
            r = 4.0 / ((3.0 / 200.0) * 640.0);
        if (write_page->compatible_mode == 7)
            r = 4.0 / ((3.0 / 200.0) * 320.0);
        if (write_page->compatible_mode == 8)
            r = 4.0 / ((3.0 / 200.0) * 640.0);
        if (write_page->compatible_mode == 9)
            r = 4.0 / ((3.0 / 350.0) * 640.0);
        if (write_page->compatible_mode == 10)
            r = 4.0 / ((3.0 / 350.0) * 640.0);
        if (write_page->compatible_mode == 11)
            r = 4.0 / ((3.0 / 480.0) * 640.0);
        if (write_page->compatible_mode == 12)
            r = 4.0 / ((3.0 / 480.0) * 640.0);
        if (write_page->compatible_mode == 13)
            r = 4.0 / ((3.0 / 200.0) * 320.0);
        // Old method: r=4.0 /( (3.0/((double)write_page->height)) * ((double)write_page->width) ); //calculate aspect ratio of image
        ir = 1 / r; // note: all drawing must multiply the x offset by ir (inverse ratio)
    } else {
        r = 1;
        ir = 1;
    }

    vx = 0;
    vy = -1;
    ex = r;
    ey = -1;
    hx = r;
    hy = 0;
    fx = r;
    fy = 1; // reset vectors
    // rotate vectors by ta?
    if (draw_ta) {
        d = draw_ta * 0.0174532925199433;
        sin_ta = std::sin(d);
        cos_ta = std::cos(d);
        px2 = vx;
        py2 = vy;
        vx = px2 * cos_ta + py2 * sin_ta;
        vy = py2 * cos_ta - px2 * sin_ta;
        px2 = hx;
        py2 = hy;
        hx = px2 * cos_ta + py2 * sin_ta;
        hy = py2 * cos_ta - px2 * sin_ta;
        px2 = ex;
        py2 = ey;
        ex = px2 * cos_ta + py2 * sin_ta;
        ey = py2 * cos_ta - px2 * sin_ta;
        px2 = fx;
        py2 = fy;
        fx = px2 * cos_ta + py2 * sin_ta;
        fy = py2 * cos_ta - px2 * sin_ta;
    }

    // convert x,y image position into a pixel coordinate
    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            px = write_page->x * write_page->scaling_x + write_page->scaling_offset_x + write_page->view_offset_x;
            py = write_page->y * write_page->scaling_y + write_page->scaling_offset_y + write_page->view_offset_y;
        } else {
            px = write_page->x + write_page->view_offset_x;
            py = write_page->y + write_page->view_offset_y;
        }
    } else {
        px = write_page->x;
        py = write_page->y;
    }

    col = write_page->draw_color;
    prefix_b = 0;
    prefix_n = 0;

    stacksize = 0;

    sub_draw_cp = s->chr;
    sub_draw_len = s->len;
    sub_draw_i = 0;

nextchar:
    if (sub_draw_i >= sub_draw_len) {

        // revert from X-stack
        if (stacksize) {
            stacksize--;
            sub_draw_cp = stack_s[stacksize];
            sub_draw_len = stack_len[stacksize];
            sub_draw_i = stack_i[stacksize]; // restore state
            // continue
            goto nextchar;
        }

        // revert px,py to image->x,y offsets
        if (write_page->clipping_or_scaling) {
            if (write_page->clipping_or_scaling == 2) {
                px = (px - write_page->view_offset_x - write_page->scaling_offset_x) / write_page->scaling_x;
                py = (py - write_page->view_offset_y - write_page->scaling_offset_y) / write_page->scaling_y;
            } else {
                px = px - write_page->view_offset_x;
                py = py - write_page->view_offset_y;
            }
        }
        write_page->x = px;
        write_page->y = py;
        return;
    }
    c = sub_draw_cp[sub_draw_i];

    if ((c >= 97) && (c <= 122))
        c -= 32; // ucase c

    if (c == 77) { // M
    m_nextchar:
        sub_draw_i++;
        if (sub_draw_i >= sub_draw_len) {
            error(5);
            return;
        }
        c = sub_draw_cp[sub_draw_i];
        if ((c == 32) || (c == 9))
            goto m_nextchar; // skip whitespace
        // check for absolute/relative positioning
        if ((c == 43) || (c == 45))
            x = 1;
        else
            x = 0;
        px2 = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        c = sub_draw_cp[sub_draw_i];
        if (c != 44) {
            error(5);
            return;
        } // expected ,
        sub_draw_i++;
        py2 = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        if (x) { // relative positioning
            xx = (px2 * ir) * hx - (py2 * ir) * vx;
            yy = px2 * hy - py2 * vy;
            px2 = px + xx * draw_scale;
            py2 = py + yy * draw_scale;
        }
        if (!prefix_b)
            fast_line(qbr(px), qbr(py), qbr(px2), qbr(py2), col);
        if (!prefix_n) {
            px = px2;
            py = py2;
        } // update position
        prefix_b = 0;
        prefix_n = 0;
        goto nextchar;
    }

    if (c == 84) { // T(A)
    ta_nextchar:
        sub_draw_i++;
        if (sub_draw_i >= sub_draw_len) {
            error(5);
            return;
        }
        c = sub_draw_cp[sub_draw_i];
        if ((c == 32) || (c == 9))
            goto ta_nextchar; // skip whitespace
        if ((c != 65) && (c != 97)) {
            error(5);
            return;
        } // not TA
        sub_draw_i++;
        d = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        draw_ta = d;
        write_page->draw_ta = draw_ta;
    ta_entry:
        // note: ta rotation is not relative to previous angle
        vx = 0;
        vy = -1;
        ex = r;
        ey = -1;
        hx = r;
        hy = 0;
        fx = r;
        fy = 1; // reset vectors
        // rotate vectors by ta
        d = draw_ta * 0.0174532925199433;
        sin_ta = std::sin(d);
        cos_ta = std::cos(d);
        px2 = vx;
        py2 = vy;
        vx = px2 * cos_ta + py2 * sin_ta;
        vy = py2 * cos_ta - px2 * sin_ta;
        px2 = hx;
        py2 = hy;
        hx = px2 * cos_ta + py2 * sin_ta;
        hy = py2 * cos_ta - px2 * sin_ta;
        px2 = ex;
        py2 = ey;
        ex = px2 * cos_ta + py2 * sin_ta;
        ey = py2 * cos_ta - px2 * sin_ta;
        px2 = fx;
        py2 = fy;
        fx = px2 * cos_ta + py2 * sin_ta;
        fy = py2 * cos_ta - px2 * sin_ta;
        goto nextchar;
    }

    if (c == 85) {
        xx = vx;
        yy = vy;
        goto udlr;
    } // U
    if (c == 68) {
        xx = -vx;
        yy = -vy;
        goto udlr;
    } // D
    if (c == 76) {
        xx = -hx;
        yy = -hy;
        goto udlr;
    } // L
    if (c == 82) {
        xx = hx;
        yy = hy;
        goto udlr;
    } // R

    if (c == 69) {
        xx = ex;
        yy = ey;
        goto udlr;
    } // E
    if (c == 70) {
        xx = fx;
        yy = fy;
        goto udlr;
    } // F
    if (c == 71) {
        xx = -ex;
        yy = -ey;
        goto udlr;
    } // G
    if (c == 72) {
        xx = -fx;
        yy = -fy;
        goto udlr;
    } // H

    if (c == 67) { // C
        sub_draw_i++;
        d = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        c64 = d;
        xx = c64;
        if (xx != d) {
            error(5);
            return;
        } // non-integer
        // if (c64<0){error(5); return;}
        // c64b=1; c64b<<=write_page->bits_per_pixel; c64b--;
        // if (c64>c64b){error(5); return;}
        col = c64;
        write_page->draw_color = col;
        goto nextchar;
    }

    if (c == 66) { // B (move without drawing prefix)
        prefix_b = 1;
        sub_draw_i++;
        goto nextchar;
    }

    if (c == 78) { // N (draw without moving)
        prefix_n = 1;
        sub_draw_i++;
        goto nextchar;
    }

    if (c == 83) { // S
        sub_draw_i++;
        d = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        if (d < 0) {
            error(5);
            return;
        }
        draw_scale = d / 4.0;
        write_page->draw_scale = draw_scale;
        goto nextchar;
    }

    if (c == 80) { // P
        sub_draw_i++;
        d = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        c64 = d;
        xx = c64;
        if (xx != d) {
            error(5);
            return;
        } // non-integer
        // if (c64<0){error(5); return;}
        // c64b=1; c64b<<=write_page->bits_per_pixel; c64b--;
        // if (c64>c64b){error(5); return;}
        c64c = c64;
        c = sub_draw_cp[sub_draw_i];
        if (c != 44) {
            error(5);
            return;
        } // expected ,
        sub_draw_i++;
        d = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        c64 = d;
        xx = c64;
        if (xx != d) {
            error(5);
            return;
        } // non-integer
        // if (c64<0){error(5); return;}
        // c64b=1; c64b<<=write_page->bits_per_pixel; c64b--;
        // if (c64>c64b){error(5); return;}
        // revert px,py to x,y offsets
        if (write_page->clipping_or_scaling) {
            if (write_page->clipping_or_scaling == 2) {
                xx = (px - write_page->view_offset_x - write_page->scaling_offset_x) / write_page->scaling_x;
                yy = (py - write_page->view_offset_y - write_page->scaling_offset_y) / write_page->scaling_y;
            } else {
                xx = px - write_page->view_offset_x;
                yy = py - write_page->view_offset_y;
            }
        } else {
            xx = px;
            yy = py;
        }
        sub_paint(xx, yy, c64c, c64, NULL, 2 + 4);
        col = c64c;
        goto nextchar;
    }

    if (c == 65) { // A
        sub_draw_i++;
        d = draw_num();
        if (draw_num_invalid || draw_num_undefined) {
            error(5);
            return;
        }
        if (d == 0) {
            draw_ta = 0;
            write_page->draw_ta = draw_ta;
            goto ta_entry;
        }
        if (d == 1) {
            draw_ta = 90;
            write_page->draw_ta = draw_ta;
            goto ta_entry;
        }
        if (d == 2) {
            draw_ta = 180;
            write_page->draw_ta = draw_ta;
            goto ta_entry;
        }
        if (d == 3) {
            draw_ta = 270;
            write_page->draw_ta = draw_ta;
            goto ta_entry;
        }
        error(5);
        return; // invalid value
    }

    if (c == 88) { // X
        sub_draw_i++;
        if ((sub_draw_i + 2) >= sub_draw_len) {
            error(5);
            return;
        }
        if (sub_draw_cp[sub_draw_i] != 3) {
            error(5);
            return;
        }
        offset = sub_draw_cp[sub_draw_i + 2] * 256 + sub_draw_cp[sub_draw_i + 1]; // offset of string descriptor in DBLOCK
        sub_draw_i += 3;
        if (stacksize == 8192) {
            error(6);
            return;
        } // X-stack "OVERFLOW" (should never occur because DBLOCK will overflow first)
        stack_s[stacksize] = sub_draw_cp;
        stack_len[stacksize] = sub_draw_len;
        stack_i[stacksize] = sub_draw_i;
        stacksize++; // backup state
        // set new state
        sub_draw_i = 0;
        x = cmem[1280 + offset + 3] * 256 + cmem[1280 + offset + 2];
        sub_draw_cp = &cmem[1280] + x;
        sub_draw_len = cmem[1280 + offset + 1] * 256 + cmem[1280 + offset + 0];
        // continue processing
        goto nextchar;
    }

    if ((c == 32) || (c == 9) || (c == 59)) {
        sub_draw_i++;
        goto nextchar;
    } // skip whitespace/semicolons

    error(5);
    return; // unknown command encountered!

udlr:
    sub_draw_i++;
    d = draw_num();
    if (draw_num_invalid) {
        error(5);
        return;
    }
    if (draw_num_undefined)
        d = 1;
    xx *= d;
    yy *= d;
    //***apply scaling here***
    xx = xx * ir;
    px2 = px + xx * draw_scale;
    py2 = py + yy * draw_scale;
    if (!prefix_b)
        fast_line(qbr(px), qbr(py), qbr(px2), qbr(py2), col);
    if (!prefix_n) {
        px = px2;
        py = py2;
    } // update position
    prefix_b = 0;
    prefix_n = 0;
    goto nextchar;
}

void sub__putimage(double f_dx1, double f_dy1, double f_dx2, double f_dy2, int32 src, int32 dst, double f_sx1, double f_sy1, double f_sx2, double f_sy2,
                   int32 passed) {

    /*
        Format & passed bits: (needs updating)
        [[{STEP}](?,?)[-[{STEP}](?,?)]][,[?][,[?][,[[{STEP}](?,?)[-[{STEP}](?,?)]][,{_SMOOTH}]]]]
        2?     1              4?       8                                 512      128
    */

    static int32 w, h, sskip, dskip, x, y, xx, yy, z, x2, y2, dbpp, sbpp;
    static img_struct *s, *d;
    static uint32 *soff32, *doff32, col, clearcol, destcol;
    static uint8 *soff, *doff;
    static uint8 *cp;
    static int32 xdir, ydir, no_stretch, no_clip, no_reverse, flip, mirror;
    static double mx, my, fx, fy, fsx1, fsy1, fsx2, fsy2, dv, dv2;
    static int32 sx1, sy1, sx2, sy2, dx1, dy1, dx2, dy2;
    static int32 sw, sh, dw, dh;
    static uint32 *pal;
    static uint32 *ulp;

    no_stretch = 0;
    no_clip = 0;
    no_reverse = 1;
    flip = 0;
    mirror = 0;

    static int32 use_hardware;
    static img_struct s_emu, d_emu; // used to emulate a source/dest image for calculation purposes
    use_hardware = 0;

    // is source a hardware handle?
    hardware_img_struct *dst_himg = NULL;
    hardware_img_struct *src_himg = NULL;
    if (src) {
        src_himg = (hardware_img_struct *)list_get(hardware_img_handles, src - HARDWARE_IMG_HANDLE_OFFSET);
        if (src_himg != NULL) { // source is hardware image
            src -= HARDWARE_IMG_HANDLE_OFFSET;

            flush_old_hardware_commands();

            s_emu.width = src_himg->w;
            s_emu.height = src_himg->h;
            s_emu.clipping_or_scaling = 0;
            s_emu.alpha_disabled = src_himg->alpha_disabled;
            s = &s_emu;

            // check dst
            if (dst < 0) {
                dst_himg = (hardware_img_struct *)list_get(hardware_img_handles, dst - HARDWARE_IMG_HANDLE_OFFSET);
                if (dst_himg == NULL) {
                    error(258);
                    return;
                }
                dst -= HARDWARE_IMG_HANDLE_OFFSET;

                d_emu.width = dst_himg->w;
                d_emu.height = dst_himg->h;
                d_emu.clipping_or_scaling = 0;
                d_emu.alpha_disabled = dst_himg->alpha_disabled;
                d = &d_emu;

            } else {
                if (dst > 1) {
                    error(5);
                    return;
                }
                dst = -dst;

                d = display_page; // use parameters from display page
            }

            sbpp = 4;
            dbpp = 4;
            use_hardware = 1;

            goto resolve_coordinates;

        } // source is hardware image
    } // src passed

    if (passed & 8) { // src
        // validate
        if (src >= 0) {
            validatepage(src);
            s = &img[page[src]];
        } else {
            src = -src;
            if (src >= nextimg) {
                error(258);
                return;
            }
            s = &img[src];
            if (!s->valid) {
                error(258);
                return;
            }
        }
    } else {
        s = read_page;
    } // src
    if (s->text) {
        error(5);
        return;
    }
    sbpp = s->bytes_per_pixel;

    if (passed & 32) { // dst
        // validate
        if (dst >= 0) {
            validatepage(dst);
            d = &img[page[dst]];
        } else {
            dst = -dst;
            if (dst >= nextimg) {
                error(258);
                return;
            }
            d = &img[dst];
            if (!d->valid) {
                error(258);
                return;
            }
        }
    } else {
        d = write_page;
    } // dst
    if (d->text) {
        error(5);
        return;
    }
    dbpp = d->bytes_per_pixel;
    if ((sbpp == 4) && (dbpp == 1)) {
        error(5);
        return;
    }
    if (s == d) {
        //{error(5); return;}//cannot put source onto itself!
        int32 temphandle = func__copyimage(dst, NULL, 0);
        passed = passed | 8; // make certain we set the flag TO LET QB64 know we're passing a handle to the temp image
        sub__putimage(f_dx1, f_dy1, f_dx2, f_dy2, temphandle, dst, f_sx1, f_sy1, f_sx2, f_sy2, passed);
        sub__freeimage(temphandle, 1);
        return;
    }

resolve_coordinates:

    // quick references
    sw = s->width;
    sh = s->height;
    dw = d->width;
    dh = d->height;

    // change coordinates according to step
    if (passed & 2) {
        f_dx1 = d->x + f_dx1;
        f_dy1 = d->y + f_dy1;
    }
    if (passed & 16) {
        f_dx2 = f_dx1 + f_dx2;
        f_dy2 = f_dy1 + f_dy2;
    }
    if (passed & 256) {
        f_sx1 = s->x + f_sx1;
        f_sy1 = s->y + f_sy1;
    }
    if (passed & 1024) {
        f_sx2 = f_sx1 + f_sx2;
        f_sy2 = f_sy1 + f_sy2;
    }

    // Here we calculate what our final point is going to be and put that value into the _DEST x/y so we can get STEP back correctly on the next call.
    // or something like that...  I have no idea how to explain what the heck I'm gdoing here!
    // Basically I'm just trying to update the x/y point that we last plot to on our screen so we can pick it back up and use it again...
    if (passed & 4) {
        // we entered both dest numbers.  Our last point plotted should be f_dx2/f_dy2
        d->x = f_dx2;
        d->y = f_dy2;
    } else {
        if (passed & 1) {
            // we only sent it the first dest value.  We want to put our rectangle on a portion of the screen starting at this point
            if (passed & 512) {
                // we have all the source values.  We want to put that rectangle over to dest starting at that point
                d->x = f_dx1 + abs(f_sx2 - f_sx1);
                d->y = f_dy1 + abs(f_sy2 - f_sy1);
            } else {
                // we want to go from f_sx1,F_sx2 to the edge of the screen and put it over to dest starting at that point
                d->x = f_dx1 + abs(sw - f_sx1);
                d->y = f_dy1 + abs(sh - f_sy1);
            }
        } else {
            // we never sent the first source value.  We want to put the image over the whole screen.
            d->x = dw;
            d->y = dh;
        }
    }

    // And here we update our source page information so the STEP will work properly there as well.
    // This seems a little simpler logic

    if (passed & 512) {
        // we sent it the stop coordinate of where we're reading from
        s->x = f_sx2;
        s->y = f_sy2;
    } else {
        // we didn't and we need to have it copy from wherever the starting point is to the bottom right of the screen.
        // so our final point read will be the source width/height
        s->x = sw;
        s->y = sh;
    }

    // resolve coordinates
    if (passed & 1) { // dx1,dy1
        if (d->clipping_or_scaling) {
            if (d->clipping_or_scaling == 2) {
                dx1 = qbr_float_to_long(f_dx1 * d->scaling_x + d->scaling_offset_x) + d->view_offset_x;
                dy1 = qbr_float_to_long(f_dy1 * d->scaling_y + d->scaling_offset_y) + d->view_offset_y;
            } else {
                dx1 = qbr_float_to_long(f_dx1) + d->view_offset_x;
                dy1 = qbr_float_to_long(f_dy1) + d->view_offset_y;
            }
        } else {
            dx1 = qbr_float_to_long(f_dx1);
            dy1 = qbr_float_to_long(f_dy1);
        }
        // note: dx2 & dy2 cannot be passed if dx1 & dy1 weren't passed
        if (passed & 4) { // dx2,dy2
            if (d->clipping_or_scaling) {
                if (d->clipping_or_scaling == 2) {
                    dx2 = qbr_float_to_long(f_dx2 * d->scaling_x + d->scaling_offset_x) + d->view_offset_x;
                    dy2 = qbr_float_to_long(f_dy2 * d->scaling_y + d->scaling_offset_y) + d->view_offset_y;
                } else {
                    dx2 = qbr_float_to_long(f_dx2) + d->view_offset_x;
                    dy2 = qbr_float_to_long(f_dy2) + d->view_offset_y;
                }
            } else {
                dx2 = qbr_float_to_long(f_dx2);
                dy2 = qbr_float_to_long(f_dy2);
            }
        } else { // dx2,dy2
            dx2 = 0;
            dy2 = 0;
        } // dx2,dy2
    } else { // dx1,dy1
        dx1 = 0;
        dy1 = 0;
        dx2 = 0;
        dy2 = 0;
    } // dx1,dy1

    if (passed & 64) { // sx1,sy1
        if (s->clipping_or_scaling) {

            if (s->clipping_or_scaling == 2) {
                sx1 = qbr_float_to_long(f_sx1 * s->scaling_x + s->scaling_offset_x) + s->view_offset_x;
                sy1 = qbr_float_to_long(f_sy1 * s->scaling_y + s->scaling_offset_y) + s->view_offset_y;
            } else {
                sx1 = qbr_float_to_long(f_sx1) + s->view_offset_x;
                sy1 = qbr_float_to_long(f_sy1) + s->view_offset_y;
            }
        } else {
            sx1 = qbr_float_to_long(f_sx1);
            sy1 = qbr_float_to_long(f_sy1);
        }
        // note: sx2 & sy2 cannot be passed if sx1 & sy1 weren't passed
        if (passed & 512) { // sx2,sy2
            if (s->clipping_or_scaling) {
                if (s->clipping_or_scaling == 2) {
                    sx2 = qbr_float_to_long(f_sx2 * s->scaling_x + s->scaling_offset_x) + s->view_offset_x;
                    sy2 = qbr_float_to_long(f_sy2 * s->scaling_y + s->scaling_offset_y) + s->view_offset_y;
                } else {
                    sx2 = qbr_float_to_long(f_sx2) + s->view_offset_x;
                    sy2 = qbr_float_to_long(f_sy2) + s->view_offset_y;
                }
            } else {
                sx2 = qbr_float_to_long(f_sx2);
                sy2 = qbr_float_to_long(f_sy2);
            }
        } else { // sx2,sy2
            sx2 = 0;
            sy2 = 0;
        } // sx2,sy2
    } else { // sx1,sy1
        sx1 = 0;
        sy1 = 0;
        sx2 = 0;
        sy2 = 0;
    } // sx1,sy1

    // all co-ordinates resolved (but omitted co-ordinates are set to 0!)

    if (use_hardware) {
        // calculate omitted co-ordinates
        if ((passed & 4) && (passed & 512))
            goto got_hw_coord; // all passed
        if (passed & 4) {      //(dx1,dy1)-(dx2,dy2)...
            if (passed & 64) { //(dx1,dy1)-(dx2,dy2),...,(sx1,sy1)
                sx2 = sx1 + abs(dx2 - dx1);
                sy2 = sy1 + abs(dy2 - dy1);
                goto got_hw_coord;
            } else { //(dx1,dy1)-(dx2,dy2)
                sx2 = sw - 1;
                sy2 = sh - 1;
                goto got_hw_coord;
            }
        }
        if (passed & 512) {   //...(sx1,sy1)-(sx2,sy2)
            if (passed & 1) { //(dx1,dy1),,(sx1,sy1)-(sx2,sy2)
                dx2 = dx1 + abs(sx2 - sx1);
                dy2 = dy1 + abs(sy2 - sy1);
                goto got_hw_coord;
            } else { //(sx1,sy1)-(sx2,sy2)
                dx2 = dw - 1;
                dy2 = dh - 1;
                goto got_hw_coord;
            }
        }
        if (passed & 64) {
            error(5);
            return;
        } // Invalid: NULL-NULL,?,?,(sx1,sy1)-NULL
        if (passed & 1) { //(dx1,dy1)
            sx2 = s->width - 1;
            sy2 = s->height - 1;
            dx2 = dx1 + sx2;
            dy2 = dy1 + sy2;
            goto got_hw_coord;
        }
        // no coords provided
        sx2 = s->width - 1;
        sy2 = s->height - 1;
        dx2 = d->width - 1;
        dy2 = d->height - 1;
    got_hw_coord:

        // create new command handle & structure
        int32 hgch = list_add(hardware_graphics_command_handles);
        hardware_graphics_command_struct *hgc = (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, hgch);

        hgc->remove = 0;

        // set command values
        hgc->command = HARDWARE_GRAPHICS_COMMAND__PUTIMAGE;

        hgc->src_img = src;
        hgc->src_x1 = sx1;
        hgc->src_y1 = sy1;
        hgc->src_x2 = sx2;
        hgc->src_y2 = sy2;

        hgc->dst_img = dst;
        hgc->dst_x1 = dx1;
        hgc->dst_y1 = dy1;
        hgc->dst_x2 = dx2;
        hgc->dst_y2 = dy2;

        hgc->smooth = 0; // unless specified, no filtering will be applied
        if (passed & 128)
            hgc->smooth = 1;

        hgc->use_alpha = 1;
        if (s->alpha_disabled)
            hgc->use_alpha = 0;
        // only consider dest alpha setting if it is a hardware image
        if (dst_himg) {
            if (d->alpha_disabled)
                hgc->use_alpha = 0;
        }

        // queue the command
        hgc->next_command = 0;
        hgc->order = display_frame_order_next;

        if (last_hardware_command_added) {
            hardware_graphics_command_struct *hgc2 =
                (hardware_graphics_command_struct *)list_get(hardware_graphics_command_handles, last_hardware_command_added);
            hgc2->next_command = hgch;
        }
        last_hardware_command_added = hgch;
        if (first_hardware_command == 0)
            first_hardware_command = hgch;

        return;

    } // use hardware

    //(decided not to throw error, QB64 will use linear filtering if/when available)
    // if (passed&128){error(5); return;}//software surfaces do not support pixel _SMOOTHing yet

    if ((passed & 4) && (passed & 512)) { // all co-ords given
        // could be stretched
        if ((abs(dx2 - dx1) == abs(sx2 - sx1)) && (abs(dy2 - dy1) == abs(sy2 - sy1))) { // non-stretched
            // could be flipped/reversed
            // could need clipping
            goto reverse;
        }
        goto stretch;
    }

    if (passed & 4) {      //(dx1,dy1)-(dx2,dy2)...
        if (passed & 64) { //(dx1,dy1)-(dx2,dy2),...,(sx1,sy1)
            sx2 = sx1 + abs(dx2 - dx1);
            sy2 = sy1 + abs(dy2 - dy1);
            // can't be stretched
            // could be flipped/reversed
            // could need clipping
            goto reverse;
        } else { //(dx1,dy1)-(dx2,dy2)
            sx2 = sw - 1;
            sy2 = sh - 1;
            // could be stretched
            if (((abs(dx2 - dx1) + 1) == sw) && ((abs(dy2 - dy1) + 1) == sh)) { // non-stretched
                // could be flipped/reversed
                // could need clipping
                goto reverse;
            }
            goto stretch;
        } // 16
    } // 2

    if (passed & 512) {   //...(sx1,sy1)-(sx2,sy2)
        if (passed & 1) { //(dx1,dy1),,(sx1,sy1)-(sx2,sy2)
            dx2 = dx1 + abs(sx2 - sx1);
            dy2 = dy1 + abs(sy2 - sy1);
            // can't be stretched
            // could be flipped/reversed
            // could need clipping
            goto reverse;
        } else { //(sx1,sy1)-(sx2,sy2)
            dx2 = dw - 1;
            dy2 = dh - 1;
            // could be stretched
            if (((abs(sx2 - sx1) + 1) == dw) && ((abs(sy2 - sy1) + 1) == dh)) { // non-stretched
                // could be flipped/reversed
                // could need clipping
                goto reverse;
            }
            goto stretch;
        } // 1
    } // 32

    if (passed & 64) {
        error(5);
        return;
    } // Invalid: NULL-NULL,?,?,(sx1,sy1)-NULL

    if (passed & 1) { //(dx1,dy1)
        sx2 = s->width - 1;
        sy2 = s->height - 1;
        dx2 = dx1 + sx2;
        dy2 = dy1 + sy2;
        goto clip;
    }

    // no co-ords given
    sx2 = s->width - 1;
    sy2 = s->height - 1;
    dx2 = d->width - 1;
    dy2 = d->height - 1;
    if ((sx2 == dx2) && (sy2 == dy2)) { // non-stretched
        // note: because 0-size image is illegal, no null size check is necessary
        goto noflip; // cannot be reversed
    }
    // precalculate required values
    w = dx2 - dx1;
    h = dy2 - dy1;
    fsx1 = sx1;
    fsy1 = sy1;
    fsx2 = sx2;
    fsy2 = sy2;
    //"pull" corners so all source pixels are evenly represented in dest rect
    if (fsx1 <= fsx2) {
        fsx1 -= 0.499999;
        fsx2 += 0.499999;
    } else {
        fsx1 += 0.499999;
        fsx2 -= 0.499999;
    }
    if (fsy1 <= fsy2) {
        fsy1 -= 0.499999;
        fsy2 += 0.499999;
    } else {
        fsy1 += 0.499999;
        fsy2 -= 0.499999;
    }
    // calc source gradients
    if (w)
        mx = (fsx2 - fsx1) / ((double)w);
    else
        mx = 0.0;
    if (h)
        my = (fsy2 - fsy1) / ((double)h);
    else
        my = 0.0;
    // note: mx & my represent the amount of change per dest pixel
    goto stretch_noreverse_noclip;

stretch:
    // stretch is required

    // mirror?
    if (dx2 < dx1) {
        if (sx2 > sx1)
            mirror = 1;
    }
    if (sx2 < sx1) {
        if (dx2 > dx1)
            mirror = 1;
    }
    if (dx2 < dx1) {
        x = dx1;
        dx1 = dx2;
        dx2 = x;
    }
    if (sx2 < sx1) {
        x = sx1;
        sx1 = sx2;
        sx2 = x;
    }
    // flip?
    if (dy2 < dy1) {
        if (sy2 > sy1)
            flip = 1;
    }
    if (sy2 < sy1) {
        if (dy2 > dy1)
            flip = 1;
    }
    if (dy2 < dy1) {
        y = dy1;
        dy1 = dy2;
        dy2 = y;
    }
    if (sy2 < sy1) {
        y = sy1;
        sy1 = sy2;
        sy2 = y;
    }

    w = dx2 - dx1;
    h = dy2 - dy1;
    fsx1 = sx1;
    fsy1 = sy1;
    fsx2 = sx2;
    fsy2 = sy2;
    //"pull" corners so all source pixels are evenly represented in dest rect
    if (fsx1 <= fsx2) {
        fsx1 -= 0.499999;
        fsx2 += 0.499999;
    } else {
        fsx1 += 0.499999;
        fsx2 -= 0.499999;
    }
    if (fsy1 <= fsy2) {
        fsy1 -= 0.499999;
        fsy2 += 0.499999;
    } else {
        fsy1 += 0.499999;
        fsy2 -= 0.499999;
    }
    // calc source gradients
    if (w)
        mx = (fsx2 - fsx1) / ((double)w);
    else
        mx = 0.0;
    if (h)
        my = (fsy2 - fsy1) / ((double)h);
    else
        my = 0.0;
    // note: mx & my represent the amount of change per dest pixel

    // crop dest offscreen pixels
    if (dx1 < 0) {
        if (mirror)
            fsx2 += ((double)dx1) * mx;
        else
            fsx1 -= ((double)dx1) * mx;
        dx1 = 0;
    }
    if (dy1 < 0) {
        if (flip)
            fsy2 += ((double)dy1) * my;
        else
            fsy1 -= ((double)dy1) * my;
        dy1 = 0;
    }
    if (dx2 >= dw) {
        if (mirror)
            fsx1 += ((double)(dx2 - dw + 1)) * mx;
        else
            fsx2 -= ((double)(dx2 - dw + 1)) * mx;
        dx2 = dw - 1;
    }
    if (dy2 >= dh) {
        if (flip)
            fsy1 += ((double)(dy2 - dh + 1)) * my;
        else
            fsy2 -= ((double)(dy2 - dh + 1)) * my;
        dy2 = dh - 1;
    }
    // crop source offscreen pixels
    if (w) { // gradient cannot be 0
        if (fsx1 < -0.4999999) {
            x = (-fsx1 - 0.499999) / mx + 1.0;
            if (mirror)
                dx2 -= x;
            else
                dx1 += x;
            fsx1 += ((double)x) * mx;
        }
        if (fsx2 > (((double)sw) - 0.5000001)) {
            x = (fsx2 - (((double)sw) - 0.500001)) / mx + 1.0;
            if (mirror)
                dx1 += x;
            else
                dx2 -= x;
            fsx2 -= (((double)x) * mx);
        }
    } // w
    if (h) { // gradient cannot be 0
        if (fsy1 < -0.4999999) {
            y = (-fsy1 - 0.499999) / my + 1.0;
            if (flip)
                dy2 -= y;
            else
                dy1 += y;
            fsy1 += ((double)y) * my;
        }
        if (fsy2 > (((double)sh) - 0.5000001)) {
            y = (fsy2 - (((double)sh) - 0.500001)) / my + 1.0;
            if (flip)
                dy1 += y;
            else
                dy2 -= y;
            fsy2 -= (((double)y) * my);
        }
    } // h
    //<0-size/offscreen?
    // note: <0-size will cause reversal of dest
    //      offscreen values will result in reversal of dest
    if (dx1 > dx2)
        return;
    if (dy1 > dy2)
        return;
    // all values are now within the boundaries of the source & dest

stretch_noreverse_noclip:
    w = dx2 - dx1 + 1;
    h = dy2 - dy1 + 1; // recalculate based on actual number of pixels

    if (sbpp == 4) {
        if (s->alpha_disabled || d->alpha_disabled)
            goto put_32_noalpha_stretch;
        goto put_32_stretch;
    }
    if (dbpp == 1) {
        if (s->transparent_color == -1)
            goto put_8_stretch;
        goto put_8_clear_stretch;
    }
    if (s->transparent_color == -1)
        goto put_8_32_stretch;
    goto put_8_32_clear_stretch;

put_32_stretch:
    // calc. starting points & change values
    if (flip) {
        if (mirror) {
            doff32 = d->offset32 + (dy2 * dw + dx2);
            dskip = -dw + w;
        } else {
            doff32 = d->offset32 + (dy2 * dw + dx1);
            dskip = -dw - w;
        }
    } else {
        if (mirror) {
            doff32 = d->offset32 + (dy1 * dw + dx2);
            dskip = dw + w;
        } else {
            doff32 = d->offset32 + (dy1 * dw + dx1);
            dskip = dw - w;
        }
    }
    if (mirror)
        xdir = -1;
    else
        xdir = 1;
    // plot rect
    yy = h;
    fy = fsy1;
    fsx1 -= mx; // prev value is moved on from
    do {
        xx = w;
        ulp = s->offset32 + sw * qbr_double_to_long(fy);
        fx = fsx1;
        do {
            //--------plot pixel--------
            switch ((col = *(ulp + qbr_double_to_long(fx += mx))) & 0xFF000000) {
            case 0xFF000000:
                *doff32 = col;
                break;
            case 0x0:
                break;
            case 0x80000000:
                *doff32 = (((*doff32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                break;
            case 0x7F000000:
                *doff32 = (((*doff32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                break;
            default:
                destcol = *doff32;
                cp = cblend + (col >> 24 << 16);
                *doff32 = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                          (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
            }; // switch
            //--------done plot pixel--------
            doff32 += xdir;
        } while (--xx);
        doff32 += dskip;
        fy += my;
    } while (--yy);
    return;

put_32_noalpha_stretch:
    // calc. starting points & change values
    if (flip) {
        if (mirror) {
            doff32 = d->offset32 + (dy2 * dw + dx2);
            dskip = -dw + w;
        } else {
            doff32 = d->offset32 + (dy2 * dw + dx1);
            dskip = -dw - w;
        }
    } else {
        if (mirror) {
            doff32 = d->offset32 + (dy1 * dw + dx2);
            dskip = dw + w;
        } else {
            doff32 = d->offset32 + (dy1 * dw + dx1);
            dskip = dw - w;
        }
    }
    if (mirror)
        xdir = -1;
    else
        xdir = 1;
    // plot rect
    yy = h;
    fy = fsy1;
    fsx1 -= mx; // prev value is moved on from
    doff32 -= xdir;
    do {
        xx = w;
        ulp = s->offset32 + sw * qbr_double_to_long(fy);
        fx = fsx1;
        do {
            *(doff32 += xdir) = *(ulp + qbr_double_to_long(fx += mx));
        } while (--xx);
        doff32 += dskip;
        fy += my;
    } while (--yy);
    return;

put_8_stretch:
    // calc. starting points & change values
    if (flip) {
        if (mirror) {
            doff = d->offset + (dy2 * dw + dx2);
            dskip = -dw + w;
        } else {
            doff = d->offset + (dy2 * dw + dx1);
            dskip = -dw - w;
        }
    } else {
        if (mirror) {
            doff = d->offset + (dy1 * dw + dx2);
            dskip = dw + w;
        } else {
            doff = d->offset + (dy1 * dw + dx1);
            dskip = dw - w;
        }
    }
    if (mirror)
        xdir = -1;
    else
        xdir = 1;
    // plot rect
    yy = h;
    fy = fsy1;
    fsx1 -= mx; // prev value is moved on from
    doff -= xdir;
    do {
        xx = w;
        cp = s->offset + sw * qbr_double_to_long(fy);
        fx = fsx1;
        do {
            *(doff += xdir) = *(cp + qbr_double_to_long(fx += mx));
        } while (--xx);
        doff += dskip;
        fy += my;
    } while (--yy);
    return;

put_8_clear_stretch:
    clearcol = s->transparent_color;
    // calc. starting points & change values
    if (flip) {
        if (mirror) {
            doff = d->offset + (dy2 * dw + dx2);
            dskip = -dw + w;
        } else {
            doff = d->offset + (dy2 * dw + dx1);
            dskip = -dw - w;
        }
    } else {
        if (mirror) {
            doff = d->offset + (dy1 * dw + dx2);
            dskip = dw + w;
        } else {
            doff = d->offset + (dy1 * dw + dx1);
            dskip = dw - w;
        }
    }
    if (mirror)
        xdir = -1;
    else
        xdir = 1;
    // plot rect
    yy = h;
    fy = fsy1;
    fsx1 -= mx; // prev value is moved on from
    do {
        xx = w;
        cp = s->offset + sw * qbr_double_to_long(fy);
        fx = fsx1;
        do {
            if ((col = *(cp + qbr_double_to_long(fx += mx))) != clearcol) {
                *doff = col;
            }
            doff += xdir;
        } while (--xx);
        doff += dskip;
        fy += my;
    } while (--yy);
    return;

put_8_32_stretch:
    pal = s->pal;
    // calc. starting points & change values
    if (flip) {
        if (mirror) {
            doff32 = d->offset32 + (dy2 * dw + dx2);
            dskip = -dw + w;
        } else {
            doff32 = d->offset32 + (dy2 * dw + dx1);
            dskip = -dw - w;
        }
    } else {
        if (mirror) {
            doff32 = d->offset32 + (dy1 * dw + dx2);
            dskip = dw + w;
        } else {
            doff32 = d->offset32 + (dy1 * dw + dx1);
            dskip = dw - w;
        }
    }
    if (mirror)
        xdir = -1;
    else
        xdir = 1;
    // plot rect
    yy = h;
    fy = fsy1;
    fsx1 -= mx; // prev value is moved on from
    doff32 -= xdir;
    do {
        xx = w;
        cp = s->offset + sw * qbr_double_to_long(fy);
        fx = fsx1;
        do {
            *(doff32 += xdir) = pal[*(cp + qbr_double_to_long(fx += mx))];
        } while (--xx);
        doff32 += dskip;
        fy += my;
    } while (--yy);
    return;

put_8_32_clear_stretch:
    clearcol = s->transparent_color;
    pal = s->pal;
    // calc. starting points & change values
    if (flip) {
        if (mirror) {
            doff32 = d->offset32 + (dy2 * dw + dx2);
            dskip = -dw + w;
        } else {
            doff32 = d->offset32 + (dy2 * dw + dx1);
            dskip = -dw - w;
        }
    } else {
        if (mirror) {
            doff32 = d->offset32 + (dy1 * dw + dx2);
            dskip = dw + w;
        } else {
            doff32 = d->offset32 + (dy1 * dw + dx1);
            dskip = dw - w;
        }
    }
    if (mirror)
        xdir = -1;
    else
        xdir = 1;
    // plot rect
    yy = h;
    fy = fsy1;
    fsx1 -= mx; // prev value is moved on from
    do {
        xx = w;
        cp = s->offset + sw * qbr_double_to_long(fy);
        fx = fsx1;
        do {
            if ((col = *(cp + qbr_double_to_long(fx += mx))) != clearcol) {
                *doff32 = pal[col];
            }
            doff32 += xdir;
        } while (--xx);
        doff32 += dskip;
        fy += my;
    } while (--yy);
    return;

reverse:
    // mirror?
    if (dx2 < dx1) {
        if (sx2 > sx1)
            mirror = 1;
    }
    if (sx2 < sx1) {
        if (dx2 > dx1)
            mirror = 1;
    }
    if (dx2 < dx1) {
        x = dx1;
        dx1 = dx2;
        dx2 = x;
    }
    if (sx2 < sx1) {
        x = sx1;
        sx1 = sx2;
        sx2 = x;
    }
    // flip?
    if (dy2 < dy1) {
        if (sy2 > sy1)
            flip = 1;
    }
    if (sy2 < sy1) {
        if (dy2 > dy1)
            flip = 1;
    }
    if (dy2 < dy1) {
        y = dy1;
        dy1 = dy2;
        dy2 = y;
    }
    if (sy2 < sy1) {
        y = sy1;
        sy1 = sy2;
        sy2 = y;
    }

clip:
    // crop dest offscreen pixels
    if (dx1 < 0) {
        if (mirror)
            sx2 += dx1;
        else
            sx1 -= dx1;
        dx1 = 0;
    }
    if (dy1 < 0) {
        if (flip)
            sy2 += dy1;
        else
            sy1 -= dy1;
        dy1 = 0;
    }
    if (dx2 >= dw) {
        if (mirror)
            sx1 += (dx2 - dw + 1);
        else
            sx2 -= (dx2 - dw + 1);
        dx2 = dw - 1;
    }
    if (dy2 >= dh) {
        if (flip)
            sy1 += (dy2 - dh + 1);
        else
            sy2 -= (dy2 - dh + 1);
        dy2 = dh - 1;
    }
    // crop source offscreen pixels
    if (sx1 < 0) {
        if (mirror)
            dx2 += sx1;
        else
            dx1 -= sx1;
        sx1 = 0;
    }
    if (sy1 < 0) {
        if (flip)
            dy2 += sy1;
        else
            dy1 -= sy1;
        sy1 = 0;
    }
    if (sx2 >= sw) {
        if (mirror)
            dx1 += (sx2 - sw + 1);
        else
            dx2 -= (sx2 - sw + 1);
        sx2 = sw - 1;
    }
    if (sy2 >= sh) {
        if (flip)
            dy1 += (sy2 - sh + 1);
        else
            dy2 -= (sy2 - sh + 1);
        sy2 = sh - 1;
    }
    //<0-size/offscreen?
    // note: <0-size will cause reversal of dest
    //      offscreen values will result in reversal of dest
    if (dx1 > dx2)
        return;
    if (dy1 > dy2)
        return;
    // all values are now within the boundaries of the source & dest

    // mirror put
    if (mirror) {
        if (sbpp == 4) {
            if (s->alpha_disabled || d->alpha_disabled)
                goto put_32_noalpha_mirror;
            goto put_32_mirror;
        }
        if (dbpp == 1) {
            if (s->transparent_color == -1)
                goto put_8_mirror;
            goto put_8_clear_mirror;
        }
        if (s->transparent_color == -1)
            goto put_8_32_mirror;
        goto put_8_32_clear_mirror;
    } // mirror put

noflip:
    if (sbpp == 4) {
        if (s->alpha_disabled || d->alpha_disabled)
            goto put_32_noalpha;
        goto put_32;
    }
    if (dbpp == 1) {
        if (s->transparent_color == -1)
            goto put_8;
        goto put_8_clear;
    }
    if (s->transparent_color == -1)
        goto put_8_32;
    goto put_8_32_clear;

put_32:
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff32 = s->offset32 + (sy2 * sw + sx1);
        sskip = -w - sw;
    } else {
        soff32 = s->offset32 + (sy1 * sw + sx1);
        sskip = sw - w;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            //--------plot pixel--------
            switch ((col = *soff32++) & 0xFF000000) {
            case 0xFF000000:
                *doff32++ = col;
                break;
            case 0x0:
                doff32++;
                break;
            case 0x80000000:
                *doff32++ = (((*doff32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                break;
            case 0x7F000000:
                *doff32++ = (((*doff32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                break;
            default:
                destcol = *doff32;
                cp = cblend + (col >> 24 << 16);
                *doff32++ = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                            (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
            }; // switch
            //--------done plot pixel--------
        } while (--xx);
        soff32 += sskip;
        doff32 += dskip;
    } while (--h);
    return;

put_32_noalpha:
    doff32 = d->offset32 + (dy1 * dw + dx1);
    if (flip) {
        soff32 = s->offset32 + (sy2 * sw + sx1);
        sskip = -sw;
    } else {
        soff32 = s->offset32 + (sy1 * sw + sx1);
        sskip = sw;
    }
    h = dy2 - dy1 + 1;
    w = (dx2 - dx1 + 1) * 4;
    while (h--) {
        memcpy(doff32, soff32, w);
        soff32 += sskip;
        doff32 += dw;
    }
    return;

put_8:
    doff = d->offset + (dy1 * dw + dx1);
    if (flip) {
        soff = s->offset + (sy2 * sw + sx1);
        sskip = -sw;
    } else {
        soff = s->offset + (sy1 * sw + sx1);
        sskip = sw;
    }
    h = dy2 - dy1 + 1;
    w = dx2 - dx1 + 1;
    while (h--) {
        memcpy(doff, soff, w);
        soff += sskip;
        doff += dw;
    }
    return;

put_8_clear:
    clearcol = s->transparent_color;
    w = dx2 - dx1 + 1;
    doff = d->offset + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx1);
        sskip = -w - sw;
    } else {
        soff = s->offset + (sy1 * sw + sx1);
        sskip = sw - w;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            if ((col = *soff++) != clearcol) {
                *doff = col;
            }
            doff++;
        } while (--xx);
        soff += sskip;
        doff += dskip;
    } while (--h);
    return;

put_8_32:
    pal = s->pal;
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx1);
        sskip = -w - sw;
    } else {
        soff = s->offset + (sy1 * sw + sx1);
        sskip = sw - w;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            *doff32++ = pal[*soff++];
        } while (--xx);
        soff += sskip;
        doff32 += dskip;
    } while (--h);
    return;

put_8_32_clear:
    pal = s->pal;
    clearcol = s->transparent_color;
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx1);
        sskip = -w - sw;
    } else {
        soff = s->offset + (sy1 * sw + sx1);
        sskip = sw - w;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            if ((col = *soff++) != clearcol) {
                *doff32 = pal[col];
            }
            doff32++;
        } while (--xx);
        soff += sskip;
        doff32 += dskip;
    } while (--h);
    return;

put_32_mirror:
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff32 = s->offset32 + (sy2 * sw + sx2);
        sskip = -sw + w;
    } else {
        soff32 = s->offset32 + (sy1 * sw + sx2);
        sskip = w + sw;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            //--------plot pixel--------
            switch ((col = *soff32--) & 0xFF000000) {
            case 0xFF000000:
                *doff32++ = col;
                break;
            case 0x0:
                doff32++;
                break;
            case 0x80000000:
                *doff32++ = (((*doff32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend128[*doff32 >> 24] << 24);
                break;
            case 0x7F000000:
                *doff32++ = (((*doff32 & 0xFEFEFE) + (col & 0xFEFEFE)) >> 1) + (ablend127[*doff32 >> 24] << 24);
                break;
            default:
                destcol = *doff32;
                cp = cblend + (col >> 24 << 16);
                *doff32++ = cp[(col << 8 & 0xFF00) + (destcol & 255)] + (cp[(col & 0xFF00) + (destcol >> 8 & 255)] << 8) +
                            (cp[(col >> 8 & 0xFF00) + (destcol >> 16 & 255)] << 16) + (ablend[(col >> 24) + (destcol >> 16 & 0xFF00)] << 24);
            }; // switch
            //--------done plot pixel--------
        } while (--xx);
        soff32 += sskip;
        doff32 += dskip;
    } while (--h);
    return;

put_32_noalpha_mirror:
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff32 = s->offset32 + (sy2 * sw + sx2);
        sskip = -sw + w;
    } else {
        soff32 = s->offset32 + (sy1 * sw + sx2);
        sskip = w + sw;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            *doff32++ = *soff32--;
        } while (--xx);
        soff32 += sskip;
        doff32 += dskip;
    } while (--h);
    return;

put_8_mirror:
    w = dx2 - dx1 + 1;
    doff = d->offset + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx2);
        sskip = -sw + w;
    } else {
        soff = s->offset + (sy1 * sw + sx2);
        sskip = w + sw;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            *doff++ = *soff--;
        } while (--xx);
        soff += sskip;
        doff += dskip;
    } while (--h);
    return;

put_8_clear_mirror:
    clearcol = s->transparent_color;
    w = dx2 - dx1 + 1;
    doff = d->offset + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx2);
        sskip = -sw + w;
    } else {
        soff = s->offset + (sy1 * sw + sx2);
        sskip = w + sw;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            if ((col = *soff--) != clearcol) {
                *doff = col;
            }
            doff++;
        } while (--xx);
        soff += sskip;
        doff += dskip;
    } while (--h);
    return;

put_8_32_mirror:
    pal = s->pal;
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx2);
        sskip = -sw + w;
    } else {
        soff = s->offset + (sy1 * sw + sx2);
        sskip = w + sw;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            *doff32++ = pal[*soff--];
        } while (--xx);
        soff += sskip;
        doff32 += dskip;
    } while (--h);
    return;

put_8_32_clear_mirror:
    pal = s->pal;
    clearcol = s->transparent_color;
    w = dx2 - dx1 + 1;
    doff32 = d->offset32 + (dy1 * dw + dx1);
    dskip = dw - w;
    if (flip) {
        soff = s->offset + (sy2 * sw + sx2);
        sskip = -sw + w;
    } else {
        soff = s->offset + (sy1 * sw + sx2);
        sskip = w + sw;
    }
    // plot rect
    h = dy2 - dy1 + 1;
    do {
        xx = w;
        do {
            if ((col = *soff--) != clearcol) {
                *doff32 = pal[col];
            }
            doff32++;
        } while (--xx);
        soff += sskip;
        doff32 += dskip;
    } while (--h);
    return;

} // sub__putimage

/**
 * Gets pixel data from a rectangular region and stores it in a memory element.
 * 
 * Copies pixel data from the read page within the specified rectangular region
 * into a memory element structure. Supports STEP coordinates and mask colors
 * for areas outside the image bounds.
 * 
 * @param x1f X coordinate of first corner (or offset if using STEP)
 * @param y1f Y coordinate of first corner (or offset if using STEP)
 * @param x2f X coordinate of second corner (or offset if using STEP)
 * @param y2f Y coordinate of second corner (or offset if using STEP)
 * @param element Pointer to byte_element_struct to store the pixel data
 * @param mask Color value to use for pixels outside image bounds
 * @param passed Bit flags: bit 0=STEP for first point, bit 1=STEP for second point, bit 2=mask provided
 */
void sub_graphics_get(float x1f, float y1f, float x2f, float y2f, void *element, uint32 mask, int32 passed) {
    //"[{STEP}](?,?)-[{STEP}](?,?),?[,?]"
    //   &1            &2            &4
    if (is_error_pending())
        return;

    static int32 x1, y1, x2, y2, z, w, h, bits, x, y, bytes, sx, sy, x3, y3, z2;
    static uint32 col, off, col1, col2, col3, col4, byte;

    if (read_page->text) {
        error(5);
        return;
    }

    // change coordinates according to step
    if (passed & 1) {
        x1f = read_page->x + x1f;
        y1f = read_page->y + y1f;
    }
    read_page->x = x1f;
    read_page->y = y1f;
    if (passed & 2) {
        x2f = read_page->x + x2f;
        y2f = read_page->y + y2f;
    }
    read_page->x = x2f;
    read_page->y = y2f;

    // resolve coordinates
    if (read_page->clipping_or_scaling) {
        if (read_page->clipping_or_scaling == 2) {
            x1 = qbr_float_to_long(x1f * read_page->scaling_x + read_page->scaling_offset_x) + read_page->view_offset_x;
            y1 = qbr_float_to_long(y1f * read_page->scaling_y + read_page->scaling_offset_y) + read_page->view_offset_y;
            x2 = qbr_float_to_long(x2f * read_page->scaling_x + read_page->scaling_offset_x) + read_page->view_offset_x;
            y2 = qbr_float_to_long(y2f * read_page->scaling_y + read_page->scaling_offset_y) + read_page->view_offset_y;
        } else {
            x1 = qbr_float_to_long(x1f) + read_page->view_offset_x;
            y1 = qbr_float_to_long(y1f) + read_page->view_offset_y;
            x2 = qbr_float_to_long(x2f) + read_page->view_offset_x;
            y2 = qbr_float_to_long(y2f) + read_page->view_offset_y;
        }
    } else {
        x1 = qbr_float_to_long(x1f);
        y1 = qbr_float_to_long(y1f);
        x2 = qbr_float_to_long(x2f);
        y2 = qbr_float_to_long(y2f);
    }

    // swap coordinates if reversed
    if (x2 < x1) {
        z = x1;
        x1 = x2;
        x2 = z;
    }
    if (y2 < y1) {
        z = y1;
        y1 = y2;
        y2 = z;
    }

    sx = read_page->width;
    sy = read_page->height;

    // boundary checking (if no mask colour was passed)
    if (!(passed & 4)) {
        if ((x1 < 0) || (y1 < 0) || (x2 >= sx) || (y2 >= sy)) {
            error(5);
            return;
        }
    }

    static byte_element_struct *ele;
    ele = (byte_element_struct *)element;
    static uint16 *dimensions;
    dimensions = (uint16 *)(ele->offset);
    static uint8 *cp, *cp1, *cp2, *cp3, *cp4;
    cp = (uint8 *)(ele->offset + 4);
    static uint32 *lp;
    lp = (uint32 *)(ele->offset + 4);

    w = x2 - x1 + 1;
    h = y2 - y1 + 1;
    bits = read_page->bits_per_pixel;

    if (bits == 1) {
        mask &= 1;
        z = (w + 7) >> 3;
        bytes = z * h + 4;
        if (bytes > ele->length) {
            error(5);
            return;
        }
        dimensions[0] = w;
        dimensions[1] = h;
        for (y = y1; y <= y2; y++) {
            z2 = 128;
            col2 = 0;
            off = y * sx + x1;
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy))
                    col = read_page->offset[off];
                else
                    col = mask;
                if (col)
                    col2 |= z2;
                z2 >>= 1;
                if (!z2) {
                    z2 = 128;
                    *cp++ = col2;
                    col2 = 0;
                }
                off++;
            }
            if (z2 != 128)
                *cp++ = col2;
        }
        return;
    } // 1

    if (bits == 2) {
        mask &= 3;
        z = (w + 7) >> 3;
        bytes = z * h + 4;
        if (bytes > ele->length) {
            error(5);
            return;
        }
        dimensions[0] = w * 2;
        dimensions[1] = h;
        for (y = y1; y <= y2; y++) {
            byte = 0;
            x3 = 0;
            off = y * sx + x1;
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy))
                    col = read_page->offset[off];
                else
                    col = mask;
                byte <<= 2;
                byte |= col;
                if ((x3 & 3) == 3) {
                    *cp++ = byte;
                    byte = 0;
                }
                x3++;
                off++;
            }
            if (x3 & 3)
                *cp++ = col2;
        }
        return;
    } // 2

    if (bits == 4) {
        mask &= 15;
        z = (w + 7) >> 3;
        bytes = z * 4 * h + 4;
        if (bytes > ele->length) {
            error(5);
            return;
        }
        dimensions[0] = w;
        dimensions[1] = h;
        y3 = 0;
        for (y = y1; y <= y2; y++) {
            z2 = 128;
            off = y * sx + x1;
            cp1 = cp + y3 * z * 4;
            cp2 = cp + y3 * z * 4 + z;
            cp3 = cp + y3 * z * 4 + z * 2;
            cp4 = cp + y3 * z * 4 + z * 3;
            col1 = 0;
            col2 = 0;
            col3 = 0;
            col4 = 0;
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy))
                    col = read_page->offset[off];
                else
                    col = mask;
                if (col & 1)
                    col1 |= z2;
                if (col & 2)
                    col2 |= z2;
                if (col & 4)
                    col3 |= z2;
                if (col & 8)
                    col4 |= z2;
                z2 >>= 1;
                if (!z2) {
                    z2 = 128;
                    *cp1++ = col1;
                    *cp2++ = col2;
                    *cp3++ = col3;
                    *cp4++ = col4;
                    col1 = 0;
                    col2 = 0;
                    col3 = 0;
                    col4 = 0;
                }
                off++;
            }
            if (z2 != 128) {
                *cp1 = col1;
                *cp2 = col2;
                *cp3 = col3;
                *cp4 = col4;
            }
            y3++;
        }
        return;
    } // 4

    if (bits == 8) {
        mask &= 255;
        bytes = w * h + 4;
        if (bytes > ele->length) {
            error(5);
            return;
        }
        dimensions[0] = w * 8;
        dimensions[1] = h;
        for (y = y1; y <= y2; y++) {
            off = y * sx + x1;
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy))
                    col = read_page->offset[off];
                else
                    col = mask;
                *cp++ = col;
                off++;
            }
        }
        return;
    } // 8

    if (bits == 32) {
        bytes = w * h * 4 + 4;
        if (bytes > ele->length) {
            error(5);
            return;
        }
        dimensions[0] = w;
        dimensions[1] = h; // note: width is left unmultiplied
        for (y = y1; y <= y2; y++) {
            off = y * sx + x1;
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy))
                    col = read_page->offset32[off];
                else
                    col = mask;
                *lp++ = col;
                off++;
            }
        }
        return;
    } // 32

} // sub_graphics_get

/**
 * Puts pixel data from a memory element onto the write page.
 * 
 * Draws pixel data stored in a memory element structure onto the write page
 * at the specified position. Supports various drawing options (PSET, PRESET,
 * AND, OR, XOR) and mask colors for transparency.
 * 
 * @param x1f X coordinate of top-left corner (or offset if using STEP)
 * @param y1f Y coordinate of top-left corner (or offset if using STEP)
 * @param element Pointer to byte_element_struct containing pixel data
 * @param option Drawing option: 0=PSET, 1=PRESET, 2=AND, 3=OR, 4=XOR
 * @param mask Color value to use as transparent/mask color
 * @param passed Bit flags: bit 0=STEP coordinates, bit 1=option provided, bit 2=mask provided
 */
void sub_graphics_put(float x1f, float y1f, void *element, int32 option, uint32 mask, int32 passed) {
    //"[{STEP}](?,?),?[,[{_CLIP}][{PSET|PRESET|AND|OR|XOR}][,?]]"
    // step->passed&1
    // clip->passed&2
    // mask->passed&4

    if (is_error_pending())
        return;

    static int32 step, clip;
    step = 0;
    clip = 0;
    if (passed & 1) {
        step = 1;
        passed -= 1;
    }
    if (passed & 2) {
        clip = 1;
        passed -= 2;
    }

    static int32 x1, y1, x2, y2, z, w, h, bits, x, y, bytes, sx, sy, x3, y3, z2;
    static uint32 col, off, col1, col2, col3, col4, byte, pixelmask;

    if (write_page->text) {
        error(5);
        return;
    }

    // change coordinates according to step
    if (step) {
        x1f += write_page->x;
        y1f += write_page->y;
        write_page->x = x1f;
        write_page->y = y1f;
    }

    // resolve coordinates
    if (write_page->clipping_or_scaling) {
        if (write_page->clipping_or_scaling == 2) {
            x1 = qbr_float_to_long(x1f * write_page->scaling_x + write_page->scaling_offset_x) + write_page->view_offset_x;
            y1 = qbr_float_to_long(y1f * write_page->scaling_y + write_page->scaling_offset_y) + write_page->view_offset_y;
        } else {
            x1 = qbr_float_to_long(x1f) + write_page->view_offset_x;
            y1 = qbr_float_to_long(y1f) + write_page->view_offset_y;
        }
    } else {
        x1 = qbr_float_to_long(x1f);
        y1 = qbr_float_to_long(y1f);
    }

    sx = write_page->width;
    sy = write_page->height;
    bits = write_page->bits_per_pixel;

    static byte_element_struct *ele;
    ele = (byte_element_struct *)element;
    static uint16 *dimensions;
    dimensions = (uint16 *)(ele->offset);
    static uint8 *cp, *cp1, *cp2, *cp3, *cp4;
    cp = (uint8 *)(ele->offset + 4);
    static uint32 *lp;
    lp = (uint32 *)(ele->offset + 4);

    static uint8 *offp;
    static uint32 *off32p;

    if (4 > ele->length) {
        error(5);
        return;
    }

    // get dimensions
    w = dimensions[0];
    h = dimensions[1];
    z = w; //(used below)
    if (bits == 2) {
        if (w & 1) {
            error(5);
            return;
        } else
            w >>= 1;
    }
    if (bits == 8) {
        if (w & 7) {
            error(5);
            return;
        } else
            w >>= 3;
    }
    x2 = x1 + w - 1;
    y2 = y1 + h - 1;

    // boundary checking (if CLIP option was not used)
    if (!clip) {
        if ((x1 < 0) || (y1 < 0) || (x2 >= sx) || (y2 >= sy)) {
            error(5);
            return;
        }
    }

    // array size check (avoid reading unallocated memory)
    if (bits == 32)
        z *= 32;
    z = (z + 7) >> 3; // bits per row->bytes per row
    bytes = h * z;
    if (bits == 4)
        bytes *= 4;
    if ((bytes + 4) > ele->length) {
        error(5);
        return;
    }

    pixelmask = write_page->mask;

    if (bits == 1) {
        mask &= 1;
        y3 = 0;
        for (y = y1; y <= y2; y++) {
            offp = (uint8 *)write_page->offset + (y * sx + x1);
            x3 = 0;
            for (x = x1; x <= x2; x++) {
                if (!(x3--)) {
                    x3 = 7;
                    col2 = *cp++;
                }
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy)) {
                    col = (col2 >> x3) & 1;
                    if ((!passed) || (col != mask)) {
                        switch (option) {
                        case 0:
                            *offp ^= col;
                            break;
                        case 1:
                            *offp = col;
                            break;
                        case 2:
                            *offp = (~col) & pixelmask;
                            break;
                        case 3:
                            *offp &= col;
                            break;
                        case 4:
                            *offp |= col;
                            break;
                        case 5:
                            *offp ^= col;
                            break;
                        }
                    } // mask
                } // bounds
                offp++;
            }
            y3++;
        }
        return;
    } // 1

    if (bits == 2) {
        mask &= 3;
        y3 = 0;
        for (y = y1; y <= y2; y++) {
            offp = (uint8 *)write_page->offset + (y * sx + x1);
            x3 = 0;
            for (x = x1; x <= x2; x++) {
                if (!(x3--)) {
                    x3 = 3;
                    col2 = *cp++;
                }
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy)) {
                    col = (col2 >> (x3 << 1)) & 3;
                    if ((!passed) || (col != mask)) {
                        switch (option) {
                        case 0:
                            *offp ^= col;
                            break;
                        case 1:
                            *offp = col;
                            break;
                        case 2:
                            *offp = (~col) & pixelmask;
                            break;
                        case 3:
                            *offp &= col;
                            break;
                        case 4:
                            *offp |= col;
                            break;
                        case 5:
                            *offp ^= col;
                            break;
                        }
                    } // mask
                } // bounds
                offp++;
            }
            y3++;
        }
        return;
    } // 2

    if (bits == 4) {
        mask &= 15;
        y3 = 0;
        for (y = y1; y <= y2; y++) {
            offp = (uint8 *)write_page->offset + (y * sx + x1);
            cp1 = cp + y3 * z * 4;
            cp2 = cp + y3 * z * 4 + z;
            cp3 = cp + y3 * z * 4 + z * 2;

            cp4 = cp + y3 * z * 4 + z * 3;
            x3 = 0;
            for (x = x1; x <= x2; x++) {
                if (!(x3--)) {
                    x3 = 7;
                    col1 = *cp1++;
                    col2 = (*cp2++) << 1;
                    col3 = (*cp3++) << 2;
                    col4 = (*cp4++) << 3;
                }
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy)) {
                    col = ((col1 >> x3) & 1) | ((col2 >> x3) & 2) | ((col3 >> x3) & 4) | ((col4 >> x3) & 8);
                    if ((!passed) || (col != mask)) {
                        switch (option) {
                        case 0:
                            *offp ^= col;
                            break;
                        case 1:
                            *offp = col;
                            break;
                        case 2:
                            *offp = (~col) & pixelmask;
                            break;
                        case 3:
                            *offp &= col;
                            break;
                        case 4:
                            *offp |= col;
                            break;
                        case 5:
                            *offp ^= col;
                            break;
                        }
                    } // mask
                } // bounds
                offp++;
            }
            y3++;
        }
        return;
    } // 4

    if (bits == 8) {
        mask &= 255;
        for (y = y1; y <= y2; y++) {
            offp = (uint8 *)write_page->offset + (y * sx + x1);
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy)) {
                    col = *cp;
                    if ((!passed) || (col != mask)) {
                        switch (option) {
                        case 0:
                            *offp ^= col;
                            break;
                        case 1:
                            *offp = col;
                            break;
                        case 2:
                            *offp = (~col) & pixelmask;
                            break;
                        case 3:
                            *offp &= col;
                            break;
                        case 4:
                            *offp |= col;
                            break;
                        case 5:
                            *offp ^= col;
                            break;
                        }
                    } // mask
                } // bounds
                offp++;
                cp++;
            }
        }
        return;
    } // 8

    if (bits == 32) {
        for (y = y1; y <= y2; y++) {
            off32p = (uint32 *)write_page->offset32 + (y * sx + x1);
            for (x = x1; x <= x2; x++) {
                if ((x >= 0) && (y >= 0) && (x < sx) && (y < sy)) {
                    col = *lp;
                    if ((!passed) || (col != mask)) {
                        switch (option) {
                        case 0:
                            *off32p ^= col;
                            break;
                        case 1:
                            *off32p = col;
                            break;
                        case 2:
                            *off32p = (~col) & pixelmask;
                            break;
                        case 3:
                            *off32p &= col;
                            break;
                        case 4:
                            *off32p |= col;
                            break;
                        case 5:
                            *off32p ^= col;
                            break;
                        }
                    } // mask
                } // bounds
                off32p++;
                lp++;
            }
        }
        return;
    } // 32
} // sub_graphics_put

// ============================================================================
// HARDWARE TEXTURE FUNCTIONS
// ============================================================================

#ifndef QB64_GUI // begin stubs

// STUB: simulate generating a hardware surface (non-GUI builds)
int32_t new_hardware_img(int32_t x, int32_t y, uint32_t *pixels, int32_t flags) {
    // create hardware img
    int32_t handle;
    hardware_img_struct *hardware_img;
    handle = list_add(hardware_img_handles);
    hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, handle);
    hardware_img->w = x;
    hardware_img->h = y;
    hardware_img->dest_context_handle = 0;
    hardware_img->depthbuffer_handle = 0;
    hardware_img->pending_commands = 0;
    hardware_img->remove = 0;
    hardware_img->alpha_disabled = 0;
    hardware_img->depthbuffer_mode = DEPTHBUFFER_MODE__ON;
    hardware_img->valid = 1;
    hardware_img->source_state.PO2_fix = PO2_FIX__OFF;
    hardware_img->source_state.texture_wrap = TEXTURE_WRAP_MODE__UNKNOWN;
    hardware_img->source_state.smooth_stretched = SMOOTH_MODE__UNKNOWN;
    hardware_img->source_state.smooth_shrunk = SMOOTH_MODE__UNKNOWN;
    if (flags & NEW_HARDWARE_IMG__BUFFER_CONTENT) {
        hardware_img->texture_handle = 0;
        if (flags & NEW_HARDWARE_IMG__DUPLICATE_PROVIDED_BUFFER) {
            hardware_img->software_pixel_buffer = NULL;
        } else {
            free(pixels); // the buffer was meant to be consumed, so we just free it immediately
            hardware_img->software_pixel_buffer = NULL;
        }
    }
    return handle;
}

// Stub implementations for non-GUI builds
int32_t new_texture_handle() { return 0; }
void hardware_img_buffer_to_texture(int32_t handle) { (void)handle; }
void hardware_img_requires_depthbuffer(hardware_img_struct *hardware_img) { (void)hardware_img; }
void free_hardware_img(int32_t handle, int32_t caller_id) {
    (void)caller_id;
    hardware_img_struct *hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, handle);
    if (hardware_img) {
        if (hardware_img->software_pixel_buffer) {
            free(hardware_img->software_pixel_buffer);
        }
        list_remove(hardware_img_handles, handle);
    }
}
void hardware_buffer_flush() {}
void set_smooth(int32_t new_mode_shrunk, int32_t new_mode_stretched) { (void)new_mode_shrunk; (void)new_mode_stretched; }
void set_texture_wrap(int32_t new_mode) { (void)new_mode; }
void set_render_source(int32_t new_handle) { (void)new_handle; }
void set_render_dest(int32_t new_handle) { (void)new_handle; }
void hardware_img_put(int32_t dst_x1, int32_t dst_y1, int32_t dst_x2, int32_t dst_y2,
                      int32_t src_img, int32_t dst_img, int32_t src_x1, int32_t src_y1,
                      int32_t src_x2, int32_t src_y2, int32_t use_alpha, int32_t smooth) {
    (void)dst_x1; (void)dst_y1; (void)dst_x2; (void)dst_y2;
    (void)src_img; (void)dst_img; (void)src_x1; (void)src_y1;
    (void)src_x2; (void)src_y2; (void)use_alpha; (void)smooth;
}
void hardware_img_tri2d(float dst_x1, float dst_y1, float dst_x2, float dst_y2,
                        float dst_x3, float dst_y3, int32_t src_img, int32_t dst_img,
                        float src_x1, float src_y1, float src_x2, float src_y2,
                        float src_x3, float src_y3, int32_t use_alpha, int32_t smooth) {
    (void)dst_x1; (void)dst_y1; (void)dst_x2; (void)dst_y2;
    (void)dst_x3; (void)dst_y3; (void)src_img; (void)dst_img;
    (void)src_x1; (void)src_y1; (void)src_x2; (void)src_y2;
    (void)src_x3; (void)src_y3; (void)use_alpha; (void)smooth;
}
void hardware_img_tri3d(float dst_x1, float dst_y1, float dst_z1,
                        float dst_x2, float dst_y2, float dst_z2,
                        float dst_x3, float dst_y3, float dst_z3,
                        int32_t src_img, int32_t dst_img,
                        float src_x1, float src_y1, float src_x2, float src_y2,
                        float src_x3, float src_y3, int32_t use_alpha,
                        int32_t smooth, int32_t cull_mode, int32_t depthbuffer_mode) {
    (void)dst_x1; (void)dst_y1; (void)dst_z1;
    (void)dst_x2; (void)dst_y2; (void)dst_z2;
    (void)dst_x3; (void)dst_y3; (void)dst_z3;
    (void)src_img; (void)dst_img;
    (void)src_x1; (void)src_y1; (void)src_x2; (void)src_y2;
    (void)src_x3; (void)src_y3; (void)use_alpha;
    (void)smooth; (void)cull_mode; (void)depthbuffer_mode;
}

#else // QB64_GUI - Full OpenGL implementations

uint32_t *NPO2_texture_generate(int32_t *px, int32_t *py, uint32_t *pixels) {
    auto ox = *px;
    auto oy = *py;

    // assume not negative & not 0
    int32_t nx = Math_RoundUpToPowerOf2(uint32_t(ox));
    int32_t ny = Math_RoundUpToPowerOf2(uint32_t(oy));

    if (nx == ox && ny == oy) {
        // no action required
        return pixels;
    }

    int32_t size_in_pixels = nx * ny;
    if (size_in_pixels > NPO2_buffer_size_in_pixels) {
        NPO2_buffer = (uint32_t *)realloc(NPO2_buffer, size_in_pixels * 4);
        NPO2_buffer_size_in_pixels = size_in_pixels;
    }

    // copy source NPO2 rectangle into destination PO2 rectangle
    if (nx == ox) { // can copy as a single block
        memcpy(NPO2_buffer, pixels, ox * oy * 4);
    } else {
        uint32_t *dst_pixel_offset = NPO2_buffer;
        uint32_t *src_pixel_offset = pixels;
        while (oy--) {
            memcpy(dst_pixel_offset, src_pixel_offset, ox * 4);
            dst_pixel_offset += nx;
            src_pixel_offset += ox;
        }
        oy = *py;
    }

    // tidy edges - extend the right-most column and bottom-most row to avoid
    // pixel/color bleeding rhs column
    if (ox != nx) {
        for (int y = 0; y < oy; y++) {
            NPO2_buffer[ox + nx * y] = NPO2_buffer[ox + nx * y - 1];
        }
    }
    // bottom row + 1 pixel for corner
    if (oy != ny) {
        for (int x = 0; x < (ox + 1); x++) {
            NPO2_buffer[nx * oy + x] = NPO2_buffer[nx * oy + x - nx];
        }
    }

    *px = nx;
    *py = ny;

    return NPO2_buffer;
}

int32_t new_texture_handle() {
    GLuint texture = 0;
    glGenTextures(1, &texture);
    return (int32_t)texture;
}

int32_t new_hardware_img(int32_t x, int32_t y, uint32_t *pixels, int32_t flags) {
    // note: non power-of-2 dimensioned textures are supported on modern 3D cards and
    //      even on some older cards, as long as mip-mapping is not being used
    //      therefore, no attempt is made to convert the non-power-of-2 SCREEN
    //      sizes via software to avoid the performance hit this would incur
    // create hardware img
    int32_t handle;
    hardware_img_struct *hardware_img;
    handle = list_add(hardware_img_handles);
    hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, handle);
    hardware_img->w = x;
    hardware_img->h = y;
    hardware_img->dest_context_handle = 0;
    hardware_img->depthbuffer_handle = 0;
    hardware_img->pending_commands = 0;
    hardware_img->remove = 0;
    hardware_img->alpha_disabled = 0;
    hardware_img->depthbuffer_mode = DEPTHBUFFER_MODE__ON;
    hardware_img->valid = 1;
    hardware_img->source_state.PO2_fix = PO2_FIX__OFF;
    hardware_img->source_state.texture_wrap = TEXTURE_WRAP_MODE__UNKNOWN;
    hardware_img->source_state.smooth_stretched = SMOOTH_MODE__UNKNOWN;
    hardware_img->source_state.smooth_shrunk = SMOOTH_MODE__UNKNOWN;

    if (flags & NEW_HARDWARE_IMG__BUFFER_CONTENT) {
        hardware_img->texture_handle = 0;
        if (flags & NEW_HARDWARE_IMG__DUPLICATE_PROVIDED_BUFFER) {
            hardware_img->software_pixel_buffer = (uint32_t *)malloc(x * y * 4);
            memcpy(hardware_img->software_pixel_buffer, pixels, x * y * 4);
        } else {
            hardware_img->software_pixel_buffer = pixels;
        }
    } else {
        hardware_img->software_pixel_buffer = NULL;
        hardware_img->texture_handle = new_texture_handle();
        glBindTexture(GL_TEXTURE_2D, hardware_img->texture_handle);
        // non-power of 2 dimensions fallback support
        static int glerrorcode;
        glerrorcode = glGetError(); // clear any previous errors
        if (force_NPO2_fix == 0)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
        glerrorcode = glGetError();
        if (glerrorcode != 0 || force_NPO2_fix == 1) {
            int32_t nx = x, ny = y;
            uint32_t *npixels = NPO2_texture_generate(&nx, &ny, pixels);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nx, ny, 0, GL_BGRA, GL_UNSIGNED_BYTE, npixels);
            hardware_img->source_state.PO2_fix = PO2_FIX__EXPANDED;
            hardware_img->PO2_w = nx;
            hardware_img->PO2_h = ny;
            glerrorcode = glGetError();
            if (glerrorcode) {
                gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, x, y, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
                glerrorcode = glGetError();
                if (glerrorcode) {
                    gui_alert("gluBuild2DMipmaps failed: %i", glerrorcode);
                }
                hardware_img->source_state.PO2_fix = PO2_FIX__MIPMAPPED;
                hardware_img->PO2_w = x;
                hardware_img->PO2_h = y;
            }
        }
        set_render_source(INVALID_HARDWARE_HANDLE);
    }
    return handle;
}

void hardware_img_buffer_to_texture(int32_t handle) {
    static hardware_img_struct *hardware_img;
    hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, handle);
    if (hardware_img->texture_handle == 0) {
        hardware_img->texture_handle = new_texture_handle();
        glBindTexture(GL_TEXTURE_2D, hardware_img->texture_handle);
        // non-power of 2 dimensions fallback support
        static int glerrorcode;
        glerrorcode = glGetError(); // clear any previous errors
        if (force_NPO2_fix == 0)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, hardware_img->w, hardware_img->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, hardware_img->software_pixel_buffer);
        glerrorcode = glGetError();
        if (glerrorcode != 0 || force_NPO2_fix == 1) {
            hardware_img->source_state.PO2_fix = PO2_FIX__EXPANDED;
            int32_t x = hardware_img->w;
            int32_t y = hardware_img->h;
            uint32_t *pixels = NPO2_texture_generate(&x, &y, hardware_img->software_pixel_buffer);
            hardware_img->PO2_w = x;
            hardware_img->PO2_h = y;
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
            glerrorcode = glGetError();
            if (glerrorcode) {
                gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, hardware_img->w, hardware_img->h, GL_BGRA, GL_UNSIGNED_BYTE, hardware_img->software_pixel_buffer);
                glerrorcode = glGetError();
                if (glerrorcode) {
                    gui_alert("gluBuild2DMipmaps failed: %i", glerrorcode);
                }
                hardware_img->source_state.PO2_fix = PO2_FIX__MIPMAPPED;
                hardware_img->PO2_w = hardware_img->w;
                hardware_img->PO2_h = hardware_img->h;
            }
        }
        free(hardware_img->software_pixel_buffer);
        hardware_img->software_pixel_buffer = NULL; // 2015 critical bug fix
        set_render_source(INVALID_HARDWARE_HANDLE);
    }
}

void hardware_img_requires_depthbuffer(hardware_img_struct *hardware_img) {
    if (hardware_img->depthbuffer_handle == 0) {
        // inspiration...
        // http://www.opengl.org/wiki/Framebuffer_Object_Examples#Color_texture.2C_Depth_texture
        static GLuint depth_tex;
        glGenTextures(1, &depth_tex);
        glBindTexture(GL_TEXTURE_2D, depth_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, hardware_img->w, hardware_img->h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depth_tex, 0 /*mipmap level*/);

        // NULL means reserve texture memory, but texels are undefined
        glClear(GL_DEPTH_BUFFER_BIT);
        hardware_img->depthbuffer_handle = depth_tex;
        set_render_source(INVALID_HARDWARE_HANDLE);
    }
}

void free_hardware_img(int32_t handle, int32_t caller_id) {
    hardware_img_struct *hardware_img;
    hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, handle);

    if (hardware_img == NULL) {
        gui_alert("free_hardware_img: image does not exist");
    }

    if (hardware_img->dest_context_handle) {
        GLuint context = (GLuint)hardware_img->dest_context_handle;
        glDeleteFramebuffersEXT(1, &context);
    }
    if (hardware_img->depthbuffer_handle) {
        GLuint depthbuffer_handle = (GLuint)hardware_img->depthbuffer_handle;
        glDeleteFramebuffersEXT(1, &depthbuffer_handle);
    }
    GLuint texture = (GLuint)hardware_img->texture_handle;
    glDeleteTextures(1, &texture);

    // if image has not been used, it may still have buffered pixel content
    if (hardware_img->software_pixel_buffer != NULL) {
        free(hardware_img->software_pixel_buffer);
    }

    list_remove(hardware_img_handles, handle);
}

void hardware_buffer_flush() {
    if (hardware_buffer_vertices_count) {
        // ref: http://stackoverflow.com/questions/5009014/draw-square-with-opengl-es-for-ios
        if (hardware_buffer_vertices_count == hardware_buffer_texcoords_count) {
            glVertexPointer(2, GL_FLOAT, 2 * sizeof(GL_FLOAT), hardware_buffer_vertices);
            glTexCoordPointer(2, GL_FLOAT, 2 * sizeof(GL_FLOAT), hardware_buffer_texcoords);
            glDrawArrays(GL_TRIANGLES, 0, hardware_buffer_vertices_count / 2);
        } else {
            glVertexPointer(3, GL_FLOAT, 3 * sizeof(GL_FLOAT), hardware_buffer_vertices);
            glTexCoordPointer(2, GL_FLOAT, 2 * sizeof(GL_FLOAT), hardware_buffer_texcoords);
            glDrawArrays(GL_TRIANGLES, 0, hardware_buffer_vertices_count / 3);
        }
        hardware_buffer_vertices_count = 0;
        hardware_buffer_texcoords_count = 0;
    }
}

void set_smooth(int32_t new_mode_shrunk, int32_t new_mode_stretched) {
    static int32_t current_mode_shrunk;
    current_mode_shrunk = render_state.source->smooth_shrunk;
    static int32_t current_mode_stretched;
    current_mode_stretched = render_state.source->smooth_stretched;
    if (new_mode_shrunk == current_mode_shrunk && new_mode_stretched == current_mode_stretched)
        return;
    hardware_buffer_flush();
    if (new_mode_shrunk == SMOOTH_MODE__DONT_SMOOTH) {
        if (render_state.source->PO2_fix == PO2_FIX__MIPMAPPED) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        } else {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        }
    }
    if (new_mode_shrunk == SMOOTH_MODE__SMOOTH) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    if (new_mode_stretched == SMOOTH_MODE__DONT_SMOOTH) {
        if (render_state.source->PO2_fix == PO2_FIX__MIPMAPPED) {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
    }
    if (new_mode_stretched == SMOOTH_MODE__SMOOTH) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    render_state.source->smooth_shrunk = new_mode_shrunk;
    render_state.source->smooth_stretched = new_mode_stretched;
}

void set_texture_wrap(int32_t new_mode) {
    static int32_t current_mode;
    current_mode = render_state.source->texture_wrap;
    if (new_mode == current_mode)
        return;
    hardware_buffer_flush();
    if (new_mode == TEXTURE_WRAP_MODE__DONT_WRAP) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    if (new_mode == TEXTURE_WRAP_MODE__WRAP) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    render_state.source->texture_wrap = new_mode;
}

void set_render_source(int32_t new_handle) {
    if (new_handle == INVALID_HARDWARE_HANDLE) {
        hardware_buffer_flush();
        render_state.source_handle = INVALID_HARDWARE_HANDLE;
        return;
    }
    int32_t current_handle;
    current_handle = render_state.source_handle;

    if (current_handle == new_handle)
        return;

    hardware_buffer_flush();

    hardware_img_struct *hardware_img;
    hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, new_handle);
    if (hardware_img->texture_handle == 0)
        hardware_img_buffer_to_texture(new_handle);
    glBindTexture(GL_TEXTURE_2D, hardware_img->texture_handle);
    render_state.source_handle = new_handle;
    render_state.source = &hardware_img->source_state;

    // note: some older systems require calling glTexParameterf after textures are rebound
    if (framebufferobjects_supported == 0) {
        render_state.source->smooth_shrunk = SMOOTH_MODE__UNKNOWN;
        render_state.source->smooth_stretched = SMOOTH_MODE__UNKNOWN;
    }
}

void set_render_dest(int32_t new_handle) {
    if (new_handle == INVALID_HARDWARE_HANDLE) {
        hardware_buffer_flush();
        render_state.dest_handle = INVALID_HARDWARE_HANDLE;
        set_view(VIEW_MODE__UNKNOWN);
        return;
    }
    // 0=primary surface
    static int32_t current_handle;
    current_handle = render_state.dest_handle;
    if (new_handle == current_handle)
        return;
    hardware_buffer_flush();
    set_view(VIEW_MODE__UNKNOWN);
    if (new_handle == 0) {
        if (framebufferobjects_supported)
            glBindFramebufferEXT(GL_FRAMEBUFFER, 0);
        render_state.dest = &dest_render_state0;
    } else {
        static hardware_img_struct *hardware_img;
        hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, new_handle);
        // convert to regular texture first if necessary
        if (hardware_img->texture_handle == 0)
            hardware_img_buffer_to_texture(new_handle);
        // does it have a dest context/FBO? if not create one
        if (hardware_img->dest_context_handle == 0) {
            static GLuint framebuffer_handle;
            framebuffer_handle = 0;
            glGenFramebuffersEXT(1, &framebuffer_handle);
            glBindFramebufferEXT(GL_FRAMEBUFFER, framebuffer_handle);
            hardware_img->dest_context_handle = framebuffer_handle;
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hardware_img->texture_handle, 0);

            glColor4f(1.f, 1.f, 1.f, 1.f);

            set_render_source(INVALID_HARDWARE_HANDLE);
        } else {
            glBindFramebufferEXT(GL_FRAMEBUFFER, hardware_img->dest_context_handle);
        }
        render_state.dest = &hardware_img->dest_state;
    }
    render_state.dest_handle = new_handle;
}

void hardware_img_put(int32_t dst_x1, int32_t dst_y1, int32_t dst_x2, int32_t dst_y2, int32_t src_img, int32_t dst_img, int32_t src_x1, int32_t src_y1, int32_t src_x2,
                      int32_t src_y2, int32_t use_alpha, int32_t smooth) {
    if (dst_img < 0)
        dst_img = 0; // both layers render to the primary context

    // ensure dst_x1/y1 represent top-left co-ordinate of destination
    static int32_t swap_tmp;
    if (dst_x2 < dst_x1) {
        swap_tmp = dst_x2;
        dst_x2 = dst_x1;
        dst_x1 = swap_tmp;
        swap_tmp = src_x2;
        src_x2 = src_x1;
        src_x1 = swap_tmp;
    }
    if (dst_y2 < dst_y1) {
        swap_tmp = dst_y2;
        dst_y2 = dst_y1;
        dst_y1 = swap_tmp;
        swap_tmp = src_y2;
        src_y2 = src_y1;
        src_y1 = swap_tmp;
    }

    set_render_dest(dst_img);

    set_view(VIEW_MODE__2D);

    if (dst_img) {
        // (no specific action required here --area reserved for future use)
    } else { // dest is 0
        environment_2d__window_rect_struct *rect;
        rect = environment_2d__screen_to_window_rect(dst_x1, dst_y1, dst_x2, dst_y2);
        dst_x1 = rect->x1;
        dst_y1 = rect->y1;
        dst_x2 = rect->x2;
        dst_y2 = rect->y2;
    }

    set_render_source(src_img);

    static hardware_img_struct *src_hardware_img;
    static int32_t src_h, src_w;
    src_hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, src_img);
    src_h = src_hardware_img->h;
    src_w = src_hardware_img->w;

    if (smooth) {
        set_smooth(SMOOTH_MODE__SMOOTH, SMOOTH_MODE__SMOOTH);
    } else {
        set_smooth(SMOOTH_MODE__DONT_SMOOTH, SMOOTH_MODE__DONT_SMOOTH);
    }

    if (use_alpha) {
        set_alpha(ALPHA_MODE__BLEND);
    } else {
        set_alpha(ALPHA_MODE__DONT_BLEND);
    }

    set_depthbuffer(DEPTHBUFFER_MODE__OFF);
    set_cull_mode(CULL_MODE__NONE);

    set_texture_wrap(TEXTURE_WRAP_MODE__DONT_WRAP);

    // adjust for render (x2 & y2 need to be one greater than the destination offset)
    dst_x2++;
    dst_y2++;

    if (src_hardware_img->source_state.PO2_fix) {
        src_w = src_hardware_img->PO2_w;
        src_h = src_hardware_img->PO2_h;
    }

    // calc source texture co-ordinates
    static float x1f, y1f, x2f, y2f;
    if (src_x1 <= src_x2) {
        x1f = ((float)src_x1 + 0.01f) / (float)src_w;
        x2f = ((float)src_x2 + 0.99f) / (float)src_w;
    } else {
        x2f = ((float)src_x2 + 0.01f) / (float)src_w;
        x1f = ((float)src_x1 + 0.99f) / (float)src_w;
    }
    if (src_y1 <= src_y2) {
        y1f = ((float)src_y1 + 0.01f) / (float)src_h;
        y2f = ((float)src_y2 + 0.99f) / (float)src_h;
    } else {
        y2f = ((float)src_y2 + 0.01f) / (float)src_h;
        y1f = ((float)src_y1 + 0.99f) / (float)src_h;
    }

    // expand buffers if necessary
    if ((hardware_buffer_vertices_count + 18) > hardware_buffer_vertices_max) {
        hardware_buffer_vertices_max = hardware_buffer_vertices_max * 2 + 18;
        hardware_buffer_vertices = (float *)realloc(hardware_buffer_vertices, hardware_buffer_vertices_max * sizeof(float));
    }
    if ((hardware_buffer_texcoords_count + 12) > hardware_buffer_texcoords_max) {
        hardware_buffer_texcoords_max = hardware_buffer_texcoords_max * 2 + 12;
        hardware_buffer_texcoords = (float *)realloc(hardware_buffer_texcoords, hardware_buffer_texcoords_max * sizeof(float));
    }

    // clockwise
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y2;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y2f;

    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y2;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y2f;
}

void hardware_img_tri2d(float dst_x1, float dst_y1, float dst_x2, float dst_y2, float dst_x3, float dst_y3, int32_t src_img, int32_t dst_img, float src_x1,
                        float src_y1, float src_x2, float src_y2, float src_x3, float src_y3, int32_t use_alpha, int32_t smooth) {
    if (dst_img < 0)
        dst_img = 0; // both layers render to the primary context

    set_render_dest(dst_img);
    set_view(VIEW_MODE__2D);

    if (dst_img) {
        static hardware_img_struct *dst_hardware_img;
        dst_hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, dst_img);

        static int32_t dst_w, dst_h;
        dst_w = dst_hardware_img->w;
        dst_h = dst_hardware_img->h;
        // NON-SEAMLESS adjustments: Extend rhs/bottom row to fill extra pixel space
        // calculate extents
        int32_t rx1;
        int32_t rx2;
        rx1 = dst_x1;
        if (dst_x2 < rx1) rx1 = dst_x2;
        if (dst_x3 < rx1) rx1 = dst_x3;
        rx2 = dst_x1;
        if (dst_x2 > rx2) rx2 = dst_x2;
        if (dst_x3 > rx2) rx2 = dst_x3;
        float xr; // the multiplier for where we should be (1=no change)
        if (rx1 == rx2) {
            xr = 1.0f;
        } else {
            xr = ((float)rx2 - (float)rx1 + 1.0) / ((float)rx2 - (float)rx1);
        }
        int32_t ry1;
        int32_t ry2;
        ry1 = dst_y1;
        if (dst_y2 < ry1) ry1 = dst_y2;
        if (dst_y3 < ry1) ry1 = dst_y3;
        ry2 = dst_y1;
        if (dst_y2 > ry2) ry2 = dst_y2;
        if (dst_y3 > ry2) ry2 = dst_y3;
        float yr; // the multiplier for where we should be (1=no change)
        if (ry1 == ry2) {
            yr = 1.0f;
        } else {
            yr = ((float)ry2 - (float)ry1 + 1.0f) / ((float)ry2 - (float)ry1);
        }
        // apply multipliers so right-most and bottom-most rows will be filled
        static int32_t basex;
        basex = rx1;
        dst_x1 = qbr_float_to_long(((float)(dst_x1 - rx1)) * xr + (float)basex);
        dst_x2 = qbr_float_to_long(((float)(dst_x2 - rx1)) * xr + (float)basex);
        dst_x3 = qbr_float_to_long(((float)(dst_x3 - rx1)) * xr + (float)basex);
        static int32_t basey;
        basey = ry1;
        dst_y1 = qbr_float_to_long(((float)(dst_y1 - ry1)) * yr + (float)basey);
        dst_y2 = qbr_float_to_long(((float)(dst_y2 - ry1)) * yr + (float)basey);
        dst_y3 = qbr_float_to_long(((float)(dst_y3 - ry1)) * yr + (float)basey);

    } else { // dest is 0
        static int32_t dst_w, dst_h;
        dst_w = environment__window_width;
        dst_h = environment__window_height;
        // NON-SEAMLESS adjustments: Extend rhs/bottom row to fill extra pixel space
        // calculate extents
        int32_t rx1;
        int32_t rx2;
        rx1 = dst_x1;
        if (dst_x2 < rx1) rx1 = dst_x2;
        if (dst_x3 < rx1) rx1 = dst_x3;
        rx2 = dst_x1;
        if (dst_x2 > rx2) rx2 = dst_x2;
        if (dst_x3 > rx2) rx2 = dst_x3;
        float xr; // the multiplier for where we should be (1=no change)
        if (rx1 == rx2) {
            xr = 1.0f;
        } else {
            xr = ((float)rx2 - (float)rx1 + 1.0) / ((float)rx2 - (float)rx1);
        }
        int32_t ry1;
        int32_t ry2;
        ry1 = dst_y1;
        if (dst_y2 < ry1) ry1 = dst_y2;
        if (dst_y3 < ry1) ry1 = dst_y3;
        ry2 = dst_y1;
        if (dst_y2 > ry2) ry2 = dst_y2;
        if (dst_y3 > ry2) ry2 = dst_y3;
        float yr; // the multiplier for where we should be (1=no change)
        if (ry1 == ry2) {
            yr = 1.0f;
        } else {
            yr = ((float)ry2 - (float)ry1 + 1.0f) / ((float)ry2 - (float)ry1);
        }
        // apply multipliers so right-most and bottom-most rows will be filled
        static int32_t basex;
        basex = qbr_float_to_long(((float)(rx1)) * environment_2d__screen_x_scale + (float)environment_2d__screen_x1);
        dst_x1 = basex + qbr_float_to_long(((float)(dst_x1 - rx1)) * environment_2d__screen_x_scale * xr);
        dst_x2 = basex + qbr_float_to_long(((float)(dst_x2 - rx1)) * environment_2d__screen_x_scale * xr);
        dst_x3 = basex + qbr_float_to_long(((float)(dst_x3 - rx1)) * environment_2d__screen_x_scale * xr);
        static int32_t basey;
        basey = qbr_float_to_long(((float)(ry1)) * environment_2d__screen_y_scale + (float)environment_2d__screen_y1);
        dst_y1 = basey + qbr_float_to_long(((float)(dst_y1 - ry1)) * environment_2d__screen_y_scale * yr);
        dst_y2 = basey + qbr_float_to_long(((float)(dst_y2 - ry1)) * environment_2d__screen_y_scale * yr);
        dst_y3 = basey + qbr_float_to_long(((float)(dst_y3 - ry1)) * environment_2d__screen_y_scale * yr);
    }

    set_render_source(src_img);

    static hardware_img_struct *src_hardware_img;
    static int32_t src_h, src_w;
    src_hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, src_img);
    src_h = src_hardware_img->h;
    src_w = src_hardware_img->w;

    if (smooth == 0) {
        set_smooth(SMOOTH_MODE__DONT_SMOOTH, SMOOTH_MODE__DONT_SMOOTH);
    }
    if (smooth == 1) {
        set_smooth(SMOOTH_MODE__SMOOTH, SMOOTH_MODE__SMOOTH);
    }
    if (smooth == 2) {
        set_smooth(SMOOTH_MODE__SMOOTH, SMOOTH_MODE__DONT_SMOOTH);
    }
    if (smooth == 3) {
        set_smooth(SMOOTH_MODE__DONT_SMOOTH, SMOOTH_MODE__SMOOTH);
    }

    set_texture_wrap(TEXTURE_WRAP_MODE__WRAP);

    if (use_alpha) {
        set_alpha(ALPHA_MODE__BLEND);
    } else {
        set_alpha(ALPHA_MODE__DONT_BLEND);
    }

    set_depthbuffer(DEPTHBUFFER_MODE__OFF);
    set_cull_mode(CULL_MODE__NONE);

    if (src_hardware_img->source_state.PO2_fix) {
        src_w = src_hardware_img->PO2_w;
        src_h = src_hardware_img->PO2_h;
    }

    // calc source texture co-ordinates
    static float x1f, y1f, x2f, y2f, x3f, y3f;
    x1f = ((float)src_x1 + 0.5f) / (float)src_w;
    x2f = ((float)src_x2 + 0.5f) / (float)src_w;
    x3f = ((float)src_x3 + 0.5f) / (float)src_w;
    y1f = ((float)src_y1 + 0.5f) / (float)src_h;
    y2f = ((float)src_y2 + 0.5f) / (float)src_h;
    y3f = ((float)src_y3 + 0.5f) / (float)src_h;

    // expand buffers if necessary
    if ((hardware_buffer_vertices_count + 9) > hardware_buffer_vertices_max) {
        hardware_buffer_vertices_max = hardware_buffer_vertices_max * 2 + 9;
        hardware_buffer_vertices = (float *)realloc(hardware_buffer_vertices, hardware_buffer_vertices_max * sizeof(float));
    }
    if ((hardware_buffer_texcoords_count + 6) > hardware_buffer_texcoords_max) {
        hardware_buffer_texcoords_max = hardware_buffer_texcoords_max * 2 + 6;
        hardware_buffer_texcoords = (float *)realloc(hardware_buffer_texcoords, hardware_buffer_texcoords_max * sizeof(float));
    }

    // clockwise
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x3;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y3;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x3f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y3f;
}

void clear_depthbuffer(int32_t dst_img) {
    hardware_buffer_flush();
    if (dst_img < 0)
        dst_img = 0; // both layers render to the primary context
    set_render_dest(dst_img);
    if (dst_img > 0) {
        hardware_img_requires_depthbuffer((hardware_img_struct *)list_get(hardware_img_handles, dst_img));
    }
    glClear(GL_DEPTH_BUFFER_BIT);
}

void hardware_img_tri3d(float dst_x1, float dst_y1, float dst_z1, float dst_x2, float dst_y2, float dst_z2, float dst_x3, float dst_y3, float dst_z3,
                        int32_t src_img, int32_t dst_img, float src_x1, float src_y1, float src_x2, float src_y2, float src_x3, float src_y3, int32_t use_alpha,
                        int32_t smooth, int32_t cull_mode, int32_t depthbuffer_mode) {
    if (dst_img < 0)
        dst_img = 0; // both layers render to the primary context

    set_render_dest(dst_img);
    set_view(VIEW_MODE__3D);

    if (dst_img) {
        static hardware_img_struct *dst_hardware_img;
        dst_hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, dst_img);
        hardware_img_requires_depthbuffer(dst_hardware_img);
    } else { // dest is 0
    }

    set_render_source(src_img);

    static hardware_img_struct *src_hardware_img;
    static int32_t src_h, src_w;
    src_hardware_img = (hardware_img_struct *)list_get(hardware_img_handles, src_img);
    src_h = src_hardware_img->h;
    src_w = src_hardware_img->w;

    if (smooth == 0) {
        set_smooth(SMOOTH_MODE__DONT_SMOOTH, SMOOTH_MODE__DONT_SMOOTH);
    }
    if (smooth == 1) {
        set_smooth(SMOOTH_MODE__SMOOTH, SMOOTH_MODE__SMOOTH);
    }
    if (smooth == 2) {
        set_smooth(SMOOTH_MODE__SMOOTH, SMOOTH_MODE__DONT_SMOOTH);
    }
    if (smooth == 3) {
        set_smooth(SMOOTH_MODE__DONT_SMOOTH, SMOOTH_MODE__SMOOTH);
    }

    set_texture_wrap(TEXTURE_WRAP_MODE__WRAP);

    if (use_alpha) {
        set_alpha(ALPHA_MODE__BLEND);
    } else {
        set_alpha(ALPHA_MODE__DONT_BLEND);
    }

    set_depthbuffer(depthbuffer_mode);

    // on frame buffers the 3D perspective is flipped vertically reversing the cull direction
    if (dst_img > 0) {
        if (cull_mode == CULL_MODE__CLOCKWISE_ONLY) {
            cull_mode = CULL_MODE__ANTICLOCKWISE_ONLY;
        } else {
            if (cull_mode == CULL_MODE__ANTICLOCKWISE_ONLY)
                cull_mode = CULL_MODE__CLOCKWISE_ONLY;
        }
    }

    set_cull_mode(cull_mode);

    if (src_hardware_img->source_state.PO2_fix) {
        src_w = src_hardware_img->PO2_w;
        src_h = src_hardware_img->PO2_h;
    }

    // calc source texture co-ordinates
    static float x1f, y1f, x2f, y2f, x3f, y3f;
    x1f = ((float)src_x1 + 0.5f) / (float)src_w;
    x2f = ((float)src_x2 + 0.5f) / (float)src_w;
    x3f = ((float)src_x3 + 0.5f) / (float)src_w;
    y1f = ((float)src_y1 + 0.5f) / (float)src_h;
    y2f = ((float)src_y2 + 0.5f) / (float)src_h;
    y3f = ((float)src_y3 + 0.5f) / (float)src_h;

    // expand buffers if necessary
    if ((hardware_buffer_vertices_count + 9) > hardware_buffer_vertices_max) {
        hardware_buffer_vertices_max = hardware_buffer_vertices_max * 2 + 9;
        hardware_buffer_vertices = (float *)realloc(hardware_buffer_vertices, hardware_buffer_vertices_max * sizeof(float));
    }
    if ((hardware_buffer_texcoords_count + 6) > hardware_buffer_texcoords_max) {
        hardware_buffer_texcoords_max = hardware_buffer_texcoords_max * 2 + 6;
        hardware_buffer_texcoords = (float *)realloc(hardware_buffer_texcoords, hardware_buffer_texcoords_max * sizeof(float));
    }

    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_z1;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_z2;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_x3;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_y3;
    hardware_buffer_vertices[hardware_buffer_vertices_count++] = dst_z3;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y1f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y2f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = x3f;
    hardware_buffer_texcoords[hardware_buffer_texcoords_count++] = y3f;
}

#endif // QB64_GUI

// note: only to be used by user functions, not internal functions
hardware_img_struct *get_hardware_img(int32_t handle) {
    hardware_img_struct *img;
    if (handle < HARDWARE_IMG_HANDLE_OFFSET || handle >= SOFTWARE_IMG_HANDLE_MIN)
        return NULL;
    img = (hardware_img_struct *)list_get(hardware_img_handles, handle - HARDWARE_IMG_HANDLE_OFFSET);
    if (img == NULL)
        return NULL;
    if (!img->valid)
        return NULL;
    return img;
}

int32_t get_hardware_img_index(int32_t handle) {
    return handle - HARDWARE_IMG_HANDLE_OFFSET;
}
