//-----------------------------------------------------------------------------------------------------
//  QB64-PE Compression Library
//  Powered by miniz (https://github.com/richgel999/miniz)
//-----------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

/**
 * @file compression.h
 * @brief Compression and decompression functions for QB64-PE
 * 
 * This header provides functions for compressing and decompressing data using
 * the DEFLATE algorithm (zlib/gzip compatible).
 */

struct qbs;

/**
 * @brief Compresses data using DEFLATE algorithm (QB64 _DEFLATE$ function)
 * @param src Source qbs string containing data to compress
 * @param level Compression level (0-9, where -1 uses default, 0 is no compression, 9 is maximum)
 * @param passed Flag indicating if level parameter was provided
 * @return qbs string containing compressed data, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__deflate(qbs *src, int32_t level = -1, int32_t passed = 0);

/**
 * @brief Decompresses data using DEFLATE algorithm (QB64 _INFLATE$ function)
 * @param text qbs string containing compressed data
 * @param originalsize Original size of the uncompressed data
 * @param passed Flag indicating if originalsize parameter was provided
 * @return qbs string containing decompressed data, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__inflate(qbs *text, int64_t originalsize, int32_t passed);
