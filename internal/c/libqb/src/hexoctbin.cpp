
#include "libqb-common.h"

#include <math.h>
#include <stdint.h>
#include <string.h>

#include "hexoctbin.h"
#include "qbs.h"
#include "rounding.h"

/**
 * @file hexoctbin.cpp
 * @brief Implementation of number format conversion functions for QB64-PE
 * 
 * This file implements functions for converting numbers to binary, octal, and
 * hexadecimal string representations, with support for negative numbers.
 */

/**
 * @brief Converts an integer to binary string (QB64 BIN$ function)
 * @param value Integer value to convert
 * @param neg_bits Minimum number of bits for negative numbers
 * @return qbs string containing binary representation
 * @note For negative numbers, expands to 16/32/64 bit boundaries if needed.
 *       Returns "0" if value is zero. Caller must free the returned qbs.
 */
qbs *func__bin(int64_t value, int32_t neg_bits) {
    static int32_t i, i2, i3, neg;
    static int64_t value2;
    static qbs *str;

    str = qbs_new(64, 1);

    // negative?
    if ((value >> 63) & 1)
        neg = 1;
    else
        neg = 0;

    // calc. most significant bit
    i2 = 0;
    value2 = value;
    if (neg) {
        for (i = 1; i <= 64; i++) {
            if (!(value2 & 1))
                i2 = i;
            value2 >>= 1;
        }
        if (i2 >= neg_bits) {
            // doesn't fit in neg_bits, so expand to next 16/32/64 boundary
            i3 = 64;
            if (i2 < 32)
                i3 = 32;
            if (i2 < 16)
                i3 = 16;
            i2 = i3;
        } else
            i2 = neg_bits;
    } else {
        for (i = 1; i <= 64; i++) {
            if (value2 & 1)
                i2 = i;
            value2 >>= 1;
        }
    }

    if (!i2) {
        str->chr[0] = 48;
        str->len = 1;
        return str;
    } //"0"

    // calc. number of characters required in i3
    i3 = i2; // equal for BIN$ because one bit = one char

    // build string
    str->len = i3;
    i3--;
    for (i = 1; i <= i2; i++) {
        str->chr[i3--] = (value & 1) + 48;
        value >>= 1;
    }

    return str;
}

/**
 * @brief Converts a floating-point number to binary string (QB64 BIN$ function for floats)
 * @param value Floating-point value to convert
 * @return qbs string containing binary representation
 * @note QBASIC doesn't have a BIN$ function. QB64 uses 32 bin digits for SINGLE/DOUBLE/FLOAT
 *       but if this range is exceeded, it uses up to 64 bin digits before generating an "OVERFLOW" error.
 *       Performs overflow check before calling func__bin. Generates error 6 (Overflow) if value
 *       is out of int64 range. Caller must free the returned qbs.
 */
qbs *func__bin_float(long double value) {
    static qbs *str;
    static int64_t ivalue;
    static int64_t uivalue;
    // ref: uint64 0-18446744073709551615
    //      int64 \969223372036854775808 to 9223372036854775807
    if ((value >= 9.223372036854776E18) || (value <= -9.223372036854776E18)) {
        // note: ideally, the following line would be used, however, qbr_longdouble_to_uint64 just does the same as qbr
        // if ((value>=1.844674407370956E19)||(value<=-9.223372036854776E18)){
        str = qbs_new(0, 1);
        error(6); // Overflow
        return str;
    }
    if (value >= 0) {
        uivalue = qbr_longdouble_to_uint64(value);
        ivalue = uivalue;
    } else {
        ivalue = qbr(value);
    }
    return func__bin(ivalue, 32);
}

/**
 * @brief Converts an integer to octal string (QB64 OCT$ function)
 * @param value Integer value to convert
 * @param neg_bits Minimum number of bits for negative numbers
 * @return qbs string containing octal representation
 * @note For negative numbers, expands to 16/32/64 bit boundaries if needed.
 *       Returns "0" if value is zero. Caller must free the returned qbs.
 */
qbs *func_oct(int64_t value, int32_t neg_bits) {

    static int32_t i, i2, i3, x, x2, neg;
    static int64_t value2;
    static qbs *str;

    str = qbs_new(22, 1);

    // negative?
    if ((value >> 63) & 1)
        neg = 1;
    else
        neg = 0;

    // calc. most significant bit
    i2 = 0;
    value2 = value;
    if (neg) {
        for (i = 1; i <= 64; i++) {
            if (!(value2 & 1))
                i2 = i;
            value2 >>= 1;
        }
        if (i2 >= neg_bits) {
            // doesn't fit in neg_bits, so expand to next 16/32/64 boundary
            i3 = 64;
            if (i2 < 32)
                i3 = 32;
            if (i2 < 16)
                i3 = 16;
            i2 = i3;
        } else
            i2 = neg_bits;
    } else {
        for (i = 1; i <= 64; i++) {
            if (value2 & 1)
                i2 = i;
            value2 >>= 1;
        }
    }

    if (!i2) {
        str->chr[0] = 48;
        str->len = 1;
        return str;
    } //"0"

    // calc. number of characters required in i3
    i3 = i2 / 3;
    if ((i3 * 3) != i2)
        i3++;

    // build string
    str->len = i3;
    i3--;
    x = 0;
    x2 = 0;
    for (i = 1; i <= i2; i++) {
        if (value & 1)
            x2 |= (1 << x);
        value >>= 1;
        x++;
        if (x == 3) {
            str->chr[i3--] = x2 + 48;
            x2 = 0;
            x = 0;
        }
    }
    if (x)
        str->chr[i3] = x2 + 48;

    return str;
}

/**
 * @brief Converts a floating-point number to octal string (QB64 OCT$ function for floats)
 * @param value Floating-point value to convert
 * @return qbs string containing octal representation
 * @note QBASIC uses 11 oct digits for SINGLE/DOUBLE or generates "OVERFLOW" if this range is exceeded.
 *       QB64 uses 11 oct digits for SINGLE/DOUBLE/FLOAT but if this range is exceeded,
 *       it uses up to 22 oct digits before generating an "OVERFLOW" error.
 *       Performs overflow check before calling func_oct. Generates error 6 (Overflow) if value
 *       is out of int64 range. Caller must free the returned qbs.
 */
qbs *func_oct_float(long double value) {
    static qbs *str;
    static int64_t ivalue;
    static int64_t uivalue;
    // ref: uint64 0-18446744073709551615
    //      int64 \969223372036854775808 to 9223372036854775807
    if ((value >= 9.223372036854776E18) || (value <= -9.223372036854776E18)) {
        // note: ideally, the following line would be used, however, qbr_longdouble_to_uint64 just does the same as qbr
        // if ((value>=1.844674407370956E19)||(value<=-9.223372036854776E18)){
        str = qbs_new(0, 1);
        error(6); // Overflow
        return str;
    }
    if (value >= 0) {
        uivalue = qbr_longdouble_to_uint64(value);
        ivalue = uivalue;
    } else {
        ivalue = qbr(value);
    }
    return func_oct(ivalue, 32);
}

/**
 * @brief Converts an integer to hexadecimal string (QB64 HEX$ function)
 * @param value Integer value to convert
 * @param neg_size Minimum number of hex digits for negative numbers
 * @return qbs string containing hexadecimal representation
 * @note Negative int64 values can be treated as positive uint64 values (and vice versa).
 *       For negative numbers, expands to 4/8/16 hex digit boundaries if needed.
 *       Returns "0" if value is zero. Caller must free the returned qbs.
 */
qbs *func_hex(int64_t value, int32_t neg_size) {
    // note: negative int64 values can be treated as positive uint64 values (and vise versa)

    static int32_t i, i2, i3, x, neg;
    static int64_t value2;
    static qbs *str;

    str = qbs_new(16, 1);

    value2 = value;
    i2 = 0;
    i3 = 0;
    for (i = 1; i <= 16; i++) {
        if (value2 & 15)
            i2 = i; // most significant digit of positive value
        if ((value2 & 15) != 15) {
            i3 = i; // most significant digit of negative value
            if ((((value2 & 8) == 0) && (i != 16)))
                i3++; // for a negative number to fit into 4/8 characters, its top bit must be on
        }
        x = value2 & 15;
        if (x > 9)
            x += 55;
        else
            x += 48;
        str->chr[16 - i] = x;
        value2 >>= 4;
    }
    if (!i2) {
        str->chr[0] = 48;
        str->len = 1;
        return str;
    } //"0"

    // negative?
    if ((value >> 63) & 1)
        neg = 1;
    else
        neg = 0;

    // change i2 from sig-digits to string-output-digits
    if (neg) {
        if (i3 <= neg_size) {
            i2 = neg_size; // extend to minimum character size
        } else {
            // didn't fit in recommended size, expand to either 4, 8 or 16 appropriately
            i2 = 16;
            if (i3 <= 8)
                i2 = 8;
            if (i3 <= 4)
                i2 = 4;
        }
    } // neg

    // adjust string to the left to remove unnecessary characters
    if (i2 != 16) {
        memmove(str->chr, str->chr + (16 - i2), i2);
        str->len = i2;
    }

    return str;
}

/**
 * @brief Converts a floating-point number to hexadecimal string (QB64 HEX$ function for floats)
 * @param value Floating-point value to convert
 * @return qbs string containing hexadecimal representation
 * @note QBASIC uses 8 hex digits for SINGLE/DOUBLE or generates "OVERFLOW" if this range is exceeded.
 *       QB64 uses 8 hex digits for SINGLE/DOUBLE/FLOAT but if this range is exceeded,
 *       it uses up to 16 hex digits before generating an "OVERFLOW" error.
 *       Performs overflow check before calling func_hex. Generates error 6 (Overflow) if value
 *       is out of int64 range. Caller must free the returned qbs.
 */
qbs *func_hex_float(long double value) {
    static qbs *str;
    static int64_t ivalue;
    static int64_t uivalue;
    // ref: uint64 0-18446744073709551615
    //      int64 \969223372036854775808 to 9223372036854775807
    if ((value >= 9.223372036854776E18) || (value <= -9.223372036854776E18)) {
        // note: ideally, the following line would be used, however, qbr_longdouble_to_uint64 just does the same as qbr
        // if ((value>=1.844674407370956E19)||(value<=-9.223372036854776E18)){
        str = qbs_new(0, 1);
        error(6); // Overflow
        return str;
    }
    if (value >= 0) {
        uivalue = qbr_longdouble_to_uint64(value);
        ivalue = uivalue;
    } else {
        ivalue = qbr(value);
    }
    return func_hex(ivalue, 8);
}
