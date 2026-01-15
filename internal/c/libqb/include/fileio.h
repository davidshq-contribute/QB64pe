//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE File I/O Module
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_FILEIO_H
#define INCLUDE_LIBQB_FILEIO_H

// ============================================================================
// DEPENDENCIES
// ============================================================================

#include "../../os.h"

// Forward declarations
struct qbs;
struct byte_element_struct;

// ============================================================================
// PUBLIC API DECLARATIONS
// ============================================================================

// File open/close operations
void sub_open(qbs *name, int32 type, int32 access, int32 sharing, int32 i, int64 record_length, int32 passed);
void sub_open_gwbasic(qbs *typestr, int32 i, qbs *name, int64 record_length, int32 passed);
void sub_close(int32 i2, int32 passed);
int32 func_freefile();

// BLOAD/BSAVE operations
void sub_bload(qbs *filename, int32 offset, int32 passed);
void sub_bsave(qbs *filename, int32 offset, int32 size);

// File status functions
int64 func_lof(int32 i);
int32 func_eof(int32 i);
void sub_seek(int32 i, int64 pos);
int64 func_seek(int32 i);
int64 func_loc(int32 i);

// ============================================================================
// INTERNAL HELPERS (exposed for other modules)
// ============================================================================

// Generic I/O helpers (internal use but needed by other modules)
int32 generic_put(int32 i, int32 offset, uint8 *cp, int32 bytes);
int32 generic_get(int32 i, int32 offset, uint8 *cp, int32 bytes);
extern int32 generic_get_bytes_read;

// Number input parsing (used by DATA/READ and file INPUT)
// Universal number representation globals
extern uint16 n_digits;
extern uint8 n_digit[256];
extern int64 n_exp;
extern uint8 n_neg;
extern uint8 n_hex;
extern long double n_float_value;
extern int64 n_int64_value;
extern uint64 n_uint64_value;

int32 n_roundincrement();
int32 n_float();
int32 n_int64();
int32 n_uint64();
int32 n_inputnumberfromdata(uint8 *data, ptrszint *data_offset, ptrszint data_size);
int32 n_inputnumberfromfile(int32 fileno);

// File input character helpers
int32 file_input_chr(int32 i);
void file_input_skip1310(int32 i, int32 c);
void file_input_nextitem(int32 i, int32 lastc);

// ============================================================================
// IMPLEMENTATION NOTES
// ============================================================================

// TODO: The following functions remain in libqb.cpp and will be migrated later:
// - sub_file_print
// - sub_file_input_string
// - sub_file_line_input_string
// - func_input
// - sub_get, sub_get2, sub_put, sub_put2

// This module provides file I/O functionality including:
// - File open/close operations with various access modes
// - Binary file operations (BLOAD/BSAVE)
// - File status and positioning functions
// - Number parsing utilities for data input
// - Low-level I/O helper functions

#endif // INCLUDE_LIBQB_FILEIO_H
