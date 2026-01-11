
#include "libqb-common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "glut-thread.h"

/**
 * @file console-only-main-thread.cpp
 * @brief Console-only program main thread implementation for QB64-PE
 * 
 * This file provides stub implementations of GLUT thread functions for console-only
 * programs that never invoke GLUT. The setup is much simpler since no graphics
 * window is created.
 */

// FIXME: Put this definition somewhere else
void MAIN_LOOP(void *);

/**
 * @brief GLUT presetup stub (console-only)
 * @param argc Command-line argument count (unused)
 * @param argv Command-line argument array (unused)
 * @note No-op for console-only programs.
 */
void libqb_glut_presetup(int argc, char **argv) {
    (void)argc;
    (void)argv;
}

/**
 * @brief Starts the main thread (console-only)
 * @param argc Command-line argument count (unused)
 * @param argv Command-line argument array (unused)
 * @note Because GLUT is not used, we can just run MAIN_LOOP without creating a new thread.
 */
void libqb_start_main_thread(int argc, char **argv) {
    (void)argc;
    (void)argv;
    // Because GLUT is not used, we can just run MAIN_LOOP without creating a
    // new thread for it.
    MAIN_LOOP(NULL);
}

/**
 * @brief Starts GLUT thread stub (console-only)
 * @note No-op for console-only programs.
 */
void libqb_start_glut_thread() {}

/**
 * @brief Checks if GLUT is up (console-only)
 * @return Always false for console-only programs
 */
bool libqb_is_glut_up() {
    return false;
}

/**
 * @brief Processes GLUT queue stub (console-only)
 * @note No-op for console-only programs.
 */
void libqb_process_glut_queue() {}

/**
 * @brief Sets cursor style stub (console-only)
 * @param style Cursor style (unused)
 * @note No-op for console-only programs.
 */
void libqb_glut_set_cursor(int style) {
    (void)style;
}

/**
 * @brief Warps mouse pointer stub (console-only)
 * @param x X coordinate (unused)
 * @param y Y coordinate (unused)
 * @note No-op for console-only programs.
 */
void libqb_glut_warp_pointer(int x, int y) {
    (void)x;
    (void)y;
}

/**
 * @brief Gets GLUT state stub (console-only)
 * @param id State identifier (unused)
 * @return Always 0 for console-only programs
 */
int libqb_glut_get(int id) {
    (void)id;
    return 0;
}

/**
 * @brief Iconifies window stub (console-only)
 * @note No-op for console-only programs.
 */
void libqb_glut_iconify_window() {}

/**
 * @brief Positions window stub (console-only)
 * @param x X position (unused)
 * @param y Y position (unused)
 * @note No-op for console-only programs.
 */
void libqb_glut_position_window(int x, int y) {
    (void)x;
    (void)y;
}

/**
 * @brief Shows window stub (console-only)
 * @note No-op for console-only programs.
 */
void libqb_glut_show_window() {}

/**
 * @brief Hides window stub (console-only)
 * @note No-op for console-only programs.
 */
void libqb_glut_hide_window() {}

/**
 * @brief Sets window title stub (console-only)
 * @param title Window title (unused)
 * @note No-op for console-only programs.
 */
void libqb_glut_set_window_title(const char *title) {
    (void)title;
}

/**
 * @brief Exits program through GLUT stub (console-only)
 * @param exitcode Exit code
 * @note Delegates to libqb_exit().
 */
void libqb_glut_exit_program(int exitcode) {
    libqb_exit(exitcode);
}

/**
 * @brief Exits the program (console-only)
 * @param code Exit code
 * @note Since there's no GLUT thread to deal with, we can just exit() like normal.
 */
void libqb_exit(int code) {
    exit(code);
}
