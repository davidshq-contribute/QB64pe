//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Unicode Conversion Module
//  Unicode encoding conversion functions between different formats
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_UNICODE_CONVERT_H
#define INCLUDE_LIBQB_UNICODE_CONVERT_H

#include "../../os.h"  // For int32 type definitions

// ============================================================================
// UNICODE CONVERSION
// ============================================================================

/// Converts text between different Unicode encoding formats.
/// Supports conversion between ASCII/CP437, UTF8, UTF16, and UTF32.
/// 
/// @param src_fmt Source format: 1=ASCII(CP437), 8=UTF8, 16=UTF16, 32=UTF32
/// @param src_buf Source buffer containing text to convert
/// @param src_size Size of source buffer in bytes
/// @param dest_fmt Destination format: 16=UTF16, 32=UTF32
/// @param dest_buf Destination buffer (must be at least src_size*4+4 bytes)
/// @return Number of bytes written to dest_buf (excluding null terminator)
/// 
/// @note Important: dest_buf must be at least src_size*4+4 in length to ensure
///       enough space is available for the conversion. The function adds a
///       null terminator to the destination buffer but does not count it in
///       the returned size.
/// @note UTF8 support is not yet implemented (src_fmt=8 will not work)
/// @note Surrogate pairs are not yet handled for UTF16
int32 convert_unicode(int32 src_fmt, void *src_buf, int32 src_size, int32 dest_fmt, void *dest_buf);

#endif // INCLUDE_LIBQB_UNICODE_CONVERT_H
