
#include "libqb-common.h"

#include <queue>
#include <unistd.h>

// note: MacOSX uses Apple's GLUT not FreeGLUT
#ifdef QB64_MACOSX
#    include <GLUT/glut.h>
#else
#    define CORE_FREEGLUT
#    include <GL/freeglut.h>
#endif

#include "glut-message.h"
#include "glut-thread.h"
#include "mutex.h"

/**
 * @file glut-msg-queue.cpp
 * @brief Implementation of GLUT message queue for QB64-PE
 * 
 * This file implements a thread-safe message queue for GLUT operations,
 * allowing the main thread to safely communicate with the GLUT thread.
 */

/**
 * @brief Mutex protecting the GLUT message queue
 */
static libqb_mutex *glut_msg_queue_lock = libqb_mutex_new();

/**
 * @brief Queue of GLUT messages waiting to be processed
 */
static std::queue<glut_message *> glut_msg_queue;

/**
 * @brief Cached GLUT window X position
 * @note These values from GLUT are read on every process of the msg queue. Calls to
 *       libqb_glut_get() can then read from these values directly rather than wait
 *       for the GLUT thread to process the command.
 */
static int glut_window_x, glut_window_y;

#ifdef CORE_FREEGLUT
static int glut_window_border_width, glut_window_header_height;
#endif

/**
 * @brief Queues a GLUT message for execution on the GLUT thread
 * @param msg Message to queue
 * @return true if queued successfully, false if GLUT is not up
 * @note If GLUT is not up, the message is finished immediately and false is returned.
 *       Thread-safe: uses mutex to protect the queue.
 */
bool libqb_queue_glut_message(glut_message *msg) {
    if (!libqb_is_glut_up()) {
        msg->finish();
        return false;
    }

    libqb_mutex_guard guard(glut_msg_queue_lock);

    glut_msg_queue.push(msg);

    return true;
}

/**
 * @brief Processes all queued GLUT messages
 * @note Must be called on the GLUT thread. Executes all queued messages and
 *       updates cached GLUT state values. Thread-safe: uses mutex to protect the queue.
 */
void libqb_process_glut_queue() {
    libqb_mutex_guard guard(glut_msg_queue_lock);

    glut_window_x = glutGet(GLUT_WINDOW_X);
    glut_window_y = glutGet(GLUT_WINDOW_Y);

#ifdef CORE_FREEGLUT
    glut_window_border_width = glutGet(GLUT_WINDOW_BORDER_WIDTH);
    glut_window_header_height = glutGet(GLUT_WINDOW_HEADER_HEIGHT);
#endif

    while (!glut_msg_queue.empty()) {
        glut_message *msg = glut_msg_queue.front();
        glut_msg_queue.pop();

        msg->execute();

        msg->finish();
    }
}

/**
 * @brief Sets the GLUT cursor style
 * @param style Cursor style constant
 * @note Queues a message to set the cursor. Thread-safe wrapper around GLUT function.
 */
void libqb_glut_set_cursor(int style) {
    libqb_queue_glut_message(new glut_message_set_cursor(style));
}

/**
 * @brief Warps the mouse pointer
 * @param x X coordinate
 * @param y Y coordinate
 * @note Queues a message to warp the pointer. Thread-safe wrapper around GLUT function.
 */
void libqb_glut_warp_pointer(int x, int y) {
    libqb_queue_glut_message(new glut_message_warp_pointer(x, y));
}

/**
 * @brief Checks if a GLUT value is cached statically
 * @param id GLUT value ID
 * @return true if the value is cached, false otherwise
 * @note Some GLUT values are cached to avoid waiting for the GLUT thread.
 */
static bool is_static_glut_value(int id) {
    return id == GLUT_WINDOW_Y || id == GLUT_WINDOW_X
#ifdef CORE_FREEGLUT
           || id == GLUT_WINDOW_BORDER_WIDTH || id == GLUT_WINDOW_HEADER_HEIGHT
#endif
        ;
}

/**
 * @brief Gets a cached static GLUT value
 * @param id GLUT value ID
 * @return Cached value, or -1 if not cached
 * @note Returns cached values without waiting for GLUT thread.
 */
static int __get_static_glut_value(int id) {
    switch (id) {
    case GLUT_WINDOW_Y:
        return glut_window_y;
    case GLUT_WINDOW_X:
        return glut_window_x;
#ifdef CORE_FREEGLUT
    case GLUT_WINDOW_BORDER_WIDTH:
        return glut_window_border_width;
    case GLUT_WINDOW_HEADER_HEIGHT:
        return glut_window_header_height;
#endif
    default:
        return -1;
    }
}

int libqb_glut_get(int id) {
    if (is_static_glut_value(id)) {
        libqb_mutex_guard guard(glut_msg_queue_lock);
        return __get_static_glut_value(id);
    }

    glut_message_get msg(id);

    libqb_queue_glut_message(&msg);
    msg.wait_for_response();

    return msg.response_value;
}

void libqb_glut_iconify_window() {
    libqb_queue_glut_message(new glut_message_iconify_window());
}

void libqb_glut_position_window(int x, int y) {
    libqb_queue_glut_message(new glut_message_position_window(x, y));
}

void libqb_glut_show_window() {
    libqb_queue_glut_message(new glut_message_show_window());
}

void libqb_glut_hide_window() {
    libqb_queue_glut_message(new glut_message_hide_window());
}

void libqb_glut_set_window_title(const char *title) {
    libqb_queue_glut_message(new glut_message_set_window_title(title));
}

void libqb_glut_exit_program(int exitcode) {
    glut_message_exit_program msg(exitcode);

    libqb_queue_glut_message(&msg);
    msg.wait_for_response();

    // Should never return
    exit(exitcode);
}
