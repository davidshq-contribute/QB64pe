
#include "libqb-common.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// note: MacOSX uses Apple's GLUT not FreeGLUT
#ifdef QB64_MACOSX
#    include <GLUT/glut.h>
#else
#    define CORE_FREEGLUT
#    include <GL/freeglut.h>
#endif

#include "glut-message.h"
#include "mac-mouse-support.h"

void glut_message_set_cursor::execute() {
    glutSetCursor(style);
}

/**
 * @brief Executes warp pointer message
 * @note Warps the mouse pointer to the specified coordinates. Must be called on GLUT thread.
 */
void glut_message_warp_pointer::execute() {
    glutWarpPointer(x, y);
}

/**
 * @brief Executes get GLUT state message
 * @note Gets a GLUT state value. Must be called on GLUT thread.
 */
void glut_message_get::execute() {
    response_value = glutGet(id);
}

/**
 * @brief Executes iconify window message
 * @note Iconifies (minimizes) the GLUT window. Must be called on GLUT thread.
 */
void glut_message_iconify_window::execute() {
    glutIconifyWindow();
}

/**
 * @brief Executes position window message
 * @note Positions the GLUT window at the specified coordinates. Must be called on GLUT thread.
 */
void glut_message_position_window::execute() {
    glutPositionWindow(x, y);
}

/**
 * @brief Executes show window message
 * @note Shows the GLUT window. Must be called on GLUT thread.
 */
void glut_message_show_window::execute() {
    glutShowWindow();
}

/**
 * @brief Executes hide window message
 * @note Hides the GLUT window. Must be called on GLUT thread.
 */
void glut_message_hide_window::execute() {
    glutHideWindow();
}

/**
 * @brief Executes set window title message
 * @note Sets the GLUT window title. Must be called on GLUT thread.
 */
void glut_message_set_window_title::execute() {
    glutSetWindowTitle(newTitle);
}

/**
 * @brief Executes exit program message
 * @note Exits the program with the specified exit code. Must be called on GLUT thread.
 */
void glut_message_exit_program::execute() {
    exit(exitCode);
}
