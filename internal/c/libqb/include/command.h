#pragma once

#include <stdint.h>

#include "qbs.h"

/**
 * @file command.h
 * @brief Command-line argument handling for QB64-PE
 * 
 * This header provides functions for accessing command-line arguments passed to the program.
 */

/**
 * @brief Global string containing the command-line string
 * @note This is set during program initialization from command-line arguments
 */
extern qbs *func_command_str;

/**
 * @brief Gets a command-line argument by index (QB64 COMMAND$ function)
 * @param index Index of the argument to retrieve (0-based)
 * @param passed Flag indicating if index parameter was provided
 * @return qbs string containing the command-line argument, or NULL on failure
 * @note If index is not provided, returns the full command-line string. Caller must free with qbs_free()
 */
qbs *func_command(int32_t index, int32_t passed);

/**
 * @brief Gets the number of command-line arguments (QB64 _COMMANDCOUNT function)
 * @return Number of command-line arguments
 * @note Returns the count of arguments passed to the program (excluding the program name)
 */
int32_t func__commandcount();

/**
 * @brief Initializes the command-line argument system
 * @param argc Number of command-line arguments
 * @param argv Array of command-line argument strings
 * @note Should be called at program startup to initialize command-line argument handling
 */
void command_initialize(int argc, char **argv);
