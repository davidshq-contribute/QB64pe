//-----------------------------------------------------------------------------------------------------
//    ___  ___   __ _ _  ___ ___   ___                       _    _ _
//   / _ \| _ ) / /| | || _ \ __| |_ _|_ __  __ _ __ _ ___  | |  (_) |__ _ _ __ _ _ _ _  _
//  | (_) | _ \/ _ \_  _|  _/ _|   | || '  \/ _` / _` / -_) | |__| | '_ \ '_/ _` | '_| || |
//   \__\_\___/\___/ |_||_| |___| |___|_|_|_\__,_\__, \___| |____|_|_.__/_| \__,_|_|  \_, |
//                                               |___/                                |__/
//
//  Powered by:
//      stb_image & stb_image_write (https://github.com/nothings/stb)
//      dr_pcx (https://github.com/mackron/dr_pcx)
//      nanosvg (https://github.com/memononen/nanosvg)
//      qoi (https://qoiformat.org)
//      pixelscalers (https://github.com/janert/pixelscalers)
//      mmpx (https://github.com/ITotalJustice/mmpx)
//
//-----------------------------------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include <cmath>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "logging.h"

/**
 * @file image.h
 * @brief Image loading, saving, and color manipulation functions for QB64-PE
 * 
 * This header provides functions for loading and saving images in various formats,
 * as well as color manipulation utilities. Supports multiple image formats through
 * various libraries (stb_image, dr_pcx, nanosvg, qoi, etc.).
 */

/**
 * @name Image Logging Macros
 * @brief Macros for logging image-related messages
 */
///@{
#define image_log_trace(...) \
    libqb_log_with_scope_trace(logscope::Image, __VA_ARGS__)

#define image_log_info(...) \
    libqb_log_with_scope_info(logscope::Image, __VA_ARGS__)

#define image_log_warn(...) \
    libqb_log_with_scope_warn(logscope::Image, __VA_ARGS__)

#define image_log_error(...) \
    libqb_log_with_scope_error(logscope::Image, __VA_ARGS__)
///@}

/**
 * @brief Debug check macro for image operations
 * @param _exp_ Expression to check
 * @note Logs a warning if the expression evaluates to false
 */
#define IMAGE_DEBUG_CHECK(_exp_) \
    do { \
        if (!(_exp_)) \
            image_log_warn("Condition (%s) failed", #_exp_); \
    } while (0)

/**
 * @brief Invalid image handle value
 * @note Returned to the caller if something goes wrong while loading an image
 */
#define INVALID_IMAGE_HANDLE -1

struct qbs;

/**
 * @brief Loads an image from a file (QB64 _LOADIMAGE function)
 * @param qbsFileName qbs string containing the image file path
 * @param bpp Bits per pixel (optional, for format conversion)
 * @param qbsRequirements qbs string containing image requirements (optional)
 * @param passed Number of parameters provided
 * @return Image handle on success, INVALID_IMAGE_HANDLE on failure
 * @note Supports multiple image formats. Returns a handle that can be used with graphics functions.
 */
int32_t func__loadimage(qbs *qbsFileName, int32_t bpp, qbs *qbsRequirements, int32_t passed);

/**
 * @brief Saves an image to a file (QB64 _SAVEIMAGE statement)
 * @param qbsFileName qbs string containing the output file path
 * @param imageHandle Image handle to save
 * @param qbsRequirements qbs string containing save requirements (optional)
 * @param passed Number of parameters provided
 * @note Saves the image in the format specified by the file extension or requirements.
 */
void sub__saveimage(qbs *qbsFileName, int32_t imageHandle, qbs *qbsRequirements, int32_t passed);

/**
 * @name BGRA Color Component Extraction Functions
 * @brief Extract color components from BGRA format (Blue, Green, Red, Alpha)
 */
///@{
/**
 * @brief Extracts the red component from a BGRA color
 * @param c BGRA color value
 * @return Red component (0-255)
 */
static inline constexpr uint8_t image_get_bgra_red(uint32_t c) {
    return uint8_t((c >> 16) & 0xFFu);
}

/**
 * @brief Extracts the green component from a BGRA color
 * @param c BGRA color value
 * @return Green component (0-255)
 */
static inline constexpr uint8_t image_get_bgra_green(uint32_t c) {
    return uint8_t((c >> 8) & 0xFFu);
}

/**
 * @brief Extracts the blue component from a BGRA color
 * @param c BGRA color value
 * @return Blue component (0-255)
 */
static inline constexpr uint8_t image_get_bgra_blue(uint32_t c) {
    return uint8_t(c & 0xFFu);
}

/**
 * @brief Extracts the alpha component from a BGRA color
 * @param c BGRA color value
 * @return Alpha component (0-255)
 */
static inline constexpr uint8_t image_get_bgra_alpha(uint32_t c) {
    return uint8_t(c >> 24);
}

/**
 * @brief Gets the BGR (without alpha) portion of a BGRA color
 * @param c BGRA color value
 * @return BGR color value (alpha bits cleared)
 */
static inline constexpr uint32_t image_get_bgra_bgr(uint32_t c) {
    return c & 0xFFFFFFu;
}

/**
 * @brief Sets the alpha component of a BGRA color
 * @param c BGRA color value
 * @param a Alpha component to set (defaults to 0xFF for opaque)
 * @return BGRA color with new alpha value
 */
static inline constexpr uint32_t image_set_bgra_alpha(uint32_t c, uint8_t a = 0xFFu) {
    return (c & 0xFFFFFFu) | (uint32_t(a) << 24);
}

/**
 * @brief Creates a BGRA color from components
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @param a Alpha component (0-255, defaults to 0xFF for opaque)
 * @return BGRA color value
 */
static inline constexpr uint32_t image_make_bgra(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFFu) {
    return uint32_t(b) | (uint32_t(g) << 8) | (uint32_t(r) << 16) | (uint32_t(a) << 24);
}
///@}

/**
 * @brief Scales a 5-bit value to 8 bits
 * @param v 5-bit value (0-31)
 * @return 8-bit scaled value (0-255)
 * @note Used for converting 16-bit color formats to 32-bit
 */
static inline constexpr int image_scale_5bits_to_8bits(int v) {
    return (v << 3) | (v >> 2);
}

/**
 * @brief Scales a 6-bit value to 8 bits
 * @param v 6-bit value (0-63)
 * @return 8-bit scaled value (0-255)
 * @note Used for converting 16-bit color formats to 32-bit
 */
static inline constexpr int image_scale_6bits_to_8bits(int v) {
    return (v << 2) | (v >> 4);
}

/**
 * @brief Swaps red and blue components in a color
 * @param clr Color value
 * @return Color with red and blue components swapped
 * @note Useful for converting between RGB and BGR formats
 */
static inline constexpr uint32_t image_swap_red_blue(uint32_t clr) {
    return ((clr & 0xFF00FF00u) | ((clr & 0x00FF0000u) >> 16) | ((clr & 0x000000FFu) << 16));
}

/**
 * @brief Clamps a color component value to valid range
 * @param n Color component value (may be outside 0-255)
 * @return Clamped value (0-255)
 * @note Ensures color components stay within valid 8-bit range
 */
static inline constexpr uint8_t image_clamp_color_component(int n) {
    return uint8_t(std::clamp(n, 0, 255));
}

/**
 * @brief Calculates Euclidean distance between two RGB colors
 * @param r1 Red component of first color
 * @param g1 Green component of first color
 * @param b1 Blue component of first color
 * @param r2 Red component of second color
 * @param g2 Green component of second color
 * @param b2 Blue component of second color
 * @return Distance between the two colors
 * @note Uses Euclidean distance formula: sqrt((r2-r1)² + (g2-g1)² + (b2-b1)²)
 */
static inline float image_calculate_rgb_distance(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {
    auto delta_r = float(r2) - float(r1);
    auto delta_g = float(g2) - float(g1);
    auto delta_b = float(b2) - float(b1);

    return sqrtf(delta_r * delta_r + delta_g * delta_g + delta_b * delta_b);
}

/**
 * @brief Calculates Manhattan distance between two RGB colors
 * @param r1 Red component of first color
 * @param g1 Green component of first color
 * @param b1 Blue component of first color
 * @param r2 Red component of second color
 * @param g2 Green component of second color
 * @param b2 Blue component of second color
 * @return Manhattan distance (sum of absolute differences)
 * @note Uses Manhattan distance: |r2-r1| + |g2-g1| + |b2-b1|
 */
static inline uint32_t image_get_color_delta(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2) {
    return uint32_t(::abs(long(r1) - long(r2)) + ::abs(long(g1) - long(g2)) + ::abs(long(b1) - long(b2)));
}

/**
 * @name RGB32 Color Creation Functions (QB64 _RGB32 function)
 * @brief Create 32-bit RGB color values with alpha channel
 */
///@{
/**
 * @brief Creates an RGB32 color from red, green, blue, and alpha components
 * @param r Red component (0-255, clamped)
 * @param g Green component (0-255, clamped)
 * @param b Blue component (0-255, clamped)
 * @param a Alpha component (0-255, clamped)
 * @return 32-bit RGB color value
 */
static inline constexpr uint32_t func__rgb32(int32_t r, int32_t g, int32_t b, int32_t a) {
    return (image_clamp_color_component(a) << 24) | (image_clamp_color_component(r) << 16) | (image_clamp_color_component(g) << 8) |
           image_clamp_color_component(b);
}

/**
 * @brief Creates an RGB32 color from red, green, and blue components (opaque)
 * @param r Red component (0-255, clamped)
 * @param g Green component (0-255, clamped)
 * @param b Blue component (0-255, clamped)
 * @return 32-bit RGB color value (alpha = 255)
 */
static inline constexpr uint32_t func__rgb32(int32_t r, int32_t g, int32_t b) {
    return 0xFF000000u | (image_clamp_color_component(r) << 16) | (image_clamp_color_component(g) << 8) | image_clamp_color_component(b);
}

/**
 * @brief Creates a grayscale RGB32 color with alpha
 * @param i Grayscale intensity (0-255, clamped)
 * @param a Alpha component (0-255, clamped)
 * @return 32-bit RGB color value (r=g=b=i)
 */
static inline constexpr uint32_t func__rgb32(int32_t i, int32_t a) {
    i = image_clamp_color_component(i);
    return (image_clamp_color_component(a) << 24) | (uint32_t(i) << 16) | (uint32_t(i) << 8) | uint32_t(i);
}

/**
 * @brief Creates a grayscale RGB32 color (opaque)
 * @param i Grayscale intensity (0-255, clamped)
 * @return 32-bit RGB color value (r=g=b=i, alpha=255)
 */
static inline constexpr uint32_t func__rgb32(int32_t i) {
    i = image_clamp_color_component(i);
    return 0xFF000000u | (uint32_t(i) << 16) | (uint32_t(i) << 8) | uint32_t(i);
}

/**
 * @brief Creates an RGBA32 color (alias for func__rgb32 with alpha)
 * @param r Red component (0-255, clamped)
 * @param g Green component (0-255, clamped)
 * @param b Blue component (0-255, clamped)
 * @param a Alpha component (0-255, clamped)
 * @return 32-bit RGBA color value
 */
static inline constexpr uint32_t func__rgba32(int32_t r, int32_t g, int32_t b, int32_t a) {
    return (image_clamp_color_component(a) << 24) | (image_clamp_color_component(r) << 16) | (image_clamp_color_component(g) << 8) |
           image_clamp_color_component(b);
}
///@}

/**
 * @name RGB32 Color Component Extraction Functions (QB64 _ALPHA32, _RED32, _GREEN32, _BLUE32)
 * @brief Extract color components from 32-bit RGB color values
 */
///@{
/**
 * @brief Extracts the alpha component from an RGB32 color (QB64 _ALPHA32 function)
 * @param col 32-bit RGB color value
 * @return Alpha component (0-255)
 */
static inline constexpr int32_t func__alpha32(uint32_t col) {
    return col >> 24;
}

/**
 * @brief Extracts the red component from an RGB32 color (QB64 _RED32 function)
 * @param col 32-bit RGB color value
 * @return Red component (0-255)
 */
static inline constexpr int32_t func__red32(uint32_t col) {
    return (col >> 16) & 0xFF;
}

/**
 * @brief Extracts the green component from an RGB32 color (QB64 _GREEN32 function)
 * @param col 32-bit RGB color value
 * @return Green component (0-255)
 */
static inline constexpr int32_t func__green32(uint32_t col) {
    return (col >> 8) & 0xFF;
}

/**
 * @brief Extracts the blue component from an RGB32 color (QB64 _BLUE32 function)
 * @param col 32-bit RGB color value
 * @return Blue component (0-255)
 */
static inline constexpr int32_t func__blue32(uint32_t col) {
    return col & 0xFF;
}
///@}