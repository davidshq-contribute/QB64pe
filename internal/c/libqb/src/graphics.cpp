//----------------------------------------------------------------------------------------------------------------------
// QB64-PE graphics support
//----------------------------------------------------------------------------------------------------------------------

#include "graphics.h"
#include "error_handle.h"
#include "libqb-common.h"
#include "qblist.h"
#include "rounding.h"
#include "../../os.h"
#include "qbs.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "cmem.h"

#ifdef QB64_WINDOWS
#include <windows.h>
#endif

// External functions from libqb.cpp
void flush_old_hardware_commands();
void validatepage(int32_t pageNumber);
int32_t imgnew(int32_t x, int32_t y, int32_t bpp);
int32_t newimg();
int32_t freeimg(uint32_t i);
void sub__font(int32_t f, int32_t i, int32_t passed);
int32_t new_hardware_img(int32_t x, int32_t y, uint32_t *pixels, int32_t flags);
hardware_img_struct *get_hardware_img(int32_t handle);
int32_t get_hardware_img_index(int32_t handle);

// Global variables from libqb.cpp
extern list *hardware_img_handles;
extern int32_t HARDWARE_IMG_HANDLE_OFFSET;
extern list *hardware_graphics_command_handles;
extern int64_t display_frame_order_next;
extern int32_t last_hardware_command_added;
extern int32_t first_hardware_command;
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

// External helper functions for drawing primitives (remain in libqb.cpp)
void pset(int32_t x, int32_t y, uint32_t col);
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

uint32 point(int32 x, int32 y) { // does not clip!
    if (read_page->bytes_per_pixel == 1) {
        return read_page->offset[y * read_page->width + x] & read_page->mask;
    } else {
        return read_page->offset32[y * read_page->width + x];
    }
    return NULL;
}

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
