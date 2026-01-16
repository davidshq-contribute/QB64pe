//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Logging Module
//  Provides comprehensive logging system with multiple levels and scopes
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>

#include "qbs.h"

// ============================================================================
// LOGGING ENUMERATIONS
// ============================================================================

/// Log message severity levels
/// Ordered from least to most severe
enum class loglevel {
    Trace,       ///< Detailed tracing information for debugging
    Information, ///< General information messages
    Warning,     ///< Warning messages for potential issues
    Error,       ///< Error messages for failures
};

/// Log message scope categories
/// Used to categorize log messages by module
enum class logscope {
    Runtime,      ///< Runtime system messages
    QB64,         ///< QB64 language processor messages
    Libqb,        ///< Libqb library messages
    Audio,         ///< Audio system messages
    Image,         ///< Image processing messages
    
    Count,         ///< Total number of scopes (for bounds checking)
};

// ============================================================================
// CORE LOGGING FUNCTIONS
// ============================================================================

/// Initializes the logging system
/// Sets up logging output and configuration
void libqb_log_init();

/// Logs a message with specified level and scope
/// Core logging function with full context information
/// @param lvl Log message severity level
/// @param scope Log message category scope
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
void libqb_log(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, ...);

/// Logs a QB64-specific message
/// Specialized logging for QB64 language processor
/// @param lvl Log message severity level
/// @param scope Log message category scope
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
void libqb_log_qb64(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, ...);

/// Logs a QB64 string message
/// Logs a QB64 string value with context
/// @param lvl Log message severity level
/// @param scope Log message category scope
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param str QB64 string to log
void libqb_log_qbs(loglevel lvl, logscope scope, const char *file, const char *func, int line, qbs *str);

/// Gets the minimum logging level
/// Returns the current minimum level that will be logged
/// @return 1=Trace, 2=Information, 3=Warning, 4=Error
uint32_t func__logminlevel();

// ============================================================================
// LOGGING MACROS
// ============================================================================

/// Logs a trace message with specified scope
/// Convenience macro for trace-level logging
/// @param scope Log message category scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_with_scope_trace(scope, fmt, ...) \
    libqb_log(loglevel::Trace, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

/// Logs an info message with specified scope
/// Convenience macro for information-level logging
/// @param scope Log message category scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_with_scope_info(scope, fmt, ...) \
    libqb_log(loglevel::Information, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

/// Logs a warning message with specified scope
/// Convenience macro for warning-level logging
/// @param scope Log message category scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_with_scope_warn(scope, fmt, ...) \
    libqb_log(loglevel::Warning, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

/// Logs an error message with specified scope
/// Convenience macro for error-level logging
/// @param scope Log message category scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_with_scope_error(scope, fmt, ...) \
    libqb_log(loglevel::Error, (scope), __FILE__, __func__, __LINE__, fmt, ## __VA_ARGS__)

// ============================================================================
// DEFAULT SCOPE LOGGING MACROS
// ============================================================================

/// Logs a trace message with Libqb scope
/// Convenience macro for trace-level logging in Libqb scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_trace(...) \
    libqb_log_with_scope_trace(logscope::Libqb, __VA_ARGS__)

/// Logs an info message with Libqb scope
/// Convenience macro for information-level logging in Libqb scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_info(...) \
    libqb_log_with_scope_info(logscope::Libqb, __VA_ARGS__)

/// Logs a warning message with Libqb scope
/// Convenience macro for warning-level logging in Libqb scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_warn(...) \
    libqb_log_with_scope_warn(logscope::Libqb, __VA_ARGS__)

/// Logs an error message with Libqb scope
/// Convenience macro for error-level logging in Libqb scope
/// @param fmt Format string for message
/// @param ... Variable arguments for format string
#define libqb_log_error(...) \
    libqb_log_with_scope_error(logscope::Libqb, __VA_ARGS__)

// ============================================================================
// QB64 INTERNAL LOGGING MACROS
// ============================================================================

/// Logs a QB64 trace message
/// Internal macro for QB64 trace-level logging
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param qbs QB64 string to log
#define sub__logtrace(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Trace, logscope::QB64, file, func, line, (qbs))

/// Logs a QB64 info message
/// Internal macro for QB64 information-level logging
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param qbs QB64 string to log
#define sub__loginfo(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Information, logscope::QB64, file, func, line, (qbs))

/// Logs a QB64 warning message
/// Internal macro for QB64 warning-level logging
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param qbs QB64 string to log
#define sub__logwarn(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Warning, logscope::QB64, file, func, line, (qbs))

/// Logs a QB64 error message
/// Internal macro for QB64 error-level logging
/// @param file Source file name
/// @param func Source function name
/// @param line Source line number
/// @param qbs QB64 string to log
#define sub__logerror(file, func, line, qbs) \
    libqb_log_qbs(loglevel::Error, logscope::QB64, file, func, line, (qbs))
