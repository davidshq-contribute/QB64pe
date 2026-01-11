
#include "libqb-common.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdarg.h>
#include <sstream>
#include <algorithm>
#include <list>
#include <queue>
#include <unordered_map>
#include <string>
#include <optional>
#include <cstring>

#include "qbs.h"
#include "mutex.h"
#include "datetime.h"
#include "logging.h"
#include "logging_private.h"

static libqb_mutex *logging_lock;
static std::list<log_handler *> handlers;
static loglevel minimum_level = loglevel::Information;

// Short-circuits the logging logic if no handlers are enabled
static bool logging_enabled = false;

// By default, only logging from the QB64 program itself is enabled.
// 'runtime' is logging that cannot be disabled (Ex. errors in logging configuration)
static uint8_t enabled_scopes[static_cast<std::size_t>(logscope::Count)] = {
    [static_cast<std::size_t>(logscope::Runtime)] = 1,
    [static_cast<std::size_t>(logscope::QB64)] = 1,
};

/**
 * @brief Gets the minimum log level as a numeric value
 * @return Numeric log level: 1=Trace, 2=Information, 3=Warning, 4=Error, 5=Disabled
 * @note Returns 5 if logging is disabled. Used by QB64 programs to query the current log level.
 */
uint32_t func__logminlevel() {
    if (!logging_enabled)
        return 5;

    switch (minimum_level) {
        case loglevel::Trace: return 1;
        case loglevel::Information: return 2;
        case loglevel::Warning: return 3;
        case loglevel::Error: return 4;
    }

    return 5;
}

/**
 * @brief Gets the name of a log level
 * @param lvl Log level
 * @return String name of the log level
 */
const char *logLevelName(loglevel lvl) {
    switch (lvl) {
        case loglevel::Trace: return "TRACE";
        case loglevel::Information: return "INFO";
        case loglevel::Warning: return "WARN";
        case loglevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

/**
 * @brief Gets the name of a log scope
 * @param scope Log scope
 * @return String name of the log scope, or nullptr if invalid
 */
const char *logScopeName(logscope scope) {
    switch (scope) {
        case logscope::QB64: return "QB64";
        case logscope::Runtime: return "runtime";
        case logscope::Libqb: return "libqb";
        case logscope::Audio: return "libqb-audio";
        case logscope::Image: return "libqb-image";
        case logscope::Count: return nullptr; // Not a valid scope
    }
    return nullptr;
}

/**
 * @brief Core logging function with variadic arguments
 * @param lvl Log level
 * @param scope Log scope
 * @param file Source file name
 * @param func Function name
 * @param line Line number
 * @param fmt Format string
 * @param args Variadic argument list
 * @param qb64_only If true, only include QB64 stack frames in stack trace
 * @note Formats the log message, resolves QB64 symbols, optionally generates stack trace,
 *       and dispatches to all registered handlers. Thread-safe: uses mutex guard.
 */
static void libqb_vlog(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, va_list args, bool qb64_only)
{
    if (!logging_enabled)
        return;

    // Errors are always logged, otherwise the scope needs to be enabled
    if (lvl != loglevel::Error && !enabled_scopes[static_cast<std::size_t>(scope)])
        return;

    if (minimum_level > lvl)
        return;

    struct log_entry entry;
    entry.level = lvl;
    entry.scope = scope;
    entry.timestamp = (double)GetTicks() / 1000; // Convert ms to number of seconds
    entry.file = file;

    std::optional<std::string> qb64_sym = libqb_log_resolve_qb64_symbol(func);
    if (qb64_sym.has_value())
        entry.func = *qb64_sym;
    else
        entry.func = func;

    entry.line = line;

    if (lvl == loglevel::Error)
        entry.stacktrace = libqb_log_get_stacktrace(qb64_only);

    char msg[200];
    vsnprintf(msg, sizeof(msg), fmt, args);
    entry.message = msg;

    libqb_mutex_guard guard(logging_lock);

    for (auto logger : handlers)
        logger->write(&entry);
}

/**
 * @brief Logs a message with printf-style formatting
 * @param lvl Log level (Trace, Information, Warning, Error)
 * @param scope Log scope (QB64, Runtime, Libqb, Audio, Image)
 * @param file Source file name (typically __FILE__)
 * @param func Function name (typically __FUNCTION__ or __func__)
 * @param line Line number (typically __LINE__)
 * @param fmt Format string (printf-style)
 * @param ... Variadic arguments for format string
 * @note Main logging function. Includes full stack trace for errors. Resolves QB64 symbols.
 *       Thread-safe. Only logs if logging is enabled and scope is enabled.
 */
void libqb_log(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    libqb_vlog(lvl, scope, file, func, line, fmt, args, false);
    va_end(args);
}

/**
 * @brief Logs a message with printf-style formatting, QB64-only stack traces
 * @param lvl Log level (Trace, Information, Warning, Error)
 * @param scope Log scope (QB64, Runtime, Libqb, Audio, Image)
 * @param file Source file name (typically __FILE__)
 * @param func Function name (typically __FUNCTION__ or __func__)
 * @param line Line number (typically __LINE__)
 * @param fmt Format string (printf-style)
 * @param ... Variadic arguments for format string
 * @note Same as libqb_log() but stack traces only include QB64 stack frames, filtering out
 *       C++ runtime frames. Useful for QB64 program debugging.
 */
void libqb_log_qb64(loglevel lvl, logscope scope, const char *file, const char *func, int line, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    libqb_vlog(lvl, scope, file, func, line, fmt, args, true);
    va_end(args);
}

/**
 * @brief Logs a message from a QB64 string (qbs)
 * @param lvl Log level (Trace, Information, Warning, Error)
 * @param scope Log scope (QB64, Runtime, Libqb, Audio, Image)
 * @param file Source file name (typically __FILE__)
 * @param func Function name (typically __FUNCTION__ or __func__)
 * @param line Line number (typically __LINE__)
 * @param str QB64 string to log
 * @note Convenience function for logging QB64 strings. Converts qbs to C string and logs it.
 *       Uses QB64-only stack traces.
 */
void libqb_log_qbs(loglevel lvl, logscope scope, const char *file, const char *func, int line, qbs *str) {
    libqb_log_qb64(lvl, scope, file, func, line, "%.*s", (int)str->len, (const char *)str->chr);
}

/**
 * @brief Initializes the logging system
 * @note Reads environment variables to configure logging:
 *       - QB64PE_LOG_HANDLERS: Comma-separated list of handlers ("console", "file")
 *       - QB64PE_LOG_SCOPES: Comma-separated list of enabled scopes ("qb64", "libqb", "all", etc.)
 *       - QB64PE_LOG_LEVEL: Minimum log level ("TRACE", "INFORMATION", "WARNING", "ERROR")
 *       - QB64PE_LOG_FILE_PATH: Path to log file (required for "file" handler)
 *       Creates mutex for thread safety. Must be called before any logging functions.
 */
void libqb_log_init() {
    logging_lock = libqb_mutex_new();

    const char *handler_cstr = getenv("QB64PE_LOG_HANDLERS");
    if (!handler_cstr)
        return;

    std::stringstream stream(handler_cstr);
    std::string handler;

    while (std::getline(stream, handler, ',')) {
        std::transform(handler.begin(), handler.end(), handler.begin(), ::tolower);
        if (handler == "console") {
            handlers.push_back(new console_log_handler());
        } else if (handler == "file") {
            handlers.push_back(new file_log_handler());
        }
    }

    if (handlers.size())
        logging_enabled = true;

    const char *scope_cstr = getenv("QB64PE_LOG_SCOPES");
    if (scope_cstr) {
        std::stringstream stream(scope_cstr);
        std::string scopeStr;

        // Clear out the default scopes
        memset(enabled_scopes, 0, sizeof(enabled_scopes));

        // 'runtime' scope is used to report configuration errors, always enabled
        enabled_scopes[static_cast<std::size_t>(logscope::Runtime)] = 1;

        while (std::getline(stream, scopeStr, ',')) {
            std::optional<size_t> scope;
            std::transform(scopeStr.begin(), scopeStr.end(), scopeStr.begin(), ::tolower);

            if (scopeStr == "qb64") {
                scope = static_cast<std::size_t>(logscope::QB64);
            } else if (scopeStr == "libqb") {
                scope = static_cast<std::size_t>(logscope::Libqb);
            } else if (scopeStr == "libqb-audio") {
                scope = static_cast<std::size_t>(logscope::Audio);
            } else if (scopeStr == "libqb-image") {
                scope = static_cast<std::size_t>(logscope::Image);
            } else if (scopeStr == "all") {
                for (size_t i = 0; i < static_cast<std::size_t>(logscope::Count); i++)
                    enabled_scopes[i] = 1;
            } else
                runtime_log_warn("Unknown log scope '%s'!", scopeStr.c_str());

            if (scope.has_value())
                enabled_scopes[*scope] = 1;
        }
    }

    const char *minlevel = getenv("QB64PE_LOG_LEVEL");
    if (minlevel) {
        std::string lvl = minlevel;
        std::transform(lvl.begin(), lvl.end(), lvl.begin(), ::toupper);

        if (lvl == "INFORMATION" || lvl == "INFO")
            minimum_level = loglevel::Information;
        else if (lvl == "TRACE")
            minimum_level = loglevel::Trace;
        else if (lvl == "WARNING" || lvl == "WARN")
            minimum_level = loglevel::Warning;
        else if (lvl == "ERROR")
            minimum_level = loglevel::Error;
        else
            runtime_log_warn("Unknown log level '%s'!", minlevel);
    }
}
