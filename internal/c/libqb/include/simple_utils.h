//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Simple Utilities Module
//  Basic utility functions for bit operations, integer building, and character conversion
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_SIMPLE_UTILS_H
#define INCLUDE_LIBQB_SIMPLE_UTILS_H

#include "../../os.h"  // For int32, int64, uint32, uint64 type definitions

// Forward declarations
struct qbs;

// ============================================================================
// BIT OPERATIONS
// ============================================================================

/// Performs a left bit rotation on a 32-bit value.
/// Rotates bits left with wrap-around behavior.
/// @param word The 32-bit value to rotate
/// @param shift Number of bits to shift left
/// @return The rotated value with bits wrapping around
uint32 rotateLeft(uint32 word, uint32 shift);

// ============================================================================
// INTEGER BUILDING
// ============================================================================

/// Builds a 64-bit signed integer from two 32-bit values.
/// Combines two 32-bit values into a single 64-bit integer.
/// @param val2 High 32 bits
/// @param val1 Low 32 bits
/// @return Combined 64-bit signed integer
int64 build_int64(uint32 val2, uint32 val1);

/// Builds a 64-bit unsigned integer from two 32-bit values.
/// Combines two 32-bit values into a single 64-bit unsigned integer.
/// @param val2 High 32 bits
/// @param val1 Low 32 bits
/// @return Combined 64-bit unsigned integer
uint64 build_uint64(uint32 val2, uint32 val1);

// ============================================================================
// ARITHMETIC HELPERS
// ============================================================================

/// Adds one to an integer value.
/// Simple increment function, primarily for testing purposes.
/// @param x Value to increment
/// @return x + 1
int32 addone(int32 x);

// ============================================================================
// CHARACTER CONVERSION
// ============================================================================

/// Converts a Unicode character to its CP437 (code page 437) equivalent.
/// Searches the code page 437 to Unicode mapping table for a match.
/// @param x Unicode character to convert
/// @return CP437 character code, or 0 if no equivalent exists
uint32 unicode_to_cp437(uint32 x);

// ============================================================================
// VARIABLE POINTER HELPERS
// ============================================================================

/// Creates a 3-byte string containing type and offset information.
/// Used for VARPTR functionality to encode variable pointer data.
/// @param type Variable type code
/// @param offset Variable offset
/// @return QB64 string containing 3-byte encoded pointer information
qbs *func_varptr_helper(uint8 type, uint16 offset);

#endif // INCLUDE_LIBQB_SIMPLE_UTILS_H
