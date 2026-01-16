//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Character Set Data Module
//  Contains the built-in 8x8 and 8x16 bitmap font data
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_CHARSET_DATA_H
#define INCLUDE_LIBQB_CHARSET_DATA_H

#include <stdint.h>

// ============================================================================
// 8x8 CHARACTER SET (16KB)
// ============================================================================

/// Length of the 8x8 character set raw data in bytes
extern const int32_t file_charset8_raw_len;

/// Raw bitmap data for the built-in 8x8 character font
/// Contains 256 characters, each 8x8 pixels (64 bytes per character)
extern const uint8_t file_charset8_raw[];

// ============================================================================
// 8x16 CHARACTER SET (32KB)
// ============================================================================

/// Length of the 8x16 character set raw data in bytes
extern const int32_t file_chrset16_raw_len;

/// Raw bitmap data for the built-in 8x16 character font
/// Contains 256 characters, each 8x16 pixels (128 bytes per character)
extern const uint8_t file_chrset16_raw[];

#endif
