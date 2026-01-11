//-----------------------------------------------------------------------------------------------------
//  QB64-PE Encoding Library
//  Powered by MODP_B64 (https://github.com/client9/stringencoders)
//-----------------------------------------------------------------------------------------------------

#pragma once

/**
 * @file encoding.h
 * @brief Base64 encoding and decoding functions for QB64-PE
 * 
 * This header provides functions for encoding and decoding data using Base64 encoding.
 */

struct qbs;

/**
 * @brief Encodes data to Base64 (QB64 _BASE64ENCODE$ function)
 * @param src Source qbs string containing data to encode
 * @return qbs string containing Base64-encoded data, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__base64encode(qbs *src);

/**
 * @brief Decodes Base64-encoded data (QB64 _BASE64DECODE$ function)
 * @param src qbs string containing Base64-encoded data
 * @return qbs string containing decoded data, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__base64decode(qbs *src);
