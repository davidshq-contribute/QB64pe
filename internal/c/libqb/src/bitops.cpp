
#include "libqb-common.h"

#include <math.h>

#include "bitops.h"

/**
 * @file bitops.cpp
 * @brief Implementation of bit manipulation functions for QB64-PE
 * 
 * This file implements low-level bit array access functions for reading and writing
 * variable-width bit fields from byte arrays.
 */

/**
 * @brief Gets unsigned bits from a bit array
 * @param bsize Number of bits to read (1-64)
 * @param base Base address of the byte array
 * @param i Bit index (0-based)
 * @return Unsigned value of the bits
 * @note Reads bsize bits starting at bit index i from the byte array.
 *       Bits are stored in little-endian format within bytes.
 */
uint64_t getubits(uint32_t bsize, uint8_t *base, intptr_t i) {
    int64_t bmask;
    bmask = ~(-(((int64_t)1) << bsize));
    i *= bsize;
    return ((*(uint64_t *)(base + (i >> 3))) >> (i & 7)) & bmask;
}

/**
 * @brief Gets signed bits from a bit array
 * @param bsize Number of bits to read (1-64)
 * @param base Base address of the byte array
 * @param i Bit index (0-based)
 * @return Signed value of the bits (sign-extended)
 * @note Reads bsize bits starting at bit index i from the byte array.
 *       Performs sign extension if the most significant bit is set.
 */
int64_t getbits(uint32_t bsize, uint8_t *base, intptr_t i) {
    int64_t bmask, bval64;
    bmask = ~(-(((int64_t)1) << bsize));
    i *= bsize;
    bval64 = ((*(uint64_t *)(base + (i >> 3))) >> (i & 7)) & bmask;
    if (bval64 & (((int64_t)1) << (bsize - 1)))
        return bval64 | (~bmask);
    return bval64;
}

/**
 * @brief Sets bits in a bit array
 * @param bsize Number of bits to write (1-64)
 * @param base Base address of the byte array
 * @param i Bit index (0-based)
 * @param val Value to write (only bsize bits are used)
 * @note Writes bsize bits starting at bit index i to the byte array.
 *       Only the lower bsize bits of val are written. Other bits are preserved.
 */
void setbits(uint32_t bsize, uint8_t *base, intptr_t i, int64_t val) {
    int64_t bmask;
    uint64_t *bptr64;
    bmask = (((uint64_t)1) << bsize) - 1;
    i *= bsize;
    bptr64 = (uint64_t *)(base + (i >> 3));
    *bptr64 = (*bptr64 & (((bmask << (i & 7)) ^ -1))) | ((val & bmask) << (i & 7));
}
