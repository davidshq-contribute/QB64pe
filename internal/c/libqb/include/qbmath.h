#pragma once

#include "error_handle.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

/**
 * @file qbmath.h
 * @brief Mathematical functions for QB64-PE
 * 
 * This header provides mathematical functions including logarithms, exponentials,
 * square roots, absolute values, and sign functions with QB64-compatible error handling.
 */

/**
 * @brief Computes natural logarithm (QB64 LOG function)
 * @param value Value to compute logarithm of (must be > 0)
 * @return Natural logarithm of value, or 0 on error
 * @note Generates error 5 if value <= 0
 */
static inline double func_log(double value) {
    if (value <= 0) {
        error(5);
        return 0;
    }
    return std::log(value);
}

/**
 * @name FIX Functions
 * @brief Truncate floating-point values toward zero
 */
///@{
/**
 * @brief Truncates a value toward zero (QB64 FIX function for DOUBLE)
 * @tparam T Numeric type
 * @param value Value to truncate
 * @return Truncated integer value
 * @note For floating-point types, truncates toward zero. For integers, returns unchanged.
 */
template <typename T> static inline T func_fix_double(T value) {
    if constexpr (std::is_floating_point_v<T>) {
        return (value < 0) ? std::ceil(value) : std::floor(value);
    } else {
        return value;
    }
}

/**
 * @brief Truncates a value toward zero (QB64 FIX function for FLOAT)
 * @tparam T Numeric type
 * @param value Value to truncate
 * @return Truncated integer value
 * @note For floating-point types, truncates toward zero. For integers, returns unchanged.
 */
template <typename T> static inline T func_fix_float(T value) {
    if constexpr (std::is_floating_point_v<T>) {
        return (value < 0) ? std::ceil(value) : std::floor(value);
    } else {
        return value;
    }
}
///@}

/**
 * @name EXP Functions
 * @brief Compute exponential (e^x)
 */
///@{
/**
 * @brief Computes e raised to the power of value (QB64 EXP function for SINGLE)
 * @param value Exponent value
 * @return e^value, or 0 on overflow
 * @note Generates error 6 if value > 88.02969 (would overflow)
 */
static inline double func_exp_single(double value) {
    if (value <= 88.02969) {
        return std::exp(value);
    }
    error(6);
    return 0;
}

/**
 * @brief Computes e raised to the power of value (QB64 EXP function for FLOAT)
 * @param value Exponent value
 * @return e^value, or 0 on overflow
 * @note Generates error 6 if value > 709.782712893 (would overflow)
 */
static inline long double func_exp_float(long double value) {
    if (value <= 709.782712893) {
        return std::exp(value);
    }
    error(6);
    return 0;
}
///@}

/**
 * @brief Computes square root (QB64 SQR function)
 * @param value Value to compute square root of (must be >= 0)
 * @return Square root of value, or 0 on error
 * @note Generates error 5 if value < 0
 */
static inline double func_sqr(double value) {
    if (value < 0) {
        error(5);
        return 0;
    }
    return std::sqrt(value);
}

/**
 * @brief Computes x raised to the power of y (QB64 ^ operator)
 * @param x Base value
 * @param y Exponent value
 * @return x^y, or 0 on error
 * @note Generates error 5 if x < 0 and y is not an integer
 */
static inline long double pow2(long double x, long double y) {
    if (x < 0) {
        if (y != std::floor(y)) {
            error(5);
            return 0;
        }
    }
    return std::pow(x, y);
}

/**
 * @brief Computes absolute value (QB64 ABS function)
 * @tparam T Arithmetic type
 * @param v Value to compute absolute value of
 * @return Absolute value of v
 * @note For unsigned types, returns the value unchanged. For signed types, returns the positive value.
 */
template <typename T> static inline T func_abs(T v) {
    static_assert(std::is_arithmetic_v<T>, "func_abs requires an arithmetic type");

    if constexpr (std::is_unsigned_v<T>) {
        return v;
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::fabs(v);
    } else if constexpr (sizeof(T) <= sizeof(int32_t)) {
        return std::abs(v);
    } else {
        return std::llabs(v);
    }
}

/**
 * @brief Computes sign of a value (QB64 SGN function)
 * @tparam T Arithmetic type
 * @param v Value to compute sign of
 * @return -1 if v < 0, 0 if v == 0, 1 if v > 0
 * @note For unsigned types, returns 0 if v == 0, otherwise 1
 */
template <typename T> static inline constexpr int32_t func_sgn(T v) {
    static_assert(std::is_arithmetic_v<T>, "func_sgn requires an arithmetic type");

    if constexpr (std::is_unsigned_v<T>) {
        return v != 0 ? 1 : 0;
    } else {
        return (T(0) < v) - (v < T(0));
    }
}
