#ifndef INCLUDE_LIBQB_ERROR_HANDLE_H
#define INCLUDE_LIBQB_ERROR_HANDLE_H

#include "qbs.h"
#include <stdint.h>

/**
 * @file error_handle.h
 * @brief Error handling functions and error code definitions
 * 
 * This header provides error handling functionality for QB64-PE programs,
 * including error reporting, error codes, and error state management.
 */

/**
 * @brief Reports an error and initiates error handling
 * @param error_number Error code (see QB_ERROR_* constants)
 * @note This function sets the error state and may trigger error handlers or terminate the program.
 *       The behavior depends on the error code and current error handling mode.
 */
void error(int32_t error_number);

/**
 * @brief Checks if an error code is a critical out-of-memory error
 * @param error_number Error code to check
 * @return true if the error is a critical OOM error (257 or 502-518), false otherwise
 * @note Error 257 is the generic "Out of memory" error, while errors 502-518 are
 *       traceable OOM errors that help identify the specific allocation site.
 */
bool is_critical_oom_error(int32_t error_number);

/**
 * @brief Gets the error index for a critical out-of-memory error
 * @param error_number Error code (must be a critical OOM error)
 * @return Error index (1-18) for display in error message
 * @note Returns 1 for error 257, and (error_number - 500) for errors 502-518.
 *       This provides a sequential index for diagnostic purposes.
 */
int get_critical_oom_error_index(int32_t error_number);

/**
 * @brief Handles a critical out-of-memory error by displaying an alert and exiting
 * @param error_number Error code (must be a critical OOM error)
 * @note This function displays a GUI alert with the error index and exits the program.
 *       The error index helps developers identify which memory allocation failed.
 *       NOTE: This function calls exit() and does not return.
 */
void handle_critical_oom_error(int32_t error_number);

/**
 * @brief Attempts to fix or recover from the current error
 * @note This function is called during error recovery. Behavior depends on the error type
 *       and whether error handlers are active.
 */
void fix_error();

// FIXME: Should be removed in the future, use `is_error_pending()`.
//
// Some spots edit this directly to clear/restore an error, those sites should
// be examined for the best solution.
extern uint32_t new_error;
extern uint32_t error_err;
extern uint32_t error_occurred;
extern uint32_t error_goto_line;
extern qbs *error_handler_history;
extern uint32_t error_handling;
extern uint32_t error_retry;

/**
 * @brief Checks if an error is currently pending
 * @return true if an error is pending, false otherwise
 * @note This is the recommended way to check for pending errors.
 *       Prefer this over directly accessing the new_error variable.
 */
static inline bool is_error_pending() {
    return new_error != 0;
}

/**
 * @brief Clears the current error state
 * @note Resets all error flags and state. Use with caution.
 */
void clear_error();

/**
 * @brief Gets the error line number (extended)
 * @return Error line number as a double (for compatibility with QB64)
 * @note Returns the line number where the error occurred
 */
double get_error_erl();

/**
 * @brief Gets the current error code
 * @return Current error code (see QB_ERROR_* constants)
 * @note Returns 0 if no error is pending
 */
uint32_t get_error_err();

/**
 * @brief Gets the line number where the error occurred (QB64 _ERRORLINE function)
 * @return Line number where error occurred, or 0 if no error
 */
int32_t func__errorline();

/**
 * @brief Gets the line number in the include file where the error occurred
 * @return Line number in include file, or 0 if no error or not in include
 */
int32_t func__inclerrorline();

/**
 * @brief Gets the filename of the include file where the error occurred
 * @return qbs string containing the include filename, or empty string if not in include
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__inclerrorfile();

/**
 * @brief Gets the error message for a given error code
 * @param errorcode Error code to get message for (if passed > 0)
 * @param passed Flag indicating if errorcode parameter was provided
 * @return qbs string containing the error message
 * @note If passed is 0, returns message for current error. Caller must free with qbs_free()
 */
qbs *func__errormessage(int32_t errorcode, int32_t passed);

/**
 * @brief Sets error line information for error reporting
 * @param errorline Line number where error occurred
 * @param incerrorline Line number in include file (if applicable)
 * @param incfilename Filename of include file (if applicable, NULL if not in include)
 * @note This function is typically called by the compiler when reporting errors
 */
void error_set_line(uint32_t errorline, uint32_t incerrorline, const char *incfilename);

#define QB_ERROR_NEXT_WITHOUT_FOR 1
#define QB_ERROR_SYNTAX_ERROR 2
#define QB_ERROR_RETURN_WITHOUT_GOSUB 3
#define QB_ERROR_OUT_OF_DATA 4
#define QB_ERROR_ILLEGAL_FUNCTION_CALL 5
#define QB_ERROR_OVERFLOW 6
#define QB_ERROR_OUT_OF_MEMORY 7
#define QB_ERROR_LABEL_NOT_DEFINED 8
#define QB_ERROR_SUBSCRIPT_OUT_OF_RANGE 9
#define QB_ERROR_DUPLICATE_DEFINITION 10
#define QB_ERROR_DIVISION_BY_ZERO 11
#define QB_ERROR_ILLEGAL_IN_DIRECT_MODE 12
#define QB_ERROR_TYPE_MISMATCH 13
#define QB_ERROR_OUT_OF_STRING_SPACE 14
#define QB_ERROR_STRING_FORMULA_TOO_COMPLEX 16
#define QB_ERROR_CANNOT_CONTINUE 17
#define QB_ERROR_FUNCTION_NOT_DEFINED 18
#define QB_ERROR_NO_RESUME 19
#define QB_ERROR_RESUME_WITHOUT_ERROR 20
#define QB_ERROR_DEVICE_TIMEOUT 24
#define QB_ERROR_DEVICE_FAULT 25
#define QB_ERROR_FOR_WITHOUT_NEXT 26
#define QB_ERROR_OUT_OF_PAPER 27
#define QB_ERROR_WHILE_WITHOUT_WEND 29
#define QB_ERROR_WEND_WITHOUT_WHILE 30
#define QB_ERROR_DUPLICATE_LABEL 33
#define QB_ERROR_SUBPROGRAM_NOT_DEFINED 35
#define QB_ERROR_ARGUMENT_COUNT_MISMATCH 37
#define QB_ERROR_ARRAY_NOT_DEFINED 38
#define QB_ERROR_VARIABLE_REQUIRED 40
#define QB_ERROR_FIELD_OVERFLOW 50
#define QB_ERROR_INTERNAL_ERROR 51
#define QB_ERROR_BAD_FILE_NAME_OR_NUMBER 52
#define QB_ERROR_FILE_NOT_FOUND 53
#define QB_ERROR_BAD_FILE_MODE 54
#define QB_ERROR_FILE_ALREADY_OPEN 55
#define QB_ERROR_FIELD_STATEMENT_ACTIVE 56
#define QB_ERROR_DEVICE_IO_ERROR 57
#define QB_ERROR_FILE_ALREADY_EXISTS 58
#define QB_ERROR_BAD_RECORD_LENGTH 59
#define QB_ERROR_DISK_FULL 61
#define QB_ERROR_INPUT_PAST_END_OF_FILE 62
#define QB_ERROR_BAD_RECORD_NUMBER 63
#define QB_ERROR_BAD_FILE_NAME 64
#define QB_ERROR_TOO_MANY_FILES 67
#define QB_ERROR_DEVICE_UNAVAILABLE 68
#define QB_ERROR_COMMUNICATION_BUFFER_OVERFLOW 69
#define QB_ERROR_PERMISSION_DENIED 70
#define QB_ERROR_DISK_NOT_READY 71
#define QB_ERROR_DISK_MEDIA_ERROR 72
#define QB_ERROR_FEATURE_UNAVAILABLE 73
#define QB_ERROR_RENAME_ACROSS_DISKS 74
#define QB_ERROR_PATH_FILE_ACCESS_ERROR 75
#define QB_ERROR_PATH_NOT_FOUND 76
#define QB_ERROR_OUT_OF_STACK_SPACE 256
#define QB_ERROR_OUT_OF_MEMORY_FATAL 257
#define QB_ERROR_INVALID_HANDLE 258
#define QB_ERROR_CANNOT_FIND_DYNAMIC_LIBRARY_FILE 259
#define QB_ERROR_FUNCTION_NOT_FOUND_IN_DYNAMIC_LIBRARY 260
#define QB_ERROR_FUNCTION_NOT_FOUND_IN_DYNAMIC_LIBRARY_261 261
#define QB_ERROR_GL_COMMAND_OUTSIDE_SUB_GL_SCOPE 270
#define QB_ERROR_END_SYSTEM_IN_SUB_GL_SCOPE 271
#define QB_ERROR_MEMORY_REGION_OUT_OF_RANGE 300
#define QB_ERROR_INVALID_SIZE 301
#define QB_ERROR_SOURCE_MEMORY_REGION_OUT_OF_RANGE 302
#define QB_ERROR_DESTINATION_MEMORY_REGION_OUT_OF_RANGE 303
#define QB_ERROR_BOTH_MEMORY_REGIONS_OUT_OF_RANGE 304
#define QB_ERROR_SOURCE_MEMORY_FREED 305
#define QB_ERROR_DESTINATION_MEMORY_FREED 306
#define QB_ERROR_MEMORY_ALREADY_FREED 307
#define QB_ERROR_MEMORY_HAS_BEEN_FREED 308
#define QB_ERROR_MEMORY_NOT_INITIALIZED 309
#define QB_ERROR_SOURCE_MEMORY_NOT_INITIALIZED 310
#define QB_ERROR_DESTINATION_MEMORY_NOT_INITIALIZED 311
#define QB_ERROR_BOTH_MEMORY_NOT_INITIALIZED 312
#define QB_ERROR_BOTH_MEMORY_FREED 313
#define QB_ERROR_ASSERT_FAILED 314
#define QB_ERROR_ASSERT_FAILED_WITH_DESCRIPTION 315
#define QB_ERROR_OUT_OF_MEMORY_FATAL_502 502
#define QB_ERROR_OUT_OF_MEMORY_FATAL_503 503
#define QB_ERROR_OUT_OF_MEMORY_FATAL_504 504
#define QB_ERROR_OUT_OF_MEMORY_FATAL_505 505
#define QB_ERROR_OUT_OF_MEMORY_FATAL_506 506
#define QB_ERROR_OUT_OF_MEMORY_FATAL_507 507
#define QB_ERROR_OUT_OF_MEMORY_FATAL_508 508
#define QB_ERROR_OUT_OF_MEMORY_FATAL_509 509
#define QB_ERROR_OUT_OF_MEMORY_FATAL_510 510
#define QB_ERROR_OUT_OF_MEMORY_FATAL_511 511
#define QB_ERROR_OUT_OF_MEMORY_FATAL_512 512
#define QB_ERROR_OUT_OF_MEMORY_FATAL_513 513
#define QB_ERROR_OUT_OF_MEMORY_FATAL_514 514
#define QB_ERROR_OUT_OF_MEMORY_FATAL_515 515
#define QB_ERROR_OUT_OF_MEMORY_FATAL_516 516
#define QB_ERROR_OUT_OF_MEMORY_FATAL_517 517
#define QB_ERROR_OUT_OF_MEMORY_FATAL_518 518

#endif
