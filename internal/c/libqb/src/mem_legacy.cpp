//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Legacy Memory Access Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "mem_legacy.h"
#include "error_handle.h"

#include <stdint.h>

// Type compatibility definitions
#ifndef int32
#define int32 int32_t
#endif
#ifndef uint8
#define uint8 uint8_t
#endif
#ifndef uint16
#define uint16 uint16_t
#endif

// External reference to conventional memory block
extern uint8 cmem[1114099];

// Current segment pointer for PEEK/POKE operations (exported for call_absolute)
uint8 *defseg = &cmem[1280]; // set to base of DBLOCK

// DEF SEG - Set segment for PEEK/POKE operations
void sub_defseg(int32_t segment, int32_t passed) {
    if (is_error_pending())
        return;
    if (!passed) {
        defseg = &cmem[1280];
        return;
    }

    if ((segment < -65536) || (segment > 65535)) { // same range as QB checks
        error(6);
    } else {
        defseg = &cmem[0] + ((uint16)segment) * 16;
    }
}

// PEEK - Read byte from memory
int32_t func_peek(int32_t offset) {
    if ((offset < -65536) || (offset > 65535)) { // same range as QB checks
        error(6);
        return 0;
    }
    return defseg[(uint16)offset];
}

// POKE - Write byte to memory
void sub_poke(int32_t offset, int32_t value) {
    if (is_error_pending())
        return;
    if ((offset < -65536) || (offset > 65535)) { // same range as QB checks
        error(6);
        return;
    }
    defseg[(uint16)offset] = value;
}
