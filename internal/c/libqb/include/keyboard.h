//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Keyboard Module
//  Lock key functions extracted from libqb.cpp
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_KEYBOARD_H
#define INCLUDE_LIBQB_KEYBOARD_H

#include <stdint.h>

// Lock key state queries
int32_t func__capslock();
int32_t func__scrolllock();
int32_t func__numlock();

// Lock key control (options: 1=ON, 2=OFF, 3=TOGGLE)
void sub__capslock(int32_t options);
void sub__scrolllock(int32_t options);
void sub__numlock(int32_t options);

#endif // INCLUDE_LIBQB_KEYBOARD_H
