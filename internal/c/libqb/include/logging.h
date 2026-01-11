#pragma once

#include <stdint.h>

#include "qbs.h"

/**
 * @file logging.h
 * @brief Logging system for QB64-PE
 * 
 * This header provides a logging system with different log levels and scopes
 * for debugging and diagnostic purposes.
 */

/**
 * @enum loglevel
 * @brief Log level enumeration
 */
enum class loglevel {
    Trace,       ///< Trace-level messages (most verbose)
    Information, ///< Informational messages
    Warning,      ///< Warning messages
    Error,       ///< Error messages (least verbose)
};

/**
 * @enum logscope
 * @brief Log scope enumeration for categorizing log messages
 */
enum class logscope {
    Runtime, ///< Runtime system messages
    QB64,    ///< QB64 compiler messages
    Libqb,   ///< libqb library messages
    Audio,   ///< Audio system messages
    Image,   ///< Image system messages

    Count,   ///< Number of scopes (for iteration)
};

/**
 * @brief Initializes the logging system
 * @note Must be called before using any logging functions
 */
void libqb_log_init();

/**
 * @brief Logs a message with printf-style formatting
 * @param lvl Log level
 * @param scope Log scope
 * @param file Source file name (typically __FILE__)
 * @param func Function name (typically __func__)
 * @param line Line number (typically __LINE__)
 * @param fmt Format string (printf-style)
 * @param ... Format arguments
 * @note Main logging function for C-style formatted strings
 */
void libqb_log(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, ...);

/**
 * @brief Logs a message from QB64 code with printf-style formatting
 * @param lvl Log level
 * @param scope Log scope
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param fmt Format string (printf-style)
 * @param ... Format arguments
 * @note Specialized logging function for QB64 code paths
 */
void libqb_log_qb64(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, ...);

/**
 * @brief Logs a message from a qbs string
 * @param lvl Log level
 * @param scope Log scope
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param str qbs string containing the log message
 * @note Logs a message from a QB64 string
 */
void libqb_log_qbs(loglevel lvl, logscope scope, const char *file, const char *func, int line, qbs *str);

/**
 * @brief Gets the minimum log level (QB64 _LOGMINLEVEL function)
 * @return Minimum log level (1=Trace, 2=Information, 3=Warning, 4=Error)
 * @note Returns 1 to 4, indicating the minimum level from Trace to Error
 */
uint32_t func__logminlevel();


/**
 * @name Scope-Specific Logging Macros
 * @brief Macros for logging with a specific scope
 */
///@{
/**
 * @brief Logs a trace-level message with scope
 * @param scope Log scope
 * @param fmt Format string
 * @param ... Format arguments
 */
#define libqb_log_with_scope_trace(scope, fmt, ...) \
    libqb_log(loglevel::Trace, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

/**
 * @brief Logs an info-level message with scope
 * @param scope Log scope
 * @param fmt Format string
 * @param ... Format arguments
 */
#define libqb_log_with_scope_info(scope, fmt, ...) \
    libqb_log(loglevel::Information, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

/**
 * @brief Logs a warning-level message with scope
 * @param scope Log scope
 * @param fmt Format string
 * @param ... Format arguments
 */
#define libqb_log_with_scope_warn(scope, fmt, ...) \
    libqb_log(loglevel::Warning, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

/**
 * @brief Logs an error-level message with scope
 * @param scope Log scope
 * @param fmt Format string
 * @param ... Format arguments
 */
#define libqb_log_with_scope_error(scope, fmt, ...) \
    libqb_log(loglevel::Error, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)
///@}

/**
 * @name Libqb-Scope Logging Macros
 * @brief Convenience macros for logging with libqb scope
 */
///@{
/**
 * @brief Logs a trace-level message (libqb scope)
 * @param ... Format string and arguments
 */
#define libqb_log_trace(...) \
    libqb_log_with_scope_trace(logscope::Libqb, __VA_ARGS__)

/**
 * @brief Logs an info-level message (libqb scope)
 * @param ... Format string and arguments
 */
#define libqb_log_info(...) \
    libqb_log_with_scope_info(logscope::Libqb, __VA_ARGS__)

/**
 * @brief Logs a warning-level message (libqb scope)
 * @param ... Format string and arguments
 */
#define libqb_log_warn(...) \
    libqb_log_with_scope_warn(logscope::Libqb, __VA_ARGS__)

/**
 * @brief Logs an error-level message (libqb scope)
 * @param ... Format string and arguments
 */
#define libqb_log_error(...) \
    libqb_log_with_scope_error(logscope::Libqb, __VA_ARGS__)
///@}

/**
 * @name QB64 Logging Macros
 * @brief Macros for logging from QB64 code using qbs strings
 */
///@{
/**
 * @brief Logs a trace-level message from QB64 (QB64 _LOGTRACE statement)
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param qbs qbs string containing the log message
 */
#define sub__logtrace(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Trace, logscope::QB64, file, func, line, (qbs))

/**
 * @brief Logs an info-level message from QB64 (QB64 _LOGINFO statement)
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param qbs qbs string containing the log message
 */
#define sub__loginfo(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Information, logscope::QB64, file, func, line, (qbs))

/**
 * @brief Logs a warning-level message from QB64 (QB64 _LOGWARN statement)
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param qbs qbs string containing the log message
 */
#define sub__logwarn(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Warning, logscope::QB64, file, func, line, (qbs))

/**
 * @brief Logs an error-level message from QB64 (QB64 _LOGERROR statement)
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param qbs qbs string containing the log message
 */
#define sub__logerror(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Error, logscope::QB64, file, func, line, (qbs))
///@}