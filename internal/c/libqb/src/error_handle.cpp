
#include "libqb-common.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "error_handle.h"
#include "logging.h"
#include "event.h"
#include "gui.h"

/**
 * @file error_handle.cpp
 * @brief Implementation of error handling functions for QB64-PE
 * 
 * This file implements the error handling system, including error reporting,
 * error recovery, and user interaction for error conditions.
 */

/**
 * @brief Flag indicating a new error has occurred
 * @note Set when error() is called. Checked by error handlers.
 */
uint32_t new_error;

/**
 * @brief Flag indicating an error has occurred
 * @note Set during error handling to track error state.
 */
uint32_t error_occurred;

/**
 * @brief Flag indicating error retry is requested
 * @note Set when user chooses to retry after an error.
 */
uint32_t error_retry;

/**
 * @brief Current error code
 * @note Stores the error number of the current error.
 */
uint32_t error_err; //=0;

/**
 * @brief Line number to goto on error
 * @note Used by ON ERROR GOTO handlers.
 */
uint32_t error_goto_line;

/**
 * @brief History of error handlers
 * @note Tracks error handler call stack.
 */
qbs *error_handler_history;

/**
 * @brief Flag indicating error handling is in progress
 * @note Prevents recursive error handling.
 */
uint32_t error_handling;

/**
 * @brief Error line number (extended, as double)
 * @note Stores the line number where the error occurred.
 */
static double error_erl; //=0;

/**
 * @brief Error line number in current file
 * @note Line number where error occurred in the main file.
 */
static uint32_t ercl;

/**
 * @brief Error line number in included file
 * @note Line number where error occurred in an included file.
 */
static uint32_t inclercl;

/**
 * @brief Filename of included file where error occurred
 * @note NULL if error occurred in main file.
 */
static const char *includedfilename;

/**
 * @brief Converts an error code to a human-readable message
 * @param errorcode Error code number
 * @return Human-readable error message string
 * @note Returns a static string describing the error. Covers all QB64 error codes
 *       including memory errors (300-315) and standard QB64 errors (0-76, etc.).
 */
static const char *human_error(int32_t errorcode) {
    // clang-format off
    switch (errorcode) {
        case 0: return "No error";
        case 1: return "NEXT without FOR";
        case 2: return "Syntax error";
        case 3: return "RETURN without GOSUB";
        case 4: return "Out of DATA";
        case 5: return "Illegal function call";
        case 6: return "Overflow";
        case 7: return "Out of memory";
        case 8: return "Label not defined";
        case 9: return "Subscript out of range";
        case 10: return "Duplicate definition";
        case 12: return "Illegal in direct mode";
        case 13: return "Type mismatch";
        case 14: return "Out of string space";
        //error 15 undefined
        case 16: return "String formula too complex";
        case 17: return "Cannot continue";
        case 18: return "Function not defined";
        case 19: return "No RESUME";
        case 20: return "RESUME without error";
        //error 21-23 undefined
        case 24: return "Device timeout";
        case 25: return "Device fault";
        case 26: return "FOR without NEXT";
        case 27: return "Out of paper";
        //error 28 undefined
        case 29: return "WHILE without WEND";
        case 30: return "WEND without WHILE";
        //error 31-32 undefined
        case 33: return "Duplicate label";
        //error 34 undefined
        case 35: return "Subprogram not defined";
        //error 36 undefined
        case 37: return "Argument-count mismatch";
        case 38: return "Array not defined";
        case 40: return "Variable required";
        case 50: return "FIELD overflow";
        case 51: return "Internal error";
        case 52: return "Bad file name or number";
        case 53: return "File not found";
        case 54: return "Bad file mode";
        case 55: return "File already open";
        case 56: return "FIELD statement active";
        case 57: return "Device I/O error";
        case 58: return "File already exists";
        case 59: return "Bad record length";
        case 61: return "Disk full";
        case 62: return "Input past end of file";
        case 63: return "Bad record number";
        case 64: return "Bad file name";
        case 67: return "Too many files";
        case 68: return "Device unavailable";
        case 69: return "Communication-buffer overflow";
        case 70: return "Permission denied";
        case 71: return "Disk not ready";
        case 72: return "Disk-media error";
        case 73: return "Feature unavailable";
        case 74: return "Rename across disks";
        case 75: return "Path/File access error";
        case 76: return "Path not found";
        case 258: return "Invalid handle";
        case 300: return "Memory region out of range";
        case 301: return "Invalid size";
        case 302: return "Source memory region out of range";
        case 303: return "Destination memory region out of range";
        case 304: return "Source and destination memory regions out of range";
        case 305: return "Source memory has been freed";
        case 306: return "Destination memory has been freed";
        case 307: return "Memory already freed";
        case 308: return "Memory has been freed";
        case 309: return "Memory not initialized";
        case 310: return "Source memory not initialized";
        case 311: return "Destination memory not initialized";
        case 312: return "Source and destination memory not initialized";
        case 313: return "Source and destination memory have been freed";
        case 314: return "_ASSERT failed";
        case 315: return "_ASSERT failed (check console for description)";
        default: return "Unprintable error";
    }
    // clang-format on
}

/**
 * @brief Gets the error goto line number
 * @return Current error goto line number
 * @note Replaces direct access to error_goto_line variable
 */
uint32_t get_error_goto_line() {
    return error_goto_line;
}

/**
 * @brief Sets the error goto line number
 * @param line Line number to goto on error
 * @note Replaces direct access to error_goto_line variable
 */
void set_error_goto_line(uint32_t line) {
    error_goto_line = line;
}

/**
 * @brief Gets the error handling state
 * @return true if error handling is in progress, false otherwise
 * @note Replaces direct access to error_handling variable
 */
bool is_error_handling() {
    return error_handling != 0;
}

/**
 * @brief Sets the error handling state
 * @param handling true if error handling is in progress, false otherwise
 * @note Replaces direct access to error_handling variable
 */
void set_error_handling(bool handling) {
    error_handling = handling ? 1 : 0;
}

/**
 * @brief Gets the error occurred flag
 * @return true if an error has occurred, false otherwise
 * @note Replaces direct access to error_occurred variable
 */
bool get_error_occurred() {
    return error_occurred != 0;
}

/**
 * @brief Sets the error occurred flag
 * @param occurred true if an error has occurred, false otherwise
 * @note Replaces direct access to error_occurred variable
 */
void set_error_occurred(bool occurred) {
    error_occurred = occurred ? 1 : 0;
}

/**
 * @brief Gets the error retry flag
 * @return true if error retry is requested, false otherwise
 * @note Replaces direct access to error_retry variable
 */
bool get_error_retry() {
    return error_retry != 0;
}

/**
 * @brief Sets the error retry flag
 * @param retry true if error retry is requested, false otherwise
 * @note Replaces direct access to error_retry variable
 */
void set_error_retry(bool retry) {
    error_retry = retry ? 1 : 0;
}

/**
 * @brief Gets the error handler history
 * @return Pointer to error handler history string
 * @note Replaces direct access to error_handler_history variable
 */
qbs *get_error_handler_history() {
    return error_handler_history;
}

/**
 * @brief Sets the error handler history
 * @param history Pointer to error handler history string
 * @note Replaces direct access to error_handler_history variable
 */
void set_error_handler_history(qbs *history) {
    error_handler_history = history;
}

/**
 * @brief Sets the current error code
 * @param err Error code to set
 * @note Replaces direct access to error_err variable
 */
void set_error_err(uint32_t err) {
    error_err = err;
}

/**
 * @brief Clears the current error state
 * @note Resets the new_error flag. Use with caution.
 */
void clear_error() {
    new_error = 0;
}

/**
 * @brief Gets the error line number (extended)
 * @return Error line number as a double
 * @note Returns the line number where the error occurred.
 */
double get_error_erl() {
    return error_erl;
}

/**
 * @brief Gets the current error code
 * @return Current error code
 * @note Returns the error number of the current error.
 */
uint32_t get_error_err() {
    return error_err;
}

/**
 * @brief Gets the error line number (QB64 _ERRORLINE function)
 * @return Line number where error occurred in main file
 */
int32_t func__errorline() {
    return ercl;
}

/**
 * @brief Gets the error line number in included file (QB64 _INCLERRORLINE function)
 * @return Line number where error occurred in included file, or 0 if in main file
 */
int32_t func__inclerrorline() {
    return inclercl;
}

/**
 * @brief Gets the included filename where error occurred (QB64 _INCLERRORFILE$ function)
 * @return qbs string containing the included filename, or empty string if in main file
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__inclerrorfile() {
    return qbs_new_txt(includedfilename);
}

/**
 * @brief Sets the error line information
 * @param errorline Line number in main file
 * @param incerrorline Line number in included file
 * @param incfilename Filename of included file
 * @note Called when an error occurs to record location information.
 */
void error_set_line(uint32_t errorline, uint32_t incerrorline, const char *incfilename) {
    ercl = errorline;
    inclercl = incerrorline;
    includedfilename = incfilename;
}

/**
 * @brief Gets the error message for an error code (QB64 _ERRORMESSAGE$ function)
 * @param errorcode Error code (uses current error if not provided)
 * @param passed Flag indicating if errorcode parameter was provided
 * @return qbs string containing the error message
 * @note Caller must free the returned qbs with qbs_free()
 */
qbs *func__errormessage(int32_t errorcode, int32_t passed) {
    if (!passed)
        errorcode = get_error_err();
    return qbs_new_txt(human_error(errorcode));
}

extern uint8_t close_program;
extern double last_line;
void end();

extern void QBMAIN(void *);

/**
 * @brief Attempts to fix or recover from the current error
 * @note Handles error recovery and user interaction. For critical errors (300-315),
 *       displays an error dialog and exits. For other errors, may allow continuation
 *       or trigger error handlers. Generates error dialogs with continue/exit options.
 *       FIXME: Contains suspicious QBMAIN(NULL) call that may need review.
 */
void fix_error() {
    char *errtitle = NULL, *errmess = NULL;
    const char *cp;
    int prevent_handling = 0, len, v;
    if ((new_error >= 300) && (new_error <= 315))
        prevent_handling = 1;
    if (!get_error_goto_line() || is_error_handling() || prevent_handling) {
        // strip path from binary name
        static int32_t i;
        static qbs *binary_name = NULL;
        if (!binary_name)
            binary_name = qbs_new(0, 0);
        qbs_set(binary_name, qbs_add(func_command(0, 1), qbs_new_txt_len("\0", 1)));
        for (i = binary_name->len; i > 0; i--) {
            if ((binary_name->chr[i - 1] == 47) || (binary_name->chr[i - 1] == 92)) {
                qbs_set(binary_name, func_mid(binary_name, i + 1, 0, 0));
                break;
            }
        }

        cp = human_error(new_error);
#define FIXERRMSG_TITLE "%s%u - %s"
#define FIXERRMSG_BODY "Line: %u (in %s)\n%s%s"
#define FIXERRMSG_MAINFILE "main module"
#define FIXERRMSG_CONT "\nContinue?"
#define FIXERRMSG_UNHAND "Unhandled Error #"
#define FIXERRMSG_CRIT "Critical Error #"

        len = snprintf(errmess, 0, FIXERRMSG_BODY, (inclercl ? inclercl : ercl), (inclercl ? includedfilename : FIXERRMSG_MAINFILE), cp,
                       (!prevent_handling ? FIXERRMSG_CONT : ""));
        errmess = (char *)malloc(len + 1);
        if (!errmess)
            exit(0); // At this point we just give up
        snprintf(errmess, len + 1, FIXERRMSG_BODY, (inclercl ? inclercl : ercl), (inclercl ? includedfilename : FIXERRMSG_MAINFILE), cp,
                 (!prevent_handling ? FIXERRMSG_CONT : ""));

        len = snprintf(errtitle, 0, FIXERRMSG_TITLE, (!prevent_handling ? FIXERRMSG_UNHAND : FIXERRMSG_CRIT), new_error, binary_name->chr);
        errtitle = (char *)malloc(len + 1);
        if (!errtitle)
            exit(0); // At this point we just give up
        snprintf(errtitle, len + 1, FIXERRMSG_TITLE, (!prevent_handling ? FIXERRMSG_UNHAND : FIXERRMSG_CRIT), new_error, binary_name->chr);

        if (prevent_handling) {
            v = gui_alert(errmess, errtitle, "ok");
            exit(0);
        } else {
            v = gui_alert(errmess, errtitle, "yesno");
        }

        if ((v == 2) || (v == 0)) {
            close_program = 1;
            end();
        }
        new_error = 0;
        return;
    }
    error_err = new_error;
    new_error = 0;
    error_erl = last_line;
    set_error_occurred(true);

    // FIXME: EWWWWW, there's no way this is correct
    QBMAIN(NULL);
    return;
}

/**
 * @brief Checks if an error code is a critical out-of-memory error
 * @param error_number Error code to check
 * @return true if the error is a critical OOM error (257 or 502-518), false otherwise
 * @note Error 257 is the generic "Out of memory" error, while errors 502-518 are
 *       traceable OOM errors that help identify the specific allocation site.
 */
bool is_critical_oom_error(int32_t error_number) {
    return error_number == 257 || (error_number >= 502 && error_number <= 518);
}

/**
 * @brief Gets the error index for a critical out-of-memory error
 * @param error_number Error code (must be a critical OOM error)
 * @return Error index (1-18) for display in error message
 * @note Returns 1 for error 257, and (error_number - 500) for errors 502-518.
 *       This provides a sequential index for diagnostic purposes.
 */
int get_critical_oom_error_index(int32_t error_number) {
    if (error_number == 257) {
        return 1;
    }
    return error_number - 500;
}

/**
 * @brief Handles a critical out-of-memory error by displaying an alert and exiting
 * @param error_number Error code (must be a critical OOM error)
 * @note This function displays a GUI alert with the error index and exits the program.
 *       The error index helps developers identify which memory allocation failed.
 */
void handle_critical_oom_error(int32_t error_number) {
    int error_index = get_critical_oom_error_index(error_number);
    char message[64];
    snprintf(message, sizeof(message), "Critical Error #%d", error_index);
    gui_alert("Out of memory", message, "ok");
    exit(0);
}

/**
 * @brief Reports an error and initiates error handling
 * @param error_number Error code (see error code constants)
 * @note This is the main error reporting function. It:
 *       - Logs the error
 *       - Handles critical errors (out of memory, etc.) by displaying dialogs and exiting
 *       - Sets new_error flag for error handlers
 *       - May trigger program termination for critical errors
 */
void error(int32_t error_number) {
    libqb_log_error("QB64 Error %d reported: %s", error_number, human_error(error_number));

    // critical errors:

    // Handle all out of memory errors (257, 502-518)
    if (is_critical_oom_error(error_number)) {
        handle_critical_oom_error(error_number);
    }

    // other critical errors
    static const struct {
        int error_code;
        const char* title;
        const char* message;
    } critical_errors[] = {
        {11, "Division by zero", "Critical Error"},
        {256, "Out of stack space", "Critical Error"},
        {259, "Cannot find dynamic library file", "Critical Error"},
        {260, "Sub/Function does not exist in dynamic library", "Critical Error"},
        {261, "Sub/Function does not exist in dynamic library", "Critical Error"},
        {270, "_GL command called outside of SUB _GL's scope", "Critical Error"},
        {271, "END/SYSTEM called within SUB _GL's scope", "Critical Error"}
    };
    
    for (size_t i = 0; i < sizeof(critical_errors) / sizeof(critical_errors[0]); i++) {
        if (error_number == critical_errors[i].error_code) {
            gui_alert(critical_errors[i].title, critical_errors[i].message, "ok");
            exit(0);
        }
    }

    if (!new_error) {
        if ((new_error == 256) || (new_error == 257))
            fix_error(); // critical error!
        if (error_number <= 0)
            error_number = 5; // Illegal function call
        new_error = error_number;
        qbevent = 1;
    }
}
