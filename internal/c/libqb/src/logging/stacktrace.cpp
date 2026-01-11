
#include "libqb-common.h"

#include <string>
#include <string.h>
#include <stdio.h>
#include <unwind.h>

#include "logging.h"
#include "logging_private.h"

/**
 * @file stacktrace.cpp
 * @brief Implementation of stack trace generation for QB64-PE
 * 
 * This file implements stack trace generation using libunwind, with support for
 * filtering QB64-only stack frames and resolving symbols.
 */

/**
 * @brief Stack trace generation state
 */
struct stacktrace_state {
    bool qb64_only;
    bool started_qb64_stack;
    int frame;
    int skipped;
    std::string str;
};

/**
 * @brief Checks if a string starts with a prefix
 * @param l String to check
 * @param r Prefix string
 * @return true if string starts with prefix, false otherwise
 */
static bool startsWith(const std::string &l, const char *r) {
    size_t rlen = strlen(r);
    if (l.size() < rlen)
        return false;

    return strncmp(l.c_str(), r, rlen) == 0;
}

/**
 * @brief Unwind handler for stack trace generation
 * @param context Unwind context
 * @param ref Pointer to stacktrace_state
 * @return Unwind reason code
 * @note Called for each stack frame during unwinding. Resolves symbols and
 *       filters out logging functions. Limits to 40 frames.
 */
static _Unwind_Reason_Code handler(struct _Unwind_Context *context, void *ref) {
    stacktrace_state *result = static_cast<stacktrace_state *>(ref);

	if (result->frame > 40)
		return _URC_NORMAL_STOP;

    result->frame++;

    const void *addr = (const void *)_Unwind_GetIP(context);
    if (!addr)
        return _URC_NORMAL_STOP;

    std::string symbol = libqb_log_resolve_symbol(addr);

    // Remove these symbols from the stacktrace
    if (startsWith(symbol, "libqb_log")
        || startsWith(symbol, "libqb_vlog")) {

        result->skipped++;
        return _URC_NO_REASON;
    }

    std::optional<std::string> qb64_symbol = libqb_log_resolve_qb64_symbol(symbol.c_str());

    if (qb64_symbol.has_value()) {
        symbol = *qb64_symbol;
        result->started_qb64_stack = true;
    } else if (result->qb64_only) {
        if (result->started_qb64_stack)
            return _URC_NORMAL_STOP;
        else
            return _URC_NO_REASON;
    }

    char line[200];
    snprintf(line, sizeof(line), "#%-2d [0x%p] in %s\n", result->frame - result->skipped, addr, symbol.c_str());

    result->str += line;
	return _URC_NO_REASON;
}

/**
 * @brief Generates a stack trace
 * @param qb64_only If true, only include QB64 stack frames
 * @return String containing the stack trace
 * @note Uses libunwind to walk the stack. Resolves symbols and filters logging functions.
 *       Returns formatted stack trace with frame numbers and addresses.
 */
std::string libqb_log_get_stacktrace(bool qb64_only) {
    stacktrace_state state = {
        .qb64_only = qb64_only,
        .started_qb64_stack = false,
        .frame = 0,
        .skipped = 0,
        .str = "",
    };

    _Unwind_Backtrace(&handler, &state);

    return state.str;
}
