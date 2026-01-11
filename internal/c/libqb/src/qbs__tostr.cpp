
#include "libqb-common.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_handle.h"
#include "qbs.h"

/**
 * @file qbs__tostr.cpp
 * @brief Implementation of _TOSTR$ function for QB64-PE
 * 
 * This file implements the modern _TOSTR$ function which converts numeric values
 * to strings without a leading space (unlike STR$) and without QB4.5 compatible rounding.
 */

/**
 * @name _TOSTR$ Function Overloads
 * @brief Convert numeric types to qbs strings (modern format, no leading space)
 */
///@{
/**
 * @brief Converts a signed 64-bit integer to string (QB64 _TOSTR$ function)
 * @param value Integer value to convert
 * @param digits Number of digits (unused for integers)
 * @param passed Flag indicating if digits parameter was provided (unused for integers)
 * @return qbs string containing the string representation
 * @note Modern format: no leading space. Caller must free the returned qbs with qbs_free().
 */
qbs *qbs__tostr(int64_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%" PRId64, value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

qbs *qbs__tostr(int32_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%i", value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

qbs *qbs__tostr(int16_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%i", value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

qbs *qbs__tostr(int8_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%i", value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

// unsigned integers
qbs *qbs__tostr(uint64_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%" PRIu64, value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

qbs *qbs__tostr(uint32_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%u", value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

qbs *qbs__tostr(uint16_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%u", value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

qbs *qbs__tostr(uint8_t value, int32_t digits, int32_t passed) {
    (void)digits;
    (void)passed;
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%u", value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

// floating points
qbs *qbs__tostr(float value, int32_t digits, int32_t passed) {
    if (passed) {
        if (digits < 0) {
            error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
            return qbs_new_txt("");
        }
        if (digits < 1)
            digits = 1;
        if (digits > 7)
            digits = 7;
    } else {
        digits = 7;
    }
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%.*G", digits, value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    }
    return tqbs;
}

/**
 * @brief Converts a double-precision float to string (QB64 _TOSTR$ function)
 * @param value Double value to convert
 * @param digits Number of significant digits (1-16, default 16)
 * @param passed Flag indicating if digits parameter was provided
 * @return qbs string containing the string representation
 * @note Modern format: no leading space. Uses 'G' format, converts 'E' to 'D' for QBASIC compatibility.
 *       Generates error 5 if digits < 0. Clamps digits to 1-16 range.
 *       Caller must free the returned qbs with qbs_free().
 */
qbs *qbs__tostr(double value, int32_t digits, int32_t passed) {
    if (passed) {
        if (digits < 0) {
            error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
            return qbs_new_txt("");
        }
        if (digits < 1)
            digits = 1;
        if (digits > 16)
            digits = 16;
    } else {
        digits = 16;
    }
    qbs *tqbs = qbs_new(32, 1);
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%.*G", digits, value);
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    } else {
        char *ex = strrchr((char *)tqbs->chr, (int)'E');
        if (ex != NULL)
            ex[0] = 'D';
    }
    return tqbs;
}

/**
 * @brief Converts a long double to string (QB64 _TOSTR$ function)
 * @param value Long double value to convert
 * @param digits Number of significant digits (1-19, default 19)
 * @param passed Flag indicating if digits parameter was provided
 * @return qbs string containing the string representation
 * @note Modern format: no leading space. Uses 'LG' format, converts 'E' to 'F' for QBASIC compatibility.
 *       Generates error 5 if digits < 0. Clamps digits to 1-19 range.
 *       Caller must free the returned qbs with qbs_free().
 */
qbs *qbs__tostr(long double value, int32_t digits, int32_t passed) {
    if (passed) {
        if (digits < 0) {
            error(QB_ERROR_ILLEGAL_FUNCTION_CALL);
            return qbs_new_txt("");
        }
        if (digits < 1)
            digits = 1;
        if (digits > 19)
            digits = 19;
    } else {
        digits = 19;
    }
    qbs *tqbs = qbs_new(32, 1);
#ifdef QB64_MINGW
    tqbs->len = __mingw_snprintf((char *)tqbs->chr, 32, "%.*LG", digits, value);
#else
    tqbs->len = snprintf((char *)tqbs->chr, 32, "%.*LG", digits, value);
#endif
    if (tqbs->len < 0 || tqbs->len >= 32) {
        error(QB_ERROR_INTERNAL_ERROR);
        tqbs->len = 0;
    } else {
        char *ex = strrchr((char *)tqbs->chr, (int)'E');
        if (ex != NULL)
            ex[0] = 'F';
    }
    return tqbs;
}
