//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Print Using Module
//  Handles PRINT USING string formatting for numeric and string values
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_PRINT_USING_H
#define INCLUDE_LIBQB_PRINT_USING_H

#include "qbs.h"
#include <stdint.h>

// ============================================================================
// PRINT USING FUNCTIONS
// ============================================================================

/// Main PRINT USING formatter
/// @param f Format string
/// @param s2 Starting position in format string
/// @param dest Destination qbs for output
/// @param pu_str String value to format (NULL for numeric)
/// @return Next position in format string
int32_t print_using(qbs *f, int32_t s2, qbs *dest, qbs *pu_str);

/// Format a 64-bit signed integer using PRINT USING
int32_t print_using_integer64(qbs *format, int64_t value, int32_t start, qbs *output);

/// Format a 64-bit unsigned integer using PRINT USING
int32_t print_using_uinteger64(qbs *format, uint64_t value, int32_t start, qbs *output);

/// Format a single-precision float using PRINT USING
int32_t print_using_single(qbs *format, float value, int32_t start, qbs *output);

/// Format a double-precision float using PRINT USING
int32_t print_using_double(qbs *format, double value, int32_t start, qbs *output);

/// Format a long double using PRINT USING
int32_t print_using_float(qbs *format, long double value, int32_t start, qbs *output);

#endif
