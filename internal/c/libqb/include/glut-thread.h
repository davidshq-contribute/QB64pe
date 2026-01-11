#ifndef INCLUDE_LIBQB_GLUT_THREAD_H
#define INCLUDE_LIBQB_GLUT_THREAD_H

/**
 * @file glut-thread.h
 * @brief GLUT thread management for QB64-PE
 * 
 * This header provides functions for managing the GLUT (OpenGL Utility Toolkit) thread,
 * which handles graphics window creation and event processing. GLUT operations must be
 * performed on a specific thread to avoid threading issues.
 */

/**
 * @brief Sets up GLUT before starting the program
 * @param argc Command-line argument count
 * @param argv Command-line argument array
 * @note Called to potentially initialize GLUT before program execution begins
 */
void libqb_glut_presetup(int argc, char **argv);

/**
 * @brief Starts the main thread, including GLUT setup
 * @param argc Command-line argument count
 * @param argv Command-line argument array
 * @note Initializes and starts the main thread with GLUT support
 */
void libqb_start_main_thread(int argc, char **argv);

/**
 * @brief Starts the GLUT thread
 * @note Used to support _ScreenShow, which can start the GLUT thread after
 *       the program has already started
 */
void libqb_start_glut_thread();

/**
 * @brief Checks if GLUT is currently running
 * @return true if GLUT is up and running, false otherwise
 * @note Indicates whether GLUT-related operations can be performed
 */
bool libqb_is_glut_up();

/**
 * @brief Processes the GLUT event queue
 * @note Called at consistent intervals from a GLUT callback to process queued events
 */
void libqb_process_glut_queue();

/**
 * @brief Exits the program properly
 * @param exitcode Exit code to return
 * @note Called to properly exit the program. GLUT requires special care to avoid
 *       segfaults when exiting.
 */
void libqb_exit(int);

/**
 * @name GLUT Thread-Safe Wrapper Functions
 * @brief Functions that perform GLUT operations on the GLUT thread
 * 
 * These functions perform the same actions as their corresponding glut* functions,
 * but they safely communicate with the GLUT thread to perform the command.
 */
///@{
/**
 * @brief Sets the cursor style (thread-safe GLUT wrapper)
 * @param style Cursor style to set
 */
void libqb_glut_set_cursor(int style);

/**
 * @brief Warps the mouse pointer to a position (thread-safe GLUT wrapper)
 * @param x X coordinate
 * @param y Y coordinate
 */
void libqb_glut_warp_pointer(int x, int y);

/**
 * @brief Gets a GLUT state value (thread-safe GLUT wrapper)
 * @param id GLUT state identifier
 * @return State value
 */
int libqb_glut_get(int id);

/**
 * @brief Iconifies (minimizes) the window (thread-safe GLUT wrapper)
 */
void libqb_glut_iconify_window();

/**
 * @brief Positions the window (thread-safe GLUT wrapper)
 * @param x X position
 * @param y Y position
 */
void libqb_glut_position_window(int x, int y);

/**
 * @brief Shows the window (thread-safe GLUT wrapper)
 */
void libqb_glut_show_window();

/**
 * @brief Hides the window (thread-safe GLUT wrapper)
 */
void libqb_glut_hide_window();

/**
 * @brief Sets the window title (thread-safe GLUT wrapper)
 * @param title Window title string
 */
void libqb_glut_set_window_title(const char *title);

/**
 * @brief Exits the program through GLUT (thread-safe GLUT wrapper)
 * @param exitcode Exit code to return
 */
void libqb_glut_exit_program(int exitcode);
///@}

/**
 * @name GLUT State Checking Macros
 * @brief Convenience macros for checking GLUT state before operations
 */
///@{
/**
 * @brief Checks if GLUT is up, errors if not
 * @param error_result Value to return if GLUT is not up
 * @note Generates error 5 and returns error_result if GLUT is not running.
 *       Use this for operations that require GLUT to be active.
 */
#define NEEDS_GLUT(error_result)                                                                                                                               \
    do {                                                                                                                                                       \
        if (!libqb_is_glut_up()) {                                                                                                                             \
            error(5);                                                                                                                                          \
            return error_result;                                                                                                                               \
        }                                                                                                                                                      \
    } while (0)

/**
 * @brief Returns early if GLUT is not up
 * @param result Value to return if GLUT is not up
 * @note Returns result if GLUT is not running. Use this for optional GLUT operations.
 */
#define OPTIONAL_GLUT(result)                                                                                                                                  \
    do {                                                                                                                                                       \
        if (!libqb_is_glut_up())                                                                                                                               \
            return result;                                                                                                                                     \
    } while (0)
///@}

#endif
