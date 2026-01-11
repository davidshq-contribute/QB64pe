
#include "libqb-common.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <stdio.h>

#include "logging.h"
#include "../logging_private.h"

/**
 * @file fp_handler.cpp
 * @brief Implementation of file pointer log handlers for QB64-PE
 * 
 * This file implements log handlers that write to FILE* streams, including
 * console and file log handlers.
 */

/**
 * @brief Writes a log entry to a file pointer
 * @param entry Log entry to write
 * @note Formats and writes the log entry to the file pointer. Includes timestamp,
 *       level, scope, file, function, line, and message. Also writes stack trace if present.
 *       Flushes the file pointer to ensure immediate output.
 */
void fp_log_writer::write(struct log_entry *entry) {
    if (!fp)
        return;

    const char *scope = logScopeName(entry->scope);

    // The main code may not have a file associated with it when compiled from
    // the IDE
    if (entry->file && *entry->file)
        fprintf(fp, "[%0.5lf] %s %s %s: %s: %d: %s\n",
                entry->timestamp,
                logLevelName(entry->level),
                scope? scope: "",
                entry->file,
                entry->func.c_str(),
                entry->line,
                entry->message.c_str());
    else
        fprintf(fp, "[%0.5lf] %s %s %s: %d: %s\n",
                entry->timestamp,
                logLevelName(entry->level),
                scope? scope: "",
                entry->func.c_str(),
                entry->line,
                entry->message.c_str());

    if (entry->stacktrace != "")
        fprintf(fp, "%s", entry->stacktrace.c_str());

    // Make sure buffers are written out immediately
    fflush(fp);
}

/**
 * @brief Constructs a console log handler
 * @note Initializes the handler to write to stderr.
 */
console_log_handler::console_log_handler() {
    fp = stderr;
}

/**
 * @brief Constructs a file log handler
 * @note Opens the log file specified by QB64PE_LOG_FILE_PATH environment variable.
 *       Opens in append mode. Prints error to stderr if file cannot be opened.
 */
file_log_handler::file_log_handler() {
    const char *filepath = getenv("QB64PE_LOG_FILE_PATH");

    fp = fopen(filepath, "a");
    if (!fp)
        fprintf(stderr, "Unable to open file '%s' for logging!", filepath);
}
