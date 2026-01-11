
#include "libqb-common.h"

#include <GL/glew.h>
#include <list>
#include <queue>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unordered_map>

// note: MacOSX uses Apple's GLUT not FreeGLUT
#ifdef QB64_MACOSX
#    include <GLUT/glut.h>
#else
#    define CORE_FREEGLUT
#    include <GL/freeglut.h>
#endif

#include "logging.h"
#include "completion.h"
#include "glut-thread.h"
#include "gui.h"
#include "mac-key-monitor.h"
#include "mac-mouse-support.h"
#include "mutex.h"
#include "thread.h"

/**
 * @file glut-main-thread.cpp
 * @brief Implementation of GLUT main thread management for QB64-PE
 * 
 * This file implements GLUT initialization and thread management, ensuring GLUT
 * runs on the main thread as required by the GLUT specification.
 * 
 * FIXME: These extern variable and function definitions should probably go
 * somewhere more global so that they can be referenced by libqb.cpp
 */
extern uint8_t *window_title;
extern int32_t framebufferobjects_supported;
extern int32_t screen_hide;

void MAIN_LOOP(void *);
void GLUT_KEYBOARD_FUNC(unsigned char key, int x, int y);
void GLUT_DISPLAY_REQUEST();
void GLUT_KEYBOARDUP_FUNC(unsigned char key, int x, int y);
void GLUT_SPECIAL_FUNC(int key, int x, int y);
void GLUT_SPECIALUP_FUNC(int key, int x, int y);
void GLUT_MOUSE_FUNC(int glut_button, int state, int x, int y);
void GLUT_MOTION_FUNC(int x, int y);
void GLUT_PASSIVEMOTION_FUNC(int x, int y);
void GLUT_RESHAPE_FUNC(int width, int height);

void GLUT_IDLEFUNC();

#ifdef CORE_FREEGLUT
void GLUT_MOUSEWHEEL_FUNC(int wheel, int direction, int x, int y);
#endif

/**
 * @brief GLUT warning handler (suppresses FreeGLUT warnings)
 * @param fmt Format string (unused)
 * @param lst Argument list (unused)
 * @note This keeps FreeGlut from dumping warnings to console.
 */
static void glutWarning(const char *fmt, va_list lst) {
    // This keeps FreeGlut from dumping warnings to console
    (void)fmt;
    (void)lst;
}

/**
 * @brief Performs all of the FreeGLUT initialization except for calling glutMainLoop()
 * @param argc Command-line argument count
 * @param argv Command-line argument array
 * @note Initializes GLUT, GLEW, sets up display mode, creates window, and registers callbacks.
 *       On Windows, enables multisampling. On macOS, registers key handler.
 */
static void initialize_glut(int argc, char **argv) {
#ifdef CORE_FREEGLUT
    glutInitWarningFunc(glutWarning);
    glutInitErrorFunc(glutWarning);
#endif

    glutInit(&argc, argv);

    mac_register_key_handler();

#ifdef QB64_WINDOWS
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
#else
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
#endif

    glutInitWindowSize(640, 400); // cannot be changed unless display_x(etc) are modified

    if (!glutGet(GLUT_DISPLAY_MODE_POSSIBLE)) // must be called on Linux or GLUT crashes
    {
        exit(1);
    }

    if (!window_title) {
        glutCreateWindow("Untitled");
    } else {
        glutCreateWindow((char *)window_title);
    }

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        gui_alert((char *)glewGetErrorString(err));
    }

    if (glewIsSupported("GL_EXT_framebuffer_object"))
        framebufferobjects_supported = 1;

    glutDisplayFunc(GLUT_DISPLAY_REQUEST);

    glutIdleFunc(GLUT_IDLEFUNC);

    glutKeyboardFunc(GLUT_KEYBOARD_FUNC);
    glutKeyboardUpFunc(GLUT_KEYBOARDUP_FUNC);
    glutSpecialFunc(GLUT_SPECIAL_FUNC);
    glutSpecialUpFunc(GLUT_SPECIALUP_FUNC);
    glutMouseFunc(GLUT_MOUSE_FUNC);
    glutMotionFunc(GLUT_MOTION_FUNC);
    glutPassiveMotionFunc(GLUT_PASSIVEMOTION_FUNC);
    glutReshapeFunc(GLUT_RESHAPE_FUNC);

#ifdef CORE_FREEGLUT
    glutMouseWheelFunc(GLUT_MOUSEWHEEL_FUNC);
#endif
}

/**
 * @brief Flag indicating GLUT has been started
 */
static bool glut_is_started;

/**
 * @brief Completion used to start GLUT thread (for $SCREENHIDE programs)
 */
static struct completion glut_thread_starter;

/**
 * @brief Completion used to signal GLUT thread initialization complete
 */
static struct completion *glut_thread_initialized;

/**
 * @brief Starts the GLUT thread
 * @note For $SCREENHIDE programs, waits for GLUT to be initialized.
 *       Completes the glut_thread_starter to signal GLUT should start.
 */
void libqb_start_glut_thread() {
    if (glut_is_started)
        return;

    struct completion init;
    completion_init(&init);

    glut_thread_initialized = &init;

    completion_finish(&glut_thread_starter);

    completion_wait(&init);
    completion_clear(&init);
}

/**
 * @brief Checks whether the GLUT thread is running
 * @return true if GLUT is started, false otherwise
 */
bool libqb_is_glut_up() {
    return glut_is_started;
}

/**
 * @brief Performs GLUT presetup
 * @param argc Command-line argument count
 * @param argv Command-line argument array
 * @note If screen is not hidden, initializes GLUT immediately.
 *       If screen is hidden, initializes completion for deferred GLUT startup.
 */
void libqb_glut_presetup(int argc, char **argv) {
    if (!screen_hide) {
        initialize_glut(argc, argv); // Initialize GLUT if the screen isn't hidden
        glut_is_started = true;
    } else {
        completion_init(&glut_thread_starter);
    }
}

/**
 * @brief Starts the main thread (GLUT thread)
 * @param argc Command-line argument count
 * @param argv Command-line argument array
 * @note Starts the 'MAIN_LOOP' in a separate thread, as GLUT has to run on the initial thread.
 *       For $SCREENHIDE programs, waits on `glut_thread_starter` completion until _ScreenShow is used.
 *       Then initializes GLUT and enters glutMainLoop().
 */
void libqb_start_main_thread(int argc, char **argv) {

    // Start the 'MAIN_LOOP' in a separate thread, as GLUT has to run on the
    // initial thread.
    struct libqb_thread *main_loop = libqb_thread_new();
    libqb_thread_start(main_loop, MAIN_LOOP, NULL);

    // This happens for $SCREENHIDE programs. This thread waits on the
    // `glut_thread_starter` completion, which will get completed if a
    // _ScreenShow is used.
    if (!glut_is_started) {
        completion_wait(&glut_thread_starter);

        initialize_glut(argc, argv);
        glut_is_started = true;

        if (glut_thread_initialized)
            completion_finish(glut_thread_initialized);
    }

    glutMainLoop();
}

/**
 * @brief Exits the program safely from GLUT thread
 * @param exitcode Exit code
 * @note Due to GLUT making use of cleanup via atexit, we have to call exit() from
 *       the same thread handling the GLUT logic so that the atexit handler also runs
 *       from that thread (not doing that can result in a segfault due to using GLUT
 *       from two threads at the same time).
 *       This is accomplished by simply queuing a GLUT message that calls exit() for us.
 *       If GLUT isn't running, calls exit() directly.
 */
void libqb_exit(int exitcode) {
    libqb_log_info("Program exiting with code: %d\n", exitcode);
    // If GLUT isn't running then we're free to do the exit() call from here
    if (!libqb_is_glut_up())
        exit(exitcode);

    libqb_glut_exit_program(exitcode);
}
