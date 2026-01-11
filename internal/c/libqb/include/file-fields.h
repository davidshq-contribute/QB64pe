#pragma once

#include <stdint.h>

#include "qbs.h"

/**
 * @file file-fields.h
 * @brief File field operations for QB64-PE
 * 
 * This header provides functions for managing fields in RANDOM access files.
 * Fields are used to define the structure of records in QB64 RANDOM files.
 */

/**
 * @brief Sets the left/right field alignment for a string
 * @param str qbs string to set field alignment for
 * @note Configures how the field is aligned when reading/writing
 */
void lrset_field(qbs *str);

/**
 * @brief Frees a field structure
 * @param str qbs string associated with the field
 * @note Releases resources associated with the field
 */
void field_free(qbs *str);

/**
 * @brief Creates a new field for a file
 * @param fileno File number to create the field for
 * @note Initializes field structures for a RANDOM access file
 */
void field_new(int32_t fileno);

/**
 * @brief Updates field information for a file
 * @param fileno File number to update
 * @note Refreshes field data after file operations
 */
void field_update(int32_t fileno);

/**
 * @brief Adds a field to a file
 * @param str qbs string defining the field
 * @param size Size of the field in bytes
 * @note Adds a new field definition to the file's field list
 */
void field_add(qbs *str, int64_t size);

/**
 * @brief Gets field data from a file (QB64 FIELD GET statement)
 * @param fileno File number
 * @param offset Offset within the record
 * @param passed Flag indicating if offset parameter was provided
 * @note Reads field data from the current record position
 */
void field_get(int32_t fileno, int64_t offset, int32_t passed);

/**
 * @brief Puts field data into a file (QB64 FIELD PUT statement)
 * @param fileno File number
 * @param offset Offset within the record
 * @param passed Flag indicating if offset parameter was provided
 * @note Writes field data to the current record position
 */
void field_put(int32_t fileno, int64_t offset, int32_t passed);
