//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Number Base Conversion Module
//  Provides binary, octal, and hexadecimal string conversion utilities
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

#include "qbs.h"

// ============================================================================
// BINARY STRING CONVERSION
// ============================================================================

/// Converts integer value to binary string representation
/// Formats integer as binary string with optional negative bit handling
/// @param value Integer value to convert
/// @param neg_bits Number of bits to use for negative representation
/// @return QB64 string containing binary representation
qbs *func__bin(int64_t value, int32_t neg_bits);

/// Converts floating-point value to binary string representation
/// Formats floating-point number as binary string
/// @param value Floating-point value to convert
/// @return QB64 string containing binary representation
qbs *func__bin_float(long double value);

// ============================================================================
// OCTAL STRING CONVERSION
// ============================================================================

/// Converts integer value to octal string representation
/// Formats integer as octal string with optional negative bit handling
/// @param value Integer value to convert
/// @param neg_bits Number of bits to use for negative representation
/// @return QB64 string containing octal representation
qbs *func_oct(int64_t value, int32_t neg_bits);

/// Converts floating-point value to octal string representation
/// Formats floating-point number as octal string
/// @param value Floating-point value to convert
/// @return QB64 string containing octal representation
qbs *func_oct_float(long double value);

// ============================================================================
// HEXADECIMAL STRING CONVERSION
// ============================================================================

/// Converts integer value to hexadecimal string representation
/// Formats integer as hexadecimal string with optional negative size handling
/// @param value Integer value to convert
/// @param neg_size Number of digits to use for negative representation
/// @return QB64 string containing hexadecimal representation
qbs *func_hex(int64_t value, int32_t neg_size);

/// Converts floating-point value to hexadecimal string representation
/// Formats floating-point number as hexadecimal string
/// @param value Floating-point value to convert
/// @return QB64 string containing hexadecimal representation
qbs *func_hex_float(long double value);
