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

// ============================================================================
// FILE OPEN/CLOSE OPERATIONS
// ============================================================================

/// Opens a file with specified access mode and parameters
/// Provides comprehensive file opening with various access modes, sharing options, and record lengths
/// @param name Path and filename of the file to open
/// @param type File type/mode (INPUT, OUTPUT, APPEND, BINARY, RANDOM)
/// @param access Access mode (READ, WRITE, READWRITE)
/// @param sharing File sharing mode (SHARED, LOCK READ, LOCK WRITE, LOCK READ WRITE)
/// @param i File handle number to assign
/// @param record_length Record length for RANDOM access files (in bytes)
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub_open(qbs *name, int32 type, int32 access, int32 sharing, int32 i, int64 record_length, int32 passed);

/// Opens a file using GW-BASIC style syntax
/// Provides compatibility with GW-BASIC OPEN statement format
/// @param typestr String containing file mode and access information
/// @param i File handle number to assign
/// @param name Path and filename of the file to open
/// @param record_length Record length for RANDOM access files (in bytes)
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub_open_gwbasic(qbs *typestr, int32 i, qbs *name, int64 record_length, int32 passed);

/// Closes an open file or all open files
/// Flushes buffers and releases file handle(s)
/// @param i2 File handle to close (use # to close all files)
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub_close(int32 i2, int32 passed);

/// Returns the next available file handle number
/// Finds an unused file handle for opening a new file
/// @return Next available file handle number, or 0 if none available
int32 func_freefile();

// ============================================================================
// BLOAD/BSAVE OPERATIONS
// ============================================================================

/// Loads binary data from a file into memory
/// Loads raw binary data from file into specified memory offset
/// @param filename Path and filename of the binary file to load
/// @param offset Memory offset where data should be loaded
/// @param passed Parameter passing flags (for QB64 compatibility)
void sub_bload(qbs *filename, int32 offset, int32 passed);

/// Saves binary data from memory to a file
/// Saves raw binary data from memory offset to file
/// @param filename Path and filename of the destination file
/// @param offset Memory offset where data starts
/// @param size Number of bytes to save
void sub_bsave(qbs *filename, int32 offset, int32 size);

// ============================================================================
// FILE STATUS FUNCTIONS
// ============================================================================

/// Returns the length of an open file in bytes
/// Gets the total file size for the specified file handle
/// @param i File handle number
/// @return File length in bytes, or -1 on error
int64 func_lof(int32 i);

/// Checks if end of file has been reached
/// Determines if the file pointer is at or beyond the end of file
/// @param i File handle number
/// @return Non-zero if at end of file, 0 if more data available
int32 func_eof(int32 i);

/// Sets the file pointer position for random or binary access
/// Moves the file pointer to the specified position for next read/write operation
/// @param i File handle number
/// @param pos New file position in bytes (1-based for sequential files)
void sub_seek(int32 i, int64 pos);

/// Returns the current file pointer position
/// Gets the current position of the file pointer for the specified file handle
/// @param i File handle number
/// @return Current file position in bytes
int64 func_seek(int32 i);

/// Returns the current record number for random access files
/// Gets the current record position for RANDOM access files
/// @param i File handle number
/// @return Current record number (1-based)
int64 func_loc(int32 i);

// ============================================================================
// INTERNAL HELPERS (exposed for other modules)
// ============================================================================

/// Generic low-level data write operation
/// Writes raw bytes to file at specified offset without format processing
/// @param i File handle number
/// @param offset File position to write to (in bytes)
/// @param cp Pointer to data buffer to write
/// @param bytes Number of bytes to write
/// @return Number of bytes actually written, or -1 on error
int32 generic_put(int32 i, int32 offset, uint8 *cp, int32 bytes);

/// Generic low-level data read operation
/// Reads raw bytes from file at specified offset without format processing
/// @param i File handle number
/// @param offset File position to read from (in bytes)
/// @param cp Pointer to destination buffer
/// @param bytes Number of bytes to read
/// @return Number of bytes actually read, or -1 on error
int32 generic_get(int32 i, int32 offset, uint8 *cp, int32 bytes);

/// Number of bytes read by the last generic_get operation
/// Updated after each generic_get call to indicate actual bytes read
extern int32 generic_get_bytes_read;

// ============================================================================
// NUMBER INPUT PARSING (used by DATA/READ and file INPUT)
// ============================================================================

// Universal number representation globals used during parsing
extern uint16 n_digits;                          ///< Number of digits parsed
extern uint8 n_digit[256];                      ///< Array of parsed digit values
extern int64 n_exp;                              ///< Exponent value for scientific notation
extern uint8 n_neg;                              ///< Flag indicating negative number
extern uint8 n_hex;                              ///< Flag indicating hexadecimal format
extern long double n_float_value;                ///< Parsed floating-point value
extern int64 n_int64_value;                     ///< Parsed 64-bit integer value
extern uint64 n_uint64_value;                   ///< Parsed unsigned 64-bit integer value

/// Calculates rounding increment for floating-point parsing
/// Determines appropriate rounding based on precision requirements
/// @return Rounding increment value
int32 n_roundincrement();

/// Parses floating-point number from input stream
/// Converts text input to floating-point value with proper format handling
/// @return Success status (non-zero on success)
int32 n_float();

/// Parses signed 64-bit integer from input stream
/// Converts text input to signed integer value with overflow checking
/// @return Success status (non-zero on success)
int32 n_int64();

/// Parses unsigned 64-bit integer from input stream
/// Converts text input to unsigned integer value with overflow checking
/// @return Success status (non-zero on success)
int32 n_uint64();

/// Parses number from DATA statement buffer
/// Extracts and parses numeric value from program's DATA statements
/// @param data Pointer to DATA buffer
/// @param data_offset Current offset within DATA buffer (updated)
/// @param data_size Total size of DATA buffer
/// @return Success status (non-zero on success)
int32 n_inputnumberfromdata(uint8 *data, ptrszint *data_offset, ptrszint data_size);

/// Parses number from file input
/// Reads and parses numeric value from text file input
/// @param fileno File handle number to read from
/// @return Success status (non-zero on success)
int32 n_inputnumberfromfile(int32 fileno);

// ============================================================================
// FILE INPUT CHARACTER HELPERS
// ============================================================================

/// Reads a single character from file
/// Gets next character from file input with proper encoding handling
/// @param i File handle number
/// @return Character value, or EOF on end of file
int32 file_input_chr(int32 i);

/// Skips line ending characters (CR/LF) in file input
/// Advances file pointer past carriage return and line feed characters
/// @param i File handle number
/// @param c Current character being processed
void file_input_skip1310(int32 i, int32 c);

/// Advances to next item in file input
/// Moves file pointer to next data item, skipping delimiters and whitespace
/// @param i File handle number
/// @param lastc Last character processed
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
