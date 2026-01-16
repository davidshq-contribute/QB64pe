//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File I/O Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#include "libqb-common.h"

#include "fileio.h"
#include "error_handle.h"
#include "filepath.h"
#include "gfs.h"
#include "handles.h"
#include "http.h"
#include "qblist.h"
#include "qbs.h"
#include "rounding.h"
#include "simple_utils.h"

#include "../../os.h"  // For int32 type definitions

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <inttypes.h>

// External references from libqb.cpp that we need
extern uint8 *cmem;
extern uint8 *defseg;
extern void qbs_print(qbs *str, int32 finish_on_new_line);
extern void evnt(int32 x);
extern qbs *qbs_inkey();
extern int32 stop_program;
extern int32 vwatch;

// Memory functions
extern uint8 *mem_static_pointer;
extern uint8 *mem_static_limit;
extern void *mem_static_malloc(int64 size);

// CPU emulation for interrupt calls
struct cpu_struct {
    uint16 ax, bx, cx, dx, bp, si, di, ds, es;
};
extern cpu_struct cpu;
extern void call_int(int32 intno);

// byte_element_struct is defined in common.h
// byte_element helper from libqb.cpp
extern void *byte_element(uint64 offset, int32 length, byte_element_struct *info);

// Globals defined in this module
int32 generic_get_bytes_read = 0;
// Note: sub_file_print_spaces stays in libqb.cpp since sub_file_print uses it

// Number parsing globals
const char *range_int64_max[] = {"9223372036854775807"};
const char *range_int64_neg_max[] = {"9223372036854775808"};
const char *range_uint64_max[] = {"18446744073709551615"};
const char *range_float_max[] = {"17976931348623157"};

uint16 n_digits = 0;
uint8 n_digit[256];
int64 n_exp = 0;
uint8 n_neg = 0;
uint8 n_hex = 0;
long double n_float_value = 0;
int64 n_int64_value = 0;
uint64 n_uint64_value = 0;

//----------------------------------------------------------------------------------------------------------------------
// Generic I/O helpers
//----------------------------------------------------------------------------------------------------------------------

/**
 * Writes data to a file at a specific offset.
 * 
 * Writes the specified number of bytes from the buffer to the file.
 * The offset is a byte offset from the start of the file (-1 means current position).
 * This function has been largely superseded by gfs_write but is kept for compatibility.
 * 
 * @param i File number
 * @param offset Byte offset from start of file (-1 for current position)
 * @param cp Pointer to data buffer to write
 * @param bytes Number of bytes to write
 * @return Always returns 0 (errors are handled internally)
 */
int32 generic_put(int32 i, int32 offset, uint8 *cp, int32 bytes) {
    // note: generic_put & generic_get have been made largely redundant by gfs_read & gfs_write
    //      offset is a byte-offset from base 0 (-1=current pos)
    //      generic_put has been kept 32-bit for compatibility
    //      the return value of generic_put is always 0
    //      though errors are handled, generic_put should only be called in error-less situations
    if (is_error_pending())
        return 0;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return 0;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (!gfs->write) {
        error(75);
        return 0;
    } // Path/file access error
    static int32 e;
    e = gfs_write(i, offset, (uint8 *)cp, bytes);
    if (e) {
        if (e == -2) {
            error(258);
            return 0;
        } // invalid handle
        if (e == -3) {
            error(54);
            return 0;
        } // bad file mode
        if (e == -4) {
            error(5);
            return 0;
        } // illegal function call
        if (e == -7) {
            error(70);
            return 0;
        } // permission denied
        error(75);
        return 0; // assume[-9]: path/file access error
    }
    return 0;
}

/**
 * Reads data from a file at a specific offset.
 * 
 * Reads the specified number of bytes from the file into the buffer.
 * The offset is a byte offset from the start of the file (-1 means current position).
 * The number of bytes actually read is stored in generic_get_bytes_read.
 * This function has been largely superseded by gfs_read but is kept for compatibility.
 * 
 * @param i File number
 * @param offset Byte offset from start of file (-1 for current position)
 * @param cp Pointer to data buffer to read into
 * @param bytes Number of bytes to read
 * @return Always returns 0 (errors are handled internally)
 */
int32 generic_get(int32 i, int32 offset, uint8 *cp, int32 bytes) {
    // note: generic_put & generic_get have been made largely redundant by gfs_read & gfs_write
    //      offset is a byte-offset from base 0 (-1=current pos)
    //      generic_get has been kept 32-bit for compatibility
    //      the return value of generic_get is always 0
    //      though errors are handled, generic_get should only be called in error-less situations
    generic_get_bytes_read = 0;
    if (is_error_pending())
        return 0;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return 0;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (!gfs->read) {
        error(75);
        return 0;
    } // Path/file access error
    static int32 e;
    e = gfs_read(i, offset, (uint8 *)cp, bytes);
    generic_get_bytes_read = gfs_read_bytes();
    if (e) {
        if (e != -10) { // note: on eof, unread buffer area becomes NULL
            if (e == -2) {
                error(258);
                return 0;
            } // invalid handle
            if (e == -3) {
                error(54);
                return 0;
            } // bad file mode
            if (e == -4) {
                error(5);
                return 0;
            } // illegal function call
            if (e == -7) {
                error(70);
                return 0;
            } // permission denied
            error(75);
            return 0; // assume[-9]: path/file access error
        }
    }
    return 0;
}

//----------------------------------------------------------------------------------------------------------------------
// File character input helpers
//----------------------------------------------------------------------------------------------------------------------

/**
 * Reads a single character from a file.
 * 
 * Returns the ASCII value (0-255) of the next character in the file.
 * Handles EOF character (CHR$(26)) specially by setting the EOF flag.
 * 
 * @param i File number
 * @return ASCII value (0-255) of the character, -1 if EOF reached, -2 on error
 */
int32 file_input_chr(int32 i) {
    // returns the ASCII value of the character (0-255)
    // returns -1 if eof reached (error to be externally handled)
    // returns -2 for other errors (internally handled), the calling function should abort

    static uint8 c;
    auto e = gfs_read(i, -1, &c, 1);
    if (e) {
        if (e == -10)
            return -1;
        if (e == -2) {
            error(258);
            return -2;
        } // invalid handle
        if (e == -3) {
            error(54);
            return -2;
        } // bad file mode
        if (e == -4) {
            error(5);
            return -2;
        } // illegal function call
        if (e == -7) {
            error(70);
            return -2;
        } // permission denied
        error(75);
        return -2; // assume[-9]: path/file access error
    }
    if (c == 26) { // eof character (go back 1 byte so subsequent reads will re-encounter the eof character)
        gfs_setpos(i, gfs_getpos(i) - 1);
        gfs_get_file_struct(i)->eof_passed = 1; // also set EOF flag
        return -1;
    }
    return c;
}

/**
 * Skips the matching newline character in a CR/LF pair.
 * 
 * If a CR (13) or LF (10) character was just read, this function checks
 * if the next character is the matching pair and skips it if so.
 * Also handles EOF character (CHR$(26)) detection.
 * 
 * @param i File number
 * @param c The character that was just read (should be 10 or 13)
 */
void file_input_skip1310(int32 i, int32 c) {
    // assumes a character of value 13 or 10 has just been read (passed)
    // peeks next character and skips it too if it is a corresponding 13 or 10 pair
    static int32 nextc;
    nextc = file_input_chr(i);
    if (nextc == -2)
        return;
    if (nextc == -1)
        return;
    if (((c == 10) && (nextc != 13)) || ((c == 13) && (nextc != 10))) {
        gfs_setpos(i, gfs_getpos(i) - 1); // go back 1 character
    } else {
        // check next character for EOF CHR$(26)
        nextc = file_input_chr(i);
        if (nextc == -2)
            return;
        if (nextc == -1)
            return;
        gfs_setpos(i, gfs_getpos(i) - 1); // go back 1 character
    }
}

/**
 * Advances the file position to the next data item.
 * 
 * Skips whitespace and separators to position the file at the start of
 * the next data item. Handles spaces, commas, and newline characters.
 * 
 * @param i File number
 * @param lastc The last character that was read
 */
void file_input_nextitem(int32 i, int32 lastc) {
    if (i < 0)
        return;
    // this may require reversing a bit too!
    int32 c, nextc;
    c = lastc;
nextchr:
    if (c == -1)
        return;
    if (c == 32) {
        nextc = file_input_chr(i);
        if (nextc == -2)
            return;
        if (nextc == -1)
            return;
        if ((nextc != 32) && (nextc != 44) && (nextc != 10) && (nextc != 13)) {
            gfs_setpos(i, gfs_getpos(i) - 1);
            return;
        } else {
            c = nextc;
            goto nextchr;
        }
    }
    if (c == 44)
        return;                   //,
    if ((c == 10) || (c == 13)) { // lf cr
        file_input_skip1310(i, c);
        return;
    }
    c = file_input_chr(i);
    if (c == -2)
        return;
    goto nextchr;
}

//----------------------------------------------------------------------------------------------------------------------
// Number parsing functions
//----------------------------------------------------------------------------------------------------------------------

/**
 * Determines if rounding should increment the integer part.
 * 
 * Checks the first digit after the decimal point to determine
 * if the integer part should be rounded up (if digit >= 5).
 * 
 * @return 1 if rounding should increment, 0 otherwise
 */
int32 n_roundincrement() {
    static int32 i, i2, i3;
    if (n_digits == 0)
        return 0;
    if (n_digits > (n_exp + 1)) { // numbers exist after the decimal point
        i = n_digit[n_exp + 1] - 48;
        if (i >= 5)
            return 1;
    }
    return 0;
}

/**
 * Converts parsed number digits to a floating-point value.
 * 
 * Processes the digits stored in the global number parsing variables
 * (n_digits, n_exp, n_neg, n_hex) and converts them to a floating-point
 * value stored in n_float_value. Supports decimal, hexadecimal, octal,
 * and binary number formats.
 * 
 * @return 1 on success, 0 on overflow or error
 */
int32 n_float() {
    // return value: Bit 0=successful
    // data
    static uint8 built[256];
    static int64 value;
    uint64 uvalue;
    static int32 i, i2, i3;
    static uint8 *max;
    max = (uint8 *)range_float_max[0];
    n_float_value = 0;
    value = 0;
    uvalue = 0;
    if (n_digits == 0)
        return 1;
    // hex?
    if (n_hex == 1) {
        if (n_digits > 16)
            return 0;
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i];
            if ((i2 >= 48) && (i2 <= 57))
                i2 -= 48;
            if ((i2 >= 65) && (i2 <= 70))
                i2 -= 55;
            if ((i2 >= 97) && (i2 <= 102))
                i2 -= 87;
            value <<= 4;
            value |= i2;
        }
        n_float_value = value;
        return 1;
    }
    // oct?
    if (n_hex == 2) {
        if (n_digits >= 22) {
            if ((n_digits > 22) || (n_digit[0] > 49))
                return 0;
        }
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i] - 48;
            value <<= 3;
            value |= i2;
        }
        n_float_value = value;
        return 1;
    }
    // bin?
    if (n_hex == 3) {
        if (n_digits > 64)
            return 0;
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i] - 48;
            value <<= 1;
            value |= i2;
        }
        n_float_value = value;
        return 1;
    }

    // max range check (+-1.7976931348623157E308)
    if (n_exp > 308)
        return 0; // overflow
    if (n_exp == 308) {
        i2 = n_digits;
        if (i2 > 17)
            i2 = 17;
        for (i = 0; i < i2; i++) {
            if (n_digit[i] > max[i])
                return 0; // overflow
            if (n_digit[i] < max[i])
                break;
        }
    }
    // too close to 0?
    if (n_exp < -324)
        return 1;
    // read & return value (via C++ function)
    // build number
    i = 0;
    if (n_neg) {
        built[i] = 45;
        i++;
    } //-
    built[i] = n_digit[0];
    i++;
    built[i] = 46;
    i++; //.
    if (n_digits == 1) {
        built[i] = 48;
        i++; // 0
    } else {
        i3 = n_digits;
        if (i3 > 17)
            i3 = 17;
        for (i2 = 1; i2 < i3; i2++) {
            built[i] = n_digit[i2];
            i++;
        }
    }
    built[i] = 69;
    i++; // E
    i2 = sprintf((char *)&built[i], "%" PRId64, n_exp);
    i = i + i2;
    built[i] = 0; // NULL terminate for sscanf

    static double sscanf_fix;
    sscanf((char *)&built[0], "%lf", &sscanf_fix);
    n_float_value = sscanf_fix;

    return 1;
}

/**
 * Converts parsed number digits to a 64-bit signed integer value.
 * 
 * Processes the digits stored in the global number parsing variables
 * (n_digits, n_exp, n_neg, n_hex) and converts them to a 64-bit signed
 * integer value stored in n_int64_value. Supports decimal, hexadecimal,
 * octal, and binary number formats with range checking.
 * 
 * @return 1 on success, 0 on overflow or error
 */
int32 n_int64() {
    // return value: Bit 0=successful
    // data
    static int64 value;
    uint64 uvalue;
    static int32 i, i2;
    static uint8 *max;
    static uint8 *neg_max;
    static int64 v0 = build_int64(0x80000000, 0x00000000);
    static int64 v1 = build_int64(0x7FFFFFFF, 0xFFFFFFFF);
    max = (uint8 *)range_int64_max[0];
    neg_max = (uint8 *)range_int64_neg_max[0];
    n_int64_value = 0;
    value = 0;
    uvalue = 0;
    if (n_digits == 0)
        return 1;
    // hex
    if (n_hex == 1) {
        if (n_digits > 16)
            return 0;
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i];
            if ((i2 >= 48) && (i2 <= 57))
                i2 -= 48;
            if ((i2 >= 65) && (i2 <= 70))
                i2 -= 55;
            if ((i2 >= 97) && (i2 <= 102))
                i2 -= 87;
            value <<= 4;
            value |= i2;
        }
        n_int64_value = value;
        return 1;
    }
    // oct
    if (n_hex == 2) {
        if (n_digits >= 22) {
            if ((n_digits > 22) || (n_digit[0] > 49))
                return 0;
        }
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i] - 48;
            value <<= 3;
            value |= i2;
        }
        n_int64_value = value;
        return 1;
    }
    // bin
    if (n_hex == 3) {
        if (n_digits > 64)
            return 0;
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i] - 48;
            value <<= 1;
            value |= i2;
        }
        n_int64_value = value;
        return 1;
    }

    // range check: int64 (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807)
    if (n_exp > 18)
        return 0; // overflow
    if (n_exp == 18) {
        i2 = n_digits;
        if (i2 > 19)
            i2 = 19; // only scan integeral digits
        for (i = 0; i < i2; i++) {
            if (n_neg) {
                if (n_digit[i] > neg_max[i])
                    return 0; // overflow
                if (n_digit[i] < neg_max[i])
                    break;
            } else {
                if (n_digit[i] > max[i])
                    return 0; // overflow
                if (n_digit[i] < max[i])
                    break;
            }
        }
    }
    // calculate integeral value
    i2 = n_digits;
    if (i2 > (n_exp + 1))
        i2 = n_exp + 1;
    for (i = 0; i < (n_exp + 1); i++) {
        uvalue *= 10;
        if (i < i2)
            uvalue = uvalue + (n_digit[i] - 48);
    }
    if (n_neg) {
        value = -uvalue;
    } else {
        value = uvalue;
    }
    // apply rounding
    if (n_roundincrement()) {
        if (n_neg) {
            if (value == v0)
                return 0;
            value--;
        } else {
            if (value == v1)
                return 0;
            value++;
        }
    }
    // return value
    n_int64_value = value;
    return 1;
}

/**
 * Converts parsed number digits to a 64-bit unsigned integer value.
 * 
 * Processes the digits stored in the global number parsing variables
 * (n_digits, n_exp, n_neg, n_hex) and converts them to a 64-bit unsigned
 * integer value stored in n_uint64_value. Supports decimal, hexadecimal,
 * octal, and binary number formats with range checking.
 * 
 * @return 1 on success, 0 on overflow or error
 */
int32 n_uint64() {
    // return value: Bit 0=successful
    // data
    static int64 value;
    uint64 uvalue;
    static int32 i, i2;
    static uint8 *max;
    static int64 v0 = build_uint64(0xFFFFFFFF, 0xFFFFFFFF);
    max = (uint8 *)range_uint64_max[0];
    n_uint64_value = 0;
    value = 0;
    uvalue = 0;
    if (n_digits == 0)
        return 1;
    // hex
    if (n_hex == 1) {
        if (n_digits > 16)
            return 0;
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i];
            if ((i2 >= 48) && (i2 <= 57))
                i2 -= 48;
            if ((i2 >= 65) && (i2 <= 70))
                i2 -= 55;
            if ((i2 >= 97) && (i2 <= 102))
                i2 -= 87;
            uvalue <<= 4;
            uvalue |= i2;
        }
        n_uint64_value = uvalue;
        return 1;
    }
    // oct
    if (n_hex == 2) {
        if (n_digits >= 22) {
            if ((n_digits > 22) || (n_digit[0] > 49))
                return 0;
        }
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i] - 48;
            uvalue <<= 3;
            uvalue |= i2;
        }
        n_uint64_value = uvalue;
        return 1;
    }
    // bin
    if (n_hex == 3) {
        if (n_digits > 64)
            return 0;
        for (i = 0; i < n_digits; i++) {
            i2 = n_digit[i] - 48;
            uvalue <<= 1;
            uvalue |= i2;
        }
        n_uint64_value = uvalue;
        return 1;
    }

    // negative?
    if (n_neg) {
        if (n_exp >= 0)
            return 0; // cannot return a negative number!
    }
    // range check: int64 (0 to 18446744073709551615)
    if (n_exp > 19)
        return 0; // overflow
    if (n_exp == 19) {
        i2 = n_digits;
        if (i2 > 20)
            i2 = 20; // only scan integeral digits
        for (i = 0; i < i2; i++) {
            if (n_digit[i] > max[i])
                return 0; // overflow
            if (n_digit[i] < max[i])
                break;
        }
    }
    // calculate integeral value
    i2 = n_digits;
    if (i2 > (n_exp + 1))
        i2 = n_exp + 1;
    for (i = 0; i < (n_exp + 1); i++) {
        uvalue *= 10;
        if (i < i2)
            uvalue = uvalue + (n_digit[i] - 48);
    }
    // apply rounding
    if (n_roundincrement()) {
        if (n_neg) {
            return 0;
        } else {
            if (uvalue == (uint64)v0)
                return 0;
            uvalue++;
        }
    }
    // return value
    n_uint64_value = uvalue;

    return 1;
}

/**
 * Parses a number from a data buffer.
 * 
 * Reads a number from the data buffer starting at data_offset and updates
 * the offset to point after the number. Supports decimal, hexadecimal (&H),
 * octal (&O), and binary (&B) formats. The parsed number is stored in global
 * variables for conversion by n_float(), n_int64(), or n_uint64().
 * 
 * @param data Pointer to the data buffer
 * @param data_offset Pointer to current offset in buffer (updated on return)
 * @param data_size Total size of the data buffer
 * @return 0 on success, 1 on overflow, 2 if out of data, 3 on syntax error
 */
int32 n_inputnumberfromdata(uint8 *data, ptrszint *data_offset, ptrszint data_size) {
    // return values:
    // 0=success!
    // 1=Overflow
    // 2=Out of DATA
    // 3=Syntax error
    // note: when read fails the data_offset MUST be restored to its old position

    // data
    static int32 i, i2, i3;
    static int32 step, c;
    static int32 exponent_digits;
    static uint8 negate_exponent;
    static int64 exponent_value;
    static int32 return_value;

    return_value = 1; // overflow (default)
    step = 0;
    negate_exponent = 0;
    exponent_value = 0;
    exponent_digits = 0;

    // prepare universal number representation
    n_digits = 0;
    n_exp = 0;
    n_neg = 0;
    n_hex = 0;

    // Out of DATA?
    if (*data_offset >= data_size)
        return 2;

    // read character
    c = data[*data_offset];

    // hex/oct/bin
    if (c == 38) { // "&"
        (*data_offset)++;
        if (*data_offset >= data_size)
            return 3; // Syntax error (missing H/O/B after &)
        c = data[*data_offset];
        if (c == 44) {
            (*data_offset)++;
            return 3; // Syntax error (missing H/O/B after &)
        }
        if ((c == 72) || (c == 104)) { // "H" or "h"
        nexthexchr:
            (*data_offset)++;
            if (*data_offset >= data_size)
                goto gotnumber;
            c = data[*data_offset];
            if (c == 44) {
                (*data_offset)++;
                goto gotnumber;
            }
            if (((c >= 48) && (c <= 57)) || ((c >= 65) && (c <= 70)) || ((c >= 97) && (c <= 102))) { // 0-9 or A-F or a-f
                if (step == 5)
                    return 3; // Syntax error (digits after type suffix)
                if (n_digits == 256)
                    return 1; // Overflow
                n_digit[n_digits] = c;
                n_digits++;
                n_hex = 1;
                goto nexthexchr;
            }
            if ((c == 33) || (c == 35) || (c == 36) || (c == 37) || (c == 38) || (c == 96) || (c == 126)) { // type suffix
                if (step <= 5) {
                    step = 5;
                    goto nexthexchr;
                }
            }
            return 3; // Syntax error (invalid HEX char)
        }
        if ((c == 79) || (c == 111)) { // "O" or "o"
        nexthexchr2:
            (*data_offset)++;
            if (*data_offset >= data_size)
                goto gotnumber;
            c = data[*data_offset];
            if (c == 44) {
                (*data_offset)++;
                goto gotnumber;
            }
            if ((c >= 48) && (c <= 55)) { // 0-7
                if (step == 5)
                    return 3; // Syntax error (digits after type suffix)
                if (n_digits == 256)
                    return 1; // Overflow
                n_digit[n_digits] = c;
                n_digits++;
                n_hex = 2;
                goto nexthexchr2;
            }
            if ((c == 33) || (c == 35) || (c == 36) || (c == 37) || (c == 38) || (c == 96) || (c == 126)) { // type suffix
                if (step <= 5) {
                    step = 5;
                    goto nexthexchr2;
                }
            }
            return 3; // Syntax error (invalid OCT char)
        }
        if ((c == 66) || (c == 98)) { // "B" or "b"
        nexthexchr3:
            (*data_offset)++;
            if (*data_offset >= data_size)
                goto gotnumber;
            c = data[*data_offset];
            if (c == 44) {
                (*data_offset)++;
                goto gotnumber;
            }
            if ((c >= 48) && (c <= 49)) { // 0-1
                if (step == 5)
                    return 3; // Syntax error (digits after type suffix)
                if (n_digits == 256)
                    return 1; // Overflow
                n_digit[n_digits] = c;
                n_digits++;
                n_hex = 3;
                goto nexthexchr3;
            }
            if ((c == 33) || (c == 35) || (c == 36) || (c == 37) || (c == 38) || (c == 96) || (c == 126)) { // type suffix
                if (step <= 5) {
                    step = 5;
                    goto nexthexchr3;
                }
            }
            return 3; // Syntax error (invalid BIN char)
        }
        return 3; // Syntax error (missing H/O/B after &)
    } // "&"

readnextchr:
    if (c == 44) {
        (*data_offset)++;
        goto gotnumber;
    }

    if (c == 45) { //-
        if (step == 0) {
            n_neg = 1;
            step = 1;
            goto nextchr;
        } // sign before integer part
        if (step == 3) {
            negate_exponent = 1;
            step = 4;
            goto nextchr;
        } // exponent sign
        return 3; // Syntax error (no - allowed in fraction part of number or after type suffix)
    }

    if (c == 43) { //+
        if (step == 0) {
            step = 1;
            goto nextchr;
        } // sign before integer part
        if (step == 3) {
            step = 4;
            goto nextchr;
        } // exponent sign
        return 3; // Syntax error (no + allowed in fraction part of number or after type suffix)
    }

    if ((c >= 48) && (c <= 57)) { // 0-9
        if (step == 5)
            return 3; // Syntax error (digit after type suffix)

        if (step <= 1) { // before decimal point
            step = 1;
            if (n_digits || (c > 48)) {
                if (n_digits)
                    n_exp++;
                if (n_digits == 256)
                    return 1; // Overflow
                n_digit[n_digits] = c;
                n_digits++;
            }
        }

        if (step == 2) { // after decimal point
            if ((n_digits == 0) && (c == 48))
                n_exp--;
            if ((n_digits) || (c > 48)) {
                if (n_digits == 256)
                    return 1; // Overflow
                n_digit[n_digits] = c;
                n_digits++;
            }
        }

        if (step >= 3) { // exponent
            step = 4;
            if ((exponent_digits) || (c > 48)) {
                if (exponent_digits == 18)
                    return 1; // Overflow
                exponent_value *= 10;
                exponent_value = exponent_value + (c - 48);
                exponent_digits++;
            }
        }

        goto nextchr;
    }

    if (c == 46) { //.
        if (step > 1)
            return 3; // Syntax error (multiple . or after type suffix)
        if (n_digits == 0)
            n_exp = -1;
        step = 2;
        goto nextchr;
    }

    if ((c == 68) || (c == 69) || (c == 70) || (c == 100) || (c == 101) || (c == 102)) { // D,E,F,d,e,f
        if (step > 2)
            return 3; // Syntax error (multiple exponents or after type suffix)
        step = 3;
        goto nextchr;
    }

    if ((c == 33) || (c == 35) || (c == 36) || (c == 37) || (c == 38) || (c == 96) || (c == 126)) { // type suffix
        if (step <= 5) {
            step = 5;
            goto nextchr;
        }
    }
    return 3; // Syntax error (invalid number char)
nextchr:
    (*data_offset)++;
    if (*data_offset >= data_size)
        goto gotnumber;
    c = data[*data_offset];
    goto readnextchr;

gotnumber:;
    if (negate_exponent)
        n_exp -= exponent_value;
    else
        n_exp += exponent_value; // complete exponent
    if (n_digits == 0) {
        n_exp = 0;
        n_neg = 0;
    } // clarify number
    return 0; // success
}

/**
 * Parses a number from a file.
 * 
 * Reads a number from the file starting at the current file position.
 * Supports decimal, hexadecimal (&H), octal (&O), and binary (&B) formats.
 * The parsed number is stored in global variables for conversion by
 * n_float(), n_int64(), or n_uint64().
 * 
 * @param fileno File number to read from
 * @return 0 on success, 1 on overflow, 2 on EOF, 3 on error
 */
int32 n_inputnumberfromfile(int32 fileno) {
    // return values:
    // 0=success
    // 1=overflow
    // 2=eof
    // 3=failed (no further errors)

    // data
    static int32 i, i2;
    static int32 step, c;
    static int32 exponent_digits;
    static uint8 negate_exponent;
    static int64 exponent_value;
    static int32 return_value;

    // tcp/ip specific data
    static qbs *str, *character;
    int32 nextc, x, x2, x3, x4;
    int32 i1;
    int32 inspeechmarks;
    static uint8 *ucbuf;
    static uint32 ucbufsiz;
    static int32 info;

    if (fileno >= 0) {
        if (gfs_fileno_valid(fileno) != 1) {
            error(52);
            return 3;
        } // Bad file name or number
        fileno = gfs_get_fileno(fileno); // convert fileno to gfs index
        static gfs_file_struct *gfs;
        gfs = gfs_get_file_struct(fileno);
        if (gfs->type != 3) {
            error(54);
            return 3;
        } // Bad file mode
        if (!gfs->read) {
            error(75);
            return 3;
        } // Path/file access error
    }

    return_value = 1; // overflow (default)
    step = 0;
    negate_exponent = 0;
    exponent_value = 0;
    exponent_digits = 0;

    // prepare universal number representation
    n_digits = 0;
    n_exp = 0;
    n_neg = 0;
    n_hex = 0;

    // skip any leading spaces
    do {
        c = file_input_chr(fileno);
        if (c == -2)
            return 3;
        if (c == -1) {
            return_value = 2;
            goto errorlabel;
        } // input past end of file
    } while (c == 32);

    // hex/oct/bin
    if (c == 38) { // "&"
        c = file_input_chr(fileno);
        if (c == -2)
            return 3;
        if (c == -1)
            goto gotnumber;
        if ((c == 72) || (c == 104)) { // "H" or "h"
        nexthexchr:
            c = file_input_chr(fileno);
            if (c == -2)
                return 3;
            if (((c >= 48) && (c <= 57)) || ((c >= 65) && (c <= 70)) || ((c >= 97) && (c <= 102))) { // 0-9 or A-F or a-f
                if (n_digits == 256)
                    goto errorlabel; // overflow
                n_digit[n_digits] = c;
                n_digits++;
                n_hex = 1;
                goto nexthexchr;
            }
            goto gotnumber;
        }
        if ((c == 79) || (c == 111)) { // "O" or "o"
        nexthexchr2:
            c = file_input_chr(fileno);
            if (c == -2)
                return 3;
            if ((c >= 48) && (c <= 55)) { // 0-7
                if (n_digits == 256)
                    goto errorlabel; // overflow
                n_digit[n_digits] = c;
                n_digits++;
                n_hex = 2;
                goto nexthexchr2;
            }
            goto gotnumber;
        }
        if ((c == 66) || (c == 98)) { // "B" or "b"
        nexthexchr3:
            c = file_input_chr(fileno);
            if (c == -2)
                return 3;
            if ((c >= 48) && (c <= 49)) { // 0-1
                if (n_digits == 256)
                    goto errorlabel; // overflow
                n_digit[n_digits] = c;
                n_digits++;
                n_hex = 3;
                goto nexthexchr3;
            }
            goto gotnumber;
        }
        goto gotnumber;
    } // "&"

readnextchr:
    if (c == -1)
        goto gotnumber;

    if (c == 45) { //-
        if (step == 0) {
            n_neg = 1;
            step = 1;
            goto nextchrlabel;
        }
        if (step == 3) {
            negate_exponent = 1;
            step = 4;
            goto nextchrlabel;
        }
        goto gotnumber;
    }

    if (c == 43) { //+
        if (step == 0) {
            step = 1;
            goto nextchrlabel;
        }
        if (step == 3) {
            step = 4;
            goto nextchrlabel;
        }
        goto gotnumber;
    }

    if ((c >= 48) && (c <= 57)) { // 0-9

        if (step <= 1) { // before decimal point
            step = 1;
            if (n_digits || (c > 48)) {
                if (n_digits)
                    n_exp++;
                if (n_digits == 256)
                    goto errorlabel; // overflow
                n_digit[n_digits] = c;
                n_digits++;
            }
        }

        if (step == 2) { // after decimal point
            if ((n_digits == 0) && (c == 48))
                n_exp--;
            if ((n_digits) || (c > 48)) {
                if (n_digits == 256)
                    goto errorlabel; // overflow
                n_digit[n_digits] = c;
                n_digits++;
            }
        }

        if (step >= 3) { // exponent
            step = 4;
            if ((exponent_digits) || (c > 48)) {
                if (exponent_digits == 18)
                    goto errorlabel; // overflow
                exponent_value *= 10;
                exponent_value = exponent_value + (c - 48);
                exponent_digits++;
            }
        }

        goto nextchrlabel;
    }

    if (c == 46) { //.
        if (step > 1)
            goto gotnumber;
        if (n_digits == 0)
            n_exp = -1;
        step = 2;
        goto nextchrlabel;
    }

    if ((c == 68) || (c == 69) || (c == 70) || (c == 100) || (c == 101) || (c == 102)) { // D,E,F,d,e,f
        if (step > 2)
            goto gotnumber;
        step = 3;
        goto nextchrlabel;
    }

    goto gotnumber; // invalid character
nextchrlabel:
    c = file_input_chr(fileno);
    if (c == -2)
        return 3;
    goto readnextchr;

gotnumber:;
    if (negate_exponent)
        n_exp -= exponent_value;
    else
        n_exp += exponent_value; // complete exponent
    if (n_digits == 0) {
        n_exp = 0;
        n_neg = 0;
    } // clarify number
    file_input_nextitem(fileno, c);
    return 0; // success

errorlabel:
    file_input_nextitem(fileno, c);
    if (fileno < 0) {
    }
    return return_value;
}

//----------------------------------------------------------------------------------------------------------------------
// File open/close operations
//----------------------------------------------------------------------------------------------------------------------

/**
 * Opens a file with specified access mode and sharing options.
 * 
 * Opens a file for RANDOM, BINARY, INPUT, OUTPUT, or APPEND access.
 * Supports access restrictions (READ, WRITE) and sharing locks.
 * For RANDOM files, record_length specifies the record size.
 * 
 * @param name Filename as a qbs string
 * @param type File access type: 1=RANDOM, 2=BINARY, 3=INPUT, 4=OUTPUT, 5=APPEND
 * @param access Access restrictions: 1=READ WRITE, 2=READ, 3=WRITE
 * @param sharing Sharing mode: 1=SHARED, 2=LOCK READ WRITE, 3=LOCK READ, 4=LOCK WRITE
 * @param i File number to assign
 * @param record_length Record length for RANDOM files (ignored for other modes)
 * @param passed Bit flags indicating which parameters were provided
 */
void sub_open(qbs *name, int32 type, int32 access, int32 sharing, int32 i, int64 record_length, int32 passed) {
    if (is_error_pending())
        return;
    //?[{FOR RANDOM|FOR BINARY|FOR INPUT|FOR OUTPUT|FOR APPEND}]
    // 1 2
    //[{ACCESS READ WRITE|ACCESS READ|ACCESS WRITE}]
    //  3
    //[{SHARED|LOCK READ WRITE|LOCK READ|LOCK WRITE}]{AS}[#]?[{LEN =}?]
    //  4                                                   5        6[1]
    static int32 x;
    static int32 g_access, g_restrictions, g_how;

    if (type == 0)
        type = 1;
    if (passed)
        if ((record_length == 0) || (record_length < -1)) {
            error(5);
            return;
        } // Illegal function call
    // note: valid record_length values are allowable but ignored by QB for non-RANDOM modes too!

    x = gfs_fileno_valid(i);
    if (x == -2) {
        error(52);
        return;
    } // Bad file name or number
    if (x == 1) {
        error(55);
        return;
    } // File already open

    if (type <= 2) {
        g_access = 3;
        g_restrictions = 0;
        g_how = 3;
    }
    if (type == 3) {
        g_access = 1;
        g_restrictions = 0;
        g_how = 0;
    }
    if (type == 4) {
        g_access = 2;
        g_restrictions = 0;
        g_how = 2;
    }
    if (type == 5) {
        g_access = 2;
        g_restrictions = 0;
        g_how = 1;
    }

    if (access == 1)
        g_access = 3;
    if (access == 2)
        g_access = 1;
    if (access == 3)
        g_access = 2;
    if (access && (g_how == 3))
        g_how = 1; // undefined access not possible when ACCESS is explicitly specified

    if (sharing == 1)
        g_restrictions = 0;
    if (sharing == 2)
        g_restrictions = 3;
    if (sharing == 3)
        g_restrictions = 1;
    if (sharing == 4)
        g_restrictions = 2;
    // note: In QB, opening a file already open for OUTPUT/APPEND created the 'file already open' error.

    //      However, from a new cmd window (or a SHELLed QB program) it can be opened!
    //      So it is not a true OS restriction/lock, just a block applied internally by QB.
    //      This is currently unsupported by QB64.

    x = gfs_open(name, g_access, g_restrictions, g_how);
    if (x < 0) {
        if (x == -5) {
            error(53);
            return;
        }
        if (x == -6) {
            error(76);
            return;
        }
        if (x == -7) {
            error(70);
            return;
        }
        if (x == -8) {
            error(68);
            return;
        }
        if (x == -11) {
            error(64);
            return;
        }
        if (x == -12) {
            error(54);
            return;
        }
        error(53);
        return; // default assumption: 'file not found'
    }

    gfs_fileno_use(i, x);

    static gfs_file_struct *f;
    f = gfs_get_file_struct(x);

    f->type = type;
    if (type == 5)
        f->type = 4;

    f->column = 1;

    if (type == 1) { // set record length
        f->record_length = 128;
        if (passed)
            if (record_length != -1)
                f->record_length = record_length;
        f->field_buffer = (uint8 *)calloc(f->record_length, 1);
    }

    if (type == 5) { // seek eof
        static int64 x64;
        x64 = gfs_lof(x);
        if (x64 > 0)
            gfs_setpos(x, x64); // not an error and not null length
    }

    if (type == 3) { // check if eof character, CHR$(26), is the first byte and set EOF accordingly
        static int64 x64;
        x64 = gfs_lof(x);
        if (x64) {
            // read first byte
            static uint8 c;
            auto e = gfs_read(x, -1, &c, 1);
            if (e) {
                // if (e==-10) return -1;
                // if (e==-2){error(258); return -2;}//invalid handle
                // if (e==-3){error(54); return -2;}//bad file mode
                // if (e==-4){error(5); return -2;}//illegal function call
                if (e == -7) {
                    error(70);
                    return;
                } // permission denied
                error(75);
                return; // assume[-9]: path/file access error
            }
            if (c == 26) {
                gfs_get_file_struct(x)->eof_passed = 1; // set EOF flag
            }
            gfs_setpos(x, 0);
        }
    } // type==3
}

/**
 * Opens a file using GW-BASIC style syntax.
 * 
 * Parses a GW-BASIC style type string (R, B, I, O, A) and opens the file
 * using the standard sub_open function. This provides compatibility with
 * older BASIC file opening syntax.
 * 
 * @param typestr Type string: "R"=RANDOM, "B"=BINARY, "I"=INPUT, "O"=OUTPUT, "A"=APPEND
 * @param i File number to assign
 * @param name Filename as a qbs string
 * @param record_length Record length for RANDOM files
 * @param passed Bit flags indicating which parameters were provided
 */
void sub_open_gwbasic(qbs *typestr, int32 i, qbs *name, int64 record_length, int32 passed) {
    if (is_error_pending())
        return;
    static int32 a, type;
    if (!typestr->len) {
        error(54);
        return;
    } // bad file mode
    a = typestr->chr[0] & 223;
    type = 0;
    if (a == 82)
        type = 1; // R
    if (a == 66)
        type = 2; // B
    if (a == 73)
        type = 3; // I
    if (a == 79)
        type = 4; // O
    if (a == 65)
        type = 5; // A
    if (!type) {
        error(54);
        return;
    } // bad file mode
    if (passed) {
        sub_open(name, type, 0, 0, i, record_length, 1);
    } else {
        sub_open(name, type, 0, 0, i, 0, 0);
    }
}

/**
 * Closes a file or all files.
 * 
 * If a file number is provided, closes that specific file.
 * If no file number is provided, closes all open files and special handles.
 * Handles both regular files and special handles (TCP, HTTP, etc.).
 * 
 * @param i2 File number to close (if passed parameter is set), or ignored if closing all
 * @param passed Bit flags: if set, closes specific file; if not set, closes all files
 */
void sub_close(int32 i2, int32 passed) {
    if (is_error_pending())
        return;
    int32 i, x; //<--RECURSIVE function - do not make this static

    if (passed) {

        if (i2 < 0) { // special handle
            // determine which close procedure to call
            x = -(i2 + 1);
            static stream_struct *st;
            static special_handle_struct *sh;
            sh = (special_handle_struct *)list_get(special_handles, x);
            if (!sh)
                return;

            switch (sh->type) {
            case special_handle_type::Stream:
                st = (stream_struct *)sh->index;

                if (st->type == stream_type::Tcp)
                    connection_close(x);

                break;

            case special_handle_type::Host:
                connection_close(x);
                break;

            case special_handle_type::Http:
                libqb_http_close(x);
                break;

            case special_handle_type::Invalid:
                // TODO: Check if anything needs to be done here
                break;
            }

            return;
        } // special handle

        if (gfs_fileno_valid(i2) == 1)
            gfs_close(gfs_get_fileno(i2));
        return;

    } // passed

    // special handles

    for (i = 1; i <= special_handles->indexes; i++) {
        if (vwatch > 0 && vwatch == i) {
            // keep connection to the IDE open for $DEBUG mode
        } else {
            sub_close(-i - 1, 1);
        }
    }

    gfs_close_all_files();
} // close

/**
 * Gets the next available file number.
 * 
 * Returns the lowest file number that is not currently in use.
 * Useful for opening files without specifying a file number.
 * 
 * @return Next available file number
 */
int32 func_freefile() {
    return gfs_fileno_freefile();
}

//----------------------------------------------------------------------------------------------------------------------
// File status functions
//----------------------------------------------------------------------------------------------------------------------

/**
 * Gets the length of a file in bytes.
 * 
 * Returns the size of the file in bytes. For special handles (TCP, HTTP),
 * returns the content length or available data size.
 * 
 * @param i File number or special handle
 * @return File length in bytes, or 0 on error
 */
int64 func_lof(int32 i) {
    static int64 size;

    if (i < 0) { // special handle?
        static special_handle_struct *sh;
        uint64 length;
        int x = -(i + 1);
        int err;

        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh) {
            error(52);
            return 0;
        }

        static stream_struct *st;
        switch (sh->type) {
        case special_handle_type::Stream:
            st = (stream_struct *)sh->index;
            stream_update(st);
            return st->in_size;

        case special_handle_type::Http:
            err = libqb_http_get_content_length(x, &length);
            if (err != 0)
                return -1;

            return length;

        default:
            error(52);
            return 0;
        }
    } // special handle

    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return 0;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    size = gfs_lof(i);
    if (size < 0) {
        if (size == -2) {
            error(258);
            return 0;
        } // invalid handle
        if (size == -3) {
            error(54);
            return 0;
        } // bad file mode
        if (size == -4) {
            error(5);
            return 0;
        } // illegal function call
        error(75);
        return 0; // assume[-9]: path/file access error
    }
    return size;
}

/**
 * Checks if end of file has been reached.
 * 
 * Returns -1 if EOF has been reached, 0 if more data is available.
 * For special handles (TCP, HTTP), checks connection status and data availability.
 * 
 * @param i File number or special handle
 * @return -1 if EOF reached, 0 if more data available, or 0 on error
 */
int32 func_eof(int32 i) {
    static int32 pos, lof, x;

    if (i < 0) { // special handle?
        x = -(i + 1);
        static special_handle_struct *sh;
        sh = (special_handle_struct *)list_get(special_handles, x);
        if (!sh) {
            error(52);
            return 0;
        }

        static stream_struct *st;
        size_t length = 0;

        switch (sh->type) {
        case special_handle_type::Stream:
            st = (stream_struct *)sh->index;
            if (st->eof)
                return -1;
            return 0;

        case special_handle_type::Http:
            // If sh->index set it overrides the EOF() check. The caller will need to
            // use _CONNECTED() to determine whether more data could be coming.
            if (sh->index == 1)
                return -1;
            else if (sh->index == 2)
                return 0;

            // Only report EOF() if the program had read all incoming data and
            // the connection is finished.
            if (libqb_http_connected(x))
                return 0;

            libqb_http_get_length(x, &length);
            if (length != 0)
                return 0;

            return -1;

        default:
            error(52);
            return 0;
        }
    } // special handle

    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return 0;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->scrn) {
        error(5);
        return 0;
    }
    if (gfs->type != 3) { // uint8 type;//qb access method (1=RANDOM,2=BINARY,3=INPUT,4=OUTPUT)
        if (gfs_eof_passed(i) == 1)
            return -1;
    } else {
        if (gfs_eof_reached(i) == 1)
            return -1;
        if (gfs_eof_passed(i) == 1)
            return -1;
    }
    return 0;
}

/**
 * Sets the file position for reading or writing.
 * 
 * Moves the file pointer to the specified position. For RANDOM files,
 * the position is interpreted as a record number (1-based). For other
 * file types, the position is a byte offset (1-based).
 * 
 * @param i File number
 * @param pos Position: record number for RANDOM files, byte offset for others (1-based)
 */
void sub_seek(int32 i, int64 pos) {
    if (is_error_pending())
        return;
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->type == 1) { // RANDOM
        pos--;
        if (pos < 0) {
            error(63);
            return;
        } // Bad record number
        pos *= gfs->record_length;
        pos++;
    }
    pos--;
    if (pos < 0) {
        error(63);
        return;
    } // Bad record number
    int32 e;
    e = gfs_setpos(i, pos);
    if (e < 0) {
        if (e == -2) {
            error(258);
            return;
        } // invalid handle
        if (e == -3) {
            error(54);
            return;
        } // bad file mode
        if (e == -4) {
            error(5);
            return;
        } // illegal function call
        error(75);
        return; // assume[-9]: path/file access error
    }
}

/**
 * Gets the current file position.
 * 
 * Returns the current position in the file. For RANDOM files, returns
 * the current record number (1-based). For other file types, returns
 * the byte offset (1-based).
 * 
 * @param i File number
 * @return Current position: record number for RANDOM, byte offset for others (1-based), or 0 on error
 */
int64 func_seek(int32 i) {
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return 0;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);
    if (gfs->scrn)
        return 0;
    if (gfs->type == 1) { // RANDOM
        return gfs_getpos(i) / gfs->record_length + 1;
    }
    return gfs_getpos(i) + 1;
}

/**
 * Gets the current file location (similar to LOC function).
 * 
 * Returns location information based on file type:
 * - RANDOM: current record number (0-based)
 * - BINARY: current byte position
 * - INPUT/OUTPUT/APPEND: current position in 128-byte records
 * - COM ports: bytes in input buffer
 * 
 * @param i File number
 * @return Location value based on file type, or 0 on error
 */
int64 func_loc(int32 i) {
    if (gfs_fileno_valid(i) != 1) {
        error(52);
        return 0;
    } // Bad file name or number
    i = gfs_get_fileno(i); // convert fileno to gfs index
    static gfs_file_struct *gfs;
    gfs = gfs_get_file_struct(i);

    if (gfs->scrn) {
        error(5);
        return 0;
    }
    if (gfs->com_port) {
#ifdef QB64_WINDOWS
        static COMSTAT c;
        ZeroMemory(&c, sizeof(COMSTAT));
        static DWORD ignore;
        if (!ClearCommError(gfs->win_handle, &ignore, &c))
            return 0;
        return c.cbInQue; // bytes in COM input buffer
#endif
    }

    if (gfs->type == 1) { // RANDOM
        return gfs_getpos(i) / gfs->record_length;
    }
    if (gfs->type == 2) { // BINARY
        return gfs_getpos(i);
    }
    // APPEND/OUTPUT/INPUT
    return gfs_getpos(i) / 128;
}

//----------------------------------------------------------------------------------------------------------------------
// BLOAD/BSAVE operations
//----------------------------------------------------------------------------------------------------------------------

/**
 * Saves a memory region to a binary file.
 * 
 * Saves a block of memory to a file in QB64 BSAVE format.
 * The file includes a header with signature, segment, offset, and size information.
 * Maximum size is 65536 bytes. The data is read from the current DEF SEG memory region.
 * 
 * @param filename Name of the file to save to
 * @param offset Memory offset within the current segment (0-65535)
 * @param size Number of bytes to save (0-65536)
 */
void sub_bsave(qbs *filename, int32 offset, int32 size) {
    if (is_error_pending())
        return;
    static std::ofstream fh;

    static qbs *tqbs = NULL;
    if (!tqbs)
        tqbs = qbs_new(0, 0);
    static qbs *nullt = NULL;
    if (!nullt)
        nullt = qbs_new(1, 0);

    static int32 x;
    nullt->chr[0] = 0;
    if ((offset < -65536) || (offset > 65535)) {
        error(6);
        return;
    } // Overflow
    offset &= 0xFFFF;
    // note: QB64 allows a maximum of 65536 bytes, QB only allows 65535
    if ((size < -65536) || (size > 65536)) {
        error(6);
        return;
    } // Overflow
    if (size != 65536)
        size &= 0xFFFF;
    qbs_set(tqbs, qbs_add(filename, nullt)); // prepare null-terminated filename
    fh.open(filepath_fix_directory(tqbs), std::ios::binary | std::ios::out);
    if (!fh.is_open()) {
        error(64);
        return;
    } // Bad file name
    x = 253;
    fh.write((char *)&x, 1); // signature byte
    x = (defseg - &cmem[0]) / 16;
    fh.write((char *)&x, 2); // segment
    x = offset;
    fh.write((char *)&x, 2); // offset
    x = size;
    if (x > 65535)
        x = 0;                                 // if filesize>65542 then size=filesize-7
    fh.write((char *)&x, 2);                   // size
    fh.write((char *)(defseg + offset), size); // data
    fh.close();
}

/**
 * Loads a binary file into memory.
 * 
 * Loads a BSAVE format file into memory. If an offset is provided,
 * loads to that position in the current DEF SEG. Otherwise, loads
 * to the position specified in the file header.
 * 
 * @param filename Name of the file to load
 * @param offset Optional memory offset within the current segment (if passed parameter is set)
 * @param passed Bit flags: if set, uses provided offset; if not set, uses file header offset
 */
void sub_bload(qbs *filename, int32 offset, int32 passed) {
    if (is_error_pending())
        return;
    static uint8 header[7];
    static std::ifstream fh;
    static qbs *tqbs = NULL;
    if (!tqbs)
        tqbs = qbs_new(0, 0);
    static qbs *nullt = NULL;
    if (!nullt)
        nullt = qbs_new(1, 0);

    static int32 x, file_seg, file_off, file_size;
    nullt->chr[0] = 0;
    if (passed) {
        if ((offset < -65536) || (offset > 65535)) {
            error(6);
            return;
        } // Overflow
        offset &= 0xFFFF;
    }
    qbs_set(tqbs, qbs_add(filename, nullt)); // prepare null-terminated filename
    fh.open(filepath_fix_directory(tqbs), std::ios::binary | std::ios::in);
    if (!fh.is_open()) {
        error(53);
        return;
    } // File not found
    fh.read((char *)header, 7);
    if (fh.gcount() != 7)
        goto errorlabel;
    if (header[0] != 253)
        goto errorlabel;
    file_seg = header[1] + header[2] * 256;
    file_off = header[3] + header[4] * 256;
    file_size = header[5] + header[6] * 256;
    if (file_size == 0) {
        fh.seekg(0, std::ios::end);
        file_size = fh.tellg();
        fh.seekg(7, std::ios::beg);
        file_size -= 7;
        if (file_size < 65536)
            file_size = 0;
    }
    if (passed) {
        fh.read((char *)(defseg + offset), file_size);
    } else {
        fh.read((char *)(&cmem[0] + file_seg * 16 + file_off), file_size);
    }
    if (fh.gcount() != file_size)
        goto errorlabel;
    fh.close();
    return;
errorlabel:
    fh.close();
    error(54); // Bad file mode
}
