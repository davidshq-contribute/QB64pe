//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File Helpers Module
//  File I/O helper functions for reading, writing, and OS information
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "file_helpers.h"
#include "error_handle.h"
#include "filepath.h"
#include "qbs.h"
#include "text.h"
#include "graphics.h"
#include "console.h"
#include "glut-thread.h"

#include "../../os.h"  // For int32, int64, uint32, uint64 type definitions

#include <cstdio>
#include <cstdlib>
#include <cstring>

// ============================================================================
// EXTERNAL DEPENDENCIES
// ============================================================================

// From libqb.cpp - global variables
extern qbs *nothingstring;
extern uint8 *window_title;

// ============================================================================
// WINDOW TITLE MANAGEMENT
// ============================================================================

void sub__title(qbs *title) {
    if (is_error_pending())
        return;
    static qbs *cz = NULL;
    if (!cz) {
        cz = qbs_new(1, 0);
        cz->chr[0] = 0;
    }
    static qbs *str = NULL;
    if (!str)
        str = qbs_new(0, 0);
    qbs_set(str, qbs_add(title, cz));

    uint8 *buf, *old_buf;
    buf = (uint8 *)malloc(str->len);
    memcpy(buf, str->chr, str->len);
    old_buf = window_title;
    window_title = buf;
    if (old_buf)
        free(old_buf);

    OPTIONAL_GLUT();

    libqb_glut_set_window_title((char *)window_title);
} // title

// ============================================================================
// CONSOLE OUTPUT
// ============================================================================

void sub__echo(qbs *message) {
    if (is_error_pending())
        return;

    int32 prevDest = func__dest();
    sub__dest(func__console());

    makefit(message);
    qbs_print(message, 0);
    qbs_print(nothingstring, 1);

    sub__dest(prevDest);

} // echo

// ============================================================================
// FILE OPERATIONS
// ============================================================================

qbs *func__readfile(qbs *filespec) {
    FILE *file;
    int32 len; // file handle; file length;
    qbs *namez;
    qbs *cont; // 0-term file name; file contents;

    namez = qbs_add(filespec, func_chr(0)); // add terminator
    filepath_fix_directory(namez);          // fix separators
    file = fopen((const char *)namez->chr, "rb");

    if (file) {
        fseek(file, 0, SEEK_END); // end pos
        len = ftell(file);        //   = file length
        rewind(file);             // rewind to start
        cont = qbs_new(len, 1);   // get new string for file contents
        fread(cont->chr, 1, len, file);
        if (ferror(file)) {
            error(QB_ERROR_PATH_FILE_ACCESS_ERROR); // something went wrong
            cont = qbs_new_txt("");                 // return empty on error
        }
        fclose(file);
        return cont;
    } else {
        error(QB_ERROR_FILE_NOT_FOUND); // most common when trying to read
        return qbs_new_txt("");         // return empty on error
    }
}

void sub__writefile(qbs *filespec, qbs *contents) {
    FILE *file; // file handle;
    qbs *namez; // 0-term file name;

    namez = qbs_add(filespec, func_chr(0)); // add terminator
    filepath_fix_directory(namez);          // fix separators
    file = fopen((const char *)namez->chr, "wb");

    if (file) {
        fwrite(contents->chr, 1, contents->len, file);
        if (ferror(file))
            error(QB_ERROR_PATH_FILE_ACCESS_ERROR); // something went wrong
        fclose(file);
    } else {
        error(QB_ERROR_PATH_NOT_FOUND); // most common when making a new file
    }
}

// ============================================================================
// SYSTEM INFORMATION
// ============================================================================

qbs *func__os() {
#ifdef QB64_WINDOWS
#    define QB64_OS_SYSTEM_STR "[WINDOWS]"
#elif defined(QB64_LINUX)
#    define QB64_OS_SYSTEM_STR "[LINUX]"
#elif defined(QB64_MACOSX)
#    define QB64_OS_SYSTEM_STR "[MACOSX]"
#else
#    define QB64_OS_SYSTEM_STR ""
#endif

#ifdef QB64_MACOSX
#    define QB64_OS_SYSTEM_EXTRA_STR "[LINUX]"
#else
#    define QB64_OS_SYSTEM_EXTRA_STR ""
#endif

#ifdef QB64_32
#    define QB64_OS_BITS_STR "[32BIT]"
#else
#    define QB64_OS_BITS_STR "[64BIT]"
#endif

#ifdef QB64_ARM
#    define QB64_OS_ARCH_STR "[ARM]"
#else
#    define QB64_OS_ARCH_STR ""
#endif

    // Have the compiler combine all our selections into one string
    return qbs_new_txt(QB64_OS_SYSTEM_STR QB64_OS_SYSTEM_EXTRA_STR QB64_OS_BITS_STR QB64_OS_ARCH_STR);
}
