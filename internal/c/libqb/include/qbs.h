//----------------------------------------------------------------------------------------------------------------------
//  QB64-PE String Management Module
//  QB64 string (QBS) creation, manipulation, and conversion utilities
//  Extracted from libqb.cpp for modularization
//----------------------------------------------------------------------------------------------------------------------

#ifndef INCLUDE_LIBQB_QBS_H
#define INCLUDE_LIBQB_QBS_H

#include <stdint.h>

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/// QB64 string field descriptor for file-based strings
/// Contains metadata for strings linked to file positions
struct qbs_field {
    int32_t fileno;     ///< File number where string is located
    int64_t fileid;     ///< File identifier
    int64_t size;        ///< Size of string data
    int64_t offset;      ///< Offset within file
};

/// QB64 string descriptor structure
/// Main structure for all QB64 string operations
struct qbs {
    uint8_t *chr;         ///< Pointer to string data (32-bit pointer)
    int32_t len;          ///< String length (must be signed for comparisons)
    
    uint8_t in_cmem;      ///< Flag: 1 if in conventional memory DBLOCK
    uint16_t *cmem_descriptor; ///< Memory descriptor pointer
    uint16_t cmem_descriptor_offset; ///< Memory descriptor offset
    
    uint32_t listi;       ///< Index in string reference list
    
    uint8_t tmp;          ///< Flag: 1 if string can be deleted immediately
    uint32_t tmplisti;    ///< Index in temporary string reference list
    
    uint8_t fixed;        ///< Flag: 1 if fixed-length string
    uint8_t readonly;      ///< Flag: 1 if string is read-only
    
    qbs_field *field;     ///< Pointer to field descriptor (for file strings)
};

// ============================================================================
// STRING CREATION FUNCTIONS
// ============================================================================

/// Creates a new QB64 string with specified length
/// Allocates a new string with uninitialized data
/// @param length Length of string to create
/// @param tmp Temporary flag (1=temporary, 0=permanent)
/// @return Pointer to new string structure
qbs *qbs_new(int32_t length, uint8_t tmp);

/// Creates a new QB64 string from C string
/// Copies a null-terminated C string into a QB64 string
/// @param txt Pointer to null-terminated C string
/// @return Pointer to new string structure
qbs *qbs_new_txt(const char *txt);

/// Creates a new QB64 string in conventional memory
/// Allocates string in conventional memory block
/// @param size Size of string to allocate
/// @param tmp Temporary flag (1=temporary, 0=permanent)
/// @return Pointer to new string structure
qbs *qbs_new_cmem(int32_t size, uint8_t tmp);

/// Creates a new QB64 string from C string with length
/// Copies specified number of characters from C string
/// @param txt Pointer to C string data
/// @param len Number of characters to copy
/// @return Pointer to new string structure
qbs *qbs_new_txt_len(const char *txt, int32_t len);

/// Creates a new QB64 string from fixed memory buffer
/// Wraps existing memory buffer as a QB64 string
/// @param offset Pointer to existing memory buffer
/// @param size Size of memory buffer
/// @param tmp Temporary flag (1=temporary, 0=permanent)
/// @return Pointer to new string structure
qbs *qbs_new_fixed(uint8_t *offset, uint32_t size, uint8_t tmp);

// ============================================================================
// STRING MANIPULATION FUNCTIONS
// ============================================================================

/// Concatenates two QB64 strings
/// Creates a new string by appending second string to first
/// @param str1 First string to concatenate
/// @param str2 Second string to concatenate
/// @return Pointer to new concatenated string structure
qbs *qbs_add(qbs *str1, qbs *str2);

/// Sets contents of one QB64 string to another
/// Copies data from second string to first
/// @param str1 Destination string
/// @param str2 Source string
/// @return Pointer to modified destination string structure
qbs *qbs_set(qbs *str1, qbs *str2);

/// Frees a QB64 string structure
/// Releases memory allocated for string
/// @param str Pointer to string structure to free
void qbs_free(qbs *str);

/// Converts QB64 string to numeric value (template)
/// Extracts numeric value from QB64 string
/// @param s Pointer to QB64 string
/// @return Numeric value of specified type
template <typename T> T qbs_val(qbs *s);

// ============================================================================
// LEGACY STR$ FUNCTIONS
// ============================================================================

/// Converts 64-bit integer to string
/// Creates string representation of 64-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(int64_t value);

/// Converts 32-bit integer to string
/// Creates string representation of 32-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(int32_t value);

/// Converts 16-bit integer to string
/// Creates string representation of 16-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(int16_t value);

/// Converts 8-bit integer to string
/// Creates string representation of 8-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(int8_t value);

/// Converts unsigned 64-bit integer to string
/// Creates string representation of unsigned 64-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(uint64_t value);

/// Converts unsigned 32-bit integer to string
/// Creates string representation of unsigned 32-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(uint32_t value);

/// Converts unsigned 16-bit integer to string
/// Creates string representation of unsigned 16-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(uint16_t value);

/// Converts unsigned 8-bit integer to string
/// Creates string representation of unsigned 8-bit integer
/// @param value Integer value to convert
/// @return Pointer to new string structure
qbs *qbs_str(uint8_t value);

/// Converts floating-point number to string
/// Creates string representation of floating-point number
/// @param value Floating-point value to convert
/// @return Pointer to new string structure
qbs *qbs_str(float value);

/// Converts double-precision floating-point number to string
/// Creates string representation of double-precision floating-point number
/// @param value Double-precision floating-point value to convert
/// @return Pointer to new string structure
qbs *qbs_str(double value);

/// Converts long double-precision floating-point number to string
/// Creates string representation of long double-precision floating-point number
/// @param value Long double-precision floating-point value to convert
/// @return Pointer to new string structure
qbs *qbs_str(long double value);

// modern _TOSTR$ function prototypes
qbs *qbs__tostr(int64_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(int32_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(int16_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(int8_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint64_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint32_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint16_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(uint8_t value, int32_t digits, int32_t passed);
qbs *qbs__tostr(float value, int32_t digits, int32_t passed);
qbs *qbs__tostr(double value, int32_t digits, int32_t passed);
qbs *qbs__tostr(long double value, int32_t digits, int32_t passed);

qbs *func_chr(int32_t value);

qbs *qbs_ucase(qbs *str);
qbs *qbs_lcase(qbs *str);
qbs *qbs_left(qbs *str, int32_t l);
qbs *qbs_right(qbs *str, int32_t l);

int32_t qbs_equal(qbs *str1, qbs *str2);
int32_t qbs_notequal(qbs *str1, qbs *str2);
int32_t qbs_greaterthan(qbs *str2, qbs *str1);
int32_t qbs_lessthan(qbs *str1, qbs *str2);
int32_t qbs_lessorequal(qbs *str1, qbs *str2);
int32_t qbs_greaterorequal(qbs *str2, qbs *str1);

int32_t qbs_asc(qbs *str, uint32_t i);
int32_t qbs_asc(qbs *str);

static inline int32_t qbs_len(qbs *str) {
    return str->len;
}

// FIXME: Usages of these outside of qbx.c (and qbs_cleanup()) need to be removed.
extern intptr_t *qbs_tmp_list;
extern uint32_t qbs_tmp_list_lasti;
extern uint32_t qbs_tmp_list_nexti;

template <typename T> static T qbs_cleanup(uint32_t base, T passvalue) {

    while (qbs_tmp_list_nexti > base) {
        qbs_tmp_list_nexti--;
        if (qbs_tmp_list[qbs_tmp_list_nexti] != -1)
            qbs_free((qbs *)qbs_tmp_list[qbs_tmp_list_nexti]);
    } // clear any temp. strings created

    return passvalue;
}

void sub_lset(qbs *dest, qbs *source);
void sub_rset(qbs *dest, qbs *source);
qbs *func_space(int32_t spaces);
qbs *func_string(int32_t characters, int32_t asciivalue);
int32_t func_instr(int32_t start, qbs *str, qbs *substr, int32_t passed);
int32_t func__instrrev(int32_t start, qbs *str, qbs *substr, int32_t passed);
void sub_mid(qbs *dest, int32_t start, int32_t l, qbs *src, int32_t passed);
qbs *func_mid(qbs *str, int32_t start, int32_t l, int32_t passed);
qbs *qbs_ltrim(qbs *str);
qbs *qbs_rtrim(qbs *str);
qbs *qbs__trim(qbs *str);
int32_t func__str_nc_compare(qbs *s1, qbs *s2);
int32_t func__str_compare(qbs *s1, qbs *s2);

// Called by vWatch
static inline void set_qbs_size(intptr_t *target_qbs, int32_t newlength) {
    qbs_set((qbs *)(*target_qbs), func_space(newlength));
}

#endif
