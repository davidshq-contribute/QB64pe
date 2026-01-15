//----------------------------------------------------------------------------------------------------------------------
// QB64-PE Console Support
// Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_CONSOLE_H
#define INCLUDE_LIBQB_CONSOLE_H

#include <stdint.h>

struct qbs;

// Console window management
int32_t func__console();
void sub__console(int32_t onoff);
void sub__consoletitle(qbs *s);
void sub__consolefont(qbs *FontName, int32_t FontSize);
void sub__console_cursor(int32_t visible, int32_t cursorsize, int32_t passed);

// Console input
int32_t func__getconsoleinput();
int32_t func__cinp(int32_t toggle, int32_t passed);

#endif
