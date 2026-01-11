#pragma once

#include "error_handle.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <type_traits>

/**
 * @file extended_math.h
 * @brief Extended mathematical functions for QB64-PE
 * 
 * This header provides additional mathematical functions including angle conversions,
 * trigonometric functions, and utility functions beyond the standard math library.
 */

/**
 * @name Angle Conversion Functions
 * @brief Convert between degrees, radians, and gradians
 */
///@{
/**
 * @brief Converts degrees to radians
 * @param value Angle in degrees
 * @return Angle in radians
 */
static inline constexpr double func_deg2rad(double value) {
    return (value * 0.01745329251994329576923690768489);
}

/**
 * @brief Converts radians to degrees
 * @param value Angle in radians
 * @return Angle in degrees
 */
static inline constexpr double func_rad2deg(double value) {
    return (value * 57.29577951308232);
}

/**
 * @brief Converts degrees to gradians
 * @param value Angle in degrees
 * @return Angle in gradians
 */
static inline constexpr double func_deg2grad(double value) {
    return (value * 1.111111111111111);
}

/**
 * @brief Converts gradians to degrees
 * @param value Angle in gradians
 * @return Angle in degrees
 */
static inline constexpr double func_grad2deg(double value) {
    return (value * 0.9);
}

/**
 * @brief Converts radians to gradians
 * @param value Angle in radians
 * @return Angle in gradians
 */
static inline constexpr double func_rad2grad(double value) {
    return (value * 63.66197723675816);
}

/**
 * @brief Converts gradians to radians
 * @param value Angle in gradians
 * @return Angle in radians
 */
static inline constexpr double func_grad2rad(double value) {
    return (value * 0.01570796326794896);
}
///@}

/**
 * @brief Gets the value of π (pi) with optional multiplier
 * @param multiplier Optional multiplier for π
 * @param passed Flag indicating if multiplier was provided
 * @return Value of π, or π * multiplier if multiplier is provided
 */
static inline constexpr double func_pi(double multiplier, int32_t passed) {
    if (passed) {
        return 3.14159265358979323846264338327950288419716939937510582 * multiplier;
    }
    return (3.14159265358979323846264338327950288419716939937510582);
}

/**
 * @name Inverse Trigonometric Functions
 * @brief Inverse secant, cosecant, and cotangent functions
 */
///@{
/**
 * @brief Computes inverse secant (arcsecant)
 * @param num Input value (must be |num| >= 1.0)
 * @return Inverse secant in radians, or 0.0 on error
 * @note Generates error 5 if |num| < 1.0
 */
static inline double func_arcsec(double num) {
    if (std::abs(num) < 1.0) {
        error(5);
        return 0.0;
    }
    return std::acos(1.0 / num);
}

/**
 * @brief Computes inverse cosecant (arccosecant)
 * @param num Input value (must be |num| >= 1.0)
 * @return Inverse cosecant in radians, or 0.0 on error
 * @note Generates error 5 if |num| < 1.0
 */
static inline double func_arccsc(double num) {
    if (std::abs(num) < 1.0) {
        error(5);
        return 0.0;
    }
    return std::asin(1.0 / num);
}

/**
 * @brief Computes inverse cotangent (arccotangent)
 * @param num Input value
 * @return Inverse cotangent in radians
 */
static inline double func_arccot(double num) {
    return 2 * std::atan(1) - std::atan(num);
}
///@}

/**
 * @name Hyperbolic Trigonometric Functions
 * @brief Hyperbolic secant, cosecant, and cotangent functions
 */
///@{
/**
 * @brief Computes hyperbolic secant
 * @param num Input value (must be <= 88.02969)
 * @return Hyperbolic secant, or 0 on error
 * @note Generates error 5 if num > 88.02969 or if division by zero would occur
 */
static inline double func_sech(double num) {
    if (num > 88.02969) {
        error(5);
        return 0;
    }
    if (std::exp(num) + std::exp(-num) == 0) {
        error(5);
        return 0;
    }
    return 2 / (std::exp(num) + std::exp(-num));
}

/**
 * @brief Computes hyperbolic cosecant
 * @param num Input value (must be <= 88.02969, and != 0)
 * @return Hyperbolic cosecant, or 0 on error
 * @note Generates error 5 if num > 88.02969 or if division by zero would occur
 */
static inline double func_csch(double num) {
    if (num > 88.02969) {
        error(5);
        return 0;
    }
    if (std::exp(num) - std::exp(-num) == 0) {
        error(5);
        return 0;
    }
    return 2 / (std::exp(num) - std::exp(-num));
}

/**
 * @brief Computes hyperbolic cotangent
 * @param num Input value (must be <= 44.014845)
 * @return Hyperbolic cotangent, or 0 on error
 * @note Generates error 5 if num > 44.014845 or if division by zero would occur
 */
static inline double func_coth(double num) {
    if (num > 44.014845) {
        error(5);
        return 0;
    }
    if (2 * std::exp(num) - 1 == 0) {
        error(5);
        return 0;
    }
    return 2 * std::exp(num) - 1;
}
///@}

/**
 * @name Trigonometric Functions
 * @brief Secant, cosecant, and cotangent functions
 */
///@{
/**
 * @brief Computes secant (1/cosine)
 * @param num Angle in radians
 * @return Secant value, or 0 on error
 * @note Generates error 5 if cosine is zero (division by zero)
 */
static inline double func_sec(double num) {
    if (std::cos(num) == 0) {
        error(5);
        return 0;
    }
    return 1 / std::cos(num);
}

/**
 * @brief Computes cosecant (1/sine)
 * @param num Angle in radians
 * @return Cosecant value, or 0 on error
 * @note Generates error 5 if sine is zero (division by zero)
 */
static inline double func_csc(double num) {
    if (std::sin(num) == 0) {
        error(5);
        return 0;
    }
    return 1 / std::sin(num);
}

/**
 * @brief Computes cotangent (1/tangent)
 * @param num Angle in radians
 * @return Cotangent value, or 0 on error
 * @note Generates error 5 if tangent is zero (division by zero)
 */
static inline double func_cot(double num) {
    if (std::tan(num) == 0) {
        error(5);
        return 0;
    }
    return 1 / std::tan(num);
}
///@}

/**
 * @brief Clamps a value between two limits
 * @tparam T Arithmetic type
 * @param value Value to clamp
 * @param limit1 One end of the clamping range
 * @param limit2 The other end of the clamping range
 * @return Clamped value between limit1 and limit2, regardless of their order
 * @note Handles reversed limits safely (unlike std::clamp which throws on reversed limits).
 *       Works correctly whether limit1 < limit2 or limit2 < limit1.
 */
template <typename T> static inline constexpr T func_clamp(T value, T limit1, T limit2) {
    static_assert(std::is_arithmetic_v<T>, "func_clamp requires an arithmetic type");

    return (limit1 > limit2) ? (value < limit2 ? limit2 : (value > limit1 ? limit1 : value)) : (value < limit1 ? limit1 : (value > limit2 ? limit2 : value));
}

/**
 * @name Power-of-2 Utility Functions
 * @brief Functions for working with powers of 2
 */
///@{
/**
 * @brief Checks if a number is a power of 2
 * @tparam T Integral type
 * @param n Number to check
 * @return true if n is a power of 2, false otherwise
 * @note Returns false for 0
 */
template <typename T> static inline constexpr bool Math_IsPowerOf2(T n) {
    static_assert(std::is_integral_v<T>, "Math_IsPowerOf2 requires an integral type");

    using UT = std::make_unsigned_t<T>;

    UT un = static_cast<UT>(n);

    return un && !(un & (un - 1));
}

/**
 * @brief Rounds a number up to the nearest power of 2
 * @tparam T Integral type
 * @param n Number to round up
 * @return Next power of 2 greater than or equal to n
 * @note If n is already a power of 2, returns n. If n is 0, returns 1.
 */
template <typename T> static inline constexpr T Math_RoundUpToPowerOf2(T n) {
    static_assert(std::is_integral_v<T>, "Math_RoundUpToPowerOf2 requires an integral type");

    using UT = std::make_unsigned_t<T>;

    UT un = static_cast<UT>(n - 1);

    if constexpr (sizeof(UT) >= 1) {
        un |= un >> 1;
        un |= un >> 2;
        un |= un >> 4;
    }
    if constexpr (sizeof(UT) >= 2) {
        un |= un >> 8;
    }
    if constexpr (sizeof(UT) >= 4) {
        un |= un >> 16;
    }
    if constexpr (sizeof(UT) >= 8) {
        un |= un >> 32;
    }

    return static_cast<T>(un + 1);
}

/**
 * @brief Rounds a number down to the nearest power of 2
 * @tparam T Integral type
 * @param n Number to round down
 * @return Largest power of 2 less than or equal to n
 * @note If n is 0, returns 0
 */
template <typename T> static inline constexpr T Math_RoundDownToPowerOf2(T n) {
    static_assert(std::is_integral_v<T>, "Math_RoundDownToPowerOf2 requires an integral type");

    using UT = std::make_unsigned_t<T>;

    UT un = static_cast<UT>(n);

    if constexpr (sizeof(UT) >= 1) {
        un |= un >> 1;
        un |= un >> 2;
        un |= un >> 4;
    }
    if constexpr (sizeof(UT) >= 2) {
        un |= un >> 8;
    }
    if constexpr (sizeof(UT) >= 4) {
        un |= un >> 16;
    }
    if constexpr (sizeof(UT) >= 8) {
        un |= un >> 32;
    }

    return static_cast<T>(un - (un >> 1));
}
///@}