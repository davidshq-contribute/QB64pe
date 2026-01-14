//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Color & Palette Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "../../os.h"

#include <cstdlib>
#include <cstring>
#include <utility>

#include "error_handle.h"
#include "graphics.h"

#include "color.h"

// External references from libqb.cpp
extern void validatepage(int32 i);
extern int32 *page;
extern int32 nextimg;
extern img_struct *img;
extern img_struct *write_page;
extern int32 write_page_index;
extern int32 read_page_index;

//----------------------------------------------------------------------------------------------------------------------
// Color matching - finds closest palette entry for given RGB values
//----------------------------------------------------------------------------------------------------------------------

uint32 matchcol(int32 r, int32 g, int32 b) {
    static int32 v, v2, n, n2, best, c;
    static int32 *p;
    p = (int32 *)write_page->pal;
    if (write_page->text)
        n2 = 16;
    else
        n2 = write_page->mask + 1;
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
        if (write_page->bytes_per_pixel == 4) {
            return (r << 16) + (g << 8) + b | 0xFF000000;
        } else { //==4
            return matchcol(r, g, b);
        } //==4
    } // passed
} // rgb

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
        if (write_page->bytes_per_pixel == 4) {
            return (a << 24) + (r << 16) + (g << 8) + b;
        } else { //==4
            if ((!a) && (write_page->transparent_color != -1))
                return write_page->transparent_color;
            return matchcol(r, g, b);
        } //==4
    } // passed
} // rgba

//----------------------------------------------------------------------------------------------------------------------
// Color channel extraction
//----------------------------------------------------------------------------------------------------------------------

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
        if (write_page->bytes_per_pixel == 4) {
            return col >> 24;
        } else { //==4
            if ((col < 0) || (col > (write_page->mask))) {
                error(5);
                return 0;
            }
            if (write_page->transparent_color == (int32)col)
                return 0;
            return 255;
        } //==4
    } // passed
}

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
        if (write_page->bytes_per_pixel == 4) {
            return col >> 16 & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (write_page->mask))) {
                error(5);
                return 0;
            }
            return write_page->pal[col] >> 16 & 0xFF;
        } //==4
    } // passed
}

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
        if (write_page->bytes_per_pixel == 4) {
            return col >> 8 & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (write_page->mask))) {
                error(5);
                return 0;
            }
            return write_page->pal[col] >> 8 & 0xFF;
        } //==4
    } // passed
}

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
        if (write_page->bytes_per_pixel == 4) {
            return col & 0xFF;
        } else { //==4
            if ((col < 0) || (col > (write_page->mask))) {
                error(5);
                return 0;
            }
            return write_page->pal[col] & 0xFF;
        } //==4
    } // passed
}

//----------------------------------------------------------------------------------------------------------------------
// Clear/transparent color
//----------------------------------------------------------------------------------------------------------------------

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
        i = write_page_index;
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
        i = write_page_index;
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
        i = write_page_index;
    }
    return img[i].color;
}

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
        i = write_page_index;
    }
    return img[i].background_color;
}

//----------------------------------------------------------------------------------------------------------------------
// Palette operations
//----------------------------------------------------------------------------------------------------------------------

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
        i = write_page_index;
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
        i = write_page_index;
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
        i = read_page_index;
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
        i = write_page_index;
    }
    if (!img[i].pal) {
        error(5);
        return;
    }
    std::swap(i, i2);
    memcpy(img[i2].pal, img[i].pal, 1024);
}
