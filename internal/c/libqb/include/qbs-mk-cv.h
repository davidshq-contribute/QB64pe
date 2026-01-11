#pragma once

#include <stdint.h>

#include "qbs.h"

/**
 * @file qbs-mk-cv.h
 * @brief String conversion functions for QB64-PE
 * 
 * This header provides functions for converting between numeric types and qbs strings,
 * including Microsoft Binary Format (MBF) conversions and type-to-string/string-to-type
 * conversions for all QB64 numeric types.
 */

/**
 * @name Microsoft Binary Format Conversion Functions
 * @brief Convert to/from Microsoft Binary Format (MBF) strings
 */
///@{
/**
 * @brief Makes a single-precision MBF string (QB64 MKSMBF$ function)
 * @param val Single-precision float value
 * @return qbs string containing MBF representation, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func_mksmbf(float val);

/**
 * @brief Makes a double-precision MBF string (QB64 MKDMBF$ function)
 * @param val Double-precision float value
 * @return qbs string containing MBF representation, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func_mkdmbf(double val);

/**
 * @brief Converts a single-precision MBF string to float (QB64 CVSMBF function)
 * @param str qbs string containing MBF data
 * @return Converted float value
 */
float func_cvsmbf(qbs *str);

/**
 * @brief Converts a double-precision MBF string to double (QB64 CVDMBF function)
 * @param str qbs string containing MBF data
 * @return Converted double value
 */
double func_cvdmbf(qbs *str);
///@}

/**
 * @name Type-to-String Conversion Functions
 * @brief Convert numeric types to qbs strings
 */
///@{
/**
 * @brief Converts a signed byte to string
 * @param v Signed byte value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *b2string(char v);

/**
 * @brief Converts an unsigned byte to string
 * @param v Unsigned byte value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *ub2string(char v);

/**
 * @brief Converts a signed 16-bit integer to string
 * @param v Signed 16-bit integer value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *i2string(int16_t v);

/**
 * @brief Converts an unsigned 16-bit integer to string
 * @param v Unsigned 16-bit integer value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *ui2string(int16_t v);

/**
 * @brief Converts a signed 32-bit integer to string
 * @param v Signed 32-bit integer value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *l2string(int32_t v);

/**
 * @brief Converts an unsigned 32-bit integer to string
 * @param v Unsigned 32-bit integer value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *ul2string(uint32_t v);

/**
 * @brief Converts a signed 64-bit integer to string
 * @param v Signed 64-bit integer value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *i642string(int64_t v);

/**
 * @brief Converts an unsigned 64-bit integer to string
 * @param v Unsigned 64-bit integer value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *ui642string(uint64_t v);

/**
 * @brief Converts a single-precision float to string
 * @param v Single-precision float value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *s2string(float v);

/**
 * @brief Converts a double-precision float to string
 * @param v Double-precision float value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *d2string(double v);

/**
 * @brief Converts a long double to string
 * @param v Long double value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *f2string(long double v);

/**
 * @brief Converts a signed pointer/offset to string
 * @param v Signed pointer/offset value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *o2string(intptr_t v);

/**
 * @brief Converts an unsigned pointer/offset to string
 * @param v Unsigned pointer/offset value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *uo2string(uintptr_t v);

/**
 * @brief Converts a signed bit field to string
 * @param bsize Bit size of the field
 * @param v Signed bit field value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *bit2string(uint32_t bsize, int64_t v);

/**
 * @brief Converts an unsigned bit field to string
 * @param bsize Bit size of the field
 * @param v Unsigned bit field value
 * @return qbs string containing the value, or NULL on failure
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *ubit2string(uint32_t bsize, uint64_t v);
///@}

/**
 * @name String-to-Type Conversion Functions
 * @brief Convert qbs strings to numeric types
 */
///@{
/**
 * @brief Converts a string to a signed byte
 * @param str qbs string to convert
 * @return Converted signed byte value
 */
char string2b(qbs *str);

/**
 * @brief Converts a string to an unsigned byte
 * @param str qbs string to convert
 * @return Converted unsigned byte value
 */
uint8_t string2ub(qbs *str);

/**
 * @brief Converts a string to a signed 16-bit integer
 * @param str qbs string to convert
 * @return Converted signed 16-bit integer value
 */
int16_t string2i(qbs *str);

/**
 * @brief Converts a string to an unsigned 16-bit integer
 * @param str qbs string to convert
 * @return Converted unsigned 16-bit integer value
 */
uint16_t string2ui(qbs *str);

/**
 * @brief Converts a string to a signed 32-bit integer
 * @param str qbs string to convert
 * @return Converted signed 32-bit integer value
 */
int32_t string2l(qbs *str);

/**
 * @brief Converts a string to an unsigned 32-bit integer
 * @param str qbs string to convert
 * @return Converted unsigned 32-bit integer value
 */
uint32_t string2ul(qbs *str);

/**
 * @brief Converts a string to a signed 64-bit integer
 * @param str qbs string to convert
 * @return Converted signed 64-bit integer value
 */
int64_t string2i64(qbs *str);

/**
 * @brief Converts a string to an unsigned 64-bit integer
 * @param str qbs string to convert
 * @return Converted unsigned 64-bit integer value
 */
uint64_t string2ui64(qbs *str);

/**
 * @brief Converts a string to a single-precision float
 * @param str qbs string to convert
 * @return Converted single-precision float value
 */
float string2s(qbs *str);

/**
 * @brief Converts a string to a double-precision float
 * @param str qbs string to convert
 * @return Converted double-precision float value
 */
double string2d(qbs *str);

/**
 * @brief Converts a string to a long double
 * @param str qbs string to convert
 * @return Converted long double value
 */
long double string2f(qbs *str);

/**
 * @brief Converts a string to a signed pointer/offset
 * @param str qbs string to convert
 * @return Converted signed pointer/offset value
 */
intptr_t string2o(qbs *str);

/**
 * @brief Converts a string to an unsigned pointer/offset
 * @param str qbs string to convert
 * @return Converted unsigned pointer/offset value
 */
uintptr_t string2uo(qbs *str);

/**
 * @brief Converts a string to an unsigned bit field
 * @param str qbs string to convert
 * @param bsize Bit size of the field
 * @return Converted unsigned bit field value
 */
uint64_t string2ubit(qbs *str, uint32_t bsize);

/**
 * @brief Converts a string to a signed bit field
 * @param str qbs string to convert
 * @param bsize Bit size of the field
 * @return Converted signed bit field value
 */
int64_t string2bit(qbs *str, uint32_t bsize);
///@}