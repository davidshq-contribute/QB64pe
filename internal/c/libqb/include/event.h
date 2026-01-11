#ifndef INCLUDE_LIBQB_EVENT_H
#define INCLUDE_LIBQB_EVENT_H

#include <stddef.h>
#include <stdint.h>

/**
 * @file event.h
 * @brief Event handling functions for QB64-PE
 * 
 * This header provides functions for handling events and errors in QB64 programs.
 */

/**
 * @name Event Type Constants
 * @brief Event type identifiers for QB64 event system
 */
///@{
#define QB64_EVENT_CLOSE 1                      ///< Window close event
#define QB64_EVENT_KEY 2                        ///< Keyboard event
#define QB64_EVENT_RELATIVE_MOUSE_MOVEMENT 3    ///< Relative mouse movement event
#define QB64_EVENT_FILE_DROP 4                  ///< File drop event
///@}

/**
 * @brief Generates a runtime error
 * @param error_number Error code to generate
 * @note Triggers QB64 error handling with the specified error number
 */
void error(int32_t error_number);

/**
 * @brief Handles an event at a specific line number
 * @param linenumber Line number where the event occurred
 * @param inclinenumber Line number in included file (0 if not in included file)
 * @param incfilename Filename of included file (NULL if not in included file)
 * @note Processes events in QB64 programs. C++ version has default parameters.
 */
#ifdef __cplusplus
void evnt(uint32_t linenumber, uint32_t inclinenumber = 0, const char *incfilename = NULL);
extern "C" int qb64_custom_event(int event, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, void *p1, void *p2);
#else
void evnt(uint32_t linenumber, uint32_t inclinenumber, const char *incfilename);
int qb64_custom_event(int event, int v1, int v2, int v3, int v4, int v5, int v6, int v7, int v8, void *p1, void *p2);
#endif

/**
 * @brief Flag indicating a new error has occurred
 * @note Set when a new error is generated
 */
extern uint32_t new_error;

/**
 * @brief Current QB64 event value
 * @note Contains the current event type being processed
 */
extern uint32_t qbevent;

/**
 * @brief Custom event handler function
 * @param event Event type identifier
 * @param v1-v8 Event parameter values
 * @param p1-p2 Event parameter pointers
 * @return Non-zero if event was handled, 0 otherwise
 * @note Allows custom handling of QB64 events. Returns non-zero if the event was processed.
 */
// (Documented above in the #ifdef block)

#endif
