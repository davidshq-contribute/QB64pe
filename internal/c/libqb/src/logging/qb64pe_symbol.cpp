
#include "libqb-common.h"

#include <string.h>
#include <stdlib.h>
#include <string>
#include <unwind.h>

#include "logging.h"
#include "logging_private.h"

/**
 * @file qb64pe_symbol.cpp
 * @brief Implementation of QB64 symbol resolution for logging
 * 
 * This file implements symbol resolution for QB64-specific function names,
 * converting internal function names (SUB_, FUNC_) to readable QB64 names.
 */

/**
 * @brief Resolves a QB64 symbol name
 * @param symbol Symbol name to resolve
 * @return Optional string containing the resolved QB64 name, or nullopt if not a QB64 symbol
 * @note Strips SUB_ and FUNC_ prefixes from symbol names. Recognizes QBMAIN as "Main QB64 code".
 *       Returns symbol name with " (QB64)" suffix for QB64 functions.
 */
std::optional<std::string> libqb_log_resolve_qb64_symbol(const char *symbol) {
    const char *qbsym = nullptr;

    if (strncmp(symbol, "SUB_", 4) == 0)
        qbsym = symbol + 4;
    else if (strncmp(symbol, "FUNC_", 5) == 0)
        qbsym = symbol + 5;

    if (strncmp(symbol, "QBMAIN(", 7) == 0)
        return "Main QB64 code";

    if (!qbsym)
        return std::nullopt;

    std::string ret = qbsym;

    return ret + " (QB64)";
}
