//----------------------------------------------------------------------------------------------------------------------
//    ___  ___   __ _  _  _  ___   ___
//   / _ \| _ ) / /| || || || _ \ / _ \
//  | (_) | _ \/ _ \__ | || ||  _/|  __/
//   \__\_\___/\___/|_||_||_||_|   \___|
//
//  QB64-PE Legacy Memory Access Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_MEM_LEGACY_H
#define INCLUDE_LIBQB_MEM_LEGACY_H

#include <stdint.h>

// Current segment pointer (exported for call_absolute in libqb.cpp)
extern uint8_t *defseg;

// DEF SEG - Set segment for PEEK/POKE operations
void sub_defseg(int32_t segment, int32_t passed);

// PEEK - Read byte from memory
int32_t func_peek(int32_t offset);

// POKE - Write byte to memory
void sub_poke(int32_t offset, int32_t value);

#endif // INCLUDE_LIBQB_MEM_LEGACY_H
