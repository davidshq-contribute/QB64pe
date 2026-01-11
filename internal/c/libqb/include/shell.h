#pragma once

#include "qbs.h"
#include <stdint.h>

/**
 * @file shell.h
 * @brief Shell command execution functions for QB64-PE
 * 
 * This header provides functions for executing shell commands and system programs.
 */

/**
 * @brief Flag indicating a shell call is currently in progress
 * @note Set to non-zero while a shell command is executing
 */
extern int32_t shell_call_in_progress;

/**
 * @brief Executes a shell command and returns the exit code (QB64 SHELL function)
 * @param str qbs string containing the command to execute
 * @return Exit code of the executed command, or error code on failure
 * @note Executes the command and waits for it to complete. Returns the program's exit code.
 */
int64_t func_shell(qbs *str);

/**
 * @brief Executes a shell command without showing the window (QB64 _SHELLHIDE function)
 * @param str qbs string containing the command to execute
 * @return Exit code of the executed command, or error code on failure
 * @note Executes the command in a hidden window. Returns the program's exit code.
 */
int64_t func__shellhide(qbs *str);

/**
 * @brief Executes a shell command (QB64 SHELL statement variant 1)
 * @param str qbs string containing the command to execute
 * @param passed Flag indicating if parameter was provided
 * @note Executes the command. Behavior may vary based on the passed flag.
 */
void sub_shell(qbs *str, int32_t passed);

/**
 * @brief Executes a shell command (QB64 SHELL statement variant 2)
 * @param str qbs string containing the command to execute
 * @param passed Flag indicating if parameter was provided
 * @note Executes the command. Behavior may vary based on the passed flag.
 */
void sub_shell2(qbs *str, int32_t passed);

/**
 * @brief Executes a shell command (QB64 SHELL statement variant 3)
 * @param str qbs string containing the command to execute
 * @param passed Flag indicating if parameter was provided
 * @note Executes the command. Behavior may vary based on the passed flag.
 */
void sub_shell3(qbs *str, int32_t passed);

/**
 * @brief Executes a shell command (QB64 SHELL statement variant 4)
 * @param str qbs string containing the command to execute
 * @param passed Flag indicating if parameter was provided
 * @note Executes the command. Behavior may vary based on the passed flag.
 */
void sub_shell4(qbs *str, int32_t passed);
