//----------------------------------------------------------------------------------------------------------------------
// QB64-PE Font Library
// Powered by FreeType (https://freetype.org/)
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include <stdio.h>

/**
 * @file font.h
 * @brief Font loading and rendering functions for QB64-PE
 * 
 * This header provides functions for loading fonts, rendering text, and measuring
 * text dimensions. Powered by FreeType library.
 */

/**
 * @brief Invalid font handle value
 * @note Returned when font operations fail
 */
#define INVALID_FONT_HANDLE 0

/**
 * @name Font Load Options
 * @brief Options for loading fonts
 */
///@{
#define FONT_LOAD_DONTBLEND 8    ///< Don't blend font rendering
#define FONT_LOAD_MONOSPACE 16   ///< Force monospace font
#define FONT_LOAD_UNICODE 32     ///< Enable Unicode support
#define FONT_LOAD_AUTOMONO 64    ///< Automatically detect monospace
///@}

/**
 * @name Font Render Options
 * @brief Options for rendering text
 */
///@{
#define FONT_RENDER_MONOCHROME 1 ///< Render text in monochrome (no anti-aliasing)
///@}

struct qbs;

/**
 * @brief CP437 to UTF-16 lookup table
 * @note Maps Code Page 437 characters to UTF-16 Unicode values
 */
extern uint16_t codepage437_to_unicode16[];

/**
 * @brief Loads a font file into memory
 * @param file_path_name Path to the font file
 * @param[out] out_bytes Pointer to store the number of bytes loaded
 * @return Pointer to loaded font data, or NULL on failure
 * @note The caller is responsible for freeing the returned memory
 */
uint8_t *FontLoadFileToMemory(const char *file_path_name, int32_t *out_bytes);

/**
 * @brief Loads a font from memory
 * @param content_original Pointer to font data in memory
 * @param content_bytes Size of font data in bytes
 * @param default_pixel_height Default pixel height for the font
 * @param which_font Font identifier
 * @param[in,out] options Font load options (see FONT_LOAD_* constants)
 * @return Font handle on success, INVALID_FONT_HANDLE on failure
 * @note Loads a font from memory and returns a handle for use with font functions
 */
int32_t FontLoad(const uint8_t *content_original, int32_t content_bytes, int32_t default_pixel_height, int32_t which_font, int32_t &options);

/**
 * @brief Frees a font handle
 * @param fh Font handle to free
 * @note Releases resources associated with the font. Do not use the handle after calling this.
 */
void FontFree(int32_t fh);

/**
 * @brief Gets the width of a font
 * @param fh Font handle
 * @return Font width in pixels, or 0 on error
 * @note Returns the width of characters in the font (useful for monospace fonts)
 */
int32_t FontWidth(int32_t fh);

/**
 * @brief Renders text using UTF-32 codepoints
 * @param fh Font handle
 * @param codepoint Array of UTF-32 codepoints to render
 * @param codepoints Number of codepoints
 * @param options Render options (see FONT_RENDER_* constants)
 * @param[out] out_data Pointer to store rendered image data
 * @param[out] out_x Pointer to store output width
 * @param[out] out_y Pointer to store output height
 * @return true on success, false on failure
 * @note Renders text and returns the image data. Caller must free out_data.
 */
bool FontRenderTextUTF32(int32_t fh, const char32_t *codepoint, int32_t codepoints, int32_t options, uint8_t **out_data, int32_t *out_x, int32_t *out_y);

/**
 * @brief Renders text using ASCII characters
 * @param fh Font handle
 * @param codepoint Array of ASCII characters to render
 * @param codepoints Number of characters
 * @param options Render options (see FONT_RENDER_* constants)
 * @param[out] out_data Pointer to store rendered image data
 * @param[out] out_x Pointer to store output width
 * @param[out] out_y Pointer to store output height
 * @return true on success, false on failure
 * @note Renders text and returns the image data. Caller must free out_data.
 */
bool FontRenderTextASCII(int32_t fh, const uint8_t *codepoint, int32_t codepoints, int32_t options, uint8_t **out_data, int32_t *out_x, int32_t *out_y);

/**
 * @brief Calculates the width of text using UTF-32 codepoints
 * @param fh Font handle
 * @param codepoint Array of UTF-32 codepoints
 * @param codepoints Number of codepoints
 * @return Width of the text in pixels
 * @note Calculates how wide the text would be when rendered
 */
int32_t FontPrintWidthUTF32(int32_t fh, const char32_t *codepoint, int32_t codepoints);

/**
 * @brief Calculates the width of text using ASCII characters
 * @param fh Font handle
 * @param codepoint Array of ASCII characters
 * @param codepoints Number of characters
 * @return Width of the text in pixels
 * @note Calculates how wide the text would be when rendered
 */
int32_t FontPrintWidthASCII(int32_t fh, const uint8_t *codepoint, int32_t codepoints);

/**
 * @brief Gets the height of a font (QB64 _UFONTHEIGHT function)
 * @param qb64_fh Font handle
 * @param passed Flag indicating if parameter was provided
 * @return Font height in pixels
 * @note Returns the height of characters in the font
 */
int32_t func__UFontHeight(int32_t qb64_fh, int32_t passed);

/**
 * @brief Calculates the width of text (QB64 _UPRINTWIDTH function)
 * @param text qbs string containing text to measure
 * @param utf_encoding UTF encoding type
 * @param qb64_fh Font handle
 * @param passed Number of parameters provided
 * @return Width of the text in pixels
 * @note Calculates how wide the text would be when rendered with the specified font
 */
int32_t func__UPrintWidth(const qbs *text, int32_t utf_encoding, int32_t qb64_fh, int32_t passed);

/**
 * @brief Gets the line spacing of a font (QB64 _ULINESPACING function)
 * @param qb64_fh Font handle
 * @param passed Flag indicating if parameter was provided
 * @return Line spacing in pixels
 * @note Returns the recommended spacing between lines for the font
 */
int32_t func__ULineSpacing(int32_t qb64_fh, int32_t passed);

/**
 * @brief Prints a string using a font (QB64 _UPRINTSTRING statement)
 * @param start_x X coordinate to start printing
 * @param start_y Y coordinate to start printing
 * @param text qbs string containing text to print
 * @param max_width Maximum width for text wrapping (optional)
 * @param utf_encoding UTF encoding type
 * @param qb64_fh Font handle
 * @param dst_img Destination image handle (optional)
 * @param passed Number of parameters provided
 * @note Renders text to the screen or specified image using the font
 */
void sub__UPrintString(int32_t start_x, int32_t start_y, const qbs *text, int32_t max_width, int32_t utf_encoding, int32_t qb64_fh, int32_t dst_img,
                       int32_t passed);

/**
 * @brief Gets character positions in text (QB64 _UCHARPOS function)
 * @param text qbs string containing text to analyze
 * @param arr Array to store character positions
 * @param utf_encoding UTF encoding type
 * @param qb64_fh Font handle
 * @param passed Number of parameters provided
 * @return Number of character positions calculated
 * @note Calculates the X position of each character when rendered
 */
int32_t func__UCharPos(const qbs *text, void *arr, int32_t utf_encoding, int32_t qb64_fh, int32_t passed);
