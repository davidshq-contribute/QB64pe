
#include "libqb-common.h"

#include <stdlib.h>
#include <string.h>

#include "command.h"

/**
 * @file command.cpp
 * @brief Implementation of command-line argument handling for QB64-PE
 * 
 * This file implements functions for accessing command-line arguments passed to the program.
 */

/**
 * @brief Full command-line string (QB64 COMMAND$)
 * @note Contains the entire command line as a single string with spaces between arguments.
 */
qbs *func_command_str = NULL;

/**
 * @brief Array of command-line arguments
 * @note Points to argv array from main().
 */
static char **func_command_array = NULL;

/**
 * @brief Number of command-line arguments
 * @note Equals argc from main().
 */
static int32_t func_command_count = 0;

/**
 * @brief Gets a command-line argument (QB64 COMMAND$ function)
 * @param index Argument index (1-based). If 0 or not provided, returns full command line.
 * @param passed Flag indicating if index parameter was provided
 * @return qbs string containing the argument, or empty string if out of bounds
 * @note Returns empty string if index is out of bounds or if command array is NULL.
 *       If passed is 0, returns the full command line (legacy support).
 */
qbs *func_command(int32_t index, int32_t passed) {
    static qbs *tqbs;
    if (passed) { // Get specific parameter
        // If out of bounds or error getting cmdline args, return empty string.
        if (index >= func_command_count || index < 0 || func_command_array == NULL) {
            tqbs = qbs_new(0, 1);
            return tqbs;
        }
        int len = strlen(func_command_array[index]);
        // Create new temp qbs and copy data into it.
        tqbs = qbs_new(len, 1);
        memcpy(tqbs->chr, func_command_array[index], len);
    } else { // Legacy support; return whole commandline
        tqbs = qbs_new(func_command_str->len, 1);
        memcpy(tqbs->chr, func_command_str->chr, func_command_str->len);
    }
    return tqbs;
}

/**
 * @brief Gets the number of command-line arguments (QB64 _COMMANDCOUNT function)
 * @return Number of arguments (argc - 1, excluding program name)
 * @note Returns the count of actual arguments, excluding the program name.
 */
int32_t func__commandcount() {
    return func_command_count - 1;
}

/**
 * @brief Initializes the command-line argument system
 * @param argc Argument count from main()
 * @param argv Argument array from main()
 * @note Called at program startup. Builds the COMMAND$ string by joining all
 *       arguments with spaces. Stores the argv array for indexed access.
 */
void command_initialize(int argc, char **argv) {
    int i = argc;
    if (i > 1) {
        // calculate required size of COMMAND$ string
        int i2 = 0;
        for (i = 1; i < argc; i++) {
            i2 += strlen(argv[i]);
            if (i != 1)
                i2++; // for a space
        }
        // create COMMAND$ string
        func_command_str = qbs_new(i2, 0);
        // build COMMAND$ string
        int i3 = 0;
        for (i = 1; i < argc; i++) {
            if (i != 1) {
                func_command_str->chr[i3] = 32;
                i3++;
            }
            memcpy(&func_command_str->chr[i3], argv[i], strlen(argv[i]));
            i3 += strlen(argv[i]);
        }
    } else {
        func_command_str = qbs_new(0, 0);
    }

    func_command_count = argc;
    func_command_array = argv;
}
