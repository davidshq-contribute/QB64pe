
#include "libqb-common.h"

#include <stdlib.h>
#include <string.h>

#include "environ.h"
#include "error_handle.h"
#include "qbs.h"

/**
 * @file environ.cpp
 * @brief Implementation of environment variable access for QB64-PE
 * 
 * This file implements functions for reading and setting environment variables.
 */

#ifdef QB64_WINDOWS
#    define envp _environ
#else
extern char **environ;
#    define envp environ
#endif

/**
 * @brief Gets the number of environment variables (QB64 _ENVIRONCOUNT function)
 * @return Number of environment variables
 * @note Counts the environment variables by iterating through the environ array.
 */
int32_t func__environcount() {
    // count array bound
    char **p = envp;
    while (*++p)
        ;
    return p - envp;
}

/**
 * @brief Gets an environment variable by name (QB64 ENVIRON$ function - name overload)
 * @param name Environment variable name
 * @return qbs string containing the variable value, or empty string if not found
 * @note Uses getenv() to look up the variable. Returns empty string if variable doesn't exist.
 */
qbs *func_environ(qbs *name) {
    char *query, *result;
    qbs *tqbs;
    query = (char *)malloc(name->len + 1);
    query[name->len] = '\0'; // add NULL terminator
    memcpy(query, name->chr, name->len);
    result = getenv(query);
    if (result) {
        int result_length = strlen(result);
        tqbs = qbs_new(result_length, 1);
        memcpy(tqbs->chr, result, result_length);
    } else {
        tqbs = qbs_new(0, 1);
    }
    return tqbs;
}

/**
 * @brief Gets an environment variable by index (QB64 ENVIRON$ function - index overload)
 * @param number Environment variable index (1-based)
 * @return qbs string containing the variable (name=value format), or empty string if out of bounds
 * @note Returns the environment variable in "name=value" format. Generates error 5 if number <= 0.
 *       Returns empty string if index is out of bounds.
 */
qbs *func_environ(int32_t number) {
    char *result;
    qbs *tqbs;
    int result_length;
    if (number <= 0) {
        tqbs = qbs_new(0, 1);
        error(5);
        return tqbs;
    }
    // Check we do not go beyond array bound
    char **p = envp;
    while (*++p)
        ;
    if (number > p - envp) {
        tqbs = qbs_new(0, 1);
        return tqbs;
    }
    result = envp[number - 1];
    result_length = strlen(result);
    tqbs = qbs_new(result_length, 1);
    memcpy(tqbs->chr, result, result_length);
    return tqbs;
}

/**
 * @brief Sets or removes an environment variable (QB64 ENVIRON statement)
 * @param str String in "name=value" or "name value" format
 * @note Name and value may be separated by '=' or space, whichever appears first.
 *       If separator is at the end of the string, the variable is removed.
 *       Uses platform-specific functions: _putenv/_putenv_s on Windows, setenv/unsetenv on POSIX.
 *       Generates error 5 if no separator is found.
 */
void sub_environ(qbs *str) {
    char *buf;
    char *separator;
    buf = (char *)malloc(str->len + 1);
    buf[str->len] = '\0';
    memcpy(buf, str->chr, str->len);
    // Name and value may be separated by = or space, whichever appears first.
    separator = buf + strcspn(buf, " =");
    if (*separator == '\0') {
        // It is an error is there is no separator
        free(buf);
        error(5);
        return;
    }
    // Split into two separate strings
    *separator = '\0';
    if (separator == &buf[str->len] - 1) {
        // Separator is at end of string, so remove the variable
#ifdef QB64_WINDOWS
        *separator = '=';
        _putenv(buf);
#else
        unsetenv(buf);
#endif
    } else {
#ifdef QB64_WINDOWS
#    if WINVER >= 0x0600
        _putenv_s(buf, separator + 1);
#    else
        *separator = '=';
        _putenv(buf);
#    endif
#else
        setenv(buf, separator + 1, 1);
#endif
    }
    free(buf);
}
