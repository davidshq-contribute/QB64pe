#pragma once

#include <stdint.h>

#include "qbs.h"

/**
 * @file hexoctbin.h
 * @brief Number format conversion functions for QB64-PE
 * 
 * This header provides functions for converting numbers to binary, octal, and hexadecimal string representations.
 */

/**
 * @name Binary Conversion Functions
 * @brief Convert values to binary string representation
 */
///@{
/**
 * @brief Converts an integer to binary string (QB64 BIN$ function)
 * @param value Integer value to convert
 * @param neg_bits Number of bits to use for negative numbers (two's complement)
 * @return qbs string containing binary representation, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__bin(int64_t value, int32_t neg_bits);

/**
 * @brief Converts a floating-point value to binary string (QB64 BIN$ function)
 * @param value Floating-point value to convert
 * @return qbs string containing binary representation, or NULL on failure
 * @note Converts the floating-point value to its binary representation.
 *       Caller must free the returned qbs with qbs_free()
 */
qbs *func__bin_float(long double value);
///@}

/**
 * @name Octal Conversion Functions
 * @brief Convert values to octal string representation
 */
///@{
/**
 * @brief Converts an integer to octal string (QB64 OCT$ function)
 * @param value Integer value to convert
 * @param neg_bits Number of bits to use for negative numbers (two's complement)
 * @return qbs string containing octal representation, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func_oct(int64_t value, int32_t neg_bits);

/**
 * @brief Converts a floating-point value to octal string (QB64 OCT$ function)
 * @param value Floating-point value to convert
 * @return qbs string containing octal representation, or NULL on failure
 * @note Converts the floating-point value to its octal representation.
 *       Caller must free the returned qbs with qbs_free()
 */
qbs *func_oct_float(long double value);
///@}

/**
 * @name Hexadecimal Conversion Functions
 * @brief Convert values to hexadecimal string representation
 */
///@{
/**
 * @brief Converts an integer to hexadecimal string (QB64 HEX$ function)
 * @param value Integer value to convert
 * @param neg_size Size parameter for negative numbers
 * @return qbs string containing hexadecimal representation, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func_hex(int64_t value, int32_t neg_size);

/**
 * @brief Converts a floating-point value to hexadecimal string (QB64 HEX$ function)
 * @param value Floating-point value to convert
 * @return qbs string containing hexadecimal representation, or NULL on failure
 * @note Converts the floating-point value to its hexadecimal representation.
 *       Caller must free the returned qbs with qbs_free()
 */
qbs *func_hex_float(long double value);
///@}