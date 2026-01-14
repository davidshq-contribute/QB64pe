//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Screen Management Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_SCREEN_H
#define INCLUDE_LIBQB_SCREEN_H

#include <stdint.h>

// Display control
void sub__display();
void sub__autodisplay();

// Fullscreen control
void sub__fullscreen(int32_t method, int32_t passed);
int32_t func__fullscreen();
int32_t func__fullscreensmooth();
void sub__allowfullscreen(int32_t method, int32_t smooth);

// Resize handling
void sub__resize(int32_t on_off, int32_t stretch_smooth);
int32_t func__resize();
int32_t func__resizewidth();
int32_t func__resizeheight();

// Scaled dimensions
int32_t func__scaledwidth();
int32_t func__scaledheight();

// Screen position
int32_t func__screenx();
int32_t func__screeny();

#endif // INCLUDE_LIBQB_SCREEN_H
