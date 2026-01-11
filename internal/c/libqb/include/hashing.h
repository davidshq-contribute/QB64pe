//-----------------------------------------------------------------------------------------------------
//  QB64-PE Hashing Library
//  Uses hash functions from miniz and FreeType
//-----------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

/**
 * @file hashing.h
 * @brief Hash function implementations for QB64-PE
 * 
 * This header provides functions for computing various hash values of data.
 * Uses hash functions from miniz and FreeType libraries.
 */

struct qbs;

/**
 * @brief Computes Adler-32 checksum (QB64 _ADLER32 function)
 * @param text qbs string containing data to hash
 * @return 32-bit Adler-32 checksum value
 * @note Adler-32 is a fast checksum algorithm. Returns 0 on error.
 */
uint32_t func__adler32(qbs *text);

/**
 * @brief Computes CRC-32 checksum (QB64 _CRC32 function)
 * @param text qbs string containing data to hash
 * @return 32-bit CRC-32 checksum value
 * @note CRC-32 is a cyclic redundancy check algorithm. Returns 0 on error.
 */
uint32_t func__crc32(qbs *text);

/**
 * @brief Computes MD5 hash (QB64 _MD5$ function)
 * @param text qbs string containing data to hash
 * @return qbs string containing hexadecimal MD5 hash, or NULL on failure
 * @note Returns a 32-character hexadecimal string representing the MD5 hash.
 *       Caller must free the returned qbs with qbs_free()
 */
qbs *func__md5(qbs *text);
