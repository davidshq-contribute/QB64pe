#pragma once

#include "qbs.h"
#include <stdint.h>

/**
 * @file environ.h
 * @brief Environment variable functions for QB64-PE
 * 
 * This header provides functions for reading and setting environment variables.
 */

/**
 * @brief Gets the number of environment variables (QB64 _ENVIRONCOUNT function)
 * @return Number of environment variables
 * @note Returns the count of environment variables available in the system
 */
int32_t func__environcount();

/**
 * @name Environment Variable Access Functions
 * @brief Get environment variable values
 */
///@{
/**
 * @brief Gets an environment variable by name (QB64 ENVIRON$ function)
 * @param name qbs string containing the environment variable name
 * @return qbs string containing the environment variable value, or empty string if not found
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func_environ(qbs *name);

/**
 * @brief Gets an environment variable by index (QB64 ENVIRON$ function)
 * @param number Index of the environment variable (0-based)
 * @return qbs string containing "NAME=VALUE" format, or empty string if index is invalid
 * @note Returns the environment variable in "NAME=VALUE" format. Caller must free with qbs_free()
 */
qbs *func_environ(int32_t number);
///@}

/**
 * @brief Sets an environment variable (QB64 ENVIRON statement)
 * @param str qbs string containing "NAME=VALUE" format to set
 * @note Sets or modifies an environment variable. Format must be "NAME=VALUE"
 */
void sub_environ(qbs *str);
