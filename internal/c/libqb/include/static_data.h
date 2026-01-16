//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Static Data Module
//  Contains icon bitmaps, palette data, and lookup tables
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_STATIC_DATA_H
#define INCLUDE_LIBQB_STATIC_DATA_H

#include <stdint.h>

// ============================================================================
// ICON BITMAP DATA
// ============================================================================

/// Width of the 16x16 QB64 icon
extern const int32_t image_qbicon16_w;
/// Height of the 16x16 QB64 icon
extern const int32_t image_qbicon16_h;
/// Raw BGRA pixel data for the 16x16 QB64 icon
extern const uint8_t image_qbicon16[];

/// Width of the 32x32 QB64 icon
extern const int32_t image_qbicon32_w;
/// Height of the 32x32 QB64 icon
extern const int32_t image_qbicon32_h;
/// Raw BGRA pixel data for the 32x32 QB64 icon
extern const uint8_t image_qbicon32[];

// ============================================================================
// DEAD CHARACTER LOOKUP TABLE (Unicode combining characters)
// ============================================================================

/// Lookup table for dead key combinations
/// Format: [deadkey UTF16], [ASCII letter], [resulting UTF16], ...0
extern const uint16_t deadchar_lookup[];

// ============================================================================
// CODE PAGE 437 TO UNICODE LOOKUP TABLE
// ============================================================================

/// Code page 437 to UTF-16 lookup table (256 entries)
/// Maps DOS/IBM PC character codes to their Unicode equivalents
extern uint16_t codepage437_to_unicode16[];

// ============================================================================
// QB64 PALETTE DATA
// ============================================================================

/// Length of the QB64 256-color palette in bytes
extern const int32_t file_qb64_pal_len;
/// QB64 default 256-color palette (BGRA format, 4 bytes per color)
extern const uint8_t file_qb64_pal[];

/// Length of the QB64 EGA palette in bytes
extern const int32_t file_qb64ega_pal_len;
/// QB64 EGA 64-color palette (BGRA format, 4 bytes per color)
extern const uint8_t file_qb64ega_pal[];

#endif
