#ifndef INCLUDE_LIBQB_ROUNDING_H
#define INCLUDE_LIBQB_ROUNDING_H

#include "event.h"

#include <cmath>
#include <stdint.h>

/**
 * @file rounding.h
 * @brief Rounding and type conversion functions for QB64-PE
 * 
 * This header provides functions for rounding floating-point values and converting
 * between numeric types with QB64-compatible behavior. Includes platform-specific
 * implementations for x86 and non-x86 architectures.
 */

#ifdef QB64_NOT_X86

#    include <cfenv>

/**
 * @brief Rounds a long double to int64_t (QB64-compatible rounding)
 * @param f Floating-point value to round
 * @return Rounded 64-bit integer value
 * @note Uses nearbyint for rounding. Handles values larger than int64_t max.
 */
static inline int64_t qbr(long double f) {
    int temp = 0;
    if (f > 9223372036854775807.0L) {
        temp = 1;
        f = f - 9223372036854775808ULL;
    }
    auto i = int64_t(std::nearbyintl(f));
    if (temp) {
        return i | 0x8000000000000000;
    }
    return i;
}

/**
 * @brief Rounds a long double to uint64_t
 * @param f Floating-point value to round
 * @return Rounded 64-bit unsigned integer value
 * @note Uses nearbyint for rounding
 */
static inline uint64_t qbr_longdouble_to_uint64(long double f) {
    return uint64_t(std::nearbyintl(f));
}

/**
 * @brief Rounds a float to int32_t
 * @param f Floating-point value to round
 * @return Rounded 32-bit integer value
 * @note Uses nearbyint for rounding
 */
static inline int32_t qbr_float_to_long(float f) {
    return int32_t(std::nearbyintf(f));
}

/**
 * @brief Rounds a double to int32_t
 * @param f Floating-point value to round
 * @return Rounded 32-bit integer value
 * @note Uses nearbyint for rounding
 */
static inline int32_t qbr_double_to_long(double f) {
    return int32_t(std::nearbyint(f));
}

/**
 * @brief Reinitializes the FPU rounding mode
 * @note Sets rounding mode to nearest (FE_TONEAREST) for non-x86 platforms
 */
static inline void fpu_reinit() {
    std::fesetround(FE_TONEAREST);
}

#else

/**
 * @name x86 FPU Rounding Functions
 * @brief QBASIC-compatible rounding using x86 FPU instructions
 * @note Uses inline assembly for direct FPU control:
 *       FLDS = load single precision
 *       FLDL = load double precision
 *       FLDT = load long double precision
 */
///@{
/**
 * @brief Rounds a long double to int64_t using x86 FPU
 * @param f Floating-point value to round
 * @return Rounded 64-bit integer value
 * @note Uses x86 FPU FISTP instruction for QBASIC-compatible rounding
 */
static inline int64_t qbr(long double f) {
    int64_t i;
    int temp = 0;
    if (f > 9223372036854775807.0L) {
        temp = 1;
        f = f - 9223372036854775808ULL;
    } // if it's too large for a signed int64, make it an unsigned int64 and return that value if possible.
    __asm__("fldt %1;"
            "fistpll %0;"
            : "=m"(i)
            : "m"(f));
    if (temp) {
        return i | 0x8000000000000000; // if it's an unsigned int64, manually set the bit flag
    }
    return i;
}

/**
 * @brief Rounds a long double to uint64_t using x86 FPU
 * @param f Floating-point value to round
 * @return Rounded 64-bit unsigned integer value
 * @note Uses x86 FPU FISTP instruction
 */
static inline uint64_t qbr_longdouble_to_uint64(long double f) {
    uint64_t i;
    __asm__("fldt %1;"
            "fistpll %0;"
            : "=m"(i)
            : "m"(f));
    return i;
}

/**
 * @brief Rounds a float to int32_t using x86 FPU
 * @param f Floating-point value to round
 * @return Rounded 32-bit integer value
 * @note Uses x86 FPU FISTP instruction
 */
static inline int32_t qbr_float_to_long(float f) {
    int32_t i;
    __asm__("flds %1;"
            "fistpl %0;"
            : "=m"(i)
            : "m"(f));
    return i;
}

/**
 * @brief Rounds a double to int32_t using x86 FPU
 * @param f Floating-point value to round
 * @return Rounded 32-bit integer value
 * @note Uses x86 FPU FISTP instruction
 */
static inline int32_t qbr_double_to_long(double f) {
    int32_t i;
    __asm__("fldl %1;"
            "fistpl %0;"
            : "=m"(i)
            : "m"(f));
    return i;
}

/**
 * @brief Reinitializes the x86 FPU rounding mode
 * @note Sets FPU control word to 0x37F for QBASIC-compatible rounding
 */
static inline void fpu_reinit() {
    unsigned int mode = 0x37F;
    asm("fldcw %0" : : "m"(*&mode));
}
///@}

#endif // x86 support

/**
 * @name CSNG Functions (Convert to Single Precision)
 * @brief Convert values to single-precision floating-point
 */
///@{
/**
 * @brief Converts long double to single precision (QB64 CSNG function)
 * @param value Long double value to convert
 * @return Single-precision value, or 0 on overflow
 * @note Generates error 6 if value is outside single-precision range
 */
static inline double func_csng_float(long double value) {
    if ((value <= 3.402823466E38L) && (value >= -3.402823466E38L)) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @brief Converts double to single precision (QB64 CSNG function)
 * @param value Double value to convert
 * @return Single-precision value, or 0 on overflow
 * @note Generates error 6 if value is outside single-precision range
 */
static inline double func_csng_double(double value) {
    if ((value <= 3.402823466E38) && (value >= -3.402823466E38)) {
        return value;
    }
    error(6);
    return 0;
}
///@}

/**
 * @brief Converts long double to double precision (QB64 CDBL function)
 * @param value Long double value to convert
 * @return Double-precision value, or 0 on overflow
 * @note Generates error 6 if value is outside double-precision range
 */
static inline double func_cdbl_float(long double value) {
    if ((value <= 1.7976931348623157E308L) && (value >= -1.7976931348623157E308L)) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @name CINT Functions (Convert to Integer)
 * @brief Convert values to 16-bit integer with range checking
 * @note func_cint_single uses func_cint_double internally
 */
///@{
/**
 * @brief Converts double to 16-bit integer (QB64 CINT function)
 * @param value Double value to convert (must be in range -32768.5 to 32767.5)
 * @return 16-bit integer value, or 0 on error
 * @note Generates error 6 if value is outside the valid range
 */
static inline int32_t func_cint_double(double value) {
    if ((value < 32767.5) && (value >= -32768.5)) {
        return qbr_double_to_long(value);
    }
    error(6);
    return 0;
}

static inline int64_t func_cint_float(long double value) {
    if ((value < 32767.5L) && (value >= -32768.5L)) {
        return qbr(value);
    }
    error(6);
    return 0;
}

/**
 * @brief Converts int32_t to 16-bit integer (QB64 CINT function)
 * @param value 32-bit integer value to convert
 * @return 16-bit integer value, or 0 on error
 * @note Generates error 6 if value is outside the valid range
 */
static inline int16_t func_cint_long(int32_t value) {
    if ((value >= -32768) && (value <= 32767)) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @brief Converts uint32_t to 16-bit integer (QB64 CINT function)
 * @param value 32-bit unsigned integer value to convert
 * @return 16-bit integer value, or 0 on error
 * @note Generates error 6 if value > 32767
 */
static inline int16_t func_cint_ulong(uint32_t value) {
    if (value <= 32767) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @brief Converts int64_t to 16-bit integer (QB64 CINT function)
 * @param value 64-bit integer value to convert
 * @return 16-bit integer value, or 0 on error
 * @note Generates error 6 if value is outside the valid range
 */
static inline int16_t func_cint_int64(int64_t value) {
    if ((value >= -32768) && (value <= 32767)) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @brief Converts uint64_t to 16-bit integer (QB64 CINT function)
 * @param value 64-bit unsigned integer value to convert
 * @return 16-bit integer value, or 0 on error
 * @note Generates error 6 if value > 32767
 */
static inline int16_t func_cint_uint64(uint64_t value) {
    if (value <= 32767) {
        return value;
    }
    error(6);
    return 0;
}
///@}

/**
 * @name CLNG Functions (Convert to Long Integer)
 * @brief Convert values to 32-bit integer with range checking
 * @note func_clng_single uses func_clng_double internally
 * @note Valid range: -2147483648 to 2147483647
 */
///@{
/**
 * @brief Converts double to 32-bit integer (QB64 CLNG function)
 * @param value Double value to convert (must be in range -2147483648.5 to 2147483647.5)
 * @return 32-bit integer value, or 0 on error
 * @note Generates error 6 if value is outside the valid range
 */
static inline int32_t func_clng_double(double value) {
    if ((value < 2147483647.5) && (value >= -2147483648.5)) {
        return qbr_double_to_long(value);
    }
    error(6);
    return 0;
}

static inline int64_t func_clng_float(long double value) {
    if ((value < 2147483647.5L) && (value >= -2147483648.5L)) {
        return qbr(value);
    }
    error(6);
    return 0;
}

/**
 * @brief Converts uint32_t to 32-bit integer (QB64 CLNG function)
 * @param value 32-bit unsigned integer value to convert
 * @return 32-bit integer value, or 0 on error
 * @note Generates error 6 if value > 2147483647
 */
static inline int32_t func_clng_ulong(uint32_t value) {
    if (value <= 2147483647) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @brief Converts int64_t to 32-bit integer (QB64 CLNG function)
 * @param value 64-bit integer value to convert
 * @return 32-bit integer value, or 0 on error
 * @note Generates error 6 if value is outside the valid range
 */
static inline int32_t func_clng_int64(int64_t value) {
    if ((value >= -2147483648) && (value <= 2147483647)) {
        return value;
    }
    error(6);
    return 0;
}

/**
 * @brief Converts uint64_t to 32-bit integer (QB64 CLNG function)
 * @param value 64-bit unsigned integer value to convert
 * @return 32-bit integer value, or 0 on error
 * @note Generates error 6 if value > 2147483647
 */
static inline int32_t func_clng_uint64(uint64_t value) {
    if (value <= 2147483647) {
        return value;
    }
    error(6);
    return 0;
}
///@}

/**
 * @name _ROUND Functions
 * @brief Round floating-point values to integer (no error checking)
 */
///@{
/**
 * @brief Rounds a long double to int64_t (QB64 _ROUND function)
 * @param value Long double value to round
 * @return Rounded 64-bit integer value
 * @note Performs rounding without range checking or error generation
 */
static inline int64_t func_round_double(long double value) {
    return qbr(value);
}

/**
 * @brief Rounds a long double to int64_t (QB64 _ROUND function)
 * @param value Long double value to round
 * @return Rounded 64-bit integer value
 * @note Performs rounding without range checking or error generation
 */
static inline int64_t func_round_float(long double value) {
    return qbr(value);
}
///@}

#endif
