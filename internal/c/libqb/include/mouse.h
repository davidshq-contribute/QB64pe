//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Mouse Module
//  Mouse input functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_MOUSE_H
#define INCLUDE_LIBQB_MOUSE_H

#include <stdint.h>

struct qbs;

// Mouse visibility
void sub__mousehide();
void sub__mouseshow(qbs *style, int32_t passed);
int32_t func__mousehidden();

// Mouse position
float func__mousex();
float func__mousey();
float func__mousemovementx();
float func__mousemovementy();
void sub__mousemove(float x, float y);

// Mouse input
int32_t func__mouseinput();
int32_t func__mousebutton(int32_t i);
int32_t func__mousewheel();

#endif // INCLUDE_LIBQB_MOUSE_H
