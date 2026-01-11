#pragma once

#include <iostream>
#include <stdint.h>

#include "qbs.h"

#ifdef QB64_WINDOWS
#    define GFS_WINDOWS

#    include <wtypes.h>
#endif

#ifndef GFS_WINDOWS
#    define GFS_C
#endif

/**
 * @file gfs.h
 * @brief Generic File System (GFS) for QB64-PE
 * 
 * GFS allows OS-specific access while maintaining 'pure' C-based routines for
 * multiplatform compatibility. 'Pure' C-based routines may not allow certain functionality,
 * such as partial file locking.
 * 
 * GFS handles/indexes are independent of QB64 handles/indexes to allow for internal files
 * to be open without interfering with QB64 file handle numbers.
 * 
 * @name GFS Error Codes
 * @brief Error codes returned by GFS functions
 * 
 * - -1: Non-specific failure
 * - -2: Invalid handle
 * - -3: Bad/incorrect file mode
 * - -4: Illegal function call (input is out of range)
 * - -5: File not found (Windows: 2)
 * - -6: Path not found (Windows: 3)
 * - -7: Access/permission denied (Windows: 5, 19)
 * - -8: Device unavailable/drive invalid (Windows: 15, 21)
 * - -9: Path/file access error
 * - -10: Read past EOF
 * - -11: Bad file name
 */

struct gfs_file_struct { // info applicable to all files
    int64_t id;          // a unique ID given to all files (currently only referenced by the FIELD statement to remove old field conditions)
    uint8_t open;
    uint8_t read;
    uint8_t write;
    uint8_t lock_read;
    uint8_t lock_write;
    int64_t pos;           //-1=unknown
    uint8_t eof_reached;   // read last character of file (set/reset by gfs_read only)
    uint8_t eof_passed;    // attempted to read past eof (set/reset by gfs_read only)
    int32_t fileno;        // link to fileno index
    uint8_t type;          // qb access method (1=RANDOM,2=BINARY,3=INPUT,4=OUTPUT)
    int64_t record_length; // used by RANDOM
    uint8_t *field_buffer;
    qbs **field_strings;     // list of qbs pointers linked to this file
    int32_t field_strings_n; // number of linked strings
    int64_t column;          // used by OUTPUT/APPEND to tab correctly (base 0)
#ifdef GFS_C
    // GFS_C data follows: (unused by custom GFS interfaces)
    std::fstream *file_handle;
    std::ofstream *file_handle_o;
#endif
#ifdef GFS_WINDOWS
    HANDLE win_handle;
#endif
    // COM port data follows (*=default)
    uint8_t com_port;              // 0=not a com port
    int32_t com_baud_rate;         //(bits per second)75,110,150,300*,600,1200,1800,2400,9600,?
    int8_t com_parity;             //[0]N,[1]E*,[2]O,[3]S,[4]M,[5]PE(none,even*,odd,space,mark,error-checking)
    int8_t com_data_bits_per_byte; // 5,6,7*,8
    int8_t com_stop_bits;          //[10]1,[15]1.5,[20]2
    // The default value is 1 for baud rates greater than 110. For
    // baud rates less than or equal to 110, the default value is
    // 1.5 when data is 5; otherwise, the value is 2.
    int8_t com_bin_asc; //[0]=BIN*,[1]=ASC
    int8_t com_asc_lf;  //[0]omit*,[1]LF(only valid with ASC)
    // note: rb_x and tb_x are ignored by QB64 (receive and transmit buffer sizes)
    int8_t com_rs;    //[0]detect*,[1]dont-detect
    int32_t com_cd_x; // 0*-65535
    int32_t com_cs_x; // 1000*,0-65535
    int32_t com_ds_x; // 1000*,0-65535
    int32_t com_op_x;
    //                 OP not used:          x omitted:     x specified:
    //                 10 times the CD or    10000 ms       0 - 65,535 milliseconds
    //                 DS timeout value,
    //                 whichever is greater

    // SCRN: support follows
    uint8_t scrn; // 0 = not a file opened as "SCRN:"
};

/**
 * @brief Checks if EOF was passed (attempted to read past end of file)
 * @param i File handle
 * @return Non-zero if EOF was passed, 0 otherwise
 */
int32_t gfs_eof_passed(int32_t i);

/**
 * @brief Checks if EOF was reached (read last character of file)
 * @param i File handle
 * @return Non-zero if EOF was reached, 0 otherwise
 */
int32_t gfs_eof_reached(int32_t i);

/**
 * @brief Gets the current file position
 * @param i File handle
 * @return Current position in bytes, or -1 if unknown
 */
int64_t gfs_getpos(int32_t i);

/**
 * @brief Checks if a fileno is valid
 * @param f File number to check
 * @return Non-zero if valid, 0 otherwise
 */
int32_t gfs_fileno_valid(int32_t f);

/**
 * @brief Gets a free file number (like QB64 FREEFILE)
 * @return Free file number
 * @note Returns the next available file number that can be used
 */
int32_t gfs_fileno_freefile();

/**
 * @brief Associates a file number with a GFS handle
 * @param f File number
 * @param i GFS handle
 * @note Links a QB64 file number to a GFS internal handle
 */
void gfs_fileno_use(int32_t f, int32_t i);

/**
 * @brief Opens a file
 * @param filename qbs string containing the file path
 * @param access Access mode
 * @param restrictions File restrictions
 * @param how How to open the file
 * @return GFS handle on success, negative error code on failure
 * @note Opens a file and returns a handle for file operations
 */
int32_t gfs_open(qbs *filename, int32_t access, int32_t restrictions, int32_t how);

/**
 * @brief Closes a file
 * @param i GFS handle to close
 * @return 0 on success, negative error code on failure
 * @note Closes the file and releases associated resources
 */
int32_t gfs_close(int32_t i);

/**
 * @brief Gets the length of a file (QB64 LOF function)
 * @param i GFS handle
 * @return File length in bytes, or negative error code on failure
 */
int64_t gfs_lof(int32_t i);

/**
 * @brief Sets the file position
 * @param i GFS handle
 * @param position Position to set
 * @return 0 on success, negative error code on failure
 */
int32_t gfs_setpos(int32_t i, int64_t position);

/**
 * @brief Writes data to a file
 * @param i GFS handle
 * @param position Position to write at
 * @param data Data to write
 * @param size Number of bytes to write
 * @return Number of bytes written, or negative error code on failure
 */
int32_t gfs_write(int32_t i, int64_t position, uint8_t *data, int64_t size);

/**
 * @brief Reads data from a file
 * @param i GFS handle
 * @param position Position to read from
 * @param[out] data Buffer to store read data
 * @param size Number of bytes to read
 * @return Number of bytes read, or negative error code on failure
 */
int32_t gfs_read(int32_t i, int64_t position, uint8_t *data, int64_t size);

/**
 * @brief Gets the number of bytes read in the last read operation
 * @return Number of bytes read
 */
int64_t gfs_read_bytes();

/**
 * @brief Locks a region of a file
 * @param i GFS handle
 * @param offset_start Start offset of the region to lock
 * @param offset_end End offset of the region to lock
 * @return 0 on success, negative error code on failure
 * @note Locks a file region to prevent other processes from accessing it
 */
int32_t gfs_lock(int32_t i, int64_t offset_start, int64_t offset_end);

/**
 * @brief Unlocks a region of a file
 * @param i GFS handle
 * @param offset_start Start offset of the region to unlock
 * @param offset_end End offset of the region to unlock
 * @return 0 on success, negative error code on failure
 */
int32_t gfs_unlock(int32_t i, int64_t offset_start, int64_t offset_end);

/**
 * @brief Gets the fileno for a file number
 * @param file_number QB64 file number
 * @return GFS fileno, or negative error code on failure
 */
int32_t gfs_get_fileno(int file_number);

/**
 * @brief Gets the file structure for a fileno
 * @param fileno GFS fileno
 * @return Pointer to file structure, or NULL on failure
 */
gfs_file_struct *gfs_get_file_struct(int fileno);

/**
 * @brief Closes all open files
 * @note Closes all files opened through GFS. Useful for cleanup.
 */
void gfs_close_all_files();
