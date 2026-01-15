//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Color & Palette Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "color.h"
#include "error_handle.h"
#include "graphics.h"
#include "libqb_state.h"

#include "../../os.h"  // For int32 type definitions

#include <cstdlib>
#include <cstring>
#include <utility>

// External references from libqb.cpp
extern void validatepage(int32 i);
extern int32 *page;
extern int32 nextimg;
extern img_struct *img;

//----------------------------------------------------------------------------------------------------------------------
// Color matching - finds closest palette entry for given RGB values
//----------------------------------------------------------------------------------------------------------------------

/**
 * Finds the closest matching palette color index for the given RGB values on the write page.
 * 
 * Searches through the current write page's palette to find the color entry that most closely
 * matches the specified RGB values using Manhattan distance (sum of absolute differences).
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return Palette index of the closest matching color
 */
uint32 matchcol(int32 r, int32 g, int32 b) {
    static int32 v, v2, n, n2, best, c;
    static int32 *p;
    img_struct *wp = libqb_get_write_page();
    p = (int32 *)wp->pal;
    if (wp->text)
        n2 = 16;
    else
        n2 = wp->mask + 1;
    v = 1000;
    best = 0;
    for (n = 0; n < n2; n++) {
        c = *p++;
        v2 = abs(b - (c & 0xFF)) + abs(g - (c >> 8 & 0xFF)) + abs(r - (c >> 16 & 0xFF));
        if (v2 < v) {
            if (!v2)
                return n; // perfect match
            v = v2;
            best = n;
        }
    } // n
    return best;
}

/**
 * Finds the closest matching palette color index for the given RGB values on a specific image.
 * 
 * Searches through the specified image's palette to find the color entry that most closely
 * matches the specified RGB values using Manhattan distance (sum of absolute differences).
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param i Image index to search palette of
 * @return Palette index of the closest matching color
 */
uint32 matchcol(int32 r, int32 g, int32 b, int32 i) {
    static int32 v, v2, n, n2, best, c;
    static int32 *p;
    p = (int32 *)img[i].pal;
    if (img[i].text)
        n2 = 16;
    else
        n2 = img[i].mask + 1;
    v = 1000;
    best = 0;
    for (n = 0; n < n2; n++) {
        c = *p++;
        v2 = abs(b - (c & 0xFF)) + abs(g - (c >> 8 & 0xFF)) + abs(r - (c >> 16 & 0xFF));
        if (v2 < v) {
            if (!v2)
                return n; // perfect match
            v = v2;
            best = n;
        }
    } // n
    return best;
}

//----------------------------------------------------------------------------------------------------------------------
// RGB/RGBA color creation
//----------------------------------------------------------------------------------------------------------------------

/**
 * Creates an RGB color value from red, green, and blue components.
 * 
 * Clamps RGB values to the 0-255 range. For 32-bit images, returns a 32-bit ARGB color value.
 * For palette-based images, returns the closest matching palette index.
 * 
 * @param r Red component (clamped to 0-255)
 * @param g Green component (clamped to 0-255)
 * @param b Blue component (clamped to 0-255)
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return 32-bit ARGB color value or palette index, depending on image format
 */
uint32 func__rgb(int32 r, int32 g, int32 b, int32 i, int32 passed) {
    if (is_error_pending())
        return 0;
    if (r < 0)
        r = 0;
    if (r > 255)
        r = 255;
    if (g < 0)
        g = 0;
    if (g > 255)
        g = 255;
    if (b < 0)
        b = 0;
    if (b > 255)
        b = 255;
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
        if (img[i].bytes_per_pixel == 4) {
            return (r << 16) + (g << 8) + b | 0xFF000000;
        } else { //==4
            return matchcol(r, g, b, i);
        } //==4
    } else {
        img_struct *wp = libqb_get_write_page();
        if (wp->bytes_per_pixel == 4) {
            return (r << 16) + (g << 8) + b | 0xFF000000;
        } else { //==4
            return matchcol(r, g, b);
        } //==4
    } // passed
} // rgb

/**
 * Creates an RGBA color value from red, green, blue, and alpha components.
 * 
 * Clamps RGBA values to the 0-255 range. For 32-bit images, returns a 32-bit ARGB color value.
 * For palette-based images, returns the closest matching palette index or transparent color
 * if alpha is 0 and transparency is enabled.
 * 
 * @param r Red component (clamped to 0-255)
 * @param g Green component (clamped to 0-255)
 * @param b Blue component (clamped to 0-255)
 * @param a Alpha component (clamped to 0-255)
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return 32-bit ARGB color value or palette index, depending on image format
 */
uint32 func__rgba(int32 r, int32 g, int32 b, int32 a, int32 i, int32 passed) {
    if (is_error_pending())
        return 0;
    if (r < 0)
        r = 0;
    if (r > 255)
        r = 255;
    if (g < 0)
        g = 0;
    if (g > 255)
        g = 255;
    if (b < 0)
        b = 0;
    if (b > 255)
        b = 255;
    if (a < 0)
        a = 0;
    if (a > 255)
        a = 255;
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
        if (img[i].bytes_per_pixel == 4) {
            return (a << 24) + (r << 16) + (g << 8) + b;
        } else { //==4
            if ((!a) && (img[i].transparent_color != -1))
                return img[i].transparent_color;
            return matchcol(r, g, b, i);
        } //==4
    } else {
        img_struct *wp = libqb_get_write_page();
        if (wp->bytes_per_pixel == 4) {
            return (a << 24) + (r << 16) + (g << 8) + b;
        } else { //==4
            if ((!a) && (wp->transparent_color != -1))
                return wp->transparent_color;
            return matchcol(r, g, b);
        } //==4
    } // passed
} // rgba

//----------------------------------------------------------------------------------------------------------------------
// Color channel extraction
//----------------------------------------------------------------------------------------------------------------------

/**
 * Extracts the alpha channel value from a color.
 * 
 * For 32-bit images, returns the alpha component (bits 24-31) of the color.
 * For palette-based images, returns 255 for opaque colors or 0 for transparent colors.
 * 
 * @param col Color value to extract alpha from
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Alpha value (0-255) or 0 on error
 */
int32 func__alpha(uint32 col, int32 i, int32 passed) {
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
        if (img[i].bytes_per_pixel == 4) {
            return col >> 24;
        } else { //==4
            if ((col < 0) || (col > (img[i].mask))) {
                error(5);
                return 0;
            }
            if (img[i].transparent_color == (int32)col)
                return 0;
            return 255;
        } //==4
    } else {
        img_struct *wp = libqb_get_write_page();
        if (wp->bytes_per_pixel == 4) {
            return col >> 24;
        } else { //==4
            if ((col < 0) || (col > (wp->mask))) {
                error(5);
                return 0;
            }
            if (wp->transparent_color == (int32)col)
                return 0;
            return 255;
        } //==4
    } // passed
}

/**
 * Extracts the red channel value from a color.
 * 
 * For 32-bit images, returns the red component (bits 16-23) of the color.
 * For palette-based images, returns the red component from the palette entry.
 * 
 * @param col Color value to extract red from
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Red value (0-255) or 0 on error
 */
int32 func__red(uint32 col, int32 i, int32 passed) {
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
        if (img[i].bytes_per_pixel == 4) {
            return col >> 16 & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (img[i].mask))) {
                error(5);
                return 0;
            }
            return img[i].pal[col] >> 16 & 0xFF;
        } //==4
    } else {
        img_struct *wp = libqb_get_write_page();
        if (wp->bytes_per_pixel == 4) {
            return col >> 16 & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (wp->mask))) {
                error(5);
                return 0;
            }
            return wp->pal[col] >> 16 & 0xFF;
        } //==4
    } // passed
}

/**
 * Extracts the green channel value from a color.
 * 
 * For 32-bit images, returns the green component (bits 8-15) of the color.
 * For palette-based images, returns the green component from the palette entry.
 * 
 * @param col Color value to extract green from
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Green value (0-255) or 0 on error
 */
int32 func__green(uint32 col, int32 i, int32 passed) {
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
        if (img[i].bytes_per_pixel == 4) {
            return col >> 8 & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (img[i].mask))) {
                error(5);
                return 0;
            }
            return img[i].pal[col] >> 8 & 0xFF;
        } //==4
    } else {
        img_struct *wp = libqb_get_write_page();
        if (wp->bytes_per_pixel == 4) {
            return col >> 8 & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (wp->mask))) {
                error(5);
                return 0;
            }
            return wp->pal[col] >> 8 & 0xFF;
        } //==4
    } // passed
}

/**
 * Extracts the blue channel value from a color.
 * 
 * For 32-bit images, returns the blue component (bits 0-7) of the color.
 * For palette-based images, returns the blue component from the palette entry.
 * 
 * @param col Color value to extract blue from
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Blue value (0-255) or 0 on error
 */
int32 func__blue(uint32 col, int32 i, int32 passed) {
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
        if (img[i].bytes_per_pixel == 4) {
            return col & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (img[i].mask))) {
                error(5);
                return 0;
            }
            return img[i].pal[col] & 0xFF;
        } //==4
    } else {
        img_struct *wp = libqb_get_write_page();
        if (wp->bytes_per_pixel == 4) {
            return col & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (wp->mask))) {
                error(5);
                return 0;
            }
            return wp->pal[col] & 0xFF;
        } //==4
    } // passed
}

//----------------------------------------------------------------------------------------------------------------------
// Clear/transparent color
//----------------------------------------------------------------------------------------------------------------------

/**
 * Sets or clears the transparent color for an image.
 * 
 * For palette-based images, sets which palette index should be treated as transparent.
 * For 32-bit images, sets all pixels matching the specified color to fully transparent.
 * Use _NONE option to disable transparency.
 * 
 * @param c Color value to set as transparent (ignored if _NONE is specified)
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags: bit 0 = _NONE, bit 1 = color provided, bit 2 = image handle provided
 */
void sub__clearcolor(uint32 c, int32 i, int32 passed) {
    //--         _NONE->1       2       4
    // id.specialformat = "[{_NONE}][?][,?]"
    if (is_error_pending())
        return;
    static img_struct *im;
    static uint32 *lp, *last;
    if (passed & 4) {
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
        i = libqb_get_write_page_index();
    }
    im = &img[i];
    // text?
    if (im->text) {
        if ((passed & 1) && (!(passed & 2)))
            return; // you can disable clearcolor using _CLEARCOLOR _NONE in text modes
        error(5);
        return;
    }
    // palette?
    if (im->pal) {
        if (passed & 1) {
            if (passed & 2) {
                error(5);
                return;
            } // invalid options
            im->transparent_color = -1;
            return;
        }
        if (!(passed & 2)) {
            error(5);
            return;
        } // invalid options
        if (c > 255) {
            error(5);
            return;
        } // invalid color
        im->transparent_color = c;
        return;
    }
    // 32-bit? (alpha is ignored in this case)
    if (passed & 1) {
        if (passed & 2) {
            error(5);
            return;
        } // invalid options
        return; // no action
    }
    if (!(passed & 2)) {
        error(5);
        return;
    } // invalid options
    c &= 0xFFFFFF;
    last = im->offset32 + im->width * im->height;
    for (lp = im->offset32; lp < last; lp++) {
        if ((*lp & 0xFFFFFF) == c)
            *lp = c;
    }
    return;
}

/**
 * Gets the current transparent color index for an image.
 * 
 * Returns the palette index used for transparency in palette-based images,
 * or -1 if transparency is disabled, or 0 for 32-bit images.
 * 
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Transparent color index, -1 if disabled, or 0 on error/invalid mode
 */
int32 func__clearcolor(int32 i, int32 passed) {
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
        i = libqb_get_write_page_index();
    }
    if (img[i].text)
        return -1;
    if (img[i].compatible_mode == 32)
        return 0;
    return img[i].transparent_color;
}

//----------------------------------------------------------------------------------------------------------------------
// Default colors
//----------------------------------------------------------------------------------------------------------------------

/**
 * Gets the default foreground color for an image.
 * 
 * Returns the color value that is used as the default drawing color
 * for the specified image or write page.
 * 
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Default color value or 0 on error
 */
uint32 func__defaultcolor(int32 i, int32 passed) {
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
        i = libqb_get_write_page_index();
    }
    return img[i].color;
}

/**
 * Gets the background color for an image.
 * 
 * Returns the color value that is used as the background color
 * for the specified image or write page.
 * 
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return Background color value or 0 on error
 */
uint32 func__backgroundcolor(int32 i, int32 passed) {
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
        i = libqb_get_write_page_index();
    }
    return img[i].background_color;
}

//----------------------------------------------------------------------------------------------------------------------
// Palette operations
//----------------------------------------------------------------------------------------------------------------------

/**
 * Gets the color value for a specific palette entry.
 * 
 * Returns the 32-bit ARGB color value stored at the specified palette index.
 * Only works with palette-based images.
 * 
 * @param n Palette index (0-255)
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 * @return 32-bit ARGB color value or 0 on error
 */
uint32 func__palettecolor(int32 n, int32 i, int32 passed) {
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
        i = libqb_get_write_page_index();
    }
    if (!img[i].pal) {
        error(5);
        return 0;
    }
    if (n < 0 || n > 255) {
        error(5);
        return 0;
    } // out of range
    return img[i].pal[n] | 0xFF000000;
}

/**
 * Sets the color value for a specific palette entry.
 * 
 * Updates the palette entry at the specified index with the new color value.
 * Only works with palette-based images.
 * 
 * @param n Palette index (0-255)
 * @param c 32-bit ARGB color value to set
 * @param i Optional image/page handle (if passed parameter is set)
 * @param passed Bit flags indicating which parameters were provided
 */
void sub__palettecolor(int32 n, uint32 c, int32 i, int32 passed) {
    if (is_error_pending())
        return;
    if (passed) {
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
        i = libqb_get_write_page_index();
    }
    if (!img[i].pal) {
        error(5);
        return;
    }
    if (n < 0 || n > 255) {
        error(5);
        return;
    } // out of range
    img[i].pal[n] = c;
}

/**
 * Copies the palette from one image to another.
 * 
 * Copies all 256 palette entries (1024 bytes) from the source image
 * to the destination image. Both images must be palette-based.
 * 
 * @param i Source image/page handle (read page if not provided)
 * @param i2 Destination image/page handle (write page if not provided)
 * @param passed Bit flags indicating which parameters were provided
 */
void sub__copypalette(int32 i, int32 i2, int32 passed) {
    if (is_error_pending())
        return;
    if (passed & 1) {
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
        i = libqb_get_read_page_index();
    }
    if (!img[i].pal) {
        error(5);
        return;
    }
    std::swap(i, i2);
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
        i = libqb_get_write_page_index();
    }
    if (!img[i].pal) {
        error(5);
        return;
    }
    std::swap(i, i2);
    memcpy(img[i2].pal, img[i].pal, 1024);
}
