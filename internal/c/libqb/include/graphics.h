//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Graphics Module
//  Graphics rendering, drawing primitives, and image management
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_GRAPHICS_H
#define INCLUDE_LIBQB_GRAPHICS_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include <stdint.h>

struct img_struct {
    void *lock_offset;
    int64_t lock_id;
    uint8_t valid;   // 0,1 0=invalid
    uint8_t text;    // if set, surface is a text surface
    uint8_t console; // dummy surface to absorb unimplemented console functionality
    uint16_t width, height;
    uint8_t bytes_per_pixel;  // 1,2,4
    uint8_t bits_per_pixel;   // 1,2,4,8,16(text),32
    uint32_t mask;            // 1,3,0xF,0xFF,0xFFFF,0xFFFFFFFF
    uint16_t compatible_mode; // 0,1,2,7,8,9,10,11,12,13,32,256
    uint32_t color, background_color, draw_color;
    uint32_t font;               // 8,14,16,?
    int16_t top_row, bottom_row; // VIEW PRINT settings, unique (as in QB) to each "page"
    int16_t cursor_x, cursor_y;  // unique (as in QB) to each "page"
    uint8_t cursor_show, cursor_firstvalue, cursor_lastvalue;

    union {
        uint8_t *offset;
        uint32_t *offset32;
    };

    uint32_t flags;
    uint32_t *pal;
    int32_t transparent_color; //-1 means no color is transparent
    uint8_t alpha_disabled;
    uint8_t holding_cursor;
    uint8_t print_mode;
    // BEGIN apm ('active page migration')
    // everything between apm points is migrated during active page changes
    // note: apm data is only relevant to graphics modes
    uint8_t apm_p1;
    int32_t view_x1, view_y1, view_x2, view_y2;
    int32_t view_offset_x, view_offset_y;
    float x, y;
    uint8_t clipping_or_scaling;
    float scaling_x, scaling_y, scaling_offset_x, scaling_offset_y;
    float window_x1, window_y1, window_x2, window_y2;
    double draw_ta;
    double draw_scale;
    uint8_t apm_p2;
    // END apm
};

// img_struct flags
#define IMG_FREEPAL 1 // free palette data before freeing image
#define IMG_SCREEN 2  // img is linked to other screen pages
#define IMG_FREEMEM 4 // if set, it means memory must be freed

// used by HSB/RGB color conversion routines
struct hsb_color
{
    double h; // [0,360] hue
    double s; // [0,1]   saturation
    double b; // [0,1]   brightness
};
struct rgb_color
{
    double r; // [0,1] red
    double g; // [0,1] green
    double b; // [0,1] blue
};

/********** Render State **********/
/*
    Apart from 'glTexParameter' based settings (with are texture specific)
    all other OpenGL states are global.
    This means when switching between dest FBOs a complete state change is inevitable.
*/
struct RENDER_STATE_DEST { // could be the primary render target or a FBO
    int32_t ignore;        // at present no relevant states appear to be FBO specific
};

struct RENDER_STATE_SOURCE { // texture states
    int32_t smooth_stretched;
    int32_t smooth_shrunk;
    int32_t texture_wrap;
    int32_t PO2_fix;
};

struct RENDER_STATE_GLOBAL { // settings not bound to specific source/target
    RENDER_STATE_DEST *dest;
    RENDER_STATE_SOURCE *source;
    int32_t dest_handle;
    int32_t source_handle;
    int32_t view_mode;
    int32_t use_alpha;
    int32_t depthbuffer_mode;
    int32_t cull_mode;
};

#define VIEW_MODE__UNKNOWN 0
#define VIEW_MODE__2D 1
#define VIEW_MODE__3D 2
#define VIEW_MODE__RESET 3
#define ALPHA_MODE__UNKNOWN -1
#define ALPHA_MODE__DONT_BLEND 0
#define ALPHA_MODE__BLEND 1
#define TEXTURE_WRAP_MODE__UNKNOWN -1
#define TEXTURE_WRAP_MODE__DONT_WRAP 0
#define TEXTURE_WRAP_MODE__WRAP 1
#define SMOOTH_MODE__UNKNOWN -1
#define SMOOTH_MODE__DONT_SMOOTH 0
#define SMOOTH_MODE__SMOOTH 1
#define PO2_FIX__OFF 0
#define PO2_FIX__EXPANDED 1
#define PO2_FIX__MIPMAPPED 2

#define DEPTHBUFFER_MODE__UNKNOWN -1
#define DEPTHBUFFER_MODE__OFF 0
#define DEPTHBUFFER_MODE__ON 1
#define DEPTHBUFFER_MODE__LOCKED 2
#define DEPTHBUFFER_MODE__CLEAR 3
#define CULL_MODE__UNKNOWN -1
#define CULL_MODE__NONE 0
#define CULL_MODE__CLOCKWISE_ONLY 1
#define CULL_MODE__ANTICLOCKWISE_ONLY 2
/********** Render State **********/

#define INVALID_HARDWARE_HANDLE -1

struct hardware_img_struct {
    int32_t w;
    int32_t h;
    int32_t texture_handle;          // if 0, imports from software_pixel_buffer automatically
    int32_t dest_context_handle;     // used when rendering other images onto this image
    int32_t depthbuffer_handle;      // generated when 3D commands are called
    int32_t pending_commands;        // incremented with each command, decremented after command is processed
    int32_t remove;                  // if =1, free immediately after all pending commands are processed
    uint32_t *software_pixel_buffer; // if NULL, generates a blank texture
    int32_t alpha_disabled;          // changed by _BLEND/_DONTBLEND commands
    int32_t depthbuffer_mode;        // changed by _DEPTHBUFFER
    int32_t valid;
    RENDER_STATE_SOURCE source_state;
    RENDER_STATE_DEST dest_state;
    int32_t PO2_w; // if PO2_FIX__EXPANDED/MIPMAPPED, these are the texture size
    int32_t PO2_h;
};

struct hardware_graphics_command_struct {
    int64_t order;        // which _DISPLAY event to bind the operation to
    int32_t next_command; // the handle of the next hardware_graphics_command of the same display-order, of 0 if last
    int64_t command;      // the command type, actually a set of bit flags

    // Bit 00: Decimal value 000001: _PUTIMAGE
    union {
        int32_t option;
        int32_t src_img; // MUST be a hardware handle
    };

    union {
        int32_t dst_img; // MUST be a hardware handle or 0 for the default 2D rendering context
        int32_t target;
    };

    float src_x1;
    float src_y1;
    float src_x2;
    float src_y2;
    float src_x3;
    float src_y3;
    float dst_x1;
    float dst_y1;
    float dst_z1;
    float dst_x2;
    float dst_y2;
    float dst_z2;
    float dst_x3;
    float dst_y3;
    float dst_z3;
    int32_t smooth; // 0 or 1 (whether to apply texture filtering)
    int32_t cull_mode;
    int32_t depthbuffer_mode;
    int32_t use_alpha; // 0 or 1 (whether to refer to the alpha component of pixel values)
    int32_t remove;
};

#define HARDWARE_GRAPHICS_COMMAND__PUTIMAGE 1
#define HARDWARE_GRAPHICS_COMMAND__FREEIMAGE_REQUEST 2
#define HARDWARE_GRAPHICS_COMMAND__FREEIMAGE 3
#define HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE 4
#define HARDWARE_GRAPHICS_COMMAND__MAPTRIANGLE3D 5
#define HARDWARE_GRAPHICS_COMMAND__CLEAR_DEPTHBUFFER 6

// ============================================================================
// FORWARD DECLARATIONS
// ============================================================================

struct qbs;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// HSB/RGB Color Conversion
// These functions convert between HSB (Hue, Saturation, Brightness) and RGB color formats

/// Creates an RGB color from HSB values.
/// @param hue Hue value (0-360 degrees)
/// @param sat Saturation (0.0-1.0)
/// @param bri Brightness (0.0-1.0)
/// @returns 32-bit RGB color value
uint32_t func__hsb32(double hue, double sat, double bri);

/// Creates an RGBA color from HSB values with alpha.
/// @param hue Hue value (0-360 degrees)
/// @param sat Saturation (0.0-1.0)
/// @param bri Brightness (0.0-1.0)
/// @param alf Alpha channel (0.0-1.0)
/// @returns 32-bit RGBA color value
uint32_t func__hsba32(double hue, double sat, double bri, double alf);

/// Extracts the hue component from an RGB color.
/// @param argb 32-bit color value
/// @returns Hue value (0-360 degrees)
double func__hue32(uint32_t argb);

/// Extracts the saturation component from an RGB color.
/// @param argb 32-bit color value
/// @returns Saturation value (0.0-1.0)
double func__sat32(uint32_t argb);

/// Extracts the brightness component from an RGB color.
/// @param argb 32-bit color value
/// @returns Brightness value (0.0-1.0)
double func__bri32(uint32_t argb);

// 3D Graphics Functions

/// Controls depth buffer settings for 3D rendering.
/// @param options Depth buffer mode flags
/// @param dst Destination image handle (optional)
/// @param passed Parameter passing flags
void sub__depthbuffer(int32_t options, int32_t dst, int32_t passed);

/// Maps a triangle from source coordinates to destination coordinates (supports 3D).
/// @param cull_options Face culling options
/// @param sx1, sy1 Source triangle first vertex
/// @param sx2, sy2 Source triangle second vertex
/// @param sx3, sy3 Source triangle third vertex
/// @param si Source image handle
/// @param dx1, dy1, dz1 Destination triangle first vertex
/// @param dx2, dy2, dz2 Destination triangle second vertex
/// @param dx3, dy3, dz3 Destination triangle third vertex
/// @param di Destination image handle
/// @param smooth_options Texture smoothing options
/// @param passed Parameter passing flags
void sub__maptriangle(int32_t cull_options, float sx1, float sy1, float sx2, float sy2, float sx3, float sy3, int32_t si, float dx1, float dy1, float dz1,
                      float dx2, float dy2, float dz2, float dx3, float dy3, float dz3, int32_t di, int32_t smooth_options, int32_t passed);

// ============================================================================
// DRAWING PRIMITIVES
// ============================================================================

/// Sets a pixel with clipping (internal helper function).
/// @param x X coordinate
/// @param y Y coordinate
/// @param col Color value
void pset_and_clip(int32_t x, int32_t y, uint32_t col);

/// Fills a rectangular area with a color (internal helper).
/// @param x1f, y1f Top-left corner coordinates
/// @param x2f, y2f Bottom-right corner coordinates
/// @param col Fill color
void qb32_boxfill(float x1f, float y1f, float x2f, float y2f, uint32_t col);

/// Draws a line (internal helper).
/// @param x1f, y1f Start point coordinates
/// @param x2f, y2f End point coordinates
/// @param col Line color
/// @param style Line style pattern
void qb32_line(float x1f, float y1f, float x2f, float y2f, uint32_t col, uint32_t style);

/// Draws a line between two points.
/// @param x1, y1 Start point coordinates
/// @param x2, y2 End point coordinates
/// @param col Line color
/// @param bf Box fill flag (if non-zero, draws a filled box instead)
/// @param style Line style pattern
/// @param passed Parameter passing flags
void sub_line(float x1, float y1, float x2, float y2, uint32_t col, int32_t bf, uint32_t style, int32_t passed);

/// Draws a circle or arc.
/// @param x, y Center coordinates
/// @param r Radius
/// @param col Circle color
/// @param start Start angle (for arcs)
/// @param end End angle (for arcs)
/// @param aspect Aspect ratio
/// @param passed Parameter passing flags
void sub_circle(double x, double y, double r, uint32_t col, double start, double end, double aspect, int32_t passed);

/// Sets a pixel to a specific color.
/// @param x, y Pixel coordinates
/// @param col Color value
/// @param passed Parameter passing flags
void sub_pset(float x, float y, uint32_t col, int32_t passed);

/// Resets a pixel to the background color.
/// @param x, y Pixel coordinates
/// @param col Color value (typically ignored, uses background)
/// @param passed Parameter passing flags
void sub_preset(float x, float y, uint32_t col, int32_t passed);

/// Fills an area starting from a point with a solid color.
/// @param x, y Starting point coordinates
/// @param fillcol Fill color
/// @param bordercol Border color (stops filling at this color)
/// @param backgroundstr Background pattern string (optional)
/// @param passed Parameter passing flags
void sub_paint(float x, float y, uint32_t fillcol, uint32_t bordercol, qbs *backgroundstr, int32_t passed);

/// Fills an area starting from a point with a pattern.
/// @param x, y Starting point coordinates
/// @param fillstr Fill pattern string
/// @param bordercol Border color (stops filling at this color)
/// @param backgroundstr Background pattern string (optional)
/// @param passed Parameter passing flags
void sub_paint(float x, float y, qbs *fillstr, uint32_t bordercol, qbs *backgroundstr, int32_t passed);

/// Reads a pixel color value (internal helper, no clipping).
/// @param x, y Pixel coordinates
/// @returns Color value at the specified pixel
uint32_t point(int32_t x, int32_t y);

/// Gets the color value of a pixel (QB64 POINT function).
/// @param x, y Pixel coordinates
/// @param passed Parameter passing flags
/// @returns Color value or other pixel attributes
double func_point(float x, float y, int32_t passed);

// ============================================================================
// IMAGE MANAGEMENT
// ============================================================================

/// Creates a new image buffer.
/// @param x Image width in pixels
/// @param y Image height in pixels
/// @param bpp Bits per pixel (1, 2, 4, 8, 16, or 32)
/// @param passed Parameter passing flags
/// @returns Image handle, or 0 on error
int32_t func__newimage(int32_t x, int32_t y, int32_t bpp, int32_t passed);

/// Creates a copy of an existing image.
/// @param i Source image handle
/// @param mode Copy mode (0=software, 1=hardware, etc.)
/// @param passed Parameter passing flags
/// @returns New image handle, or 0 on error
int32_t func__copyimage(int32_t i, int32_t mode, int32_t passed);

/// Frees an image and releases its resources.
/// @param i Image handle to free
/// @param passed Parameter passing flags
void sub__freeimage(int32_t i, int32_t passed);

/// Frees all images (cleanup function).
void freeallimages();

/// Sets the source image for drawing operations.
/// @param i Source image handle
void sub__source(int32_t i);

/// Sets the destination image for drawing operations.
/// @param i Destination image handle
void sub__dest(int32_t i);

/// Gets the current source image handle.
/// @returns Source image handle
int32_t func__source();

/// Gets the current destination image handle.
/// @returns Destination image handle
int32_t func__dest();

/// Gets the current display image handle.
/// @returns Display image handle
int32_t func__display();

/// Enables alpha blending for an image.
/// @param i Image handle
/// @param passed Parameter passing flags
void sub__blend(int32_t i, int32_t passed);

/// Disables alpha blending for an image.
/// @param i Image handle
/// @param passed Parameter passing flags
void sub__dontblend(int32_t i, int32_t passed);

/// Sets alpha channel values for specific colors.
/// @param a Alpha value (0-255)
/// @param c Color to set alpha for
/// @param c2 Optional second color range
/// @param i Image handle
/// @param passed Parameter passing flags
void sub__setalpha(int32_t a, uint32_t c, uint32_t c2, int32_t i, int32_t passed);

/// Gets the width of an image.
/// @param i Image handle
/// @param passed Parameter passing flags
/// @returns Image width in pixels
int32_t func__width(int32_t i, int32_t passed);

/// Gets the height of an image.
/// @param i Image handle
/// @param passed Parameter passing flags
/// @returns Image height in pixels
int32_t func__height(int32_t i, int32_t passed);

/// Gets the pixel size (bits per pixel) of an image.
/// @param i Image handle
/// @param passed Parameter passing flags
/// @returns Bits per pixel
int32_t func__pixelsize(int32_t i, int32_t passed);

/// Checks if alpha blending is enabled for an image.
/// @param i Image handle
/// @param passed Parameter passing flags
/// @returns Non-zero if blending is enabled
int32_t func__blend(int32_t i, int32_t passed);

// ============================================================================
// DRAW COMMAND
// ============================================================================

/// Executes DRAW command string (turtle graphics).
/// @param s DRAW command string
void sub_draw(qbs *s);

// ============================================================================
// IMAGE OPERATIONS
// ============================================================================

/// Copies a portion of a source image to a destination image.
/// @param f_dx1, f_dy1 Destination top-left coordinates
/// @param f_dx2, f_dy2 Destination bottom-right coordinates
/// @param src Source image handle
/// @param dst Destination image handle
/// @param f_sx1, f_sy1 Source top-left coordinates
/// @param f_sx2, f_sy2 Source bottom-right coordinates
/// @param passed Parameter passing flags
void sub__putimage(double f_dx1, double f_dy1, double f_dx2, double f_dy2, int32_t src, int32_t dst,
                   double f_sx1, double f_sy1, double f_sx2, double f_sy2, int32_t passed);

/// Gets pixel data from a rectangular area (QB64 GET command).
/// @param x1f, y1f Top-left corner coordinates
/// @param x2f, y2f Bottom-right corner coordinates
/// @param element Pointer to destination buffer
/// @param mask Color mask for pixel data
/// @param passed Parameter passing flags
void sub_graphics_get(float x1f, float y1f, float x2f, float y2f, void *element, uint32_t mask, int32_t passed);

/// Puts pixel data to a rectangular area (QB64 PUT command).
/// @param x1f, y1f Top-left corner coordinates
/// @param element Pointer to source pixel data
/// @param option PUT operation mode (PSET, PRESET, XOR, OR, AND)
/// @param mask Color mask for pixel data
/// @param passed Parameter passing flags
void sub_graphics_put(float x1f, float y1f, void *element, int32_t option, uint32_t mask, int32_t passed);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// This module provides comprehensive graphics functionality including:
// - Drawing primitives (lines, circles, boxes, pixels)
// - Image creation and management
// - HSB/RGB color conversion
// - 3D graphics support (depth buffer, triangle mapping)
// - Image copying and manipulation (GET/PUT operations)
// - Alpha blending and transparency control
// - Turtle graphics (DRAW command)
//
// The module uses both software and hardware rendering paths, with hardware
// rendering utilizing OpenGL for improved performance.

#endif // INCLUDE_LIBQB_GRAPHICS_H
