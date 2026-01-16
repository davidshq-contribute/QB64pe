//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Simple Utilities Module
//  Basic utility functions for bit operations, integer building, and character conversion
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "simple_utils.h"
#include "static_data.h"
#include "qbs.h"

#include "../../os.h"  // For int32, int64, uint32, uint64 type definitions

// ============================================================================
// BIT OPERATIONS
// ============================================================================

uint32 rotateLeft(uint32 word, uint32 shift) {
    return (word << shift) | (word >> (32 - shift));
}

// ============================================================================
// INTEGER BUILDING
// ============================================================================

int64 build_int64(uint32 val2, uint32 val1) {
    static int64 val;
    val = val2;
    val <<= 32;
    val |= val1;
    return val;
}

uint64 build_uint64(uint32 val2, uint32 val1) {
    static uint64 val;
    val = val2;
    val <<= 32;
    val |= val1;
    return val;
}

// ============================================================================
// ARITHMETIC HELPERS
// ============================================================================

int32 addone(int32 x) {
    return x + 1;
} // for testing purposes only

// ============================================================================
// CHARACTER CONVERSION
// ============================================================================

uint32 unicode_to_cp437(uint32 x) {
    static int32 i;
    for (i = 0; i <= 255; i++) {
        if (x == codepage437_to_unicode16[i])
            return i;
    }
    return 0;
}

// ============================================================================
// VARIABLE POINTER HELPERS
// ============================================================================

qbs *func_varptr_helper(uint8 type, uint16 offset) {
    //*creates a 3 byte string using the values given
    qbs *tqbs;
    tqbs = qbs_new(3, 1);
    tqbs->chr[0] = type;
    tqbs->chr[1] = offset & 255;
    tqbs->chr[2] = offset >> 8;
    return tqbs;
}
