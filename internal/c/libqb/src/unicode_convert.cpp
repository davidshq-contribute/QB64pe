//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE Unicode Conversion Module
//  Unicode encoding conversion functions between different formats
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "unicode_convert.h"
#include "static_data.h"

#include "../../os.h"  // For int32, uint8, uint16, uint32 type definitions

// ============================================================================
// UNICODE CONVERSION
// ============================================================================

int32 convert_unicode(int32 src_fmt, void *src_buf, int32 src_size, int32 dest_fmt, void *dest_buf) {
    /*
        important: to ensure enough space is available for the conversion, dest_buf must be at least src_size*4+4 in length
        returns: the number of bytes written to dest_buf
        fmt values:
        1=ASCII(CP437)
        8=UTF8
        16=UTF16
        32=UTF32
    */

    static int32 dest_size;
    dest_size = 0;

    // setup source
    uint8 *src_uint8p = NULL;
    if (src_fmt == 1) {
        src_uint8p = (uint8 *)src_buf;
    }
    uint16 *src_uint16p = NULL;
    if (src_fmt == 16) {
        src_uint16p = (uint16 *)src_buf;
        src_size = src_size - (src_size & 1); // cull trailing bytes
    }
    uint32 *src_uint32p = NULL;
    if (src_fmt == 32) {
        src_uint32p = (uint32 *)src_buf;
        src_size = src_size - (src_size & 3); // cull trailing bytes
    }

    // setup dest
    uint16 *dest_uint16p = NULL;
    if (dest_fmt == 16) {
        dest_uint16p = (uint16 *)dest_buf;
    }
    uint32 *dest_uint32p = NULL;
    if (dest_fmt == 32) {
        dest_uint32p = (uint32 *)dest_buf;
    }

    uint32 x; // scalar

    while (src_size) {

        // convert src to scalar UNICODE value 'x'

        if (src_fmt == 1) { // CP437
            x = *src_uint8p++;
            src_size--;
            x = codepage437_to_unicode16[x];
        }
        if (src_fmt == 16) { // UTF16
            src_size -= 2;
            x = *src_uint16p++;
            // note: does not handle surrogate pairs yet
        }
        if (src_fmt == 32) { // UTF32
            src_size -= 4;
            x = *src_uint32p++;
        }

        // convert scalar UNICODE value 'x' to dest

        if (dest_fmt == 16) { // UTF16
            *dest_uint16p++ = x;
            dest_size += 2;
            // note: does not handle surrogate pairs yet
        }
        if (dest_fmt == 32) { // UTF32
            *dest_uint32p++ = x;
            dest_size += 4;
        }

    } // loop

    // add NULL terminator (does not change the size in bytes returned)
    if (dest_fmt == 16)
        *dest_uint16p = 0;
    if (dest_fmt == 32)
        *dest_uint32p = 0;

    return dest_size;
}
